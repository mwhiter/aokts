/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.

	utilunit.h: Declares various utility functions for units management.

	VIEW/CONTROLLER
**/

#ifndef UTILUNIT_H
#define UTILUNIT_H

#include "../model/scen.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/** Sorting **/

#define NUM_SORTS	2
enum Sorts
{
	SORT_ID,
	SORT_Name
};
extern const char *sorts[NUM_SORTS];

/** Utility Functions **/

/*
	UnitList_Fill: Resets, then fills a unit listbox with data from an array of Unit structs.
*/
void UnitList_Fill(HWND listbox,		/* The listbox to fill. */
				   enum Sorts sorttype,
				   const std::vector<Unit>& list);

/**
 * Appends a new item in the listbox and returns its position index.
 */
int UnitList_Append(HWND listbox, const Unit * unit, size_t u_index);

/*
	UnitList_InsertItem: Inserts a new item in the listbox (in sort order)
		and returns its position.
*/
int UnitList_InsertItem(HWND listbox, enum Sorts sorttype,
						const std::vector<Unit>& list, const Unit *which);

/*
	UnitList_FillGroup: Fills a listbox with units in a group. Pass NULL as group
		to load all units.
*/
void UnitList_FillGroup(HWND typebox, const UnitGroupLink *group);

/* Selector Dialog */

/**
 * This structure contains parameters for the UnitSel dialog box.
 */
struct UnitEdit
{
	// Vector of players from which to display units.
	Player const * players;
	// If the initial player is known, caller may specify it here, with E/C
	// numbering. Otherwise, the dialog box will discover the player number
	// from the specified ids.
	int player;
	int count;
	UID ids[MAX_UNITSEL];
};

struct PlayersUnit
{
    int player;
    Unit * u;

    PlayersUnit() : player(-1), u(NULL)
    {
    }
};

std::string get_unit_full_name(UID id);

PlayersUnit find_map_unit(UID id);

/*
 * Displays a modal unit-selection dialog box.
 *
 * @param instance Module in which dialog templates reside.
 * @param parent Parent window of the dialog box.
 * @param ue UnitEdit structure defining properties of the selector
 * @param multisel true if multiple units should be selectable
 * @return non-zero if the user pressed "OK", zero otherwise
 */
INT_PTR UnitSelDialogBox(HINSTANCE instance,
						 HWND parent,
						 UnitEdit& ue,
						 bool multisel = false);

/**
 * Displays a modal, single unit selection dialog box.
 *
 * @param parent The parent window of the dialog box.
 * @param players Array of Players to display. (Must have NUM_PLAYERS entries.)
 * @param uid In/Out Specifies initial UID to select, returns new selection.
 * @param uid_specified Set true if uid contains a meaningful initial value.
 *
 * @return true if the user pressed "OK", false if "Cancel"
 */
bool SingleUnitSelDialogBox(HWND parent, Player const * players,
							UID& uid, bool uid_specified = false);

#endif	//UTILUNIT_H
