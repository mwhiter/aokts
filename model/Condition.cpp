#include "Condition.h"
#include <sstream>
#include "TriggerVisitor.h"

#include "../view/utilunit.h"
#include "../util/utilio.h"
#include "../util/Buffer.h"
#include "../util/helper.h"

#pragma pack(push, 4)
// An condition as stored in in scenario
struct Genie_Condition
{
	long type;
	long check;
	long amount;
	long resource_type;
	long uid_object;
	long uid_location;
	long unit_const;
	long player;
	long technology;
	long timer;
	long reserved;
	AOKRECT area;
	long unit_group;
	long unit_type;
	long ai_signal;
};
#pragma pack(pop)

Condition::Condition()
:	ECBase(CONDITION),
	amount(-1),
	res_type(-1),
	object(-1),
	u_loc(-1),
	pUnit(NULL),
	player(-1),
	pTech(NULL),
	timer(-1),
	reserved(-1),
	// AOKRECT default constructor OK
	group(-1),
	utype(-1),
	ai_signal(-1)
{
}

Condition::Condition(Buffer& b)
:	ECBase(CONDITION)
{
	// read flat data
	Genie_Condition genie;
	b.read(&genie, sizeof(genie));
	std::swap(genie.type, genie.check); // HACK: un-swap type, check
	fromGenie(genie);
}

std::string Condition::getName(bool tip, NameFlags::Value flags) const
{
    if (!tip) {
	    return (type < scen.pergame->max_condition_types) ? types[type] : "Unknown!";
	} else {
	    std::string stype = std::string("");
        std::ostringstream convert;
        switch (type) {
            case 0: // Undefined
                // Let this act like a separator
                convert << "                                                                                    ";
                stype.append(convert.str());
                break;
            case 1: // Bring object to area
                convert << "unit " << object << " (" << get_unit_full_name(object) << ")";
                if (area.left == -1 && area.right == -1 && area.top == -1 && area.bottom == -1) {
                    convert << " is on the map";
                } else {
                    if (area.left == area.right && area.top == area.bottom) {
                        convert << " is at (" << area.left << ", " << area.top << ")";
                    } else {
                        convert << " is in the area (" << area.left << ", " << area.top << ") - (" << area.right << ", " << area.bottom << ")";
                    }
                }
                stype.append(convert.str());
                break;
            case 2: // Bring object to object
                convert << "unit " << object << " (" << get_unit_full_name(object) << ") is next to unit " << u_loc << " (" << get_unit_full_name(u_loc) << ")";
                stype.append(convert.str());
                break;
            case 3: // Own
            case 4: // Own Fewer
            case 5: // In Area
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
                    if (pUnit && pUnit->id()) {
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
                    if (area.left == -1 && area.right == -1 && area.top == -1 && area.bottom == -1) {
                        convert << " on the map";
                    } else {
                        if (area.left == area.right && area.top == area.bottom) {
                            convert << " at (" << area.left << ", " << area.top << ")";
                        } else {
                            convert << " in area (" << area.left << ", " << area.bottom << ") - (" << area.right << ", " << area.top << ")";
                        }
                    }
                    stype.append(convert.str());
                }
                break;
            case 6: // Destroy object
                convert << "unit " << object;
                convert << " (" << get_unit_full_name(object) << ")";
                convert << " is destroyed";
                stype.append(convert.str());
                break;
            case 7: // Unit captured
                convert << playerPronoun(player) << " captured unit " << object;
                convert << " (" << get_unit_full_name(object) << ")";
                stype.append(convert.str());
                break;
            case 8: // Accumulated
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
		                            convert << playerPronoun(player) << " has " << amount << " ";
                                    convert << std::string( resname.begin(), resname.end());
                                    convert << "(res_type " << res_type << ")";
		                            break;
		                        }
	                        }
	                    }
                        break;
                }
                stype.append(convert.str());
                break;
            case  9: // Researched
                if (pTech && pTech->id()) {
                    convert << playerPronoun(player) << " has tech ";
                    std::wstring techname(pTech->name());
                    convert << std::string( techname.begin(), techname.end());
                    convert << " researched";
                } else {
                    convert << "INVALID";
                }
                stype.append(convert.str());
                break;
            case  17: // Researching
                if (pTech && pTech->id()) {
                    convert << playerPronoun(player) << " is researching ";
                    std::wstring techname(pTech->name());
                    convert << std::string( techname.begin(), techname.end());
                } else {
                    convert << "INVALID";
                }
                stype.append(convert.str());
                break;
            case 10: // Time
                convert << time_string(timer) << " has passed";
                stype.append(convert.str());
                break;
            case 11: // Unit selected
                convert << "selected unit " << object;
                convert << " (" << get_unit_full_name(object) << ")";
                stype.append(convert.str());
                break;
            case 12: // AI script goal
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
            case 15: // object visible
                convert << "unit " << object;
                convert << " (" << get_unit_full_name(object) << ") is visible";
                stype.append(convert.str());
                break;
            case 13: // Player defeated
                if (player == 0) {
                    convert << "Gaia";
                } else {
                    convert << playerPronoun(player);
                }
                convert << " is defeated";
                stype.append(convert.str());
                break;
            case 18: // Units Garrisoned
                if (amount == 1) {
                    convert << "unit " << object << " (" << get_unit_full_name(object) << ") has " << amount << " units garrisoned";
                } else {
                    convert << "unit " << object << " (" << get_unit_full_name(object) << ") has one unit garrisoned";
                }
                stype.append(convert.str());
                break;
            case 19: // Difficulty
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
            case 24: // Queued Past Pop Cap
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

