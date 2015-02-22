/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	scen.cpp -- Scenario wrapper code

	MODEL?
**/

#include "scen.h"

#include "../util/zlibfile.h"
#include "../view/utilui.h"
#include "../util/utilio.h"
#include "../util/settings.h"
#include "../util/Buffer.h"

#include <direct.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <iterator>

using std::vector;
using std::pair;

/* some options */
#define BMP_READ_HACK

/* See scen_const.h
struct PerVersion
{
	int messages_count;
	bool mstrings;
	int max_disables1; // max disable tech and unit
	int max_disables2; // max disable buildings
	UCNST max_unit; // max unit types
	int max_research; // max research
	int max_tech; // max tech
	int max_terrains; // max tech
}; */

/* PerVersions */
// AOE
const PerVersion Scenario::pv1_15 =
{
	5,
	true,
	30, 20,
	374,
	118,
	140,
	32
};

// AOK
const PerVersion Scenario::pv1_18 =
{
	5,
	true,
	30, 20,
	750,
	426,
	438,
	32
};

// AOC
const PerVersion Scenario::pv1_22 =
{
	6,
	true,
	30, 20,
	866,
	460,
	514,
	42
};

//// AOFE
//const PerVersion Scenario::pv1_22 =
//{
//	6,
//	true,
//	30, 20,
//	866, // the true value of this number is very importint!
//	460,
//	514,
//	42
//};

// AOHD
const PerVersion Scenario::pv1_23 =
{
	6,
	true,
	30, 20,
	865,
	459,
	513,
	41
};

// SWGB
const PerVersion Scenario::pv1_30 =
{
	6,
	true,
	60, 60,
	866, // install SWGB and use AGE to find the these values
	460,
	514,
	42
};

/* The Scenario */

#define SMSG	" For stability\'s sake, I must discontinue reading the scenario." \
				"\nPlease contact cyan.spam@gmail.com regarding this error."

const char e_none[]	= "Scenario operation completed successfully.";
const char e_compr[]	= "Zlib compression/decompression failed.";
const char e_mem[]	= "I ran out of memory. This probably means "\
	"that the scenario is invalid or that I got out of sync.";
const char e_zver[]	= "Your version of zlib1.dll is incompatible. I need at least %s. ";
const char e_digit[]	= "Oops, my programmer (David Tombs) made an error.\nProblem: %s" SMSG;
const char e_bitmap[]	= "Sorry, the bitmap data in this scenario appears incomplete." SMSG;

inline int myround(float n)
{
	return int((n >= 0.0) ? n + 0.5 : n - 0.5);
}

Scenario::Scenario()
:	mod_status(false),
	next_uid(0), bBitmap(false), files(NULL)
{
//	strcpy(mark, "DGT");
	strcpy(players[GAIA_INDEX].name, "GAIA");
}

Scenario::~Scenario()
{
	delete [] files;
}

void Scenario::reset()
{
	int i;

	/* Internal */
	mod_status = false;
	ver1 = SV1_AOE2TC;
	ver2 = SV2_AOE2TC;

	/* "Blank" scenario */
	strcpy(header.version, "1.21");

	perversion = &pv1_22; // no disables dialog doesn't crash

	next_uid = 0;
	version2 = 1.22F;
	memset(origname, 0, sizeof(origname));

	for (i = 0; i < 6; i++)
	{
		messages[i].erase();
		mstrings[i] = -1;
	}

	memset(cinem, 0, sizeof(cinem));
	bBitmap = false;

	lock_teams = 0;
	for (i = 0; i < NUM_PLAYERS; i++)
		players[i].reset();
	players[0].enable = true;
	players[0].human = true;
	Player::num_players = EC_NUM_PLAYERS;

	memset(&vict, 0, sizeof(vict));
	map.reset();

	trigver = 1.6;
	objstate = 0;
	triggers.clear();
	t_order.clear();

	files = NULL;

	memset(msg, 0, sizeof(msg));
}

Scenario::AOKBMP::AOKBMP()
:	colors(NULL), image(NULL)
{
	//prep fileheader
	file_hdr.bfType = 0x4D42;	//"MB"
	file_hdr.bfReserved1 = 0;
	file_hdr.bfReserved2 = 0;

	/* these are set later when we read the BMP from the scenario */
	file_hdr.bfSize = 0;
	file_hdr.bfOffBits = 0;
}

Scenario::AOKBMP::~AOKBMP()
{
	delete [] colors;
	delete [] image;
}

void Scenario::AOKBMP::read(FILE *in)
{
	readbin(in, &info_hdr);

#ifdef BMP_READ_HACK
	if (info_hdr.biClrUsed == 0)
		info_hdr.biClrUsed = 256;
	info_hdr.biSizeImage = info_hdr.biWidth * info_hdr.biHeight;
#endif

	file_hdr.bfOffBits = (sizeof(BITMAPFILEHEADER) + info_hdr.biSize + info_hdr.biClrUsed * sizeof(RGBQUAD));
	file_hdr.bfSize = (file_hdr.bfOffBits + info_hdr.biSizeImage);

	colors = new RGBQUAD[info_hdr.biClrUsed];
	fread(colors, sizeof(RGBQUAD), info_hdr.biClrUsed, in);

	image = new char[info_hdr.biSizeImage];
	fread(image, sizeof(char), info_hdr.biSizeImage, in);
}

void Scenario::AOKBMP::write(FILE *out)
{
	fwrite(&info_hdr, sizeof(info_hdr), 1, out);
	fwrite(colors, sizeof(RGBQUAD), info_hdr.biClrUsed, out);
	fwrite(image, sizeof(char), info_hdr.biSizeImage, out);
}

bool Scenario::AOKBMP::toFile(const char *path)
{
	FILE *bmp_out;

	if (!file_hdr.bfSize)
		return false;

	bmp_out = fopen(path, "wb");
	if (!bmp_out)
		return false;

	fwrite(&file_hdr, sizeof(BITMAPFILEHEADER), 1, bmp_out);
	fwrite(&info_hdr, sizeof(BITMAPINFOHEADER), 1, bmp_out);
	fwrite(colors, sizeof(RGBQUAD), info_hdr.biClrUsed, bmp_out);
	fwrite(image, info_hdr.biSizeImage, 1, bmp_out);

	fclose(bmp_out);
	return true;
}

void Scenario::AOKBMP::reset()
{
	file_hdr.bfSize = 0;
	delete [] colors;
	delete [] image;
}

bool Scenario::isAok() //The Age of Kings
{
	bool ret = false;

	if (ver1 == SV1_AOE2)
		ret = true;

	return ret;
}

bool Scenario::isScx() //The Conquerors
{
	bool ret = false;

	if (ver1 == SV1_AOE2TC)
		ret = true;

	return ret;
}

bool Scenario::isScx2() //The Forgotten
{
	bool ret = false;

	if (ver1 == SV1_AOE2TC && ver2 == SV2_AOE2TF)
		ret = true;

	return ret;
}

char Scenario::StandardAI[] = "RandomGame";
char Scenario::StandardAI2[] = "Promisory";

/* Open a scenario, read header, and read compressed data */

void Scenario::open(const char *path, const char *dpath)
{
	using std::runtime_error;
	using std::logic_error;

	int clen;	//length of compressed data

	printf("Opening scenario \"%s\"\n", path);

	/* Initialization */

	mod_status = false;
	memset(msg, 0, sizeof(msg));

	AutoFile scx(path, "rb");

	/* Header */
	if (!header.read(scx.get()))
	{
		printf("Invalid scenario header in %s\n", path);
		throw runtime_error("Not a valid scenario.");
	}

	if (!strncmp(header.version, "1.18", 4))
	{
		ver1 = SV1_AOE2;
	}
	else if (!strncmp(header.version, "1.21", 4))
	{
		ver1 = SV1_AOE2TC;
	}
	else if (!strncmp(header.version, "1.30", 4))
	{
		ver1 = SV1_AOE2TC;
	}
	else if (header.version[2] == '1' &&
		(header.version[3] == '0' || header.version[3] == '1')) // 1.10 or 1.11
	{
		ver1 = SV1_AOE1;
	}
	else
	{
		printf("Unrecognized scenario version: %s.\n", header.version);
		throw bad_data_error("unrecognized format version");
	}

	/* Inflate and Read Compressed Data */
	clen = fsize(path) - (header.length + 8);	//subtract header length

	if (clen <= 0)
		throw bad_data_error("no compressed data");

	printf("Allocating %d bytes for compressed data buffer.\n", clen);

	// Open file before allocating buffer since file is more likely to fail.
	AutoFile tempout(dpath, "wb"); // TODO: exclusive access?

	unsigned char *compressed = new unsigned char[clen];
	fread(compressed, sizeof(char), clen, scx.get());
	int code = inflate_file(compressed, clen, tempout.get());
	delete [] compressed;

	tempout.close();

	// throw exception if zlib returned an error
	switch (code)
	{
	case Z_STREAM_ERROR:
		throw logic_error("internal logic fault: zlib stream error");

	case Z_DATA_ERROR:
		throw bad_data_error("invalid compressed data");

	case Z_MEM_ERROR:
		throw std::bad_alloc("not enough memory for decompression");

	case Z_BUF_ERROR:
		throw logic_error("internal logic fault: zlib buffer error");

	case Z_VERSION_ERROR:
		throw runtime_error("Your version of zlib1.dll is incompatible. I need " ZLIB_VERSION);
	}

	read_data(dpath);
}

/* Open a destination scenario, write the header, and optionally write compressed data */

