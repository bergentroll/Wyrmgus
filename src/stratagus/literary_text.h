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
//      (c) Copyright 2016-2021 by Andrettin
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

class chapter
{
public:
	std::string Name;				/// Name of the chapter
	int ID = 0;
	bool Introduction = false;		/// Whether this is an introductory chapter
	std::vector<std::string> Pages;	/// Pages of text
};

class literary_text final : public named_data_entry, public data_type<literary_text>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "literary_text";
	static constexpr const char *database_folder = "literary_texts";

	explicit literary_text(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	chapter *GetChapter(const std::string &chapter_name) const;
	
	std::string Author;				/// Author of the text
	std::string Translator;			/// Translator of the text
	std::string Publisher;			/// Publisher of the text
	std::string CopyrightNotice;	/// Copyright notice explaining that this text is in the public domain, or is licensed under an open-source license
	std::string Notes;				/// Notes to appear on the cover of the text
	int Year = 0;						/// Year of publication
	int InitialPage = 1;				/// Page in which the text begins
	std::vector<std::unique_ptr<chapter>> Chapters;	/// The chapters of the text
};

}

extern void LiteraryTextCclRegister();
