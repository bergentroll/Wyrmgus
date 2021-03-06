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
//      (c) Copyright 1998-2021 by Lutz Sammer, Nehal Mistry,
//                                 Jimmy Salmon and Andrettin
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

#include "ui/cursor.h"
#include "video/intern_video.h"

#include "civilization.h"
#include "database/defines.h"
#include "editor.h"
#include "map/map.h"
#include "map/map_layer.h"
#include "map/tile.h"
#include "map/tile_flag.h"
#include "translate.h"
#include "ui/cursor_type.h"
#include "ui/interface.h"
#include "ui/ui.h"
#include "unit/unit.h"
#include "unit/unit_type.h"
#include "unit/unit_type_variation.h"
//Wyrmgus start
#include "upgrade/upgrade.h"
//Wyrmgus end
#include "video/video.h"

#include <QPixmap>

CursorState CurrentCursorState;    /// current cursor state (point,...)
ButtonCmd CursorAction;            /// action for selection
int CursorValue;             /// value for CursorAction (spell type f.e.)
std::string CustomCursor;    /// custom cursor for button

// Event changed mouse position, can alter at any moment
PixelPos CursorScreenPos;    /// cursor position on screen
PixelPos CursorStartScreenPos;  /// rectangle started on screen
PixelPos CursorStartMapPos;/// position of starting point of selection rectangle, in Map pixels.


/*--- DRAW BUILDING  CURSOR ------------------------------------------------*/
const wyrmgus::unit_type *CursorBuilding;           /// building cursor

namespace wyrmgus {

void cursor::clear()
{
	data_type::clear();

	CursorBuilding = nullptr;
	cursor::current_cursor = nullptr;
	UnitUnderCursor = nullptr;
}

void cursor::set_current_cursor(cursor *cursor)
{
	if (cursor == cursor::current_cursor) {
		return;
	}

	cursor::current_cursor = cursor;

	if (cursor != nullptr) {
		if (!cursor->get_graphics()->IsLoaded()) {
			cursor->get_graphics()->Load(false, defines::get()->get_scale_factor());
		}

		const QPixmap pixmap = QPixmap::fromImage(cursor->get_graphics()->get_scaled_image());
		const QPoint hot_pos = cursor->get_hot_pos() * defines::get()->get_scale_factor();
		const QCursor qcursor(pixmap, hot_pos.x(), hot_pos.y());

		QMetaObject::invokeMethod(QApplication::instance(), [qcursor] {
			if (QApplication::overrideCursor() != nullptr) {
				QApplication::changeOverrideCursor(qcursor);
			} else {
				QApplication::setOverrideCursor(qcursor);
			}
		}, Qt::QueuedConnection);
	}
}

cursor::cursor(const std::string &identifier) : data_entry(identifier), type(cursor_type::point)
{
}


cursor::~cursor()
{
}

void cursor::initialize()
{
	this->graphics = CGraphic::New(this->get_file().string(), this->get_frame_size());

	if (this->civilization != nullptr) {
		this->civilization->set_cursor(this->get_type(), this);
	} else {
		cursor::map_cursor(this->get_type(), this);
	}

	data_entry::initialize();
}

void cursor::set_file(const std::filesystem::path &filepath)
{
	if (filepath == this->get_file()) {
		return;
	}

	this->file = database::get()->get_graphics_path(this->get_module()) / filepath;
}

}

/**
**  Get the amount of cursors sprites to load
*/
int GetCursorsCount()
{
	int count = 0;
	for (const cursor *cursor : cursor::get_all()) {
		if (cursor->get_graphics() != nullptr && !cursor->get_graphics()->IsLoaded()) {
			count++;
		}
	}

	return count;
}

/**
**  Draw rectangle cursor when visible
**
**  @param corner1   Screen start position of rectangle
**  @param corner2   Screen end position of rectangle
*/
static void DrawVisibleRectangleCursor(PixelPos corner1, PixelPos corner2)
{
	const CViewport &vp = *UI.SelectedViewport;

	//  Clip to map window.
	//  FIXME: should re-use CLIP_RECTANGLE in some way from linedraw.c ?
	vp.Restrict(corner2.x, corner2.y);

	if (corner1.x > corner2.x) {
		std::swap(corner1.x, corner2.x);
	}
	if (corner1.y > corner2.y) {
		std::swap(corner1.y, corner2.y);
	}
	const int w = corner2.x - corner1.x + 1;
	const int h = corner2.y - corner1.y + 1;

	Video.DrawRectangleClip(ColorGreen, corner1.x, corner1.y, w, h);
}

