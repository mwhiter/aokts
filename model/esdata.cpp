/*
	esdata.cpp
	
	MODEL
*/

#define XML_UNICODE_WCHAR_T

#include "esdata.h"

#include "../util/utilio.h"
#include "../util/Buffer.h"
#include <expat.h>
#include <stdio.h>
#include <malloc.h>	//free() for strdup()
#include <stdexcept>

const size_t XMLBUFF_SIZE = 2048;

/* Link */

Link::Link()
:	_next(NULL)
{
}

Link::Link(int id, const wchar_t * name)
:	_next(NULL), _id(id), _name(name)
{
}

bool Link::read(const wchar_t * name, const wchar_t * value)
{
	bool ret = true;

	if (!wcscmp(name, L"name"))
		_name = value;
	else if (!wcscmp(name, L"id"))
		swscanf(value, L"%d", &_id);
	else
		ret = false;

	return ret;
}

Link const *getById(Link const * list, int id)
{
	// Translate -1 into NULL for no selection
	if (id == -1)
	{
		return NULL;
	}

	// Lookup item if it exists
	Link const * item = findId(list, id);

	if (item != NULL)
	{
		return item;
	}
	else
	{
		throw std::domain_error("Could not lookup ID in game data.");
	}
}

const Link* readLink(Buffer &b, const Link *list)
{
	long id;

	b.read(&id, sizeof(long));

	return getById(list, id);
}

void writeLink(Buffer &b, const Link *link)
{
	long id;
	
	id = (link) ? link->id() : -1;

	b.write(&id, sizeof(id));
}

UnitLink::UnitLink()
{
}

UnitLink::UnitLink(int id, const wchar_t * name)
:	Link(id, name)
{
}

UnitGroupLink::UnitGroupLink()
:	list(NULL)
{
}

UnitGroupLink::~UnitGroupLink()
{
	delete [] list;
}

CivLink::CivLink()
:	units(NULL), unit_tail(NULL)
{
}

CivLink::~CivLink()
{
	destroylist(units);
}

void addNode(Link *&head, Link *&tail, Link *item)
{
	if (!head)
		head = item;

	if (tail)
		tail->setNext(item);

	tail = item;
}

ESDATA::ESDATA()
:	techs(NULL), tech_tail(NULL),
	colors(NULL), color_tail(NULL),
	unitgroups(NULL), unitgroup_tail(NULL),
	ug_buildings(NULL), ug_units(NULL)
{
}

ESDATA::~ESDATA()
{
	destroylist(techs);
	destroylist(colors);
}

void ESDATA::readTech(const XML_Char **attrs)
{
	TechLink *link = new TechLink;

	while (*attrs)
	{
		if (link->read(attrs[0], attrs[1]))
			attrs++;
		else
			attrs++;

		attrs++;
	}

	addNode((Link*&)techs, (Link*&)tech_tail, link);
	tech_count++;
}

void ESDATA::readColor(const XML_Char **attrs)
{
	ColorLink *link = new ColorLink;

	while (*attrs)
	{
		if (!link->read(attrs[0], attrs[1]))
		{
			if (!wcscmp(attrs[0], L"color"))
				swscanf(attrs[1], L"%x", &link->ref);
		}

		attrs += 2;
	}

	addNode((Link*&)colors, (Link*&)color_tail, link);
	color_count++;
}

void ESDATA::readUnit(const XML_Char **attrs)
{
	UnitLink *link = new UnitLink;

	while (*attrs)
	{
		if (link->read(*attrs, attrs[1]))
			attrs++;
		else
			attrs++;

		attrs++;
	}

	units.push_back(link);
	unit_count++;
}

void ESDATA::readTerrain(const XML_Char **attrs)
{
	ColorLink *link = new ColorLink;

	while (*attrs)
	{
		if (!link->read(*attrs, attrs[1]))
		{
			if (!wcscmp(attrs[0], L"color"))
				swscanf(attrs[1], L"%x", &link->ref);
		}

		attrs +=2 ;
	}

	addNode((Link*&)terrains, (Link*&)terrain_tail, link);
	terrain_count++;
}

void ESDATA::readSimple(const XML_Char **attrs)
{
	Link *link = new Link;
	Link **dest = NULL, **dest_tail = NULL;

	while (*attrs)
	{
		if (link->read(*attrs, attrs[1]))
			attrs++;
		else
			attrs++;

		attrs++;
	}

	switch (rstate.pos)
	{
	case ESD_resources:
		resources.push_back(link);
		res_count++;
		break;
	case ESD_aitypes:
		aitypes.push_back(link);
		aitype_count++;
		break;
	}
}

void ESDATA::readResources(const XML_Char **)
{
	rstate.pos = ESD_resources;
}

void ESDATA::readAitypes(const XML_Char **)
{
	rstate.pos = ESD_aitypes;
}

