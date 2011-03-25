/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	unitedit.cpp -- Functions for the unit editor.

	VIEW/CONTROLLER
**/

#include "editors.h"

#include "../util/settings.h"

#include <stdio.h>
#include "../resource.h"
#include "utilui.h"
#include "LCombo.h"
#include "LinkListBox.h"
#include "utilunit.h"
#include "mapview.h"

using std::vector;

/**
 * Offset of currently-selected unit in vector.
 */
size_t u_index = SIZE_MAX; // SIZE_MAX is special "no selection" value

//LB index of currently selected constant
unsigned int c_index = UINT_MAX; // UINT_MAX is "no selection" value

#define NUM_LISTS	5
#define NUM_ROTATES 8
const char *rotates[NUM_ROTATES] = { "0/4", "1/4", "2/4", "3/4", "4/4", "5/4", "6/4", "7/4" };

const char szTitle[] =
"Unit Editor";
const char errorUnfoundUnit[] =
"Could not find unit to load. I most likely incorrectly deleted a unit.";
const char errorUnfoundType[] =
"You have some strange units on this map, my friend. Unpredictable results may occur.";
const char warningNoSelDelete[] =
"Unknown unit selected! Deleting aborted.";

inline int truncate(float x)
{
	return (int)x;
}

bool Units_Load(HWND dialog)
{
	bool ret = true;
	Unit *u;
	Unit blank;	//for dummy purposes
	HWND unitbox = GetDlgItem(dialog, IDC_U_UNIT);
	const UnitLink *type;

	if (u_index == SIZE_MAX)
	{
		u = &blank;
		// Use first unit type
		u->setType(esdata.units.head());
	}

	else if (u_index < propdata.p->units.size())
	{
		u = &propdata.p->units[u_index];
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			static_cast<int>(u->x), static_cast<int>(u->y));
	}

	else
	{
		u = &blank;
		ret = false;
		MessageBox(dialog, errorUnfoundUnit, szTitle, MB_ICONERROR);
	}

	if (!(type = u->getType()))
	{
		MessageBox(dialog, errorUnfoundType, szTitle, MB_ICONERROR);
		return false;
	}
	SetDlgItemInt(dialog, IDC_U_CONST, type->id(), FALSE);
	c_index = ListBox_Find(unitbox, type);

	//if unit is not in current list, change list to "All"
	if (c_index == UINT_MAX)
	{
		SendDlgItemMessage(dialog, IDC_U_TYPE, CB_SETCURSEL, 0, 0);
		UnitList_FillGroup(unitbox, NULL);

		//now search for unit on the "All" list. (If this fails, too bad.)
		if ((c_index = ListBox_Find(unitbox, type)) == UINT_MAX)
			MessageBox(dialog, errorUnfoundType, szTitle, MB_ICONWARNING);
	}

	SendDlgItemMessage(dialog, IDC_U_UNIT, LB_SETCURSEL, c_index, 0);

	SetDlgItemFloat(dialog, IDC_U_X, u->x);
	SetDlgItemFloat(dialog, IDC_U_Y, u->y);
	SendDlgItemMessage(dialog, IDC_U_ROTATE, CB_SETCURSEL, (int)(u->rotate / PI * 4), 0);
	SetDlgItemInt(dialog, IDC_U_FRAME, u->frame, FALSE);
	SetDlgItemInt(dialog, IDC_U_GARRISON, u->garrison, TRUE);

	return ret;
}

void Units_Save(HWND dialog)
{
	if (u_index != SIZE_MAX)
	{
		Unit& u = propdata.p->units[u_index];
		// Get the selected UnitLink * from constant selection box.
		const Link * ucnst_sel =
			LinkListBox_GetSel(GetDlgItem(dialog, IDC_U_UNIT));

		u.x =		GetDlgItemFloat(dialog, IDC_U_X);
		u.y =		GetDlgItemFloat(dialog, IDC_U_Y);
		if (ucnst_sel) // will be NULL if user changed group
			u.setType(static_cast<const UnitLink *>(ucnst_sel));
		u.rotate =	(float)SendDlgItemMessage(dialog, IDC_U_ROTATE, CB_GETCURSEL, 0, 0) / 4 * (float)PI;
		u.frame = (short)GetDlgItemInt(dialog, IDC_U_FRAME, NULL, FALSE);
		u.garrison = GetDlgItemInt(dialog, IDC_U_GARRISON, NULL, TRUE);
	}
}

