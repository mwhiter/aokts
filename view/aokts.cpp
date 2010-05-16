/*
	AOK Trigger Studio / SWGB Trigger Studio
	by David Tombs (aka DiGiT): cyan.spam@gmail.com
	WINDOWS VERSION.

	-------------------------------------------------------------------------------

	Copyright (C) 2007 David Tombs

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

	-------------------------------------------------------------------------------

	This source will compile to both AOKTS and SWGBTS depending the the
	definition of the preprocessor macro GAME (aok=1 or swgb=2). Default
	is aok.

	aokts.cpp -- Defines the core functions of AOKTS

	VIEW/CONTROLLER
*/

#include "../util/utilio.h"
#include "../util/zlibfile.h"
#include "../util/settings.h"
#include "../model/scen.h"
#include "../model/TrigXmlVisitor.h"
#include "../model/TrigXmlReader.h"
#include "editors.h"
#include "mapview.h"
#include "../util/winugly.h"
#include "utilui.h"

#include <commdlg.h>
#include "LCombo.h"
#include "../resource.h" // must be included after Windows stuff
#include <ctype.h>
#include <fstream>

/* Microsoft-specific stuff */
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

/** External Globals (see respective headers for descriptions) **/

PropSheetData propdata;
Setts setts;
ESDATA esdata;

/** Internal Globals **/

HINSTANCE aokts;

/*
	The system-defined DialogProc for property sheets will
	be stored here when it is replaced with SetWindowLongPtr().
	The replacement DialogProc will call this for any messages
	it does not process.
*/
DLGPROC pproc;

/*
	These are the IDs of the standard Property Sheet buttons.
	They are disabled and hidden since AOKTS uses essentially
	a hack of a Property Sheet. (The menu covers these
	functions, instead.)
*/
const WORD PropSheetButtons[] =
{ IDOK, IDCANCEL, IDHELP };

/* Each editor's property page proc (in order of Dialog ID). */
DLGPROC procs[NUM_PAGES] =
{
	&IMsgsDlgProc,
	&PlyDlgProc,
	&VictDlgProc,
	&DisDlgProc,
	&MapDlgProc,
	&UnitDlgProc,
	&TrigDlgProc
};

/* Strings */
const char * askSaveChanges =
"Do you want to save your changes?";

#if (GAME == 2)

const char *szTitle = "SWGB Trigger Studio";
const char welcome[] =
"Welcome to SWGBTS! Please open a scenario or make a new one.";
const char warnNoVersionChange[] =
"Warning: SWGBTS cannot yet convert between versions. Saving an sc1"
" as an scx will only change the extension!";
const char extOpen[] =
"All Scenarios (*.scx, *.sc1)\0*.scx;*.sc1\0All files (*.*)\0*.*\0";
const char extSave[] =
"Scenarios (*.scx)\0*.scx\0Expansion Scenarios (*.sc1)\0*.sc1\0All files\0*.*\0";
const char datapath[] = "data_swgb.xml";
const char warnNoAOEII[] =
"You don't seem to have Star Wars: Galactic Battlegrounds installed. I don't know how useful this will be.";

#elif (GAME == 1)

const char *szTitle = "AOK Trigger Studio";
const char welcome[] =
"Welcome to AOKTS! Please open a scenario or make a new one.";
const char warnNoVersionChange[] =
"Warning: AOKTS cannot yet convert between versions. Saving an scx"
" as an scn will only change the extension!";
const char extOpen[] =
"All Scenarios (*.scn, *.scx)\0*.scn;*.scx\0All files (*.*)\0*.*\0";
const char extSave[] =
"Scenarios (*.scn)\0*.scn\0Expansion Scenarios (*.scx)\0*.scx\0All files\0*.*\0";
const char datapath[] = "data_aok.xml";
const char warnNoAOEII[] =
"You don't seem to have Age of Empires II installed. I don't know how useful this will be.";

#else

#error unsupported game

#endif

/** Functions **/

const char *getFilenameFromPath(const char *path)
{
	const char *ret;

	ret = strrchr(path, '\\') + 1;
	if (!ret)
	{
		ret = strrchr(path, '/') + 1;

		if (!ret)
			ret = path;
	}

	return ret;
}

