#ifndef AOKTS_CONDITION_H
#define AOKTS_CONDITION_H

#include "ECBase.h"

// Fwd declaration, don't want to #include it.
class TriggerVisitor;

#pragma pack(push, 4)	//everything in effects/conditions is a long

struct ConditionVirtualTypeAOC {
    enum Value {
	    None,
	    SinglePlayer,
	    Taunt,
	    AIScriptGoal,
	    StartingAgeStandard,
	    StartingResourcesStandard,
	    Regicide,
	    Deathmatch,
	    OneClickGarrison
    };
};

struct ConditionVirtualTypeUP {
    enum Value {
	    None,
	    SinglePlayer,
	    Taunt,
	    AIScriptGoal,
	    StartingAgeStandard,
	    StartingResourcesStandard,
	    Regicide,
	    Deathmatch,
	    OneClickGarrison
    };
};

enum ConditionType
{
	CONDITION_None,
	CONDITION_BringObjectToArea,
	CONDITION_BringObjectToObject,
	CONDITION_OwnObjects,
	CONDITION_OwnFewerObjects,
	CONDITION_ObjectsInArea,
	CONDITION_DestroyObject,
	CONDITION_CaptureObject,
	CONDITION_AccumulateAttribute,
	CONDITION_ResearchTehcnology,
	CONDITION_Timer,
	CONDITION_ObjectSelected,
	CONDITION_AISignal,
	CONDITION_PlayerDefeated,
	CONDITION_ObjectHasTarget,
	CONDITION_ObjectVisible,
	CONDITION_ObjectNotVisible,
	CONDITION_ResearchingTechnology,
	CONDITION_UnitsGarrisoned,
	CONDITION_DifficultyLevel,
	CONDITION_OwnFewerFoundations,   // (SWGB only)
	CONDITION_SelectedObjectsInArea, // (SWGB only)
	CONDITION_PoweredObjectsInArea,  // (SWGB only)
	CONDITION_UnitsQueuedPastPopCap  // (works in AOK)
};

class Condition : public ECBase
{
public:
	Condition();
	Condition(Buffer&); // reads back data written by tobuffer()

	// rule of three: default copy constructor, destructor, assignment fine

	void read(FILE *in);
	void write(FILE *out);
	void tobuffer(Buffer &b) const;

	std::string getName(bool tip = false, NameFlags::Value flag=NameFlags::NONE) const;

	int getPlayer() const;
	void setPlayer(int);
    bool get_valid_since_last_check();
    bool check_and_save();
	bool check() const;

	/**
	 * Accepts a TriggerVisitor.
	 */
	void accept(TriggerVisitor&);

	long amount;
	long res_type;
	UID object;
	UID u_loc;
	const UnitLink *pUnit;	//long in file, of course
	long player;	// GAIA = 0, Player 1 = 1, ...
	const TechLink *pTech;	//long in file, of course
	long timer;
	long reserved;
	AOKRECT area;
	long group;
	long utype;
	long ai_signal;

    static const int NUM_CONDITIONS_AOK = 20;
    static const int NUM_CONDITIONS_SWGB = 22;
    static const int NUM_CONDITIONS_CC = 24;

    static const int NUM_VIRTUAL_CONDITIONS_AOK = 1;
    static const int NUM_VIRTUAL_CONDITIONS_AOC = 8;
    static const int NUM_VIRTUAL_CONDITIONS_UP = 8;
    static const int NUM_VIRTUAL_CONDITIONS_AOHD = 1;
    static const int NUM_VIRTUAL_CONDITIONS_AOF = 1;
    static const int NUM_VIRTUAL_CONDITIONS_SWGB = 1;
    static const int NUM_VIRTUAL_CONDITIONS_CC = 1;

	static const char* types_aok[NUM_CONDITIONS_AOK];
	static const char* types_swgb[NUM_CONDITIONS_SWGB];
	static const char* types_cc[NUM_CONDITIONS_CC];
	static const char* types_short_aok[NUM_CONDITIONS_AOK];
	static const char* types_short_swgb[NUM_CONDITIONS_SWGB];
	static const char* types_short_cc[NUM_CONDITIONS_CC];

	static const char* virtual_types_aok[NUM_VIRTUAL_CONDITIONS_AOK];
	static const char* virtual_types_aoc[NUM_VIRTUAL_CONDITIONS_AOC];
	static const char* virtual_types_up[NUM_VIRTUAL_CONDITIONS_UP];
	static const char* virtual_types_aohd[NUM_VIRTUAL_CONDITIONS_AOHD];
	static const char* virtual_types_aof[NUM_VIRTUAL_CONDITIONS_AOF];
	static const char* virtual_types_swgb[NUM_VIRTUAL_CONDITIONS_SWGB];
	static const char* virtual_types_cc[NUM_VIRTUAL_CONDITIONS_CC];

	static const char** types;
	static const char** types_short;
    static const char** virtual_types;

    static const int NUM_TAUNT_SETS = 64;
	static const char* taunt_set[NUM_TAUNT_SETS];

private:
	void fromGenie(const struct Genie_Condition&);
	struct Genie_Condition toGenie() const;

	bool valid_since_last_check;
};

#pragma pack(pop)

#endif // AOKTS_CONDITION_H