int Scenario::save(const char *path, const char *dpath, bool write, int convert, SaveFlags::Value flags)
{
	size_t uc_len;
	int code = 0;	//return from zlib functions

	AutoFile scx(path, "wb");

	switch (convert)
	{
		case 1:
			ver1 = SV1_AOE2;
			ver2 = SV2_AOE2;
			if (flags & SaveFlags::CONVERT_AOK)
			    aoc_to_aok();
			break;
		case 2:
			ver1 = SV1_AOE2TC;
			ver2 = SV2_AOE2TC;
			if (flags & SaveFlags::CONVERT_EFFECTS)
			    hd_to_up();
			if (flags & SaveFlags::CONVERT_AOK)
			    aok_to_aoc();
			break;
		case 3:
			ver1 = SV1_AOE2TC;
			ver2 = SV2_AOE2TF;
			if (flags & SaveFlags::CONVERT_EFFECTS)
			    up_to_hd();
			if (flags & SaveFlags::CONVERT_AOK)
			    aok_to_aoc();
			break;
		case 4:
			ver1 = SV1_SWGB;
			ver2 = SV2_SWGB;
			if (flags & SaveFlags::CONVERT_AOK)
			    aok_to_aoc();
			break;
	}

	/* Header */
	header.write(scx.get(), messages, getPlayerCount(), ver1);

	/* Write uncompressed data to a temp file */
	if (write)
		write_data(dpath);

	/* Then compress and write it to the scenario */
	uc_len = fsize(dpath);
	// Open file before buffer alloc since it's more likely to fail.
	AutoFile temp(dpath, "rb");
	// FIXME: don't allocate this ridiculous thing
	unsigned char *uncompressed = new unsigned char[uc_len];
	fread(uncompressed, sizeof(unsigned char), uc_len, temp.get());
	temp.close();

	code = deflate_file(uncompressed, uc_len, scx.get());

	/* translate zlib codes to AOKTS_ERROR codes */
	switch (code)
	{
	case Z_OK:
	case Z_STREAM_END:
		code = ERR_none;
		break;

	case Z_STREAM_ERROR:
		code = ERR_digit;
		sprintf(msg, e_digit, "zlib stream error");
		break;

	case Z_MEM_ERROR:
		code = ERR_mem;
		printf("Could not allocate memory for compression stream.\n");
		strcpy(msg, e_mem);
		break;

	case Z_BUF_ERROR:
		code = ERR_digit;
		sprintf(msg, e_digit, "zlib buffer error.");
		break;

	case Z_VERSION_ERROR:
		code = ERR_zver;
		sprintf(msg, e_zver, ZLIB_VERSION);
		break;
	default:
		code = ERR_unknown;
		strcpy(msg, "Unknown error!");
		break;
	}

	delete [] uncompressed;
	fflush(stdout);	//report errors to logfile

	return code;
}

bool Scenario::_header::read(FILE *scx)
{
	bool ret = true;
	long check;
	unsigned long len;	//Instructions string length

	fread(version, sizeof(char), 4, scx);

	fread(&length, sizeof(long), 1, scx);

	fread(&check, sizeof(long), 1, scx);
	REPORTS(check == 2, ret = false, "Header check value invalid.\n");

	fread(&timestamp, sizeof(timestamp), 1, scx);

	fread(&len, sizeof(long), 1, scx);
	SKIP(scx, len);	//instructions

	SKIP(scx, sizeof(long) * 2);	//unknown & playercount

	return ret;
}

void Scenario::_header::write(FILE *scx, const SString *instr, long players, ScenVersion1 v)
{
	long num;

	switch (v)
	{
	case SV1_AOE2:
		strcpy(version, "1.18");
		break;
	case SV1_AOE2TC:
		strcpy(version, "1.21");
		break;
	case SV1_SWGB:
		strcpy(version, "1.30");
		break;
	default:
		strcpy(version, "0.00");
		break;
	}
	fwrite(version, sizeof(char), 4, scx);

	/* Length calculation is a little tricky */
	length = 0x14 + instr->lwn();
	fwrite(&length, sizeof(long), 1, scx);

	num = 2;
	fwrite(&num, sizeof(long), 1, scx);

	fwrite(&timestamp, sizeof(timestamp), 1, scx);

	instr->write(scx, sizeof(long));

	NULLS(scx, sizeof(long));

	fwrite(&players, sizeof(long), 1, scx);
}

void Scenario::_header::reset()
{
	memset(version, 0, sizeof(version));
	length = 0;
	_time32(&timestamp);
}

/*
	FEP = For Each Player.

	Requires an int i. Takes p from player 1 through player 16.
*/
#define FEP(p) \
	for (i = PLAYER1_INDEX, p = players; i < NUM_PLAYERS; i++, p++)

/**
 * I suppose this is as good a place as any to document how I designed the
 * reading and writing of scenario data. The idea is that each class knows how
 * to read and write its own data. Unforunately, this is complicated by the
 * scenario format having data spread everywhere (e.g., Player Data 1, 2, 3,
 * 4). So, the rule boils down to:
 *
 * The class that contains the destination data member will read and write that
 * data member.
 *
 * If I were starting over from scratch, I'd probably design a separate class
 * just to do reading and writing, but I'm not. :)
 */

void Scenario::read_data(const char *path)	//decompressed data
{
	int i;
	Player *p;

	printf("Read decompressed data file %s...\n", path);
	AutoFile dc2in(path, "rb"); // temp decompressed file

	/* Compressed Header */

	readbin(dc2in.get(), &next_uid);
	readbin(dc2in.get(), &version2);

	if (setts.intense)
		printf("Debug 1.\n");

	switch (myround(version2 * 100))
	{
	case 115:
		perversion = &pv1_15;
		ver2 = SV2_AOE1;
		break;

	case 118:
	case 119:
	case 120:
	case 121:
		perversion = &pv1_18;
		ver2 = SV2_AOE2;
		break;

	case 122:
		perversion = &pv1_22;
		ver2 = SV2_AOE2TC;
		break;

	case 123:
		perversion = &pv1_23;
		ver2 = SV2_AOE2TF;
		break;

	case 130:
		perversion = &pv1_30;
		ver2 = SV2_SWGB;
		break;

	default:
		printf("Unrecognized scenario version2: %f.\n", version2);
		throw bad_data_error("unrecognized format version");
	}

	FEP(p)
		p->read_header_name(dc2in.get());

	if (ver1 >= SV1_AOE2)
	{
		FEP(p)
			p->read_header_stable(dc2in.get());
	}

	FEP(p)
		p->read_data1(dc2in.get());

	if (setts.intense)
		printf("Debug 2.\n");

	readbin(dc2in.get(), &unknown);
	readunk(dc2in.get(), (char)0, "post-PlayerData1 char");
	readunk(dc2in.get(), -1.0F, "unknown3");

	readcs<unsigned short>(dc2in.get(), origname, sizeof(origname));

	/* Messages & Cinematics */
	if (perversion->mstrings)
		fread(mstrings, sizeof(long), perversion->messages_count, dc2in.get());

	for (i = 0; i < perversion->messages_count; i++)
		messages[i].read(dc2in.get(), sizeof(short));

	for (i = 0; i < NUM_CINEM; i++)
		readcs<unsigned short>(dc2in.get(), cinem[i], sizeof(cinem[i]));

	/* Bitmap */
	readbin(dc2in.get(), &bBitmap);
	SKIP(dc2in.get(), sizeof(long) * 2);	//x, y (duplicate)
	readunk<short>(dc2in.get(), bBitmap ? -1 : 1,
		"bitmap unknown");

	if (bBitmap)
		bitmap.read(dc2in.get());

	/* Player Data 2 */

	if (setts.intense)
		printf("Debug 3.\n");

	for (i = 0; i < NUM_UNK; i++)
		unk[i].read(dc2in.get(), sizeof(short));

	FEP(p)
		readcs<unsigned short>(dc2in.get(), p->ai, sizeof(p->ai));

	FEP(p)
		p->read_aifile(dc2in.get());

	FEP(p)
		p->read_aimode(dc2in.get());

	readunk(dc2in.get(), sect, "Resources sect begin", true);

	FEP(p)
		p->read_resources(dc2in.get());

	/* Global Victory */

	readunk(dc2in.get(), sect, "Global victory sect begin", true);

	readbin(dc2in.get(), &vict);

	/* Diplomacy */

	FEP(p)
		p->read_diplomacy(dc2in.get());

	SKIP(dc2in.get(), 0x2D00);	//should I check this? probably.

	readunk(dc2in.get(), sect, "diplomacy sect middle", true);

	SKIP(dc2in.get(), sizeof(long) * NUM_PLAYERS);	//other allied victory

	if (ver2 == SV2_AOE2TF || ver2 == SV2_SWGB)
		readbin(dc2in.get(), &lock_teams);

	/* Disables */

	// UGH why not just one big struct for each player?!
	FEP(p)
		p->read_ndis_techs(dc2in.get());
	FEP(p)
		p->read_dis_techs(dc2in.get(), *perversion);
	FEP(p)
		p->read_ndis_units(dc2in.get());
	FEP(p)
		p->read_dis_units(dc2in.get(), *perversion);
	FEP(p)
		p->read_ndis_bldgs(dc2in.get());
	FEP(p)
		p->read_dis_bldgs(dc2in.get(), *perversion);

	if (setts.intense)
		printf("Debug 4.\n");

	readunk<long>(dc2in.get(), 0, "Disables unused 1", true);
	readunk<long>(dc2in.get(), 0, "Disables unused 2", true);
	readbin(dc2in.get(), &all_techs);
	FEP(p)
		p->read_age(dc2in.get());

	/* Map */

	readunk(dc2in.get(), sect, "map sect begin", true);

	players[0].read_camera_longs(dc2in.get());
	map.read(dc2in.get(), ver1);

	/* Population Limits & Units */

	readunk<long>(dc2in.get(), 9, "Player count before units", true);

	for (i = PLAYER1_INDEX; i < GAIA_INDEX; i++)
		players[i].read_data4(dc2in.get(), ver1);

	// GAIA first! Grrrr.
	players[GAIA_INDEX].read_units(dc2in.get());

	for (i = PLAYER1_INDEX; i < 8; i++)
		players[i].read_units(dc2in.get());

	/* Player Data 3 */

	// TODO: cleanup stopped here

	readunk<long>(dc2in.get(), 9, "Player count before PD3", true);

	for (i = PLAYER1_INDEX; i < 8; i++)
	{
		players[i].read_data3(dc2in.get(),
			i == PLAYER1_INDEX ? editor_pos : NULL	//See scx_format.txt.
			);
	}

	if (setts.intense)
		printf("Debug 5.\n");
	/* Triggers */

	readunk<double>(dc2in.get(), 1.6, "Trigger system version");
	readbin(dc2in.get(), &objstate);

	if (setts.intense)
		printf("Debug 9.\n");
	/* Triggers */

	unsigned long n_trigs = readval<unsigned long>(dc2in.get());
	triggers.resize(n_trigs);

	if (setts.intense)
		printf("Debug 10.\n");
	if (n_trigs)
	{
		Trigger *t = &(*triggers.begin());

		//read triggers
		for (unsigned int i = 0; i < n_trigs; ++i)
		{
			if (setts.intense)
				printf("Reading trigger %d.\n", i);
			t++->read(dc2in.get());
		}

		// Directly read trigger order: this /is/ allowed by std::vector.
		t_order.resize(n_trigs);
		readbin(dc2in.get(), &t_order.front(), n_trigs);

		// save the trigger display order to the trigger objects
		for (unsigned int j = 0; j < n_trigs; j++) {
			//printf("%hu\n",j);
			triggers.at(t_order[j]).display_order = j;
		}

		// verify just the first one because I'm lazy
		if (t_order.front() > n_trigs)
			throw bad_data_error("Trigger order list corrupted. Possibly out of sync.");
	}

    if (ver2 == SV2_AOE2) {
        // this code is from aokts-0.3.6, which could open aok files
	    /* Included Files */
	    fread(&unk3, sizeof(long), 2, dc2in.get());	//unk3, unk4

	    if (unk3 == 1 && unk4 == 1)
		    fseek(dc2in.get(), 396, SEEK_CUR);
	} else {
	    /* Included Files */
	    readbin(dc2in.get(), &unk3);
	    readbin(dc2in.get(), &unk4); // aok files sometimes fail here

	    if (unk4 == 1)
	    {
		    printf("unk4 hack around %x\n", ftell(dc2in.get()));
		    SKIP(dc2in.get(), 396);
	    }
	}

	if (unk3 == 1)
	{
		readbin(dc2in.get(), &cFiles);

		if (cFiles > 0)
		{
			files = new AOKFile[cFiles];

			for (i = 0; i < cFiles; i++)
			{
				readcs<unsigned long>(dc2in.get(),
					files[i].name, sizeof(files[i].name));
				files[i].data.read(dc2in.get(), sizeof(long));
			}
		}
	}

	if (fgetc(dc2in.get()) != EOF)
		throw bad_data_error("Unrecognized data at end.");

	// FILE close taken care of by AutoFile! yay.

	printf("Done.\n");
}

