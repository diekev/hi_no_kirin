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
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "scene.h"

#include <glm/gtc/matrix_transform.hpp>

#include <kamikaze/primitive.h>
#include <kamikaze/outils/chaîne_caractère.h>

Scene::Scene()
    : m_root(new SceneNode)
    , m_current_node(m_root.get())
{}

void Scene::add_node(SceneNode *node)
{
	assert(node != nullptr);

	/* Make sure node has a unique name. */
	auto name = node->name();

	if (ensureUniqueName(name)) {
		node->name(name);
	}

	/* Set node as active. */
	m_active_node = node;

	/* Add node as the child of the current node. */
	m_current_node->add_child(node);

	/* Add node to the dependency graph. */
	m_depsgraph.create_node(node);

	notify_listeners(event_type::object | event_type::added);
}

void Scene::remove_node(SceneNode *node)
{
	assert(node != nullptr);

	/* Add node as the child of the current node. */
	m_current_node->remove_child(node);

	/* Remove node from the dependency graph. */
	m_depsgraph.remove_node(node);

	if (node == m_active_node) {
		m_active_node = nullptr;
	}

	/* Release memory. */
	delete node;

	notify_listeners(event_type::object | event_type::removed);
}

void Scene::intersect(const Ray &/*ray*/)
{
}

/* Select the object which is closest to pos. */
void Scene::selectObject(const glm::vec3 &/*pos*/)
{
	/* TODO: reimplement this, perhaps using a scene level BVH. */
}

Depsgraph *Scene::depsgraph()
{
	return &m_depsgraph;
}

SceneNode *Scene::active_node()
{
	return m_active_node;
}

SceneNode *Scene::current_node()
{
	return m_current_node;
}

void Scene::current_node(SceneNode *node)
{
	m_current_node = node;
}

SceneNode *Scene::root_node()
{
	return m_root.get();
}

void Scene::tagObjectUpdate()
{
	if (!m_active_node) {
		return;
	}

	m_active_node->updateMatrix();

	if (m_active_node->collection()) {
		for (auto &prim : m_active_node->collection()->primitives()) {
			prim->tagUpdate();
		}
	}

	notify_listeners(event_type::object | event_type::modified);
}

bool Scene::ensureUniqueName(std::string &name) const
{
	return ensure_unique_name(name, [&](const std::string &str)
	{
		for (const auto &object : m_nodes) {
			if (object->name() == str) {
				return false;
			}
		}

		return true;
	});
}

void Scene::set_active_node(SceneNode *node)
{
	m_active_node = node;
	notify_listeners(event_type::object | event_type::selected);
}

void Scene::updateForNewFrame(const Context &context)
{
	m_depsgraph.evaluate_for_time_change(context);
}

void Scene::evalObjectDag(const Context &context, SceneNode *node)
{
	m_depsgraph.evaluate(context, node);
}

int Scene::flags() const
{
	return m_flags;
}

void Scene::set_flags(int flag)
{
	m_flags |= flag;
}

void Scene::unset_flags(int flag)
{
	m_flags &= ~flag;
}

bool Scene::has_flags(int flag)
{
	return (m_flags & flag) != 0;
}

void Scene::supprime_tout()
{
	for (const auto &node : m_nodes) {
		m_depsgraph.remove_node(node.get());
	}

	m_nodes.clear();
}

int Scene::startFrame() const
{
	return m_start_frame;
}

void Scene::startFrame(int value)
{
	m_start_frame = value;
	notify_listeners(event_type::time | event_type::modified);
}

int Scene::endFrame() const
{
	return m_end_frame;
}

void Scene::endFrame(int value)
{
	m_end_frame = value;
	notify_listeners(event_type::time | event_type::modified);
}

int Scene::currentFrame() const
{
	return m_cur_frame;
}

void Scene::currentFrame(int value)
{
	m_cur_frame = value;
	notify_listeners(event_type::time | event_type::modified);
}

float Scene::framesPerSecond() const
{
	return m_fps;
}

void Scene::framesPerSecond(float value)
{
	m_fps = value;
	notify_listeners(event_type::time | event_type::modified);
}