/**
**  Draw cursor for selecting building position.
*/
//Wyrmgus start
//static void DrawBuildingCursor()
void DrawBuildingCursor()
//Wyrmgus end
{
	// Align to grid
	const CViewport &vp = *UI.MouseViewport;
	const Vec2i mpos = vp.ScreenToTilePos(CursorScreenPos);
	const PixelPos screenPos = vp.TilePosToScreen_TopLeft(mpos);
	const int z = UI.CurrentMapLayer->ID;

	CUnit *ontop = nullptr;

	//
	//  Draw building
	//
#ifdef DYNAMIC_LOAD
	if (!CursorBuilding->G->IsLoaded()) {
		LoadUnitTypeSprite(CursorBuilding);
	}
#endif
	PushClipping();
	vp.SetClipping();

	const QPoint center_tile_pos = mpos + CursorBuilding->get_tile_center_pos_offset();
	const wyrmgus::time_of_day *time_of_day = UI.CurrentMapLayer->get_tile_time_of_day(center_tile_pos);

//	DrawShadow(*CursorBuilding, CursorBuilding->StillFrame, screenPos);
	if (CursorBuilding->GetDefaultVariation(CPlayer::GetThisPlayer()) && CursorBuilding->GetDefaultVariation(CPlayer::GetThisPlayer())->ShadowSprite) {
		DrawShadow(*CursorBuilding, CursorBuilding->GetDefaultVariation(CPlayer::GetThisPlayer())->ShadowSprite, CursorBuilding->StillFrame, screenPos);
	} else if (CursorBuilding->ShadowSprite) {
		DrawShadow(*CursorBuilding, CursorBuilding->ShadowSprite, CursorBuilding->StillFrame, screenPos);
	}
	//Wyrmgus end
	//Wyrmgus start
	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), BackpackImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), MountImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

