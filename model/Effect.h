#ifndef AOKTS_EFFECT_H
#define AOKTS_EFFECT_H

#include "ECBase.h"

// Fwd declaration, don't want to #include it.
class TriggerVisitor;

#define NUM_EFFECTS	37	//+1 for undefined
#define EFFECT_CONTROLS	23

//todo: check UP, HD
#define MAX_UNITSEL 22

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
	void setPlayer(int);
	bool check() const;

	std::string getName() const;
	std::string getNameTip() const;

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
	static const char* types_short[NUM_EFFECTS];

private:
	void fromGenie(const struct Genie_Effect&);
	struct Genie_Effect toGenie() const;
};

#endif // AOKTS_EFFECT_H
