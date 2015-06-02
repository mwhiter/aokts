#include "Effect.h"
#include <sstream>
#include <algorithm>
#include "scen.h"
#include "TriggerVisitor.h"

#include "../view/utilunit.h"
#include "../util/utilio.h"
#include "../view/utilui.h"
#include "../util/Buffer.h"
#include "../util/helper.h"
#include "../util/settings.h"
#include "../util/cpp11compat.h"

using std::vector;

extern class Scenario scen;

#pragma pack(push, 4)
// An effect as stored in scenario, up to strings
struct Genie_Effect
{
	long type;
	long check;
	long ai_goal;
	long amount;
	long resource;
	long diplomacy;
	long num_selected;
	long uid_location;
	long unit_constant;
	long player_source;
	long player_target;
	long technology;
	long textid;
	long soundid;
	long display_time;
	long trigger_index;
	AOKPT location;
	AOKRECT area;
	long unit_group;
	long unit_type;
	long panel;
};
#pragma pack(pop)

Effect::Effect()
:	ECBase(EFFECT),
	ai_goal(-1),
	amount(-1),
	res_type(-1),
	diplomacy(-1),
	num_sel(-1),
	uid_loc(-1),
	pUnit(NULL),
	s_player(-1),
	t_player(-1),
	pTech(NULL),
	textid(-1),
	soundid(-1),
	disp_time(-1),
	trig_index(-1),
	// location default ctor fine
	// area default ctor fine
	group(-1),
	utype(-1),
	panel(-1)
{
	memset(uids, -1, sizeof(uids));
}

//Effect::Effect( const Effect& other ) {
//}
//
//Effect::~Effect()
//{
//}

Effect::Effect(Buffer &b)
:	ECBase(EFFECT)
{
	// read flat data
	Genie_Effect genie;
	b.read(&genie, sizeof(genie));
	std::swap(genie.type, genie.check); // HACK: un-swap type, check
	fromGenie(genie);

	// move on to non-flat data
	text.read(b, sizeof(long));
	sound.read(b, sizeof(long));
	if (num_sel > 0)
		b.read(uids, sizeof(uids));
}

void Effect::tobuffer(Buffer &b) const
{
	/* Even though the Genie format sucks, we use it for Buffer operations
	 * (i.e., copy & paste) since it's easier to maintain one sucky format than
	 * one sucky and one slightly-less-sucky format.
	 */

	// write flat data
	Genie_Effect genie = toGenie();
	std::swap(genie.type, genie.check); // HACK: swap type, check
	b.write(&genie, sizeof(genie));

	text.write(b, sizeof(long));
	sound.write(b, sizeof(long));
	if (num_sel > 0)
		b.write(uids, sizeof(uids));
}

void Effect::read(FILE *in)
{
	Genie_Effect genie;

	readbin(in, &genie);
	fromGenie(genie);

	text.read(in, sizeof(long));
	sound.read(in, sizeof(long));

    // this helped me find why there was heap corruption
    // && num_sel < 22
    // Need a bigger number for max selected units
	if (num_sel > 0)
		readbin(in, uids, num_sel);

    // DODGY! should be removed as soon as bug is fixed
	if ((long)trig_index < 0) {
	    trig_index = -1;
	}
}

void Effect::write(FILE *out)
{
	Genie_Effect genie = toGenie();
	writebin(out, &genie);

	text.write(out, sizeof(long));
	sound.write(out, sizeof(long));
	if (num_sel > 0)
		fwrite(uids, sizeof(long), num_sel, out);
}

