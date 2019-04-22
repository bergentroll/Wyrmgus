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
/**@name detailed_data_element.cpp - The detailed data element source file. */
//
//      (c) Copyright 2019 by Andrettin
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
//

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "detailed_data_element.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

void DetailedDataElement::_bind_methods()
{
	BIND_PROPERTIES();
	
	ClassDB::bind_method(D_METHOD("set_hidden", "hidden"), [](DetailedDataElement *detailed_data_element, const bool hidden){ detailed_data_element->Hidden = hidden; });
	ClassDB::bind_method(D_METHOD("is_hidden"), &DetailedDataElement::IsHidden);
	ClassDB::bind_method(D_METHOD("set_description", "description"), [](DetailedDataElement *detailed_data_element, const String &description){ detailed_data_element->Description = description; });
	ClassDB::bind_method(D_METHOD("get_description"), &DetailedDataElement::GetDescription);
	ClassDB::bind_method(D_METHOD("set_quote", "quote"), [](DetailedDataElement *detailed_data_element, const String &quote){ detailed_data_element->Quote = quote; });
	ClassDB::bind_method(D_METHOD("get_quote"), &DetailedDataElement::GetQuote);
	ClassDB::bind_method(D_METHOD("set_background", "background"), [](DetailedDataElement *detailed_data_element, const String &background){ detailed_data_element->Background = background; });
	ClassDB::bind_method(D_METHOD("get_background"), &DetailedDataElement::GetBackground);
	
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hidden"), "set_hidden", "is_hidden");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description"), "set_description", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "quote"), "set_quote", "get_quote");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "background"), "set_background", "get_background");
}