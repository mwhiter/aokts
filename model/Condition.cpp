#include "Condition.h"
#include "TriggerVisitor.h"

#include "../util/utilio.h"
#include "../util/Buffer.h"

#pragma pack(push, 4)
// An condition as stored in in scenario
struct Genie_Condition
{
	long type;
	long check;
	long amount;
	long resource_type;
	long uid_object;
	long uid_location;
	long unit_const;
	long player;
	long technology;
	long timer;
	long unknown;
	AOKRECT area;
	long unit_group;
	long unit_type;
	long ai_signal;
};
#pragma pack(pop)

Condition::Condition()
:	ECBase(CONDITION),
	amount(-1),
	res_type(-1),
	object(-1),
	u_loc(-1),
	pUnit(NULL),
	player(-1),
	pTech(NULL),
	timer(-1),
	u1(-1),
	// AOKRECT default constructor OK
	group(-1),
	utype(-1),
	ai_signal(-1)
{
}

Condition::Condition(Buffer& b)
:	ECBase(CONDITION)
{
	Genie_Condition genie;
	b.read(&genie, sizeof(genie));
	std::swap(genie.type, genie.check); // HACK: un-swap type, check
	fromGenie(genie);
}

std::string Condition::getName() const
{
	return (type < NUM_CONDS) ? types[type] : "Unknown!";
}

int Condition::getPlayer() const
{
	return player;
}

void Condition::setPlayer(int pplayer)
{
	player = pplayer;
}

bool Condition::check() const
{
	switch (type)
	{
	case CONDITION_BringObjectToArea:
		return (object >= 0 && area.left >= 0);

	case CONDITION_BringObjectToObject:
		return (object >= 0 && u_loc >= 0);

	case CONDITION_OwnObjects:
	case CONDITION_OwnFewerObjects:
	case CONDITION_OwnFewerFoundations:
		return (player >= 0 && amount >= 0);

	case CONDITION_ObjectsInArea:
	case CONDITION_SelectedObjectsInArea:
	case CONDITION_PoweredObjectsInArea:
		return (area.left >= 0 && amount >= 0);

	case CONDITION_DestroyObject:
		return (object >= 0);

	case CONDITION_CaptureObject:
		return (object >= 0 && player >= 0);

	case CONDITION_AccumulateAttribute:
		return (player >= 0 && res_type >= 0);

	case CONDITION_ResearchTehcnology:
	case CONDITION_ResearchingTechnology:
		return (player >= 0 && pTech->id() >= 0);

	case CONDITION_Timer:
		return (timer >= 0);

	case CONDITION_ObjectSelected:
		return (object >= 0);

	case CONDITION_AISignal:
		return (ai_signal >= 0);

	case CONDITION_PlayerDefeated:
	case CONDITION_UnitsQueuedPastPopCap:
		return (player >= 0);

	case CONDITION_ObjectHasTarget:
		return (object >= 0 && u_loc >= 0);

	case CONDITION_ObjectVisible:
	case CONDITION_ObjectNotVisible:
		return (object >= 0);

	//CONDITION_ResearchingTechnology above

	case CONDITION_UnitsGarrisoned:
		return (object >= 0 && amount >= 0);

	case CONDITION_DifficultyLevel:
		return (amount >= 0);

	//CONDITION_OwnFewerFoundations shares with OwnObjects above
	//CONDITION_SelectedObjectsInArea shares with ObjectsInArea above
	//CONDITION_PoweredObjectsInArea shares with ObjectsInArea above
	//CONDITION_UnitsQueuedPastPopCap shares with PlayerDefeated above

	default:
		return false;
	}
}

void Condition::read(FILE *in)
{
	Genie_Condition genie;

	readbin(in, &genie);
	fromGenie(genie);
}

void Condition::fromGenie(const Genie_Condition& genie)
{
	if (genie.check != CONDITION)
		throw bad_data_error("Condition has incorrect check value.");

	if (genie.type > MAX_CONDITION)
		printf("WARNING: Unknown condition %d.\n", type);

	type = genie.type;
	ttype = static_cast<TType>(genie.check);
	res_type = genie.resource_type;
	amount = genie.amount;
	object = genie.uid_object;
	u_loc = genie.uid_location;
	pUnit = static_cast<const UnitLink*>(
			getById(esdata.units, genie.unit_const));
	player = genie.player;
	pTech = static_cast<const TechLink*>(
			getById(esdata.techs, genie.technology));
	timer = genie.timer;
	u1 = genie.unknown;
	area = genie.area;
	group = genie.unit_group;
	utype = genie.unit_type;
	ai_signal = genie.ai_signal;
}

void Condition::write(FILE *out)
{
	long temp;

	fwrite(&type, sizeof(long), 6, out);
	temp = (pUnit) ? pUnit->id() : -1;
	fwrite(&temp, sizeof(long), 1, out);
	fwrite(&player, sizeof(long), 1, out);
	temp = (pTech) ? pTech->id() : -1;
	fwrite(&temp, sizeof(long), 1, out);
	fwrite(&timer, sizeof(long), 9, out);
}

void Condition::tobuffer(Buffer &b) const
{
	b.write(&ttype, sizeof(long));
	b.write(&type, sizeof(long));
	b.write(&amount, sizeof(long) * 4);
	writeLink(b, pUnit);
	b.write(&player, sizeof(long));
	writeLink(b, pTech);
	b.write(&timer, sizeof(long) * 9);
}

void Condition::accept(TriggerVisitor& tv)
{
	tv.visit(*this);
}
