/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2016 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "simulation.h"

#include <kamikaze/context.h>
#include <kamikaze/mesh.h>
#include <kamikaze/prim_points.h>

#include "object.h"
#include "scene.h"

#include "util/utils_glm.h"

struct GravityData {
	int index;
	const char *name;
	float value;
};

enum {
	GRVT_CALLISTO,
	GRVT_CERES,
	GRVT_EARTH,
	GRVT_ERIS,
	GRVT_EUROPA,
	GRVT_GANYMEDE,
	GRVT_IO,
	GRVT_JUPITER,
	GRVT_MARS,
	GRVT_MERCURY,
	GRVT_MOON,
	GRVT_NEPTUN,
	GRVT_OBERON,
	GRVT_PLUTO,
	GRVT_SATURN,
	GRVT_SUN,
	GRVT_TITAN,
	GRVT_TITANIA,
	GRVT_TRITON,
	GRVT_URANUS,
	GRVT_VENUS,

	GRVT_CUSTOM,
};

/* From https://en.wikipedia.org/wiki/Gravity_of_Earth */
static GravityData gravities[] = {
    { GRVT_CALLISTO, "Callisto",  1.235f },
    { GRVT_CERES,    "Ceres",     0.28f },
    { GRVT_EARTH,    "Earth",     9.80665f },
    { GRVT_ERIS,     "Eris",      0.82f },
    { GRVT_EUROPA,   "Europa",    1.314f },
    { GRVT_GANYMEDE, "Ganymede",  1.428f },
    { GRVT_IO,       "Io",        1.796f },
    { GRVT_JUPITER,  "Jupiter",  24.79f },
    { GRVT_MARS,     "Mars",      3.711f },
    { GRVT_MERCURY,  "Mercury",   3.703f },
    { GRVT_MOON,     "Moon",      1.625f },
    { GRVT_NEPTUN,   "Neptune",  11.15f },
    { GRVT_OBERON,   "Oberon",    0.346f },
    { GRVT_PLUTO,    "Pluto",     0.62f },
    { GRVT_SATURN,   "Saturn",   10.44f },
    { GRVT_SUN,      "Sun",     274.1f },
    { GRVT_TITAN,    "Titan",     1.352f },
    { GRVT_TITANIA,  "Titania",   0.379f },
    { GRVT_TRITON,   "Triton",    0.779f },
    { GRVT_URANUS,   "Uranus",    8.69f },
    { GRVT_VENUS,    "Venus",     8.872f },
    { GRVT_CUSTOM,   "Custom",    0.0f },
};

Simulation::Simulation(Solver *solver)
    : m_solver(solver)
{
	m_name = m_solver->name();
	add_input("Input");

	EnumProperty gravity_enum;

	for (const GravityData &gravity : gravities) {
		gravity_enum.insert(gravity.name, gravity.index);
	}

	add_prop("Gravity", property_type::prop_enum);
	set_prop_enum_values(gravity_enum);
	set_prop_default_value_int(GRVT_EARTH);

	add_prop("Gravity (custom)", property_type::prop_float);
	set_prop_default_value_float(1.0f);
	set_prop_min_max(0.0f, 100.0f);

	/* TODO: get properties from solver */

//	for (auto &prop : m_solver->props()) {
//		props.push_back(prop);
//	}
}

Simulation::~Simulation()
{
	delete m_solver;

	for (auto &state : m_states) {
		delete state.second.collection;
	}
}

bool Simulation::update_properties()
{
	auto gravity_index = eval_enum("Gravity");
	set_prop_visible("Gravity (custom)", (gravity_index == GRVT_CUSTOM));
	return true;
}

float Simulation::eval_gravity()
{
	auto gravity_index = eval_enum("Gravity");

	if (gravity_index == GRVT_CUSTOM) {
		return eval_float("Gravity (custom)");
	}

	return gravities[gravity_index].value;
}

void Simulation::step(const EvaluationContext * const context)
{
	auto scene = context->scene;

	if (m_start_frame != scene->startFrame()) {
		m_start_frame = scene->startFrame();
	}

	if (scene->currentFrame() == m_start_frame) {
		/* Save object state. */
		sync_states();

		m_last_frame = scene->currentFrame();
		return;
	}

	/* Make sure we didn't advance too much. */
	if (scene->currentFrame() != m_last_frame + 1) {
		return;
	}

	m_last_frame = scene->currentFrame();

	m_simcontext.gravity = glm::vec3(0.0f, -eval_gravity(), 0.0f);
	const auto frame_time = 1.0f / scene->framesPerSecond();
	m_simcontext.time_step = (context->time_direction == TIME_DIR_BACKWARD) ? -frame_time : frame_time;

	for (SceneInputSocket *input : m_inputs) {
		if (!input->link) {
			continue;
		}

		Object *object = static_cast<Object *>(input->link->parent);

		m_solver->solve_for_object(m_simcontext, object);
	}
}

