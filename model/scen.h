/*
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.

	scen.h -- wraps a scenario file in class Scenario and defines the interface.

	Notes:
	This file's code is platform-independant albeit windows.h is included for the bitmap structs.
	'u' stands for an unknown member. (There are a lot of these.)

	MODEL?
*/

#ifndef SCEN_H
#define SCEN_H

#include "datatypes.h"
#include "trigger.h"
#include "Player.h"

//MAC USERS: Please replace this block of code with whatever you need.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>	//for bitmaps, POINT, and RECT

#include <stdlib.h>	//for _MAX_FNAME and the like.
#include <time.h>		//for time_t typedef.

typedef struct _iobuf FILE;	//makes including <stdio.h> unnecessary

/* Options */

#define MAX_MAPSIZE		480	//maximum size of one side of a map
#define PLAYER1_INDEX	0	//index of Player 1 (may change)

#define UNREAD(n)

/** Structs **/

struct AOKFile
{
	char name[_MAX_FNAME];
	SString data;
};

/* Map */

#pragma pack(push, 1)	//set packing alignment to work with terrain array

class Map	//wraps all data regarding the scenario map
{
public:
//in scenario

	long aitype;	//Map type for AI recognition. (see aitypes[])
	unsigned long x, y;	//Should always be equal, but are stored separately in the file.

	struct Terrain		//This is one tile of terrain.
	{
		Terrain();		//sets cnst and elev to default values.

		unsigned char cnst;	//terrain tile constant. (see esdata)
		unsigned char elev;	//terrain elevation. (0-8, I believe)

	} terrain[MAX_MAPSIZE][MAX_MAPSIZE];	//the terrain tile arrays

//internal

	Map();

	void reset();
	void read(FILE *in, ScenVersion v);
	void write(FILE *out, ScenVersion v);

	/*	readArea: copies terrain data from a buffer to the specified area */
	bool readArea(Buffer &from, const RECT &area);

	/*	writes a specified area of the map to a buffer

		area: inclusive area to copy

		returns: true if successful
	*/
	bool writeArea(Buffer &b, const RECT &area);
};

#pragma pack(4)

struct Victory
{
	enum Modes
	{
		MODE_Standard, MODE_Conquest, MODE_Score, MODE_Time, MODE_Custom
	};

	long conq, z1, relics, z2, expl, z3;
	long all, mode, score, time;	//really bools, just stored as longs in-file
};

/*	MapCopyCache: caches data in-between Scenario::map_size() and Scenario::
	map_copy() so we don't have to do some loops twice. */
class MapCopyCache;

/* The Scenario Wrapper */

#define NUM_UNK 32

class Scenario
{
	static const long sect = 0xFFFFFF9D;

// Private structs
	struct AOKBMP
	{
		BITMAPFILEHEADER file_hdr;
		BITMAPINFOHEADER info_hdr;
		RGBQUAD *colors;
		char *image;

		AOKBMP();
		~AOKBMP();

		void read(FILE *in);
		void write(FILE *out);
		bool toFile(const char *path);
		void reset();
	};

	const PerVersion *perversion;	//yes, I know that's a word. :-P

	static const PerVersion pv1_18;
	static const PerVersion pv1_21;
	static const PerVersion pv1_22;
	static const PerVersion pv1_23;
	static const PerVersion pv1_30;

//	Internal Status Stuff (aka not in the SCX)
	bool mod_status;

	/**
	 * Reads decompressed scenario data from specified path.
	 */
	void read_data(const char *path);

	/**
	 * Writes to-be-compressed scenario data to the specified path.
	 */
	int write_data(const char *path);

public:
	Scenario();
	~Scenario();
	static char StandardAI[];

	char msg[_MAX_FNAME + 100];	//contains any error message reported by a function.

	/* The scenario */

	// Internal use, set according to header::version[]
	ScenVersion ver;

//	Un-compressed Header
	struct _header
	{
		char version[5];		//Version string (+ 1 for NULL)
		long length;			//length of header (after this point)
		__time32_t timestamp;	//last save
		//left out instructions in header
		UNREAD(long pcount);

		void reset();
		bool read(FILE *scx);	//error return
		void write(FILE *scx, const SString *instr, long players);
	} header;

//	Compression starts here
	UID	next_uid;
	float	ver2;
	long	unknown;	//usually 1?
	char	origname[_MAX_FNAME];
	long	mstrings[NUM_MSGS];
	SString messages[NUM_MSGS];
	char	cinem[NUM_CINEM][0x20];	//pre-game, victory, lose, bkg
	long	bBitmap;
	AOKBMP bitmap;
	SString unk[NUM_UNK];	//probably part of PlayerData2
	Player players[NUM_PLAYERS];
	Victory vict;
	long dis_bldgx;
	long all_techs;
	Map map;
	float	editor_pos[2];
	char    unk2;     // might be related to triggers?
	SVector <Trigger> triggers;
	std::vector<unsigned long> t_order;

	long unk3; // TODO: rename
	long unk4;
	long cFiles;
	AOKFile *files;

	void open(const char *path, const char *dpath);
	int save(const char *path, const char *dpath, bool write);
	void reset();
	bool export_bmp(const char *path);
	//exFile: Exports all (index of -1) or one file to the specified directory
	bool exFile(const char *directory, long index);

	/*
		clean_triggers: cleans trigger list of unused triggers

		Note: Necessary because the UI only deletes the t_order entry, not the trigger itself.
	*/
	void clean_triggers();

	/*
		insert_trigger: Inserts a copy of a trigger into the trigger list after
		a specified trigger.

		@param t The trigger to copy.
		@param after The index of the preceding trigger.

		Returns the index of the inserted trigger in the trigger vector.

		Note: This both appends the trigger to the trigger list and then
		inserts it into t_order.
	*/
	size_t insert_trigger(Trigger *t, size_t after);

	/**
	 * Applies a TriggerVisitor to all triggers in the scenario.
	 */
	void accept(TriggerVisitor&);

	/*	map_size: returns size of memory needed to copy rectangle.
	
		WARNING: This function assumes source is a valid rectangle, ie. (top < bottom &&
		left < right).
	
		We need to do this since the app has to allocate the memory using Win API
		functions for clipboard.

		Note: The returned MapCopyCache object is deleted in map_copy().
	*/
	int map_size(const RECT &source, MapCopyCache *&mcc);

	/*	map_copy: copies terrain and units from a specified area of the map
		to a Buffer.

		Returns: Error code

		Note: MapCopyCache is deleted by this function.
	*/
	AOKTS_ERROR map_copy(Buffer &to, const MapCopyCache *mcc);

	/*	map_paste: "pastes" terrain and units from a buffer to a specified point.

		Note: The buffer should have been filled by map_copy.
	*/
	AOKTS_ERROR map_paste(const POINT &to, Buffer &from);

	int getPlayerCount();

	bool needsave()	{ return mod_status; }

	bool isExpansion();
};

#undef UNREAD

#pragma pack(pop)	//restore default packing alignment

#endif //SCEN_H