std::string pluralizeUnitType(std::string unit_type_name) { 
    std::ostringstream convert;
    replaced(unit_type_name, ", Unpacked", "");
    if (!unit_type_name.empty() && *unit_type_name.rbegin() != 's' && !replaced(unit_type_name, "man", "men")) {
        convert << unit_type_name << "s";
    } else {
        convert << unit_type_name;
    }
    return convert.str();
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

std::string Effect::selectedUnits() const {
    std::ostringstream convert;
    if (s_player >= 0)
        convert << playerPronoun(s_player) << " ";
    if (num_sel > 1) {
        convert << pluralizeUnitType(unitTypeName(pUnit));
    } else {
        convert << unitTypeName(pUnit);
    }
	for (int i = 0; i < num_sel; i++) {
        convert << " " << uids[i] << " (" << get_unit_full_name(uids[i]) << ")";
	}
    return convert.str();
}

// functor used for getting a subset of an array of PlayerUnits
class playersunit_ucnst_equals
{
public:
	playersunit_ucnst_equals(UCNST cnst)
		: _cnst(cnst)
	{
	}

	bool operator()(const PlayersUnit& pu)
	{
	    bool sametype = false;
        if (pu.u) {
            const UnitLink * ul = pu.u->getType();
	        if (ul->id() == _cnst) {
	            sametype = true;
	        }
        }
		return (sametype);
	}

private:
	UCNST _cnst;
};

// functor used for getting a subset of an array of PlayerUnits
class playersunit_ucnst_notequals
{
public:
	playersunit_ucnst_notequals(UCNST cnst)
		: _cnst(cnst)
	{
	}

	bool operator()(const PlayersUnit& pu)
	{
	    bool difftype = false;
        if (pu.u) {
            const UnitLink * ul = pu.u->getType();
	        if (ul->id() != _cnst) {
	            difftype = true;
	        }
        }
		return (difftype);
	}

private:
	UCNST _cnst;
};

std::string Effect::getName(bool tip, NameFlags::Value flags) const
{
    if (!tip) {
	    return (type < scen.pergame->max_effect_types) ? getTypeName(type, false) : "Unknown!";
	} else {
        bool valid_area = !(area.left == -1 && area.right == -1 && area.top == -1 && area.bottom == -1);

        std::string stype = std::string("");
        std::ostringstream convert;
        switch (type) {
            case 0: // Undefined
                // Let this act like a separator
                convert << "                                                                                    ";
                stype.append(convert.str());
                break;
            case 1: // Change diplomacy
                convert << playerPronoun(s_player);
                switch (diplomacy) {
                case 0:
                    convert << " allies with ";
                    break;
                case 1:
                    convert << " becomes neutral to ";
                    break;
                case 2:
                    convert << " <INVALID DIPLOMACY> to ";
                    break;
                case 3:
                    convert << " declares war on ";
                    break;
                }
                convert << playerPronoun(t_player);
                stype.append(convert.str());
                break;
            case 2: // Research
                {
                    bool hastech = pTech && pTech->id();
                    std::wstring wtechname;
                    std::string techname;
                    if (hastech) {
                        wtechname = std::wstring(pTech->name());
                        techname = std::string(wtechname.begin(), wtechname.end());
                        if (panel >= 1 && panel <= 3) {
                        switch (panel) {
                        case 1:
                            convert << "enable " << playerPronoun(s_player) << " to research " << techname;
                            break;
                        case 2:
                            convert << "disable " << playerPronoun(s_player) << " to research " << techname;
                            break;
                        case 3:
                            convert << "enable " << playerPronoun(s_player) << " to research " << techname << " regardless of civ";
                            break;
                        }
                        } else {
                            convert << playerPronoun(s_player);
                            convert << " researches " << techname;
                        }
                    } else {
                        convert << "INVALID";
                    }
                }
                stype.append(convert.str());
                break;
            case 3: // Send chat
                switch (s_player) {
                    case -1:
                    case 0:
                        convert << "tell everyone";
                        break;
                    default:
                        convert << "tell p" << s_player;
                }
                convert << " \"" << text.c_str() << "\"";
                stype.append(convert.str());
                break;
            case 4: // Play sound
                convert << "play sound " << sound.c_str();
                stype.append(convert.str());
                break;
            case 5: // Tribute
                // what is the significance of losing 2147483647?
                // i guess it resets?
                if (amount == 1410065407) {
                    // reset to 0
                    convert << "reset ";
                    convert << playerPronoun(s_player) << "'s";
                } else if (amount >= 0) {
                    convert << playerPronoun(s_player);
                    if (t_player == 0) {
                        convert << " loses";
                    } else {
                        convert << " gives ";
                        convert << "p" << t_player;
                    }
                    convert << " " << amount << " ";
                } else {
                    if (t_player == 0) {
                        if (res_type < 4) {
                            convert << "p" << s_player << " silently gets " << -amount << " ";
                        } else {
                            convert << "p" << s_player << " gets " << -amount << " ";
                        }
                    } else {
                        convert << "p" << t_player << " silently gives ";
                        convert << playerPronoun(s_player);
                        convert << " " << -amount << " ";
                    }
                }
                switch (res_type) {
                    case 0: // Food
                        convert << "food";
                        break;
                    case 1: // Wood
                        convert << "wood";
                        break;
                    case 2: // Stone
                        convert << "stone";
                        break;
                    case 3: // Gold
                        convert << "gold";
                        break;
                    case 20: // Units killed
                        if (amount == 1) {
                            convert << "kill";
                        } else {
                            convert << "kills";
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
                                    convert << std::string( resname.begin(), resname.end());
                                    convert << " (Res " << res_type << ")";
		                            break;
		                        }
	                        }
	                    }
                        break;
                }
                if (amount == 1410065407) {
                    convert << " to 0";
                }
                if (amount >= 0 && t_player > 0) {
                    convert << " (displays tribute alert)";
                }
                stype.append(convert.str());
                break;
            case 8: // Activate
            case 9: // Deactivate trigger
                //stype.append(types_short[type]);
                switch (type) {
                    case 8:
                        stype.append("activate ");
                        break;
                    case 9:
                        stype.append("deactivate ");
                        break;
                }
                if (trig_index != (unsigned)-1 && trig_index != (unsigned)-2) {
                    if (trig_index < scen.triggers.size() && trig_index >= 0) {
                        if (setts.showdisplayorder) {
                            stype.append("<").append(toString(scen.triggers.at(trig_index).display_order)).append("> ");
                        }
                        // can't make recursion > 0 until I find a way
                        // to limit it
                        stype.append(scen.triggers.at(trig_index).getName(setts.pseudonyms,true,0));
                    } else {
                        stype.append("<?>");
                    }
                    //convert << trig_index;
                    //stype.append(convert.str());
                }
                break;
            case 10:
                switch (ai_goal) {
                default:
                    if (ai_goal >= -258 && ai_goal <= -3) {
                        // AI Shared Goal
                        convert << "complete AI shared goal " << ai_goal + 258;
                    } else if (ai_goal >= 774 && ai_goal <= 1029) {
	                    // Set AI Signal
                        convert << "signal AI " << ai_goal - 774;
                    } else {
                        convert << "AI signalled " << ai_goal;
                    }
                }
                stype.append(convert.str());
                break;;
            case 16: // Change view
                if (location.x >= 0 && location.y >= 0 && s_player >= 1) {
                    convert << "change view for p" << s_player << " to (" << location.x << ", " << location.y << ")";
                } else {
                    convert << "INVALID";
                }
                stype.append(convert.str());
                break;
            case 6: // Unlock Gate
            case 7: // Lock Gate
            case 14: // Kill Object
            case 15: // Remove
            case 17: // Unload
            case 22: // Freeze unit
                switch (type) {
                case 6:
                    convert << "unlock";
                    break;
                case 7:
                    convert << "lock";
                    break;
                case 14:
                    convert << "kill";
                    break;
                case 15:
                    convert << "remove";
                    break;
                case 17:
                    convert << "unload";
                    break;
                case 22:
                    convert << "freeze";
                    break;
                }
                convert << " " << selectedUnits();
                if (valid_area) {
                    if (area.left == area.right && area.top == area.bottom) {
                        convert << " at (" << area.left << "," << area.top << ")";
                    } else {
                        switch (type) {
                            case 6:
                            case 7:
                            case 22:
                                convert << " in area";
                                break;
                            case 14:
                                convert << " within";
                                break;
                            default:
                                convert << " from area";
                        }
                        convert << " (" << area.left << ", " << area.bottom << ") - (" << area.right << ", " << area.top << ")";
                    }
                }
                if (location.x >= 0 && location.y >= 0) {
                    convert << " at (" << location.x << ", " << location.y << ")";
                }
                stype.append(convert.str());
                break;
            case 29: // Stop unit
                if (panel >= 1 && panel <= 9) {
                    convert << "place" << " " << selectedUnits() << " into control group " << panel;
                } else {
                    convert << "Stop " << selectedUnits();
                    if (valid_area) {
                        if (area.left == area.right && area.top == area.bottom) {
                            convert << " at (" << area.left << "," << area.top << ")";
                        } else {
                            switch (type) {
                            case 6:
                            case 7:
                            case 22:
                                convert << " in";
                                break;
                            default:
                                convert << " from";
                            }
                            convert << " area (" << area.left << ", " << area.bottom << ") - (" << area.right << ", " << area.top << ")";
                        }
                    }
                    if (location.x >= 0 && location.y >= 0) {
                        convert << " at (" << location.x << ", " << location.y << ")";
                    }
                    stype.append(convert.str());
                    break;
                }
                stype.append(convert.str());
                break;
            case 11: // Create object
                convert << "create";
                convert << " " << playerPronoun(s_player);
                if (pUnit && pUnit->id()) {
                    std::wstring unitname(pUnit->name());
                    std::string un(unitname.begin(), unitname.end());
                    convert << " " << un;
                } else {
                    convert << " INVALID EFFECT";
                }
                convert << " at (" << location.x << ", " << location.y << ")";
                stype.append(convert.str());
                break;
            case 19: // Patrol object
                try
                {
                    // keep in mind multiple units can possess the same id, but
                    // it only operates on the last farm with that id.
                    //
                    // s_player may not be selected. Therefore, go through all
                    // the units in scenario.
                    //
                    // A gaia farm could be made infinite.
                    //Player * p = scen.players + s_player;
                    //for (vector<Unit>::const_iterator iter = p->units.begin(); iter != sorted.end(); ++iter) {
                    //}

                    if (num_sel > 0) {
                        std::vector<PlayersUnit> all (num_sel);
	                    for (int i = 0; i < num_sel; i++) {
                            all[i] = find_map_unit(uids[i]);
	                    }
                        std::vector<PlayersUnit> farms (num_sel);
                        std::vector<PlayersUnit> other (num_sel);

                        std::vector<PlayersUnit>::iterator it;
                        it = copy_if (all.begin(), all.end(), farms.begin(), playersunit_ucnst_equals(50) );
                        farms.resize(std::distance(farms.begin(),it));  // shrink container to new size

                        it = copy_if (all.begin(), all.end(), other.begin(), playersunit_ucnst_notequals(50) );
                        other.resize(std::distance(other.begin(),it));  // shrink container to new size

                        if (farms.size() > 0) {
                            convert << "reseed ";
                            for(std::vector<PlayersUnit>::iterator it = farms.begin(); it != farms.end(); ++it) {
                                convert << it->u->ident << " (" <<
                                    get_unit_full_name(it->u->ident)
                                    << ")" << " ";
                            }
	                    }

                        if (other.size() > 0) {
                            convert << "patrol ";
                            for(std::vector<PlayersUnit>::iterator it = other.begin(); it != other.end(); ++it) {
                                convert << it->u->ident << " (" <<
                                    get_unit_full_name(it->u->ident)
                                    << ")" << " ";
                            }
	                    }
	                } else {
                        convert << "patrol / reseed nothing";
	                }
                    stype.append(convert.str());
                }
	            catch (std::exception& ex)
	            {
                    stype.append(ex.what());
	            }
                break;
            case 18: // Change Ownership
            case 12: // Task object
                switch (type) {
                    case 18:
                        convert << "convert";
                        break;
                    default:
                        convert << "task";
                }
                convert << " " << selectedUnits();
                if (valid_area) {
                    if (area.left == area.right && area.top == area.bottom) {
                        convert << " at (" << area.left << "," << area.top << ")";
                    } else {
                        convert << " in (" << area.left << ", " << area.bottom << ") - (" << area.right << ", " << area.top << ")";
                    }
                }
                switch (type) {
                case 18:
                    convert << " to";
                    convert << " " << playerPronoun(t_player);
                    break;
                default:
                    if (location.x >= 0 && location.y >= 0) {
                        convert << " to (" << location.x << ", " << location.y << ")";
                    } else {
                        convert << " to unit " << uid_loc << " (" << get_unit_full_name(uid_loc) << ")";
                    }
                }
                stype.append(convert.str());
                break;
            case 13: // Declare victory
                convert << "p" << s_player << " victory";
                stype.append(convert.str());
                break;
            case 20: // Instructions
                convert << "instruct players \"" << text.c_str() << "\"";
                stype.append(convert.str());
                break;
            case 26: // Rename
                //stype.append(types_short[type]);
                stype.append("rename '");
                stype.append(text.c_str());
                stype.append("'");
                convert << " " << selectedUnits();
                if (valid_area) {
                    if (area.left == area.right && area.top == area.bottom) {
                        convert << " at (" << area.left << "," << area.top << ")";
                    } else {
                        convert << " over area (" << area.left << "," << area.bottom << ") - (" << area.right << ", " << area.top << ")";
                    }
                }
                stype.append(convert.str());
                break;
            case 24: // Damage
                {
                    std::string sunit("");
                    bool unit_set_selected = pUnit && pUnit->id(); // also use unit class and type
                    convert << playerPronoun(s_player) << "'s ";
	                if (num_sel > 0) {
	                    if (num_sel == 1) {
                            convert << "unit ";
                        } else {
                            convert << "units ";
                        }
	                    for (int i = 0; i < num_sel; i++) {
                            convert << uids[i] << " (" << get_unit_full_name(uids[i]) << ") ";
	                    }
	                    if (unit_set_selected) {
	                        convert << " and ";
	                    }
                    }
                    if (unit_set_selected) {
                        std::wstring unitname(pUnit->name());
                        std::string un(unitname.begin(), unitname.end());
                        if (!un.empty() && *un.rbegin() != 's' && !replaced(un, "man", "men")) {
                            convert << un << "s ";
                        } else {
                            convert << un << " ";
                        }
                    } else {
	                    if (num_sel <= 0) {
                            convert << "units ";
	                    }
                    }
                    if (unit_set_selected) {
                        if (valid_area) {
                            if (area.left == area.right && area.top == area.bottom) {
                                convert << "at (" << area.left << "," << area.top << ") ";
                            } else {
                                if (amount == -2147483647) {
                                    convert << "in ";
                                } else {
                                    convert << "from ";
                                }
                                convert << "area (" << area.left << "," << area.bottom << ") - (" << area.right << ", " << area.top << ") ";
                            }
                        }
                    }

                    // The above is setting up for the below

                    sunit.append(convert.str());
                    convert.str("");
                    convert.clear();
                    if (amount == -2147483647) {
                        convert << "make " << sunit << "invincible";
                    } else {
                        if (amount < 0) {
                            convert << "buff " << sunit << "with " << -amount << " HP";
                        } else {
                            convert << "damage " << sunit << "by " << amount << " HP";
                        }
                    }
                    stype.append(convert.str());
                }
                break;
            case 27: // HP
            case 28: // Attack
            case 30: // UP Speed
            case 31: // UP Range
            case 32: // UP Armor1
            case 33: // UP Armor2
                {
                    std::string sunit("");
                    bool unit_set_selected = pUnit && pUnit->id(); // also use unit class and type
                    convert << playerPronoun(s_player) << "'s ";
	                if (num_sel > 0) {
	                    if (num_sel == 1) {
                            convert << "unit ";
                        } else {
                            convert << "units ";
                        }
	                    for (int i = 0; i < num_sel; i++) {
                            convert << uids[i] << " (" << get_unit_full_name(uids[i]) << ") ";
	                    }
	                    if (unit_set_selected) {
	                        convert << " and ";
	                    }
                    }
                    if (unit_set_selected) {
                        convert << unitTypeName(pUnit) << " ";
                        if (valid_area) {
                            if (area.left == area.right && area.top == area.bottom) {
                                convert << "at (" << area.left << "," << area.top << ")";
                            } else {
                                convert << "in area (" << area.left << "," << area.bottom << ") - (" << area.right << ", " << area.top << ")";
                            }
                        }
                    } else {
	                    if (num_sel <= 0) {
                            convert << "units ";
	                    }
                    }

                    // The above is setting up for the below

                    sunit.append(convert.str());
                    convert.str("");
                    convert.clear();
                    if (amount > 0) {
                        convert << "+";
                    }
                    convert << amount << " " << getTypeName(type, true) << " to "  << sunit;
                    stype.append(convert.str());
                }
                break;
            default:
                stype.append((type < scen.pergame->max_effect_types) ? getTypeName(type, true) : "Unknown!");
        }

        return flags&NameFlags::LIMITLEN?stype.substr(0,100):stype;
    }
}

