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

#include "persona.h"

void Persona::add_prop(std::string name, std::string ui_name, property_type type)
{
	Property prop;
	prop.name = std::move(name);
	prop.ui_name = std::move(ui_name);
	prop.type = type;
	prop.visible = true;

	switch (type) {
		case property_type::prop_bool:
			prop.data = std::experimental::any(false);
			break;
		case property_type::prop_float:
			prop.data = std::experimental::any(0.0f);
			break;
		case property_type::prop_vec3:
			prop.data = std::experimental::any(glm::vec3(0.0f));
			break;
		case property_type::prop_enum:
		case property_type::prop_int:
			prop.data = std::experimental::any(int(0));
			break;
		case property_type::prop_input_file:
		case property_type::prop_output_file:
		case property_type::prop_string:
		case property_type::prop_list:
			prop.data = std::experimental::any(std::string(""));
			break;
	}

	assert(!prop.data.empty());

	m_props.push_back(std::move(prop));
}

void Persona::set_prop_visible(const std::string &prop_name, bool visible)
{
	Property *prop = find_property(prop_name);

	if (prop) {
		prop->visible = visible;
	}
}

bool Persona::update_properties()
{
	return false;
}

int Persona::eval_int(const std::string &prop_name)
{
	const Property *prop = find_property(prop_name);

	if (prop) {
		return std::experimental::any_cast<int>(prop->data);
	}

	return 0;
}

float Persona::eval_float(const std::string &prop_name)
{
	const Property *prop = find_property(prop_name);

	if (prop) {
		return std::experimental::any_cast<float>(prop->data);
	}

	return 0.0f;
}

int Persona::eval_enum(const std::string &prop_name)
{
	return eval_int(prop_name);
}

int Persona::eval_bool(const std::string &prop_name)
{
	const Property *prop = find_property(prop_name);

	if (prop) {
		return std::experimental::any_cast<bool>(prop->data);
	}

	return false;
}

glm::vec3 Persona::eval_vec3(const std::string &prop_name)
{
	const Property *prop = find_property(prop_name);

	if (prop) {
		return std::experimental::any_cast<glm::vec3>(prop->data);
	}

	return glm::vec3(0.0f, 0.0f, 0.0f);
}

std::string Persona::eval_string(const std::string &prop_name)
{
	const Property *prop = find_property(prop_name);

	if (prop) {
		return std::experimental::any_cast<std::string>(prop->data);
	}

	return {};
}

void Persona::set_prop_min_max(const float min, const float max)
{
	Property &prop = this->m_props.back();
	prop.min = min;
	prop.max = max;
}

void Persona::set_prop_enum_values(const EnumProperty &enum_prop)
{
	Property &prop = this->m_props.back();

	assert(prop.type == property_type::prop_enum || prop.type == property_type::prop_list);

	prop.enum_items = enum_prop;
}

void Persona::set_prop_enum_values(const std::string &prop_name, const EnumProperty &enum_prop)
{
	Property *prop = find_property(prop_name);

	if (!prop) {
		return;
	}

	assert(prop->type == property_type::prop_enum || prop->type == property_type::prop_list);

	prop->enum_items = enum_prop;
}

void Persona::set_prop_default_value_int(int value)
{
	Property &prop = this->m_props.back();

	assert(prop.type == property_type::prop_int || prop.type == property_type::prop_enum);

	prop.data = std::experimental::any(value);
}

void Persona::set_prop_default_value_float(float value)
{
	Property &prop = this->m_props.back();

	assert(prop.type == property_type::prop_float);

	prop.data = std::experimental::any(value);
}

void Persona::set_prop_default_value_bool(bool value)
{
	Property &prop = this->m_props.back();

	assert(prop.type == property_type::prop_bool);

	prop.data = std::experimental::any(value);
}

void Persona::set_prop_default_value_string(const std::string &value)
{
	Property &prop = this->m_props.back();

	assert(prop.type == property_type::prop_string ||
	       prop.type == property_type::prop_input_file ||
	       prop.type == property_type::prop_output_file ||
	       prop.type == property_type::prop_list);

	prop.data = std::experimental::any(value);
}

void Persona::set_prop_default_value_vec3(const glm::vec3 &value)
{
	Property &prop = this->m_props.back();

	assert(prop.type == property_type::prop_vec3);

	prop.data = std::experimental::any(value);
}

void Persona::set_prop_tooltip(std::string tooltip)
{
	Property &prop = this->m_props.back();
	prop.tooltip = std::move(tooltip);
}

std::vector<Property> &Persona::props()
{
	return m_props;
}

void Persona::valeur_propriete_bool(const std::string &prop_name, bool valeur)
{
	Property *prop = find_property(prop_name);

	if (!prop) {
		return;
	}

	assert(prop->type == property_type::prop_bool);

	prop->data = std::experimental::any(valeur);
}

void Persona::valeur_propriete_int(const std::string &prop_name, int valeur)
{
	Property *prop = find_property(prop_name);

	if (!prop) {
		return;
	}

	assert(prop->type == property_type::prop_int
		   || prop->type == property_type::prop_enum);

	prop->data = std::experimental::any(valeur);
}

void Persona::valeur_propriete_float(const std::string &prop_name, float valeur)
{
	Property *prop = find_property(prop_name);

	if (!prop) {
		return;
	}

	assert(prop->type == property_type::prop_float);

	prop->data = std::experimental::any(valeur);
}

void Persona::valeur_propriete_vec3(const std::string &prop_name, const glm::vec3 &valeur)
{
	Property *prop = find_property(prop_name);

	if (!prop) {
		return;
	}

	assert(prop->type == property_type::prop_vec3);

	prop->data = std::experimental::any(valeur);
}

void Persona::valeur_propriete_string(const std::string &prop_name, const std::string &valeur)
{
	Property *prop = find_property(prop_name);

	if (!prop) {
		return;
	}

	assert(prop->type == property_type::prop_string
		   || prop->type == property_type::prop_input_file
		   || prop->type == property_type::prop_output_file
		   || prop->type == property_type::prop_list);

	prop->data = std::experimental::any(valeur);
}