/*
	SetSaveState: Sets state of Save/Save As buttons on the File menu.

	HWND window: Handle to the window that owns the menu.
	UINT state: The new state for each.
*/
inline void SetSaveState(HWND window, UINT state)
{
	HMENU menu = GetMenu(window);
	EnableMenuItem(menu, ID_FILE_SAVE, state);
	EnableMenuItem(menu, ID_FILE_SAVE_AS, state);
	EnableMenuItem(menu, ID_FILE_SAVE_AS2, state);
}

/*
	UpdateRecentMenu: Updates both the state and the text of the recent file menu items.
*/
void UpdateRecentMenu(HMENU menu)
{
	RecentFile *rfile;
	WORD item;
	MENUITEMINFO menuitem;
	char buffer[_MAX_FNAME];

	rfile = setts.recent_first;
	item = 0;

	menuitem.cbSize = sizeof(MENUITEMINFO);
	menuitem.fMask = MIIM_TYPE | MIIM_STATE;	//can't use MIIM_STRING, too new
	menuitem.fType = MFT_STRING;	//must set this with MIIM_TYPE.
	menuitem.dwTypeData = buffer;
	menuitem.fState = MFS_ENABLED;

	while (rfile && item < MAX_RECENT)
	{
		sprintf(buffer, "&%d %s", item + 1, rfile->display);
		SetMenuItemInfo(menu, ID_FILE_RECENT1 + item++, FALSE, &menuitem);
		rfile = rfile->next;
	}
}

/*
	FileSave: Handles a request to save the currently open file.

	Parameters:
	HWND sheet: Handle to the property sheet.
	bool as:		Save or Save As?
*/
void FileSave(HWND sheet, bool as, bool write)
{
	int error;			//error value from Scenario::save()
	HWND cpage;			//the current property page
	HCURSOR previous;	//the mouse cursor before/after save operation
	OPENFILENAME ofn;
	char titleBuffer[100];

	//init
	cpage = PropSheet_GetCurrentPageHwnd(sheet);

	//Save As: Ask for filename.
	if (as || *setts.ScenPath == '\0')
	{
		char dir[_MAX_PATH];
		strcpy(dir, setts.BasePath);
		strcat(dir, "Scenario");

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = sheet;
		ofn.lpstrFilter = extSave;
		ofn.lpstrCustomFilter = NULL;
		ofn.nFilterIndex = scen.isExpansion() + 1;
		ofn.lpstrFile = setts.ScenPath;
		ofn.nMaxFile = _MAX_PATH;
		ofn.lpstrFileTitle = NULL;
		ofn.lpstrInitialDir = dir;
		ofn.lpstrTitle = NULL;
		ofn.Flags = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = "scx";

		if (!GetSaveFileName(&ofn))
			return;

		if (ofn.nFilterIndex != (unsigned)scen.isExpansion() + 1)
			MessageBox(sheet, warnNoVersionChange, "aokts", MB_ICONWARNING);

		if (!*scen.origname)
			strcpy(scen.origname, setts.ScenPath + ofn.nFileOffset);

		/* Update window title since filename has probably changed */
		_snprintf(titleBuffer, sizeof(titleBuffer),
			"%s - %s", szTitle, setts.ScenPath + ofn.nFileOffset);
		SetWindowText(sheet, titleBuffer);
	}

	//update scenario data
	SendMessage(cpage, AOKTS_Saving, 0, 0);

	//perform before-saving operations
	previous = SetCursor(LoadCursor(NULL, IDC_WAIT));
	SendMessage(cpage, AOKTS_Closing, 0, 0);	//for IDD_TRIGGERS, old data after scen.clean_triggers()
	scen.clean_triggers();

	//save the scenario
	try
	{
		error = scen.save(setts.ScenPath, setts.TempPath, write);
		SetCursor(previous);
	}
	catch (std::exception &ex)
	{
		// TODO: better atomic cursor handling?
		SetCursor(previous);
		MessageBox(sheet, ex.what(), "Scenario Save Error", MB_ICONWARNING);
	}

	//perform after-saving operations
	SendMessage(cpage, AOKTS_Loading, 0, 0);
	SetWindowText(propdata.statusbar, scen.msg);

	//report any errors
	if (error)
	{
		MessageBox(sheet, scen.msg, "Save Error", MB_OK);
	}
}

