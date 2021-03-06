//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
//      (c) Copyright 2021 by Andrettin
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.

#include "stratagus.h"

#include "ui/interface_style.h"

#include "database/database.h"
#include "ui/button_state.h"
#include "ui/button_style.h"
#include "ui/interface_element_type.h"
#include "video/video.h"

namespace wyrmgus {

interface_style::interface_style(const std::string &identifier) : data_entry(identifier)
{
}

interface_style::~interface_style()
{
}

void interface_style::process_sml_scope(const sml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "large_button") {
		this->large_button = std::make_unique<button_style>(this);
		database::process_sml_data(this->large_button, scope);
	} else {
		data_entry::process_sml_scope(scope);
	}
}

void interface_style::initialize()
{
	if (this->large_button != nullptr) {
		this->large_button->initialize();
	}

	data_entry::initialize();
}

const std::shared_ptr<CGraphic> &interface_style::get_interface_element_graphics(const interface_element_type type, const std::string &qualifier) const
{
	switch (type) {
		case interface_element_type::large_button: {
			const button_style *button = this->get_button(type);
			const button_state state = string_to_button_state(qualifier);
			return button->get_graphics(state);
		}
		default:
			throw std::runtime_error("Invalid interface element type: \"" + std::to_string(static_cast<int>(type)) + "\".");
	}
}

const button_style *interface_style::get_button(const interface_element_type type) const
{
	switch (type) {
		case interface_element_type::large_button:
			return this->large_button.get();
		default:
			throw std::runtime_error("Invalid button interface element type: \"" + std::to_string(static_cast<int>(type)) + "\".");
	}
}

}
