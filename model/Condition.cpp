#include "Condition.h"
#include <sstream>
#include "TriggerVisitor.h"

#include "../view/utilunit.h"
#include "../util/utilio.h"
#include "../util/Buffer.h"
#include "../util/helper.h"

Condition::Condition()
:	ECBase(CONDITION),
	amount(-1),
	res_type(-1),
	object(-1),
	u_loc(-1),
	unit_cnst(-1),
	pUnit(NULL),
	player(-1),
	tech_cnst(-1),
	pTech(NULL),
	timer(-1),
	reserved(-1),
	// AOKRECT default constructor OK
	group(-1),
	utype(-1),
	ai_signal(-1),
	unknown1(-1),
	unknown2(-1),
	valid_since_last_check(true)
{
}

Condition::Condition(Buffer& b)
:	ECBase(CONDITION)
{
    b.read(&ttype, sizeof(ttype));
    b.read(&type, sizeof(type)); // unswap order of type with version, above, when using clipboard
    if (ttype == CONDITION || ttype == CONDITION_HD4) {
        b.read(&amount, sizeof(type));
        b.read(&res_type, sizeof(res_type));
        b.read(&object, sizeof(object));
        b.read(&u_loc, sizeof(u_loc));
        b.read(&unit_cnst, sizeof(unit_cnst));
        pUnit = esdata.units.getByIdSafe(unit_cnst);
        b.read(&player, sizeof(player));
        b.read(&tech_cnst, sizeof(tech_cnst));
	    pTech = esdata.techs.getByIdSafe(tech_cnst);
        b.read(&timer, sizeof(timer));
        b.read(&reserved, sizeof(reserved));
        b.read(&area, sizeof(area));
        b.read(&group, sizeof(group));
        b.read(&utype, sizeof(utype));
        b.read(&ai_signal, sizeof(ai_signal));
        if (ttype == CONDITION_HD4) {
            b.read(&unknown1, sizeof(unknown1));
            b.read(&unknown2, sizeof(unknown2));
        }
    } else {
		throw bad_data_error("Condition has incorrect check value.");
    }

	check_and_save();
}

inline std::string unitTypeName(const UnitLink *pUnit) {
    std::ostringstream convert;
    if (pUnit && pUnit->id()) {
        std::wstring unitname(pUnit->name());
        std::string un(unitname.begin(), unitname.end());
        convert << un;
    } else {
        convert << "unit";
    }
    return convert.str();
}

std::string Condition::selectedUnits() const {
    std::ostringstream convert;
    bool class_selected = group >= 0 && group < NUM_GROUPS - 1;
    bool unit_type_selected = utype >= 0 && utype < NUM_UTYPES - 1;
    if (player >= 0)
        convert << playerPronoun(player) << " ";
    if (pUnit && pUnit->id()) {
        convert << unitTypeName(pUnit);
    } else if (class_selected || unit_type_selected) { // account for groups[0]={-1,None}
        if (class_selected && unit_type_selected) {
            convert << "units of class " << groups[group + 1].name;
            convert << "and type " << groups[group + 1].name;
        } else if (class_selected) {
            convert << "units of class " << groups[group + 1].name;
        } else {
            convert << "units of type " << groups[group + 1].name;
        }
    } else {
        convert << "units";
    }
    if (!(object == -1 && !valid_unit_id(object))) {
        convert << " " << object << " (" << get_unit_full_name(object) << ")";
    }
    return convert.str();
}