/*
	FileOpen: Handles a request to open a file. (Either by menu or generated by the app.)
	
	Parameters:
	HWND sheet: Handle to the property sheet.
	bool ask:	Should AOKTS ask the user which file?
	int recent:	Optionally open from one of the recent file entries. (-1 to disable.)
*/
void FileOpen(HWND sheet, bool ask, int recent)
{
	OPENFILENAME ofn;
	struct RecentFile *file = NULL;	//the file info will be stored here one way or another
	char titleBuffer[100];
	const char *filename;

	HWND page = PropSheet_GetCurrentPageHwnd(sheet);

	//save the scenario if changes have been made (NOT FUNCTIONAL)
	if (scen.needsave())
	{
		int sel = MessageBox(sheet, askSaveChanges, "Save", MB_YESNOCANCEL);

		if (sel == IDYES)
			FileSave(sheet, false, true);

		else if (sel == IDCANCEL)
			return;	//stop closing
	}

	/* Using a recent file... */
	if (recent >= 0)
	{
		ofn.Flags = 0;	//make sure no random flags set
		file = setts.recent_first;

		/* Parse the linked list to find the one we want */
		while (recent--)
		{
			if (file)
				file = file->next;
			else
			{
				MessageBox(sheet,
					"Warning: Recent File open failed.",
					"Open Warning", MB_ICONWARNING);
			}
		}

		strcpy(setts.ScenPath, file->path);
	}
	/* Prompt the user for a filename. */
	else if (ask)
	{
		struct RecentFile *r_parse;
		char dir[_MAX_PATH];
		strcpy(dir, setts.BasePath);
		strcat(dir, "Scenario");

		ofn.lStructSize =	sizeof(OPENFILENAME);
		ofn.hwndOwner =		sheet;
		//ofn.hInstance unused
		ofn.lpstrFilter =	extOpen;
		ofn.lpstrCustomFilter = NULL;	//user should not set custom filters
		ofn.nFilterIndex =	1;
		ofn.lpstrFile =		setts.ScenPath;
		ofn.nMaxFile =		_MAX_PATH;
		ofn.lpstrFileTitle = NULL;
		ofn.lpstrInitialDir = dir;
		ofn.lpstrTitle =	NULL;
		ofn.Flags =			OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;
		ofn.lpstrDefExt =	"scx";
		
		if (!GetOpenFileName(&ofn))
			return;

		/* Now check if selected file is already on recent list. */
		r_parse = setts.recent_first;
		while (r_parse)
		{
			if (!strcmp(r_parse->path, setts.ScenPath))
			{
				file = r_parse;
				break;
			}
			r_parse = r_parse->next;
		}
	}

	/* Handle recent file stuff */
	if (!file)
	{
		file = setts.recent_getnext();
		strcpy(file->path, setts.ScenPath);
		strcpy(file->display, setts.ScenPath + ofn.nFileOffset);
	}
	setts.recent_push(file);
	UpdateRecentMenu(propdata.menu);

	/* Open it! */
	SendMessage(page, AOKTS_Closing, 0, 0);
	// set hourglass, might take more than 1ms
	HCURSOR previous = SetCursor(LoadCursor(NULL, IDC_WAIT));
	scen.reset();
	try
	{
		scen.open(setts.ScenPath, setts.TempPath);
		SetCursor(previous);
		SetSaveState(sheet, ofn.Flags & OFN_READONLY ? MF_GRAYED : MF_ENABLED);

		// set status bar text
		SetWindowTextW(propdata.statusbar, L"Scenario loaded successfully.");
	}
	catch (std::exception &ex)
	{
		// TODO: better atomic cursor handling?
		SetCursor(previous);

		// set status bar text
		SetWindowText(propdata.statusbar, ex.what());

		// report error to user
		std::string desc = "Sorry, could not read scenario file. ";
		desc.append(ex.what());
		desc.append("\n\nIf the game can open the scenario, please report this error.");
		printf("User message: %s\n", desc.c_str());
		MessageBox(sheet, desc.c_str(), "Scenario Load Error", MB_ICONWARNING);

		// unless it's a debug build, clear the bad data
	#ifndef _DEBUG
		scen.reset();
	#endif
	}

	//report errors to logfile
	fflush(stdout);

	/* Updates*/
	SendMessage(page, AOKTS_Loading, 0, 0);
	MapView_Reset(propdata.mapview, true);

	filename = getFilenameFromPath(setts.ScenPath);

	_snprintf(titleBuffer, sizeof(titleBuffer),
		"%s - %s", szTitle, filename);

	SetWindowText(sheet, titleBuffer);
}

