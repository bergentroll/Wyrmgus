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
/**@name map.h - The map headerfile. */
//
//      (c) Copyright 1998-2006 by Vladi Shabanski, Lutz Sammer, and
//                                 Jimmy Salmon
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

#ifndef __MAP_H__
#define __MAP_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class CMap map.h
**
**  \#include "map.h"
**
**  This class contains all information about a Stratagus map.
**  A map is a rectangle of any size.
**
**  The map class members:
**
**  CMap::Fields
**
**    An array CMap::Info::Width * CMap::Info::Height of all fields
**    belonging to this map.
**
**  CMap::NoFogOfWar
**
**    Flag if true, the fog of war is disabled.
**
**  CMap::Tileset
**
**    Tileset data for the map. See ::CTileset. This contains all
**    information about the tile.
**
**  CMap::TileModelsFileName
**
**    Lua filename that loads all tilemodels
**
**  CMap::TileGraphic
**
**    Graphic for all the tiles
**
**  CMap::FogGraphic
**
**    Graphic for fog of war
**
**  CMap::Info
**
**    Descriptive information of the map. See ::CMapInfo.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string>
//Wyrmgus start
#include <map>
//Wyrmgus end

#ifndef __MAP_TILE_H__
#include "tile.h"
#endif

#include "color.h"
#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CGraphic;
class CPlayer;
class CFile;
class CTileset;
class CUnit;
class CUnitType;
//Wyrmgus start
class CFaction;
class CUniqueItem;
//Wyrmgus end

/*----------------------------------------------------------------------------
--  Map
----------------------------------------------------------------------------*/

//Wyrmgus start
//#define MaxMapWidth  256  /// max map width supported
//#define MaxMapHeight 256  /// max map height supported
#define MaxMapWidth  512  /// max map width supported
#define MaxMapHeight 512  /// max map height supported
//Wyrmgus end

//Wyrmgus start
enum DegreeLevels {
	ExtremelyHighDegreeLevel,
	VeryHighDegreeLevel,
	HighDegreeLevel,
	MediumDegreeLevel,
	LowDegreeLevel,
	VeryLowDegreeLevel,
	
	MaxDegreeLevels
};
//Wyrmgus end

//Wyrmgus start
class CMapTemplate
{
public:
	CMapTemplate() :
		Width(0), Height(0),
		MainTemplate(NULL), BaseTerrain(NULL), SurroundingTerrain(NULL)
	{
	}

	void SetTileTerrain(const Vec2i &pos, CTerrainType *terrain);
	void ParseTerrainFile(bool overlay = false);
	CTerrainType *GetTileTerrain(const Vec2i &pos, bool overlay = false);
	
	std::string Name;
	std::string Ident;
	std::string TerrainFile;
	std::string OverlayTerrainFile;
	int Width;
	int Height;
	CMapTemplate *MainTemplate;									/// Main template in which this one is located
	CTerrainType *BaseTerrain;
	CTerrainType *SurroundingTerrain;
	std::vector<CMapTemplate *> Subtemplates;
	std::vector<std::pair<CTerrainType *, int>> GeneratedTerrains;
	std::vector<std::pair<CTerrainType *, int>> PlayerLocationGeneratedTerrains;
	std::vector<std::pair<CTerrainType *, int>> ExternalGeneratedTerrains;
	std::vector<std::pair<CUnitType *, int>> GeneratedResources; /// the first element of the pair is the resource's unit type, and the second is the quantity
	std::vector<std::pair<CUnitType *, int>> PlayerLocationGeneratedResources;
	std::vector<CTerrainType *> TileTerrains;
	std::vector<CTerrainType *> TileOverlayTerrains;
	std::map<std::pair<int, int>, std::tuple<CUnitType *, int, CUniqueItem *>> Resources; /// Resources, mapped to the tile position
	std::vector<std::tuple<Vec2i, CUnitType *, CFaction *, int, int>> Units; /// Units; first value is the tile position, and the last ones are start year and end year
	std::map<std::pair<int, int>, std::string> TileLabels; /// labels to appear for certain tiles
};
//Wyrmgus end

/*----------------------------------------------------------------------------
--  Map info structure
----------------------------------------------------------------------------*/

/**
**  Get info about a map.
*/
class CMapInfo
{
public:
	//Wyrmgus start
//	bool IsPointOnMap(int x, int y) const
	bool IsPointOnMap(int x, int y, int z = 0) const
	//Wyrmgus end
	{
		//Wyrmgus start
//		return (x >= 0 && y >= 0 && x < MapWidth && y < MapHeight);
		return (x >= 0 && y >= 0 && x < MapWidths[z] && y < MapHeights[z]);
		//Wyrmgus end
	}