std::string Condition::getName(bool tip, NameFlags::Value flags) const
{
    if (!tip) {
	    return (type < scen.pergame->max_condition_types) ? types[type] : "Unknown!";
	} else {
	    std::string stype = std::string("");
        std::ostringstream convert;
        switch (type) {
            case (long)ConditionType::None:
                // Let this act like a separator
                convert << "                                                                                    ";
                stype.append(convert.str());
                break;
            case (long)ConditionType::BringObjectToArea:
                convert << "unit " << object << " (" << get_unit_full_name(object) << ")";
                if (valid_full_map()) {
                    convert << " is on the map";
                } else {
                    if (valid_area_location()) {
                        convert << " is at (" << area.left << ", " << area.top << ")";
                    } else {
                        convert << " is in the area (" << area.left << ", " << area.top << ") - (" << area.right << ", " << area.bottom << ")";
                    }
                }
                stype.append(convert.str());
                break;
            case (long)ConditionType::BringObjectToObject:
                convert << "unit " << object << " (" << get_unit_full_name(object) << ") is next to unit " << u_loc << " (" << get_unit_full_name(u_loc) << ")";
                stype.append(convert.str());
                break;
            case (long)ConditionType::OwnObjects:
            case (long)ConditionType::OwnFewerObjects:
            case (long)ConditionType::ObjectsInArea:
                { // we define some variables in this block, therefore need scope as we are also in a case
                    convert << playerPronoun(player) << " has ";
                    switch (type) {
                        case 3:
                        case 5:
                            if (amount == 0) {
                                convert << "any number of";
                            } else {
                                convert << "at least " << amount;
                            }
                            break;
                        case 4:
                            if (amount == 0) {
                                convert << "no";
                            } else {
                                convert << "at most " << amount;
                            }
                            break;
                    }
                    if (valid_unit_spec()) {
                        std::string un(wstringToString(pUnit->name()));
                        if (amount > 1 && !un.empty() && *un.rbegin() != 's' && !replaced(un, "man", "men")) {
                            convert << " " << un << "s";
                        } else {
                            convert << " " << un;
                        }
                    } else {
                        if (amount != 1) {
                            convert << " units";
                        } else {
                            convert << " unit";
                        }
                    }
                    if (valid_full_map()) {
                        convert << " on the map";
                    } else {
                        if (valid_area_location()) {
                            convert << " at (" << area.left << ", " << area.top << ")";
                        } else {
                            convert << " in area (" << area.left << ", " << area.bottom << ") - (" << area.right << ", " << area.top << ")";
                        }
                    }
                    stype.append(convert.str());
                }
                break;
            case (long)ConditionType::DestroyObject:
                convert << "unit " << object;
                convert << " (" << get_unit_full_name(object) << ")";
                convert << " is destroyed";
                stype.append(convert.str());
                break;
            case (long)ConditionType::CaptureObject:
                convert << playerPronoun(player) << " captured unit " << object;
                convert << " (" << get_unit_full_name(object) << ")";
                stype.append(convert.str());
                break;
            case (long)ConditionType::AccumulateAttribute:
                switch (res_type) {
                    case 0: // Food accumulated
                        convert << playerPronoun(player) << " has " << amount << " food";
                        break;
                    case 1: // Wood accumulated
                        convert << playerPronoun(player) << " has " << amount << " wood";
                        break;
                    case 2: // Stone accumulated
                        convert << playerPronoun(player) << " has " << amount << " stone";
                        break;
                    case 3: // Gold accumulated
                        convert << playerPronoun(player) << " has " << amount << " gold";
                        break;
                    case 20: // Units killed
                        if (amount == 1) {
                            convert << playerPronoun(player) << " kills a unit";
                        } else {
                            convert << playerPronoun(player) << " has killed " << amount << " units";
                        }
                        break;
                    case 44: // Kill ratio
                        if (amount == 0) {
                            convert << playerPronoun(player) << " has equal kills and fatalities";
                        } else if (amount == 1) {
                            convert << playerPronoun(player) << " has killed one more than lost";
                        } else if (amount > 0) {
                            convert << playerPronoun(player) << " has " << amount << " more kills than fatalities";
                        } else if (amount == -1) {
                            convert << playerPronoun(player) << " has lost one more unit than has killed";
                        } else {
                            convert << playerPronoun(player) << " has " << -amount << " more fatalities than kills";
                        }
                        break;
                    default:
                        //convert << types_short[type];
                        if (res_type >= 0) {
                            const Link * list = esdata.resources.head();
	                        for (int i=0; list; list = list->next(), i++)
	                        {
		                        if (i == res_type) {
                                    std::wstring resname(list->name());
		                            convert << playerPronoun(player) << "'s ";
                                    convert << std::string( resname.begin(), resname.end());
                                    convert << " = " << amount;
		                            break;
		                        }
	                        }
	                    }
                        break;
                }
                stype.append(convert.str());
                break;
            case (long)ConditionType::ResearchTehcnology:
                if (valid_technology_spec()) {
                    convert << playerPronoun(player) << " has tech ";
                    std::wstring techname(pTech->name());
                    convert << std::string( techname.begin(), techname.end());
                    convert << " researched";
                } else {
                    convert << "INVALID";
                }
                stype.append(convert.str());
                break;
            case (long)ConditionType::ResearchingTechnology:
                if (pTech && pTech->id()) {
                    convert << playerPronoun(player) << " is researching ";
                    std::wstring techname(pTech->name());
                    convert << std::string( techname.begin(), techname.end());
                } else {
                    convert << "INVALID";
                }
                stype.append(convert.str());
                break;
            case (long)ConditionType::Timer:
                convert << time_string(timer) << " has passed";
                stype.append(convert.str());
                break;
            case (long)ConditionType::ObjectSelected:
                convert << "selected unit " << object;
                convert << " (" << get_unit_full_name(object) << ")";
                stype.append(convert.str());
                break;
            case (long)ConditionType::AISignal:
                switch (ai_signal) {
                case -1034:
                    convert << "singleplayer / cheats enabled";
                    break;
                case -1036:
                    convert << "starting resources set to standard";
                    break;
                case -1039:
                    convert << "regicide";
                    break;
                case -1040:
                    convert << "deathmatch";
                    break;
                case -70850:
                    convert << "one-click garrison";
                    break;
                default:
                    if (ai_signal >= -518 && ai_signal <= -7) {
                        int signal = ai_signal + 518;
                        int taunt_player = signal / 64;
                        int taunt_set_id = signal % 64;
                        convert << "player " << taunt_player + 1 << " taunted " << taunt_set[taunt_set_id];
                    } else if (ai_signal >= -774 && ai_signal <= -519) {
                        convert << "AI goal " << ai_signal + 774 << " achieved";
                    } else {
                        convert << "AI signalled " << ai_signal;
                    }
                }
                stype.append(convert.str());
                break;
            case (long)ConditionType::ObjectVisible:
                convert << "unit " << object;
                convert << " (" << get_unit_full_name(object) << ") is visible";
                stype.append(convert.str());
                break;
            case (long)ConditionType::PlayerDefeated:
                if (player == 0) {
                    convert << "Gaia";
                } else {
                    convert << playerPronoun(player);
                }
                convert << " is defeated";
                stype.append(convert.str());
                break;
            case (long)ConditionType::ObjectHasTarget:
                convert << "unit " << object << " (" << get_unit_full_name(object) << ") is targetting";
                if (null_location_unit()) {
                    convert << " something";
                } else {
                    convert << " " << u_loc << " (" << get_unit_full_name(u_loc) << ")";
                }
                stype.append(convert.str());
                break;
            case (long)ConditionType::UnitsGarrisoned:
                if (amount == 1) {
                    convert << "unit " << object << " (" << get_unit_full_name(object) << ") has " << amount << " units garrisoned";
                } else {
                    convert << "unit " << object << " (" << get_unit_full_name(object) << ") has one unit garrisoned";
                }
                stype.append(convert.str());
                break;
            case (long)ConditionType::DifficultyLevel:
                convert << "difficulty is ";
                switch (amount) {
                    case 0:
                        convert << "Harder";
                        break;
                    case 1:
                        convert << "Hard";
                        break;
                    case 2:
                        convert << "Moderate";
                        break;
                    case 3:
                        convert << "Standard";
                        break;
                    case 4:
                        convert << "Easiest";
                        break;
                }
                stype.append(convert.str());
                break;
            case (long)ConditionType::UnitsQueuedPastPopCap_SWGB:
                convert << playerPronoun(player) << " has " << amount << " units queued past the pop cap";
                stype.append(convert.str());
                break;
            default:
                stype.append((type < scen.pergame->max_condition_types) ? types_short[type] : "Unknown!");
        }

        return flags&NameFlags::LIMITLEN?stype.substr(0,100):stype;
    }
}

