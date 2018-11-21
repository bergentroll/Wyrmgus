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
/**@name calendar.cpp - The calendar source file. */
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "calendar.h"

#include "config.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

std::vector<CCalendar *> CCalendar::Calendars;
std::map<std::string, CCalendar *> CCalendar::CalendarsByIdent;
CCalendar * CCalendar::BaseCalendar = NULL;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

void CDayOfTheWeek::ProcessConfigData(CConfigData *config_data)
{
	for (size_t i = 0; i < config_data->Properties.size(); ++i) {
		std::string key = config_data->Properties[i].first;
		std::string value = config_data->Properties[i].second;
		
		if (key == "name") {
			this->Name = value;
		} else {
			fprintf(stderr, "Invalid day of the week property: \"%s\".\n", key.c_str());
		}
	}
}

void CMonth::ProcessConfigData(CConfigData *config_data)
{
	for (size_t i = 0; i < config_data->Properties.size(); ++i) {
		std::string key = config_data->Properties[i].first;
		std::string value = config_data->Properties[i].second;
		
		if (key == "name") {
			this->Name = value;
		} else if (key == "days") {
			this->Days = std::stoi(value);
		} else {
			fprintf(stderr, "Invalid month property: \"%s\".\n", key.c_str());
		}
	}
}

CCalendar::~CCalendar()
{
	for (size_t i = 0; i < DaysOfTheWeek.size(); ++i) {
		delete DaysOfTheWeek[i];
	}
	DaysOfTheWeek.clear();
	
	for (size_t i = 0; i < Months.size(); ++i) {
		delete Months[i];
	}
	Months.clear();
}

/**
**  Get a calendar
*/
CCalendar *CCalendar::GetCalendar(const std::string &ident)
{
	if (CalendarsByIdent.find(ident) != CalendarsByIdent.end()) {
		return CalendarsByIdent.find(ident)->second;
	}
	
	return NULL;
}

CCalendar *CCalendar::GetOrAddCalendar(const std::string &ident)
{
	CCalendar *calendar = GetCalendar(ident);
	
	if (!calendar) {
		calendar = new CCalendar;
		calendar->Ident = ident;
		Calendars.push_back(calendar);
		CalendarsByIdent[ident] = calendar;
	}
	
	return calendar;
}

void CCalendar::ClearCalendars()
{
	for (size_t i = 0; i < Calendars.size(); ++i) {
		delete Calendars[i];
	}
	Calendars.clear();
	
	BaseCalendar = NULL;
}

int CCalendar::GetTimeOfDay(const unsigned long long hours, const int hours_per_day)
{
	int standardized_hour = hours % hours_per_day;
	standardized_hour *= DefaultHoursPerDay;
	standardized_hour /= hours_per_day;
	
	if (standardized_hour >= 5 && standardized_hour <= 7) {
		return DawnTimeOfDay;
	} else if (standardized_hour >= 8 && standardized_hour <= 10) {
		return MorningTimeOfDay;
	} else if (standardized_hour >= 11 && standardized_hour <= 13) {
		return MiddayTimeOfDay;
	} else if (standardized_hour >= 14 && standardized_hour <= 16) {
		return AfternoonTimeOfDay;
	} else if (standardized_hour >= 17 && standardized_hour <= 19) {
		return DuskTimeOfDay;
	} else if (standardized_hour >= 20 && standardized_hour <= 22) {
		return FirstWatchTimeOfDay;
	} else if (standardized_hour >= 23 || (standardized_hour >= 0 && standardized_hour <= 1)) {
		return MidnightTimeOfDay;
	} else if (standardized_hour >= 2 && standardized_hour <= 4) {
		return SecondWatchTimeOfDay;
	}
	
	return NoTimeOfDay;
}

