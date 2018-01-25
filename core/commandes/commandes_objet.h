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

#include "undo.h"

void enregistre_commandes_objet(CommandFactory *usine);

class Object;
class Scene;
class SolverFactory;

class AddObjectCmd : public Command {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	AddObjectCmd() = default;
	~AddObjectCmd() = default;

	void execute(Main *main, const Context &context) override;
	void undo() override;
	void redo() override;
};

class AddNodeCmd : public Command {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	AddNodeCmd() = default;
	~AddNodeCmd() = default;

	void execute(Main *main, const Context &context) override;
	void undo() override;
	void redo() override;
};

class AddPresetObjectCmd : public Command {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	AddPresetObjectCmd() = default;
	~AddPresetObjectCmd() = default;

	void execute(Main *main, const Context &context) override;
	void undo() override;
	void redo() override;
};
