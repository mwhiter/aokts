/**
 * AOK Trigger Studio (see aokts.cpp for legal conditions)
 * NullBuffer.cpp, defines functions for class NullBuffer
 */

#include "NullBuffer.h"
#include <stdexcept>

using std::logic_error;

NullBuffer::NullBuffer()
: _size(0)
{
}

void NullBuffer::read(void * dest, const size_t length)
{
	throw logic_error("NullBuffer does not support reads");
}

void NullBuffer::reads(char * dest, const size_t lensize)
{
	throw logic_error("NullBuffer does not support reads");
}

void NullBuffer::skip(const size_t offset)
{
	_size += offset;
}

void NullBuffer::write(const void * source, const size_t length)
{
	_size += length;
}

void NullBuffer::writes(const char * source, const size_t lensize)
{
	// no null terminator
	_size += lensize + strlen(source);
}

void NullBuffer::fill(int value, size_t length)
{
	_size += length;
}

size_t NullBuffer::size() const
{
	return _size;
}

