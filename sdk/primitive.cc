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

#include "primitive.h"

#include <algorithm>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "outils/chaîne_caractère.h"
#include "outils/rendu.h"

Primitive::Primitive(const Primitive &other)
    : m_dimensions(other.m_dimensions)
    , m_scale(other.m_scale)
    , m_inv_size(other.m_inv_size)
    , m_rotation(other.m_rotation)
    , m_min(other.m_min)
    , m_max(other.m_max)
    , m_pos(other.m_pos)
    , m_matrix(other.m_matrix)
    , m_inv_matrix(other.m_inv_matrix)
    , m_name(other.m_name)
    , m_draw_bbox(other.m_draw_bbox)
    , m_need_update(other.m_need_update)
    , m_need_data_update(other.m_need_data_update)
{
	for (auto attr : m_attributes) {
		delete attr;
	}

	m_attributes.clear();

	for (auto attr : other.m_attributes) {
		this->add_attribute(new Attribute(*attr));
	}
}

Primitive::~Primitive()
{
	for (auto &attr : m_attributes) {
		delete attr;
	}
}

bool Primitive::intersect(const Ray &ray, float &min) const
{
	const auto inv_dir = 1.0f / ray.dir;
	const auto t_min = (m_min - ray.pos) * inv_dir;
	const auto t_max = (m_max - ray.pos) * inv_dir;
	const auto t1 = glm::min(t_min, t_max);
	const auto t2 = glm::max(t_min, t_max);
	const auto t_near = glm::max(t1.x, glm::max(t1.y, t1.z));
	const auto t_far = glm::min(t2.x, glm::min(t2.y, t2.z));

	if (t_near < t_far && t_near < min) {
		min = t_near;
		return true;
	}

	return false;
}

void Primitive::drawBBox(const bool b)
{
	m_draw_bbox = b;
}

bool Primitive::drawBBox() const
{
	return m_draw_bbox;
}

Cube *Primitive::bbox() const
{
	return m_bbox.get();
}

glm::vec3 Primitive::pos() const
{
	return m_pos;
}

glm::vec3 &Primitive::pos()
{
	m_need_update = true;
	return m_pos;
}

glm::vec3 Primitive::scale() const
{
	return m_scale;
}

glm::vec3 &Primitive::scale()
{
	m_need_update = true;
	return m_scale;
}

glm::vec3 Primitive::rotation() const
{
	return m_rotation;
}

glm::vec3 &Primitive::rotation()
{
	m_need_update = true;
	return m_rotation;
}

const glm::mat4 &Primitive::matrix() const
{
	return m_matrix;
}

glm::mat4 &Primitive::matrix()
{
	return m_matrix;
}

void Primitive::matrix(const glm::mat4 &m)
{
	m_matrix = m;
	m_inv_matrix = glm::inverse(m);
}

void Primitive::update()
{
	if (m_need_update) {
		m_bbox.reset(new Cube(m_min, m_max));
		m_need_update = false;
	}
}

void Primitive::tagUpdate()
{
	m_need_update = true;
	m_need_data_update = true;
}

std::string Primitive::name() const
{
	return m_name;
}

void Primitive::name(const std::string &name)
{
	m_name = name;
}

void Primitive::add_attribute(Attribute *attr)
{
	if (!has_attribute(attr->name(), attr->type())) {
		m_attributes.push_back(attr);
	}
}

Attribute *Primitive::add_attribute(const std::string &name, const AttributeType type, size_t size)
{
	auto attr = attribute(name, type);

	if (attr == nullptr) {
		attr = new Attribute(name, type, size);
		m_attributes.push_back(attr);
	}

	return attr;
}

Attribute *Primitive::attribute(const std::string &name, const AttributeType type)
{
	auto iter = std::find_if(m_attributes.begin(), m_attributes.end(),
	                         [&](Attribute *attr)
	{
		return (attr->type() == type) && (attr->name() == name);
	});

	if (iter == m_attributes.end()) {
		return nullptr;
	}

	return *iter;
}

void Primitive::remove_attribute(const std::string &name, const AttributeType type)
{
	auto iter = std::find_if(m_attributes.begin(), m_attributes.end(),
	                         [&](Attribute *attr)
	{
		return (attr->type() == type) && (attr->name() == name);
	});

	if (iter != m_attributes.end()) {
		delete *iter;
	}

	m_attributes.erase(iter);
}

bool Primitive::has_attribute(const std::string &name, const AttributeType type)
{
	return (attribute(name, type) != nullptr);
}

/* ********************************************** */

SubCollection::SubCollection(const std::string &name)
    : m_name(name)
{}

void SubCollection::add(Primitive *primitive)
{
	m_collection.push_back(primitive);
}

void SubCollection::remove(Primitive *primitive)
{
	auto iter = std::find(m_collection.begin(), m_collection.end(), primitive);
	m_collection.erase(iter);
}

const std::string &SubCollection::name() const
{
	return m_name;
}

const bool SubCollection::empty() const
{
	return m_collection.empty();
}

const std::vector<Primitive *> &SubCollection::primitives() const
{
	return m_collection;
}

