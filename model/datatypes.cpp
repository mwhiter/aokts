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
#include "../util/Buffer.h"
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
		} else {
			newbuff[0] = '\0';
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

/*
 * len is optional for already determined. So no need to read again.
 */
bool SString::read(FILE *in, size_t lensize, unsigned long len)
{
	bool ret = true;

    if (!len)
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

void SString::read(Buffer& b, size_t lensize)
{
	// read the length
	unsigned long len = 0;
	b.read(&len, lensize);

	// read the string, if available
	if (len)
	{
		char * buff = unlock(len + 1);
		b.read(buff, len);
		buff[len] = '\0';
		lock();
	}
}

bool SString::read(FILE *in)
{
	size_t lensize = sizeof(char);
	size_t len = 0;
	bool ret = true;

	char *temp = new char[32];

	readbin(in, temp, 1);
	while (temp[len] != '\0') {
		len++;
		readbin(in, temp + len, 1);
	}

	delete [] data;
	data = temp;

	return ret;
}

void SString::write(FILE *out, size_t lensize, bool force, bool writelen, bool nonull) const
{
	unsigned long len = length();
	if ((len || force) && !nonull)
		len++;

    if (writelen)
	    fwrite(&len, lensize, 1, out);
	fwrite(c_str(), sizeof(char), len, out);
}

void SString::write(Buffer &b, size_t lensize) const
{
	unsigned long len = length();
	if (len)
		++len; // +1 for null termination

	b.write(&len, lensize);
	b.write(c_str(), len);
}
