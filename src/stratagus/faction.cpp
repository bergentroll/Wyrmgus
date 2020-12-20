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
//      (c) Copyright 2019-2020 by Andrettin
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

#include "stratagus.h"

#include "faction.h"

#include "ai/ai_force_template.h"
#include "ai/ai_force_type.h"
#include "character.h"
#include "civilization.h"
#include "diplomacy_state.h"
#include "faction_history.h"
#include "faction_tier.h"
#include "faction_type.h"
#include "government_type.h"
#include "luacallback.h"
#include "player_color.h"
#include "script/condition/and_condition.h"
#include "unit/unit_type.h"
#include "util/container_util.h"
#include "util/string_util.h"
#include "util/vector_util.h"

namespace wyrmgus {

void faction::process_title_names(title_name_map &title_names, const sml_data &scope)
{
	scope.for_each_child([&](const sml_data &child_scope) {
		faction::process_title_name_scope(title_names, child_scope);
	});

	scope.for_each_property([&](const sml_property &property) {
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();
		wyrmgus::government_type government_type = string_to_government_type(key);
		title_names[government_type][faction_tier::none] = value;
	});
}

void faction::process_title_name_scope(title_name_map &title_names, const sml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const wyrmgus::government_type government_type = string_to_government_type(tag);

	scope.for_each_property([&](const sml_property &property) {
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();
		const faction_tier tier = string_to_faction_tier(key);
		title_names[government_type][tier] = value;
	});
}

void faction::process_character_title_name_scope(character_title_name_map &character_title_names, const sml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const character_title title_type = GetCharacterTitleIdByName(tag);

	scope.for_each_child([&](const sml_data &child_scope) {
		faction::process_character_title_name_scope(character_title_names[title_type], child_scope);
	});

	scope.for_each_property([&](const sml_property &property) {
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();
		const wyrmgus::government_type government_type = string_to_government_type(key);
		character_title_names[title_type][government_type][faction_tier::none][gender::none] = value;
	});
}

void faction::process_character_title_name_scope(std::map<wyrmgus::government_type, std::map<faction_tier, std::map<gender, std::string>>> &character_title_names, const sml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const wyrmgus::government_type government_type = string_to_government_type(tag);

	scope.for_each_child([&](const sml_data &child_scope) {
		faction::process_character_title_name_scope(character_title_names[government_type], child_scope);
	});
}

void faction::process_character_title_name_scope(std::map<faction_tier, std::map<gender, std::string>> &character_title_names, const sml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const faction_tier faction_tier = string_to_faction_tier(tag);

	scope.for_each_property([&](const sml_property &property) {
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();
		const gender gender = string_to_gender(key);
		character_title_names[faction_tier][gender] = value;
	});
}

faction::faction(const std::string &identifier)
	: detailed_data_entry(identifier), type(faction_type::none), default_tier(faction_tier::barony), min_tier(faction_tier::none), max_tier(faction_tier::none), default_government_type(government_type::monarchy)
{
}

faction::~faction()
{
}

void faction::process_sml_property(const sml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "adjective") {
		this->Adjective = value;
	} else if (key == "faction_upgrade") {
		this->FactionUpgrade = value;
	} else {
		data_entry::process_sml_property(property);
	}
}

void faction::process_sml_scope(const sml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "develops_from") {
		for (const std::string &value : values) {
			faction *other_faction = faction::get(value);
			this->DevelopsFrom.push_back(other_faction);
			other_faction->DevelopsTo.push_back(this);
		}
	} else if (tag == "title_names") {
		faction::process_title_names(this->title_names, scope);
	} else if (tag == "character_title_names") {
		scope.for_each_child([&](const sml_data &child_scope) {
			faction::process_character_title_name_scope(this->character_title_names, child_scope);
		});
	} else if (tag == "preconditions") {
		auto preconditions = std::make_unique<and_condition>();
		database::process_sml_data(preconditions, scope);
		this->preconditions = std::move(preconditions);
	} else if (tag == "conditions") {
		auto conditions = std::make_unique<and_condition>();
		database::process_sml_data(conditions, scope);
		this->conditions = std::move(conditions);
	} else {
		data_entry::process_sml_scope(scope);
	}
}