/* Write to-be-compressed data to a temp file */

int Scenario::write_data(const char *path)
{
	FILE *dcout;
	int i;
	long num;
	float f;
	int num_players = Player::num_players;	//for quick access
	Player *p;

	dcout = fopen(path, "wb"); // FIXME: error handling?

	/* Compressed header */

	writebin(dcout, &next_uid);

	switch (ver2)
	{
		case SV2_AOE2:
			version2 = 1.20F;
			break;
		case SV2_AOE2TC:
			version2 = 1.22F;
			break;
		case SV2_AOE2TF:
			version2 = 1.23F;
			break;
		case SV2_SWGB:
			version2 = 1.30F;
			break;
		default:
			version2 = 0.00F;
			break;
	}
	writebin(dcout, &version2);

	FEP(p)
		p->write_header_name(dcout);

	if (ver1 >= SV1_AOE2)
	{
	    FEP(p)
		    p->write_header_stable(dcout);
	}

	FEP(p)
		p->write_data1(dcout);

	fwrite(&unknown, sizeof(long), 1, dcout);
	putc(0, dcout);
	f = -1;
	fwrite(&f, 4, 1, dcout);
	writecs<unsigned short>(dcout, origname, false);

	/* Messages & Cinematics */
	if (perversion->mstrings)
		fwrite(mstrings, sizeof(long), perversion->messages_count, dcout);

	for (i = 0; i < perversion->messages_count; i++)
		messages[i].write(dcout, sizeof(short));

	for (i = 0; i < NUM_CINEM; i++)
	{
		writecs<unsigned short>(dcout, cinem[i], false);
	}

	fwrite(&bBitmap, 4, 1, dcout);
	fwrite(&bitmap.info_hdr.biWidth, 8, 1, dcout);	//width, height
	if (!bBitmap)
	{
		num = 1;
		fwrite(&num, 2, 1, dcout);
	}
	else
	{
		writeval(dcout, (short)-1);
		bitmap.write(dcout);
	}
	NULLS(dcout, 0x40);

	/* Player Data 2 */

	FEP(p)
	{
		writecs<unsigned short>(dcout, p->ai, false);
	}
	FEP(p)
	{
		NULLS(dcout, 8);
		p->aifile.write(dcout, sizeof(unsigned long));
	}
	FEP(p)
		putc(p->aimode, dcout);

	writeval(dcout, sect);
	FEP(p)
		fwrite(&p->resources, sizeof(long), 6, dcout);

	/* Global Victory */

	writeval(dcout, sect);
	fwrite(&vict, sizeof(vict), 1, dcout);

	/* Diplomacy */

#ifndef NOWRITE_D1
	FEP(p)
		fwrite(p->diplomacy, sizeof(long), NUM_PLAYERS, dcout);
#else
	NULLS(dcout, sizeof(long) * NUM_PLAYERS * NUM_PLAYERS);
#endif

	NULLS(dcout, 0x2D00);	//lol

	writeval(dcout, sect);

	//2nd allied victory
	FEP(p)
	{
		num = p->avictory;
		fwrite(&num, sizeof(long), 1, dcout);
	}

	if (ver2 == SV2_AOE2TF)
		fwrite(&lock_teams, sizeof(long), 1, dcout);

	/* Disables */

	FEP(p)
		fwrite(&p->ndis_t, sizeof(long), 1, dcout);
	FEP(p)
		fwrite(&p->dis_tech, sizeof(long), perversion->max_disables1, dcout);
	FEP(p)
		fwrite(&p->ndis_u, sizeof(long), 1, dcout);
	FEP(p)
		fwrite(&p->dis_unit, sizeof(long), perversion->max_disables1, dcout);
	FEP(p)
		fwrite(&p->ndis_b, sizeof(long), 1, dcout);
	FEP(p)
		fwrite(&p->dis_bldg, sizeof(long), perversion->max_disables2, dcout);

	NULLS(dcout, 0x8);
	fwrite(&all_techs, sizeof(long), 1, dcout);
	FEP(p)
		fwrite(&p->age, 4, 1, dcout);

	/* Map */

	writeval(dcout, sect);
	num = (long)players[0].camera[1];
	fwrite(&num, 4, 1, dcout);
	num = (long)players[0].camera[0];
	fwrite(&num, 4, 1, dcout);
	map.write(dcout, ver1);

	/* Population Limits & Units */

	fwrite(&num_players, 4, 1, dcout);
	for (i = PLAYER1_INDEX; i < num_players - 1; i++)
	{
		float resources[6];
		resources[0] = (float)players[i].resources[2];	//food
		resources[1] = (float)players[i].resources[1];
		resources[2] = (float)players[i].resources[0];	//gold
		resources[3] = (float)players[i].resources[3];
		resources[4] = (float)players[i].resources[4];
		resources[5] = 0.0F;
		if (ver1 == SV1_SWGB) {
		    fwrite(resources, sizeof(float), 5, dcout);
		} else {
		    fwrite(resources, sizeof(float), 6, dcout);
		}

		if (ver1 >= SV1_AOE2TC)
			fwrite(&players[i].pop, 4, 1, dcout);
	}

	players[8].write_units(dcout);
	for (i = PLAYER1_INDEX; i < num_players - 1; i++)
	{
		players[i].write_units(dcout);
	}

	/* Player Data 3 */

	fwrite(&num_players, 4, 1, dcout);
	for (i = PLAYER1_INDEX; i < num_players - 1; i++)
	{
		players[i].write_data3(dcout, i,
			i == PLAYER1_INDEX ? editor_pos : NULL	//See scx_format.txt.
			);
	}

	/* Triggers */

	fwrite(&trigver, 8, 1, dcout); //trigger system version
	writebin(dcout, &objstate); //objectives state

	if (triggers.size() > 0)
	{
	    Trigger *t_parse = &(*triggers.begin());
	    num = triggers.size();
	    fwrite(&num, sizeof(long), 1, dcout);

	    i = num;
	    while (i--)
	    {
		    t_parse->write(dcout);
		    t_parse++;
	    }

	    // Write out t_order vector
	    if (num > 0)  // ugly: have to do this check because we call front()
	    {
		    fwrite(&t_order.front(), sizeof(long), num, dcout);
	    }
	}

	/* Included Files */

	writebin(dcout, &unk3);
	NULLS(dcout, sizeof(unk4)); // we don't write the ES-only data

	if (unk3 == 1)
	{
		writebin(dcout, &cFiles);

		for (long i = 0; i != cFiles; ++i)
		{
			writecs<unsigned long>(dcout, files[i].name);
			files[i].data.write(dcout, sizeof(long));
		}
	}

	fclose(dcout);

	return ERR_none;
}

bool Scenario::export_bmp(const char *path)
{
	if (!bBitmap)
		return false;

	return bitmap.toFile(path);
}

//I HATE THIS FUNCTION!
void Scenario::clean_triggers()
{
	vector<bool> kill(triggers.size(), true); //contains whether a trigger should be deleted

	// don't kil the ones that are still indexed on t_order
	for (vector<unsigned long>::const_iterator i = t_order.begin();
		i != t_order.end(); ++i)
		kill[*i] = false;

	/* Now delete the ones still on the kill list. We do this backwards because
	 * that only invalidates the indices on and after the current one. */
	for (size_t i = triggers.size(); i != 0; --i)
	{
		// i is the index - 1 to make i != 0 work
		size_t t_index = i - 1;

		if (!kill[t_index])	//If it shouldn't be deleted...
			continue;           //... skip all this


		triggers.erase(triggers.begin() + t_index);

		// Decrement t_order entries referencing triggers beyond the one we
		// just removed.
		for (vector<unsigned long>::iterator j = t_order.begin();
			j != t_order.end(); ++j)
		{
			if (*j > t_index)
				--(*j);
		}

		if (triggers.size() > 0) { // need this because may have just deleted (see above) the last trigger
		    //adjust effect's trigger indexes accordingly (UGH!)
		    Trigger *t_parse = &(*triggers.begin());
		    for (size_t j = 0; j != triggers.size(); ++j)
		    {
			    for (vector<Effect>::iterator i = t_parse->effects.begin();
				    i != t_parse->effects.end(); ++i)
			    {
				    if (i->trig_index > t_index)
					    i->trig_index--;
				    else if (i->trig_index == t_index)
					    i->trig_index = (unsigned)-1; // TODO: hack cast
			    }

			    t_parse++;
		    }
		}
	}
}

size_t Scenario::insert_trigger(Trigger *t, size_t after)
{
    triggers.push_back(*t);
	size_t tindex = triggers.size() - 1;

	// TODO: does the -1 work?
	if (after == -1)
	{
		t_order.push_back(tindex);
	}
	else
	{
		// Find the /after/ value in t_order.
		vector<unsigned long>::iterator where =
			std::find(t_order.begin(), t_order.end(), after);

		// If the /after/ value was actually there, insert after it.
		// (Otherwise, we'll insert at the end, i.e. append.)
		if (where != t_order.end())
			++where;

		t_order.insert(where, tindex);
	}

	return tindex;
}

void Scenario::accept(TriggerVisitor& tv)
{
	for (size_t i = 0; i != triggers.size(); ++i)
		triggers.at(i).accept(tv);
}