/*
	FileClose: Handles user close request.
*/
void FileClose(HWND sheet, HWND control)
{
	HWND page = (HWND)SendMessage(sheet, PSM_GETCURRENTPAGEHWND, 0, 0);
	int sel = IDYES;

	if (!control)
		SendMessage(page, AOKTS_Closing, 0, 0);

	if (scen.needsave())
	{
		sel = MessageBox(sheet, "Do you want to save your changes?", "Save", MB_YESNOCANCEL);
		if (sel == IDYES)
			FileSave(sheet, false, true);
		else if (sel == IDCANCEL)
			return;	//stop closing
	}
	scen.reset();
	
	if (!control)
		SendMessage(page, AOKTS_Loading, 0, 0);

	SetSaveState(sheet, MF_ENABLED);
	SetWindowText(propdata.statusbar, "Scenario reset.");
	SendMessage(propdata.mapview, MAP_Reset, 0, 0);

	SetWindowText(sheet, szTitle);
}

/**
 * Handles a user request to dump triggers to textual format.
 */
void OnFileTrigWrite(HWND dialog)
{
	char path[MAX_PATH] = "trigs.xml";

	// TODO: set the path to aokts directory.
	if (!GetSaveFileNameA(dialog, path, MAX_PATH))
		return;

	AutoFile textout(path, "w");
	scen.accept(TrigXmlVisitor(textout));
}

/**
 * Handles a user request to read triggers from above textual format.
 */
void OnFileTrigRead(HWND dialog)
{
	char path[MAX_PATH] = "";

	if (!GetOpenFileNameA(dialog, path, MAX_PATH))
		return;

	std::ifstream textin(path, std::ios_base::in);
	TrigXmlReader reader;
	reader.read(textin);
}

/*
	PropSheetProc: Handles special messages pertaining to the property sheet.

	Note: See PropSheetProc in the Platform SDK docs for parameters and notes.
*/
int CALLBACK PropSheetProc(HWND sheet, UINT msgid, LPARAM lParam)
{
	switch (msgid)
	{
	case PSCB_PRECREATE:
		{
			DLGTEMPLATE *templ = (DLGTEMPLATE*)lParam;

			templ->cy += 5;

			//add a minimize box
			templ->style |= WS_MINIMIZEBOX;
		}
		break;

	case PSCB_INITIALIZED:
		{
			HWND tooltip;
			HICON icon;

			/* Add Menu. */
			propdata.menu = LoadMenu(aokts, (LPCSTR)IDM_MAIN);
			SetMenu(sheet, propdata.menu);
			SetSaveState(sheet, MF_GRAYED);

			/* Enable appropriate recent file items. */
			UpdateRecentMenu(propdata.menu);

			/* Remove unused buttons. */
			for (int i = 0; i < sizeof(PropSheetButtons) / sizeof(WORD); i++)
			{
				HWND hWnd = GetDlgItem(sheet, PropSheetButtons[i]);
				if (hWnd != NULL)
				{
					ShowWindow(hWnd, SW_HIDE);
					EnableWindow(hWnd, FALSE);
				}
			}

			/* Add a tooltip window */
			tooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, "AOKTS Tooltip", WS_POPUP,
				CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
				sheet, NULL, aokts, NULL);
			TooltipInit(tooltip);

			/* Set the big icon */
			icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LOGO));
			Window_SetIcon(sheet, ICON_BIG, icon);
		}
		break;
	}
	return 0;
}