int Condition::getPlayer() const
{
	return player;
}

void Condition::setPlayer(int pplayer)
{
	player = pplayer;
}

bool Condition::get_valid_since_last_check() {
    return valid_since_last_check;
}

bool Condition::check_and_save()
{
    return valid_since_last_check = check();
}

inline bool Condition::valid_full_map() const {
    return (area.left == -1 && area.right == -1 && area.bottom == -1 && area.top == -1);
}

inline bool Condition::valid_partial_map() const {
    return (area.left >=  0 && area.right >= area.left && area.bottom >=  0 && area.top >= area.bottom);
}

inline bool Condition::valid_area() const {
    return valid_full_map() || valid_partial_map();
}

inline bool Condition::valid_area_location() const {
    return area.left == area.right && area.top == area.bottom;
}

inline bool Condition::valid_unit_spec() const {
    // utype >= 0
	return pUnit != NULL && pUnit->id() >= 0;
}

inline bool Condition::valid_technology_spec() const {
	return pTech != NULL && pTech->id() >= 0;
}

inline bool Condition::null_object() const {
	return object == -1;
}

inline bool Condition::valid_object() const {
	return valid_unit_id(object);
}

inline bool Condition::null_location_unit() const {
	return u_loc == -1;
}

inline bool Condition::valid_location_unit() const {
	return valid_unit_id(u_loc);
}

