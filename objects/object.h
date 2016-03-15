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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#pragma once

#include <ego/bufferobject.h>
#include <ego/program.h>
#include <QString>

#include "util/util_render.h"

class ViewerContext;

enum {
	DRAW_WIRE = 0,
	DRAW_SOLID = 1,
};

enum {
	OBJECT    = 0,
	LEVEL_SET = 1,
	VOLUME    = 2,
};

class Object {
protected:
	ego::BufferObject::Ptr m_buffer_data;
	ego::Program m_program;
	size_t m_elements;
	unsigned int m_draw_type;

	std::vector<glm::vec3> m_vertices;
	glm::vec3 m_dimensions, m_scale, m_inv_size, m_rotation;
	glm::vec3 m_min, m_max, m_pos;
	glm::mat4 m_matrix, m_inv_matrix;

	QString m_name;

	bool m_draw_bbox, m_draw_topology, m_need_update;

	void updateMatrix();

public:
	Object();
	virtual ~Object() = default;

	virtual int type() const { return OBJECT; }

	virtual bool intersect(const Ray &ray, float &min) const;
	virtual void render(ViewerContext *context, const bool for_outline) = 0;
	void setDrawType(int draw_type);

	virtual void drawBBox(const bool b);
	virtual bool drawBBox() const { return m_draw_bbox; }
	virtual void drawTreeTopology(const bool b);
	virtual bool drawTreeTopology() const { return m_draw_topology; }

	glm::vec3 pos() const;
	glm::vec3 &pos() { m_need_update = true; return m_pos; }
	glm::vec3 scale() const;
	glm::vec3 &scale() { m_need_update = true; return m_scale; }
	glm::vec3 rotation() const;
	glm::vec3 &rotation() { m_need_update = true; return m_rotation; }

	/* Return the object's matrix, mainly intended for rendering the active object */
	glm::mat4 matrix() const { return m_matrix; }
	glm::mat4 &matrix() { return m_matrix; }

	virtual void update();

	QString name() const;
	void name(const QString &name);
};