void faction::initialize()
{
	if (this->get_type() == faction_type::tribe) {
		this->definite_article = true;
	}

	std::sort(this->AiBuildingTemplates.begin(), this->AiBuildingTemplates.end(), [](const std::unique_ptr<CAiBuildingTemplate> &a, const std::unique_ptr<CAiBuildingTemplate> &b) {
		return a->get_priority() > b->get_priority();
	});

	for (auto &kv_pair : this->ai_force_templates) {
		std::sort(kv_pair.second.begin(), kv_pair.second.end(), [](const std::unique_ptr<ai_force_template> &a, const std::unique_ptr<ai_force_template> &b) {
			return a->get_priority() > b->get_priority();
		});
	}

	if (this->get_parent_faction() != nullptr) {
		const faction *parent_faction = this->get_parent_faction();

		if (this->FactionUpgrade.empty()) { //if the faction has no faction upgrade, inherit that of its parent faction
			this->FactionUpgrade = parent_faction->FactionUpgrade;
		}

		//inherit button icons from parent civilization, for button actions which none are specified
		for (std::map<ButtonCmd, IconConfig>::const_iterator iterator = parent_faction->ButtonIcons.begin(); iterator != parent_faction->ButtonIcons.end(); ++iterator) {
			if (this->ButtonIcons.find(iterator->first) == this->ButtonIcons.end()) {
				this->ButtonIcons[iterator->first] = iterator->second;
			}
		}

		for (std::map<std::string, std::map<CDate, bool>>::const_iterator iterator = parent_faction->HistoricalUpgrades.begin(); iterator != parent_faction->HistoricalUpgrades.end(); ++iterator) {
			if (this->HistoricalUpgrades.find(iterator->first) == this->HistoricalUpgrades.end()) {
				this->HistoricalUpgrades[iterator->first] = iterator->second;
			}
		}
	}

	if (this->get_min_tier() == faction_tier::none) {
		this->min_tier = this->get_default_tier();
	}

	if (this->get_max_tier() == faction_tier::none) {
		this->max_tier = this->get_default_tier();
	}

	data_entry::initialize();
}

void faction::check() const
{
	if (this->civilization == nullptr) {
		throw std::runtime_error("Faction \"" + this->get_identifier() + "\" has no civilization.");
	}

	if (this->get_type() == faction_type::none) {
		throw std::runtime_error("Faction \"" + this->get_identifier() + "\" has no type.");
	}

	for (const auto &kv_pair : this->ai_force_templates) {
		for (const std::unique_ptr<ai_force_template> &force_template : kv_pair.second) {
			force_template->check();
		}
	}

	if (this->get_preconditions() != nullptr) {
		this->get_preconditions()->check_validity();
	}

	if (this->get_conditions() != nullptr) {
		this->get_conditions()->check_validity();
	}
}

data_entry_history *faction::get_history_base()
{
	return this->history.get();
}

void faction::reset_history()
{
	this->history = std::make_unique<faction_history>(this->get_default_tier(), this->get_default_government_type(), this->get_default_capital());
}

std::string_view faction::get_title_name(const wyrmgus::government_type government_type, const faction_tier tier) const
{
	if (this->get_type() != faction_type::polity) {
		return string::empty_str;
	}

	auto find_iterator = this->title_names.find(government_type);
	if (find_iterator != this->title_names.end()) {
		auto sub_find_iterator = find_iterator->second.find(tier);
		if (sub_find_iterator != find_iterator->second.end()) {
			return sub_find_iterator->second;
		}
	}

	return this->get_civilization()->get_title_name(government_type, tier);
}

std::string_view faction::get_character_title_name(const character_title title_type, const wyrmgus::government_type government_type, const faction_tier tier, const gender gender) const
{
	auto find_iterator = this->character_title_names.find(title_type);
	if (find_iterator != this->character_title_names.end()) {
		auto secondary_find_iterator = find_iterator->second.find(government_type);
		if (secondary_find_iterator == find_iterator->second.end()) {
			secondary_find_iterator = find_iterator->second.find(government_type::none);
		}

		if (secondary_find_iterator != find_iterator->second.end()) {
			auto tertiary_find_iterator = secondary_find_iterator->second.find(tier);
			if (tertiary_find_iterator == secondary_find_iterator->second.end()) {
				tertiary_find_iterator = secondary_find_iterator->second.find(faction_tier::none);
			}

			if (tertiary_find_iterator != secondary_find_iterator->second.end()) {
				auto quaternary_find_iterator = tertiary_find_iterator->second.find(gender);
				if (quaternary_find_iterator == tertiary_find_iterator->second.end()) {
					quaternary_find_iterator = tertiary_find_iterator->second.find(gender::none);
				}

				if (quaternary_find_iterator != tertiary_find_iterator->second.end()) {
					return quaternary_find_iterator->second;
				}
			}
		}
	}

	return this->get_civilization()->get_character_title_name(title_type, this->get_type(), government_type, tier, gender);
}