bool Condition::check() const
{
    if (type < 1 || type >= scen.pergame->max_condition_types) {
        return false;
    }

	switch (type)
	{
	case ConditionType::BringObjectToArea:
		return (valid_unit_id(object) && valid_partial_map());

	case ConditionType::BringObjectToObject:
		return (valid_unit_id(object) && valid_location_unit());

	case ConditionType::ObjectHasTarget:
		return (valid_unit_id(object) && (null_location_unit() || valid_location_unit()));

	case ConditionType::OwnObjects:
	case ConditionType::OwnFewerObjects:
	case ConditionType::OwnFewerFoundations_SWGB:
		return (player >= 0 && amount >= 0 && valid_area());

	case ConditionType::ObjectsInArea:
	case ConditionType::SelectedObjectsInArea_SWGB:
	case ConditionType::PoweredObjectsInArea_SWGB:
		return (valid_area() && amount >= 0);

	case ConditionType::DestroyObject:
		return (valid_unit_id(object));

	case ConditionType::CaptureObject:
		return (valid_unit_id(object) && player >= 0);

	case ConditionType::AccumulateAttribute:
		return (player >= 0 && res_type >= 0);

	case ConditionType::ResearchTehcnology:
	case ConditionType::ResearchingTechnology:
		return (player >= 0 && valid_technology_spec());

	case ConditionType::Timer:
		return (timer >= 0);

	case ConditionType::ObjectSelected:
		return (valid_unit_id(object));

	case ConditionType::AISignal:
		return (true);

	case ConditionType::PlayerDefeated:
	case ConditionType::UnitsQueuedPastPopCap_SWGB:
		return (player >= 0);

	case ConditionType::ObjectVisible:
	case ConditionType::ObjectNotVisible:
		return (valid_unit_id(object));

	//ConditionType::ResearchingTechnology above

	case ConditionType::UnitsGarrisoned:
		return (valid_unit_id(object) && amount >= 0);

	case ConditionType::DifficultyLevel:
		return (amount >= 0);

	//ConditionType::OwnFewerFoundations shares with OwnObjects above
	//ConditionType::SelectedObjectsInArea shares with ObjectsInArea above
	//ConditionType::PoweredObjectsInArea shares with ObjectsInArea above
	//ConditionType::UnitsQueuedPastPopCap shares with PlayerDefeated above

	default:
		return false;
	}
}

void Condition::read(FILE *in)
{
    readbin(in, &type);
    readbin(in, &ttype);
    if (ttype == CONDITION || ttype == CONDITION_HD4) {
        ttype = static_cast<TType>(ttype);
        readbin(in, &amount);
        readbin(in, &res_type);
        readbin(in, &object);
        readbin(in, &u_loc);
        readbin(in, &unit_cnst);
        pUnit = esdata.units.getByIdSafe(unit_cnst);
        readbin(in, &player);
        readbin(in, &tech_cnst);
	    pTech = esdata.techs.getByIdSafe(tech_cnst);
        readbin(in, &timer);
        readbin(in, &reserved);
        readbin(in, &area);
        readbin(in, &group);
        readbin(in, &utype);
        readbin(in, &ai_signal);
        if (ttype == CONDITION_HD4) {
            readbin(in, &unknown1);
            readbin(in, &unknown2);
        }
    } else {
		throw bad_data_error("Condition has incorrect check value.");
    }

	check_and_save();
}