bool Scenario::exFile(const char *directory, long index)
{
	int c;
	FILE *out;
	AOKFile *parse = files;
	char buffer[_MAX_PATH];

	_mkdir(directory);

	if (index < 0)
	{
		c = cFiles;
	}
	else
	{
		c = 1;
		parse += index;
	}

	while (c--)
	{
		sprintf(buffer, "%s\\%s.per", directory, parse->name);
		if (out = fopen(buffer, "wb"))
		{
			fwrite(parse->data.c_str(), sizeof(char), parse->data.length(), out);
			fclose(out);
		}
		else
			break;

		parse++;
	}

	return (c == -1);	//if it doesn't, we broke early
}

/*	MAP COPY

	Map copy data is stored as follows in memory:

	size_t width, height, unitcount[NUM_PLAYERS];
	Terrain terrain[x][y];
	Unit units[unitcount];	//x and y modified to dist from the bottom-left corner of copied area
*/

class MapCopyCache
{
public:
	RECT source;

	size_t unitcount[NUM_PLAYERS];
	std::vector<Unit> units;
	//SList<Unit> units;
};

#define ISINRECT(r, x, y) \
	(x >= r.left && x <= r.right && y >= r.bottom && y <= r.top)

#define ISATPOINT(r, x, y) \
	(x == r.x && y == r.y)

// functor to check if a unit is in a RECT
class unit_in_rect : public std::unary_function<const Unit, bool>
{
public:
	unit_in_rect(const RECT& rect)
		: _rect(rect)
	{}

	// I'm a little confused that you can use a reference here, since we tell
	// others we take "const Unit", but it works. Shrug.
	bool operator()(const Unit& u) const
	{
		return ISINRECT(_rect, u.x, u.y);
	}

private:
	RECT _rect;
};

int Scenario::map_size(const RECT &source, MapCopyCache *&mcc)
{
	Player *p;
	int ret;
	int i;

	mcc = new MapCopyCache;

	CopyRect(&mcc->source, &source);

	/* x, u, unitcount */
	ret = sizeof(size_t) * 2;
	ret += sizeof(mcc->unitcount);

	/* space for the tiles */
	ret += sizeof(Map::Terrain) * (source.top - source.bottom + 1) * (source.right - source.left + 1);

	/* space for the units */
	unit_in_rect in_source(source);
	FEP(p)
	{
		using std::vector;

		// Convenience, to ensure we modify the right unitcount.
		size_t& unitcount = mcc->unitcount[i];

		// Get the initial size: this is a hack because I'm too lazy to make
		// individual vectors for each player.
		unitcount = mcc->units.size();

		std::remove_copy_if(p->units.begin(), p->units.end(),
			std::back_inserter(mcc->units), std::not1(in_source));

		unitcount = mcc->units.size() - unitcount;
		ret += unitcount * Unit::size;
	}

	return ret;
}

AOKTS_ERROR Scenario::add_activation(size_t start, size_t end, size_t to) {

    Trigger *d_trig = &triggers.at(to);
    Effect e = Effect();

    for (size_t i = start; i <= end; i++) {
        e.trig_index = i;
        e.type = 8;
        d_trig->effects.push_back(e);
    }

	return ERR_none;
}

AOKTS_ERROR Scenario::move_triggers(size_t start, size_t end, size_t to) {
	size_t num = triggers.size();
	// don't need to do this as start is unsigned: start >= 0
	if (num > 0 && end >= start && to < num && end < num) {
        for (size_t i = 0; i < num; i++) {
            long displacement = to - start;
            size_t range = end - start;
            size_t toend = to + range;
            if (triggers.at(i).display_order >= (long)start && triggers.at(i).display_order <= (long)end) {
                triggers.at(i).display_order += displacement;
            } else {
                if (displacement > 0 && triggers.at(i).display_order > (long)end && triggers.at(i).display_order <= (long)toend) {
                    triggers.at(i).display_order -= (range + 1);
                } else if (displacement < 0 && triggers.at(i).display_order >= (long)to && triggers.at(i).display_order < (long)start) {
                    triggers.at(i).display_order += (range + 1);
                }
            }
            //t_order[(size_t)triggers.at(i).display_order] = i;
        }

        for (size_t i = 0; i < num; i++) {
            if (triggers.at(i).display_order >= 0 && triggers.at(i).display_order < (long)triggers.size())
                t_order[triggers.at(i).display_order] = i;
        }
        //clean_triggers();
    }

	return ERR_none;
}

AOKTS_ERROR Scenario::swap_players(int a, int b) {
    std::swap(players[a], players[b]);

    a++;
    b++;

    // each triggers
	for (vector<Trigger>::iterator trig = triggers.begin(); trig != triggers.end(); ++trig) {
	    // each effect
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
	        if (iter->s_player == a)
	            iter->s_player = b;
	        else if (iter->s_player == b)
	            iter->s_player = a;

	        if (iter->t_player == a)
	            iter->t_player = b;
	        else if (iter->t_player == b)
	            iter->t_player = a;
	    }
	    // conditions
	    for (vector<Condition>::iterator iter = trig->conds.begin(); iter != trig->conds.end(); ++iter) {
	        if (iter->player == a)
	            iter->player = b;
	        else if (iter->player == b)
	            iter->player = a;
        }
	}

	return ERR_none;
}

AOKTS_ERROR Scenario::delete_triggers(size_t start, size_t end) {
	size_t num = triggers.size();
	// don't need to do this as start is unsigned: start >= 0
	if (num > 0 && end >= start && end < num) {
        for (size_t i = start; i <= end; i++) {
	        vector<unsigned long>::iterator iter =
		        std::find(t_order.begin(), t_order.end(), i);

	        if (iter != t_order.end()) // Is this check necessary?
		        t_order.erase(iter);
		}
		//update_display_order(); // need this because some trigger ids are now missing and will crash sync.
		//how to do this? only way I know how is to save and reload
		//scenario
		//clean_triggers(); // not this one
    }
	return ERR_none;
}

AOKTS_ERROR Scenario::duplicate_triggers(size_t start, size_t end, size_t to) {
	size_t num = triggers.size();
	// don't need to do this as start is unsigned: start >= 0
	if (num > 0 && end >= start && to < num && end < num) {
        for (size_t i = 0; i < num; i++) {
            long displacement = to - start;
            size_t range = end - start;
            size_t toend = to + range;
            if (triggers.at(i).display_order >= (long)start && triggers.at(i).display_order <= (long)end) {
                // copy it to its spot
                triggers.at(i).display_order += displacement;
            } else {
                if (displacement > 0 && triggers.at(i).display_order > (long)end && triggers.at(i).display_order <= (long)toend) {
                    triggers.at(i).display_order -= (range + 1);
                } else if (displacement < 0 && triggers.at(i).display_order >= (long)to && triggers.at(i).display_order < (long)start) {
                    triggers.at(i).display_order += (range + 1);
                }
            }
            //t_order[(size_t)triggers.at(i).display_order] = i;
        }

        for (size_t i = 0; i < num; i++) {
            if (triggers.at(i).display_order >= 0 && triggers.at(i).display_order < (long)triggers.size())
                t_order[triggers.at(i).display_order] = i;
        }
        //clean_triggers();
    }

	return ERR_none;
}

AOKTS_ERROR Scenario::sync_triggers() {
	size_t num = triggers.size();
	if (num > 0) {
        for (size_t i = 0; i < num; i++) {
            if (triggers.at(i).display_order != (long)i) {
                for (size_t j = i+1; j < num; j++) {
                    if (triggers.at(j).display_order == (long)i) {
                        iter_swap(triggers.begin() + i, triggers.begin() + j);
                        for (size_t k = 0; k < num; k++) {
                            for (size_t ie = 0; ie < triggers.at(k).effects.size(); ie++) {
                                if (triggers.at(k).effects.at(ie).trig_index == i) {
                                    triggers.at(k).effects.at(ie).trig_index = j;
                                } else if (triggers.at(k).effects.at(ie).trig_index == j) {
                                    triggers.at(k).effects.at(ie).trig_index = i;
                                }
                            }
                        }
                    }
                }
            }
		    //// save the trigger display order to the trigger objects
		    //for (unsigned int j = 0; j < n_trigs; j++) {
			//    //printf("%hu\n",j);
			//    triggers.at(t_order[j]).display_order = j;
		    //}
			//triggers.at(.display_order = j;
			t_order[i] = i;

		    //*t_order.begin() = i;
        }
        //for (size_t i = 0; i < num; i++) {
        //    t_order[triggers.at(i).display_order] = i;
        //}
    }

	return ERR_none;
}

AOKTS_ERROR Scenario::map_copy(Buffer &to, const MapCopyCache *mcc)
{
	size_t w, h;
	unsigned x, y;

	x = mcc->source.left;
	y = mcc->source.bottom;

	w = mcc->source.right - x;
	h = mcc->source.top - y;

	to.write(&w, sizeof(w));
	to.write(&h, sizeof(h));
	to.write(mcc->unitcount, sizeof(mcc->unitcount));

	map.writeArea(to, mcc->source);

	for (vector<Unit>::const_iterator iter = mcc->units.begin();
		iter != mcc->units.end(); ++iter)
	{
		Unit u(*iter);
		u.x -= x;
		u.y -= y;
		u.toBuffer(to);
	}

	delete mcc;

	return ERR_none;
}

AOKTS_ERROR Scenario::map_paste(const POINT &to, Buffer &from)
{
	using std::vector;

	size_t x, y, unitcount[NUM_PLAYERS];
	unsigned i;
	Player *p;

	from.read(&x, sizeof(x));
	from.read(&y, sizeof(y));
	from.read(unitcount, sizeof(unitcount));

	RECT area;
	area.left = to.x;
	area.bottom = to.y;
	area.right = area.left + x;
	area.top = area.bottom + y;

	map.readArea(from, area);
	area.right += 1;
	area.top += 1;

	unit_in_rect in_dest(area);
	FEP(p)
	{
		/* find units in the paste area and erase them */
		p->units.erase(
			remove_if(p->units.begin(), p->units.end(), in_dest),
			p->units.end());

		/* add the units from the copy buffer */
		size_t count = unitcount[i];
		while (count--)
		{
			Unit u(from);
			u.ident = next_uid++;
			u.x += to.x;
			u.y += to.y;
			p->units.push_back(u);
		}
	}

	return ERR_none;
}

