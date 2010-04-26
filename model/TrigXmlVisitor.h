/*
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.

	TrigXmlVisitor.h -- declares class TrigXmlVisitor

	MODEL
*/

#include "TriggerVisitor.h"

class AutoFile;

/**
 * TriggerVisitor to print triggers, effects, and conditions to a file in XML
 * format.
 */
class TrigXmlVisitor : public TriggerVisitor
{
public:
	TrigXmlVisitor(AutoFile&);
	~TrigXmlVisitor();

	void visit(Trigger&);
	void visit(Effect&);
	void visit(Condition&);
	void visitEnd(Trigger&);

private:
	AutoFile& _af;
	unsigned _trigcount;
};