const char * Effect::getTypeName(size_t type, bool concise) const
{
	return concise?types_short[type]:types[type];
}

int Effect::getPlayer() const
{
	return s_player;
}

void Effect::setPlayer(int player)
{
	s_player = player;
}

/*
 * False positives are better than false negatives.
 */
bool Effect::check() const
{
	switch (type)
	{
	case EFFECT_ChangeDiplomacy:
		return (s_player >= 0 && t_player >= 0 && diplomacy >= 0);

	case EFFECT_ResearchTechnology:
		return (s_player >= 0 && pTech);

	case EFFECT_SendChat:
		return (s_player >= 0 && *text.c_str());	//AOK missing text check

	case EFFECT_PlaySound:
		return (s_player >= 0 && sound.length());	//AOK missing sound check

	case EFFECT_SendTribute:
		return (s_player >= 0 && t_player >= 0 && res_type >= 0);

	case EFFECT_UnlockGate:
	case EFFECT_LockGate:
		return (num_sel >= 0);

	case EFFECT_ActivateTrigger:
	case EFFECT_DeactivateTrigger:
		return (trig_index >= 0 && trig_index != (unsigned)-1 && trig_index != (unsigned)-2 && trig_index < scen.triggers.size());

	case EFFECT_AIScriptGoal:
		//return (s_player >= 0 && ai_goal >= 0);
		return true;

	case EFFECT_CreateObject:
		return (s_player >= 0 &&
			location.x >= 0 && location.y >= 0 && pUnit);

	case EFFECT_TaskObject:
	case EFFECT_KillObject:
	case EFFECT_RemoveObject:
	    //return (num_sel == 0 && area.left == -1 && area.right == -1 && area.top == -1 && area.bottom == -1) ||
	    //       (num_sel >= 0 && area.right >= area.left && area.bottom >= area.top);
	case EFFECT_FreezeUnit:
	case EFFECT_StopUnit:
		return true;

	case EFFECT_DeclareVictory:
		return (s_player >= 0);

	//EFFECT_KillObject, EFFECT_RemoveObject above.

	case EFFECT_ChangeView:
		return (s_player >= 0 && location.x >= 0 && location.y >= 0);

	case EFFECT_Unload:
		return (s_player >= 0
			&& (num_sel >= 0 || area.left >= 0 || utype >= 0)	//AOK missing this
			&& location.x >=0 && location.y >= 0);

	case EFFECT_ChangeOwnership:
		return true;
		//return (s_player >= 0 && t_player >= 0
		//	&& (num_sel >= 0 || area.left >= 0 || utype >= 0));	//AOK missing this

	case EFFECT_Patrol:
		return (num_sel >= 0 && location.x >= 0 && location.y >= 0);

	case EFFECT_DisplayInstructions:
	    // panel == -1 is acceptable because Azzzru's scripts omit panel
	    // to shorten SCX file and scenario still works fine.
		return (panel >= -1 && disp_time >= 0
			&& (*text.c_str() || textid));	//AOK missing text

	case EFFECT_ClearInstructions:
		return (panel >= 0);

	case EFFECT_UseAdvancedButtons:
		return true;	//no properties to set

	case EFFECT_DamageObject:
	case EFFECT_ChangeObjectHP:
	case EFFECT_ChangeObjectAttack:
		//return (amount != -1);		//amount can be negative, but can't be -1 (or can it? it appears red in scenario editor. It CAN be -1 (in AOHD at least)
		return true;
		//	&& (num_sel >= 0 || area.left >= 0));	//AOK missing this
	case EFFECT_SnapView: //Equal to UP Change Speed
		return true;
	case EFFECT_EnableTech: //Equal to UP Change Armor #1
		return true;
	case EFFECT_DisableTech: //Equal to UP Change Armor #2
		return true;
	case EFFECT_Unknown31: //Equal to UP Change Range
		return true;
		//return (amount != 0		//amount can be negative
		//	&& (num_sel >= 0 || area.left >= 0));	//AOK missing this

	case EFFECT_PlaceFoundation:
		return (s_player >= 0 && pUnit
			&& location.x >= 0 && location.y >= 0);

	case EFFECT_ChangeObjectName:
		return true;
		//return (num_sel >= 0);	//no text check
		//return (area.left >= 0);	//no text check

	//EFFECT_ChangeObjectHP, EFFECT_ChangeObjectAttack, EFFECT_StopUnit, and new UP effects above

	//EFFECT_SnapView shares with EFFECT_ChangeView above


		return (pTech != NULL);

	case EFFECT_EnableUnit:
	case EFFECT_DisableUnit:
		return (pUnit != NULL);

	case EFFECT_FlashObjects:
		return (area.left >= 0); //SWGB missing this

	default:
		return false;	//unknown effect type
	}
}