AOKTS_ERROR Scenario::map_scale(const RECT &area, const float scale)
{
	//LONG dx, dy, w, h;

	map.scaleArea(area, scale);

    //// each player
	//for (int i = 0; i < NUM_PLAYERS; i++) {
    //    // camera
    //    if (ISINRECT(from, players[i].camera[0], players[i].camera[0])) {
    //        players[i].camera[0] += dx;
    //        players[i].camera[1] += dy;
    //    }

    //    // units
	//    for (vector<Unit>::iterator iter = players[i].units.begin(); iter != players[i].units.end(); ++iter) {
    //        if (ISINRECT(from, iter->x, iter->y)) {
	//	        iter->x += dx;
	//	        iter->y += dy;
	//	    } else if (ISINRECT(torect, iter->x, iter->y)) {
	//	        iter->x -= dx;
	//	        iter->y -= dy;
	//	    }
	//    }
	//}

	//Trigger *trig = triggers.first();
	//long num = triggers.count();

    //// triggers
	//long i = num;
	//while (i--)
	//{
	//    // effects
	//    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
	//	    if (ISINRECT(from, iter->location.x, iter->location.y)) {
	//	        iter->location.x += dx;
	//	        iter->location.y += dy;
	//	    } else if (ISINRECT(torect, iter->location.x, iter->location.y)) {
	//	        iter->location.x -= dx;
	//	        iter->location.y -= dy;
	//	    }
	//	    if (ISINRECT(from, iter->area.right, iter->area.top)) {
	//	        iter->area.right += dx;
	//	        iter->area.top += dy;
	//	    } else if (ISINRECT(torect, iter->area.right, iter->area.top)) {
	//	        iter->area.right -= dx;
	//	        iter->area.top -= dy;
	//	    }
	//	    if (ISINRECT(from, iter->area.left, iter->area.bottom)) {
	//	        iter->area.left += dx;
	//	        iter->area.bottom += dy;
	//	    } else if (ISINRECT(torect, iter->area.left, iter->area.bottom)) {
	//	        iter->area.left -= dx;
	//	        iter->area.bottom -= dy;
	//	    }
	//	}
	//    // conditions
	//    for (vector<Condition>::iterator iter = trig->conds.begin(); iter != trig->conds.end(); ++iter) {
	//	    if (ISINRECT(from, iter->area.right, iter->area.top)) {
	//	        iter->area.right += dx;
	//	        iter->area.top += dy;
	//	    } else if (ISINRECT(torect, iter->area.right, iter->area.top)) {
	//	        iter->area.right -= dx;
	//	        iter->area.top -= dy;
	//	    }
	//	    if (ISINRECT(from, iter->area.left, iter->area.bottom)) {
	//	        iter->area.left += dx;
	//	        iter->area.bottom += dy;
	//	    } else if (ISINRECT(torect, iter->area.left, iter->area.bottom)) {
	//	        iter->area.left -= dx;
	//	        iter->area.bottom -= dy;
	//	    }
	//	}
	//	trig++;
	//}

	return ERR_none;
}

AOKTS_ERROR Scenario::remove_trigger_names()
{
	long num = triggers.size();
	if (num > 0) {
	    Trigger *trig = &(*triggers.begin());

        // triggers
	    long i = num;
	    while (i--)
	    {
	        strcpy(trig->name, "");
		    trig++;
	    }
	}

	return ERR_none;
}

AOKTS_ERROR Scenario::remove_trigger_descriptions()
{
	long num = triggers.size();
	if (num > 0) {
	    Trigger *trig = &(*triggers.begin());

        // triggers
	    long i = num;
	    while (i--)
	    {
		    char *cstr = trig->description.unlock(1);
	        strcpy(cstr, "");
		    trig->description.lock();
		    trig++;
	    }
	}

	return ERR_none;
}

AOKTS_ERROR Scenario::swap_trigger_names_descriptions()
{
	long num = triggers.size();
	if (num > 0) {
	    Trigger *trig = &(*triggers.begin());

        // triggers
	    long i = num;
	    while (i--)
	    {
	        char buffer[128];
		    char *cstr = trig->description.unlock(128);
	        strncpy ( buffer, cstr, 128 );
	        buffer[128] = '\0';
	        strncpy ( cstr, trig->name, 128 );
	        cstr[128] = '\0';
		    trig->description.lock();
	        strncpy ( trig->name, buffer, 128 );
	        trig->name[128] = '\0';
		    trig++;
	    }
	}

	return ERR_none;
}

bool up_to_aofe_test(const Effect & e)
{
     return e.type == 30 || e.type == 31 || e.type == 32 || e.type == 33;
}

AOKTS_ERROR Scenario::up_to_aofe() {
	long num = triggers.size();
	if (num < 1)
	    return ERR_none;
	Trigger *trig = &(*triggers.begin());

    // triggers
	long i = num;
	while (i--)
	{
	    // using a lamba function, delete speed range armor1 armor2
        trig->effects.erase(std::remove_if(trig->effects.begin(), trig->effects.end(), up_to_aofe_test), trig->effects.end());
		trig++;
	}
	return ERR_none;
}

bool up_to_10c_test(const Effect & e)
{
    return e.type == 30 || e.type == 31 || e.type == 32 || e.type == 33
        || (e.type == 24 && e.s_player == 0)
        || (e.type == 26 && (e.area.left != -2 || e.area.right != -1 || e.area.top != -1 || e.area.bottom != -1));
}

AOKTS_ERROR Scenario::up_to_10c() {
	long num = triggers.size();
	if (num < 1)
	    return ERR_none;
	Trigger *trig = &(*triggers.begin());

    // triggers
	long i = num;
	while (i--)
	{
	    // using a lamba function, delete speed range armor1 armor2
        // remove effects that change name over area
        // prevent using damage on gaia
        trig->effects.erase(std::remove_if(trig->effects.begin(), trig->effects.end(), up_to_10c_test), trig->effects.end());
		trig++;
	}
	return ERR_none;
}

AOKTS_ERROR Scenario::up_to_hd() {
	long num = triggers.size();
	if (num < 1)
	    return ERR_none;
	Trigger *trig = &(*triggers.begin());

    bool removed = false;

    // triggers
	long i = num;
	while (i--)
	{
	    // unify effects
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end();) {
	        if (iter->type == 32) { // armor 1
	            for (vector<Effect>::iterator iter2 = (iter + 1); iter2 != trig->effects.end(); ++iter2) {
	                if (iter2->type == 33 && iter->amount == iter2->amount) { // armor 1
	                    trig->effects.erase(iter2);
                        removed = true;
	                    break;
	                }
	            }
	        }

	        if (!removed)
	            ++iter;
	        removed = false;
		}

	    // convert remaining effects
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
	        switch (iter->type) {
	            case 30: // speed
	                iter->type = 33;
	            break;
	            case 31: // range
	                iter->type = 32;
	            break;
	            case 32: // armor 1
	                iter->type = 31;
	            break;
	            case 33: // armor 2
	                iter->type = 31;  // no way to make this function reversible
	            break;
	        }
	    }

		trig++;
	}
	return ERR_none;
}

AOKTS_ERROR Scenario::aoc_to_aok() {
	//Player *p;
	//int i;
	//FEP(p)
	//	p->ucount=1.0F;
	return ERR_none;
}

AOKTS_ERROR Scenario::aok_to_aoc() {
	//Player *p;
	//int i;
	//FEP(p)
	//	p->ucount=2.0F;
	return ERR_none;
}

AOKTS_ERROR Scenario::hd_to_up() {
	long num = triggers.size();
	if (num < 1)
	    return ERR_none;
	Trigger *trig = &(*triggers.begin());

    // triggers
	long i = num;
	while (i--)
	{
	    // effects
	    vector<Effect> neweffects;
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
	        switch (iter->type) {
	            case 33: // speed
	                iter->type = 30;
	            break;
	            case 32: // range
	                iter->type = 31;
	            break;
	            case 31: // armor
	                iter->type = 32; // this is up armor 1. ∴ add an effect with type 33
		            neweffects.push_back(*iter);
		            neweffects.back().type = 33;
	            break;
	        }
		}
	    for (vector<Effect>::iterator iter = neweffects.begin(); iter != neweffects.end(); ++iter) {
	        trig->effects.push_back(*iter);
	    }
		trig++;
	}

    // each player i
	for (int i = 0; i < NUM_PLAYERS; i++) {
        // units
	    for (std::vector<Unit>::iterator unit = players[i].units.begin(); unit != players[i].units.end(); ++unit) {
            switch (unit->getType()->id()) {
            case 445: // Church 4 (AOF) / INVALID (AOFE)
                // change to Monastery (104)
                unit->setType(esdata.units.getById(104));
                break;
            case 918: // Brown Rock (AOF) / Monk With Relic (AOFE)
                // change to Rock (104)
                unit->setType(esdata.units.getById(623));
                break;
            }
	    }
	}

	return ERR_none;
}

AOKTS_ERROR Scenario::fix_trigger_outliers() {
	RECT area;
	area.left = -1;
	area.bottom = -1;
	area.right = map.x - 1;
	area.top = map.y - 1;

	long num = triggers.size();
	if (num < 1)
	    return ERR_none;
	Trigger *trig = &(*triggers.begin());

    // triggers
	long i = num;
	while (i--)
	{
	    // effects
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
		    if (!ISINRECT(area, iter->location.x, iter->location.y)) {
		        iter->location.x = area.right;
		        iter->location.y = area.top;
		    }
		    if (!ISINRECT(area, iter->area.left, iter->area.bottom)) {
		        iter->area.left = area.right;
		        iter->area.bottom = area.top;
		    }
		    if (!ISINRECT(area, iter->area.right, iter->area.top)) {
		        iter->area.right = area.right;
		        iter->area.top = area.top;
		    }
		    //if (iter->location.x == 123 && iter->location.y == 123) {
		    //    iter->location.x = -1;
		    //    iter->location.y = -1;
		    //}
		    //if (iter->area.left == 123 && iter->area.bottom == 123) {
		    //    iter->area.left = -1;
		    //    iter->area.bottom = -1;
		    //}
		    //if (iter->area.right == 123 && iter->area.top == 123) {
		    //    iter->area.right = -1;
		    //    iter->area.top = -1;
		    //}
		}
	    // conditions
	    for (vector<Condition>::iterator iter = trig->conds.begin(); iter != trig->conds.end(); ++iter) {
		    if (!ISINRECT(area, iter->area.left, iter->area.bottom)) {
		        iter->area.left = area.right;
		        iter->area.bottom = area.top;
		    }
		    if (!ISINRECT(area, iter->area.right, iter->area.top)) {
		        iter->area.right = area.right;
		        iter->area.top = area.top;
		    }
		    //if (iter->area.left == 123 && iter->area.bottom == 123) {
		    //    iter->area.left = -1;
		    //    iter->area.bottom = -1;
		    //}
		    //if (iter->area.right == 123 && iter->area.top == 123) {
		    //    iter->area.right = -1;
		    //    iter->area.top = -1;
		    //}
		    //if (iter->type == 1 && ((trig->display_order >= 164 && trig->display_order <= 181) || (trig->display_order >= 193 && trig->display_order <= 324))) {
		    //    iter->area.right = 123;
		    //    iter->area.top = 123;
		    //}
		    //if (iter->area.left == 123 && iter->area.bottom == 123) {
		    //    iter->area.left = -1;
		    //    iter->area.bottom = -1;
		    //}
		}
		trig++;
	}
	return ERR_none;
}

