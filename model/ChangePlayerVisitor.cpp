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
	e.setPlayer(_player);
}

void ChangePlayerVisitor::visit(Condition& c)
{
	c.setPlayer(_player);
}