void Effect::accept(TriggerVisitor& tv)
{
	tv.visit(*this);
}

void Effect::fromGenie(const Genie_Effect& genie)
{
	if (genie.check != EFFECT)
		throw bad_data_error("Effect has incorrect check value.");

	type = genie.type;
	ttype = static_cast<TType>(genie.check);
	ai_goal = genie.ai_goal;
	amount = genie.amount;
	res_type = genie.resource;
	diplomacy = genie.diplomacy;
	num_sel = genie.num_selected;
	uid_loc = genie.uid_location;
	pUnit = esdata.units.getByIdSafe(genie.unit_constant);
	s_player = genie.player_source;
	t_player = genie.player_target;
	pTech = esdata.techs.getByIdSafe(genie.technology);
	textid = genie.textid;
	soundid = genie.soundid;
	disp_time = genie.display_time;
	trig_index = genie.trigger_index;
	//if (trig_index == (unsigned)-4 || trig_index == (unsigned)-5 || trig_index == (unsigned)-11) {
	//if (trig_index > 1000) {
	//    trig_index = (unsigned)-1;
	//}
	location = genie.location;
	area = genie.area;
	group = genie.unit_group;
	utype = genie.unit_type;
	panel = genie.panel;
}

Genie_Effect Effect::toGenie() const
{
	Genie_Effect ret =
	{
		type,
		ttype,
		ai_goal,
		amount,
		res_type,
		diplomacy,
		num_sel,
		uid_loc,
		(pUnit) ? pUnit->id() : -1,
		s_player,
		t_player,
		(pTech) ? pTech->id() : -1,
		textid,
		soundid,
		disp_time,
		trig_index,
		location,
		area,
		group,
		utype,
		panel
	};

	return ret;
}

