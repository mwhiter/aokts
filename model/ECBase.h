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
		i = ECBase::GAIA_INDEX;

enum TType
{
	NONE,	//just so we can have an invalid value for error checking
	TRIGGER,
	EFFECT = 0x17,
	CONDITION = 0x10
};

#pragma pack(push, 4)

/******************************************************************************
 * AOKRECT: just like Windows's struct RECT, but MINE!
 *****************************************************************************/

struct AOKRECT
{
	long top;
	long right;
	long bottom;
	long left;

	AOKRECT(long top = -1, long right = -1, long bottom = -1, long left = -1);
};

/*	AOK points are y,x apparently (or at least the way
	I'm seeing them). */
struct AOKPT
{
	long y, x;

	AOKPT(long yy = -1, long xx = -1)
		: y(yy), x(xx)
	{
	}
};

/**
 * This class contains some common functionality between Effects and
 * Conditions, and enables some basic RTTI.
 */
class ECBase
{
protected:
	ECBase(enum TType c, long t = 0);	//called by Effect() and Condition()

public:
	static const int GAIA_INDEX = 0;

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
