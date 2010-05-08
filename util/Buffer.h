#ifndef INC_BUFFER_H
#define INC_BUFFER_H

/**
 * AOK Trigger Studio (see aokts.cpp for legal conditions)
 * Buffer.h, declares class Buffer.
 *
 * UTIL
 */

/**
 * An abstract class allowing reading & writing of useful datatypes.
 */
class Buffer
{
public:
	virtual void read(void *dest, size_t length) = 0;
	virtual void reads(char *dest, size_t lensize) = 0;
	virtual void skip(size_t offset) = 0;

	virtual void write(const void *source, size_t length) = 0;
	virtual void writes(const char *source, size_t lensize) = 0;
	virtual void fill(int value, size_t length) = 0;
};

#endif // INC_BUFFER_H
