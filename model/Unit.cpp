/*
	Unit.cpp: defines Unit class functions
	
	MODEL
*/

#include "Unit.h"
#include "../util/utilio.h"
#include "../util/Buffer.h"

Unit::Unit(UID id)
:	x(0), y(0), u_float(1), ident(id), cnst(0),
	u_char(0), rotate(0), frame(0), garrison(-1),
	pType(NULL)
{}

Unit::Unit(Buffer &from)
{
	from.read(this, size);
	pType = esdata.units.getByIdSafe(cnst);
}

void Unit::read(FILE *in)
{
	fread(this, size, 1, in);
	pType = esdata.units.getByIdSafe(cnst);
}

const UnitLink *Unit::getType()
{
	return (pType) ? pType : (pType = (UnitLink*)esdata.units.getById(cnst));
}

const UnitLink *Unit::getType() const
{
	return (pType) ? pType : (UnitLink*)esdata.units.getById(cnst);
}

void Unit::setType(const UnitLink *t)
{
	assert(t);
	pType = t;
	cnst = (UCNST)t->id();
}

void Unit::toBuffer(Buffer &to) const
{
	to.write(this, size);
}

size_t Unit::size = 29;
