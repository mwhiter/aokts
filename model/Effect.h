#ifndef AOKTS_EFFECT_H
#define AOKTS_EFFECT_H

#include "ECBase.h"

// Fwd declaration, don't want to #include it.
class TriggerVisitor;

#define NUM_EFFECTS	37	//+1 for undefined
#define MAX_UNITSEL 22

#pragma pack(push, 4)	//everything in effects/conditions is a long

/*	AOK points are y,x apparently (or at least the way
	I'm seeing them). */
struct AOKPT
{
	long y, x;
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
	EFFECT_SnapView,
	EFFECT_Unknown31,
	EFFECT_EnableTech,
	EFFECT_DisableTech,
	EFFECT_EnableUnit,
	EFFECT_DisableUnit,
	EFFECT_FlashObjects
};

#if (GAME == 1)
#define MAX_EFFECT EFFECT_StopUnit

#elif (GAME == 2)
#define MAX_EFFECT EFFECT_FlashObjects

#endif

class Effect : public ECBase
{
public:
	Effect();
	Effect(Buffer&); // reads back data written by tobuffer()

	// rule of three: default copy constructor, destructor, assignment fine

	void clear();
	void set(const Effect &e);
	void tobuffer(Buffer &b) const;
	void read(FILE *in);
	void write(FILE *out);

	int size() const;
	int getPlayer() const;
	void setPlayer(int);
	bool check() const;

	std::string getName() const;

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
	long stringid;
	long u2;
	long disp_time;
	unsigned long trig_index;
	AOKPT location;
	AOKRECT area;
	long group;	//unit group
	long utype;
	long panel;

	SString text;
	SString sound;
	UID uids[MAX_UNITSEL];		//array of selected units

	static const char* types[NUM_EFFECTS];
};

#pragma pack(pop)

#endif // AOKTS_EFFECT_H
