/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	disablesedit.cpp -- Defines functions for Disables editor.

	VIEW/CONTROLLER
**/

#include "editors.h"
#include "../resource.h"
#include "LinkListBox.h"
#include "utilunit.h"
#include "utilui.h"

/* Disabled */

#define MAX_DISABLED 30
#define NUM_TYPES 3

enum DTypes
{ DIS_bldg, DIS_unit, DIS_tech };

const char *dtypes[NUM_TYPES] = { "Buildings", "Units", "Techs" };

const char d_title[] = "Disables Editor";

/*
	Disables_HandleAdd: Moves a string from left box to right box.

	data: optionally specifies string by data. otherwise, current sel.
*/
void Disables_HandleAdd(HWND dialog, int data)
{
	int index;
	HWND list_all;
	const Link *l = NULL, *list;

	list_all = GetDlgItem(dialog, IDC_D_ALL);
	if (propdata.sel0 == DIS_tech)
		list = esdata.techs;
	else
		list = esdata.units;

	/* Get item index */
	if (data >= 0)
	{
		int count;

		l = getById(list, data);

		count = SendMessage(list_all, LB_GETCOUNT, 0, 0);
		for (index = 0; index < count; index++)
		{
			if (LinkListBox_Get(list_all, index) == l)
				break;
		}
	}
	else
	{
		index = SendMessage(list_all, LB_GETCURSEL, 0, 0);
		if (index >= 0)
			l = LinkListBox_Get(list_all, index);
	}

	/* Delete */
	SendMessage(list_all, LB_DELETESTRING, index, 0);

	/* Add copy item to selection list */
	if (l)
		LinkListBox_Add(GetDlgItem(dialog, IDC_D_SEL), l);
	else
		MessageBox(dialog, "Could not locate selection.", "Disables editor", MB_ICONWARNING);
}

void Disables_HandleDel(HWND dialog)
{
	WPARAM index;
	const Link * data;
	HWND list_sel;

	list_sel = GetDlgItem(dialog, IDC_D_SEL);

	index = SendMessage(list_sel, LB_GETCURSEL, 0, 0);

	if (index == LB_ERR)
		return; // TODO: error recovery?

	// Retrieve and remove from "Selected" list
	data = LinkListBox_Get(list_sel, index);
	SendMessage(list_sel, LB_DELETESTRING, index, 0);

	// Add link back to "All" list
	LinkListBox_Add(GetDlgItem(dialog, IDC_D_ALL), data);
}

void Disables_HandleClear(HWND dialog)
{
	HWND list_sel = GetDlgItem(dialog, IDC_D_SEL);
	HWND list_all = GetDlgItem(dialog, IDC_D_ALL);

	for (LRESULT i = SendMessage(list_sel, LB_GETCOUNT, 0, 0); i != 0; i--)
	{
		// Retrieve and remove from "Selected" list
		const Link * data = LinkListBox_Get(list_sel, 0);
		SendMessage(list_sel, LB_DELETESTRING, 0, 0);

		// Add link back to "All" list
		LinkListBox_Add(list_all, data);
	}
}

void LoadDisables(HWND dialog)
{
	HWND list_sel, list_all;
	int i;
	long *d_parse;

	list_sel = GetDlgItem(dialog, IDC_D_SEL);
	list_all = GetDlgItem(dialog, IDC_D_ALL);

	SendMessage(list_sel, LB_RESETCONTENT, 0, 0);
	SendMessage(list_all, LB_RESETCONTENT, 0, 0);

	switch (propdata.sel0)
	{
	case DIS_bldg:

		if (esdata.ug_buildings)
			UnitList_FillGroup(list_all, esdata.ug_buildings);
		else
			MessageBox(dialog, "Could not load building list.", d_title, MB_ICONWARNING);

		d_parse = propdata.p->dis_bldg;
		for (i = 0; i < propdata.p->ndis_b; i++)
			Disables_HandleAdd(dialog, *d_parse++);

		break;

	case DIS_tech:

		LinkListBox_Fill(list_all, esdata.techs);

		d_parse = propdata.p->dis_tech;
		for (i = 0; i < propdata.p->ndis_t; i++)
		{
			if (*d_parse > 0)
				Disables_HandleAdd(dialog, *d_parse++);
		}

		break;

	case DIS_unit:

		if (esdata.ug_units)
			UnitList_FillGroup(list_all, esdata.ug_units);
		else
			MessageBox(dialog, "Could not load unit list.", d_title, MB_ICONWARNING);

		d_parse = propdata.p->dis_unit;
		for (i = 0; i < propdata.p->ndis_u; i++)
			Disables_HandleAdd(dialog, *d_parse++);
		break;
	}
}