	//Wyrmgus start
//	bool IsPointOnMap(const Vec2i &pos) const
	bool IsPointOnMap(const Vec2i &pos, int z = 0) const
	//Wyrmgus end
	{
		//Wyrmgus start
//		return IsPointOnMap(pos.x, pos.y);
		return IsPointOnMap(pos.x, pos.y, z);
		//Wyrmgus end
	}

	void Clear();

public:
	std::string Description;    /// Map description
	std::string Filename;       /// Map filename
	int MapWidth;               /// Map width
	int MapHeight;              /// Map height
	//Wyrmgus start
	std::vector<int> MapWidths;	/// Map width for each map layer
	std::vector<int> MapHeights; /// Map height for each map layer
	//Wyrmgus end
	int PlayerType[PlayerMax];  /// Same player->Type
	int PlayerSide[PlayerMax];  /// Same player->Side
	unsigned int MapUID;        /// Unique Map ID (hash)
};

/*----------------------------------------------------------------------------
--  Map itself
----------------------------------------------------------------------------*/

/// Describes the world map
class CMap
{
public:
	CMap();
	~CMap();

	//Wyrmgus start
//	unsigned int getIndex(int x, int y) const
	unsigned int getIndex(int x, int y, int z = 0) const
	//Wyrmgus end
	{
		//Wyrmgus start
//		return x + y * this->Info.MapWidth;
		return x + y * this->Info.MapWidths[z];
		//Wyrmgus end
	}
	//Wyrmgus start
//	unsigned int getIndex(const Vec2i &pos) const
	unsigned int getIndex(const Vec2i &pos, int z = 0) const
	//Wyrmgus end
	{
		//Wyrmgus start
//		return getIndex(pos.x, pos.y);
		return getIndex(pos.x, pos.y, z);
		//Wyrmgus end
	}

	//Wyrmgus start
//	CMapField *Field(unsigned int index) const
	CMapField *Field(unsigned int index, int z = 0) const
	//Wyrmgus end
	{
		//Wyrmgus start
//		return &this->Fields[index];
		return &this->Fields[z][index];
		//Wyrmgus end
	}
	/// Get the MapField at location x,y
	//Wyrmgus start
//	CMapField *Field(int x, int y) const
	CMapField *Field(int x, int y, int z = 0) const
	//Wyrmgus end
	{
		//Wyrmgus start
//		return &this->Fields[x + y * this->Info.MapWidth];
		return &this->Fields[z][x + y * this->Info.MapWidths[z]];
		//Wyrmgus end
	}
	//Wyrmgus start
//	CMapField *Field(const Vec2i &pos) const
	CMapField *Field(const Vec2i &pos, int z = 0) const
	//Wyrmgus end
	{
		//Wyrmgus start
//		return Field(pos.x, pos.y);
		return Field(pos.x, pos.y, z);
		//Wyrmgus end
	}

	/// Alocate and initialise map table.
	void Create();
	/// Build tables for map
	void Init();
	/// Clean the map
	void Clean();
	/// Cleanup memory for fog of war tables
	void CleanFogOfWar();
	
	//Wyrmgus start
	void SetTileTerrain(const Vec2i &pos, CTerrainType *terrain, int z);
	void RemoveTileOverlayTerrain(const Vec2i &pos, int z = 0);
	void SetOverlayTerrainDestroyed(const Vec2i &pos, bool destroyed, int z = 0);
	void SetOverlayTerrainDamaged(const Vec2i &pos, bool damaged, int z = 0);
	void CalculateTileTransitions(const Vec2i &pos, bool overlay = false);
	void CalculateTileVisibility(const Vec2i &pos);
	void AdjustTileMapIrregularities(bool overlay, const Vec2i &min_pos, const Vec2i &max_pos, int z = 0);
	void AdjustTileMapTransitions(const Vec2i &min_pos, const Vec2i &max_pos, int z = 0);
	void GenerateTerrain(CTerrainType *terrain, int seed_number, int expansion_number, const Vec2i &min_pos, const Vec2i &max_pos, bool preserve_coastline = false);
	void GenerateResources(CUnitType *unit_type, int quantity, const Vec2i &min_pos, const Vec2i &max_pos, bool grouped = false);
	//Wyrmgus end

	//Wyrmgus start
	void ClearOverlayTile(const Vec2i &pos);
	/*
	/// Remove wood from the map.
	void ClearWoodTile(const Vec2i &pos);
	/// Remove rock from the map.
	void ClearRockTile(const Vec2i &pos);
	*/
	//Wyrmgus end