bool Condition::check() const
{
	switch (type)
	{
	case CONDITION_BringObjectToArea:
		return (object >= 0 && area.left >= 0);

	case CONDITION_BringObjectToObject:
		return (object >= 0 && u_loc >= 0);

	case CONDITION_OwnObjects:
	case CONDITION_OwnFewerObjects:
	case CONDITION_OwnFewerFoundations:
		return (player >= 0 && amount >= 0 && !(area.top == 0 && area.left == 0 && area.bottom == 0 && area.right == 0));

	case CONDITION_ObjectsInArea:
	case CONDITION_SelectedObjectsInArea:
	case CONDITION_PoweredObjectsInArea:
		return (area.left >= 0 && amount >= 0);

	case CONDITION_DestroyObject:
		return (object >= 0);

	case CONDITION_CaptureObject:
		return (object >= 0 && player >= 0);

	case CONDITION_AccumulateAttribute:
		return (player >= 0 && res_type >= 0);

	case CONDITION_ResearchTehcnology:
	case CONDITION_ResearchingTechnology:
		return (player >= 0 && pTech->id() >= 0);

	case CONDITION_Timer:
		return (timer >= 0);

	case CONDITION_ObjectSelected:
		return (object >= 0);

	case CONDITION_AISignal:
		return (true);

	case CONDITION_PlayerDefeated:
	case CONDITION_UnitsQueuedPastPopCap:
		return (player >= 0);

	case CONDITION_ObjectHasTarget:
		return (object >= 0 && u_loc >= 0);

	case CONDITION_ObjectVisible:
	case CONDITION_ObjectNotVisible:
		return (object >= 0);

	//CONDITION_ResearchingTechnology above

	case CONDITION_UnitsGarrisoned:
		return (object >= 0 && amount >= 0);

	case CONDITION_DifficultyLevel:
		return (amount >= 0);

	//CONDITION_OwnFewerFoundations shares with OwnObjects above
	//CONDITION_SelectedObjectsInArea shares with ObjectsInArea above
	//CONDITION_PoweredObjectsInArea shares with ObjectsInArea above
	//CONDITION_UnitsQueuedPastPopCap shares with PlayerDefeated above

	default:
		return false;
	}
}

void Condition::read(FILE *in)
{
	Genie_Condition genie;

	readbin(in, &genie);
	fromGenie(genie);
}

void Condition::write(FILE *out)
{
	Genie_Condition genie = toGenie();
	writebin(out, &genie);
}

void Condition::tobuffer(Buffer &b) const
{
	/* Even though the Genie format sucks, we use it for Buffer operations
	 * (i.e., copy & paste) since it's easier to maintain one sucky format than
	 * one sucky and one slightly-less-sucky format.
	 */

	// write flat data
	Genie_Condition genie = toGenie();
	std::swap(genie.type, genie.check); // HACK: swap type, check
	b.write(&genie, sizeof(genie));
}

void Condition::accept(TriggerVisitor& tv)
{
	tv.visit(*this);
}

void Condition::fromGenie(const Genie_Condition& genie)
{
	if (genie.check != CONDITION)
		throw bad_data_error("Condition has incorrect check value.");

	type = genie.type;
	ttype = static_cast<TType>(genie.check);
	res_type = genie.resource_type;
	amount = genie.amount;
	object = genie.uid_object;
	u_loc = genie.uid_location;
	pUnit = esdata.units.getByIdSafe(genie.unit_const);
	player = genie.player;
	pTech = esdata.techs.getByIdSafe(genie.technology);
	timer = genie.timer;
	reserved = genie.reserved;
	area = genie.area;
	group = genie.unit_group;
	utype = genie.unit_type;
	ai_signal = genie.ai_signal;
}

Genie_Condition Condition::toGenie() const
{
	Genie_Condition ret =
	{
		type,
		ttype,
		amount,
		res_type,
		object,
		u_loc,
		pUnit ? pUnit->id() : -1,
		player,
		pTech ? pTech->id() : -1,
		timer,
		reserved,
		area,
		group,
		utype,
		ai_signal
	};

	return ret;
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
    "None"
};

const char *Condition::virtual_types_aoc[] = {
    "None",
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
    "None"
};

const char *Condition::virtual_types_aof[] = {
    "None"
};

const char *Condition::virtual_types_up[] = {
    "None",
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
    "None"
};

const char *Condition::virtual_types_swgb[] = {
    "None"
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
