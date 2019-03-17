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
/**@name music.cpp - Background music support */
//
//      (c) Copyright 2002-2006 by Lutz Sammer, Nehal Mistry, and Jimmy Salmon
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
-- Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "sound/sound_server.h"

#include "iolib.h"
#include "wyrmgus.h"

#include <oamlGodotModule/oamlGodotModule.h>

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

void InitMusicOAML()
{
	const std::string filename = LibraryFileName("oaml.defs");
	Wyrmgus::GetInstance()->GetOamlModule()->Init(filename.c_str());

	SetMusicVolume(GetMusicVolume());
}

void LoadOAMLDefinitionsFile(const std::string &file_path)
{
	const std::string filename = LibraryFileName(file_path.c_str());
	Wyrmgus::GetInstance()->GetOamlModule()->ReadDefsFile(filename.c_str());
}