void Condition::write(FILE *out)
{
	writebin(out, &type);
	writebin(out, &ttype);
	writebin(out, &amount);
	writebin(out, &res_type);
	writebin(out, &object);
	writebin(out, &u_loc);
	unit_cnst =  pUnit ? pUnit->id() : -1,
	writebin(out, &unit_cnst);
	writebin(out, &player);
	tech_cnst = pTech ? pTech->id() : -1;
	writebin(out, &tech_cnst);
	writebin(out, &timer);
	writebin(out, &reserved);
	writebin(out, &area);
	writebin(out, &group);
	writebin(out, &utype);
	writebin(out, &ai_signal);
    if (ttype == CONDITION_HD4) {
	    writebin(out, &unknown1);
	    writebin(out, &unknown2);
    }
}

/* Used forBuffer operations (i.e., copy & paste) */
void Condition::tobuffer(Buffer &b)// const (make it const when unit_cnst gets set elsewhere)
{
	b.write(&ttype, sizeof(ttype));
	b.write(&type, sizeof(type)); // swap order of type with version, above, when using clipboard
	b.write(&amount, sizeof(amount));
	b.write(&res_type, sizeof(res_type));
	b.write(&object, sizeof(object));
	b.write(&u_loc, sizeof(u_loc));
	unit_cnst = pUnit ? pUnit->id() : -1;
	b.write(&unit_cnst, sizeof(unit_cnst));
	b.write(&player, sizeof(tech_cnst));
	tech_cnst = pTech ? pTech->id() : -1;
	b.write(&tech_cnst, sizeof(tech_cnst));
	b.write(&timer, sizeof(timer));
	b.write(&reserved, sizeof(reserved));
	b.write(&area, sizeof(area));
	b.write(&group, sizeof(group));
	b.write(&utype, sizeof(utype));
	b.write(&ai_signal, sizeof(ai_signal));
    if (ttype == CONDITION_HD4) {
	    b.write(&utype, sizeof(unknown1));
	    b.write(&utype, sizeof(unknown2));
    }
}

void Condition::accept(TriggerVisitor& tv)
{
	tv.visit(*this);
}

const char *Condition::types_aok[] =
{
	"",
	"Bring Object to Area",
	"Bring Object to Object",
	"Own Objects",
	"Own Fewer Objects",
	"Objects in Area",
	"Destroy Object",
	"Capture Object",
	"Accumulate Attribute",
	"Researched Technology",
	"Timer",
	"Object Selected",
	"AI Signal",
	"Player Defeated",
	"Object Has Target",
	"Object Visible",
	"Object Not Visible",
	"Researching Technology",
	"Units Garrisoned",
	"Difficulty Level"
};

const char *Condition::types_swgb[] =
{
	"",
	"Bring Object to Area",
	"Bring Object to Object",
	"Own Objects",
	"Own Fewer Objects",
	"Objects in Area",
	"Destroy Object",
	"Capture Object",
	"Accumulate Attribute",
	"Researched Technology",
	"Timer",
	"Object Selected",
	"AI Signal",
	"Player Defeated",
	"Object Has Target",
	"Object Visible",
	"Object Not Visible",
	"Researching Technology",
	"Units Garrisoned",
	"Difficulty Level",
	"Own Fewer Foundations",
	"Selected Objects in Area",
};

const char *Condition::types_cc[] =
{
	"",
	"Bring Object to Area",
	"Bring Object to Object",
	"Own Objects",
	"Own Fewer Objects",
	"Objects in Area",
	"Destroy Object",
	"Capture Object",
	"Accumulate Attribute",
	"Researched Technology",
	"Timer",
	"Object Selected",
	"AI Signal",
	"Player Defeated",
	"Object Has Target",
	"Object Visible",
	"Object Not Visible",
	"Researching Technology",
	"Units Garrisoned",
	"Difficulty Level",
	"Own Fewer Foundations",
	"Selected Objects in Area",
	"Powered Objects in Area",
	"Units Queued Past Pop Cap"
};

const char *Condition::types_short_aok[] =
{
	"",
	"Arrived",
	"At Object",
	"Own",
	"Own Fewer",
	"In Area",
	"Destroyed",
	"Captured",
	"Accumulated",
	"Researched",
	"Time",
	"Selected",
	"AI Signal",
	"Defeated",
	"Targetting",
	"Visible",
	"Not Visible",
	"Researching",
	"Garrisoned",
	"Difficulty"
};