const char *Effect::types_aok[] = {
	"",
	"Change Diplomacy",
	"Research Technology",
	"Send Chat",
	"Play Sound",
	"Send Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate Trigger",
	"Deactivate Trigger",
	"AI Script Goal",
	"Create Object",
	"Task Object",
	"Declare Victory",
	"Kill Object",
	"Remove Object",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol Units / Reseed Farms",
	"Display Instructions",
	"Clear Instructions",
	"Freeze Unit",
	"Use Advanced Buttons"
};

const char *Effect::types_aoc[] = {
	"",
	"Change Diplomacy",
	"Research Technology",
	"Send Chat",
	"Play Sound",
	"Send Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate Trigger",
	"Deactivate Trigger",
	"AI Script Goal",
	"Create Object",
	"Task Object",
	"Declare Victory",
	"Kill Object",
	"Remove Object",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol Units / Reseed Farms",
	"Display Instructions",
	"Clear Instructions",
	"Freeze Unit",
	"Use Advanced Buttons",
	"Damage Object",
	"Place Foundation",
	"Change Object Name",
	"Change Object HP",
	"Change Object Attack",
	"Stop Unit"
};

const char *Effect::types_up[] = {
	"",
	"Change Diplomacy",
	"Research Technology",
	"Send Chat",
	"Play Sound",
	"Send Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate Trigger",
	"Deactivate Trigger",
	"AI Script Goal",
	"Create Object",
	"Task Object",
	"Declare Victory",
	"Kill Object",
	"Remove Object",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol Units / Reseed Farms",
	"Display Instructions",
	"Clear Instructions",
	"Freeze Unit",
	"Use Advanced Buttons",
	"Damage Object",
	"Place Foundation",
	"Change Object Name",
	"Change Object HP",
	"Change Object Attack",
	"Stop Unit",
	"Change Speed",
	"Change Range",
	"Change Mele Armor",
	"Change Piercing Armor"
};