AOKTS_ERROR Scenario::compress_unit_ids()
{
    // triggers
	long newu = 0;
	long oldu;

	// create array for the unit
	long numunits = 0;
	for (int i = 0; i < NUM_PLAYERS; i++) {
	    numunits += players[i].units.size();
	}
    std::vector<std::pair<int, int>> unittrans;
    unittrans.reserve(numunits);

	newu = 0;
    // each player i
	for (int i = 0; i < NUM_PLAYERS; i++) {
        // units
	    for (std::vector<Unit>::iterator unit = players[i].units.begin(); unit != players[i].units.end(); ++unit, ++newu) {

	        oldu = unit->ident;

            // don't do anything for units with negative ids
            if (oldu >= 0) {
	            unittrans.push_back(std::make_pair(unit->ident, newu));
	        } else {
                newu--;
            }
	    }
	}

	next_uid = newu;

    // each player i
	for (int i = 0; i < NUM_PLAYERS; i++) {
        // each unit
	    for (std::vector<Unit>::iterator unit = players[i].units.begin(); unit != players[i].units.end(); ++unit) {
	        // update ids including duplicates
	        for (std::vector<std::pair<int, int>>::iterator unitt = unittrans.begin(); unitt != unittrans.end(); ++unitt) {
	            if (unit->ident == unitt->first) {
	                unit->ident = unitt->second;
	                break;
	            }
	        }
	        // update garrisons
	        for (std::vector<std::pair<int, int>>::iterator unitt = unittrans.begin(); unitt != unittrans.end(); ++unitt) {
	            // != 0 check is for AoK. default was 0 not -1.
	            if (unit->garrison != 0 && unit->garrison == unitt->first) {
	                unit->garrison = unitt->second;
	                break;
	            }
	        }
	    }
	}

    // each triggers
	for (vector<Trigger>::iterator trig = triggers.begin(); trig != triggers.end(); ++trig) {
	    // each effect
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
	        // each unit trans
	        for (std::vector<std::pair<int, int>>::iterator unitt = unittrans.begin(); unitt != unittrans.end(); ++unitt) {
	            // update uid_loc
	            if (iter->uid_loc == unitt->first) {
	                iter->uid_loc = unitt->second;
	                break;
	            }
	        }
	    }
	    // each effect
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
	        // each uid
	        for (int u = 0; u < iter->num_sel; u++) {
	            // each unit trans
	            for (std::vector<std::pair<int, int>>::iterator unitt = unittrans.begin(); unitt != unittrans.end(); ++unitt) {
	                // update uid
	                if (iter->uids[u] == unitt->first) {
	                    iter->uids[u] = unitt->second;
	                    break;
	                }
	            }
		    }
		}
	    // conditions
	    for (vector<Condition>::iterator iter = trig->conds.begin(); iter != trig->conds.end(); ++iter) {
	        // each unit trans
	        for (std::vector<std::pair<int, int>>::iterator unitt = unittrans.begin(); unitt != unittrans.end(); ++unitt) {
	            // update u_loc
	            if (iter->u_loc == unitt->first) {
	                iter->u_loc = unitt->second;
	                break;
	            }
	        }
	        // each unit trans
	        for (std::vector<std::pair<int, int>>::iterator unitt = unittrans.begin(); unitt != unittrans.end(); ++unitt) {
	            // update object
	            if (iter->object == unitt->first) {
	                iter->object = unitt->second;
	                break;
	            }
		    }
        }
	}

	return ERR_none;
}

AOKTS_ERROR Scenario::randomize_unit_frames(HWND dialog)
{
    // compiler will set first array element to the value you've
    // provided (0) and all others will be set to zero because it is a
    // default value for omitted array elements.
    // can't do this. need to use compiler constants for sizes. also {0}
    // doesn't work in vc++ 2005
    //unsigned char lookup[perversion->max_unit][2] = {0};
    //map<unsigned char, unsigned char> lookup;
    vector< pair<short,float>> lookup(perversion->max_unit, std::make_pair(0,0.0F));

    UCNST cunitid;
    // each player
	for (int i = 0; i < NUM_PLAYERS; i++) {
        // units
        vector<Unit>::iterator end = players[i].units.end();
        for (vector<Unit>::iterator iter = players[i].units.begin(); iter != end; ++iter) {
	        cunitid = iter->getType()->id();
	        if (cunitid >= 0 && cunitid < perversion->max_unit){
	            if (iter->frame > lookup.at(cunitid).first) {
	                lookup.at(cunitid).first = iter->frame;
	            }
	            if (iter->rotate > lookup.at(cunitid).second) {
	                lookup.at(cunitid).second = iter->rotate;
	            }
	        } else {
	            //MessageBox(dialog, toString<int>(cunitid).c_str(), "", MB_ICONERROR);
	        }
	    }
	}
    // each player
	for (int i = 0; i < NUM_PLAYERS; i++) {
        // units
        vector<Unit>::iterator end = players[i].units.end();
	    for (vector<Unit>::iterator iter = players[i].units.begin(); iter != end; ++iter) {
	        cunitid = iter->getType()->id();
	        // 264 - 272 are cliffs (avoid) when game is aoe2
	        if (cunitid >= 0 && cunitid < perversion->max_unit && !((ver1 == SV1_AOE2 || ver1 == SV1_AOE2TC) && cunitid >= 264 && cunitid <= 272)){
                iter->frame = (short)(rand() % (lookup.at(cunitid).first + 1));
                iter->rotate = (float)(rand() % (int)(lookup.at(cunitid).second / (float)PI * 4 + 1)) / 4 * (float)PI;
            }
	    }
	}
	return ERR_none;
}

AOKTS_ERROR Scenario::water_cliffs_visibility(const bool visibility)
{
    LONG numunits = players[8].units.size();
    Unit * u;
    Map::Terrain * t;
	for (int j = 0; j < numunits; j++) {
	    // 264 - 272 are cliffs (avoid) when game is aoe2
	    u = &(players[8].units.at(j));
	    if (u->getType()->id() >= 264 && players[8].units.at(j).getType()->id() <= 272) {
            t = &map.terrain[(int)floor(u->x)][(int)floor(u->y)];
            switch (t->cnst) {
            case 1:
            case 4:
            case 22:
            case 23:
                if (visibility) {
                    u->rotate = 1;
                    u->frame = 1;
                } else {
                    u->rotate = 0;
                    u->frame = 0;
                }
                break;
            }
	    }
	}
	return ERR_none;
}


AOKTS_ERROR Scenario::randomize_unit_frames(const unsigned int cnst)
{
    switch (cnst)
    {
    case 349: // Forest, Oak
    case 351: // Forest, Palm
        LONG numunits = players[8].units.size();
	    for (int j = 0; j < numunits; j++) {
	        if (players[8].units.at(j).getType()->id() == cnst) {
                players[8].units.at(j).rotate = (float)(rand() % (int)(8));
                players[8].units.at(j).frame = (short)(rand() % (int)(13));
	        }
	    }
        break;
        /*
    default:
        // each player
	    for (int i = 0; i < NUM_PLAYERS; i++) {
            // units
            LONG numunits = players[i].units.size();
	        for (int j = 0; j < numunits; j++) {
	            if (players[i].units.at(j).getType()->id() == cnst) {
                    players[i].units.at(j).rotate = (float)(rand() % (int)(8));
                    players[i].units.at(j).frame = (short)(rand() % (int)(13));
	            }
	        }
	    }
	    */
	}
	return ERR_none;
}

AOKTS_ERROR Scenario::map_duplicate(const RECT &from, const POINT &to, OpFlags::Value flags)
{
	LONG dx, dy, w, h, truew, trueh;

	dx = to.x - from.left;
	dy = to.y - from.bottom;
	w = from.right - from.left;
	h = from.top - from.bottom;
    // +1 because right - left == 0 represents 1 tile
	truew = w + 1;
	trueh = h + 1;

	RECT truefrom;
	truefrom.left = from.left;
	truefrom.bottom = from.bottom;
	truefrom.right = from.left + truew;
	truefrom.top = from.bottom + trueh;

	RECT trueto;
	trueto.left = to.x;
	trueto.right = to.x + truew;
	trueto.bottom = to.y;
	trueto.top = to.y + trueh;

	RECT areato;
	areato.left = to.x;
	areato.right = to.x + w;
	areato.bottom = to.y;
	areato.top = to.y + h;

    if (flags & OpFlags::UNITS){
        // each player
	    for (int i = 0; i < NUM_PLAYERS; i++) {
            // units
            vector<Unit>::iterator end = players[i].units.end();
            LONG numunits = players[i].units.size();
	        for (int j = 0; j < numunits; j++) {
                if (ISINRECT(truefrom, players[i].units.at(j).x, players[i].units.at(j).y)) {
	                Unit spec(players[i].units.at(j));
	                spec.ident = scen.next_uid++;
		            spec.x += dx;
		            spec.y += dy;
		            players[i].units.push_back(spec);
		        }
	        }
	    }
	}

	dx = to.x - truefrom.left;
	dy = to.y - truefrom.bottom;

    LONG cw = truefrom.right - truefrom.left;
    LONG ch = truefrom.top - truefrom.bottom;

	RECT torect;
	torect.left = to.x;
	torect.right = to.x + cw;
	torect.bottom = to.y;
	torect.top = to.y + ch;

    if ((flags & OpFlags::TERRAIN) || (flags & OpFlags::ELEVATION)){
        Map::Terrain blank;
	    blank.cnst=0;
	    blank.elev=0;

	    /* perform some validation on input */
	    if (!(truefrom.bottom < 0 || static_cast<unsigned>(truefrom.top) > map.y ||
		    truefrom.left < 0 || static_cast<unsigned>(truefrom.right) > map.x ||
	        torect.bottom < 0 || static_cast<unsigned>(torect.top) > map.y ||
		    torect.left < 0 || static_cast<unsigned>(torect.right) > map.x)) {

            // temporarily store the area to move before copying
            vector<vector<Map::Terrain>> frombuffer(cw, vector<Map::Terrain>(ch));
	        for (LONG i = 0; i < cw; i++) {
		        for (LONG j = 0; j < ch; j++) {
		            frombuffer[i][j] = map.terrain[truefrom.left + i][truefrom.bottom + j];
		        }
	        }

            if (flags & OpFlags::TERRAIN){
	            for (LONG i = 0; i < cw; i++) {
		            for (LONG j = 0; j < ch; j++) {
		                map.terrain[torect.left + i][torect.bottom + j].cnst = frombuffer[i][j].cnst;
		            }
	            }
	        }

            if (flags & OpFlags::ELEVATION){
	            for (LONG i = 0; i < cw; i++) {
		            for (LONG j = 0; j < ch; j++) {
		                map.terrain[torect.left + i][torect.bottom + j].elev = frombuffer[i][j].elev;
		            }
	            }
	        }
	    }
	}


	return ERR_none;
}