	/// convert map pixelpos coordonates into tilepos
	Vec2i MapPixelPosToTilePos(const PixelPos &mapPos) const;
	/// convert tilepos coordonates into map pixel pos (take the top left of the tile)
	PixelPos TilePosToMapPixelPos_TopLeft(const Vec2i &tilePos) const;
	/// convert tilepos coordonates into map pixel pos (take the center of the tile)
	PixelPos TilePosToMapPixelPos_Center(const Vec2i &tilePos) const;
	
	//Wyrmgus start
	CTerrainType *GetTileTerrain(const Vec2i &pos, bool overlay = false) const;
	CTerrainType *GetTileTopTerrain(const Vec2i &pos, bool seen = false) const;
	Vec2i GenerateUnitLocation(const CUnitType *unit_type, CFaction *faction, const Vec2i &min_pos, const Vec2i &max_pos) const;
	//Wyrmgus end

	/// Mark a tile as seen by the player.
	//Wyrmgus start
//	void MarkSeenTile(CMapField &mf);
	void MarkSeenTile(CMapField &mf, int z = 0);
	//Wyrmgus end

	/// Regenerate the forest.
	//Wyrmgus start
//	void RegenerateForest();
	void RegenerateForest(int z = 0);
	//Wyrmgus end
	/// Reveal the complete map, make everything known.
	//Wyrmgus start
//	void Reveal();
	void Reveal(bool only_person_players = false);
	//Wyrmgus end
	/// Save the map.
	void Save(CFile &file) const;

	//
	// Wall
	//
	/// Wall is hit.
	void HitWall(const Vec2i &pos, unsigned damage);
	/// Set wall on field.
	//Wyrmgus start
	/*
	void RemoveWall(const Vec2i &pos);
	/// Set wall on field.
	void SetWall(const Vec2i &pos, bool humanwall);
	*/
	//Wyrmgus end

	/// Returns true, if wall on the map tile field
	bool WallOnMap(const Vec2i &pos) const;
	//Wyrmgus start
	/*
	/// Returns true, if human wall on the map tile field
	bool HumanWallOnMap(const Vec2i &pos) const;
	/// Returns true, if orc wall on the map tile field
	bool OrcWallOnMap(const Vec2i &pos) const;
	*/
	//Wyrmgus end
	
	//Wyrmgus start
	bool CurrentTerrainCanBeAt(const Vec2i &pos, bool overlay = false);
	bool TileBordersOnlySameTerrain(const Vec2i &pos, CTerrainType *new_terrain);
	bool TileBordersBuilding(const Vec2i &pos);
	bool TileHasUnitsIncompatibleWithTerrain(const Vec2i &pos, CTerrainType *terrain);
	bool IsPointInASubtemplateArea(const Vec2i &pos) const;
	//Wyrmgus end

	//UnitCache

	/// Insert new unit into cache
	void Insert(CUnit &unit);

	/// Remove unit from cache
	void Remove(CUnit &unit);

	//Wyrmgus start
//	void Clamp(Vec2i &pos) const;
	void Clamp(Vec2i &pos, int z = 0) const;
	//Wyrmgus end

	//Warning: we expect typical usage as xmin = x - range
	//Wyrmgus start
//	void FixSelectionArea(Vec2i &minpos, Vec2i &maxpos)
	void FixSelectionArea(Vec2i &minpos, Vec2i &maxpos, int z = 0)
	//Wyrmgus end
	{
		minpos.x = std::max<short>(0, minpos.x);
		minpos.y = std::max<short>(0, minpos.y);

		//Wyrmgus start
//		maxpos.x = std::min<short>(maxpos.x, Info.MapWidth - 1);
//		maxpos.y = std::min<short>(maxpos.y, Info.MapHeight - 1);
		maxpos.x = std::min<short>(maxpos.x, Info.MapWidths[z] - 1);
		maxpos.y = std::min<short>(maxpos.y, Info.MapHeights[z] - 1);
		//Wyrmgus end
	}

private:
	/// Build tables for fog of war
	void InitFogOfWar();

	//Wyrmgus start
	/*
	/// Correct the surrounding seen wood fields
	void FixNeighbors(unsigned short type, int seen, const Vec2i &pos);
	/// Correct the seen wood field, depending on the surrounding
	void FixTile(unsigned short type, int seen, const Vec2i &pos);
	*/
	//Wyrmgus end

