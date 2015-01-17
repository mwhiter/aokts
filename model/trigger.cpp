/* MODEL */

#include "trigger.h"
#include "TriggerVisitor.h"

#include "../util/helper.h"
#include "../util/utilio.h"
#include "../util/Buffer.h"
#include "../util/settings.h"
#include "../view/utilui.h"
#include "scen.h"

#include <vector>
#include <algorithm>

using std::vector;

extern class Scenario scen;

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

/*
 * DON'T LET AOKTS GET RECURSIVE WITH TRIGGER HINTS (if one trigger's
 * name displays the name of another)
 */
std::string Trigger::getName(bool tip, bool limitlen)
{
    std::ostringstream ss;
    std::string result;
    if (!tip) {
	    result = std::string(name);
	    goto theend;
	} else {
        bool c_own_fewer = false;
        bool c_in_area = false;
        bool c_has_gold = false;
        bool c_x_units_killed = false;
        bool c_x_buildings_razed = false;
        bool e_create_unit = false;
        bool e_remove_unit = false;
        bool e_has_unit_type = false;
        bool e_buff = false;
        bool e_nerf = false;
        bool e_activate = false;
        bool e_deactivate = false;
        int e_n_activated = false;
        const char * text = "";
        std::string unit_type_name = "";
        bool e_has_text = false;
        int activated = -1;
        int amount = 0;
        int victims = 0;
        int razings = 0;
        int gold_total = 0;
        int player = -1;
        int killer = -1;
        int timer = -1;
        int n_conds = 0;
        int n_effects = 0;
        bool victory = false;
        bool victor[9];
        bool defeat = false;
        bool e_research = false;
        Condition * last_cond = NULL;
        Effect * last_effect = NULL;

        for (int i = 0; i < 9; i++) {
            victor[i] = false;
        }

	    // conditions
	    for (vector<Condition>::iterator iter = conds.begin(); iter != conds.end(); ++iter) {
	        n_conds++;
	        last_cond = &(*iter);
	        switch (iter->type) {
	        case 4:  // own fewer objects
	            c_own_fewer = true;
	            break;
	        case 5:  // objects in area
	            c_in_area = true;
	            break;
	        case 8:  // accumulate attribute
	            switch (iter->res_type) {
	            case 3:
	                c_has_gold = true;
	                player = iter->player;
	                break;
	            case 20:
	                c_x_units_killed = true;
	                killer = iter->player;
	                victims = iter->amount;
	                break;
	            case 43:
	                c_x_buildings_razed = true;
	                killer = iter->player;
	                razings = iter->amount;
	                break;
	            }
	            break;
	        case 10:
	            if (iter->timer > 0 && iter->timer > timer) {
	                timer = iter->timer;
	            }
	            break;
	        case 13:
	            defeat = true;
	            player = iter->player;
	            break;
	        }
        }

	    // each effect
	    for (vector<Effect>::iterator iter = effects.begin(); iter != effects.end(); ++iter) {
	        n_effects++;
	        last_effect = &(*iter);
	        if (strlen(iter->text.c_str()) > 0) {
	            e_has_text = true;
	            text = iter->text.c_str();
	        }
	        switch (iter->type) {
	        case 2:  // research
	            e_research = true;
	            break;
            case 5:  // send tribute
                amount = iter->amount;
                if (iter->t_player == 0) {
                    amount = -amount;
                }
                if (iter->res_type == 3) {
                    gold_total += amount;
	            }
                break;
            case 8:  // activate
                e_activate = true;
                e_n_activated ++;
                activated = iter->trig_index;
                break;
            case 9:  // deactivate
                e_deactivate = true;
                break;
	        case 11: // create unit
	            {
	                player = iter->s_player;
	                e_create_unit = true;
	                e_has_unit_type = iter->pUnit && iter->pUnit->id();
	                if (e_has_unit_type) {
                        std::wstring unitname(iter->pUnit->name());
                        unit_type_name = std::string(unitname.begin(), unitname.end());
                    }
                }
	            break;
	        case 13: // declare victory
	            if (iter->s_player > 0) {
	                victor[iter->s_player] = true;
	                victory = true;
	            }
	            break;
	        case 15: // remove unit
	            {
	                player = iter->s_player;
	                e_remove_unit = true;
	                e_has_unit_type = iter->pUnit && iter->pUnit->id();
	                if (e_has_unit_type) {
                        std::wstring unitname(iter->pUnit->name());
                        unit_type_name = std::string(unitname.begin(), unitname.end());
                    }
                }
	            break;
            case 24: // Damage
            case 27: // HP
            case 28: // Attack
            case 30: // UP Speed
            case 31: // UP Range
            case 32: // UP Armor1
            case 33: // UP Armor2
                amount = iter->amount;
                if (amount < 0) {
	                e_nerf = true;
	            } else if (amount > 0) {
	                e_buff = true;
	            }
                break;
	        }
	    }

	    bool e_earn_gold = gold_total > 0;
	    bool e_lose_gold = gold_total < 0;
	    bool e_only_1_activated = e_n_activated == 1;
        bool only_one_cond = n_effects == 0 && n_conds == 1;
        bool only_one_effect = n_effects == 1 && n_conds == 0;

        if (only_one_cond) {
            ss << last_cond->getName(setts.displayhints);
            result = ss.str();
            goto theend;
        }
        if (only_one_effect) {
            ss << last_effect->getName(setts.displayhints);
            result = ss.str();
            goto theend;
        }
        if (timer > 0) {
            if (this->loop) {
                ss << "every " << time_string(timer) << " ";
                if (!this->state) {
                    ss << "after activated ";
                }
            } else if (this->state) {
                ss << "at " << time_string(timer) << " ";
            } else {
                ss << "in " << time_string(timer) << " ";
            }
        }

        if (e_only_1_activated && activated >= 0) {
            // this is recursive, fix this
            //ss << scen.triggers.at(activated).getName(setts.displayhints);
            // do this for the moment
            ss << scen.triggers.at(activated).getName(false);
            result = ss.str();
            goto theend;
        }

        if (defeat) {
            ss << "p" << player << " disconnected";
            result = ss.str();
            goto theend;
        }

        if (victory) {
            ss << "victory to ";
            for (int i = 0; i < 9; i++) {
                if (victor[i])
                    ss << "p" << i << " ";
            }
            result = ss.str();
            goto theend;
        }

        if (c_in_area && e_remove_unit) {
            switch (player) {
                case -1:
                    ss << "?";
                    break;
                case 0:
                    ss << "Gaia";
                    break;
                default:
                    ss << "p" << player;
            }
            ss << " buy special";
            if (e_has_text) {
                ss << " \"" << trim(std::string(text)) << "\"";
            }
            result = ss.str();
            goto theend;
        }
        //if (c_has_gold) {
        //    ss << "has gold";
        //}
        //if (e_create_unit) {
        //    ss << "create_unit";
        //}
        //if (e_lose_gold) {
        //    ss << "lose gold";
        //}
        if (c_in_area && c_has_gold && e_buff && e_lose_gold) {
            ss << "purchase buff for " << -gold_total << " gold";
            result = ss.str();
            goto theend;
        }
        if (c_in_area && c_has_gold && e_create_unit && e_lose_gold) {
            ss << "buy " << unit_type_name << " for " << -gold_total << " gold";
            result = ss.str();
            goto theend;
        }
        if (c_own_fewer && e_create_unit && e_has_unit_type) {
            switch (player) {
                case -1:
                    ss << "?";
                    break;
                case 0:
                    ss << "Gaia";
                    break;
                default:
                    ss << "p" << player;
            }
            ss << " spawn " << unit_type_name;
            result = ss.str();
            goto theend;
        }
        if (c_x_units_killed) {
            ss << "reward ";
            switch (killer) {
                case -1:
                    ss << "?";
                    break;
                case 0:
                    ss << "Gaia";
                    break;
                default:
                    ss << "p" << killer;
            }
            ss << " " << victims << " kills";
            if (e_earn_gold) {
                ss << " earn " << gold_total << " gold";
                result = ss.str();
                goto theend;
            }
            result = ss.str();
            goto theend;
        }
        if (c_x_buildings_razed) {
            ss << "reward ";
            switch (killer) {
                case -1:
                    ss << "?";
                    break;
                case 0:
                    ss << "Gaia";
                    break;
                default:
                    ss << "p" << killer;
            }
            ss << " " << razings << " razings";
            if (e_earn_gold) {
                ss << " earn " << gold_total << " gold";
                result = ss.str();
                goto theend;
            }
            result = ss.str();
            goto theend;
        }
        if (e_earn_gold) {
            ss << "earn " << gold_total << " gold";
            result = ss.str();
            goto theend;
        }
        if (e_create_unit) {
            ss << "create ";
            switch (killer) {
                case -1:
                    ss << "?";
                    break;
                case 0:
                    ss << "Gaia";
                    break;
                default:
                    ss << "p" << killer;
            }
            ss << unit_type_name;
            result = ss.str();
            goto theend;
        }
        if (e_nerf) {
            ss << "nerf some units";
            result = ss.str();
            goto theend;
        }
        if (e_buff) {
            ss << "buff some units";
            result = ss.str();
            goto theend;
        }
        if (e_research) {
            ss << "do research";
            result = ss.str();
            goto theend;
        }
        if (e_remove_unit) {
            ss << "remove some units";
            result = ss.str();
            goto theend;
        }
        if (e_has_text) {
            ss << " " << trim(std::string(text));
        }
        result = ss.str();
        goto theend;
	}

    //int aInt = 368;
    //char str[15];
    //sprintf(str, "%d", Int);
    //if (strlen(name) > 0)
    //return (std::string(name))
    //}
theend:
    return limitlen?result.substr(0,100):result;
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
