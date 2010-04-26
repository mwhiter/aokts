#ifndef AOKTS_TRIGGER_H
#define AOKTS_TRIGGER_H

/* MODEL */

typedef struct _iobuf FILE;	//makes including <stdio.h> unnecessary

#include "scen_const.h"
#include "Effect.h"
#include "Condition.h"
#include <vector>

// Fwd declaration, don't want to #include it.
class TriggerVisitor;

/* Triggers */

#pragma pack(push, 1)

class Trigger
{
	void read(FILE *in);
	void write(FILE *out);

public:
	Trigger();
	Trigger(const Trigger &t);
	Trigger(char *data, const int size);

	const static size_t MAX_TRIGNAME = 128;

	long state, loop;
	char u1, obj;
	long obj_order;
	SString description;
	char name[MAX_TRIGNAME];
	std::vector<Effect> effects;
	std::vector<Condition> conds;

	void tobuffer(char *buffer, int size) const;
	int size() const;

	/*	get_player: Returns non-GAIA player for conditions/effect in the trigger.

		Returns:
		* 0 if all players are unassigned (GAIA)
		* -1 if there is a mix of non-GAIA players
		* EC player index if above are false
	*/
	int get_player();

	/**
	 * Accepts a TriggerVisitor and sends it along to the effects and
	 * conditions as well. (See TriggerVisitor.h)
	 */
	void accept(TriggerVisitor&);

	friend class Scenario;
};

#pragma pack(pop)

#endif // AOKTS_TRIGGER_H
