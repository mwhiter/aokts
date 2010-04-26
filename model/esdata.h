#ifndef INC_ESDATA_H
#define INC_ESDATA_H

/*
	esdata.h: Manages standard AOE data.

	MODEL
*/

#include "datatypes.h"
#include <expat.h>
#include <string>

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

/** Need this to destroy any type of Link list **/
template <class XLink> void destroylist(XLink *parse)
{
	XLink *t;

	while (parse)
	{
		t = parse;
		parse = (XLink*)parse->next();
		delete t;
	}
}

/* Link I/O functions */

const Link *getById(const Link *list, int id);

// read an ID from Buffer and return the relevant Link
const Link* readLink(Buffer& b, const Link *list);
bool writeLink(Buffer& b, const Link *link);

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

class CivLink : public Link
{
public:
	CivLink();
	~CivLink();

	UnitLink *units, *unit_tail;
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
	void addNode(Link *&head, Link *&tail, Link *item);
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
	TechLink *techs;
	ColorLink *colors;
	UnitLink *units;
	Link *resources;
	Link *aitypes;
	ColorLink *terrains;
	UnitGroupLink *unitgroups;
	CivLink *civs;

	/* Special unit groups */
	UnitGroupLink *ug_buildings, *ug_units;

	ESDATA();
	~ESDATA();
	void load(const char *path);

	int getCount(enum ESD_GROUP group);
	UnitLink *getUnitById(int id);

private:
	/* LL Tails */
	// TODO: this is stupid, I should use real containers for this
	TechLink *tech_tail;
	ColorLink *color_tail;
	UnitLink *unit_tail;
	Link *res_tail;
	Link *aitype_tail;
	ColorLink *terrain_tail;
	UnitGroupLink *unitgroup_tail;
	CivLink *civ_tail;

	struct
	{
		enum ESD_GROUP pos;
		CivLink *civ;
	} rstate;

	friend void XMLCALL startHandler(void *, const XML_Char *, const XML_Char **);
	friend void XMLCALL endHandler(void *, const XML_Char *);
} esdata;

#endif // INC_ESDATA_H
