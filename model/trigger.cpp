/* MODEL */

#include "trigger.h"
#include "TriggerVisitor.h"

#include "../util/utilio.h"
#include "../util/Buffer.h"
#include "../util/settings.h"

#include <vector>
#include <algorithm>

/* Triggers, Conditions, and Effects */

#undef PLAYER1_INDEX
#define PLAYER1_INDEX 1

Trigger::Trigger()
:	state(1), loop(0), u1(0), obj(0), obj_order(0), display_order(-1)
{
	memset(name, 0, sizeof(name));
}

Trigger::Trigger(const Trigger &t) // TODO: we can use the compiler's version
:	state(t.state), loop(t.loop), u1(t.u1), obj(t.obj), obj_order(t.obj_order),
	description(t.description), effects(t.effects), conds(t.conds), display_order(t.display_order)
{
	strcpy(name, t.name);
}

Trigger::Trigger(Buffer& buffer)
{
	/* We use std::vectors here for RAII. */
	using std::vector;

	int num;

	buffer.read(&state, 14);
	buffer.fill(0, sizeof(long));
	description.read(buffer, sizeof(long));
	buffer.reads(name, sizeof(long));

	/* effects */

	buffer.read(&num, sizeof(long));
	vector<Effect> effects;				// temporarily holds Effects before we get to order info
	effects.reserve(num);

	for (int i = 0; i < num; i++)
		effects.push_back(Effect(buffer));

	while (num--)
	{
		long index;
		buffer.read(&index, sizeof(index));
		this->effects.push_back(effects[index]);
	}

	effects.clear(); // free the memory early

	/* conditions */

	buffer.read(&num, sizeof(long));
	vector<Condition> conditions;
	conditions.reserve(num);

	for (int i = 0; i < num; i++)
		conditions.push_back(Condition(buffer));

	while (num--)
	{
		long index;
		buffer.read(&index, sizeof(index));
		this->conds.push_back(conditions[index]);
	}

	display_order = -1;
}

void Trigger::read(FILE *in)
{
	using std::vector;

	long n_effects, n_conds;

	// TODO: read in as stru3ct
	/*long testlong;
	unsigned char testchar;
	readbin(in, &testlong); printf("State: "); show_binrep(testlong);
	readbin(in, &testlong); printf("Loop: "); show_binrep(testlong);
	readbin(in, &testchar); printf("u1: "); show_binrep(testchar);
	readbin(in, &testchar); printf("obj: "); show_binrep(testchar);
	readbin(in, &testlong); printf("obj_order: "); show_binrep(testlong);
	readbin(in, &testlong); printf("Zeroes: "); show_binrep(testlong);
	readbin(in, &testlong); printf("Description: "); show_binrep(testlong);
	readbin(in, &testlong); printf("Name: "); show_binrep(testlong);*/
	
	readbin(in, &state);
	readbin(in, &loop);
	readbin(in, &u1);
	readbin(in, &obj);
	readbin(in, &obj_order);
	readunk<long>(in, 0, "trigger zeroes", false);

	description.read(in, sizeof(long));
	//readcsDebug<unsigned short>(in, name, sizeof(name));
	readcs<unsigned long>(in, name, sizeof(name));
	//unsigned short testlong;
	//readbin(in, &testlong);

	
	
	//read effects
	readbin(in, &n_effects);
	if (setts.intense) {
	    printf("nEffects: ");
	    show_binrep(n_effects);
	}

	if (n_effects > 0)
	{
		// construct them all so we can directly call read()
		vector<Effect> effects(n_effects);

		for (int i = 0; i < n_effects; i++)
		{
			try
			{
				effects[i].read(in);
			}
			catch (std::exception &)
			{
				printf("Effect %d invalid.\n", i);
				throw;
			}
		}

		while (n_effects--)
		{
			long order = readval<long>(in);
			// I keep the effects in the proper order in memory, unlike AOK.
			this->effects.push_back(effects[order]);
		}
	}

	
	//read conditions
	readbin(in, &n_conds);
	if (setts.intense) {
	    printf("nConds: ");
	    show_binrep(n_conds);
	}

	if (n_conds > 0)
	{
		// construct them all so we can directly call read()
		vector<Condition> conditions(n_conds);

		for (int i = 0; i < n_conds; i++)
		{
			try
			{
				conditions[i].read(in);
			}
			catch (std::exception &)
			{
				printf("Condition %d invalid.\n", i);
				throw;
			}
		}

		while (n_conds--)
		{
			long order = readval<long>(in);
			conds.push_back(conditions[order]);
		}
	}
}

