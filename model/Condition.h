#ifndef AOKTS_CONDITION_H
#define AOKTS_CONDITION_H

#include "ECBase.h"

// Fwd declaration, don't want to #include it.
class TriggerVisitor;

#define NUM_CONDS	24	//+1 for undefined
#define COND_CONTROLS	16

#pragma pack(push, 4)	//everything in effects/conditions is a long

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

	std::string getName() const;
	std::string getNameTip() const;

	int getPlayer() const;
	void setPlayer(int);
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

	static const char* types[NUM_CONDS];
	static const char* types_short[NUM_CONDS];

private:
	void fromGenie(const struct Genie_Condition&);
	struct Genie_Condition toGenie() const;
};

#pragma pack(pop)

#endif // AOKTS_CONDITION_H
