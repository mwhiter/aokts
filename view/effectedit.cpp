/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	effectedit.cpp -- Editors for effects.

	VIEW/CONTROLLER
**/

#include "../model/scen.h"

#include "editors.h"

#include "../util/settings.h"
#include "../res/resource.h"
#include "../model/Effect.h"
#include "ecedit.h"
#include "utilui.h"
#include "LCombo.h"
#include "utilunit.h"
#include "mapview.h"
#include "../util/winugly.h"
#include <stdio.h>

extern Scenario scen;

/* Editor classes */

EditEffect::EditEffect(Effect &source)
:	e(source)
{}

void EditEffect::update(Trigger *t)
{
	t->effects[index] = e;
}

void MakeNSelectedString(int count, HWND dest)
{
	char buffer[0x100];
	char *pos = buffer;
	size_t maxbytes = sizeof(buffer) - (pos - buffer + 1);

	int ret;
	switch (count) {
	case -1:
	    ret = _snprintf(pos, maxbytes, "No units selected", count);
	    break;
	case 0:
	    ret = _snprintf(pos, maxbytes, "0 units selected", count);
	    break;
	case 1:
	    ret = _snprintf(pos, maxbytes, "1 unit selected:", count);
	    break;
	default:
	    ret = _snprintf(pos, maxbytes, "%d units selected:", count);
	    break;
	}
	pos += ret;
	*pos = '\0';

	SetWindowText(dest, buffer);
}

/*
	MakeUIDString: Fills buffer with a comma-delimited list of UIDs in e.
*/
void MakeUIDString(UID *list, int count, HWND dest)
{
	char buffer[0x100];
	char *pos = buffer;
	int ret;

	for (int i = 0; i < count; i++)
	{
		if (i)
			*pos++ = ',';

		/* this is pretty ugly here, but at least it doesn't crash */
		ret = _snprintf(pos, sizeof(buffer) - (pos - buffer + 1), "%u", list[i]);

		if (ret == -1)
			break;

		pos += ret;
	}
	*pos = '\0';

	SetWindowText(dest, buffer);
}

inline void setloc(HWND dialog, bool point)
{
	ENABLE_WND(IDC_E_LOCX, point);
	ENABLE_WND(IDC_E_LOCY, point);
	ENABLE_WND(IDC_E_LOCUID, !point);
	ENABLE_WND(IDC_E_OPENSEL2, !point);
}

// ../res/resource.h
// go to IDC_E_START

