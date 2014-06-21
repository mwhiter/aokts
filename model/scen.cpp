/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	scen.cpp -- Scenario wrapper code

	MODEL?
**/

#include "scen.h"

#include "../util/zlibfile.h"
#include "../util/utilio.h"
#include "../util/settings.h"
#include "../util/Buffer.h"

#include <direct.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <algorithm>
#include <functional>

using std::vector;

/* some options */
#define BMP_READ_HACK

/* PerVersions */
const PerVersion Scenario::pv1_18 =
{
	5,
	true,
	30, 20
};

const PerVersion Scenario::pv1_22 =
{
	6,
	true,
	30, 20
};

const PerVersion Scenario::pv1_23 =
{
	6,
	true,
	30, 20
};

const PerVersion Scenario::pv1_30 =
{
	6,
	true,
	60, 60
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

/* Game-specific settings */
#if (GAME == 1)
#define PLAYER_POP_UNREAD_COUNT 6

#elif (GAME == 2)
#define PLAYER_POP_UNREAD_COUNT 5

#endif

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
	ver = SVER_AOE2TC;

	/* "Blank" scenario */
	strcpy(header.version, "1.21");
	next_uid = 0;
	ver2 = 1.22F;
	memset(origname, 0, sizeof(origname));

	for (i = 0; i < 6; i++)
	{
		messages[i].erase();
		mstrings[i] = -1;
	}

	memset(cinem, 0, sizeof(cinem));
	bBitmap = false;

	for (i = 0; i < NUM_PLAYERS; i++)
		players[i].reset();
	players[0].enable = true;
	players[0].human = true;
	Player::num_players = 9;

	memset(&vict, 0, sizeof(vict));
	map.reset();

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

bool Scenario::isExpansion()
{
	bool ret;

#if (GAME == 1)
	switch (ver)
	{
	case SVER_AOE1:
	case SVER_AOE2:
	default:
		ret = false;
		break;

	case SVER_AOE2TC:
		ret = true;
	}
#elif (GAME == 2)
	switch (ver)
	{
	case SVER_AOE2TC:
	default:
		ret = false;
	}
#endif

	return ret;
}

char Scenario::StandardAI[] = "RandomGame";

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
		printf("Invalid scenario header in %s", path);
		throw runtime_error("Not a valid scenario.");
	}

	if (header.version[2] == '1' && header.version[3] == '8') // 1.18
	{
		ver = SVER_AOE2;
	}
	else if (header.version[2] == '2' && header.version[3] == '1') // 1.21
	{
		ver = SVER_AOE2TC;
	}
#ifdef _DEBUG	//testing AOE1 support in debug mode
	else if (header.version[2] == '1' &&
		(header.version[3] == '0' || header.version[3] == '1')) // 1.10 or 1.11
	{
		ver = SVER_AOE1;
	}
