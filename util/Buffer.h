#ifndef INC_BUFFER_H
#define INC_BUFFER_H

/**
 * AOK Trigger Studio (see aokts.cpp for legal conditions)
 * Buffer.h, declares class Buffer.
 *
 * UTIL
 */

class SString;

/**
 * An abstract class allowing reading & writing of useful datatypes.
 */
class Buffer
{
public:
	virtual bool read(void *dest, const int length) = 0;
	virtual bool reads(char *dest, const int lensize) = 0;
	virtual bool reads(SString &dest, const int lensize) = 0;

	virtual void write(const void *source, const int length) = 0;
	virtual void writes(char *source, const int lensize) = 0;
	virtual void writes(const SString &item, const int lensize) = 0;

	virtual void skip(int offset) = 0;
	virtual void fill(int value, int length) = 0;
};

#endif // INC_BUFFER_H
