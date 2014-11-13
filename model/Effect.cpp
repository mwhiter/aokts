#include "Effect.h"
#include <sstream>
#include "scen.h"
#include "TriggerVisitor.h"

#include "../util/utilio.h"
#include "../util/Buffer.h"

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

	if (type > MAX_EFFECT)
		printf("WARNING: Unknown effect %d.\n", type);
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
    std::string stype = std::string((type < NUM_EFFECTS) ? types_short[type] : "Unknown!");
    if (type == 8 || type == 9) {
        if (trig_index >= 0 && trig_index != (unsigned)-1 && trig_index != (unsigned)-2) {
            stype.append(": ");
            stype.append(scen.triggers.at(trig_index).name);
            //std::ostringstream convert;
            //convert << trig_index;
            //stype.append(convert.str());
        }
    }
    if (type == 13) {
        stype.append(": ");
        std::ostringstream convert;
        convert << s_player;
        stype.append(convert.str());
    }
    if (type == 5 || type == 24 || type == 27 || type == 28 || type == 30 || type == 31 || type == 32 || type == 33) {
        stype.append(": ");
        std::ostringstream convert;
        convert << amount;
        stype.append(convert.str());
    }
    if (type == 2) {
        if (pTech && pTech->id()) {
            stype.append(": ");
            std::wstring techname(pTech->name());
            stype.append(std::string( techname.begin(), techname.end()));
        }
    }
    if (type == 3 || type == 20 || type == 26) {
        stype.append(": ");
        stype.append(text.c_str());
    }
    // create object
    if (type == 11 || type == 14 || type == 15 || type == 18) {
        //if ((pUnit && pUnit->id())) {
            //if (utype >= 0) {
            //    std::ostringstream convert;
            //    convert << pUnit->id();
            //    stype.append(convert.str());
            //}
            if (pUnit && pUnit->id()) {
                stype.append(": ");
                std::wstring unitname(pUnit->name());
                stype.append(std::string( unitname.begin(), unitname.end()));
                //std::ostringstream convert;
                //convert << pUnit->id();
                //stype.append(convert.str());
            }
        //}
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
		return (panel >= 0 && disp_time >= 0
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

	if (genie.type >= NUM_EFFECTS) // MAX_EFFECTS = NUM_EFFECTS - 1
		printf("WARNING: Unknown effect %d.\n", genie.type);

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
	"Ch UP Speed - HD Attack-Move",
	"Ch UP Range - HD Armor",
	"Ch UP Armor1 - HD Range",
	"Ch UP Armor2 - HD Speed",
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
