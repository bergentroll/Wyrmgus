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
//      (c) Copyright 2020-2021 by Andrettin
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

#include "database/data_entry_history.h"

class CUpgrade;

namespace wyrmgus {

class upgrade_class;

class civilization_history final : public data_entry_history
{
	Q_OBJECT

	Q_PROPERTY(QVariantList acquired_upgrade_classes READ get_acquired_upgrade_classes_qstring_list)
	Q_PROPERTY(QVariantList acquired_upgrades READ get_acquired_upgrades_qstring_list)

public:
	const std::vector<upgrade_class *> &get_acquired_upgrade_classes() const
	{
		return this->acquired_upgrade_classes;
	}

	QVariantList get_acquired_upgrade_classes_qstring_list() const;

	Q_INVOKABLE void add_acquired_upgrade_class(upgrade_class *upgrade_class)
	{
		this->acquired_upgrade_classes.push_back(upgrade_class);
	}

	Q_INVOKABLE void remove_acquired_upgrade_class(upgrade_class *upgrade_class);

	const std::vector<CUpgrade *> &get_acquired_upgrades() const
	{
		return this->acquired_upgrades;
	}

	QVariantList get_acquired_upgrades_qstring_list() const;

	Q_INVOKABLE void add_acquired_upgrade(CUpgrade *upgrade)
	{
		this->acquired_upgrades.push_back(upgrade);
	}

	Q_INVOKABLE void remove_acquired_upgrade(CUpgrade *upgrade);

private:
	std::vector<upgrade_class *> acquired_upgrade_classes;
	std::vector<CUpgrade *> acquired_upgrades;
};

}