// AoK
const char etable1_aok[Effect::NUM_EFFECTS_AOK][EditEffect::N_CONTROLS] = // Using 0 instead of -1 to waste less space
{	//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// None
	{ 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change Diplomacy
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Research Technology
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Send Chat
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Play Sound
	{ 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Tribute
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Unlock Gate
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Lock Gate
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Activate Trigger
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Deactivate Trigger
	{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// AI Script Goal
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Create Object
	{ 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0 },	// Task Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Declare Victory
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Kill Object
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Remove Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change View
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0, 0 },	// Unload
	{ 0, 0, 0, 0, 2, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Change Ownership
	{ 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 },	// Patrol
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Display Instructions
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Clear Instructions
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Freeze Unit
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Use Advanced Buttons
};

// AoC v1.0c
const char etable1_10C[Effect::NUM_EFFECTS_AOC][EditEffect::N_CONTROLS] = // Using 0 instead of -1 to waste less space
{	//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// None
	{ 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change Diplomacy
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Research Technology
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Send Chat
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Play Sound
	{ 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Tribute
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Unlock Gate
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Lock Gate
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Activate Trigger
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Deactivate Trigger
	{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// AI Script Goal
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Create Object
	{ 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0 },	// Task Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Declare Victory
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Kill Object
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Remove Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change View
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0, 0 },	// Unload
	{ 0, 0, 0, 0, 2, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Change Ownership
	{ 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 },	// Patrol
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Display Instructions
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Clear Instructions
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Freeze Unit
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Use Advanced Buttons
	{ 0, 2, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Damage Object
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Place Foundation
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change Object Name
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Change Object HP
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 },	// Change Object Attack
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0 }, // Stop Unit
};

// AoC v1.4RC
const char etable1_14RC[Effect::NUM_EFFECTS_UP][EditEffect::N_CONTROLS] = // Using 0 instead of -1 to waste less space
{	//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// None
	{ 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change Diplomacy
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Research Technology
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Send Chat
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Play Sound
	{ 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Tribute
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Unlock Gate
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Lock Gate
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Activate Trigger
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Deactivate Trigger
	{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// AI Script Goal
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Create Object
	{ 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1 },	// Task Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Declare Victory
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Kill Object
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Remove Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change View
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0, 1 },	// Unload
	{ 0, 0, 0, 0, 2, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Ownership
	{ 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 },	// Patrol
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Display Instructions
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Clear Instructions
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Freeze Unit
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Use Advanced Buttons
	{ 0, 2, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Damage Object
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Place Foundation
	{ 0, 0, 0, 0, 1, 0, 2, 2, 0, 0, 1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 1 },	// Change Object Name
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Object HP
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Object Attack
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Stop Unit
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Speed
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Range
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Mele Armor
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Piercing Armor
};

// AOHD
const char etable1_aohd[Effect::NUM_EFFECTS_AOHD][EditEffect::N_CONTROLS] = // Using 0 instead of -1 to waste less space
{	//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// None
	{ 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change Diplomacy
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Research Technology
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Send Chat
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Play Sound
	{ 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Tribute
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Unlock Gate
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Lock Gate
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Activate Trigger
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Deactivate Trigger
	{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// AI Script Goal
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Create Object
	{ 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1 },	// Task Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Declare Victory
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Kill Object
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Remove Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change View
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0, 1 },	// Unload
	{ 0, 0, 0, 0, 2, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Ownership
	{ 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 },	// Patrol
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Display Instructions
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Clear Instructions
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Freeze Unit
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Use Advanced Buttons
	{ 0, 2, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Damage Object
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Place Foundation
	{ 0, 0, 0, 0, 1, 0, 2, 2, 0, 0, 1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 1 },	// Change Object Name
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Object HP
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Object Attack
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Stop Unit
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Attack-move
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Armor
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Speed
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Range
};

// SWGB
const char etable1_swgb[Effect::NUM_EFFECTS_SWGB][EditEffect::N_CONTROLS] = // Using 0 instead of -1 to waste less space
{	//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// None
	{ 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change Diplomacy
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Research Technology
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Send Chat
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Play Sound
	{ 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Tribute
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Unlock Gate
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Lock Gate
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Activate Trigger
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Deactivate Trigger
	{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// AI Script Goal
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Create Object
	{ 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1 },	// Task Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Declare Victory
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Kill Object
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Remove Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change View
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0, 1 },	// Unload
	{ 0, 0, 0, 0, 2, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Ownership
	{ 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 },	// Patrol
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Display Instructions
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Clear Instructions
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Freeze Unit
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Use Advanced Buttons
	{ 0, 2, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Damage Object
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Place Foundation
	{ 0, 0, 0, 0, 1, 0, 2, 2, 0, 0, 1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 1 },	// Change Object Name
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Object HP
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Object Attack
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Stop Unit
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Ch UP Speed - HD Attack-Move
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Ch UP Range - HD Armor
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Ch UP Armor1 - HD Range
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Ch UP Armor2 - HD Speed
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Enable Unit
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Disable Unit
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 }	// Flash Objects
};

// SWGB:CC
const char etable1_cc[Effect::NUM_EFFECTS_CC][EditEffect::N_CONTROLS] = // Using 0 instead of -1 to waste less space
{	//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// None
	{ 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change Diplomacy
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Research Technology
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Send Chat
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Play Sound
	{ 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Tribute
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Unlock Gate
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Lock Gate
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Activate Trigger
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Deactivate Trigger
	{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// AI Script Goal
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Create Object
	{ 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1 },	// Task Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Declare Victory
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Kill Object
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Remove Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Change View
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0, 1 },	// Unload
	{ 0, 0, 0, 0, 2, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Ownership
	{ 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 },	// Patrol
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Display Instructions
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },	// Clear Instructions
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Freeze Unit
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Use Advanced Buttons
	{ 0, 2, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Damage Object
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Place Foundation
	{ 0, 0, 0, 0, 1, 0, 2, 2, 0, 0, 1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 1 },	// Change Object Name
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Object HP
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Change Object Attack
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Stop Unit
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Ch UP Speed - HD Attack-Move
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Ch UP Range - HD Armor
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Ch UP Armor1 - HD Range
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Ch UP Armor2 - HD Speed
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Enable Unit
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Disable Unit
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 }, // Flash Objects
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Turn Input Off
	{ 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 1 },	// Turn Input On
};

// AoC v1.4RC virtual effects
const char etable1_14RC_virtual[Effect::NUM_VIRTUAL_EFFECTS_UP][EditEffect::N_CONTROLS] = // Using 0 instead of -1 to waste less space
{	//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// None
	{ 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Enable Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Disable Object
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Enable Technology
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Disable Technology
	{ 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Enable Technology for Any Civ
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Set HP
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Heal Object
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Set Aggressive
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Set Defensive
	{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Set Stand Ground
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },	// Set No Attack
	{ 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0 },	// Resign
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Flash Objects
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0 },	// Set AP
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0 },	// Set Control Group 1
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },	// Set Control Group 2
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0 },	// Set Control Group 3
	{ 0, 0, 0, 0, 2, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0 },	// Set Control Group 4
	{ 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0 },	// Set Control Group 5
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 },	// Set Control Group 6
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 },	// Set Control Group 7
	{ 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0 },	// Set Control Group 8
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } 	// Set Control Group 9
};

void EffectControls(HWND dialog, int type)
{
	int i;

	HWND control = GetDlgItem(dialog, IDC_E_TEXT);

	if (type >= scen.pergame->max_effect_types)
	{
		for (i = IDC_E_START; i <= IDC_E_END; i++)
			ENABLE_WND(i, true);
		ENABLE_WND(IDC_E_TEXTID, true);
		ENABLE_WND(IDC_E_SOUNDID, true);
		ENABLE_WND(IDC_E_OPENSEL, true);
		ENABLE_WND(IDC_E_OPENSEL2, true);
		SendMessage(control, EM_SETLIMITTEXT, 256, 0);
		return;
	}

	for (i = IDC_E_START; i <= IDC_E_END; i++)
		ENABLE_WND(i, setts.editall);
	ENABLE_WND(IDC_E_TEXTID, false);
	ENABLE_WND(IDC_E_SOUNDID, false);
	ENABLE_WND(IDC_E_OPENSEL, false);
	ENABLE_WND(IDC_E_OPENSEL2, false);

	const char *table;
	switch (scen.game) {
	case AOK:
	    table = etable1_aok[type];
	    break;
	case AOC:
	    table = etable1_10C[type];
	    break;
	case UP:
	    table = etable1_14RC[type];
	    break;
	case AOHD:
	case AOF:
	    table = etable1_aohd[type];
	    break;
	case SWGB:
	    table = etable1_swgb[type];
	    break;
	case SWGBCC:
	    table = etable1_cc[type];
	    break;
	default:
	    table = etable1_aok[type];
    }

	for (i = 0; i < EditEffect::N_CONTROLS; i++)
	{
		if (table[i])
		{
			switch (i)
			{
				case 4:
					ENABLE_WND(IDC_E_OPENSEL, true);
					break;
				case 5:
					ENABLE_WND(IDC_E_OPENSEL2, true);
					break;
				case 10:
					ENABLE_WND(IDC_E_TEXTID, true);
					break;
				case 11:
					ENABLE_WND(IDC_E_SOUNDID, true);
					break;
			}
			ENABLE_WND(IDC_E_START + i, true);
		}
	}

	switch (type)
	{
		case EFFECT_SendChat:
			i = 64;
			break;
		case EFFECT_DisplayInstructions:
			i = 256;
			break;
		default:
			i = 0;
			break;
	}
	SendMessage(control, EM_SETLIMITTEXT, i, 0);
}

const char *dnames[4] = { "Ally", "Neutral", "Unknown", "Enemy" };
const char *pnames[3] = { "Objectives", "Hints", "Scouts" };
const wchar_t *noselecte = L"<none>";

void E_Init(HWND dialog)
{
	Combo_Fill(dialog, IDC_E_TYPE, Effect::types,scen.pergame->max_effect_types);
	Combo_Fill(dialog, IDC_E_VTYPE, Effect::virtual_types, scen.pergame->max_virtual_effect_types + 1); // +1 for None option

	Combo_Fill(dialog, IDC_E_SPLAY, players_ec, EC_NUM_PLAYERS);
	Combo_Fill(dialog, IDC_E_TPLAY, players_ec, EC_NUM_PLAYERS);
	Combo_Fill(dialog, IDC_E_DSTATE, dnames, 4);
	Combo_Fill(dialog, IDC_E_PANEL, pnames, 3); // should this be 4?
	LCombo_Fill(dialog, IDC_E_RESEARCH, esdata.techs.head());
	LCombo_Fill(dialog, IDC_E_RESTYPE, esdata.resources.head());
	Combo_PairFill(GetDlgItem(dialog, IDC_E_GROUP), NUM_GROUPS, groups);
	Combo_PairFill(GetDlgItem(dialog, IDC_E_UTYPE), NUM_UTYPES, utypes);
	LCombo_Fill(dialog, IDC_E_UCNST, esdata.units.head(), noselecte);
}

void LoadVirtualTypeEffects(HWND dialog, EditEffect *data) {
	Effect *e = &data->e;

    switch (scen.game) {
    case AOK:
    case SWGB:
	    SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
        break;
    case UP:
        switch (e->type) {
        case 2: // Research Technology
            switch (e->panel) {
            case 1:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::EnableTechnology, 0);
                break;
            case 2:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::DisableTechnology, 0);
                break;
            case 3:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::EnableTechnologyAnyCiv, 0);
                break;
            default:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        case 11: // Create Object
            switch (e->panel) {
            case 1:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::EnableObject, 0);
                break;
            case 2:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::DisableObject, 0);
                break;
            default:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        case 16: // Change View
            switch (e->panel) {
            case 1:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::SnapView, 0);
                break;
            default:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        case 13: // Declare Victory
            switch (e->panel) {
            case 1:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::Resign, 0);
                break;
            default:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        case 18: // Change Ownership
            switch (e->panel) {
            case 1:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::FlashObjects, 0);
                break;
            default:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        case 22: // Freeze Object
            switch (e->panel) {
            case 1:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::SetAggressive, 0);
                break;
            case 2:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::SetDefensive, 0);
                break;
            case 3:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::SetStandGround, 0);
                break;
            case 4:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::SetNoAttackWithoutHalt, 0);
                break;
            default:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        case 27: // Change Object HP
            switch (e->panel) {
            case 1:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::SetHP, 0);
                break;
            case 2:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::HealObject, 0);
                break;
            default:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        case 28: // Change Object Attack
            switch (e->panel) {
            case 1:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::SetAP, 0);
                break;
            default:
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        case 29: // Stop Unit
            if (e->panel >= 1 && e->panel <= 9) {
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::SetControlGroup1 + e->panel - 1, 0);
            } else {
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        default:
	        SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
        }
        break;
    case AOC:
    case AOHD:
    case AOF:
        switch (e->type) {
        case 2: // Freeze unit
            if (e->num_sel == 0) {
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, (long)EffectVirtualTypeUP::SetControlGroup1 + e->panel - 1, 0);
            } else {
	            SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
            }
            break;
        default:
	        SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_SETCURSEL, 0, 0);
        }
        break;
    }
}

void LoadEffect(HWND dialog, EditEffect *data)
{
	Effect *e = &data->e;

	// Refresh trigger combo box.
	SendMessageW(GetDlgItem(dialog, IDC_E_TRIG), CB_RESETCONTENT, 0, 0);
	data->TrigCallback(GetDlgItem(dialog, IDC_E_TRIG), e->trig_index);

	SendDlgItemMessage(dialog, IDC_E_TYPE, CB_SETCURSEL, e->type, 0);
	SetDlgItemInt(dialog, IDC_E_TYPEVAL, e->type, TRUE);
	SetDlgItemText(dialog, IDC_E_SOUND, e->sound.c_str());
	SetDlgItemInt(dialog, IDC_E_SOUNDID, e->soundid, TRUE);

	if (e->panel >= 0 && e->panel <= 2) {
	    SendDlgItemMessage(dialog, IDC_E_PANEL, CB_SETCURSEL, e->panel, 0);
	} else {
	    SetDlgItemInt(dialog, IDC_E_PANEL, e->panel, TRUE);
	}
	SetDlgItemText(dialog, IDC_E_TEXT, e->text.c_str());
	SetDlgItemInt(dialog, IDC_E_DTIME, e->disp_time, TRUE);
	SetDlgItemInt(dialog, IDC_E_TEXTID, e->textid, TRUE);
	SendDlgItemMessage(dialog, IDC_E_SPLAY, CB_SETCURSEL, e->s_player, 0);
	SendDlgItemMessage(dialog, IDC_E_TPLAY, CB_SETCURSEL, e->t_player, 0);
	SendDlgItemMessage(dialog, IDC_E_DSTATE, CB_SETCURSEL, e->diplomacy, 0);
	SetDlgItemInt(dialog, IDC_E_LOCX, e->location.x, TRUE);
	SetDlgItemInt(dialog, IDC_E_LOCY, e->location.y, TRUE);

    MakeNSelectedString(e->num_sel, GetDlgItem(dialog, IDC_E_NSELECTED));
	if (e->num_sel > 0)
		MakeUIDString(e->uids, e->num_sel, GetDlgItem(dialog, IDC_E_UIDS));
	else
		SetDlgItemText(dialog, IDC_E_UIDS, "");

	SetDlgItemInt(dialog, IDC_E_LOCUID, e->uid_loc, TRUE);
	SetDlgItemInt(dialog, IDC_E_AREAX1, e->area.left, TRUE);
	SetDlgItemInt(dialog, IDC_E_AREAY1, e->area.bottom, TRUE);
	SetDlgItemInt(dialog, IDC_E_AREAX2, e->area.right, TRUE);
	SetDlgItemInt(dialog, IDC_E_AREAY2, e->area.top, TRUE);
	SetDlgItemInt(dialog, IDC_E_AIGOAL, e->ai_goal, TRUE);
	LCombo_Select(dialog, IDC_E_UCNST, e->pUnit);
	Combo_SelectByData(GetDlgItem(dialog, IDC_E_GROUP), e->group);
	Combo_SelectByData(GetDlgItem(dialog, IDC_E_UTYPE), e->utype);
	LCombo_Select(dialog, IDC_E_RESEARCH, e->pTech);
	SetDlgItemInt(dialog, IDC_E_AMOUNT, e->amount, TRUE);
	LCombo_SelById(dialog, IDC_E_RESTYPE, e->res_type);
	SetDlgItemInt(dialog, IDC_E_TRIGID, e->trig_index, TRUE);
	if (e->pUnit) {
	    SetDlgItemInt(dialog, IDC_E_UCNSTID, e->pUnit->id(), TRUE);
	} else {
	    SetDlgItemInt(dialog, IDC_E_UCNSTID, -1, TRUE);
	}

	LoadVirtualTypeEffects(dialog, data);
}

void SaveEffect(HWND dialog, EditEffect *data)
{
	Effect *e = &data->e;

    // CB_ERR is returned from no selection. this shows that CB_ERR == -1.
	//MessageBox(dialog, std::string("CB_ERR").append(toString<int>(CB_ERR)).c_str(), "Effect Editor", MB_OKCANCEL);
	// that means that when an item is not selected in a combobox,
	// CB_GETCURSEL saves -1. I think that this is the default value for
	// unused fields in AoK.  However, what about Azzzru's panel having no
	// value? That is different as what he does shortens the SCX file.
	// How does AoKTS load his scenarios then? This means that a -1
	// value for panel should not give error to aokts.
	int newtype = SendDlgItemMessage(dialog, IDC_E_TYPE, CB_GETCURSEL, 0, 0);
	if (newtype != CB_ERR)
	{
		e->type = newtype;
		GetWindowText(GetDlgItem(dialog, IDC_E_SOUND), e->sound);
		e->soundid = GetDlgItemInt(dialog, IDC_E_SOUNDID, NULL, TRUE);
		BOOL translated = NULL;
		int panel = GetDlgItemInt(dialog, IDC_E_PANEL, &translated, TRUE);
		if (!translated) {
		    SString panelname;

            GetWindowText(GetDlgItem(dialog, IDC_E_PANEL), panelname);
		    panel = SendDlgItemMessage(dialog, IDC_E_PANEL, CB_FINDSTRING, -1, (LPARAM) (LPCTSTR)panelname.c_str());

		    if (panel == CB_ERR)
		        panel = -1;
		}
		e->panel = panel;
		GetWindowText(GetDlgItem(dialog, IDC_E_TEXT), e->text);
		e->disp_time = GetDlgItemInt(dialog, IDC_E_DTIME, NULL, TRUE);
		e->pUnit = (UnitLink*)LCombo_GetSelPtr(dialog, IDC_E_UCNST);
		e->textid = GetDlgItemInt(dialog, IDC_E_TEXTID, NULL, TRUE);
		e->s_player = SendDlgItemMessage(dialog, IDC_E_SPLAY, CB_GETCURSEL, 0, 0);
		e->t_player = SendDlgItemMessage(dialog, IDC_E_TPLAY, CB_GETCURSEL, 0, 0);
		e->diplomacy = (enum Diplomacy)SendDlgItemMessage(dialog, IDC_E_DSTATE, CB_GETCURSEL, 0, 0);
		e->location.x = GetDlgItemInt(dialog, IDC_E_LOCX, NULL, TRUE);
		e->location.y = GetDlgItemInt(dialog, IDC_E_LOCY, NULL, TRUE);
		e->area.left = GetDlgItemInt(dialog, IDC_E_AREAX1, NULL, TRUE);
		e->area.bottom = GetDlgItemInt(dialog, IDC_E_AREAY1, NULL, TRUE);
		e->area.right = GetDlgItemInt(dialog, IDC_E_AREAX2, NULL, TRUE);
		e->area.top = GetDlgItemInt(dialog, IDC_E_AREAY2, NULL, TRUE);
		e->pTech = (TechLink*)LCombo_GetSelPtr(dialog, IDC_E_RESEARCH);
		e->ai_goal = GetDlgItemInt(dialog, IDC_E_AIGOAL, NULL, TRUE);
		e->amount = GetDlgItemInt(dialog, IDC_E_AMOUNT, NULL, TRUE);
		e->res_type = LCombo_GetSelId(dialog, IDC_E_RESTYPE);

		//get the data, not the index, for these
		e->trig_index = Combo_GetSelData(GetDlgItem(dialog, IDC_E_TRIG));
		e->group = Combo_GetSelData(GetDlgItem(dialog, IDC_E_GROUP));
		e->utype = Combo_GetSelData(GetDlgItem(dialog, IDC_E_UTYPE));
	}
}

/*	Should be:

	Patrol, Task Object, Remove Object, Rename, Change HP, Change Attack, Freeze Unit,
	Stop Unit, Kill Object, Damage Object.
*/
const bool multiSelEffect[] =
{
	false,	// None
	false,	// Change Diplomacy
	false,	// Research Technology
	false,	// Send Chat
	false,	// Play Sound
	false,	// Tribute
	false,	// Unlock Gate
	false,	// Lock Gate
	false,	// Activate Trigger
	false,	// Deactivate Trigger
	false,	// AI Script Goal
	false,	// Create Object
	true,	// Task Object
	false,	// Declare Victory
	true,	// Kill Object
	true,	// Remove Object
	false,	// Change View
	false,	// Unload
	false,	// Change Ownership
	true,	// Patrol
	false,	// Display Instructions
	false,	// Clear Instructions
	true,	// Freeze Unit
	false,	// Use Advanced Buttons
	true,	// Damage Object
	false,	// Place Foundation
	true,	// Change Object Name
	true,	// Change Object HP
	true,	// Change Object Attack
	true,	// Stop Unit
	true,	// Ch UP Speed - HD Attack-Move
	true,	// Ch UP Range - HD Armor
	true,	// Ch UP Armor1 - HD Range
	true,	// Ch UP Armor2 - HD Speed
	true,   // Enable Unit
	true,   // Disable Unit
	true    // Flash Objects
};

void OnOpenSel(HWND dialog, EditEffect *data)
{
	INT_PTR ret;
	struct UnitEdit ue =
	{ data->players, data->e.s_player, data->e.num_sel };
	if (data->e.num_sel > 0)
		memcpy(ue.ids, data->e.uids, sizeof(UID) * data->e.num_sel);

	ret = UnitSelDialogBox(
		GetModuleHandle(NULL), dialog, ue, multiSelEffect[data->e.type]);

	if (ret)
	{
		data->e.s_player = ue.player;
		data->e.num_sel = ue.count;
		memcpy(data->e.uids, ue.ids, sizeof(UID) * ue.count);

		/* Update controls */
		MakeNSelectedString(data->e.num_sel, GetDlgItem(dialog, IDC_E_NSELECTED));
		MakeUIDString(data->e.uids, data->e.num_sel, GetDlgItem(dialog, IDC_E_UIDS));
		SendDlgItemMessage(dialog, IDC_E_SPLAY, CB_SETCURSEL, data->e.s_player, 0);
	}
}

void E_HandleSetFocus(HWND dialog, WORD id)
{
	EditEffect * data =
		static_cast<EditEffect*>(GetDialogUserData_ptr(dialog));

	if (data->mapview)
	{
		switch (id)
		{
		case IDC_E_LOCX:
		case IDC_E_LOCY:
			SendMessage(data->mapview, MAP_HighlightPoint,
				GetDlgItemInt(dialog, IDC_E_LOCX, NULL, TRUE),
				GetDlgItemInt(dialog, IDC_E_LOCY, NULL, TRUE));
			break;

		case IDC_E_AREAX1:
		case IDC_E_AREAY1:
		case IDC_E_AREAX2:
		case IDC_E_AREAY2:
			SendMessage(data->mapview, MAP_HighlightPoint,
				GetDlgItemInt(dialog, IDC_E_AREAX1, NULL, TRUE),
				GetDlgItemInt(dialog, IDC_E_AREAY1, NULL, TRUE));
			SendMessage(data->mapview, MAP_HighlightPoint,
				GetDlgItemInt(dialog, IDC_E_AREAX2, NULL, TRUE),
				GetDlgItemInt(dialog, IDC_E_AREAY2, NULL, TRUE));
			break;
		}
	}
}

void E_HandleKillFocus(HWND dialog, WORD)
{
	EditEffect * data =
		static_cast<EditEffect*>(GetDialogUserData_ptr(dialog));

	if (data->mapview)
		SendMessage(data->mapview, MAP_UnhighlightPoint,
			MAP_UNHIGHLIGHT_ALL, 0);
}

void E_HandleChangeType(HWND dialog, EditEffect *data)
{
/*
	static const char *promptClearEffect =
		"Would you like to clear the effect? (Recommended)";

	if (data->e.type != EFFECT_None &&
		MessageBox(dialog, promptClearEffect, "Effect Editor", MB_YESNO) == IDYES)
	{
		data->e = Effect();
	}
*/
    // Assume the user wants to change to similar effect. Otherwise
    // would make new
	int newtype = SendDlgItemMessage(dialog, IDC_E_TYPE, CB_GETCURSEL, 0, 0);
	//if (data->e.type != newtype)
	//	data->e = Effect();
	data->e.type = newtype;
	EffectControls(dialog, newtype);
	LoadEffect(dialog, data);
}

void E_HandleChangeVType(HWND dialog, EditEffect *data)
{
	int newtype = SendDlgItemMessage(dialog, IDC_E_VTYPE, CB_GETCURSEL, 0, 0);
	if (newtype == 0) {
	    EffectControls(dialog, data->e.type);
	    return;
	}

	data->e = Effect();

    switch (scen.game) {
    case UP:
        switch (newtype) {
        case 0: // None
            //data->e.panel = -1;
            break;
        case 1: // Enable Object
            data->e.panel = 1;
            data->e.type = 11;
            break;
        case 2: // Disable Object
            data->e.panel = 2;
            data->e.type = 11;
            break;
        case 3: // Enable Technology
            data->e.panel = 1;
            data->e.type = 2;
            break;
        case 4: // Disable Technology
            data->e.panel = 2;
            data->e.type = 2;
            break;
        case 5: // Enable Tech For Any Civ
            data->e.panel = 3;
            data->e.type = 2;
            break;
        case 6: // Set HP
            data->e.panel = 1;
            data->e.type = 27;
            break;
        case 7: // Heal Object
            data->e.panel = 2;
            data->e.type = 27;
            break;
        case 8: // Set Aggressive
            data->e.panel = 1;
            data->e.type = 22;
            break;
        case 9: // Set Defensive
            data->e.panel = 2;
            data->e.type = 22;
            break;
        case 10: // Stand Ground
            data->e.panel = 3;
            data->e.type = 22;
            break;
        case 11: // No Attack Without Halt
            data->e.panel = 4;
            data->e.type = 22;
            break;
        case 12: // Resign
            data->e.panel = 1;
            data->e.type = 13;
            break;
        case 13: // Flash Objects
            data->e.panel = 1;
            data->e.type = 18;
            break;
        case 14: // Set AP
            data->e.panel = 1;
            data->e.type = 28;
            break;
        case 24: // Snap View
            data->e.panel = 1;
            data->e.type = 16;
            break;
        }
        if (newtype >=15 && newtype <= 23) { // Set control group 1-9
            data->e.panel = newtype - 14;
            data->e.type = 29;
        }
        break;
    default:
	    EffectControls(dialog, data->e.type);
	    // change to:
	    //VirtualEffectControls(dialog, data->e.type);
    }

	LoadEffect(dialog, data);
}

const char warnInvalidE[] =
"Watch out, this effect appears to be invalid.";

const char warnWeirdResource[] =
"The resource you selected is non-standard and may have unpredictable consequences.";

void E_HandleCommand(HWND dialog, WORD id, WORD code, HWND control)
{
	/* Most of them use this */
	class EditEffect *data = (EditEffect*)GetWindowLongPtr(dialog, DWLP_USER);

	switch (code)
	{
	case BN_CLICKED:
		switch (id)
		{
		case IDC_E_AREA_ALL:
			{
				SetDlgItemInt(dialog, IDC_E_AREAX1, -1, TRUE);
				SetDlgItemInt(dialog, IDC_E_AREAY1, -1, TRUE);
				SetDlgItemInt(dialog, IDC_E_AREAX2, -1, TRUE);
				SetDlgItemInt(dialog, IDC_E_AREAY2, -1, TRUE);
			}
			break;

		case IDC_E_CLEAR:
			{
			    data->e = Effect();
			    LoadEffect(dialog, data);
			}
			break;

		case IDOK:
			{
				bool valid;
				int ret = IDOK;

				SaveEffect(dialog, data);
				valid = data->e.check();

		        if (!valid && !setts.editall)
					ret = MessageBox(dialog, warnInvalidE, "Effect Editor", MB_OKCANCEL);

				if (ret == IDOK)
				{
					SendMessage(data->parent, EC_Closing,
						MAKELONG(1, valid), reinterpret_cast<LPARAM>(data));
					DestroyWindow(dialog);
				}
			}
			break;

		case IDCANCEL:
			SendMessage(data->parent, EC_Closing,
				0, reinterpret_cast<LPARAM>(data));
			DestroyWindow(dialog);
			break;

		case IDC_E_OPENSEL:
			OnOpenSel(dialog, data);
			break;

		case IDC_E_OPENSEL2:
			if (SingleUnitSelDialogBox(dialog, data->players,
				data->e.uid_loc, (data->e.uid_loc != -1)))
			{
				SetDlgItemInt(dialog, IDC_E_LOCUID, data->e.uid_loc, FALSE);
			}
			break;

		}
		break;

	case CBN_SELCHANGE:
		switch (id)
		{
			case IDC_E_TYPE:
				E_HandleChangeType(dialog, data);
				break;

			case IDC_E_VTYPE:
				E_HandleChangeVType(dialog, data);
				break;

			case IDC_E_RESTYPE:
				if (SendMessage(control, CB_GETCURSEL, 0, 0) >= NUM_STYPES &&
					setts.warnbits & WARN_WEIRDRESOURCE)
					MessageBox(dialog, warnWeirdResource, "Warning", MB_ICONWARNING);
				break;

			case IDC_E_TRIG:
				SetDlgItemInt(dialog, IDC_E_TRIGID, data->e.trig_index, TRUE);
				break;
		}
		break;

	case EN_SETFOCUS:
		E_HandleSetFocus(dialog, id);
		break;

	case EN_KILLFOCUS:
		E_HandleKillFocus(dialog, id);
		break;
	}
}

const char errorNoDataE[] =
"No Data! Functionality will be impaired.";

const char noteTrigDeleted[] =
"Selected trigger deleted.";

/**
 * Initializes the trigger editor, namely the tree's image list, in response to
 * WM_INITDIALOG.
*/
BOOL Handle_WM_INITDIALOG(HWND dialog, class EditEffect* data)
{
	E_Init(dialog);

	if (!data)
	{
		MessageBox(dialog, errorNoDataE, "Error", MB_ICONWARNING);
		DestroyWindow(dialog);
        return FALSE;
	}

	LoadEffect(dialog, data);

	SetDialogUserData(dialog, data);
	EffectControls(dialog, data->e.type);

    return TRUE;
}

INT_PTR CALLBACK EffectWndProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;
	class EditEffect *data;	//most use it, but we don't know where it comes from yet

	switch (msg)
	{
	case WM_INITDIALOG:
		return Handle_WM_INITDIALOG(dialog, (EditEffect*)lParam);

	case WM_COMMAND:
		ret = 0;
		E_HandleCommand(dialog, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;

	case WM_NOTIFY:
		{
			NMHDR *header = (NMHDR*)lParam;
			switch (header->code)
			{
			case PSN_SETACTIVE:
		        return Handle_WM_INITDIALOG(dialog, (EditEffect*)lParam);
			}
		}
		break;

	case AOKTS_Loading:
		return Handle_WM_INITDIALOG(dialog, (EditEffect*)lParam);

	case WM_DESTROY:
		data = (EditEffect*)GetWindowLongPtr(dialog, DWLP_USER);
		delete data;
		break;

	case EC_Update:
		data = (EditEffect*)GetWindowLongPtr(dialog, DWLP_USER);
		data->index += wParam;
		break;

	case EC_RefreshTriggers:
		{
			HWND combobox = GetDlgItem(dialog, IDC_E_TRIG);
			data = (EditEffect*)GetWindowLongPtr(dialog, DWLP_USER);

			SendMessage(combobox, CB_RESETCONTENT, 0, 0);
			data->TrigCallback(combobox, data->e.trig_index);

			if (wParam == data->e.trig_index)
				MessageBox(dialog, noteTrigDeleted, "Effect Editor Notice", MB_OK);
		}
		break;
	}

	return ret;
}
