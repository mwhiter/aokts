#include "ChangePlayerVisitor.h"

ChangePlayerVisitor::ChangePlayerVisitor(int to)
: _player(to)
{
}

void ChangePlayerVisitor::visit(Trigger&)
{
	// not concerned with triggers
}

void ChangePlayerVisitor::visit(Effect& e)
{
	if (e.getPlayer() > ECBase::GAIA_INDEX)
		e.setPlayer(_player);
}

void ChangePlayerVisitor::visit(Condition& c)
{
	if (c.getPlayer() > ECBase::GAIA_INDEX)
		c.setPlayer(_player);
}

