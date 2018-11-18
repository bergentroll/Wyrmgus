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
/**@name terrain_type.h - The terrain type headerfile. */
//
//      (c) Copyright 2018 by Andrettin
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

#ifndef __TERRAIN_TYPE_H__
#define __TERRAIN_TYPE_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "color.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CConfigData;
class CGraphic;
class CPlayerColorGraphic;
class CUnitType;

class CTerrainType
{
public:
	CTerrainType() :
		ID(-1), Flags(0), SolidAnimationFrames(0), Resource(-1),
		Overlay(false), Buildable(false), AllowSingle(false), Hidden(false),
		PixelSize(32, 32),
		UnitType(NULL), Graphics(NULL), ElevationGraphics(NULL), PlayerColorGraphics(NULL)
	{
		Color.R = 0;
		Color.G = 0;
		Color.B = 0;
		Color.A = 0;
	}
	
	static CTerrainType *GetTerrainType(std::string ident);
	static CTerrainType *GetOrAddTerrainType(std::string ident);
	static void LoadTerrainTypeGraphics();
	static void ClearTerrainTypes();
	
	static std::vector<CTerrainType *>  TerrainTypes;
	static std::map<std::string, CTerrainType *> TerrainTypesByIdent;
	static std::map<std::string, CTerrainType *> TerrainTypesByCharacter;
	static std::map<std::tuple<int, int, int>, CTerrainType *> TerrainTypesByColor;

	void ProcessConfigData(CConfigData *config_data);

	std::string Ident;
	std::string Name;
	std::string Character;
	CColor Color;
	int ID;
	int SolidAnimationFrames;
	int Resource;
	unsigned int Flags;
	bool Overlay;												/// Whether this terrain type belongs to the overlay layer
	bool Buildable;
	bool AllowSingle;											/// Whether this terrain type has transitions for single tiles
	bool Hidden;
	PixelSize PixelSize;
	CUnitType *UnitType;
	CGraphic *Graphics;
	CGraphic *ElevationGraphics;								/// Semi-transparent elevation graphics, separated so that borders look better
	CPlayerColorGraphic *PlayerColorGraphics;
	std::vector<CTerrainType *> BaseTerrainTypes;				/// Possible base terrain types for this terrain type (if it is an overlay terrain)
	std::vector<CTerrainType *> BorderTerrains;					/// Terrain types which this one can border
	std::vector<CTerrainType *> InnerBorderTerrains;			/// Terrain types which this one can border, and which "enter" this tile type in transitions
	std::vector<CTerrainType *> OuterBorderTerrains;			/// Terrain types which this one can border, and which are "entered" by this tile type in transitions
	std::vector<int> SolidTiles;
	std::vector<int> DamagedTiles;
	std::vector<int> DestroyedTiles;
	std::map<std::tuple<int, int>, std::vector<int>> TransitionTiles;	/// Transition graphics, mapped to the tile type (-1 means any tile) and the transition type (i.e. northeast outer)
	std::map<std::tuple<int, int>, std::vector<int>> AdjacentTransitionTiles;	/// Transition graphics for the tiles adjacent to this terrain type, mapped to the tile type (-1 means any tile) and the transition type (i.e. northeast outer)
};

//@}

#endif // !__TERRAIN_TYPE_H__