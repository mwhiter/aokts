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
	    SetControlGroup9,
	    SnapView,
	    MaxAmount,
	    MinAmount,
	    SetAISignal,
	    SetAISharedGloal,
	    EnableCheats,
    };
};

struct EffectVirtualTypeAOK {
    enum Value {
	    None,
	    MaxAmount,
	    MinAmount,
	    ReseedFarm,
    };
};

struct EffectVirtualTypeSWGB {
    enum Value {
	    None,
	    MaxAmount,
	    MinAmount,
    };
};

struct EffectVirtualTypeSWGBCC {
    enum Value {
	    None,
	    MaxAmount,
	    MinAmount,
    };
};

struct EffectVirtualTypeAOC {
    enum Value {
	    None,
	    MaxAmount,
	    MinAmount,
	    SetAISignal,
	    SetAISharedGloal,
	    EnableCheats,
	    FreezeUnit,
	    ReseedFarm,
    };
};

struct EffectVirtualTypeHD {
    enum Value {
	    None,
	    MaxAmount,
	    MinAmount,
	    FreezeUnit,
	    ReseedFarm,
    };
};

struct EffectType {
    enum Value {
	    None,
	    ChangeDiplomacy,
	    ResearchTechnology,
	    SendChat,
	    Sound,
	    SendTribute,
	    UnlockGate,
	    LockGate,
	    ActivateTrigger,
	    DeactivateTrigger,
	    AIScriptGoal,
	    CreateObject,
	    TaskObject,
	    DeclareVictory,
	    KillObject,
	    RemoveObject,
	    ChangeView,
	    Unload,
	    ChangeOwnership,
	    Patrol,
	    DisplayInstructions,
	    ClearInstructions,
	    FreezeUnit,
	    UseAdvancedButtons,
	    DamageObject,
	    PlaceFoundation,
	    ChangeObjectName,
	    ChangeObjectHP,
	    ChangeObjectAttack,
	    StopUnit,
	    ChangeSpeed_UP,
	        SnapView_SWGB = ChangeSpeed_UP,
	        AttackMove_HD = ChangeSpeed_UP,
	    ChangeRange_UP,
	        DisableAdvancedButtons_SWGB = ChangeRange_UP,
	        ChangeArmor_HD = ChangeRange_UP,
	    ChangeMeleArmor_UP,
	        ChangeRange_HD = ChangeMeleArmor_UP,
	        EnableTech_SWGB = ChangeMeleArmor_UP,
	    ChangePiercingArmor_UP,
	        ChangeSpeed_HD = ChangePiercingArmor_UP,
	        DisableTech_SWGB = ChangePiercingArmor_UP,
	    EnableUnit_SWGB,
	        HealObject_HD = EnableUnit_SWGB,
	    DisableUnit_SWGB,
	        TeleportObject_HD = DisableUnit_SWGB,
	    FlashUnit_SWGB,
	        ChangUnitStance_HD = FlashUnit_SWGB,
	    InputOff_CC,
	    InputOn_CC,
	};
};

class Effect : public ECBase
{
private:
    bool valid_full_map() const;
    bool valid_partial_map() const;
    bool valid_area_location() const;
    bool valid_area() const;
    bool has_selected() const;
    bool valid_selected() const;
    bool has_unit_constant() const;
    bool valid_unit_constant() const;
    bool valid_unit_spec() const;
    bool valid_technology_spec() const;
    bool valid_location_coord() const;
    bool valid_location_unit() const;
    bool null_location_unit() const;
    bool valid_source_player() const;
    bool valid_target_player() const;
    bool valid_trigger() const;
    bool valid_panel() const;
    bool valid_destination() const;
    bool valid_points() const;

    bool valid_since_last_check;

public:
	Effect();
//	~Effect();
//    Effect( const Effect& other );
	Effect(Buffer&); // reads back data written by tobuffer()

