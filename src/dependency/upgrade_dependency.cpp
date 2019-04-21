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
/**@name upgrade_dependency.cpp - The upgrade dependency source file */
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

#include "dependency/upgrade_dependency.h"

#include "player.h"
#include "unit/unit.h"
#include "upgrade/upgrade.h"
#include "upgrade/upgrade_structs.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

void CUpgradeDependency::ProcessConfigDataProperty(const std::pair<std::string, std::string> &property)
{
	const std::string &key = property.first;
	std::string value = property.second;
	if (key == "upgrade") {
		value = FindAndReplaceString(value, "_", "-");
		this->Upgrade = CUpgrade::Get(value);
		if (!this->Upgrade) {
			fprintf(stderr, "Invalid upgrade: \"%s\".\n", value.c_str());
		}
	} else {
		fprintf(stderr, "Invalid upgrade dependency property: \"%s\".\n", key.c_str());
	}
}

bool CUpgradeDependency::Check(const CPlayer *player, const bool ignore_units) const
{
	return UpgradeIdAllowed(*player, this->Upgrade->ID) == 'R';
}

bool CUpgradeDependency::Check(const CUnit *unit, const bool ignore_units) const
{
	return this->Check(unit->Player, ignore_units) || unit->GetIndividualUpgrade(this->Upgrade);
}

std::string CUpgradeDependency::GetString(const std::string &prefix) const
{
	std::string str = prefix + this->Upgrade->GetName().utf8().get_data() + '\n';
	return str;
}
