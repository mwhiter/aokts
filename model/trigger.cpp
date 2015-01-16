/* MODEL */

#include "trigger.h"
#include "TriggerVisitor.h"

#include "../util/utilio.h"
#include "../util/Buffer.h"
#include "../util/settings.h"
#include "../view/utilui.h"

#include <vector>
#include <algorithm>

using std::vector;

/* Triggers, Conditions, and Effects */

#undef PLAYER1_INDEX
#define PLAYER1_INDEX 1

Trigger::Trigger()
//:	state(1), loop(0), u1(0), u2(0), obj(0), obj_order(0), display_order(-1), obj_str_id(0)
:	state(1), loop(0), u1(0), obj(0), obj_order(0), display_order(-1), obj_str_id(0)
{
	memset(name, 0, sizeof(name));
}

Trigger::Trigger(const Trigger &t) // TODO: we can use the compiler's version
:	state(t.state), loop(t.loop), u1(t.u1), obj(t.obj), obj_order(t.obj_order), obj_str_id(t.obj_str_id),
	description(t.description), effects(t.effects), conds(t.conds), display_order(t.display_order)
{
	strcpy(name, t.name);
}

Trigger::Trigger(Buffer& buffer)
{
	/* We use std::vectors here for RAII. */
	using std::vector;

	int num;

	buffer.read(&state, 18);
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

std::string Trigger::getName(bool tip)
{
	return std::string(name).c_str();
}

bool compare_effect_nametip(const Effect& first,
				  const Effect& second)
{
    //make change ownership act like it is prefixed with create (so that
    //create comes first

    std::string s = std::string(first.getName(true));
    std::string t = std::string(second.getName(true));

    // kill must come after create (hawk explosions)
    if (first.type == 14) {
        s.insert (0, "create");
    }

    if (second.type == 14) {
        t.insert (0, "create");
    }

    // remove must come before create (clear way for spawn)
    if (first.type == 15) {
        s.insert (0, "creat ");
    }

    if (second.type == 15) {
        t.insert (0, "creat ");
    }

    // change ownership must come after create (non-convertible gaia)
    if (first.type == 18) {
        s.insert (0, "create");
    }

    if (second.type == 18) {
        t.insert (0, "create");
    }

	// Windows doesn't support POSIX strcasecmp().
	int sort = _stricmp(s.c_str(),
		t.c_str());

	return (sort < 0);
}

bool compare_condition_nametip(const Condition& first,
				  const Condition& second)
{
	// Windows doesn't support POSIX strcasecmp().
	int sort = _stricmp(first.getName(true).c_str(),
		second.getName(true).c_str());

	return (sort < 0);
}

void Trigger::sort_conditions()
{
    std::sort(this->conds.begin(), this->conds.end(), compare_condition_nametip);
}

void Trigger::sort_effects()
{
    std::sort(this->effects.begin(), this->effects.end(), compare_effect_nametip);
}

void Trigger::read(FILE *in)
{
	using std::vector;

	long n_effects, n_conds;

	// TODO: read in as struct
	readbin(in, &state); // enabled/disabled
	readbin(in, &loop);  // looping
	readbin(in, &u1);    // unknown. trim trigger?
	//readbin(in, &u2);    // unknown. trim trigger?
	if (setts.intense) {
	    show_binrep(u1);
	    //swapByteOrder(u1);
	    //swapByteOrder(u2);
	    printf("trigger unknown: %u\n", u1);
	    //printf("trigger unknown: %u\n", u2);
	}
	readbin(in, &obj);
	readbin(in, &obj_order);
	readbin(in, &obj_str_id);

	description.read(in, sizeof(long));
	readcs<unsigned long>(in, name, sizeof(name));

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
		//effects.resize(n_effects);

		for (int i = 0; i < n_effects; i++)
		{
			try
			{
				effects[i].read(in);
				//this->effects[i].read(in);
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
	        printf("effect order: %d", order);
	        //show_binrep(order);
			// I keep the effects in the proper order in memory, unlike AOK.
			this->effects.push_back(effects[order]);
		}
	}

	//if (u1 != 76548) {

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
	//}
}

void Trigger::write(FILE *out)
{
	int num, i;

	fwrite(&state, 18, 1, out);	//state, loop, u1, obj, obj_order, obj_str_id
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

	// trigger
	buffer.write(&state, 18);
	description.write(buffer, sizeof(long));
	buffer.writes(name, sizeof(long));

	// effects
	num = effects.size();
	buffer.write(&num, sizeof(long));
	for (i = 0; i < num; i++)
		effects[i].tobuffer(buffer);

	// effects order
	for (i = 0; i < num; i++)
		buffer.write(&i, sizeof(long));

	// conditions
	num = conds.size();
	buffer.write(&num, sizeof(long));
	for (i = 0; i < num; i++)
		conds[i].tobuffer(buffer);

	// conditions order
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
