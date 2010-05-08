/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	datatypes.h -- Defines custom datatypes for AOKTS (a variable-length string and a vector)

	Note: This file is cross-platform.

	MODEL
**/

#ifndef SSTRING_H
#define SSTRING_H

#include <new>        /* for placement new */
#include <assert.h>
#include <tchar.h>

#ifndef NULL
#define NULL    0
#endif

#define MAX_HEAPSIZE 0x7ffdefff

typedef struct _iobuf FILE;
class Buffer;

/* Simple Variable-length string */

class SString
{
	char *data;	//points to either stored string or a null character
	int len;
	void put(const char *d);	//store string when len already set

	static char null;	//now 

public:
	SString();
	SString(const char *d, int l);	//same as set(const char *, int)
	SString(const char *d);				//same as set(const char *)

	// rule of three
	SString(const SString &s);			//same as set(const char *, int)
	~SString();
	void operator=(const SString &s);

	void erase();							//erases string (just a null remains)
	void set(const char *d, int l);	//ignores any null termination
	int set(const char *d);				//copies until null
	char *unlock(int want);				//returns buffer with at least want space
	void lock();							//trims buffer after unlock()

	/* I/O */
	bool read(FILE *in, size_t lensize);

	/**
	 * Read the string from a Buffer.
	 */
	void read(Buffer&, size_t lensize);

	/**
	 * Write the string to a FILE.
	 * @param lensize number of bytes in length field
	 * @param force forces a write of a single null even if empty
	 */
	void write(FILE *out, size_t lensize, bool force = false) const;

	/**
	 * Write the string to a Buffer.
	 * @param lensize number of bytes in length field
	 */
	void write(Buffer&, size_t lensize) const;
	
	inline int total_size() const		{ return sizeof(long) + len + 1; }
	inline int length() const			{ return len; }
	/* length with null */
	inline int lwn() const				{ return (len) ? len + 1 : len; }
	inline const char *c_str() const	{ return data ? data : &null; }
};

/* "Simple" Vector (basically just a remake of std::vector) */

#ifndef EXTRA		//Extra space allocated
#define EXTRA 5
#endif

/*	FOREACH: Loop instruction for parsing an SVector.

	Requires a variable count.
	*/
#define FOREACH(v, ptr) \
	for (count = (v).count(), ptr = (v).first(); \
		count; count--, ptr++)

