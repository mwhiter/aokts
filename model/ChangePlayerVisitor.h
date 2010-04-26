/*
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.

	ChangePlayerVisitor.h -- declares class ChangePlayerVisitor

	MODEL
*/

#include "TriggerVisitor.h"

class ChangePlayerVisitor : public TriggerVisitor
{
public:
	ChangePlayerVisitor(int to);

	void visit(Trigger&);
	void visit(Effect&);
	void visit(Condition&);

private:
	int _player;
};
