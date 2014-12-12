#include "Effect.h"
#include <sstream>
#include "scen.h"
#include "TriggerVisitor.h"

#include "../util/utilio.h"
#include "../view/utilui.h"
#include "../util/Buffer.h"
#include "../util/helper.h"

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

	if (num_sel > 0)
		readbin(in, uids, num_sel);
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

std::string Effect::getName() const
{
	return (type < NUM_EFFECTS) ? types[type] : "Unknown!";
}

std::string Effect::getNameTip() const
{
    std::string stype = std::string("");
    std::ostringstream convert;
    switch (type) {
    case 2: // Research
        if (s_player == 0) {
            convert << "Gaia";
        } else {
            convert << "p" << s_player;
        }
        convert << " gets ";
        if (pTech && pTech->id()) {
            std::wstring techname(pTech->name());
            convert << std::string( techname.begin(), techname.end());
        }
        convert << " technology";
        stype.append(convert.str());
        break;
    case 3: // Send chat
        convert << "tell ";
        if (s_player == 0) {
            convert << "Gaia";
        } else {
            convert << "p" << s_player;
        }
        convert << " \"" << text.c_str() << "\"";
        stype.append(convert.str());
        break;
    case 5: // Tribute
        // what is the significance of losing 2147483647
        // i guess it resets?
        if (amount == 1410065407) {
            // reset to 0
            convert << "Reset ";
            if (s_player == 0) {
                convert << "Gaia" << "'s ";
            } else {
                convert << "p" << s_player << "'s ";
            }
        } else if (amount >= 0) {
            if (s_player == 0) {
                convert << "Gaia";
            } else {
                convert << "p" << s_player;
            }
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
                if (s_player == 0) {
                    convert << "Gaia";
                } else {
                    convert << "p" << s_player;
                }
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
                        convert << "(res_type " << res_type << ")";
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
            convert << " (sends tribute alert)";
        }
        stype.append(convert.str());
        break;
    case 8: // Activate
    case 9: // Deactivate trigger
        stype.append(types_short[type]);
        stype.append(" ");
        if (trig_index != (unsigned)-1 && trig_index != (unsigned)-2) {
            stype.append(scen.triggers.at(trig_index).name).append(" <").append(toString(scen.triggers.at(trig_index).display_order)).append(">");
            //convert << trig_index;
            //stype.append(convert.str());
        }
        break;
    case 16: // Change view
        if (location.x >= 0 && location.y >= 0 && s_player >= 1) {
            convert << "Change view for p" << s_player << " to (" << location.x << ", " << location.y << ")";
        } else {
            convert << "INVALID";
        }
        stype.append(convert.str());
        break;
    case 14: // Kill Object
    case 15: // Remove
        if (type == 14 ) {
            convert << "kill ";
        } else {
            convert << "remove ";
        }
        if (s_player == 0) {
            convert << "Gaia";
        } else {
            convert << "p" << s_player << "'s";
        }
        if (pUnit && pUnit->id()) {
            std::wstring unitname(pUnit->name());
            std::string un(unitname.begin(), unitname.end());
            if (!un.empty() && *un.rbegin() != 's' && !replaced(un, "man", "men")) {
                convert << " " << un << "s";
            } else {
                convert << " " << un;
            }
        } else {
            convert << " units";
        }
        if (!(area.left == -1 && area.right == -1 && area.top == -1 && area.bottom == -1)) {
            if (area.left == area.right && area.top == area.bottom) {
                convert << " at (" << area.left << "," << area.top << ")";
            } else {
                convert << " from area (" << area.left << ", " << area.top << ") - (" << area.right << ", " << area.bottom << ")";
            }
        }
        stype.append(convert.str());
        break;
    case 11: // Create object
        convert << "create ";
        if (s_player == 0) {
            convert << "Gaia";
        } else {
            convert << "p" << s_player;
        }
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
    case 12: // Task object
        convert << "task ";
        if (s_player == 0) {
            convert << "Gaia";
        } else {
            convert << "p" << s_player;
        }
        if (pUnit && pUnit->id()) {
            std::wstring unitname(pUnit->name());
            std::string un(unitname.begin(), unitname.end());
            convert << " " << un;
        } else {
            convert << " unit";
        }
        convert << " from area (" << area.left << "," << area.top << ") - (" << area.right << ", " << area.bottom << ")";
        convert << " to (" << location.x << ", " << location.y << ")";
        stype.append(convert.str());
        break;
    case 18: // Change Ownership
        stype.append(types_short[type]);
        stype.append(" ");
        if (pUnit && pUnit->id()) {
            std::wstring unitname(pUnit->name());
            stype.append(std::string( unitname.begin(), unitname.end()));
        }
        break;
    case 13: // Declare victory
        stype.append(types_short[type]);
        stype.append(" ");
        convert << s_player;
        stype.append(convert.str());
        break;
    case 20: // Instructions
        convert << "Instruct players \"" << text.c_str() << "\"";
        stype.append(convert.str());
        break;
    case 26: // Rename
        stype.append(types_short[type]);
        stype.append(" ");
        stype.append(text.c_str());
        break;
    case 24: // Damage
        {
            std::string sunit("");
            bool unit_set_selected = pUnit && pUnit->id(); // also use unit class and type
            if (s_player > 0) {
                convert << "p" << s_player << "'s ";
            }
	        if (num_sel > 0) {
	            if (num_sel == 1) {
                    convert << "unit ";
                } else {
                    convert << "units ";
                }
	            for (int i = 0; i < num_sel; i++) {
                    convert << uids[i] << " ";
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
                if (!(area.left == -1 && area.right == -1 && area.top == -1 && area.bottom == -1)) {
                    if (area.left == area.right && area.top == area.bottom) {
                        convert << "at (" << area.left << "," << area.top << ")";
                    } else {
                        convert << "from area (" << area.left << "," << area.top << ") - (" << area.right << ", " << area.bottom << ")";
                    }
                }
            }
            sunit.append(convert.str());
            convert.str("");
            convert.clear();
            if (amount == -2147483647) {
                convert << "Make " << sunit << " invincible";
            } else {
                if (amount < 0) {
                    convert << "Buff " << sunit << " with " << -amount << " HP";
                } else {
                    convert << "Damage " << sunit << " by " << amount << " HP";
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
        convert << "Change ";
        convert << types_short[type];
        convert << " by ";
        convert << amount;
        stype.append(convert.str());
        break;
    default:
        stype.append((type < NUM_EFFECTS) ? types_short[type] : "Unknown!");
    }

	return stype;
}

int Effect::getPlayer() const
{
	return s_player;
}

void Effect::setPlayer(int player)
{
	s_player = player;
}

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
		return (trig_index >= 0 && trig_index != (unsigned)-1 && trig_index != (unsigned)-2);

	case EFFECT_AIScriptGoal:
		return (s_player >= 0 && ai_goal >= 0);

	case EFFECT_CreateObject:
		return (s_player >= 0 &&
			location.x >= 0 && location.y >= 0 && pUnit);

	case EFFECT_TaskObject:
	case EFFECT_KillObject:
	case EFFECT_RemoveObject:
		return true;
	case EFFECT_FreezeUnit:
	case EFFECT_StopUnit:
		return (num_sel >= 0 || area.left >= 0 || utype >= 0);	//AOK missing this

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

	//EFFECT_FreezeUnit above

	case EFFECT_UseAdvancedButtons:
		return true;	//no properties to set

	case EFFECT_DamageObject:
	case EFFECT_ChangeObjectHP:
	case EFFECT_ChangeObjectAttack:
		return true;
		//return (amount != 0		//amount can be negative
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

const char *Effect::types[] =
{
	"Undefined",
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
	"Patrol",
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
	"Change UP Speed - HD Attack-Move",
	"Change UP Range - HD Armor",
	"Change UP Armor1 - HD Range",
	"Change UP Armor2 - HD Speed",
	"Enable Unit",
	"Disable Unit",
	"Flash Objects"
};

const char *Effect::types_short[] =
{
	"Undefined",
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
	"Speed - HD Attack-Move",
	"Range - HD Armor",
	"Armor1 - HD Range",
	"Armor2 - HD Speed",
	"Enable Unit",
	"Disable Unit",
	"Flash Objects"
};
