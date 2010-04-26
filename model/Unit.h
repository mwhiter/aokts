/*
	Unit.h: declares class Unit, representing a scenario unit
	
	MODEL
*/

#include "scen_const.h"

#include <stdio.h> // for FILE

#pragma pack(push, 1)

class Unit
{
public:
	Unit(UID id = 0);
	Unit(Buffer &from);

	float	y, x;		//stupid order
	float	u_float;	//always 1.0 (any use for this?)
	UID		ident;	//identifies unit for triggers, etc.
private:
	UCNST	cnst;		//constant (type) of unit
public:
	char	u_char;	//always 2
	float	rotate;	//in radians
	short	frame;
	UID		garrison;

	static size_t size;

	void read(FILE *in);

	const UnitLink *getType();
	const UnitLink *getType() const;
	void setType(const UnitLink *t);
//	int size() const;
	void toBuffer(Buffer &b) const;

private:
	const UnitLink *pType;
};

#pragma pack(pop)