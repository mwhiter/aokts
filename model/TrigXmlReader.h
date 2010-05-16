/**
 * AOK Trigger Studio (See aokts.cpp for legal conditions.)
 *
 * TrigXmlReader.h -- declares class TrigXmlReader
 *
 * MODEL
 */

#include "trigger.h"
#include <vector>
#include <istream>

/**
 * This is a class responsible for reading triggers from XML (as written by
 * TrigXmlVisitor) and returning a vector<Trigger> from it.
 */
class TrigXmlReader
{
public:
	/**
	 * Read XML from the given istream and return the vector<Trigger>.
	 */
	std::vector<Trigger> read(std::istream&);
};

