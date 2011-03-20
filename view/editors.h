/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	editors.h -- Defines user interface for all non-trigger editing

	VIEW/CONTROLLER
**/

#ifndef AOKTS_EDITOR_H
#define AOKTS_EDITOR_H

#include "../model/scen.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Options */

//Number of property pages (tabs)
#define NUM_PAGES 7

/* Global Variables */

//Custom windows messages
enum AOKTS_Msgs
{
	/**
	 * Instructs active page to load new data.
	 *
	 * Return value: ignored.
	 */
	AOKTS_Loading = WM_APP,

	/**
	 * Instructs active page to save any data.
	 *
	 * Return value: ignored.
	 */
	AOKTS_Saving,

	/**
	 * Instructs active page to reset UI.
	 *
	 * Return value: ignored.
	 */
	AOKTS_Closing,
};

#define NUM_FORMATS 2

//data shared by the property pages
extern class Scenario scen;
extern struct PropSheetData
{
	int pindex;			//current player number
	class Player *p;			//current player struct
	int sel0, sel1;	//page dependant, should be reset on SETACTIVE
	HWND statusbar;
	HWND mapview;
	HMENU menu;
	UINT tformat, ecformat, mcformat;	//clipboard formats
} propdata;

/*	Note:
	Each dialog has its own DialogProc, command handler, loading,
	and saving function. Each command handler has overlapping cases
	for BN_CLICKED, LBN_SELCHANGE, CBN_SELCHANGE, menuitem clicks,
	and accelerators; as they share numerical values. This is safe
	because LBN_ and CBN_ have no 0	notification code, so the case
	will be true only for 1, that is, the _SELCHANGEs.
*/

INT_PTR CALLBACK IMsgsDlgProc(HWND page, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PlyDlgProc(HWND page, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK VictDlgProc(HWND page, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DisDlgProc(HWND page, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MapDlgProc(HWND page, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK UnitDlgProc(HWND page, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TrigDlgProc(HWND page, UINT msg, WPARAM wParam, LPARAM lParam);

#endif	//AOKTS_EDITOR_H