	// rule of three: default copy constructor, destructor, assignment fine

	void tobuffer(Buffer &b);// const; (make it const when unit_cnst gets set elsewhere)
	void read(FILE *in);
	void write(FILE *out);

	int getPlayer() const;
	const char * getTypeName(size_t type, bool concise = false) const;
	void setPlayer(int);
    bool get_valid_since_last_check();
    bool check_and_save();
	bool check() const;

    std::string selectedUnits() const;

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
	long unit_cnst; // pUnit redundant
	const UnitLink *pUnit;	//long in file, of course
	long s_player;	// GAIA = 0, Player 1 = 1, ...
	long t_player;	// GAIA = 0, Player 1 = 1, ...
	long tech_cnst; // pTech redundant
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
	long unknown;

	SString text;
	SString sound;

#pragma pack(push, 4)	// we read these straight from the file
	UID uids[MAX_UNITSEL];		//array of selected units
#pragma pack(pop)

    static const int NUM_EFFECTS_AOK = 24;
    static const int NUM_EFFECTS_AOC = 30;
    static const int NUM_EFFECTS_AOHD = 34;
    static const int NUM_EFFECTS_AOHD4 = 37;
    static const int NUM_EFFECTS_AOF = 34;
    static const int NUM_EFFECTS_AOF4 = 37;
    static const int NUM_EFFECTS_UP = 34;
    static const int NUM_EFFECTS_SWGB = 37;
    static const int NUM_EFFECTS_CC   = 39;

    static const int NUM_VIRTUAL_EFFECTS_AOK = 3;
    static const int NUM_VIRTUAL_EFFECTS_AOC = 7;
    static const int NUM_VIRTUAL_EFFECTS_UP = 30;
    static const int NUM_VIRTUAL_EFFECTS_AOHD = 4;
    static const int NUM_VIRTUAL_EFFECTS_AOF = 4;
    static const int NUM_VIRTUAL_EFFECTS_SWGB = 4;
    static const int NUM_VIRTUAL_EFFECTS_CC = 4;

	static const char* types_aok[NUM_EFFECTS_AOK];
	static const char* types_aoc[NUM_EFFECTS_AOC];
	static const char* types_up[NUM_EFFECTS_UP];
	static const char* types_aohd[NUM_EFFECTS_AOHD4];
	static const char* types_aof[NUM_EFFECTS_AOF4];
	static const char* types_swgb[NUM_EFFECTS_SWGB];
	static const char* types_cc[NUM_EFFECTS_CC];
	static const char* types_short_aok[NUM_EFFECTS_AOK];
	static const char* types_short_aoc[NUM_EFFECTS_AOC];
	static const char* types_short_up[NUM_EFFECTS_UP];
	static const char* types_short_aohd[NUM_EFFECTS_AOHD4];
	static const char* types_short_aof[NUM_EFFECTS_AOF4];
	static const char* types_short_swgb[NUM_EFFECTS_SWGB];
	static const char* types_short_cc[NUM_EFFECTS_CC];

	static const char* virtual_types_aok[NUM_VIRTUAL_EFFECTS_AOK];
	static const char* virtual_types_aoc[NUM_VIRTUAL_EFFECTS_AOC];
	static const char* virtual_types_up[NUM_VIRTUAL_EFFECTS_UP];
	static const char* virtual_types_aohd[NUM_VIRTUAL_EFFECTS_AOHD];
	static const char* virtual_types_aof[NUM_VIRTUAL_EFFECTS_AOF];
	static const char* virtual_types_swgb[NUM_VIRTUAL_EFFECTS_SWGB];
	static const char* virtual_types_cc[NUM_VIRTUAL_EFFECTS_CC];

    static const char** types;
    static const char** types_short;
    static const char** virtual_types;

private:
	void fromGenie(const struct Genie_Effect&);
	struct Genie_Effect toGenie() const;
};

#endif // AOKTS_EFFECT_H