void Simulation::sync_states()
{
	for (SceneInputSocket *input : m_inputs) {
		if (!input->link) {
			continue;
		}

		auto object = static_cast<Object *>(input->link->parent);

		auto iter = m_states.find(object);

		if (iter == m_states.end()) {
			ObjectState state;

			if ((m_solver->flags() & solver_flag::data) != 0) {
				state.collection = object->collection()->copy();
			}

			if ((m_solver->flags() & solver_flag::transform) != 0) {
				state.pos = object->eval_vec3("Position");
				state.rot = object->eval_vec3("Rotation");
				state.scale = object->eval_vec3("Size");
			}

			m_states[object] = state;
		}
		else {
			const ObjectState &state = iter->second;

			if ((m_solver->flags() & solver_flag::data) != 0) {
				auto collection = object->collection();
				object->collection(state.collection->copy());
				delete collection;
			}

			if ((m_solver->flags() & solver_flag::transform) != 0) {
				object->set_prop_value("Position", state.pos);
				object->set_prop_value("Rotation", state.rot);
				object->set_prop_value("Size", state.scale);
				object->updateMatrix();
			}
		}

		m_solver->add_required_attributes(object);
	}
}

/* ************************************************************************** */

void register_builtin_solvers(SolverFactory *factory)
{
	REGISTER_SOLVER(factory, "Free Fall Solver", FreeFallSolver);
	REGISTER_SOLVER(factory, "Simple Particle Solver", SimpleParticleSolver);
	REGISTER_SOLVER(factory, "Simple RBD Solver", SimpleRBDSolver);
}

/* ************************************************************************** */

FreeFallSolver::FreeFallSolver()
{
	set_flags(solver_flag::transform);
}

const char *FreeFallSolver::name() const
{
	return "Free Fall Solver";
}

void FreeFallSolver::solve_for_object(const SimulationContext &context, Object *object)
{
	auto pos = object->eval_vec3("Position");
	pos += context.time_step * context.gravity;
	object->set_prop_value("Position", pos);

	object->updateMatrix();
}

/* ************************************************************************** */

struct PhysicsPlane {
	glm::vec3 pos;
	glm::vec3 nor;
};

static bool check_collision(PhysicsPlane *plane, const glm::vec3 &pos, const glm::vec3 &vel)
{
	const auto &XPdotN = glm::dot(pos - plane->pos, plane->nor);

	/* Is it within epsilon of the plane? */
	if (XPdotN >= std::numeric_limits<float>::epsilon()) {
		return false;
	}

	/* Is it going towards the plane (less than zero)? */
	if (glm::dot(plane->nor, vel) >= 0.0f) {
		return false;
	}

	return true;
}

SimpleParticleSolver::SimpleParticleSolver()
{
	set_flags(solver_flag::data);
}

const char *SimpleParticleSolver::name() const
{
	return "Simple Particle Solver";
}

void SimpleParticleSolver::solve_for_object(const SimulationContext &context, Object *object)
{
	/* TODO: expose this somehow. */
//	const auto &mass = 0.05f;

	PhysicsPlane plane;
	plane.pos = glm::vec3(0.0f, 0.0f, 0.0f);
	plane.nor = glm::vec3(0.0f, 1.0f, 0.0f);

	/* Only work on points primitive. */
	for (Primitive *prim : primitive_iterator(object->collection(), PrimPoints::id)) {
		auto particles = static_cast<PrimPoints *>(prim);
		auto points = particles->points();
		auto velocity_attr = particles->addAttribute("velocity", ATTR_TYPE_VEC3, points->size());

		for (size_t i = 0, e = points->size(); i < e; ++i) {
			/* compute velocity: v = f/m */
			const auto &force = context.time_step * context.gravity;
			const auto &velocity = velocity_attr->vec3(i) + force;
			velocity_attr->vec3(i, velocity);

			(*points)[i] += velocity;
			auto pos = (*points)[i];

			/* Get pos in object space. */
			pos = glm::mat3(object->matrix()) * pos + object->eval_vec3("Position");

			if (check_collision(&plane, pos, velocity)) {
//				const auto &delta = plane.pos - object->eval_vec3("Position");
//				(*points)[i][1] = delta[1];
				/* Do collision response */
				velocity_attr->vec3(i, -velocity);
			}
		}

		prim->tagUpdate();
	}
}

