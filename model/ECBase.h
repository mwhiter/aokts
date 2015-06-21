#ifndef AOKTS_ECBASE_H
#define AOKTS_ECBASE_H

#include "scen_const.h"

/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.

	ECBase.h: defines class ECBase

	MODEL
**/

struct NameFlags {
    enum Value{
        NONE                           = 0x01,
        LIMITLEN                       = 0x02
    };
};

/**
 * Since Effects & Conditions use a different player-numbering system, and
 * there's no sure way to deduce how the scenario would handle Player > 8, we
 * just limit E/C to players 1-8 and GAIA.
 */
const size_t EC_NUM_PLAYERS = 9;

// And some macros to convert between standard and E/C numbering.
#define P_StdToEC(i) \
	if (++i == 9) \
		i = 0;
#define P_ECToStd(i) \
	if (--i == -1) \
		i = GAIA_INDEX;

/**
 * Contains player names in E/C ordering.
 * TODO: put in ECBase
 */
extern char const *players_ec[EC_NUM_PLAYERS + 1];

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
	long bottom;
	long left;
	long top;
	long right;

	AOKRECT(long bottom = -1, long left = -1, long top = -1, long right = -1);
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

	virtual std::string getName(bool tip = false, NameFlags::Value flag=NameFlags::NONE) const = 0;

	/**
	 * @return the player with which this E/C is associated, or -1 if none
	 */
	virtual int getPlayer() const = 0;
	virtual void setPlayer(int player) = 0;

	/*	Note: The clipboard data has type and ttype reversed
		from the order AOK uses. */
	virtual void tobuffer(Buffer &b) const = 0;
};

#pragma pack(pop)

#endif // AOKTS_ECBASE_H