int faction::GetUpgradePriority(const CUpgrade *upgrade) const
{
	if (!upgrade) {
		fprintf(stderr, "Error in faction::GetUpgradePriority: the upgrade is null.\n");
	}
	
	if (this->UpgradePriorities.find(upgrade) != this->UpgradePriorities.end()) {
		return this->UpgradePriorities.find(upgrade)->second;
	}
	
	return this->civilization->GetUpgradePriority(upgrade);
}

int faction::get_force_type_weight(const ai_force_type force_type) const
{
	if (force_type == ai_force_type::none) {
		throw std::runtime_error("Error in faction::get_force_type_weight: the force_type is none.");
	}
	
	const auto find_iterator = this->ai_force_type_weights.find(force_type);
	if (find_iterator != this->ai_force_type_weights.end()) {
		return find_iterator->second;
	}
	
	if (this->get_parent_faction() != nullptr) {
		return this->get_parent_faction()->get_force_type_weight(force_type);
	}
	
	return this->civilization->get_force_type_weight(force_type);
}

CCurrency *faction::GetCurrency() const
{
	if (this->Currency != nullptr) {
		return this->Currency;
	}
	
	if (this->get_parent_faction() != nullptr) {
		return this->get_parent_faction()->GetCurrency();
	}
	
	return this->civilization->GetCurrency();
}

bool faction::uses_simple_name() const
{
	return this->simple_name || this->get_type() != faction_type::polity;
}

const std::vector<std::unique_ptr<ai_force_template>> &faction::get_ai_force_templates(const ai_force_type force_type) const
{
	if (force_type == ai_force_type::none) {
		throw std::runtime_error("Error in faction::get_ai_force_templates: the force_type is none.");
	}
	
	const auto find_iterator = this->ai_force_templates.find(force_type);
	if (find_iterator != this->ai_force_templates.end()) {
		return find_iterator->second;
	}
	
	if (this->get_parent_faction() != nullptr) {
		return this->get_parent_faction()->get_ai_force_templates(force_type);
	}
	
	return this->civilization->get_ai_force_templates(force_type);
}

const std::vector<std::unique_ptr<CAiBuildingTemplate>> &faction::GetAiBuildingTemplates() const
{
	if (this->AiBuildingTemplates.size() > 0) {
		return this->AiBuildingTemplates;
	}
	
	if (this->get_parent_faction() != nullptr) {
		return this->get_parent_faction()->GetAiBuildingTemplates();
	}
	
	return this->civilization->GetAiBuildingTemplates();
}

const std::vector<std::string> &faction::get_ship_names() const
{
	if (this->ship_names.size() > 0) {
		return this->ship_names;
	}
	
	if (this->get_parent_faction() != nullptr) {
		return this->get_parent_faction()->get_ship_names();
	}
	
	return this->civilization->get_ship_names();
}

QStringList faction::get_ship_names_qstring_list() const
{
	return container::to_qstring_list(this->get_ship_names());
}

void faction::remove_ship_name(const std::string &ship_name)
{
	vector::remove_one(this->ship_names, ship_name);
}

unit_type *faction::get_class_unit_type(const unit_class *unit_class) const
{
	if (unit_class == nullptr) {
		return nullptr;
	}

	auto find_iterator = this->class_unit_types.find(unit_class);
	if (find_iterator != this->class_unit_types.end()) {
		return find_iterator->second;
	}

	if (this->get_parent_faction() != nullptr) {
		return this->get_parent_faction()->get_class_unit_type(unit_class);
	}

	return this->civilization->get_class_unit_type(unit_class);
}

bool faction::is_class_unit_type(const unit_type *unit_type) const
{
	return unit_type == this->get_class_unit_type(unit_type->get_unit_class());
}

CUpgrade *faction::get_class_upgrade(const upgrade_class *upgrade_class) const
{
	if (upgrade_class == nullptr) {
		return nullptr;
	}

	auto find_iterator = this->class_upgrades.find(upgrade_class);
	if (find_iterator != this->class_upgrades.end()) {
		return find_iterator->second;
	}

	if (this->get_parent_faction() != nullptr) {
		return this->get_parent_faction()->get_class_upgrade(upgrade_class);
	}

	return this->get_civilization()->get_class_upgrade(upgrade_class);
}

const std::vector<CFiller> &faction::get_ui_fillers() const
{
	if (!this->ui_fillers.empty()) {
		return this->ui_fillers;
	}

	if (this->get_parent_faction() != nullptr) {
		return this->get_parent_faction()->get_ui_fillers();
	}

	return this->get_civilization()->get_ui_fillers();
}

void faction::remove_dynasty(const wyrmgus::dynasty *dynasty)
{
	vector::remove(this->dynasties, dynasty);
}

}