void CCalendar::ProcessConfigData(CConfigData *config_data)
{
	std::string base_day_of_the_week;
	
	for (size_t i = 0; i < config_data->Properties.size(); ++i) {
		std::string key = config_data->Properties[i].first;
		std::string value = config_data->Properties[i].second;
		
		if (key == "name") {
			this->Name = value;
		} else if (key == "base_calendar") {
			bool is_base_calendar = StringToBool(value);
			if (is_base_calendar) {
				CCalendar::BaseCalendar = this;
			}
		} else if (key == "year_label") {
			this->YearLabel = value;
		} else if (key == "negative_year_label") {
			this->NegativeYearLabel = value;
		} else if (key == "base_day_of_the_week") {
			value = FindAndReplaceString(value, "_", "-");
			base_day_of_the_week = value;
		} else {
			fprintf(stderr, "Invalid calendar property: \"%s\".\n", key.c_str());
		}
	}
	
	for (size_t i = 0; i < config_data->Children.size(); ++i) {
		CConfigData *child_config_data = config_data->Children[i];
		
		if (child_config_data->Tag == "day_of_the_week") {
			CDayOfTheWeek *day_of_the_week = new CDayOfTheWeek;
			day_of_the_week->ID = this->DaysOfTheWeek.size();
			this->DaysOfTheWeek.push_back(day_of_the_week);
			day_of_the_week->Calendar = this;
			day_of_the_week->Ident = child_config_data->Ident;
			this->DaysOfTheWeekByIdent[day_of_the_week->Ident] = day_of_the_week;
			day_of_the_week->ProcessConfigData(child_config_data);
		} else if (child_config_data->Tag == "month") {
			CMonth *month = new CMonth;
			month->ProcessConfigData(child_config_data);
			this->Months.push_back(month);
			this->DaysPerYear += month->Days;
		} else if (child_config_data->Tag == "chronological_intersection") {
			CCalendar *calendar = NULL;
			CDate date;
			CDate intersecting_date;
				
			for (size_t j = 0; j < child_config_data->Properties.size(); ++j) {
				std::string key = child_config_data->Properties[j].first;
				std::string value = child_config_data->Properties[j].second;
				
				if (key == "calendar") {
					value = FindAndReplaceString(value, "_", "-");
					calendar = CCalendar::GetCalendar(value);
					if (!calendar) {
						fprintf(stderr, "Calendar \"%s\" does not exist.\n", value.c_str());
					}
				} else if (key == "date") {
					date = CDate::FromString(value);
				} else if (key == "intersecting_date") {
					intersecting_date = CDate::FromString(value);
				} else {
					fprintf(stderr, "Invalid year difference property: \"%s\".\n", key.c_str());
				}
			}
			
			if (!calendar) {
				fprintf(stderr, "Chronological intersection has no \"calendar\" property.\n");
				continue;
			}
			
			if (date.Year == 0) {
				fprintf(stderr, "Chronological intersection has no \"date\" property.\n");
				continue;
			}
			
			if (intersecting_date.Year == 0) {
				fprintf(stderr, "Chronological intersection has no \"intersecting_date\" property.\n");
				continue;
			}
			
			this->AddChronologicalIntersection(calendar, date, intersecting_date);
		} else {
			fprintf(stderr, "Invalid calendar property: \"%s\".\n", child_config_data->Tag.c_str());
		}
	}
	
	if (this->Months.empty()) {
		fprintf(stderr, "No months have been defined for calendar \"%s\".\n", this->Ident.c_str());
	}
	
	this->Initialized = true;
	
	if (!base_day_of_the_week.empty()) {
		this->BaseDayOfTheWeek = this->GetDayOfTheWeekByIdent(base_day_of_the_week);
	}
	
	//inherit the intersection points from the intersecting calendar that it has with third calendars
	for (std::map<CCalendar *, std::map<CDate, CDate>>::iterator iterator = this->ChronologicalIntersections.begin(); iterator != this->ChronologicalIntersections.end(); ++iterator) {
		CCalendar *intersecting_calendar = iterator->first;
		
		this->InheritChronologicalIntersectionsFromCalendar(intersecting_calendar);
	}
}

