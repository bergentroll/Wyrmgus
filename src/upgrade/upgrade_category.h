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

#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace wyrmgus {

enum class upgrade_category_rank;

class upgrade_category final : public named_data_entry, public data_type<upgrade_category>
{
	Q_OBJECT

	Q_PROPERTY(wyrmgus::upgrade_category_rank rank MEMBER rank READ get_rank)
	Q_PROPERTY(wyrmgus::upgrade_category* category MEMBER category)

public:
	static constexpr const char *class_identifier = "upgrade_category";
	static constexpr const char *database_folder = "upgrade_categories";

	explicit upgrade_category(const std::string &identifier);

	virtual void check() const override;

	upgrade_category_rank get_rank() const
	{
		return this->rank;
	}

	const upgrade_category *get_category() const
	{
		return this->category;
	}

	const upgrade_category *get_category(const upgrade_category_rank rank) const
	{
		if (this->get_category() != nullptr) {
			if (this->get_category()->get_rank() == rank) {
				return this->get_category();
			}

			if (this->get_category()->get_rank() < rank) {
				return this->get_category()->get_category(rank);
			}
		}

		return nullptr;
	}

private:
	upgrade_category_rank rank;
	upgrade_category *category = nullptr; //the upper category to which this upgrade category belongs
};

}