/*
	MakeSheet: Creates the main property sheet window.

	Parameters:
	HINSTANCE app: Handle to the application loading the sheet.

	Note: Called once and only once by WinMain().
*/
HWND MakeSheet(HINSTANCE app)
{
	PROPSHEETHEADER header;
	HPROPSHEETPAGE pages[NUM_PAGES];
	PROPSHEETPAGE pg;	//used to create each page
	HWND sheet;

	//create pages

	pg.dwSize = sizeof(PROPSHEETPAGE);
	pg.dwFlags = PSP_DEFAULT;
	pg.hInstance = app;

	for (int i = 0; i < NUM_PAGES; i++)
	{
		pg.pszTemplate = MAKEINTRESOURCE(IDD_MSGS + i);	//template IDs are in display order
		pg.pfnDlgProc = procs[i];
		pg.lParam = 0;
		pages[i] = CreatePropertySheetPage(&pg);
	}

	//create sheet

	header.dwSize = sizeof(header);
	header.dwFlags = PSH_MODELESS | PSH_USECALLBACK |
		PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP | PSH_USEICONID;
	header.hwndParent = NULL;
	header.hInstance = app;
	header.pszIcon = MAKEINTRESOURCE(IDI_LOGO);
	header.pszCaption = szTitle;
	header.nPages = NUM_PAGES;
	header.nStartPage = 0;
	header.phpage = pages;

	header.pfnCallback = &PropSheetProc;

	sheet = (HWND)PropertySheet(&header);

	//add status bar here (can't be done in PropertySheetProc)
	propdata.statusbar = CreateWindow(STATUSCLASSNAME, welcome,
		WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
		sheet, (HMENU)IDS_MAIN, aokts, NULL);

	return sheet;
}

/*
	DefaultDialogProc: A simple DialogProc currently used for the About box.

	Note: See DialogProc in the Platform SDK docs for parameters and notes.
*/
INT_PTR CALLBACK DefaultDialogProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		if (wParam == IDOK)
		{
			EndDialog(dialog, TRUE);
			return 0;
		}
		if (wParam == IDCANCEL)
		{
			EndDialog(dialog, FALSE);
			return 0;
		}
	}
	return 0;
}

/*
	DisplayStats: Fills out controls for statistics dialog.
*/
BOOL DisplayStats(HWND dialog)
{
	// TODO: split this into model/view

	UINT total = 0, i;
	UINT ne = 0, nc = 0;
	Trigger *t_parse;
	Player *p_parse = scen.players;

	/* total enabled players */
	SetDlgItemInt(dialog, IDC_S_PLAYERS, scen.getPlayerCount(), FALSE);

	/* Units (including buildings, GAIA stuff, etc.) */
	total = 0;
	for (i = 0; i < NUM_PLAYERS; i++, p_parse++)
	{
		int count = p_parse->units.size();
		if (i <= GAIA_INDEX)
			SetDlgItemInt(dialog, IDC_S_UNITS1 + i, count, FALSE);
		total += count;

		SetDlgItemInt(dialog, IDC_S_DISABLE1 + i, p_parse->ndis_u + p_parse->ndis_b, FALSE);
	}
	SetDlgItemInt(dialog, IDC_S_UNITS, total, FALSE);

	/* total triggers */
	SetDlgItemInt(dialog, IDC_S_TRIGGERS, scen.triggers.count(), FALSE);

	/* total effects & conditions */
	total = scen.triggers.count();
	t_parse = scen.triggers.first();
	while (total--)
	{
		ne += t_parse->effects.size();
		nc += t_parse->conds.size();
		t_parse++;
	}
	SetDlgItemInt(dialog, IDC_S_CONDITIONS, nc, FALSE);
	SetDlgItemInt(dialog, IDC_S_EFFECTS, ne, FALSE);

	/* map size (why here?) */
	SetDlgItemInt(dialog, IDC_S_MAPSIZE, scen.map.x, FALSE);

	return TRUE;
}

/*
	StatsDialogProc

	Note: See DialogProc in the Platform SDK docs for parameters and notes.
*/
INT_PTR CALLBACK StatsDialogProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return DisplayStats(dialog);

	case WM_CLOSE:
		EndDialog(dialog, FALSE);
		break;

	case WM_COMMAND:
		if (wParam == IDOK)
		{
			EndDialog(dialog, TRUE);
			return 0;
		}
	}
	return 0;
}

/* OnMenuSelect */
void OnMenuSelect(WORD id, WORD flags, HMENU handle)
{
	HINSTANCE res = GetModuleHandle(NULL);
	char buffer[0x50] = "";

	LoadString(res, id, buffer, sizeof(buffer));
	SetWindowText(propdata.statusbar, buffer);
}