bool CCalendar::IsInitialized()
{
	return this->Initialized;
}

CDayOfTheWeek *CCalendar::GetDayOfTheWeekByIdent(const std::string &ident)
{
	if (this->DaysOfTheWeekByIdent.find(ident) != this->DaysOfTheWeekByIdent.end()) {
		return this->DaysOfTheWeekByIdent.find(ident)->second;
	}
	
	return NULL;
}

void CCalendar::AddChronologicalIntersection(CCalendar *intersecting_calendar, const CDate &date, const CDate &intersecting_date)
{
	if (!intersecting_calendar) {
		return;
	}
	
	if (this->ChronologicalIntersections[intersecting_calendar].find(date) != this->ChronologicalIntersections[intersecting_calendar].end()) {
		return; //already defined
	}
	
	this->ChronologicalIntersections[intersecting_calendar][date] = intersecting_date;
	
	intersecting_calendar->AddChronologicalIntersection(this, intersecting_date, date);
	
	InheritChronologicalIntersectionsFromCalendar(intersecting_calendar);
}

void CCalendar::InheritChronologicalIntersectionsFromCalendar(CCalendar *intersecting_calendar)
{
	if (!intersecting_calendar || !intersecting_calendar->IsInitialized()) {
		return;
	}
	
	for (std::map<CCalendar *, std::map<CDate, CDate>>::iterator iterator = intersecting_calendar->ChronologicalIntersections.begin(); iterator != intersecting_calendar->ChronologicalIntersections.end(); ++iterator) {
		CCalendar *third_calendar = iterator->first;
		
		if (third_calendar == this || !third_calendar->IsInitialized()) {
			continue;
		}
		
		for (std::map<CDate, CDate>::iterator sub_iterator = iterator->second.begin(); sub_iterator != iterator->second.end(); ++sub_iterator) {
			this->AddChronologicalIntersection(third_calendar, sub_iterator->first.ToCalendar(intersecting_calendar, this), sub_iterator->second);
		}
	}
}

std::pair<CDate, CDate> CCalendar::GetBestChronologicalIntersectionForDate(CCalendar *calendar, const CDate &date) const
{
	std::pair<CDate, CDate> chronological_intersection(*(new CDate), *(new CDate));
	
	if (this->ChronologicalIntersections.find(calendar) == this->ChronologicalIntersections.end()) {
		return chronological_intersection;
	}
	
	const std::map<CDate, CDate> &chronological_intersections_submap = this->ChronologicalIntersections.find(calendar)->second;
	
	std::map<CDate, CDate>::const_iterator iterator = chronological_intersections_submap.lower_bound(date); //get the lower bound for the chronological intersection submap
	
	if (iterator == chronological_intersections_submap.end()) { //no chronological intersections found for the lower bound
		if (!chronological_intersections_submap.empty()) {
			iterator--; //get the element just before the end
			chronological_intersection.first = iterator->first; //date in the given calendar
			chronological_intersection.second = iterator->second; //date in the calendar we want to convert to
		}
		return chronological_intersection;
	} else {
		chronological_intersection.first = iterator->first; //date in the given calendar
		chronological_intersection.second = iterator->second; //date in the calendar we want to convert to
		
		if (iterator->first == date) { //exact match, just return this intersection
			return chronological_intersection;
		}
		
		//see whether the next element has a smaller year difference
		int first_year_difference = abs(date.Year - iterator->first.Year);
		
		iterator++;
		
		if (iterator != chronological_intersections_submap.end()) {
			int second_year_difference = abs(date.Year - iterator->first.Year);
			
			if (second_year_difference < first_year_difference) {
				chronological_intersection.first = iterator->first; //date in the given calendar
				chronological_intersection.second = iterator->second; //date in the calendar we want to convert to
			}
		}
	}
	
	return chronological_intersection;
}

//@}