AOKTS_ERROR Scenario::map_delete(const RECT &from, const POINT &to, OpFlags::Value flags)
{
	LONG dx, dy, w, h, truew, trueh;

	dx = to.x - from.left;
	dy = to.y - from.bottom;
	w = from.right - from.left;
	h = from.top - from.bottom;
    // +1 because right - left == 0 represents 1 tile
	truew = w + 1;
	trueh = h + 1;

	RECT truefrom;
	truefrom.left = from.left;
	truefrom.bottom = from.bottom;
	truefrom.right = from.left + truew;
	truefrom.top = from.bottom + trueh;

	RECT trueto;
	trueto.left = to.x;
	trueto.right = to.x + truew;
	trueto.bottom = to.y;
	trueto.top = to.y + trueh;

	RECT areato;
	areato.left = to.x;
	areato.right = to.x + w;
	areato.bottom = to.y;
	areato.top = to.y + h;

    // each player
	for (int i = 0; i < NUM_PLAYERS; i++) {
        if (flags & OpFlags::UNITS){
            players[i].erase_unit_area(truefrom);
	    }
	}

	return ERR_none;


    if (!(flags & OpFlags::TRIGGERS))
	    return ERR_none;

	Trigger *trig = &(*triggers.begin());
	long num = triggers.size();

    // triggers
	long i = num;
	while (i--)
	{
	    // effects
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
		    if (ISINRECT(from, iter->location.x, iter->location.y)) {
		        iter->location.x += dx;
		        iter->location.y += dy;
		    } else if (ISINRECT(areato, iter->location.x, iter->location.y)) {
		        iter->location.x -= dx;
		        iter->location.y -= dy;
		    }
		    if (ISINRECT(from, iter->area.right, iter->area.top)) {
		        iter->area.right += dx;
		        iter->area.top += dy;
		    } else if (ISINRECT(areato, iter->area.right, iter->area.top)) {
		        iter->area.right -= dx;
		        iter->area.top -= dy;
		    }
		    if (ISINRECT(from, iter->area.left, iter->area.bottom)) {
		        iter->area.left += dx;
		        iter->area.bottom += dy;
		    } else if (ISINRECT(areato, iter->area.left, iter->area.bottom)) {
		        iter->area.left -= dx;
		        iter->area.bottom -= dy;
		    }
		}
	    // conditions
	    for (vector<Condition>::iterator iter = trig->conds.begin(); iter != trig->conds.end(); ++iter) {
		    if (ISINRECT(from, iter->area.right, iter->area.top)) {
		        iter->area.right += dx;
		        iter->area.top += dy;
		    } else if (ISINRECT(areato, iter->area.right, iter->area.top)) {
		        iter->area.right -= dx;
		        iter->area.top -= dy;
		    }
		    if (ISINRECT(from, iter->area.left, iter->area.bottom)) {
		        iter->area.left += dx;
		        iter->area.bottom += dy;
		    } else if (ISINRECT(areato, iter->area.left, iter->area.bottom)) {
		        iter->area.left -= dx;
		        iter->area.bottom -= dy;
		    }
		}
		trig++;
	}

	return ERR_none;
}

AOKTS_ERROR Scenario::sort_conds_effects()
{
	Trigger *trig = &(*triggers.begin());
	long num = triggers.size();

    // triggers
	long i = num;
	while (i--)
	{
		trig->sort_effects();
		trig->sort_conditions();
		trig++;
	}
	return ERR_none;
}

// Remember the RANDOMIZE flag
AOKTS_ERROR Scenario::map_repeat(const RECT &target, const POINT &source, OpFlags::Value flags)
{
	LONG dx, dy, w, h, truew, trueh;

	dx = target.left - source.x;
	dy = target.bottom - source.y;
	w = target.right - target.left;
	h = target.top - target.bottom;
    // +1 because right - left == 0 represents 1 tile
	truew = w + 1;
	trueh = h + 1;
	RECT truetarget;
	truetarget.left = target.left;
	truetarget.bottom = target.bottom;
	truetarget.right = target.left + truew;
	truetarget.top = target.bottom + trueh;

	RECT truesource;
	truesource.left = source.x;
	truesource.right = source.x + 1;
	truesource.bottom = source.y;
	truesource.top = source.y + 1;

    LONG cw = truetarget.right - truetarget.left;
    LONG ch = truetarget.top - truetarget.bottom;

	//RECT sourcerect;
	//sourcerect.left = source.x;
	//sourcerect.right = source.x + cw;
	//sourcerect.bottom = source.y;
	//sourcerect.top = source.y + ch;

    if (flags & OpFlags::UNITS){
        // each player
	    for (int i_p = 0; i_p < NUM_PLAYERS; i_p++) {
            // units
            vector<Unit>::iterator end = players[i_p].units.end();
            LONG numunits = players[i_p].units.size();
	        for (int i_u = 0; i_u < numunits; i_u++) {
                if (ISINRECT(truesource, players[i_p].units.at(i_u).x, players[i_p].units.at(i_u).y)) {
	                for (LONG i = 0; i < cw; i++) {
		                for (LONG j = 0; j < ch; j++) {
	                        Unit spec(players[i_p].units.at(i_u));
	                        spec.ident = scen.next_uid++;
		                    spec.x += dx + i;
		                    spec.y += dy + j;
		                    players[i_p].units.push_back(spec);
		                }
		            }
		        }
	        }
	    }
	}

    if (flags & OpFlags::TERRAIN){
	    for (LONG i = target.left; i <= target.right; i++) {
		    for (LONG j = target.bottom; j <= target.top; j++) {
		        map.terrain[i][j].cnst = map.terrain[source.x][source.y].cnst;
		    }
		}
	}

    if (flags & OpFlags::ELEVATION){
	    for (LONG i = target.left; i <= target.right; i++) {
		    for (LONG j = target.bottom; j <= target.top; j++) {
		        map.terrain[i][j].elev = map.terrain[source.x][source.y].elev;
		    }
		}
	}
	return ERR_none;
}

AOKTS_ERROR Scenario::map_move(const RECT &from, const POINT &to, OpFlags::Value flags)
{
	LONG dx, dy, w, h, truew, trueh;

	dx = to.x - from.left;
	dy = to.y - from.bottom;
	w = from.right - from.left;
	h = from.top - from.bottom;
    // +1 because right - left == 0 represents 1 tile
	truew = w + 1;
	trueh = h + 1;

	RECT truefrom;   // would more appropriately be caled "unit from" as opposed to "trigger from" as triggers are integers
	truefrom.left = from.left;
	truefrom.bottom = from.bottom;
	truefrom.right = from.left + truew;
	truefrom.top = from.bottom + trueh;

    if ((flags & OpFlags::TERRAIN) || (flags & OpFlags::ELEVATION)){
        Map::Terrain blank;
	    blank.cnst=0;
	    blank.elev=0;

	    LONG dx = 0, dy = 0;
	    dx = to.x - truefrom.left;
	    dy = to.y - truefrom.bottom;

        LONG cw = truefrom.right - truefrom.left;
        LONG ch = truefrom.top - truefrom.bottom;

	    RECT torect;
	    torect.left = to.x;
	    torect.right = to.x + cw;
	    torect.bottom = to.y;
	    torect.top = to.y + ch;

	    //xstep = dx/abs(dx);
	    //ystep = dy/abs(dy);

	    /* perform some validation on input */
	    if (!(truefrom.bottom < 0 || static_cast<unsigned>(truefrom.top) > map.y ||
		    truefrom.left < 0 || static_cast<unsigned>(truefrom.right) > map.x ||
	        torect.bottom < 0 || static_cast<unsigned>(torect.top) > map.y ||
		    torect.left < 0 || static_cast<unsigned>(torect.right) > map.x)) {

            // temporarily store the area to move before overwriting
            vector<vector<Map::Terrain>> frombuffer(cw, vector<Map::Terrain>(ch));
	        for (LONG i = 0; i < cw; i++) {
		        for (LONG j = 0; j < ch; j++) {
		            frombuffer[i][j] = map.terrain[truefrom.left + i][truefrom.bottom + j];
		        }
	        }

            // temporarily store the destination terrain before overwriting
            vector<vector<Map::Terrain>> destbuffer(cw, vector<Map::Terrain>(ch));
	        for (LONG i = 0; i < cw; i++) {
		        for (LONG j = 0; j < ch; j++) {
		            destbuffer[i][j] = map.terrain[torect.left + i][torect.bottom + j];
		        }
	        }

            if (flags & OpFlags::TERRAIN){
	            // gets priority
	            for (LONG i = 0; i < cw; i++) {
		            for (LONG j = 0; j < ch; j++) {
		                map.terrain[torect.left + i][torect.bottom + j].cnst = frombuffer[i][j].cnst;
		            }
	            }
	        }

            if (flags & OpFlags::ELEVATION){
	            // gets priority
	            for (LONG i = 0; i < cw; i++) {
		            for (LONG j = 0; j < ch; j++) {
		                map.terrain[torect.left + i][torect.bottom + j].elev = frombuffer[i][j].elev;
		            }
	            }
	        }
	    }
    }

    // each player
	for (int i = 0; i < NUM_PLAYERS; i++) {
        if (flags & OpFlags::TERRAIN){
            // camera
            if (ISINRECT(truefrom, players[i].camera[0], players[i].camera[0])) {
                players[i].camera[0] += dx;
                players[i].camera[1] += dy;
            }
        }

        if (flags & OpFlags::UNITS){
            // units
	        for (vector<Unit>::iterator iter = players[i].units.begin(); iter != players[i].units.end(); ++iter) {
                if (ISINRECT(truefrom, iter->x, iter->y)) {
		            iter->x += dx;
		            iter->y += dy;
		        }
	        }
	    }
	}

    if (!(flags & OpFlags::TRIGGERS))
	    return ERR_none;

	Trigger *trig = &(*triggers.begin());
	long num = triggers.size();

    // triggers
	long i = num;
	while (i--)
	{
	    // effects
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
		    if (ISINRECT(from, iter->location.x, iter->location.y)) {
		        iter->location.x += dx;
		        iter->location.y += dy;
		    }
		    if (ISINRECT(from, iter->area.right, iter->area.top)) {
		        iter->area.right += dx;
		        iter->area.top += dy;
		    }
		    if (ISINRECT(from, iter->area.left, iter->area.bottom)) {
		        iter->area.left += dx;
		        iter->area.bottom += dy;
		    }
		}
	    // conditions
	    for (vector<Condition>::iterator iter = trig->conds.begin(); iter != trig->conds.end(); ++iter) {
		    if (ISINRECT(from, iter->area.right, iter->area.top)) {
		        iter->area.right += dx;
		        iter->area.top += dy;
		    }
		    if (ISINRECT(from, iter->area.left, iter->area.bottom)) {
		        iter->area.left += dx;
		        iter->area.bottom += dy;
		    }
		}
		trig++;
	}

	return ERR_none;
}

