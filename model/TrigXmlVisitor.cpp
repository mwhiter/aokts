#include "TrigXmlVisitor.h"
#include "../util/utilio.h"

#include <string>
#include <algorithm>

using std::string;

string escape(const char * str)
{
	static const std::string lt = "&lt;";
	static const std::string gt = "&gt;";
	static const std::string amp = "&amp;";

	// Use a string as a buffer.
	string s;

	// We'll need at least the length of the string
	s.reserve(strlen(str));

	// Copy string character-by-character so that we can replace special XML
	// chars: <>&
	for (; *str; ++str)
	{
		switch (*str)
		{
		case '<':
			s.insert(s.end(), lt.begin(), lt.end());
			break;
		case '>':
			s.insert(s.end(), gt.begin(), gt.end());
			break;
		case '&':
			s.insert(s.end(), amp.begin(), amp.end());
			break;
		default:
			s.push_back(*str);
		}
	}

	return s;
}

/**
 * Writes specified value with specified tag, if value != -1.
 */
inline void writeXmlLong(FILE * out, const char * tag, long value)
{
	if (value != -1)
		fprintf(out, "\t\t<%s>%d</%s>\n", tag, value, tag);
}

inline void writeXmlString(FILE * out, const char * tag, const SString& str)
{
	if (str.length())
		fprintf(out, "\t\t<%s>%s</%s>\n",
			tag, escape(str.c_str()).c_str(), tag);
}

inline void writeXml(FILE * out, const char * tag, const AOKRECT& rect)
{
	if (rect.top != -1) // just assume the rest is
	{
		fprintf(out, "\t\t<%s_ur>%d,%d</%s_ur>\n", tag, rect.right, rect.top, tag);
		fprintf(out, "\t\t<%s_ll>%d,%d</%s_ll>\n", tag, rect.left, rect.bottom, tag);
	}
}

inline void writeXml(FILE * out, const char * tag, const AOKPT& pt)
{
	if (pt.x != -1) // just assume the rest is
		fprintf(out, "\t\t<%s>%d,%d</%s>\n", tag, pt.x, pt.y, tag);
}


TrigXmlVisitor::TrigXmlVisitor(AutoFile& af)
: _af(af), _trigcount(0)
{
	static const char * xmlheader =
		"<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";

	fputs(xmlheader, _af.get());
	fputs("<triggers>\n", _af.get());
}

TrigXmlVisitor::~TrigXmlVisitor()
{
	fputs("</triggers>\n", _af.get());
}

void TrigXmlVisitor::visit(Trigger& t)
{
	FILE * out = _af.get();

	fprintf(out, "<trigger id=\"%u\">\n", _trigcount++);
	fprintf(out, "\t<name>%s</name>\n",
		escape(t.name).c_str());
	fprintf(out, "\t<enabled>%d</enabled>\n", t.state);
	fprintf(out, "\t<looping>%d</looping>\n", t.loop);
	fprintf(out, "\t<objective>%d</objective>\n", t.obj);
	fprintf(out, "\t<desc_order>%d</desc_order>\n", t.obj_order);
	fprintf(out, "\t<display_order>%d</display_order>\n", t.display_order);
	if (t.description.length())
		fprintf(out, "\t<description>%s</description>\n",
			escape(t.description.c_str()).c_str());
}

void TrigXmlVisitor::visit(Effect& e)
{
	FILE * out = _af.get();

	fputs("\t<effect>\n", out);

	fprintf(out, "\t\t<type>%s</type>\n", e.getName().c_str());

	writeXmlLong(out, "ai_goal", e.ai_goal);
	writeXmlLong(out, "amount", e.amount);
	writeXmlLong(out, "resource", e.res_type);
	writeXmlLong(out, "diplomacy", e.diplomacy);
	writeXmlLong(out, "num_selected", e.num_sel); // TODO: don't write this
	writeXmlLong(out, "unit_ids", e.num_sel > 0 ? e.uids[0] : -1);
	writeXmlLong(out, "location_unit", e.uid_loc);
	if (e.pUnit)
		writeXmlLong(out, "unit_type", e.pUnit->id());
	writeXmlLong(out, "player_source", e.s_player);
	writeXmlLong(out, "player_target", e.t_player);
	if (e.pTech)
		writeXmlLong(out, "technology", e.pTech->id());
	writeXmlLong(out, "textid", e.textid);
	writeXmlLong(out, "display_time", e.disp_time);
	writeXmlLong(out, "trigger", e.trig_index);
	writeXml(out, "location", e.location);
	writeXml(out, "area", e.area);
	writeXmlLong(out, "unit_group", e.group);
	writeXmlLong(out, "building_type", e.utype);
	writeXmlLong(out, "instruction_panel", e.panel);
	writeXmlString(out, "text", e.text);
	writeXmlString(out, "sound_file", e.sound);

	fputs("\t</effect>\n", out);
}

void TrigXmlVisitor::visit(Condition& c)
{
	FILE * out = _af.get();

	fputs("\t<condition>\n", out);

	fprintf(out, "\t\t<type>%s</type>\n", c.getName().c_str());

	writeXmlLong(out, "amount", c.amount);
	writeXmlLong(out, "resource", c.res_type);
	writeXmlLong(out, "unit_object", c.object);
	writeXmlLong(out, "unit_location", c.u_loc);
	if (c.pUnit)
		writeXmlLong(out, "unit_type", c.pUnit->id());
	writeXmlLong(out, "player", c.player);
	if (c.pTech)
		writeXmlLong(out, "technology", c.pTech->id());
	writeXmlLong(out, "timer", c.timer);
	writeXml(out, "area", c.area);
	writeXmlLong(out, "unit_group", c.group);
	writeXmlLong(out, "building_type", c.utype);
	writeXmlLong(out, "ai_signal", c.ai_signal);

	fputs("\t</condition>\n", out);
}

void TrigXmlVisitor::visitEnd(Trigger&)
{
	fputs("</trigger>\n", _af.get());
}
