/**
 * AOK Trigger Studio (see aokts.cpp for legal conditions)
 * MemBuffer.cpp, defines functions to class MemBuffer.
 *
 * UTIL
 */

#include "MemBuffer.h"
// FIXME: get rid of this dependency
#include "../model/datatypes.h"

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

bool MemBuffer::read(void *dest, const int length)
{
	memcpy(dest, pos, length);
	pos += length;

	return pos - buffer <= blen;
}

bool MemBuffer::reads(char *dest, const int lensize)
{
	size_t len = 0;

	memcpy(&len, pos, lensize);
	pos += lensize;

	if (len)
		memcpy(dest, pos, len);
	else
		*dest = '\0';

	pos += len;

	return pos - buffer <= blen;
}

bool MemBuffer::reads(SString &dest, const int lensize)
{
	size_t len = 0;

	memcpy(&len, pos, lensize);
	pos += lensize;

	if (len)
	{
		char *buff = dest.unlock(len + 1);
		memcpy(buff, pos, len);
		buff[len] = '\0';
		dest.lock();
		pos += len;
	}

	return pos - buffer <= blen;
}

void MemBuffer::write(const void *source, const int length)
{
	checkFits(length);

	memcpy(pos, source, length);
	pos += length;
}

void MemBuffer::writes(char *source, const int lensize)
{
	size_t len = strlen(source);

	checkFits(lensize + len);

	memcpy(pos, &len, lensize);
	pos += lensize;

	memcpy(pos, source, len);
	pos += len;
}

void MemBuffer::writes(const SString &source, const int lensize)
{
	int len = source.length();
	if (len)
		len++;  // +1 for null-termination

	checkFits(lensize + len);

	memcpy(pos, &len, lensize);
	pos += lensize;

	memcpy(pos, source.c_str(), len);
	pos += len;
}

void MemBuffer::skip(int offset)
{
	checkFits(offset);

	pos += offset;
}

void MemBuffer::fill(int value, int length)
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