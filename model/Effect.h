#ifndef AOKTS_EFFECT_H
#define AOKTS_EFFECT_H

#include "ECBase.h"

// Fwd declaration, don't want to #include it.
class TriggerVisitor;

#define NUM_EFFECTS	37	//+1 for undefined
#define EFFECT_CONTROLS	23
#define NUM_VIRTUAL_EFFECTS	24

// Need to enforce these
// AoC
//#define MAX_UNITSEL 22
// UP
//#define MAX_UNITSEL 40
// HD
#define MAX_UNITSEL 60

enum EffectVirtualType
{
	EFFECT_VIRTUAL_None,
	EFFECT_VIRTUAL_EnableObject,
	EFFECT_VIRTUAL_DisableObject,
	EFFECT_VIRTUAL_EnableTechnology,
	EFFECT_VIRTUAL_DisableTechnology,
	EFFECT_VIRTUAL_EnableTechnologyAnyCiv,
	EFFECT_VIRTUAL_SetHP,
	EFFECT_VIRTUAL_HealObject,
	EFFECT_VIRTUAL_SetAggressive,
	EFFECT_VIRTUAL_SetDefensive,
	EFFECT_VIRTUAL_SetStandGround,
	EFFECT_VIRTUAL_SetNoAttackWithoutHalt,
	EFFECT_VIRTUAL_Resign,
	EFFECT_VIRTUAL_FlashObjects,
	EFFECT_VIRTUAL_SetAP,
	EFFECT_VIRTUAL_SetControlGroup1,
	EFFECT_VIRTUAL_SetControlGroup2,
	EFFECT_VIRTUAL_SetControlGroup3,
	EFFECT_VIRTUAL_SetControlGroup4,
	EFFECT_VIRTUAL_SetControlGroup5,
	EFFECT_VIRTUAL_SetControlGroup6,
	EFFECT_VIRTUAL_SetControlGroup7,
	EFFECT_VIRTUAL_SetControlGroup8,
	EFFECT_VIRTUAL_SetControlGroup9
};

enum EffectType
{
	EFFECT_None,
	EFFECT_ChangeDiplomacy,
	EFFECT_ResearchTechnology,
	EFFECT_SendChat,
	EFFECT_PlaySound,
	EFFECT_SendTribute,
	EFFECT_UnlockGate,
	EFFECT_LockGate,
	EFFECT_ActivateTrigger,
	EFFECT_DeactivateTrigger,
	EFFECT_AIScriptGoal,
	EFFECT_CreateObject,
	EFFECT_TaskObject,
	EFFECT_DeclareVictory,
	EFFECT_KillObject,
	EFFECT_RemoveObject,
	EFFECT_ChangeView,
	EFFECT_Unload,
	EFFECT_ChangeOwnership,
	EFFECT_Patrol,
	EFFECT_DisplayInstructions,
	EFFECT_ClearInstructions,
	EFFECT_FreezeUnit,
	EFFECT_UseAdvancedButtons,
	EFFECT_DamageObject,
	EFFECT_PlaceFoundation,
	EFFECT_ChangeObjectName,
	EFFECT_ChangeObjectHP,
	EFFECT_ChangeObjectAttack,
	EFFECT_StopUnit,

	/* SWGB-only after here */
	EFFECT_SnapView, //Equal to UP Change Speed
	EFFECT_Unknown31, //Equal to UP Change Range
	EFFECT_EnableTech, //Equal to UP Change Armor #1
	EFFECT_DisableTech, //Equal to UP Change Range #2
	EFFECT_EnableUnit,
	EFFECT_DisableUnit,
	EFFECT_FlashObjects
};

class Effect : public ECBase
{
public:
	Effect();
//	~Effect();
//    Effect( const Effect& other );
	Effect(Buffer&); // reads back data written by tobuffer()

	// rule of three: default copy constructor, destructor, assignment fine

	void tobuffer(Buffer &b) const;
	void read(FILE *in);
	void write(FILE *out);

	int getPlayer() const;
	const char * getTypeName(size_t type, bool concise = false) const;
	void setPlayer(int);
	bool check() const;

	std::string getName(bool tip = false, NameFlags::Value flag=NameFlags::NONE) const;

	/**
	 * Accepts a TriggerVisitor.
	 */
	void accept(TriggerVisitor&);

	long ai_goal;
	long amount;
	long res_type;
	long diplomacy;
	long num_sel;
	UID uid_loc;	//the selected location unit
	const UnitLink *pUnit;	//long in file, of course
	long s_player;	// GAIA = 0, Player 1 = 1, ...
	long t_player;	// GAIA = 0, Player 1 = 1, ...
	const TechLink *pTech;	//long in file, of course
	long textid;
	long soundid;
	long disp_time;
	unsigned long trig_index;
	AOKPT location;
	AOKRECT area;
	long group;	//unit group
	long utype;
	long panel;

	SString text;
	SString sound;

#pragma pack(push, 4)	// we read these straight from the file
	UID uids[MAX_UNITSEL];		//array of selected units
#pragma pack(pop)

	static const char* types[NUM_EFFECTS];
	static const char* types_aohd[NUM_EFFECTS];
	static const char* types_short[NUM_EFFECTS];
	static const char* types_short_aohd[NUM_EFFECTS];
	static const char* virtual_types[NUM_VIRTUAL_EFFECTS];

private:
	void fromGenie(const struct Genie_Effect&);
	struct Genie_Effect toGenie() const;
};

#endif // AOKTS_EFFECT_H