template <class _AC> class SVector
{
	_AC *begin;	//beginning of allocated space (may not be filled)
	_AC *end;	//end of allocated space
	_AC *next;	//next object to be contructs
	bool realloc_flag;	//set if reallocation occurs, cleared by realloced()

	/*	Private function prototypes

	/* copies, then returns the end of the destination **
	_AC *copy(_AC *dest, const _AC *source, int count);

	/* Automatically expands the controlled sequence by 1 + EXTRA **
	void expand();

	*/

	_AC *copy(_AC *dest, const _AC *source, int count)
	{
		_AC *next = dest;

		while (count--)
			new (next++) _AC(*source++);

		return next;
	}

	void expand()
	{
		int new_space = count() + 1 + EXTRA;
		int old_count = count();
		_AC *old_begin = begin;

		_AC *new_array = (_AC *)operator new(new_space * sizeof(_AC));
		next = copy(new_array, begin, old_count);

		while (--old_count > 0)
			old_begin++->~_AC();

		operator delete ((void*)begin);

		begin = new_array;
		end = new_array + new_space;

		realloc_flag = true;
	}

public:
	SVector()
	: begin(NULL), end(NULL), next(NULL)
	{
	}

	SVector(const SVector &source)
	:	begin(NULL), end(NULL), next(NULL)	//end, next set in case source.count() = 0
	{
		int new_count = source.count();

		if (new_count)
		{
			allocate(new_count, false);
			next = copy(begin, source.first(), new_count);
		}
	}

	~SVector()
	{
		while (next-- > begin)
			next->~_AC();	//destruct each object

		operator delete ((void*)begin);	//then deallocate the mem
	}

	/* Public function prototypes

	/* Removes all members **
	void clear();

	/* checks whether list has been reallocated lately. **
	bool realloced();

	/* returns a member **
	_AC *at(index_t offset);

	/* returns a const member **
	const _AC *at(index_t offset) const;

	/* adds an item, returns index **
	int append(const _AC &item);

	/*  pre-allocates space, direct specifies whether items
	    will be added with append() or memcpy() **
	bool allocate(size_t count, bool direct);

	/* removes items by pointer **
	bool remove(_AC *first, size_t count = 1);

	/* removes an item by index **
	bool remove(index_t index);

	/* inserts an item here **
	void insert(_AC *here, const _AC &item);

	*/

	void clear()
	{
		while (next > begin)
		{
			--next;			//decrement to target object
			next->~_AC();	//then destruct it
		}

		operator delete ((void*)begin);

		begin = end = next = NULL;
	}

	_AC* at(size_t offset)
	{
		_AC *item = ((signed)offset < next - begin) && (offset >= 0) ? begin + offset : NULL;

		return item;
	}

	const _AC* at(size_t offset) const
	{
		const _AC *item = begin + offset;
		assert(item >= begin);		//only checks in debug

		return item < next ? item : NULL;
	}

	int append(const _AC &item)
	{
		_AC source(item);	//needs one away from main list

		if (end <= next)
			expand();

		new (next++) _AC(source);

		return count() - 1;
	}

	bool allocate(size_t count, bool direct)
	{
		int new_space;

		if (!count)
			return false;

		/* Clear previous contents. (Good idea?) */
		if (begin)
			clear();

		new_space = count + EXTRA;

		begin = (_AC *)operator new(new_space * sizeof(_AC));
		end = begin + new_space;
		next = begin;

		if (direct && next)
		{
			_AC blank;

			while (count--)
				append(blank);	//construct count items for memory overlay
		}

		return (begin != 0);
	}

	bool remove(_AC *first, size_t count = 1)
	{
		int move_count = next - first - 1;	//-1 for soon-to-be removed item
		_AC *killit = first;
		_AC *source = first + count;

		if (count > 0)
		{
			next -= count;

			while (count--)
				killit++->~_AC();

			while (move_count--)
			{
				new (first++) _AC(*source);
				source->~_AC();
				source++;
			}

			return true;
		}

		return false;
	}
	
	bool remove(size_t index)
	{
		_AC *item = at(index);

		assert(item != NULL);

		return remove(item, 1);
	}
	
	/*	I used to have item passed by reference, but I realized that this
		was unreliable since item's reference can change in the middle of
		the below function while it moves around stuff */
	void insert(_AC *here, const _AC item)
	{
		if (next >= end)	//do we need to expand?
		{
			_AC *dest;
			int new_space = count() + 1 + EXTRA;

			//allocate a new array and copy up to the reserved spot
			_AC *new_array = (_AC *)operator new(new_space * sizeof(_AC));
			dest = copy(new_array, begin, here - begin);

			//"insert" the item
			new (dest++) _AC(item);

			//copy the rest
			next = copy(dest, here, next - here);

			//set the pointers
			begin = new_array;
			end = begin + new_space;

			realloc_flag = true;
		}
		else
		{
			_AC *source = next - 1;
			_AC *dest = next++;

			/* Move everything back one */
			while (dest > here)
			{
				new (dest--) _AC(*source);
				source->~_AC();
				source--;
			}

			new (dest) _AC(item);	//source = here
		}
	}
	
	inline int count() const	{ return next - begin; }
	inline _AC* first()			{ return begin; }
	inline const _AC* first() const { return begin; }
	inline _AC* last()			{ return next - 1; }
	inline const _AC* last() const { return  next - 1; }
};

/*
	A "simple" linkedlist

	Note: This list _does not_ make copies of the items you feed to
	it. Call clear(true) before destructing to delete the items, too.
*/
template <class _AC>
struct SLink
{
	/*  This probably doesn't have to be allocated separately,
	    but I do anyway to limit code generation. */
	_AC *item;
	SLink *prev, *next;
};

template <class _AC>
class SList
{
	SLink <_AC> *f, *l;

public:
	SList();
	~SList();

	SLink <_AC> *append(_AC *item);
	bool remove(SLink <_AC> *link);

	void clear(bool delItems);

	inline SLink <_AC> *first()
	{return f;}
	inline const SLink <_AC> *first() const
	{return f;}
	inline SLink <_AC> *last()
	{return l;}
	inline const SLink <_AC> *last() const
	{return l;}
};

template <class _AC>
SList <_AC>::SList()
:	f(NULL), l(NULL)
{}

template <class _AC>
SList <_AC>::~SList()
{
	clear(false);
}

template <class _AC> void SList<_AC>::clear(bool delItems)
{
	SLink <_AC> *link = f, *next;

	while (link)
	{
		next = link->next;
		if (delItems)
			delete link->item;
		delete link;

		link = next;
	}

	f = NULL;
	l = NULL;
}

template <class _AC> SLink <_AC> *SList <_AC>::append(_AC *item)
{
	SLink <_AC> *link = new SLink<_AC>;

	if (link && item)
	{
		link->item = item;
		link->prev = l;
		link->next = NULL;

		if (l)
		{
			l->next = link;
		}

		if (!f)
			f = link;

		l = link;
	}

	return link;
}

template <class _AC> bool SList <_AC>::remove(SLink <_AC> *which)
{
	bool ret = false;

	for (SLink <_AC> *link = f; link; link = link->next)
	{
		if (link == which)
		{
			ret = true;

			link->prev->next = link->next;
			link->next->prev = link->prev;

			delete link->item;
			delete link;
		}
	}

	return ret;
}

#endif //SSTRING_H