	/// Regenerate the forest.
	//Wyrmgus start
//	void RegenerateForestTile(const Vec2i &pos);
	void RegenerateForestTile(const Vec2i &pos, int z = 0);
	//Wyrmgus end

public:
	//Wyrmgus start
//	CMapField *Fields;              /// fields on map
	std::vector<CMapField *> Fields;              /// fields on map
	//Wyrmgus end
	bool NoFogOfWar;           /// fog of war disabled

	CTileset *Tileset;          /// tileset data
	std::string TileModelsFileName; /// lua filename that loads all tilemodels
	CGraphic *TileGraphic;     /// graphic for all the tiles
	static CGraphic *FogGraphic;      /// graphic for fog of war
	//Wyrmgus start
	CGraphic *SolidTileGraphics[16];   /// separate graphics for solid tiles
	std::vector<std::pair<Vec2i, Vec2i>> SubtemplateAreas;
	//Wyrmgus end

	CMapInfo Info;             /// descriptive information
};


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

//Wyrmgus start
extern std::vector<CMapTemplate *>  MapTemplates;
extern std::map<std::string, CMapTemplate *> MapTemplateIdentToPointer;
//Wyrmgus end

extern CMap Map;  /// The current map
extern char CurrentMapPath[1024]; /// Path to the current map
//Wyrmgus start
extern int CurrentMapLayer;
//Wyrmgus end

/// Contrast of fog of war
extern int FogOfWarOpacity;
/// fog of war color
extern CColor FogOfWarColor;
/// Forest regeneration
extern int ForestRegeneration;
/// Flag must reveal the map
extern int FlagRevealMap;
/// Flag must reveal map when in replay
extern int ReplayRevealMap;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

//Wyrmgus start
extern CMapTemplate *GetMapTemplate(std::string map_ident);
extern std::string GetDegreeLevelNameById(int degree_level);
extern int GetDegreeLevelIdByName(std::string degree_level);
//Wyrmgus end

#define MARKER_ON_INDEX
//
// in map_fog.c
//
/// Function to (un)mark the vision table.
#ifndef MARKER_ON_INDEX
//Wyrmgus start
//typedef void MapMarkerFunc(const CPlayer &player, const Vec2i &pos);
typedef void MapMarkerFunc(const CPlayer &player, const Vec2i &pos, int z);
//Wyrmgus end
#else
//Wyrmgus start
//typedef void MapMarkerFunc(const CPlayer &player, const unsigned int index);
typedef void MapMarkerFunc(const CPlayer &player, const unsigned int index, int z);
//Wyrmgus end
#endif

/// Filter map flags through fog
extern int MapFogFilterFlags(CPlayer &player, const Vec2i &pos, int mask);
extern int MapFogFilterFlags(CPlayer &player, const unsigned int index, int mask);
/// Mark a tile for normal sight
extern MapMarkerFunc MapMarkTileSight;
/// Unmark a tile for normal sight
extern MapMarkerFunc MapUnmarkTileSight;
/// Mark a tile for cloak detection
extern MapMarkerFunc MapMarkTileDetectCloak;
/// Unmark a tile for cloak detection
extern MapMarkerFunc MapUnmarkTileDetectCloak;

/// Mark sight changes
extern void MapSight(const CPlayer &player, const Vec2i &pos, int w,
					 //Wyrmgus start
//					 int h, int range, MapMarkerFunc *marker);
					 int h, int range, MapMarkerFunc *marker, int z);
					 //Wyrmgus end
/// Update fog of war
extern void UpdateFogOfWarChange();

//
// in map_radar.c
//

/// Mark a tile as radar visible, or incrase radar vision
extern MapMarkerFunc MapMarkTileRadar;

/// Unmark a tile as radar visible, decrease is visible by other radar
extern MapMarkerFunc MapUnmarkTileRadar;

/// Mark a tile as radar jammed, or incrase radar jamming'ness
extern MapMarkerFunc MapMarkTileRadarJammer;

/// Unmark a tile as jammed, decrease is jamming'ness
extern MapMarkerFunc MapUnmarkTileRadarJammer;


//
// in map_wall.c
//
//Wyrmgus start
/*
/// Correct the seen wall field, depending on the surrounding
extern void MapFixSeenWallTile(const Vec2i &pos);
/// Correct the surrounding seen wall fields
extern void MapFixSeenWallNeighbors(const Vec2i &pos);
/// Correct the real wall field, depending on the surrounding
extern void MapFixWallTile(const Vec2i &pos);
*/
//Wyrmgus end

//
// in script_map.cpp
//
/// Set a tile
extern void SetTile(unsigned int tile, const Vec2i &pos, int value = 0);
inline void SetTile(unsigned int tile, int x, int y, int value = 0)
{
	const Vec2i pos(x, y);
	SetTile(tile, pos, value);
}

