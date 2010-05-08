#ifndef INC_NULLBUFFER_H
#define INC_NULLBUFFER_H

/**
 * AOK Trigger Studio (see aokts.cpp for legal conditions)
 * NullBuffer.h, declares class NullBuffer.
 *
 * UTIL
 */

#include "Buffer.h"

class NullBuffer : public Buffer
{
public:

	NullBuffer();

	/** Buffer implementation **/

	void read(void *dest, size_t length);
	void reads(char *dest, size_t lensize);
	void skip(size_t offset);

	void write(const void * source, size_t length);
	void writes(const char * source, size_t lensize);
	void fill(int value, size_t length);

	/** Our own special methods **/

	size_t size() const;

private:
	size_t _size;
};

#endif // INC_NULLBUFFER_H