/* OnCompressOrDecompress */
// TODO: fix alternate cohesion crap
void OnCompressOrDecompress(HWND sheet, bool compress)
{
	int size, ret;
	char path[_MAX_PATH];

	if (!GetOpenFileNameA(sheet, path, sizeof(path)))
		return;

	size = fsize(path);
	std::vector<unsigned char> buffer(size);

	AutoFile fIn(path, "rb");
	fread(&buffer[0], sizeof(char), size, fIn.get()); // contiguous
	fIn.close();

	path[0] = '\0';   // don't pre-fill path
	if (!GetSaveFileNameA(sheet, path, sizeof(path)))
		return;

	AutoFile fOut(path, "wb");

	if (compress)
		ret = deflate_file(&buffer[0], size, fOut.get());
	else
		ret = inflate_file(&buffer[0], size, fOut.get());

	fOut.close();

	if (ret >= 0)
		MessageBox(sheet, "Operation completed successfully.",
		"Raw Compression/Decompression", MB_OK);
	else
		MessageBox(sheet, "Operation failed.",
		"Raw Compression/Decompression", MB_ICONWARNING);
}

/*
	Sheet_HandleCommand: Handles all commands routed to property sheet (mostly menuitem stuff).
*/
bool Sheet_HandleCommand(HWND sheet, WORD code, WORD id, HWND control)
{
	bool ret = true;

	switch (id)
	{
	case ID_APP_ABOUT:
		DialogBoxParam(aokts, (LPCSTR)IDD_ABOUT, sheet, DefaultDialogProc, 0L);
		break;

	case ID_HELP:
		WinHelp(sheet, "aokts.hlp", HELP_CONTENTS, 0);
		break;

	case ID_FILE_OPEN:
		FileOpen(sheet, true, -1);
		break;

	case ID_FILE_NEW:
	case ID_FILE_CLOSE:
		FileClose(sheet, control);
		break;

	case ID_APP_EXIT:
		if (scen.needsave())
		{
			int sel = MessageBox(sheet, "Do you want to save your changes?", "Save", MB_YESNOCANCEL);
			if (sel == IDYES)
				FileSave(sheet, false, true);
			else if (sel == IDCANCEL)
				break;	//stop closing
		}
		DestroyWindow(sheet);
		break;

	case ID_FILE_SAVE:
		FileSave(sheet, false, true);
		break;

	case ID_FILE_SAVE_AS:
		FileSave(sheet, true, true);
		break;

	case ID_FILE_SAVE_AS2:
		FileSave(sheet, true, false);

	case ID_FILE_DUMP:
		if (!scen.exFile("dump", -1))
		{
			MessageBox(sheet, "Dump failed.", "Scenario Dump", MB_ICONWARNING);
		}
		break;

	case ID_FILE_TRIGWRITE:
		OnFileTrigWrite(sheet);
		break;

	case ID_FILE_TRIGREAD:
		OnFileTrigRead(sheet);
		break;

	case ID_FILE_RECENT1:
	case ID_FILE_RECENT2:
	case ID_FILE_RECENT3:
	case ID_FILE_RECENT4:
		FileOpen(sheet, false, id - ID_FILE_RECENT1);
		break;

	case IDCANCEL:
	case IDOK:
		assert(false);
		break;

	case ID_VIEW_STATISTICS:
		DialogBoxParam(aokts, MAKEINTRESOURCE(IDD_STATS), sheet, StatsDialogProc, 0);
		break;

	case ID_VIEW_STATUS_BAR:
		if (GetMenuState(GetMenu(sheet), ID_VIEW_STATUS_BAR, MF_BYCOMMAND) & MF_CHECKED)
		{
			ShowWindow(propdata.statusbar, SW_HIDE);
			CheckMenuItem(GetMenu(sheet), ID_VIEW_STATUS_BAR, MF_BYCOMMAND);
		}
		else
		{
			ShowWindow(propdata.statusbar, SW_SHOW);
			CheckMenuItem(GetMenu(sheet), ID_VIEW_STATUS_BAR, MF_BYCOMMAND | MF_CHECKED);
		}
		break;

	case ID_VIEW_MAP:
		if (GetMenuState(GetMenu(sheet), ID_VIEW_MAP, MF_BYCOMMAND) & MF_CHECKED)
		{
			// hide window
			ShowWindow(propdata.mapview, SW_HIDE);
			// clear check
			CheckMenuItem(GetMenu(sheet), ID_VIEW_MAP, MF_BYCOMMAND);
		}
		else
		{
			ShowWindow(propdata.mapview, SW_SHOW);
			CheckMenuItem(GetMenu(sheet), ID_VIEW_MAP, MF_BYCOMMAND | MF_CHECKED);
		}
		break;

	case ID_TOOLS_COMPRESS:
		OnCompressOrDecompress(sheet, true);
		break;

	case ID_TOOLS_DECOMPRESS:
		OnCompressOrDecompress(sheet, false);
		break;

	default:
		ret = false;
	}

	return ret;
}

