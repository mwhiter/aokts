/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	datatypes.cpp -- Variable-length String Implementation

	Note: This file is cross-platform.

	MODEL
**/

#include "datatypes.h"
#include <string.h>
#include "../util/utilio.h"
#include <stdlib.h>

char SString::null = '\0';

SString::SString()
:	len(0), data(NULL)
{
}

SString::SString(const char *d, int l)
:	len(l)
{
	put(d);
}

SString::SString(const char *d)
:	len(strlen(d))
{
	put(d);
}

SString::SString(const SString &s)
:	len(s.len)
{
	put(s.data);
}

SString::~SString()
{
	if (len)	//if data points to null, len will be zero
		delete [] data;
}

void SString::operator=(const SString& s)
{
	set(s.data, s.len);
}

void SString::erase()
{
	if (len)
	{
		delete [] data;
		len = 0;
	}

	data = NULL;
}

void SString::set(const char *d, int l)
{
	if (len)
		delete [] data;

	len = l;
	put(d);
}

int SString::set(const char *d)
{
	if (len)
		delete [] data;

	len = strlen(d);
	put(d);
	return len;
}

void SString::put(const char *d)
{
	if (d)
	{
		data = new char[len + 1];
		strncpy(data, d, len);
		data[len] = '\0';
	}
	else
		data = NULL;
}

char *SString::unlock(int want)
{
	want++;	//add 1 for NULL termination

	if (len < want)	//do we have enough already?
	{
		char *newbuff = new char[want];

		if (len)	//if something's already in data, copy it and free it
		{
			strncpy(newbuff, data, len);
			newbuff[len] = '\0';
			delete [] data;
		}

		data = newbuff;
	}

	return data;
}

void SString::lock()
{
	char *newbuff;

	len = strlen(data);
	newbuff = new char[len + 1];

	strncpy(newbuff, data, len);
	newbuff[len] = '\0';

	delete [] data;
	data = newbuff;
}

bool SString::read(FILE *in, size_t lensize)
{
	size_t len = 0;
	bool ret = true;

	fread(&len, 1, lensize, in);
	if (len)
	{
		char *buff = unlock(len + 1);
		readbin(in, buff, len);
		buff[len] = '\0';
		lock();
	}
	else
		erase();

	return ret;
}

void SString::write(FILE *out, size_t lensize, bool force) const
{
	int len = length();
	if (len || force)
		len++;

	fwrite(&len, lensize, 1, out);
	fwrite(c_str(), sizeof(char), len, out);
}

/* Buffer */

Buffer::Buffer(char *b, int l)
:	blen(l)
{
	buffer = b;
	pos = buffer;
	mine = false;
}
/*
Buffer::Buffer(const char *b, int l)
:	blen(l)
{
}
*/
Buffer::Buffer(int l)
:	blen(l)
{
	buffer = new char[blen];
	pos = buffer;
	mine = true;
}

Buffer::~Buffer()
{
	if (mine)
		delete [] buffer;
}

bool Buffer::read(void *dest, const int length)
{
	memcpy(dest, pos, length);
	pos += length;

	return pos - buffer <= blen;
}

bool Buffer::reads(char *dest, const int lensize)
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

bool Buffer::reads(SString &dest, const int lensize)
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

bool Buffer::write(const void *source, const int length)
{
	memcpy(pos, source, length);
	pos += length;

	return pos - buffer <= blen;
}

bool Buffer::writes(char *source, const int lensize)
{
	size_t len = strlen(source);

	memcpy(pos, &len, lensize);
	pos += lensize;

	memcpy(pos, source, len);
	pos += len;

	return pos - buffer <= blen;
}

bool Buffer::writes(const SString &source, const int lensize)
{
	int len = source.length();
	if (len)
		len++;

	memcpy(pos, &len, lensize);
	pos += lensize;

	memcpy(pos, source.c_str(), len);
	pos += len;

	return pos - buffer <= blen;
}

bool Buffer::skip(int offset)
{
	pos += offset;

	return pos - buffer <= blen;
}

bool Buffer::fill(int value, int length)
{
	memset(pos, value, length);
	pos += length;
	
	return pos - buffer <= blen;
}
