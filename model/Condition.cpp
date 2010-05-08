#include "Condition.h"
#include "TriggerVisitor.h"

#include "../util/utilio.h"
#include "../util/Buffer.h"

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
	b.read(&ttype, sizeof(long));
	b.read(&type, sizeof(long));
	b.read(&amount, sizeof(long) * 4);
	pUnit = static_cast<const UnitLink*>(readLink(b, esdata.units));
	b.read(&player, sizeof(long));
	pTech = static_cast<const TechLink*>(readLink(b, esdata.techs));
	b.read(&timer, sizeof(long) * 9);
}

int Condition::size() const
{
	return 18 * sizeof(long);
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
	long temp;

	// FIXME: read with a struct
	readbin(in, &type, 6);

	if (ttype != CONDITION)
		throw bad_data_error("Condition has incorrect check value.");

	readbin(in, &temp);
	if (temp >= 0)
		pUnit = static_cast<const UnitLink*>(getById(esdata.units, temp));

	readbin(in, &player);

	readbin(in, &temp);
	if (temp >= 0)
		pTech = static_cast<const TechLink*>(getById(esdata.techs, temp));

	readbin(in, &timer, 9);

	if (type > MAX_CONDITION)
		printf("WARNING: Unknown condition %d.\n", type);
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
