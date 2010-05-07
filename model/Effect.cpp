#include "Effect.h"
#include "TriggerVisitor.h"

#include "../util/utilio.h"

#define EFFECT_MEMBERS 22	//count of all the static members of class Effect

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
	stringid(-1),
	u2(-1),
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

Effect::Effect(Buffer &buffer)
:	ECBase(EFFECT)
{
	buffer.read(&ttype, sizeof(long));
	buffer.read(&type, sizeof(long));
	buffer.read(&ai_goal, 6 * sizeof(long));
	pUnit = static_cast<const UnitLink*>(readLink(buffer, esdata.units));
	buffer.read(&s_player, 2 * sizeof(long));
	pTech = static_cast<const TechLink*>(readLink(buffer, esdata.techs));
	buffer.read(&stringid, 13 * sizeof(long));
	buffer.reads(text, sizeof(long));
	buffer.reads(sound, sizeof(long));
	if (num_sel > 0)
		buffer.read(uids, sizeof(uids));
}

void Effect::tobuffer(Buffer &buffer) const
{
	buffer.write(&ttype, sizeof(long));
	buffer.write(&type, sizeof(long));
	buffer.write(&ai_goal, 6 * sizeof(long));
	writeLink(buffer, pUnit);
	buffer.write(&s_player, 2 * sizeof(long));
	writeLink(buffer, pTech);
	buffer.write(&stringid, 13 * sizeof(long));
	buffer.writes(text, 4);
	buffer.writes(sound, sizeof(long));
	if (num_sel > 0)
		buffer.write(uids, sizeof(uids));
}

void Effect::set(const Effect &e)
{
	type = e.type;
	memcpy(&ai_goal, &e.ai_goal, EFFECT_MEMBERS * sizeof(long));
	text.set(e.text.c_str());
	sound.set(e.sound.c_str());
	if (e.num_sel > 0)
		memcpy(uids, &e.uids, sizeof(uids));

	pTech = e.pTech;
	pUnit = e.pUnit;
}

// An effect as stored in in scenario, up to strings
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
	long stable;         // string table for text
	long unknown;
	long display_time;
	long trigger_index;
	AOKPT location;
	AOKRECT area;
	long unit_group;
	long unit_type;
	long panel;
};

void Effect::read(FILE *in)
{
	long temp;

	// FIXME: read as struct
	readbin(in, &type, 8);
	readbin(in, &temp);
	if (temp != -1)
		pUnit = static_cast<const UnitLink*>(getById(esdata.units, temp));
	readbin(in, &s_player, 2);
	readbin(in, &temp);
	if (temp != -1)
		pTech = static_cast<const TechLink*>(getById(esdata.techs, temp));
	readbin(in, &stringid, 13);

	if (ttype != EFFECT)
		throw bad_data_error("Effect has incorrect check value.");

	text.read(in, sizeof(long));
	sound.read(in, sizeof(long));

	if (num_sel > 0)
		readbin(in, uids, num_sel);

	if (type > MAX_EFFECT)
		printf("WARNING: Unknown effect %d.\n", type);
}

void Effect::write(FILE *out)
{
	long temp;

	fwrite(&type, sizeof(long), 8, out);
	temp = (pUnit) ? pUnit->id() : -1;
	fwrite(&temp, sizeof(long), 1, out);
	fwrite(&s_player, sizeof(long), 2, out);
	temp = (pTech) ? pTech->id() : -1;
	fwrite(&temp, sizeof(long), 1, out);
	fwrite(&stringid, sizeof(long), 13, out);
	text.write(out, sizeof(long));
	sound.write(out, sizeof(long));
	if (num_sel > 0)
		fwrite(uids, sizeof(long), num_sel, out);
}

int Effect::size() const
{
	int ret = 26 * sizeof(long) + sizeof(sound) + sizeof(uids);
	ret += text.total_size();
	return ret;
}

std::string Effect::getName() const
{
	return (type < NUM_EFFECTS) ? types[type] : "Unknown!";
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
		return (trig_index >= 0);

	case EFFECT_AIScriptGoal:
		return (s_player >= 0 && ai_goal >= 0);

	case EFFECT_CreateObject:
		return (s_player >= 0 &&
			location.x >= 0 && location.y >= 0 && pUnit);

	case EFFECT_TaskObject:
	case EFFECT_KillObject:
	case EFFECT_RemoveObject:
	case EFFECT_FreezeUnit:
	case EFFECT_StopUnit:
		return (num_sel >= 0 || area.left >= 0 || utype >= 0);	//AOK missing this

	case EFFECT_DeclareVictory:
		return (s_player >= 0);

	//EFFECT_KillObject, EFFECT_RemoveObject above.

	case EFFECT_ChangeView:
	case EFFECT_SnapView:
		return (s_player >= 0 && location.x >= 0 && location.y >= 0);

	case EFFECT_Unload:
		return (s_player >= 0
			&& (num_sel >= 0 || area.left >= 0 || utype >= 0)	//AOK missing this
			&& location.x >=0 && location.y >= 0);

	case EFFECT_ChangeOwnership:
		return (s_player >= 0 && t_player >= 0
			&& (num_sel >= 0 || area.left >= 0 || utype >= 0));	//AOK missing this

	case EFFECT_Patrol:
		return (num_sel >= 0 && location.x >= 0 && location.y >= 0);

	case EFFECT_DisplayInstructions:
		return (panel >= 0 && disp_time >= 0
			&& (*text.c_str() || stringid));	//AOK missing text

	case EFFECT_ClearInstructions:
		return (panel >= 0);

	//EFFECT_FreezeUnit above

	case EFFECT_UseAdvancedButtons:
		return true;	//no properties to set

	case EFFECT_DamageObject:
	case EFFECT_ChangeObjectHP:
	case EFFECT_ChangeObjectAttack:
		return (amount != 0		//amount can be negative
			&& (num_sel >= 0 || area.left >= 0));	//AOK missing this

	case EFFECT_PlaceFoundation:
		return (s_player >= 0 && pUnit
			&& location.x >= 0 && location.y >= 0);

	case EFFECT_ChangeObjectName:
		return (num_sel >= 0);	//no text check

	//EFFECT_ChangeObjectHP, EFFECT_ChangeObjectAttack, EFFECT_StopUnit above

	//EFFECT_SnapView shares with EFFECT_ChangeView above

	case EFFECT_EnableTech:
	case EFFECT_DisableTech:
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
	"Snap View",
	"Unknown (31)",
	"Enable Tech",
	"Disable Tech",
	"Enable Unit",
	"Disable Unit",
	"Flash Objects"
};
