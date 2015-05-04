#ifndef AOKTS_EFFECT_H
#define AOKTS_EFFECT_H

#include "ECBase.h"

// Fwd declaration, don't want to #include it.
class TriggerVisitor;

// Need to enforce these
// AoC
//#define MAX_UNITSEL 22
// UP
//#define MAX_UNITSEL 40
// HD
#define MAX_UNITSEL 60

struct EffectVirtualTypeUP {
    enum Value {
	    None,
	    EnableObject,
	    DisableObject,
	    EnableTechnology,
	    DisableTechnology,
	    EnableTechnologyAnyCiv,
	    SetHP,
	    HealObject,
	    SetAggressive,
	    SetDefensive,
	    SetStandGround,
	    SetNoAttackWithoutHalt,
	    Resign,
	    FlashObjects,
	    SetAP,
	    SetControlGroup1,
	    SetControlGroup2,
	    SetControlGroup3,
	    SetControlGroup4,
	    SetControlGroup5,
	    SetControlGroup6,
	    SetControlGroup7,
	    SetControlGroup8,
	    SetControlGroup9
    };
};

struct EffectVirtualTypeAOK {
    enum Value {
	    None,
	    ReseedFarm,
    };
};

struct EffectVirtualTypeSWGB {
    enum Value {
	    None,
    };
};

struct EffectVirtualTypeAOC {
    enum Value {
	    None,
	    ReseedFarm,
	    FreezeUnit,
    };
};

struct EffectVirtualTypeAOHD {
    enum Value {
	    None,
	    ReseedFarm,
	    FreezeUnit,
    };
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
private:
    std::string selectedUnits() const;

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

    static int num_effects; // set this to one of the below
    static const int NUM_EFFECTS_AOC = 37;
    static const int NUM_EFFECTS_SWGB = 39;

    static int num_virtual_effects; // set this to one of the below
    static const int NUM_VIRTUAL_EFFECTS_AOK = 1;
    static const int NUM_VIRTUAL_EFFECTS_AOC = 2;
    static const int NUM_VIRTUAL_EFFECTS_UP = 24;
    static const int NUM_VIRTUAL_EFFECTS_AOHD = 2;
    static const int NUM_VIRTUAL_EFFECTS_SWGB = 1;

	static const char* types_aoc[NUM_EFFECTS_AOC];
	static const char* types_aohd[NUM_EFFECTS_AOC];
	static const char* types_swgb[NUM_EFFECTS_SWGB];
	static const char* types_short_aoc[NUM_EFFECTS_AOC];
	static const char* types_short_aohd[NUM_EFFECTS_AOC];
	static const char* types_short_swgb[NUM_EFFECTS_SWGB];

	static const char* virtual_types_aok[NUM_VIRTUAL_EFFECTS_AOK];
	static const char* virtual_types_aoc[NUM_VIRTUAL_EFFECTS_AOC];
	static const char* virtual_types_up[NUM_VIRTUAL_EFFECTS_UP];
	static const char* virtual_types_aohd[NUM_VIRTUAL_EFFECTS_AOHD];
	static const char* virtual_types_swgb[NUM_VIRTUAL_EFFECTS_SWGB];

    static const char** types; // set to one of effect_types... above
    static const char** types_short; // set to one of effect_types_short... above
    static const char** virtual_types; // set to one of effect_types_short... above

private:
	void fromGenie(const struct Genie_Effect&);
	struct Genie_Effect toGenie() const;
};

#endif // AOKTS_EFFECT_H