//	DrawUnitType(*CursorBuilding, CursorBuilding->Sprite, CPlayer::GetThisPlayer()->Index,
//				 CursorBuilding->StillFrame, screenPos);
	// get the first variation which has the proper upgrades for this player (to have the proper appearance of buildings drawn in the cursor, according to the upgrades)
	if (CursorBuilding->GetDefaultVariation(CPlayer::GetThisPlayer()) && CursorBuilding->GetDefaultVariation(CPlayer::GetThisPlayer())->Sprite) {
		DrawUnitType(*CursorBuilding, CursorBuilding->GetDefaultVariation(CPlayer::GetThisPlayer())->Sprite, CPlayer::GetThisPlayer()->Index,
				CursorBuilding->StillFrame, screenPos, time_of_day);
	} else {
		DrawUnitType(*CursorBuilding, CursorBuilding->Sprite, CPlayer::GetThisPlayer()->Index,
				CursorBuilding->StillFrame, screenPos, time_of_day);
	}
	//Wyrmgus end
	
	//Wyrmgus start
	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), HairImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), PantsImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), ClothingImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), HelmetImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), BootsImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), LeftArmImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), ClothingLeftArmImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

	DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), ShieldImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);
	
	if (CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), RightHandImageLayer) != nullptr) {
		DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), RightArmImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

		DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), ClothingRightArmImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

		DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), WeaponImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);
		
		DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), RightHandImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);
	} else {
		DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), WeaponImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

		DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), RightArmImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);

		DrawPlayerColorOverlay(*CursorBuilding, CursorBuilding->GetDefaultLayerSprite(CPlayer::GetThisPlayer(), ClothingRightArmImageLayer), CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);
	}

	if (CursorBuilding->GetDefaultVariation(CPlayer::GetThisPlayer()) && CursorBuilding->GetDefaultVariation(CPlayer::GetThisPlayer())->LightSprite) {
		DrawOverlay(*CursorBuilding, CursorBuilding->GetDefaultVariation(CPlayer::GetThisPlayer())->LightSprite, CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);
	} else if (CursorBuilding->LightSprite) {
		DrawOverlay(*CursorBuilding, CursorBuilding->LightSprite, CPlayer::GetThisPlayer()->Index, CursorBuilding->StillFrame, screenPos, time_of_day);
	}
	//Wyrmgus end
	
	if (CursorBuilding->CanAttack && CursorBuilding->Stats->Variables[ATTACKRANGE_INDEX].Value > 0) {
		const PixelPos center(screenPos + CursorBuilding->get_scaled_half_tile_pixel_size());
		const int radius = (CursorBuilding->Stats->Variables[ATTACKRANGE_INDEX].Max + (CursorBuilding->get_tile_width() - 1)) * wyrmgus::defines::get()->get_scaled_tile_width() + 1;
		Video.DrawCircleClip(ColorRed, center.x, center.y, radius);
	}

	//
	//  Draw the allow overlay
	//
	int f;
	if (!Selected.empty()) {
		f = 1;
		for (size_t i = 0; f && i < Selected.size(); ++i) {
			f = ((ontop = CanBuildHere(Selected[i], *CursorBuilding, mpos, z)) != nullptr);
			// Assign ontop or null
			ontop = (ontop == Selected[i] ? nullptr : ontop);
		}
	} else {
		f = ((ontop = CanBuildHere(NoUnitP, *CursorBuilding, mpos, z)) != nullptr);
		if (!Editor.Running || ontop == (CUnit *)1) {
			ontop = nullptr;
		}
	}

	const tile_flag mask = CursorBuilding->MovementMask;
	int h = CursorBuilding->get_tile_height();
	// reduce to view limits
	h = std::min(h, vp.MapPos.y + vp.MapHeight - mpos.y);
	int w0 = CursorBuilding->get_tile_width();
	w0 = std::min(w0, vp.MapPos.x + vp.MapWidth - mpos.x);

	while (h--) {
		int w = w0;
		while (w--) {
			const Vec2i posIt(mpos.x + w, mpos.y + h);
			uint32_t color;

			if (f && (ontop ||
					  CanBuildOn(posIt, MapFogFilterFlags(*CPlayer::GetThisPlayer(), posIt,
														  mask & ((!Selected.empty() && Selected[0]->tilePos == posIt) ?
																  ~(tile_flag::land_unit | tile_flag::sea_unit) : static_cast<tile_flag>(-1)), z), z, CPlayer::GetThisPlayer(), CursorBuilding))
				&& UI.CurrentMapLayer->Field(posIt)->player_info->IsTeamExplored(*CPlayer::GetThisPlayer())) {
				color = ColorGreen;
			} else {
				color = ColorRed;
			}
			Video.FillTransRectangleClip(color, screenPos.x + w * wyrmgus::defines::get()->get_scaled_tile_width(),
										 screenPos.y + h * wyrmgus::defines::get()->get_scaled_tile_height(), wyrmgus::defines::get()->get_scaled_tile_width(), wyrmgus::defines::get()->get_scaled_tile_height(), 95);
		}
	}
	PopClipping();
}

/**
**  Draw the cursor.
*/
void DrawCursor()
{
	// Selecting rectangle
	if (CurrentCursorState == CursorState::Rectangle && CursorStartScreenPos != CursorScreenPos) {
		const PixelPos cursorStartScreenPos = UI.MouseViewport->scaled_map_to_screen_pixel_pos(CursorStartMapPos);

		DrawVisibleRectangleCursor(cursorStartScreenPos, CursorScreenPos);
	}

	//  Cursor may not exist if we are loading a game or something.
	//  Only draw it if it exists
	if (cursor::get_current_cursor() == nullptr) {
		return;
	}

	const PixelPos pos = CursorScreenPos - cursor::get_current_cursor()->get_hot_pos() * defines::get()->get_scale_factor();

	//  Last, Normal cursor.
	cursor::get_current_cursor()->get_graphics()->DrawFrameClip(cursor::get_current_cursor()->get_current_frame(), pos.x, pos.y);
}

/**
**  Animate the cursor.
**
**  @param ticks  Current tick
*/
void CursorAnimate(unsigned ticks)
{
	static unsigned last = 0;

	if (cursor::get_current_cursor() == nullptr || !cursor::get_current_cursor()->get_frame_rate()) {
		return;
	}

	if (ticks > last + cursor::get_current_cursor()->get_frame_rate()) {
		last = ticks + cursor::get_current_cursor()->get_frame_rate();
		cursor::get_current_cursor()->increment_current_frame();
		if ((cursor::get_current_cursor()->get_frame_rate() & 127) >= cursor::get_current_cursor()->get_graphics()->NumFrames) {
			cursor::get_current_cursor()->reset_current_frame();
		}
	}
}