/*
	MainDlgProc: The DialogProc for the main property sheet window.

	Note: See DialogProc in Platform SDK docs for parameters and notes.
*/
#define CALLPROC()	CallWindowProc((WNDPROC)pproc, sheet, msg, wParam, lParam)

INT_PTR CALLBACK MainDlgProc(HWND sheet, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(sheet);
		return CALLPROC();

	case WM_SYSCOMMAND:
		//the overloaded DialogProc screwes up the automatic SC_CLOSE translation to WM_CLOSE
		if (wParam == SC_CLOSE)
			DestroyWindow(sheet);
		else
			return CALLPROC();
		break;

	case WM_COMMAND:
		ret = 0;	//processing message
		if (!Sheet_HandleCommand(sheet, HIWORD(wParam), LOWORD(wParam), (HWND)lParam))
			CALLPROC();
		break;

	case WM_DESTROY:
		{
			HWND page = (HWND)SendMessage(sheet, PSM_GETCURRENTPAGEHWND, 0, 0);
			SendMessage(page, AOKTS_Closing, 0, 0);
			WinHelp(sheet, "aokts.hlp", HELP_QUIT, 0);
			PostQuitMessage(0);
		}
		return CALLPROC();

	case WM_HELP:
		WinHelp(sheet, "aokts.hlp", HELP_CONTENTS, 0);
		break;

	case WM_MENUSELECT:
		OnMenuSelect(LOWORD(wParam), HIWORD(wParam), (HMENU)lParam);
		break;

	case MAP_Close:
		CheckMenuItem(GetMenu(sheet), ID_VIEW_MAP, MF_BYCOMMAND | MF_UNCHECKED);
		propdata.mapview = NULL;
		break;

	case MAP_Click:
		SendMessage(
			PropSheet_GetCurrentPageHwnd(sheet),
			MAP_Click, wParam, lParam);
		break;

	default:
		return CALLPROC();
	}

	return ret;
}

char *getCmdlinePath(char *cmdline, char *buffer)
{
	while (*cmdline == ' ')
		cmdline++;

	if (!*cmdline)
	{
		printf("Too few arguments!\n");
		return NULL;
	}

	if (*cmdline == '\"')	//surrounding doublequotes
	{
		while (*cmdline && *cmdline != '\"' )
			*buffer++ = *cmdline++;
		if (*cmdline == '\"')
			cmdline++;
	}
	else
	{
		while (*cmdline && *cmdline != ' ')
			*buffer++ = *cmdline++;
	}

	*buffer = '\0';

	return cmdline;
}

/*
	ProcessCmdline: processes the command-line and returns whether to exit
*/
bool ProcessCmdline(char *cmdline)
{
	bool ret = true;
	char pathIn[_MAX_PATH], pathOut[_MAX_PATH];
	FILE *fileIn, *fileOut;
	int size, code;
	unsigned char *buffer;
	int c;

	switch (c = tolower(cmdline[1]))
	{
	case 'c':
	case 'u':
		cmdline += 2;
		cmdline = getCmdlinePath(cmdline, pathIn);
		if (!cmdline)
			break;
		cmdline = getCmdlinePath(cmdline, pathOut);
		if (!cmdline)
			break;

		if (c == 'c')
			printf("Compressing %s to %s...", pathIn, pathOut);
		else
			printf("Decompressing %s to %s...", pathIn, pathOut);

		size = fsize(pathIn);
		buffer = new unsigned char[size];
		if (!buffer)
		{
			printf("not enough memory.");
			break;
		}

		fileIn = fopen(pathIn, "rb");
		if (!fileIn)
		{
			printf("couldn\'t open input file");
			delete [] buffer;
			break;
		}
		fread(buffer, sizeof(char), size, fileIn);
		fclose(fileIn);

		fileOut = fopen(pathOut, "wb");
		if (!fileOut)
		{
			printf("couldn\'t open output file");
			delete [] buffer;
			break;
		}

		if (c == 'c')
			code = deflate_file(buffer, size, fileOut);
		else
			code = inflate_file(buffer, size, fileOut);

		fclose(fileOut);

		if (code >= 0)
			printf("done!");
		else
			printf("failed.");

		break;

	default:
		ret = false;
	}

	return ret;
}