const char *Effect::types_swgb[] = {
	"",
	"Change Alliance",
	"Research Technology",
	"Send Chat",
	"Play Sound",
	"Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate Trigger",
	"Deactivate Trigger",
	"AI Script Goal",
	"Create Object",
	"Task Object",
	"Declare Victory",
	"Kill Object",
	"Remove Object",
	"Scroll View",
	"Unload",
	"Change Ownership",
	"Patrol Units",
	"Display Instructions",
	"Clear Instructions",
	"Freeze Unit",
	"Enable Advanced Buttons",
	"Damage Object",
	"Place Foundation",
	"Change Object Name",
	"Change Object HP",
	"Change Object Attack",
	"Stop Unit",
	"Snap View",
	"Disable Advanced Buttons",
	"Enable Tech",
	"Disable Tech",
	"Enable Unit",
	"Disable Unit",
	"Flash Objects"
};

const char *Effect::types_cc[] = {
	"",
	"Change Alliance",
	"Research Technology",
	"Send Chat",
	"Play Sound",
	"Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate Trigger",
	"Deactivate Trigger",
	"AI Script Goal",
	"Create Object",
	"Task Object",
	"Declare Victory",
	"Kill Object",
	"Remove Object",
	"Scroll View",
	"Unload",
	"Change Ownership",
	"Patrol Units",
	"Display Instructions",
	"Clear Instructions",
	"Freeze Unit",
	"Enable Advanced Buttons",
	"Damage Object",
	"Place Foundation",
	"Change Object Name",
	"Change Object HP",
	"Change Object Attack",
	"Stop Unit",
	"Snap View",
	"Disable Advanced Buttons",
};