#endif
	else
	{
		printf("Unrecognized scenario version: %s.", header.version);
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

int Scenario::save(const char *path, const char *dpath, bool write)
{
	size_t uc_len;
	int code = 0;	//return from zlib functions

	AutoFile scx(path, "wb");

	/* Header */
	header.write(scx.get(), messages, getPlayerCount());

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
	REPORTS(check == 2, ret = false, "Header check value invalid.");

	fread(&timestamp, sizeof(timestamp), 1, scx);

	fread(&len, sizeof(long), 1, scx);
	SKIP(scx, len);	//instructions

	SKIP(scx, sizeof(long) * 2);	//unknown & playercount

	return ret;
}

void Scenario::_header::write(FILE *scx, const SString *instr, long players)
{
	long num;

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

inline int myround(float n)
{
	return int((n >= 0.0) ? n + 0.5 : n - 0.5);
}

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
	readbin(dc2in.get(), &ver2);

	printf("Version: %d...\n", myround(ver2 * 100));
	/* Set PerVersion according to ver2 */
	switch (myround(ver2 * 100))
	{
	case 118:
	case 119:
	case 120:
	case 121:
		perversion = &pv1_18;
		break;

	case 122:
		perversion = &pv1_22;
		break;

	case 123:
		perversion = &pv1_23;
		printf("SCX v%f.\n", ver2);
		break;

	case 130:
		perversion = &pv1_30;
		break;

	default:
		printf("Unrecognized scenario version2: %f.\n", ver2);
		throw bad_data_error("unrecognized format version");
	}

	FEP(p)
		p->read_header_name(dc2in.get());

	if (ver >= SVER_AOE2)
	{
		FEP(p)
			p->read_header_stable(dc2in.get());
	}

	FEP(p)
		p->read_data1(dc2in.get());

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
	/*FEP(p)
		p->read_dis_bldgsx(dc2in.get());*/
	/*readunk<long>(dc2in.get(), -1, "Disables extended", true);*/
	switch (myround(ver2 * 100))
	{
	case 123:
		readbin(dc2in.get(), &dis_bldgx);
		break;
	}
	readunk<long>(dc2in.get(), 0, "Disables unused 1", true);
	readunk<long>(dc2in.get(), 0, "Disables unused 2", true);
	readbin(dc2in.get(), &all_techs);
	FEP(p)
		p->read_age(dc2in.get());

	/* Map */

	readunk(dc2in.get(), sect, "map sect begin", true);

	players[0].read_camera_longs(dc2in.get());
	map.read(dc2in.get(), ver);

	/* Population Limits & Units */

	readunk<long>(dc2in.get(), 9, "Player count before units", true);

	for (i = PLAYER1_INDEX; i < GAIA_INDEX; i++)
		players[i].read_data4(dc2in.get(), ver);

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
	readunk<double>(dc2in.get(), 1.6, "post-PlayerData3 unknown");
	readbin(dc2in.get(), &unk2);
	check<char>(unk2, 0, "post-PlayerData3 unknown char");

	/* Triggers */

	unsigned long n_trigs = readval<unsigned long>(dc2in.get());
	triggers.allocate(n_trigs, true);

	if (n_trigs)
	{
		Trigger *t = triggers.first();

		//read triggers
		for (unsigned i = 0; i != n_trigs; ++i)
		{
			if (setts.intense)
				printf("Reading trigger %d.\n", i);
			t++->read(dc2in.get());
		}

		// Directly read trigger order: this /is/ allowed by std::vector.
		t_order.resize(n_trigs);
		readbin(dc2in.get(), &t_order.front(), n_trigs);

		// save the trigger display order to the trigger objects
		for (int i = 0; i < n_trigs; i++) {
			triggers.at(t_order[i])->display_order = i;
		}

		// verify just the first one because I'm lazy
		if (t_order.front() > n_trigs)
			throw bad_data_error("Trigger order list corrupted. Possibly out of sync.");
	}

	/* Included Files */
	readbin(dc2in.get(), &unk3);
	readbin(dc2in.get(), &unk4);

	if (unk4 == 1)
	{
		printf("unk4 hack around %x\n", ftell(dc2in.get()));
		SKIP(dc2in.get(), 396);
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

	/* save as 1.22 if it is 1.23. disable this or the below */
	/*f = ver2 - 0.01;
	if (myround(ver2 * 100) == 123) {
		writebin(dcout, &f);
	} else {
		writebin(dcout, &ver2);
	}*/

	/* saves normally. disable this or the above */
	writebin(dcout, &ver2);

	FEP(p)
		p->write_header_name(dcout);

	FEP(p)
		p->write_header_stable(dcout);
	
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
	/*FEP(p)
		fwrite(&p->dis_bldgx, 4, 1, dcout);*/
	
	/* disable this. save as 1.22 */
	switch (myround(ver2 * 100))
	{
	case 123:
	fwrite(&dis_bldgx, sizeof(long), 1, dcout);
	}
	
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
	map.write(dcout, ver);

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
		fwrite(resources, sizeof(float), PLAYER_POP_UNREAD_COUNT, dcout);

		if (ver == SVER_AOE2TC)
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
	double temp = 1.6;
	fwrite(&temp, 8, 1, dcout);
	writebin(dcout, &unk2);

	/* Triggers */

	Trigger *t_parse = triggers.first();
	num = triggers.count();
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
	vector<bool> kill(triggers.count(), true); //contains whether a trigger should be deleted

	// don't kil the ones that are still indexed on t_order
	for (vector<unsigned long>::const_iterator i = t_order.begin();
		i != t_order.end(); ++i)
		kill[*i] = false;

	/* Now delete the ones still on the kill list. We do this backwards because
	 * that only invalidates the indices on and after the current one. */
	for (size_t i = triggers.count(); i != 0; --i)
	{
		// i is the index - 1 to make i != 0 work
		size_t t_index = i - 1;

		if (!kill[t_index])	//If it shouldn't be deleted...
			continue;           //... skip all this


		triggers.remove(t_index);

		// Decrement t_order entries referencing triggers beyond the one we
		// just removed.
		for (vector<unsigned long>::iterator j = t_order.begin();
			j != t_order.end(); ++j)
		{
			if (*j > t_index)
				--(*j);
		}

		//adjust effect's trigger indexes accordingly (UGH!)
		Trigger *t_parse = triggers.first();
		for (size_t j = 0; j != triggers.count(); ++j)
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

size_t Scenario::insert_trigger(Trigger *t, size_t after)
{
	size_t tindex = triggers.append(*t);

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
	for (size_t i = 0; i != triggers.count(); ++i)
		triggers.at(i)->accept(tv);
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
:	cnst(0), elev(2)
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
	x = *MapSizes;
	y = *MapSizes;

	parse = *terrain;
	for (int i = 0; i < MAX_MAPSIZE * MAX_MAPSIZE; i++)
	{
		parse->cnst = 0;
		parse->elev = 2;
		parse++;
	}
}

void Map::read(FILE *in, ScenVersion version)
{
	Terrain *t_parse;

	if (version >= SVER_AOE2TC)
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

void Map::write(FILE *out, ScenVersion version)
{
	unsigned int count;
	Terrain *t_parse;

	//there's no aitype in non-TC
	if (version >= SVER_AOE2TC)
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