/* ********************************************** */

PrimitiveCollection::PrimitiveCollection(PrimitiveFactory *factory)
    : PrimitiveCollection()
{
	m_factory = factory;
}

PrimitiveCollection::~PrimitiveCollection()
{
	free_all();
}

Primitive *PrimitiveCollection::build(const std::string &key)
{
	assert(m_factory->registered(key));
	auto prim = (*m_factory)(key);

	this->add(prim);

	return prim;
}

void PrimitiveCollection::add(Primitive *prim)
{
	m_collection.push_back(prim);

	if (prim == nullptr) {
		return;
	}

	auto name = prim->name();

	bool changed = ensure_unique_name(name, [&](const std::string &str)
	{
		for (const auto &prim : m_collection) {
			if (prim->name() == str) {
				return false;
			}
		}

		return true;
	});

	if (changed) {
		prim->name(name);
	}
}

void PrimitiveCollection::clear()
{
	m_collection.clear();
}

void PrimitiveCollection::free_all()
{
	for (auto &prim : m_collection) {
		delete prim;
	}

	clear();
}

PrimitiveCollection *PrimitiveCollection::copy() const
{
	auto collection = new PrimitiveCollection(this->m_factory);

	for (const auto &prim : this->m_collection) {
		collection->add(prim->copy());
	}

	return collection;
}

const std::vector<Primitive *> &PrimitiveCollection::primitives() const
{
	return m_collection;
}

void PrimitiveCollection::destroy(Primitive *prim)
{
	auto iter = std::find(m_collection.begin(), m_collection.end(), prim);

	if (iter != m_collection.end()) {
		delete *iter;
	}

	m_collection.erase(iter);
}

void PrimitiveCollection::destroy(const std::vector<Primitive *> &prims)
{
	for (auto prim : prims) {
		destroy(prim);
	}
}

void PrimitiveCollection::copy_collection(const PrimitiveCollection &coll)
{
	for (auto prim : primitive_iterator(&coll)) {
		this->add(prim);
	}
}

void PrimitiveCollection::merge_collection(PrimitiveCollection &coll)
{
	copy_collection(coll);
	coll.clear();
}

PrimitiveFactory *PrimitiveCollection::factory() const
{
	return m_factory;
}

SubCollection *PrimitiveCollection::create_subcollection(const std::string &name)
{
	auto sub_collection = new SubCollection(name);
	m_subcollection[name] = sub_collection;
	return sub_collection;
}

void PrimitiveCollection::destroy_subcollection(SubCollection *sub_collection)
{
	m_subcollection.erase(sub_collection->name());
	delete sub_collection;
}

int PrimitiveCollection::refcount() const
{
	return m_ref;
}

void PrimitiveCollection::incref()
{
	++m_ref;
}

void PrimitiveCollection::decref()
{
	--m_ref;
}

/* ********************************************** */

primitive_iterator::primitive_iterator()
{
	collection.add(nullptr);
	m_iter = collection.primitives().begin();
	m_end = collection.primitives().end();
}

primitive_iterator::primitive_iterator(const PrimitiveCollection *collection)
    : primitive_iterator(collection, -1)
{}

primitive_iterator::primitive_iterator(const PrimitiveCollection *collection, int type)
    : m_type(type)
{
	if (!collection) {
		this->collection.add(nullptr);
		m_iter = this->collection.primitives().begin();
		m_end = this->collection.primitives().end();
		return;
	}

	m_iter = collection->primitives().begin();
	m_end = collection->primitives().end();

	/* Return if there is no type. */
	if (type == -1) {
		return;
	}

	/* Make sure the first primitive is of the right type. */
	for (const auto &primitive : collection->primitives()) {
		if (primitive->typeID() == type) {
			break;
		}

		++m_iter;
	}
}

primitive_iterator::primitive_iterator(const primitive_iterator &rhs)
    : m_iter(rhs.m_iter)
    , m_end(rhs.m_end)
    , m_type(rhs.m_type)
{}

primitive_iterator &primitive_iterator::operator++()
{
	if (m_type == -1) {
		++m_iter;
		return *this;
	}

	do {
		++m_iter;
	}
	while (m_iter != m_end && (*m_iter)->typeID() != m_type);

	return *this;
}

const primitive_iterator::reference primitive_iterator::operator*() const
{
	return *m_iter;
}

primitive_iterator::pointer primitive_iterator::operator->() const
{
	return &(*m_iter);
}

primitive_iterator::value_type primitive_iterator::get() const
{
	return (m_iter != m_end) ? *m_iter : nullptr;
}

bool operator==(const primitive_iterator &ita, const primitive_iterator &itb) noexcept
{
	Primitive *a = ita.get();
	Primitive *b = itb.get();
	return a == b;
}

bool operator!=(const primitive_iterator &ita, const primitive_iterator &itb) noexcept
{
	return !(ita == itb);
}

primitive_iterator begin(const primitive_iterator &iter)
{
	return iter;
}

primitive_iterator end(const primitive_iterator &)
{
	return primitive_iterator();
}