const char *Effect::types_aohd[] = {
	"",
	"Change Diplomacy",
	"Research Technology",
	"Send Chat",
	"Play Sound",
	"Send Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate Trigger",
	"Deactivate Trigger",
	"AI Script Goal",
	"Create Object",
	"Task Object",
	"Declare Victory",
	"Kill Object",
	"Remove Object",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol Units / Reseed Farms",
	"Display Instructions",
	"Clear Instructions",
	"Freeze Unit",
	"Use Advanced Buttons",
	"Damage Object",
	"Place Foundation",
	"Change Object Name",
	"Change Object HP",
	"Change Object Attack",
	"Stop Unit",
	"Attack-Move",
	"Change Armor",
	"Change Range",
	"Change Speed",
};

const char *Effect::types_aof[] = {
	"",
	"Change Diplomacy",
	"Research Technology",
	"Send Chat",
	"Play Sound",
	"Send Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate Trigger",
	"Deactivate Trigger",
	"AI Script Goal",
	"Create Object",
	"Task Object",
	"Declare Victory",
	"Kill Object",
	"Remove Object",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol Units / Reseed Farms",
	"Display Instructions",
	"Clear Instructions",
	"Freeze Unit",
	"Use Advanced Buttons",
	"Damage Object",
	"Place Foundation",
	"Change Object Name",
	"Change Object HP",
	"Change Object Attack",
	"Stop Unit",
	"Attack-Move",
	"Change Armor",
	"Change Range",
	"Change Speed",
};

const char *Effect::types_short_aok[] = {
	"",
	"Change Diplomacy",
	"Research",
	"Chat",
	"Sound",
	"Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate",
	"Deactivate",
	"AI Script Goal",
	"Create",
	"Task",
	"Declare Victory",
	"Kill",
	"Remove",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol / Reseed",
	"Instructions",
	"Clear Instructions",
	"Freeze",
	"Use Advanced Buttons"
};

const char *Effect::types_short_aoc[] = {
	"",
	"Change Diplomacy",
	"Research",
	"Chat",
	"Sound",
	"Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate",
	"Deactivate",
	"AI Script Goal",
	"Create",
	"Task",
	"Declare Victory",
	"Kill",
	"Remove",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol / Reseed",
	"Instructions",
	"Clear Instructions",
	"Freeze",
	"Use Advanced Buttons",
	"Damage",
	"Place Foundation",
	"Rename",
	"HP",
	"Attack",
	"Stop Unit"
};