void ESDATA::readUnitGroup(const XML_Char **attrs)
{
	UnitGroupLink *link = new UnitGroupLink;
	const UnitLink **uparse;
	int i;

	while (*attrs)
	{
		if (link->read(*attrs, attrs[1]))
			attrs++;
		else if (!wcscmp(*attrs, L"units"))
		{
			wchar_t *dup, *p;
			dup = _wcsdup(attrs[1]);

			/* Count the number of units in group */
			p = dup;
			link->count = 1;	//one more unit than commas
			while (p = wcschr(p, ','))
			{
				link->count++;
				p++;	//skip past comma
			}

			/* Allocate and load the list */
			link->list = new const UnitLink*[link->count];
			uparse = link->list;
			if (!uparse)
				return;
			p = wcstok(dup, L",");
			do
			{
				swscanf(p, L"%d", &i);
				if (*uparse = (UnitLink*)esdata.units.getById(i))
					uparse++;
				else
					printf("Discarding unknown unit type %d from group.\n", i);
			} while (p = wcstok(NULL, L","));

			attrs++;
		}
		else
			attrs++;

		attrs++;
	}

	if (!link->name())
	{
		puts("ERROR: No name value for unit group link. Chucking.");
		return;
	}

	if (!link->list || !*link->list)
	{
		printf("Discarding empty unit group %s.\n", link->name());
		delete [] link->list;
		link->list = NULL;
		delete link;
		return;
	}

	// TODO: any less-hardcoded way to do this?
	if (!ug_buildings && !wcscmp(link->name(), L"Buildings"))
		ug_buildings = link;
	else if (!ug_units && !wcscmp(link->name(), L"Units"))
		ug_units = link;

	// FIXME: ugly casts
	addNode((Link*&)unitgroups, (Link*&)unitgroup_tail, link);
	unitgroup_count++;
}

void ESDATA::readCiv(const XML_Char **attrs)
{
	CivLink *link = new CivLink;

	while (*attrs)
	{
		if (link->read(*attrs, attrs[1]))
			attrs++;
		else
			attrs++;

		attrs++;
	}

	addNode((Link*&)civs, (Link*&)civ_tail, link);
	civ_count++;

	rstate.civ = link;
}

#define PF(n, f) { n, f }	//I just prefer this syntax
ESDATA::pFunc ESDATA::pfuncs[] = 
{
	PF(L"tech", &ESDATA::readTech),
	PF(L"color", &ESDATA::readColor),
	PF(L"protounit", &ESDATA::readUnit),
	PF(L"resources", &ESDATA::readResources),
	PF(L"simple", &ESDATA::readSimple),
	PF(L"aitypes", &ESDATA::readAitypes),
	PF(L"terrain", &ESDATA::readTerrain),
	PF(L"group", &ESDATA::readUnitGroup),
	PF(L"civ", &ESDATA::readCiv),
	PF(NULL, NULL)
};

void XMLCALL startHandler(void *userdata, const XML_Char *name, const XML_Char **attrs)
{
	ESDATA *esdata = (ESDATA*)userdata;
	ESDATA::pFunc *pf = esdata->pfuncs;

	while (pf->name)
	{
		if (!wcscmp(name, pf->name))
			break;
		pf++;
	}

/*	Now I know you'll look at this and think "omfg this looks so
	complex!", but it's really one of those things that looks more
	complicated to the coder than to the machine. The ASM is
	surprisingly simple. It's really much simpler than a bunch of
	if statements. */
	if (pf->name)
		(esdata->*(pf->f))(attrs);
}

/* End tags */

void ESDATA::endResources()
{
	rstate.pos = ESD_nowhere;
}

void ESDATA::endCiv()
{
	rstate.civ = NULL;
}

void XMLCALL endHandler(void *userdata, const XML_Char *name)
{
	ESDATA *esdata = (ESDATA *)userdata;

	if (!wcscmp(name, L"resources"))
		esdata->endResources();
}

void XMLCALL characterDataHandler(void *, const XML_Char *, int)
{
	// do nothing
}

void ESDATA::load(const char *path)
{
	XML_Parser parser;
	FILE *datafile;
	char buffer[XMLBUFF_SIZE];

	/* init */
	tech_count = 0;
	color_count = 0;
	unit_count = 0;
	res_count = 0;
	aitype_count = 0;
	terrain_count = 0;
	unitgroup_count = 0;
	civ_count = 0;
	rstate.pos = ESD_nowhere;
	rstate.civ = NULL;
	printf("\nReading esdata from \"%s\"...\n", path);

	parser = XML_ParserCreate(NULL);
	if (!parser)
		throw std::runtime_error("Failed to create parser.");

	XML_SetElementHandler(parser, startHandler, endHandler);
	XML_SetCharacterDataHandler(parser, characterDataHandler);
	XML_SetUserData(parser, this);

	datafile = fopen(path, "r");

	if (!datafile)
	{
		XML_ParserFree(parser);
		throw std::runtime_error("Failed to open file.");
	}

	while (true)
	{
		// Don't use readbin() because it's OK if we don't read the whole
		// thing.
		size_t read = fread(buffer, sizeof(char), XMLBUFF_SIZE, datafile);

		if (!XML_Parse(parser, buffer, read, (read != XMLBUFF_SIZE)))  // isFinal if we didn't read whole buff
		{
			XML_Error code = XML_GetErrorCode(parser);
			printf("Expat error code %d (line %d): %s\n",
				code, XML_GetCurrentLineNumber(parser), XML_ErrorString(code));

			fclose(datafile);
			XML_ParserFree(parser);

			throw std::runtime_error("XML Parse failed.");
		}

		if (read != XMLBUFF_SIZE)
			break;
	}

	fclose(datafile);

	printf("Loaded:\n"
		"%d technologies,\n"
		"%d colors,\n"
		"%d units,\n"
		"%d resources,\n"
		"%d aitypes,\n"
		"%d terrains,\n"
		"%d unitgroups,\n"
		"and %d civilizations.\n",
		tech_count, color_count, unit_count, res_count, aitype_count,
		terrain_count, unitgroup_count, civ_count);

	XML_ParserFree(parser);

	printf("Done.\n\n");
}

int ESDATA::getCount(enum ESD_GROUP group)
{
	switch (group)
	{
	case ESD_techs:
		return tech_count;
	case ESD_units:
		return unit_count;
	case ESD_colors:
		return color_count;
	case ESD_resources:
		return res_count;
	case ESD_aitypes:
		return aitype_count;
	case ESD_terrains:
		return terrain_count;
	}

	return 0;
}