//Wyrmgus start
extern void SetTileTerrain(std::string terrain_ident, const Vec2i &pos, int value = 0, int z = 0);
inline void SetTileTerrain(std::string terrain_ident, int x, int y, int value = 0, int z = 0)
{
	const Vec2i pos(x, y);
	SetTileTerrain(terrain_ident, pos, value, z);
}
extern void SetMapTemplateTileTerrain(std::string map_ident, std::string terrain_ident, int x, int y, std::string tile_label = "");
extern void SetMapTemplateTileTerrainByID(std::string map_ident, int terrain_id, int x, int y, std::string tile_label = "");
extern void ApplyMapTemplate(std::string map_template_ident, int start_x = 0, int start_y = 0, int map_start_x = 0, int map_start_y = 0, int z = 0);
//Wyrmgus end

/// register ccl features
extern void MapCclRegister();

//
// mixed sources
//
/// Save a stratagus map (smp format)
//Wyrmgus start
//extern int SaveStratagusMap(const std::string &filename, CMap &map, int writeTerrain);
extern int SaveStratagusMap(const std::string &filename, CMap &map, int writeTerrain, bool is_mod = false);
//Wyrmgus end


/// Load map presentation
extern void LoadStratagusMapInfo(const std::string &mapname);

/// Returns true, if the unit-type(mask can enter field with bounds check
extern bool CheckedCanMoveToMask(const Vec2i &pos, int mask);
/// Returns true, if the unit-type can enter the field
//Wyrmgus start
//extern bool UnitTypeCanBeAt(const CUnitType &type, const Vec2i &pos);
extern bool UnitTypeCanBeAt(const CUnitType &type, const Vec2i &pos, int z = 0);
//Wyrmgus end
/// Returns true, if the unit can enter the field
extern bool UnitCanBeAt(const CUnit &unit, const Vec2i &pos);

/// Preprocess map, for internal use.
extern void PreprocessMap();

// in unit.c

/// Mark on vision table the Sight of the unit.
void MapMarkUnitSight(CUnit &unit);
/// Unmark on vision table the Sight of the unit.
void MapUnmarkUnitSight(CUnit &unit);

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/// Can a unit with 'mask' enter the field
inline bool CanMoveToMask(const Vec2i &pos, int mask)
{
	return !Map.Field(pos)->CheckMask(mask);
}

/// Handle Marking and Unmarking of radar vision
//Wyrmgus start
//inline void MapMarkRadar(const CPlayer &player, const Vec2i &pos, int w, int h, int range)
inline void MapMarkRadar(const CPlayer &player, const Vec2i &pos, int w, int h, int range, int z)
//Wyrmgus end
{
	//Wyrmgus start
//	MapSight(player, pos, w, h, range, MapMarkTileRadar);
	MapSight(player, pos, w, h, range, MapMarkTileRadar, z);
	//Wyrmgus end
}
//Wyrmgus start
//inline void MapUnmarkRadar(const CPlayer &player, const Vec2i &pos, int w, int h, int range)
inline void MapUnmarkRadar(const CPlayer &player, const Vec2i &pos, int w, int h, int range, int z)
//Wyrmgus end
{
	//Wyrmgus start
//	MapSight(player, pos, w, h, range, MapUnmarkTileRadar);
	MapSight(player, pos, w, h, range, MapUnmarkTileRadar, z);
	//Wyrmgus end
}
/// Handle Marking and Unmarking of radar vision
//Wyrmgus start
//inline void MapMarkRadarJammer(const CPlayer &player, const Vec2i &pos, int w, int h, int range)
inline void MapMarkRadarJammer(const CPlayer &player, const Vec2i &pos, int w, int h, int range, int z)
//Wyrmgus end
{
	//Wyrmgus start
//	MapSight(player, pos, w, h, range, MapMarkTileRadarJammer);
	MapSight(player, pos, w, h, range, MapMarkTileRadarJammer, z);
	//Wyrmgus end
}
//Wyrmgus start
//inline void MapUnmarkRadarJammer(const CPlayer &player, const Vec2i &pos, int w, int h, int range)
inline void MapUnmarkRadarJammer(const CPlayer &player, const Vec2i &pos, int w, int h, int range, int z)
//Wyrmgus end
{
	//Wyrmgus start
//	MapSight(player, pos, w, h, range, MapUnmarkTileRadarJammer);
	MapSight(player, pos, w, h, range, MapUnmarkTileRadarJammer, z);
	//Wyrmgus end
}

//@}

#endif // !__MAP_H__