void SimpleParticleSolver::add_required_attributes(Object *object)
{
	for (Primitive *prim : primitive_iterator(object->collection(), PrimPoints::id)) {
		auto particles = static_cast<PrimPoints *>(prim);
		auto points = particles->points();
		auto vel_attr = particles->addAttribute("velocity", ATTR_TYPE_VEC3, points->size());

		for (size_t i = 0, e = points->size(); i < e; ++i) {
			vel_attr->vec3(i, glm::vec3(0.0f, 0.0f, 0.0f));
		}
	}
}

/* ************************************************************************** */

SimpleRBDSolver::SimpleRBDSolver()
{
	set_flags(solver_flag::data);
}

const char *SimpleRBDSolver::name() const
{
	return "Simple RBD Solver";
}

void SimpleRBDSolver::solve_for_object(const SimulationContext &context, Object *object)
{
	/* TODO: expose this somehow. */
//	const auto &mass = 1.0f;

	PhysicsPlane plane;
	plane.pos = glm::vec3(0.0f, 0.0f, 0.0f);
	plane.nor = glm::vec3(0.0f, 1.0f, 0.0f);

	/* Only work on points primitive. */
	for (Primitive *prim : primitive_iterator(object->collection(), Mesh::id)) {
		std::cerr << "=========================\n";

		auto mesh = static_cast<Mesh *>(prim);
		auto points = mesh->points();
		auto velocity_attr = mesh->attribute("velocity", ATTR_TYPE_VEC3);

		std::cerr << "Number of points: " << points->size() << '\n';

		const auto &force = context.time_step * context.gravity;
		auto center_of_mass = glm::vec3(0.0f, 0.0f, 0.0f);
		auto pos = glm::vec3(0.0f, 0.0f, 0.0f);

		/* Compute center of mass: x = sum_i(m_i*x_i)/M, where:
		 * - m_i: mass of the individual points
		 * - x_i: positions of the individual points
		 * - M: mass of the object
		 *
		 *  TODO: mass of the individual points
		 */
		for (size_t i = 0, e = points->size(); i < e; ++i) {
			pos = glm::mat3(object->matrix()) * (*points)[i] + object->eval_vec3("Position");
			center_of_mass += pos;
		}

		/* TODO: mass of the object */
		center_of_mass /= points->size();

		std::cerr << "Center of mass: " << center_of_mass << '\n';

		auto torque = glm::vec3(0.0f, 0.0f, 0.0f);

		/* Compute torque. */
		for (size_t i = 0, e = points->size(); i < e; ++i) {
			pos = glm::mat3(object->matrix()) * (*points)[i] + object->eval_vec3("Position");
			torque += (pos - center_of_mass) * force;
		}

		std::cerr << "Torque: " << torque << '\n';

		for (size_t i = 0, e = points->size(); i < e; ++i) {
			/* compute velocity: v = f/m */
			const auto &velocity = velocity_attr->vec3(i) + force;
			velocity_attr->vec3(i, velocity);

			(*points)[i] += velocity;
		}

		prim->tagUpdate();
	}
}

void SimpleRBDSolver::add_required_attributes(Object *object)
{
	for (Primitive *prim : primitive_iterator(object->collection(), Mesh::id)) {
		auto mesh = static_cast<Mesh *>(prim);
		auto points = mesh->points();
		auto vel_attr = mesh->addAttribute("velocity", ATTR_TYPE_VEC3, points->size());

		for (size_t i = 0, e = points->size(); i < e; ++i) {
			vel_attr->vec3(i, glm::vec3(0.0f, 0.0f, 0.0f));
		}

		auto torque_attr = mesh->addAttribute("torque", ATTR_TYPE_VEC3, 1);
		torque_attr->float_(0, 0.0f);
	}
}

/* ************************************************************************** */

solver_flag &operator&=(solver_flag &lhs, solver_flag rhs)
{
	return (lhs = lhs & rhs);
}

solver_flag &operator|=(solver_flag &lhs, solver_flag rhs)
{
	return (lhs = lhs | rhs);
}

solver_flag &operator^=(solver_flag &lhs, solver_flag rhs)
{
	return (lhs = lhs ^ rhs);
}

solver_flag Solver::flags()
{
	return m_flags;
}

void Solver::set_flags(solver_flag flags)
{
	m_flags |= flags;
}

void Solver::unset_flags(solver_flag flags)
{
	m_flags &= ~flags;
}