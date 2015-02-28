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
	    return (type < NUM_CONDS) ? types[type] : "Unknown!";
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
                    if (player == 0) {
                        convert << "Gaia";
                    } else {
                        convert << "p" << player;
                    }
                    convert << " has ";
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
                        std::wstring unitname(pUnit->name());
                        std::string un(unitname.begin(), unitname.end());
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
                convert << "p" << player << " captured unit " << object;
                convert << " (" << get_unit_full_name(object) << ")";
                stype.append(convert.str());
                break;
            case 8: // Accumulated
                switch (res_type) {
                    case 0: // Food accumulated
                        convert << "p" << player << " has " << amount << " food";
                        break;
                    case 1: // Wood accumulated
                        convert << "p" << player << " has " << amount << " wood";
                        break;
                    case 2: // Stone accumulated
                        convert << "p" << player << " has " << amount << " stone";
                        break;
                    case 3: // Gold accumulated
                        convert << "p" << player << " has " << amount << " gold";
                        break;
                    case 20: // Units killed
                        if (amount == 1) {
                            convert << "p" << player << " kills a unit";
                        } else {
                            convert << "p" << player << " has killed " << amount << " units";
                        }
                        break;
                    case 44: // Kill ratio
                        if (amount == 0) {
                            convert << "p" << player << " has equal kills and fatalities";
                        } else if (amount == 1) {
                            convert << "p" << player << " has killed one more than lost";
                        } else if (amount > 0) {
                            convert << "p" << player << " has " << amount << " more kills than fatalities";
                        } else if (amount == -1) {
                            convert << "p" << player << " has lost one more unit than has killed";
                        } else {
                            convert << "p" << player << " has " << -amount << " more fatalities than kills";
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
		                            convert << "p" << player << " has " << amount << " ";
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
                    convert << "p" << player << " has tech ";
                    std::wstring techname(pTech->name());
                    convert << std::string( techname.begin(), techname.end());
                    convert << " researched";
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
                convert << "AI signalled " << ai_signal;
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
                    convert << "p" << player;
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
                convert << "difficulty ";
                switch (amount) {
                    case 0:
                        convert << "harder";
                        break;
                    case 1:
                        convert << "hard";
                        break;
                    case 2:
                        convert << "moderate";
                        break;
                    case 3:
                        convert << "standard";
                        break;
                    case 4:
                        convert << "easiest";
                        break;
                }
                stype.append(convert.str());
                break;
            case 24: // Queued Past Pop Cap
                convert << "p" << player << " has " << amount << " units queued past the pop cap";
                stype.append(convert.str());
                break;
            default:
                stype.append((type < NUM_CONDS) ? types_short[type] : "Unknown!");
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
		return (ai_signal >= 0);

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

const char *Condition::types[] =
{
	"Undefined",
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

const char *Condition::types_short[] =
{
	"Undefined",
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
