#ifndef INC_ESDATA_H
#define INC_ESDATA_H

/*
	esdata.h: Manages standard AOE data.

	MODEL
*/

#include "datatypes.h"
#include <expat.h>
#include <string>

class Buffer;

/* geniedata type links */

/**
 * First off, this class definitely violates the "program to an interface, not
 * an implementation" rule. It's both the base class of Genie data objects and
 * a non-template link class.
 *
 * It and its subclasses read data into themselves from repeated calls to the
 * read() function with key/value pairs.
 */
class Link
{
public:
	Link();
	Link(int id, const wchar_t * name);

	bool read(const wchar_t * name, const wchar_t * value);

	Link* next();
	const Link* next() const;
	void setNext(Link*);
	int id() const;
	const wchar_t * name() const;

private:
	Link* _next;
	int _id;
	std::wstring _name;
};

inline Link* Link::next()
{
	return _next;
}

inline const Link* Link::next() const
{
	return _next;
}

inline void Link::setNext(Link* link)
{
	_next = link;
}

inline int Link::id() const
{
	return _id;
}

inline const wchar_t * Link::name() const
{
	return _name.c_str();
}

/* Derived links */

class TechLink : public Link
{
public:
	char *civ;
};

class ColorLink : public Link
{
public:
	long ref;
};

class UnitLink : public Link
{
public:
	UnitLink();
	UnitLink(int id, const wchar_t * name);

	long hp;
};

class UnitGroupLink : public Link
{
public:
	UnitGroupLink();
	~UnitGroupLink();

	unsigned count;
	const UnitLink **list;
};

/**
 * This class is a custom linked-list implementation that provides convenient
 * functions for looking up an item basd
 */
template <class T> class LinkList
{
public:
	LinkList();
	~LinkList();

	/**
	 * Returns the first item in the list, from which all other items can be
	 * accessed.
	 */
	T * head();
	T const * head() const;

	/**
	 * Returns the item in the list that has the given id, or throws a
	 * domain_error if no such item exists.
	 */
	T const * getById(int id) const;

	/**
	 * Looks up an item by its id. Returns a pointer to the item that is
	 * already in the list if it exists, or creates a new instance, adds it to
	 * the list, and returns it.
	 *
	 * Unlike getById(), this will not throw a domain_error but will always
	 * return a valid pointer.
	 */
	T * getByIdSafe(int id);

private:
	T * _head;
	T * _tail;

	/**
	 * Appends the given item to the list. I don't like "push_back", but it's
	 * C++ convention.
	 *
	 * After this operation, the list owns the pointer. It will delete it when
	 * destructing the list.
	 */
	void push_back(T * item);

	// HACK: allow ESDATA only to call push_back
	friend class ESDATA;
};

enum ESD_GROUP
{
	ESD_nowhere,
	ESD_techs,
	ESD_units,
	ESD_colors,
	ESD_resources,
	ESD_aitypes,
	ESD_terrains,
	ESD_unitgroups,
	ESD_civs
};

extern class ESDATA
{
	int tech_count, color_count, unit_count, res_count, aitype_count,
		terrain_count, unitgroup_count, civ_count;

	/*	I know, I should probably use a hashtable for this,
		but I don't expect adding enough different tags to
		warrant it. */
	static struct pFunc
	{
		const wchar_t * name;
		void (ESDATA::*f)(const XML_Char **attrs);
	} pfuncs[];

	/* Reading funcs */
	void readTech(const XML_Char **attrs);
	void readColor(const XML_Char **attrs);
	void readUnit(const XML_Char **attrs);
	void readTerrain(const XML_Char **attrs);
	void readSimple(const XML_Char **attrs);
	void readResources(const XML_Char **attrs);
	void readAitypes(const XML_Char **attrs);
	void readUnitGroup(const XML_Char **attrs);
	void readCiv(const XML_Char **attrs);

	void endResources();
	void endCiv();

public:
	/* LL heads */
	LinkList<TechLink> techs;
	LinkList<ColorLink> colors;
	LinkList<UnitLink> units;
	LinkList<Link> resources;
	LinkList<Link> aitypes;
	LinkList<ColorLink> terrains;
	LinkList<UnitGroupLink> unitgroups;
	LinkList<Link> civs;

	/* Special unit groups */
	UnitGroupLink *ug_buildings, *ug_units;

	ESDATA();
	void load(const char *path);

	int getCount(enum ESD_GROUP group);

private:

	struct
	{
		enum ESD_GROUP pos;
	} rstate;

	friend void XMLCALL startHandler(void *, const XML_Char *, const XML_Char **);
	friend void XMLCALL endHandler(void *, const XML_Char *);
} esdata;

/** Template Member Function definitions here... don't look! **/

template <class T> LinkList<T>::LinkList()
:   _head(NULL),
	_tail(NULL)
{
}

template <class T> LinkList<T>::~LinkList()
{
	T *parse = _head;

	while (parse)
	{
		T * t = parse;
		parse = static_cast<T*>(parse->next());
		delete t;
	}
}

template <class T> T * LinkList<T>::head()
{
	return _head;
}

/**
 * Finds the link with the given id, or returns NULL if none could be found.
 */
template <class T> T findId(T head, int id)
{
	for (T parse = head;
		parse;
		parse = static_cast<T>(parse->next()))   // Return type isn't covariant
	{
		if (parse->id() == id)
		{
			return parse;
		}
	}

	return NULL;
}

template <class T> T const * LinkList<T>::getById(int id) const
{
	// Translate -1 into NULL for no selection
	if (id == -1)
	{
		return NULL;
	}

	// Lookup item if it exists
	T const * item = findId(_head, id);

	if (item != NULL)
	{
		return item;
	}
	else
	{
		throw std::domain_error("Could not lookup ID in game data.");
	}
}

template <class T> T * LinkList<T>::getByIdSafe(int id)
{
	// Translate -1 into NULL for no selection
	if (id == -1)
	{
		return NULL;
	}

	// Lookup item if it exists
	T * item = findId(_head, id);

	// Item did not exist, we need to make a new one.
	if (item == NULL)
	{
		printf("Creating new gamedata item for unrecognized id %d.\n", id);

		// First, create a name for the new item
		// Use a buffer big enough to hold MAX_INT
		const size_t BUFSIZE = 32;
		wchar_t name[BUFSIZE];
		swprintf(name, BUFSIZE, L"?%d", id);

		// Create the new instance
		item = new T(id, name);

		// Add the item to the list
		push_back(item);
	}

	// Always return item
	return item;
}

template <class T> void LinkList<T>::push_back(T * item)
{
	// If list is empty, make item the head.
	if (!_head)
		_head = item;

	// If list already has a tail, set the current tail's next ptr to the item.
	if (_tail)
		_tail->setNext(item);

	// item will always be the new tail.
	_tail = item;
}

#endif // INC_ESDATA_H
