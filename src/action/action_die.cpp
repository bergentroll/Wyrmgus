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
/**@name action_die.cpp - The die action. */
//
//      (c) Copyright 1998-2020 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#include "action/action_die.h"

#include "animation.h"
#include "iolib.h"
#include "map/map.h"
#include "unit/unit.h"
#include "unit/unit_type.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/* static */ COrder *COrder::NewActionDie()
{
	return new COrder_Die;
}

/* virtual */ void COrder_Die::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-die\"");
	if (this->Finished) {
		file.printf(", \"finished\"");
	}
	file.printf("}");
}

/* virtual */ bool COrder_Die::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	return false;
}

/* virtual */ bool COrder_Die::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Die::Show(const CViewport &, const PixelPos &lastScreenPos) const
{
	return lastScreenPos;
}


static bool AnimateActionDie(CUnit &unit)
{
	const stratagus::animation_set *animations = unit.GetAnimations();

	if (animations == nullptr) {
		return false;
	}
	if (animations->Death[unit.DamagedType]) {
		UnitShowAnimation(unit, animations->Death[unit.DamagedType].get());
		return true;
	} else if (animations->Death[ANIMATIONS_DEATHTYPES]) {
		UnitShowAnimation(unit, animations->Death[ANIMATIONS_DEATHTYPES].get());
		return true;
	}
	return false;
}


/* virtual */ void COrder_Die::Execute(CUnit &unit)
{
	// Show death animation
	if (AnimateActionDie(unit) == false) {
		// some units has no death animation
		unit.Anim.Unbreakable = 0;
	}
	if (unit.Anim.Unbreakable) {
		return ;
	}
	const stratagus::unit_type &type = *unit.Type;

	// Die sequence terminated, generate corpse.
	if (type.get_corpse_type() == nullptr) {
		unit.Remove(nullptr);
		//Wyrmgus start
		UnitClearOrders(unit);
		//Wyrmgus end
		unit.Release();
		return ;
	}

	const stratagus::unit_type *corpse_type = type.get_corpse_type();
	Assert(type.get_tile_width() >= corpse_type->get_tile_width() && type.get_tile_height() >= corpse_type->get_tile_height());

	// Update sight for new corpse
	// We have to unmark BEFORE changing the type.
	// Always do that, since types can have different vision properties.

	//Wyrmgus start
//	unit.Remove(nullptr);
	MapUnmarkUnitSight(unit);
	//Wyrmgus end
	unit.Type = corpse_type;
	unit.Stats = &corpse_type->Stats[unit.Player->Index];
	//Wyrmgus start
	const unsigned int var_size = UnitTypeVar.GetNumberVariable();
	unit.Variable = corpse_type->Stats[unit.Player->Index].Variables;
	//Wyrmgus end
	UpdateUnitSightRange(unit);
	//Wyrmgus start
//	unit.Place(unit.tilePos);
	MapMarkUnitSight(unit);
	//Wyrmgus end

	unit.Frame = 0;
	UnitUpdateHeading(unit);
	AnimateActionDie(unit); // with new corpse.
}
