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

#pragma once

#include "graphs/object_graph.h"
#include "graphs/scene_node.h"

class Node;
class Noeud;
class PrimitiveCollection;

class Object : public SceneNode {
	PrimitiveCollection *m_collection = nullptr;

	glm::mat4 m_matrix = glm::mat4(0.0f);
	glm::mat4 m_inv_matrix = glm::mat4(0.0f);

	Graph m_graph;

	Object *m_parent = nullptr;
	std::vector<Object *> m_children;

public:
	Object(const Context &contexte);
	~Object() = default;

	PrimitiveCollection *collection() const;
	void collection(PrimitiveCollection *coll);

	/* Return the object's matrix. */
	void matrix(const glm::mat4 &m);
	const glm::mat4 &matrix() const;

	/* Noeuds. */
	void ajoute_noeud(Noeud *noeud);

	Graph *graph();
	const Graph *graph() const;

	void updateMatrix();

	void addChild(Object *child);
	void removeChild(Object *child);
	const std::vector<Object *> &children() const;

	Object *parent() const;
	void parent(Object *parent);
};