#define MAP_OFFSET 10
HWND MakeMapView(HWND sheet, int cmdshow)
{
	HWND ret;
	RECT rect;

	GetWindowRect(sheet, &rect);
	ret = CreateMapView(sheet, rect.right + MAP_OFFSET, rect.top);
	ShowWindow(ret, cmdshow);

	return ret;
}

/*
	WinMain: The entry-point function.

	Note: See WinMain in Platform SDK docs for parameters and notes.
*/

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPTSTR cmdline, int cmdshow)
{
	MSG msg;
	BOOL ret;
	HWND sheet, active;
	HACCEL accelerators;

#ifdef MSVC_MEMLEAK_CHECK
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);	//check for memory leaks
#endif

	//basic initializations
	aokts = inst;
	propdata.p = scen.players;	//start pointing to first member
	ret = setts.load();
	if (*setts.logname)
		freopen(setts.logname, "w", stdout);
	printf("Opened log file.\n");

	//process any compress/decompress requests
	if ((*cmdline == '/' || *cmdline == '-') && ProcessCmdline(cmdline))
			return 0;

	//read genie data
	try
	{
		esdata.load(datapath);
	}
	catch (std::exception& ex)
	{
		printf("Could not load data: %s\n", ex.what());
		MessageBox(NULL,
			"Could not read Genie Data from data.xml. Terminating...",
			"Error", MB_ICONERROR);
		return 0;
	}

	//create the property sheet & init misc data
	InitCommonControls();
	sheet = MakeSheet(inst);
	propdata.tformat = RegisterClipboardFormat("AOKTS Trigger");
	propdata.ecformat = RegisterClipboardFormat("AOKTS EC");
	propdata.mcformat = RegisterClipboardFormat("AOKTS Mapcopy");
	accelerators = LoadAccelerators(inst, (LPCTSTR)IDA_MAIN);	//checked for err later
	
	//give the sheet its own DialogProc
	pproc = (DLGPROC)SetWindowLong(sheet, DWL_DLGPROC, (LONG)&MainDlgProc);

	//check for errors down here, after we create the sheet
	if (!accelerators)
	{
		MessageBox(sheet,
			"Keyboard Accelerators failed to load. Keyboard shortcuts will not be available.",
			"Warning", MB_ICONWARNING);
	}
	if (!propdata.tformat | !propdata.ecformat)
	{
		MessageBox(sheet,
			"Could not register clipboard format. Clipboard operations will not function.",
			"Warning", MB_ICONWARNING);
	}
	if (!ret)
		MessageBox(sheet, warnNoAOEII, "Warning", MB_ICONWARNING);

	//open mapview window
	propdata.mapview = MakeMapView(sheet, cmdshow);

	//check for, then open the scenario specified in command string
	if (*cmdline != '\0')
	{
		if (*cmdline == '"')
		{
			cmdline++;	//increment past first doublequote
			*strrchr(cmdline, '"') = '\0';	//find last " and replace it
		}

		strcpy(setts.ScenPath, cmdline);
		FileOpen(sheet, false, -1);
	}

	//the message loop
	while (ret = GetMessage(&msg, NULL, 0, 0))
	{
		if (ret < 0)	//did GetMessage() fail?
		{
			MessageBox(sheet,
				"Unable to retrieve messages from queue. Click OK to terminate.",
				"AOKTS Fatal Error", MB_ICONERROR);
			break;
		}

		// Give first dibs to keyboard accelerators and the propsheet.
		if (TranslateAccelerator(sheet, accelerators, &msg) ||
			PropSheet_IsDialogMessage(sheet, &msg))
			continue;

		// Usually active is the sheet. If it's not, it's a modeless dialog and
		// it should get a crack, too.
		if ((active = GetActiveWindow()) != sheet &&
			IsDialogMessage(active, &msg))
			continue;

		// If we get here, it's just a normal message, so Translate and
		// Dispatch.
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//cleanup
	if (setts.DelTempOnExit)
		DeleteFile(setts.TempPath);

	fclose(stdout);

	return msg.wParam;
}
