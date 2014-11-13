/**
 * AOK Trigger Studio (see aokts.cpp for legal conditions)
 * MemBuffer.cpp, defines functions to class MemBuffer.
 *
 * UTIL
 */

#include "MemBuffer.h"

#include <stdexcept>

using std::out_of_range;

/* MemBuffer */

MemBuffer::MemBuffer(char *b, int l)
:	blen(l)
{
	buffer = b;
	pos = buffer;
	mine = false;
}
/*
MemBuffer::MemBuffer(const char *b, int l)
:	blen(l)
{
}
*/
MemBuffer::MemBuffer(int l)
:	blen(l)
{
	buffer = new char[blen];
	pos = buffer;
	mine = true;
}

MemBuffer::~MemBuffer()
{
	if (mine)
		delete [] buffer;
}

void MemBuffer::read(void *dest, const size_t length)
{
	checkFits(length);

	memcpy(dest, pos, length);
	pos += length;
}

void MemBuffer::reads(char *dest, const size_t lensize)
{
	size_t len = 0;

	checkFits(lensize);

	memcpy(&len, pos, lensize);
	pos += lensize;

	checkFits(len);
	if (len)
		memcpy(dest, pos, len);
	else
		*dest = '\0';

	pos += len;
}

void MemBuffer::write(const void *source, const size_t length)
{
	checkFits(length);

	memcpy(pos, source, length);
	pos += length;
}

void MemBuffer::writes(const char *source, const size_t lensize)
{
	size_t len = strlen(source) + 1;

	checkFits(lensize + len);

	memcpy(pos, &len, lensize);
	pos += lensize;

	memcpy(pos, source, len);
	pos += len;
}

void MemBuffer::skip(size_t offset)
{
	checkFits(offset);

	pos += offset;
}

void MemBuffer::fill(int value, size_t length)
{
	checkFits(length);

	memset(pos, value, length);
	pos += length;
}

void MemBuffer::checkFits(size_t length) const
{
	if (pos + length > buffer + blen)
		throw out_of_range("not enough space in MemBuffer");
}