/*
	UnitList_ChangeType: Handles change the type constant of a unit.
*/
void UnitList_ChangeType(HWND dialog, HWND typebox)
{
	HWND selbox, ttext;
	
	selbox = GetDlgItem(dialog, IDC_U_SELU);
	ttext  = GetDlgItem(dialog, IDC_U_CONST);
	enum Sorts sort =
		(enum Sorts)SendDlgItemMessage(dialog, IDC_U_SORT, CB_GETCURSEL, 0, 0);

	unsigned int type_index = SendMessage(typebox, LB_GETCURSEL, 0, 0);

	// If it's a new constant, update selected unit.
	if (type_index != c_index && u_index != SIZE_MAX)
	{
		vector<Unit>& units = propdata.p->units; // for convenience

		// Get index of selected unit in listbox
		unsigned int selected_index = SendMessage(selbox, LB_GETCURSEL, 0, 0);

		Unit& u = units[u_index];

		Units_Save(dialog);

		//then "update" the string in the selection box
		SendMessage(selbox, LB_DELETESTRING, selected_index, 0);
		selected_index = UnitList_InsertItem(selbox, sort, units, &u);
		SendMessage(selbox, LB_SETCURSEL, selected_index, 0);
	}

	c_index = type_index;	//update this even if no unit selected
	UnitLink *type = (UnitLink*)SendMessage(typebox, LB_GETITEMDATA, type_index, 0);
	if (type)
		SetWindowText(ttext, type->id());
	else
		SetWindowTextW(ttext, L"ERR");
}

void Units_Reset(HWND dialog)
{
	enum Sorts sort =
		(enum Sorts)SendDlgItemMessage(dialog, IDC_U_SORT, CB_GETCURSEL, 0, 0);
	UnitList_Fill(
		GetDlgItem(dialog, IDC_U_SELU),
		sort,
		propdata.p->units);
	u_index = SIZE_MAX;
	ENABLE_WND(IDC_U_DEL, false);
	Units_Load(dialog);
}

void Units_HandleDelete(HWND dialog)
{
	unsigned index, count, data;
	HWND selbox = GetDlgItem(dialog, IDC_U_SELU);

	if (u_index == SIZE_MAX)
	{
		MessageBox(dialog, warningNoSelDelete, szTitle, MB_ICONWARNING);
		return;
	}

	propdata.p->erase_unit(u_index);

	index = SendMessage(selbox, LB_GETCURSEL, 0, 0);
	SendMessage(selbox, LB_DELETESTRING, index, 0);

	/* Update the other items' data. */
	count = SendMessage(selbox, LB_GETCOUNT, 0, 0);
	for (unsigned i = 0; i < count; i++)
	{
		data = SendMessage(selbox, LB_GETITEMDATA, i, 0);
		if (data > u_index)
			SendMessage(selbox, LB_SETITEMDATA, i, data - 1);
	}

	/* Set selection to next item down. If there isn't one, reset stuff. */
	if (SendMessage(selbox, LB_GETCOUNT, 0, 0))
	{
		SendMessage(selbox, LB_SETCURSEL, index, 0);
		u_index = SendMessage(selbox, LB_GETITEMDATA, index, 0);
		Units_Load(dialog);
	}
	else
	{
		ENABLE_WND(IDC_U_DEL, false);
		u_index = SIZE_MAX;
	}
}

void Units_HandleSelChange(HWND dialog, HWND listbox)
{
	int index;

	Units_Save(dialog);
	SendMessage(propdata.mapview, MAP_UnhighlightPoint,
		MAP_UNHIGHLIGHT_ALL, 0);
	index = SendMessage(listbox, LB_GETCURSEL, 0, 0);
	u_index = SendMessage(listbox, LB_GETITEMDATA, index, 0);

	if (u_index >= 0)
	{
		EnableWindow(GetDlgItem(dialog, IDC_U_DEL), TRUE);
		EnableMenuItem(propdata.menu, ID_EDIT_DELETE, MF_ENABLED);
	}

	Units_Load(dialog);
}

void Units_HandleTypeChange(HWND dialog, HWND typelist)
{
	HWND unitlist = GetDlgItem(dialog, IDC_U_UNIT);

	// See Units_HandleInit for LCombo_Fill() with UnitGroupLink*.
	UnitList_FillGroup(unitlist,
		static_cast<const UnitGroupLink *>(LinkComboBox_GetSelPtr(typelist)));
}

void Units_HandleAdd(HWND dialog)
{
	Unit u(scen.next_uid++);
	HWND listbox = GetDlgItem(dialog, IDC_U_SELU);

	u.setType(static_cast<const UnitLink *>(
		LinkListBox_Get(GetDlgItem(dialog, IDC_U_UNIT), c_index)));
	propdata.p->units.push_back(u);
	int index = UnitList_Append(listbox, &u, propdata.p->units.size() - 1);

	// Select new item.
	SendMessage(listbox, LB_SETCURSEL, index, 0);
	ENABLE_WND(IDC_U_DEL, true);
}

