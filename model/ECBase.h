#ifndef AOKTS_ECBASE_H
#define AOKTS_ECBASE_H

#include "scen_const.h"

/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.

	ECBase.h: defines class ECBase

	MODEL
**/

/** Some macros because the effect/condition player
	numbers are weird. **/
#define P_StdToEC(i) \
	if (++i == 9) \
		i = 0;
#define P_ECToStd(i) \
	if (--i == -1) \
		i = GAIA_INDEX;

enum TType
{
	NONE,	//just so we can have an invalid value for error checking
	TRIGGER,
	EFFECT = 0x17,
	CONDITION = 0x10
};

#pragma pack(push, 4)

/**
 * This class contains some common functionality between Effects and
 * Conditions, and enables some basic RTTI.
 */
class ECBase
{
protected:
	ECBase(enum TType c, long t = 0);	//called by Effect() and Condition()

public:
	long type;	//identifies type of condition/effect
	TType ttype;	//identifies EFFECT or CONDITION

	virtual std::string getName() const = 0;

	/**
	 * @return the player with which this E/C is associated, or -1 if none
	 */
	virtual int getPlayer() const = 0;
	virtual void setPlayer(int player) = 0;

	virtual int size() const = 0;

	/*	Note: The clipboard data has type and ttype reversed
		from the order AOK uses. */
	virtual void tobuffer(Buffer &b) const = 0;
};

#pragma pack(pop)

#endif // AOKTS_ECBASE_H