const char *Effect::types_short_up[] = {
	"",
	"Change Diplomacy",
	"Research",
	"Chat",
	"Sound",
	"Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate",
	"Deactivate",
	"AI Script Goal",
	"Create",
	"Task",
	"Declare Victory",
	"Kill",
	"Remove",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol / Reseed",
	"Instructions",
	"Clear Instructions",
	"Freeze",
	"Use Advanced Buttons",
	"Damage",
	"Place Foundation",
	"Rename",
	"HP",
	"Attack",
	"Stop Unit",
	"Speed",
	"Range",
	"Mele Armor",
	"Piercing Armor"
};

const char *Effect::types_short_aohd[] = {
	"",
	"Change Diplomacy",
	"Research",
	"Chat",
	"Sound",
	"Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate",
	"Deactivate",
	"AI Script Goal",
	"Create",
	"Task",
	"Declare Victory",
	"Kill",
	"Remove",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol / Reseed",
	"Instructions",
	"Clear Instructions",
	"Freeze",
	"Use Advanced Buttons",
	"Damage",
	"Place Foundation",
	"Rename",
	"HP",
	"Attack",
	"Stop Unit",
	"Attack-Move",
	"Armor",
	"Range",
	"Speed"
};

const char *Effect::types_short_aof[] = {
	"",
	"Change Diplomacy",
	"Research",
	"Chat",
	"Sound",
	"Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate",
	"Deactivate",
	"AI Script Goal",
	"Create",
	"Task",
	"Declare Victory",
	"Kill",
	"Remove",
	"Change View",
	"Unload",
	"Change Ownership",
	"Patrol / Reseed",
	"Instructions",
	"Clear Instructions",
	"Freeze",
	"Use Advanced Buttons",
	"Damage",
	"Place Foundation",
	"Rename",
	"HP",
	"Attack",
	"Stop Unit",
	"Attack-Move",
	"Armor",
	"Range",
	"Speed"
};

const char *Effect::types_short_swgb[] = {
	"",
	"Change Alliance",
	"Research",
	"Chat",
	"Sound",
	"Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate",
	"Deactivate",
	"AI Script Goal",
	"Create",
	"Task",
	"Declare Victory",
	"Kill",
	"Remove",
	"Scroll View",
	"Unload",
	"Change Ownership",
	"Patrol",
	"Instructions",
	"Clear Instructions",
	"Freeze",
	"Use Advanced Buttons",
	"Damage",
	"Place Foundation",
	"Rename",
	"HP",
	"Attack",
	"Stop Unit",
	"Snap View",
	"Disable Advanced",
	"Enable Tech",
	"Disable Tech",
	"Enable Unit",
	"Disable Unit",
	"Flash"
};

const char *Effect::types_short_cc[] = {
	"",
	"Change Alliance",
	"Research",
	"Chat",
	"Sound",
	"Tribute",
	"Unlock Gate",
	"Lock Gate",
	"Activate",
	"Deactivate",
	"AI Script Goal",
	"Create",
	"Task",
	"Declare Victory",
	"Kill",
	"Remove",
	"Scroll View",
	"Unload",
	"Change Ownership",
	"Patrol",
	"Instructions",
	"Clear Instructions",
	"Freeze",
	"Use Advanced Buttons",
	"Damage",
	"Place Foundation",
	"Rename",
	"HP",
	"Attack",
	"Stop Unit",
	"Snap View",
	"Disable Advanced",
	"Enable Tech",
	"Disable Tech",
	"Enable Unit",
	"Disable Unit",
	"Flash",
	"Input Off",
	"Input On"
};

const char *Effect::virtual_types_up[] = {
    "None",
    "Enable Object",
    "Disable Object",
    "Enable Technology",
    "Disable Technology",
    "Enable Tech (Any Civ)",
    "Set HP",
    "Heal Object",
    "Aggressive Stance",
    "Defensive Stance",
    "Stand Ground",
    "No Attack Stance",
    "Resign",
    "Flash Objects",
    "Set AP",
    "Set Control Group 1",
    "Set Control Group 2",
    "Set Control Group 3",
    "Set Control Group 4",
    "Set Control Group 5",
    "Set Control Group 6",
    "Set Control Group 7",
    "Set Control Group 8",
    "Set Control Group 9",
    "Snap View",
    "Set AI Signal",
    "Set AI Shared Goal"
};

const char *Effect::virtual_types_aoc[] = {
    "None",
    "Set AI Signal",
    "Set AI Shared Goal",
    "Freeze unit",
};

const char *Effect::virtual_types_aohd[] = {
    "None",
    "Freeze unit",
};

const char *Effect::virtual_types_aof[] = {
    "None",
    "Freeze unit",
};

const char *Effect::virtual_types_swgb[] = {
    "None",
    "Freeze unit"
};

const char *Effect::virtual_types_cc[] = {
    "None",
    "Freeze unit"
};

const char *Effect::virtual_types_aok[] = {
    "None"
};

const char** Effect::types;
const char** Effect::types_short;
const char** Effect::virtual_types;