AOKTS_ERROR Scenario::map_swap(const RECT &from, const POINT &to, OpFlags::Value flags)
{
	LONG dx, dy, w, h, truew, trueh;

	dx = to.x - from.left;
	dy = to.y - from.bottom;
	w = from.right - from.left;
	h = from.top - from.bottom;
    // +1 because right - left == 0 represents 1 tile
	truew = w + 1;
	trueh = h + 1;

	RECT truefrom;
	truefrom.left = from.left;
	truefrom.bottom = from.bottom;
	truefrom.right = from.left + truew;
	truefrom.top = from.bottom + trueh;

	RECT trueto;
	trueto.left = to.x;
	trueto.right = to.x + truew;
	trueto.bottom = to.y;
	trueto.top = to.y + trueh;

	RECT areato;
	areato.left = to.x;
	areato.right = to.x + w;
	areato.bottom = to.y;
	areato.top = to.y + h;

    if ((flags & OpFlags::TERRAIN) || (flags & OpFlags::ELEVATION)){
        Map::Terrain blank;
	    blank.cnst=0;
	    blank.elev=0;

	    LONG dx = 0, dy = 0;
	    dx = to.x - truefrom.left;
	    dy = to.y - truefrom.bottom;

        LONG cw = truefrom.right - truefrom.left;
        LONG ch = truefrom.top - truefrom.bottom;

	    RECT torect;
	    torect.left = to.x;
	    torect.right = to.x + cw;
	    torect.bottom = to.y;
	    torect.top = to.y + ch;

	    //xstep = dx/abs(dx);
	    //ystep = dy/abs(dy);

	    /* perform some validation on input */
	    if (!(truefrom.bottom < 0 || static_cast<unsigned>(truefrom.top) > map.y ||
		    truefrom.left < 0 || static_cast<unsigned>(truefrom.right) > map.x ||
	        torect.bottom < 0 || static_cast<unsigned>(torect.top) > map.y ||
		    torect.left < 0 || static_cast<unsigned>(torect.right) > map.x)) {

            // temporarily store the area to move before overwriting
            vector<vector<Map::Terrain>> frombuffer(cw, vector<Map::Terrain>(ch));
	        for (LONG i = 0; i < cw; i++) {
		        for (LONG j = 0; j < ch; j++) {
		            frombuffer[i][j] = map.terrain[truefrom.left + i][truefrom.bottom + j];
		        }
	        }

            // temporarily store the destination terrain before overwriting
            vector<vector<Map::Terrain>> destbuffer(cw, vector<Map::Terrain>(ch));
	        for (LONG i = 0; i < cw; i++) {
		        for (LONG j = 0; j < ch; j++) {
		            destbuffer[i][j] = map.terrain[torect.left + i][torect.bottom + j];
		        }
	        }

            if (flags & OpFlags::TERRAIN){
                // swap
	            for (LONG i = 0; i < cw; i++) {
		            for (LONG j = 0; j < ch; j++) {
		                map.terrain[truefrom.left + i][truefrom.bottom + j].cnst = destbuffer[i][j].cnst;
		            }
	            }

	            // gets priority
	            for (LONG i = 0; i < cw; i++) {
		            for (LONG j = 0; j < ch; j++) {
		                map.terrain[torect.left + i][torect.bottom + j].cnst = frombuffer[i][j].cnst;
		            }
	            }
	        }

            if (flags & OpFlags::ELEVATION){
                // swap
	            for (LONG i = 0; i < cw; i++) {
		            for (LONG j = 0; j < ch; j++) {
		                map.terrain[truefrom.left + i][truefrom.bottom + j].elev = destbuffer[i][j].elev;
		            }
	            }

	            // gets priority
	            for (LONG i = 0; i < cw; i++) {
		            for (LONG j = 0; j < ch; j++) {
		                map.terrain[torect.left + i][torect.bottom + j].elev = frombuffer[i][j].elev;
		            }
	            }
	        }
	    }
    }

    // each player
	for (int i = 0; i < NUM_PLAYERS; i++) {
        if (flags & OpFlags::TERRAIN){
            // camera
            if (ISINRECT(truefrom, players[i].camera[0], players[i].camera[0])) {
                players[i].camera[0] += dx;
                players[i].camera[1] += dy;
            }
        }

        if (flags & OpFlags::UNITS){
            // units
	        for (vector<Unit>::iterator iter = players[i].units.begin(); iter != players[i].units.end(); ++iter) {
                if (ISINRECT(truefrom, iter->x, iter->y)) {
		            iter->x += dx;
		            iter->y += dy;
		        } else if (ISINRECT(trueto, iter->x, iter->y)) {
		            iter->x -= dx;
		            iter->y -= dy;
		        }
	        }
	    }
	}

    if (!(flags & OpFlags::TRIGGERS))
	    return ERR_none;

	Trigger *trig = &(*triggers.begin());
	long num = triggers.size();

    // triggers
	long i = num;
	while (i--)
	{
	    // effects
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
		    if (ISINRECT(from, iter->location.x, iter->location.y)) {
		        iter->location.x += dx;
		        iter->location.y += dy;
		    } else if (ISINRECT(areato, iter->location.x, iter->location.y)) {
		        iter->location.x -= dx;
		        iter->location.y -= dy;
		    }
		    if (ISINRECT(from, iter->area.right, iter->area.top)) {
		        iter->area.right += dx;
		        iter->area.top += dy;
		    } else if (ISINRECT(areato, iter->area.right, iter->area.top)) {
		        iter->area.right -= dx;
		        iter->area.top -= dy;
		    }
		    if (ISINRECT(from, iter->area.left, iter->area.bottom)) {
		        iter->area.left += dx;
		        iter->area.bottom += dy;
		    } else if (ISINRECT(areato, iter->area.left, iter->area.bottom)) {
		        iter->area.left -= dx;
		        iter->area.bottom -= dy;
		    }
		}
	    // conditions
	    for (vector<Condition>::iterator iter = trig->conds.begin(); iter != trig->conds.end(); ++iter) {
		    if (ISINRECT(from, iter->area.right, iter->area.top)) {
		        iter->area.right += dx;
		        iter->area.top += dy;
		    } else if (ISINRECT(areato, iter->area.right, iter->area.top)) {
		        iter->area.right -= dx;
		        iter->area.top -= dy;
		    }
		    if (ISINRECT(from, iter->area.left, iter->area.bottom)) {
		        iter->area.left += dx;
		        iter->area.bottom += dy;
		    } else if (ISINRECT(areato, iter->area.left, iter->area.bottom)) {
		        iter->area.left -= dx;
		        iter->area.bottom -= dy;
		    }
		}
		trig++;
	}

	return ERR_none;
}

int Scenario::getPlayerCount()
{
	int ret = 0;
	Player *p;
	int i;

	FEP(p)
		ret += p->enable;

	return ret;
}

/* Map */

Map::Terrain::Terrain()
:	cnst(0), elev(1)
{
}

Map::Map()
:	aitype(-1), x(0), y(0)
{
}

void Map::reset()
{
	Terrain *parse;

	aitype = esdata.aitypes.head()->id();	// pick the first one
	x = MapSizes[1];
	y = MapSizes[1];

	parse = *terrain;
	for (int i = 0; i < MAX_MAPSIZE * MAX_MAPSIZE; i++)
	{
		parse->cnst = 0;
		parse->elev = 1;
		parse++;
	}
}

void Map::read(FILE *in, ScenVersion1 version)
{
	Terrain *t_parse;

	if (version >= SV1_AOE2TC)
		readbin(in, &aitype);

	readbin(in, &x);
	readbin(in, &y);

	/**
	 * Note that I treat x as the row indices, not the usual col indices,
	 * since this is more natural once the square is rotated into the diamond.
	 */
	for (unsigned int i = 0; i < x; i++)
	{
		unsigned int count = y;
		t_parse = terrain[i];

		while (count--)
		{
			readbin(in, t_parse++);
			readunk<char>(in, 0, "terrain zero");
		}
	}
}

void Map::write(FILE *out, ScenVersion1 version)
{
	unsigned int count;
	Terrain *t_parse;

	//there's no aitype in non-TC
	if (version >= SV1_AOE2TC)
		fwrite(this, sizeof(long), 3, out);	//aitype, x, y
	else
		fwrite(&x, sizeof(long), 2, out);	//x, y

	for (unsigned int i = 0; i < x; i++)
	{
		count = y;
		t_parse = terrain[i];

		while (count--)
		{
			fwrite(t_parse++, sizeof(Terrain), 1, out);
			putc(0, out);
		}
	}
}

bool Map::readArea(Buffer &from, const RECT &area)
{
	Terrain *parse;

	for (LONG i = area.left; i <= area.right; i++)
	{
		parse = terrain[i] + area.bottom;

		for (LONG j = area.bottom; j <= area.top; j++)
		{
			from.read(parse, sizeof(*parse));
			parse++;
		}
	}

	return true;
}

bool Map::writeArea(Buffer &b, const RECT &area)
{
	Terrain *parse;

	/* perform some validation on input */
	if (area.bottom < 0 || static_cast<unsigned>(area.top) > y ||
		area.left < 0 || static_cast<unsigned>(area.right) > x)
		return false;

	for (LONG i = area.left; i <= area.right; i++)
	{
		parse = terrain[i] + area.bottom;

		for (LONG j = area.bottom; j <= area.top; j++)
		{
			b.write(parse, sizeof(Terrain));
			parse++;
		}
	}

	return true;
}

bool Map::scaleArea(const RECT &area, const float scale)
{

    return true;
}
