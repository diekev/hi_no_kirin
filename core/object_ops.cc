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

#include "object_ops.h"

#include <kamikaze/context.h>
#include <kamikaze/modifiers.h>
#include <kamikaze/object.h>
#include <kamikaze/paramfactory.h>

#include "scene.h"

/* *************************** add object command *************************** */

AddObjectCmd::~AddObjectCmd()
{
	if (m_was_undone) {
		delete m_object;
	}
}

void AddObjectCmd::execute(EvaluationContext *context)
{
	m_scene = context->scene;
	m_object = (*context->object_factory)(m_name);

	assert(m_object != nullptr);
	m_object->name(m_name.c_str());

	assert(m_scene != nullptr);
	m_scene->addObject(m_object);
}

void AddObjectCmd::undo()
{
	m_scene->removeObject(m_object);
	m_was_undone = true;
}

void AddObjectCmd::redo()
{
	m_scene->addObject(m_object);
	m_was_undone = false;
}

void AddObjectCmd::setUIParams(ParamCallback */*cb*/)
{
}

Command *AddObjectCmd::registerSelf()
{
	return new AddObjectCmd;
}

/* ************************** add modifier command ************************** */

void AddModifierCmd::execute(EvaluationContext *context)
{
	m_scene = context->scene;
	m_object = m_scene->currentObject();

	auto modifier = (*context->modifier_factory)(m_name);
	modifier->setName(m_name);

	assert(m_object != nullptr);
	m_object->addModifier(modifier);

	/* TODO */
	m_scene->evalObjectModifiers();
}

void AddModifierCmd::undo()
{
	//m_scene->removeObject(m_object);
}

void AddModifierCmd::redo()
{
	//m_scene->addObject(m_object);
}

void AddModifierCmd::setUIParams(ParamCallback */*cb*/)
{
}

Command *AddModifierCmd::registerSelf()
{
	return new AddModifierCmd;
}

/* *************************** load object command ************************** */

# if 0
LoadFromFileCmd::LoadFromFileCmd(Scene *scene, const QString &filename)
    : m_scene(scene)
    , m_object(nullptr)
    , m_filename(filename)
    , m_was_undone(false)
{}

LoadFromFileCmd::~LoadFromFileCmd()
{
	if (m_was_undone) {
		delete m_object;
	}
}

void LoadFromFileCmd::execute(EvaluationContext *context)
{
	m_scene = context->scene;

	using openvdb::FloatGrid;
	using openvdb::Vec3s;

	openvdb::initialize();
	openvdb::io::File file(m_filename.toStdString());

	if (!file.open()) {
		std::cerr << "Unable to open file \'" << m_filename.toStdString() << "\'\n";
		return;
	}

	FloatGrid::Ptr grid;

	if (file.hasGrid(openvdb::Name("Density"))) {
		grid = openvdb::gridPtrCast<FloatGrid>(file.readGrid(openvdb::Name("Density")));
	}
	else if (file.hasGrid(openvdb::Name("density"))) {
		grid = openvdb::gridPtrCast<FloatGrid>(file.readGrid(openvdb::Name("density")));
	}
	else {
		openvdb::GridPtrVecPtr grids = file.getGrids();
		grid = openvdb::gridPtrCast<FloatGrid>((*grids)[0]);
	}

	auto meta_map = file.getMetadata();

	file.close();

	if ((*meta_map)["creator"]) {
		auto creator = (*meta_map)["creator"]->str();

		/* If the grid comes from Blender (Z-up), rotate it so it is Y-up */
		if (creator == "Blender/Smoke") {
			Timer("Transform Blender Grid");
			grid = transform_grid(*grid, Vec3s(-M_PI_2, 0.0f, 0.0f),
			                      Vec3s(1.0f), Vec3s(0.0f), Vec3s(0.0f));
		}
	}

	if (grid->getGridClass() == openvdb::GRID_LEVEL_SET) {
		m_object = new LevelSet(grid);
	}
	else {
		m_object = new Volume(grid);
	}

	m_object->name(grid->getName().c_str());
	m_scene->addObject(m_object);
}

void LoadFromFileCmd::undo()
{
	m_scene->removeObject(m_object);
	m_was_undone = true;
}

void LoadFromFileCmd::redo()
{
	m_scene->addObject(m_object);
	m_was_undone = false;
}

#endif