void Units_HandleCommand(HWND dialog, WORD code, WORD id, HWND control)
{
	switch (code)
	{
	case BN_CLICKED:	//and menuitem
	case CBN_SELCHANGE:	//and accelerator, LBN_SELCHANGE
		switch (id)
		{
		case IDC_U_SELP:	//CBN_SELCHANGE
			Units_Save(dialog);
			propdata.pindex = SendMessage(control, CB_GETCURSEL, 0, 0);
			propdata.p = scen.players + propdata.pindex;
			Units_Reset(dialog);
			break;

		case IDC_U_SELU:	//LBN_SELCHANGE
			Units_HandleSelChange(dialog, control);
			break;

		case IDC_U_TYPE:	//CBN_SELCHANGE
			Units_HandleTypeChange(dialog, control);
			break;

		case IDC_U_ADD:		//BN_CLICKED
			Units_HandleAdd(dialog);
			break;

		case IDC_U_DEL:			//BN_CLICKED
		case ID_EDIT_DELETE:	//accelerator
			Units_HandleDelete(dialog);
			break;

		case IDC_U_UNIT:		//LBN_SELCHANGE
			UnitList_ChangeType(dialog, control);
			break;

		case IDC_U_SORT:
			Units_Reset(dialog);
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

BOOL Units_HandleInit(HWND dialog)
{
	Combo_Fill(dialog, IDC_U_SELP, Player::names, NUM_PLAYERS);
	Combo_Fill(dialog, IDC_U_ROTATE, rotates, NUM_ROTATES);
	Combo_Fill(dialog, IDC_U_SORT, sorts, NUM_SORTS);
	SendDlgItemMessage(dialog, IDC_U_SORT, CB_SETCURSEL, 0, 0);

	LCombo_Fill(dialog, IDC_U_TYPE, esdata.unitgroups, L"All");
	UnitList_FillGroup(GetDlgItem(dialog, IDC_U_UNIT), NULL);

	//set edit control limits
	SendDlgItemMessage(dialog, IDC_U_X, EM_SETLIMITTEXT, 5, 0);
	SendDlgItemMessage(dialog, IDC_U_Y, EM_SETLIMITTEXT, 5, 0);

	return TRUE;
}

void Units_HandleMapClick(HWND dialog, int x, int y)
{
	if (u_index != SIZE_MAX)
	{
		Unit &u = propdata.p->units[u_index];
		int ret = MessageBox(dialog,
			"Do you want to move the selected unit?",
			"Unit Editor", MB_YESNO);
		if (ret = IDYES)
		{
			HWND mv = propdata.mapview;
			SendMessage(mv, MAP_UnhighlightPoint,
				truncate(u.x), truncate(u.y));
			// TODO: always add 0.5?
			SetDlgItemFloat(dialog, IDC_U_X, x + 0.5);
			SetDlgItemFloat(dialog, IDC_U_Y, y + 0.5);
			Units_Save(dialog);
			SendMessage(mv, MAP_HighlightPoint, x, y);
			SendMessage(mv, MAP_Reset, 0, 0);
		}
	}
}

INT_PTR CALLBACK UnitDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			ret = Units_HandleInit(dialog);
			break;

		case WM_COMMAND:
			ret = 0;
			Units_HandleCommand(dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
			break;

		case WM_NOTIFY:
			{
				NMHDR *header = (NMHDR*)lParam;
				switch (header->code)
				{
				case PSN_SETACTIVE:
					SendDlgItemMessage(
						dialog, IDC_U_SELP, CB_SETCURSEL, propdata.pindex, 0);
					Units_Reset(dialog);
					break;

				case PSN_KILLACTIVE:
					Units_Save(dialog);
					SendMessage(
						propdata.mapview, MAP_UnhighlightPoint, MAP_UNHIGHLIGHT_ALL, 0);
				}
			}
			break;

		case WM_VKEYTOITEM:
			if (LOWORD(wParam) == VK_DELETE)
				Units_HandleDelete(dialog);
			ret = -1;
			break;

		case AOKTS_Loading:
			Units_Reset(dialog);
			break;

		case AOKTS_Saving:
			Units_Save(dialog);
			break;

		case MAP_Click:
			Units_HandleMapClick(dialog, LOWORD(lParam), HIWORD(lParam));
			break;
		}
	}
	catch (std::exception& ex)
	{
		// Show a user-friendly message, bug still crash to allow getting all
		// the debugging info.
		unhandledExceptionAlert(dialog, msg, ex);
		throw;
	}

	return ret;
}