void Trigger::write(FILE *out)
{
	int num, i;

	fwrite(&state, 14, 1, out);	//state, loop, u1, obj, obj_order
	NULLS(out, 4);
	description.write(out, sizeof(long), true);
	num = strlen(name) + 1;
	fwrite(&num, 4, 1, out);
	fwrite(name, sizeof(char), num, out);

	//Effects

	num = effects.size();

	fwrite(&num, 4, 1, out);
	for (i = 0; i < num; i++)
		effects[i].write(out);

	for (i = 0; i < num; i++)
		fwrite(&i, 4, 1, out);

	//Conditions

	num = conds.size();

	fwrite(&num, 4, 1, out);
	for (i = 0; i < num; i++)
		conds[i].write(out);

	for (i = 0; i < num; i++)
		fwrite(&i, 4, 1, out);
}

void Trigger::tobuffer(Buffer& buffer) const
{
	int i, num;

	buffer.write(&state, 14);
	buffer.fill(0, sizeof(long));
	description.write(buffer, sizeof(long));
	buffer.writes(name, sizeof(long));

	//effects

	num = effects.size();

	buffer.write(&num, sizeof(long));
	for (i = 0; i < num; i++)
		effects[i].tobuffer(buffer);

	for (i = 0; i < num; i++)
		buffer.write(&i, sizeof(long));

	//conditions

	num = conds.size();

	buffer.write(&num, sizeof(long));

	for (i = 0; i < num; i++)
		conds[i].tobuffer(buffer);

	//condition order
	for (i = 0; i < num; i++)
		buffer.write(&i, sizeof(long));
}

class get_ec_player
{
public:
	get_ec_player()
		: _player(0) // assume GAIA at start
	{}

	int player() const
	{
		return _player;
	}

	void operator()(const ECBase& e)
	{
		int player = e.getPlayer();

		// If player is GAIA or -1, skip this effect.
		if (player > ECBase::GAIA_INDEX && player != _player)
		{
			if (!_player) // no player assigned yet?
				_player = player;
			else
				_player = -1; // not the same player, error
		}
	}

private:
	int _player;
};

int Trigger::get_player()
{
	get_ec_player gep;
	gep = std::for_each(effects.begin(), effects.end(), gep);
	gep = std::for_each(conds.begin(), conds.end(), gep);

	return gep.player();
}

/**
 * Functor to send a TriggerVisitor to Conditions and Effects.
 */
class send_visitor
{
public:
	send_visitor(TriggerVisitor& tv)
		: _tv(tv)
	{}

	void operator()(Condition& c)
	{
		c.accept(_tv);
	}

	void operator()(Effect& e)
	{
		e.accept(_tv);
	}
	
private:
	TriggerVisitor& _tv;
};

void Trigger::accept(TriggerVisitor& tv)
{
	tv.visit(*this);

	send_visitor sv(tv);
	std::for_each(effects.begin(), effects.end(), sv);
	std::for_each(conds.begin(), conds.end(), sv);

	tv.visitEnd(*this);
}

const char *Condition::types[] =
{
	"Undefined",
	"Bring Object to Area",
	"Bring Object to Object",
	"Own Objects",
	"Own Fewer Objects",
	"Objects in Area",
	"Destroy Object",
	"Capture Object",
	"Accumulate Attribute",
	"Research Technology",
	"Timer",
	"Object Selected",
	"AI Signal",
	"Player Defeated",
	"Object Has Target",
	"Object Visible",
	"Object Not Visible",
	"Researching Technology",
	"Units Garrisoned",
	"Difficulty Level",
	"Own Fewer Foundations",
	"Selected Objects in Area",
	"Powered Objects in Area",
	"Units Queued Past Pop Cap"
};

const char *Condition::types_short[] =
{
	"Undefined",
	"In Area",
	"At Object",
	"Own",
	"Own Fewer",
	"In Area",
	"Destroyed",
	"Captured",
	"Accumulated",
	"Researched",
	"Time",
	"Selected",
	"AI Signal",
	"Defeated",
	"Targetting",
	"Visible",
	"Not Visible",
	"Researching",
	"Garrisoned",
	"Difficulty",
	"Fewer Foundations",
	"Selected in Area",
	"Powered in Area",
	"Queued Past Pop Cap"
};