const char *Condition::types_short_swgb[] =
{
	"",
	"Arrived",
	"At Object",
	"Own",
	"Own Fewer",
	"In Area",
	"Destroyed",
	"Captured",
	"Accumulated",
	"Researched",
	"Time",
	"Selected",
	"AI Signal",
	"Defeated",
	"Targetting",
	"Visible",
	"Not Visible",
	"Researching",
	"Garrisoned",
	"Difficulty",
	"Fewer Foundations",
	"Selected in Area"
};

const char *Condition::types_short_cc[] =
{
	"",
	"Arrived",
	"At Object",
	"Own",
	"Own Fewer",
	"In Area",
	"Destroyed",
	"Captured",
	"Accumulated",
	"Researched",
	"Time",
	"Selected",
	"AI Signal",
	"Defeated",
	"Targetting",
	"Visible",
	"Not Visible",
	"Researching",
	"Garrisoned",
	"Difficulty",
	"Fewer Foundations",
	"Selected in Area",
	"Powered in Area",
	"Queued Past Pop Cap"
};

const char *Condition::virtual_types_aok[] = {
    ""
};

const char *Condition::virtual_types_aoc[] = {
    "",
    "Singleplayer / Cheats Enabled",
    "Taunt",
    "AI Script Goal",
    "Starting age: Standard",
    "Starting resources: Standard",
    "Regicide",
    "Deathmatch",
    //"One-click Garrison",
};

const char *Condition::virtual_types_aohd[] = {
    ""
};

const char *Condition::virtual_types_aof[] = {
    ""
};

const char *Condition::virtual_types_up[] = {
    "",
    "Singleplayer / Cheats Enabled",
    "Taunt",
    "AI Script Goal",
    "Starting age: Standard",
    "Starting resources: Standard",
    "Regicide",
    "Deathmatch",
    //"One-click Garrison",
};

const char *Condition::virtual_types_cc[] = {
    ""
};

const char *Condition::virtual_types_swgb[] = {
    ""
};

const char *Condition::taunt_set[] = {
    "1, 2, 3 or 4",
    "5, 6, 7 or 8",
    "9, 10, 11 or 12",
    "13, 14, 15 or 16",
    "17, 18, 19 or 20",
    "21, 22, 23 or 24",
    "25, 26, 27 or 28",
    "29, 30, 31 or 32",
    "33, 34, 35 or 36",
    "37, 38, 39 or 40",
    "41, 42, 43 or 44",
    "45, 46, 47 or 48",
    "49, 50, 51 or 52",
    "53, 54, 55 or 56",
    "57, 58, 59 or 60",
    "61, 62, 63 or 64",
    "65, 66, 67 or 68",
    "69, 70, 71 or 72",
    "73, 74, 75 or 76",
    "77, 78, 79 or 80",
    "81, 82, 83 or 84",
    "85, 86, 87 or 88",
    "89, 90, 91 or 92",
    "93, 94, 95 or 96",
    "97, 98, 99 or 100",
    "101, 102, 103 or 104",
    "105, 106, 107 or 108",
    "109, 110, 111 or 112",
    "113, 114, 115 or 116",
    "117, 118, 119 or 120",
    "121, 122, 123 or 124",
    "125, 126, 127 or 128",
    "129, 130, 131 or 132",
    "133, 134, 135 or 136",
    "137, 138, 139 or 140",
    "141, 142, 143 or 144",
    "145, 146, 147 or 148",
    "149, 150, 151 or 152",
    "153, 154, 155 or 156",
    "157, 158, 159 or 160",
    "161, 162, 163 or 164",
    "165, 166, 167 or 168",
    "169, 170, 171 or 172",
    "173, 174, 175 or 176",
    "177, 178, 179 or 180",
    "181, 182, 183 or 184",
    "185, 186, 187 or 188",
    "189, 190, 191 or 192",
    "193, 194, 195 or 196",
    "197, 198, 199 or 200",
    "201, 202, 203 or 204",
    "205, 206, 207 or 208",
    "209, 210, 211 or 212",
    "213, 214, 215 or 216",
    "217, 218, 219 or 220",
    "221, 222, 223 or 224",
    "225, 226, 227 or 228",
    "229, 230, 231 or 232",
    "233, 234, 235 or 236",
    "237, 238, 239 or 240",
    "241, 242, 243 or 244",
    "245, 246, 247 or 248",
    "249, 250, 251 or 252",
    "253, 254, 255 or 256"
};

const char** Condition::types;
const char** Condition::types_short;
const char** Condition::virtual_types;
