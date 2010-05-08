/**
 * MemBuffer.h, defines class MemBuffer
 * AOKTS, GPL, all that.
 */

#include "Buffer.h"

/**
 * An implementation of Buffer that uses a statically-sized chunk of memory as
 * its backing store.
 */
class MemBuffer : public Buffer
{
public:
	MemBuffer(char *b, int l);
//	Buffer(const char *b, int l);
	MemBuffer(int l);
	~MemBuffer();

	bool read(void *dest, const int length);
	bool reads(char *dest, const int lensize);
	bool reads(SString &dest, const int lensize);

	void write(const void *source, const int length);
	void writes(char *source, const int lensize);
	void writes(const SString &item, const int lensize);

	void skip(int offset);
	void fill(int value, int length);

	const char *get() const	{ return buffer; }

private:
	bool mine;	//did i create memory?
	bool writeable;
	char *buffer;
	char *pos;
	int blen;

	// throws std::out_of_range if the specified length won't fit in the buffer
	void checkFits(size_t length) const;
};