void SaveDisables(HWND dialog)
{
	int count;
	HWND list_sel;
	long *array;

	list_sel = GetDlgItem(dialog, IDC_D_SEL);
	count = SendMessage(list_sel, LB_GETCOUNT, 0, 0);

	/* count must be limited to MAX_DISABLED */
	if (count > MAX_DISABLED)
		count = MAX_DISABLED;	//TODO: buildings?

	switch (propdata.sel0)
	{
	case DIS_bldg:
		propdata.p->ndis_b = count;
		array = propdata.p->dis_bldg;
		break;
		
	case DIS_tech:
		propdata.p->ndis_t = count;
		array = propdata.p->dis_tech;
		break;
		
	case DIS_unit:
		propdata.p->ndis_u = count;
		array = propdata.p->dis_unit;
		break;

	default:
		return; // at least don't crash below
	}

	for (int i = 0; i < count; i++)
	{
		const Link *t = LinkListBox_Get(list_sel, i);
		if (t) // TODO: error handling?
			*array++ = t->id();
	}
}

void Disables_HandleCommand(HWND dialog, WORD code, WORD id, HWND control)
{
	switch (code)
	{
	case BN_CLICKED:
	case CBN_SELCHANGE:
		switch (id)
		{
		case IDC_D_SPLY:
			SaveDisables(dialog);
			propdata.pindex = SendMessage(control, CB_GETCURSEL, 0, 0);
			propdata.p = &scen.players[propdata.pindex];
			LoadDisables(dialog);
			break;

		case IDC_D_STYPE:
			SaveDisables(dialog);
			propdata.sel0 = SendMessage(control, CB_GETCURSEL, 0, 0);
			LoadDisables(dialog);
			break;

		case IDC_D_ADD:
			Disables_HandleAdd(dialog, -1);
			break;

		case IDC_D_DEL:
			Disables_HandleDel(dialog);
			break;

		case IDC_D_CLR:
			Disables_HandleClear(dialog);
			break;

		case ID_EDIT_COPY:
			SendMessage(GetFocus(), WM_COPY, 0, 0);
			break;

		case ID_EDIT_CUT:
			SendMessage(GetFocus(), WM_CUT, 0, 0);
			break;

		case ID_EDIT_PASTE:
			SendMessage(GetFocus(), WM_PASTE, 0, 0);
			break;
		}
		break;

	case EN_SETFOCUS:
		EnableMenuItem(propdata.menu, ID_EDIT_COPY, MF_ENABLED);
		EnableMenuItem(propdata.menu, ID_EDIT_CUT, MF_ENABLED);
		if (IsClipboardFormatAvailable(CF_TEXT))
			EnableMenuItem(propdata.menu, ID_EDIT_PASTE, MF_ENABLED);
		break;

	case EN_KILLFOCUS:
		EnableMenuItem(propdata.menu, ID_EDIT_COPY, MF_GRAYED);
		EnableMenuItem(propdata.menu, ID_EDIT_CUT, MF_GRAYED);
		EnableMenuItem(propdata.menu, ID_EDIT_PASTE, MF_GRAYED);
		break;
	}
}

/**
 * Resets the dialog when new data is available, such as when activating the
 * dialog.
 */
static void reset(HWND dialog)
{
	propdata.sel0 = 0;
	LoadDisables(dialog);
	SendDlgItemMessage(dialog, IDC_D_SPLY, CB_SETCURSEL, propdata.pindex, 0);
	SendDlgItemMessage(dialog, IDC_D_STYPE, CB_SETCURSEL, 0, 0);
}

INT_PTR CALLBACK DisDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			Combo_Fill(dialog, IDC_D_SPLY, Player::names, NUM_PLAYERS - 1);
			Combo_Fill(dialog, IDC_D_STYPE, dtypes, NUM_TYPES);
			return TRUE;

		case WM_COMMAND:
			Disables_HandleCommand(
				dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
			break;

		case WM_NOTIFY:
			{
				NMHDR *header = (NMHDR*)lParam;
				switch (header->code)
				{
				case PSN_SETACTIVE:
					reset(dialog);
					break;
				case PSN_KILLACTIVE:
					SaveDisables(dialog);
					break;
				}
			}
			break;

		case AOKTS_Loading:
			reset(dialog);
			return 0;

		case AOKTS_Saving:
			SaveDisables(dialog);
		}
	}
	catch (std::exception& ex)
	{
		// Show a user-friendly message, bug still crash to allow getting all
		// the debugging info.
		unhandledExceptionAlert(dialog, msg, ex);
		throw;
	}

	return 0;
}
