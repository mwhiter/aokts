/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	conditionedit.cpp -- Editors for conditions.

	VIEW/CONTROLLER
**/

#include "../util/settings.h"
#include "../res/resource.h"
#include "ecedit.h"
#include "utilui.h"
#include "LCombo.h"
#include "utilunit.h"
#include "mapview.h"
#include "../util/winugly.h"
#include <stdio.h>

/* Editor classes */

void EditCondition::update(Trigger *t)
{
	t->conds[index] = c;
}

// AoC v1.0c
const char ctable1_10C[NUM_CONDS][COND_CONTROLS] = // Using 0 instead of -1 to waste less space
{	//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// None
	{ 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 },	// Bring Object to Area
	{ 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Bring Object to Object
	{ 1, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0 },	// Own Objects
	{ 1, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0 },	// Own Fewer Objects
	{ 1, 0, 0, 0, 2, 1, 0, 0, 0, 1, 1, 1, 1, 2, 2, 0 },	// Objects in Area
	{ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Destroy Object
	{ 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Capture Object
	{ 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Accumulate Attribute
	{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Research Technology
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },	// Timer
	{ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Object Selected
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },	// AI Signal
	{ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Player Defeated
	{ 0, 0, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0 },	// Object Has Target
	{ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Object Visible
	{ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Object Not Visible
	{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Researching Tech
	{ 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// Units Garrisoned
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Difficulty Level
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // OwnFewerFoundations    (SWGB only)
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // SelectedObjectsInArea  (SWGB only)
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // PoweredObjectsInArea   (SWGB only)
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  // UnitsQueuedPastPopCap  (works in AOK)
};

// ../res/resource.h
// go to IDC_C_START

// AoC v1.4RC
const char ctable1_14RC[NUM_CONDS][COND_CONTROLS] = // Using 0 instead of -1 to waste less space
{	//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// None
	{ 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0 },	// Bring Object to Area
	{ 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Bring Object to Object
	{ 1, 0, 0, 0, 2, 1, 0, 0, 1, 0, 0, 0, 0, 2, 2, 0 },	// Own Objects
	{ 1, 0, 0, 0, 2, 1, 0, 0, 1, 0, 0, 0, 0, 2, 2, 0 },	// Own Fewer Objects
	{ 1, 0, 0, 0, 2, 1, 0, 0, 1, 1, 1, 1, 1, 2, 2, 0 },	// Objects in Area
	{ 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Destroy Object
	{ 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Capture Object
	{ 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Accumulate Attribute
	{ 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Research Technology
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },	// Timer
	{ 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Object Selected
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 },	// AI Signal
	{ 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Player Defeated
	{ 0, 0, 1, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 2, 2, 0 },	// Object Has Target
	{ 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Object Visible
	{ 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Object Not Visible
	{ 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Researching Tech
	{ 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },	// Units Garrisoned
	{ 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // Difficulty Level
	{ 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // OwnFewerFoundations    (SWGB only)
	{ 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // SelectedObjectsInArea  (SWGB only)
	{ 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // PoweredObjectsInArea   (SWGB only)
	{ 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }  // UnitsQueuedPastPopCap  (works in AOK)
};

void ConditionControls(HWND dialog, int type)
{
	int i;

	if (type >= NUM_CONDS)
	{
		for (i = IDC_C_START; i <= IDC_C_END; i++)
			ENABLE_WND(i, true);
		ENABLE_WND(IDC_C_USEL1, true);
		ENABLE_WND(IDC_C_USEL2, true);
		return;
	}

	for (i = IDC_C_START; i <= IDC_C_END; i++)
		ENABLE_WND(i, false || setts.editall);
	ENABLE_WND(IDC_C_USEL1, false);
	ENABLE_WND(IDC_C_USEL2, false);

	const char *table = ctable1_14RC[type];

	for (i = 0; i < COND_CONTROLS; i++)
	{
		if (table[i])
		{
			ENABLE_WND(IDC_C_START + i, true);
			if (i == 2)
				ENABLE_WND(IDC_C_USEL1, true);
			if (i == 3)
				ENABLE_WND(IDC_C_USEL2, true);
		}
	}
}

const wchar_t *noselectc = L"<none>";

void C_Init(HWND dialog)
{
	Combo_Fill(dialog, IDC_C_TYPE, Condition::types, NUM_CONDS);
	Combo_Fill(dialog, IDC_C_PLAYER, players_ec, EC_NUM_PLAYERS);
	LCombo_Fill(dialog, IDC_C_RESEARCH, esdata.techs.head());
	LCombo_Fill(dialog, IDC_C_RESTYPE, esdata.resources.head());
	Combo_PairFill(GetDlgItem(dialog, IDC_C_GROUP), NUM_GROUPS, groups);
	Combo_PairFill(GetDlgItem(dialog, IDC_C_UTYPE), NUM_UTYPES, utypes);
	LCombo_Fill(dialog, IDC_C_UCNST, esdata.units.head(), noselectc);
}

void LoadCond(HWND dialog, EditCondition *data)
{
	Condition *c = &data->c;

	SendDlgItemMessage(dialog, IDC_C_TYPE, CB_SETCURSEL, c->type, 0);
	SendDlgItemMessage(dialog, IDC_C_PLAYER, CB_SETCURSEL, c->player, 0);
	SetDlgItemInt(dialog, IDC_C_UIDOBJ, c->object, TRUE);
	SetDlgItemInt(dialog, IDC_C_UIDLOC, c->u_loc, TRUE);
	LCombo_Select(dialog, IDC_C_UCNST, c->pUnit);
	Combo_SelectByData(GetDlgItem(dialog, IDC_C_GROUP), c->group);
	Combo_SelectByData(GetDlgItem(dialog, IDC_C_UTYPE), c->utype);
	SetDlgItemInt(dialog, IDC_C_AREAX1, c->area.left, TRUE);
	SetDlgItemInt(dialog, IDC_C_AREAY1, c->area.bottom, TRUE);
	SetDlgItemInt(dialog, IDC_C_AREAX2, c->area.right, TRUE);
	SetDlgItemInt(dialog, IDC_C_AREAY2, c->area.top, TRUE);
	SetDlgItemInt(dialog, IDC_C_TIMER, c->timer, TRUE);
	SetDlgItemInt(dialog, IDC_C_AISIG, c->ai_signal, TRUE);
	LCombo_Select(dialog, IDC_C_RESEARCH, c->pTech);
	SetDlgItemInt(dialog, IDC_C_RESERVED, c->reserved, TRUE);
	SetDlgItemInt(dialog, IDC_C_AMOUNT, c->amount, TRUE);
	LCombo_SelById(dialog, IDC_C_RESTYPE, c->res_type);
	if (c->reserved == -1) {
	    SendMessage(GetDlgItem(dialog, IDC_C_REVERSE), WM_SETTEXT, 0, (LPARAM) _T("Reverse Condition"));
	} else if (c->reserved == -256) {
	    SendMessage(GetDlgItem(dialog, IDC_C_REVERSE), WM_SETTEXT, 0, (LPARAM) _T("Unreverse Condition"));
	} else {
	    SendMessage(GetDlgItem(dialog, IDC_C_REVERSE), WM_SETTEXT, 0, (LPARAM) _T("Reset Value"));
	}
}

void SaveCond(HWND dialog, EditCondition *data)
{
	Condition *c = &data->c;

	int newtype = SendDlgItemMessage(dialog, IDC_C_TYPE, CB_GETCURSEL, 0, 0);
	if (newtype != CB_ERR)
	{
		c->type = newtype;
		c->pUnit = (UnitLink*)LCombo_GetSelPtr(dialog, IDC_C_UCNST);
		c->player = SendDlgItemMessage(dialog, IDC_C_PLAYER, CB_GETCURSEL, 0, 0);
		c->object = GetDlgItemInt(dialog, IDC_C_UIDOBJ, NULL, TRUE);
		c->u_loc = GetDlgItemInt(dialog, IDC_C_UIDLOC, NULL, TRUE);
		c->group = SendDlgItemMessage(dialog, IDC_C_GROUP, CB_GETCURSEL, 0, 0);
		c->area.left = GetDlgItemInt(dialog, IDC_C_AREAX1, NULL, TRUE);
		c->area.bottom = GetDlgItemInt(dialog, IDC_C_AREAY1, NULL, TRUE);
		c->area.right = GetDlgItemInt(dialog, IDC_C_AREAX2, NULL, TRUE);
		c->area.top = GetDlgItemInt(dialog, IDC_C_AREAY2, NULL, TRUE);
		c->timer = GetDlgItemInt(dialog, IDC_C_TIMER, NULL, TRUE);
		c->ai_signal = GetDlgItemInt(dialog, IDC_C_AISIG, NULL, TRUE);
		c->reserved = GetDlgItemInt(dialog, IDC_C_RESERVED, NULL, TRUE);
		c->pTech = (TechLink*)LCombo_GetSelPtr(dialog, IDC_C_RESEARCH);
		c->amount = GetDlgItemInt(dialog, IDC_C_AMOUNT, NULL, TRUE);
		c->res_type = LCombo_GetSelId(dialog, IDC_C_RESTYPE);

		c->group = Combo_GetSelData(GetDlgItem(dialog, IDC_C_GROUP));
		c->utype = Combo_GetSelData(GetDlgItem(dialog, IDC_C_UTYPE));
	}
}

void C_HandleSetFocus(HWND dialog, WORD id)
{
	EditEC * data =
		static_cast<EditEC*>(GetDialogUserData_ptr(dialog));

	switch (id)
	{
	case IDC_C_AREAX1:
	case IDC_C_AREAY1:
	case IDC_C_AREAX2:
	case IDC_C_AREAY2:
		SendMessage(data->mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_C_AREAX1, NULL, TRUE),
			GetDlgItemInt(dialog, IDC_C_AREAY1, NULL, TRUE));
		SendMessage(data->mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_C_AREAX2, NULL, TRUE),
			GetDlgItemInt(dialog, IDC_C_AREAY2, NULL, TRUE));
		break;
	}
}

void C_HandleKillFocus(HWND dialog, WORD)
{
	EditEC * data =
		static_cast<EditEC*>(GetDialogUserData_ptr(dialog));

	if (data->mapview)
	{
		SendMessage(data->mapview, MAP_UnhighlightPoint,
			MAP_UNHIGHLIGHT_ALL, 0);
	}
}

void C_HandleChangeType(HWND dialog, EditCondition *data)
{
/*
	static const char *promptClearCondition =
		"Would you like to clear the condition? (Recommended)";

	if (data->c.type != CONDITION_None &&
		MessageBox(dialog, promptClearCondition, "Condition Editor", MB_YESNO) == IDYES)
	{
		// reset to default-constructor state
		data->c = Condition();
	}
*/
    // Assume the user wants to change to similar condition. Otherwise
    // would make new
	int newtype = SendDlgItemMessage(dialog, IDC_C_TYPE, CB_GETCURSEL, 0, 0);
	//if (data->c.type != newtype)
	//	data->c = Condition();
	data->c.type = newtype;
	ConditionControls(dialog, newtype);
	LoadCond(dialog, data);
}

const char warnInvalidC[] =
"Watch out, this condition appears to be invalid.";

void C_HandleCommand(HWND dialog, WORD id, WORD code, HWND)
{
	EditCondition *data = (EditCondition*)GetWindowLongPtr(dialog, DWLP_USER);

	switch (code)
	{
	case BN_CLICKED:
		switch (id)
		{
		case IDC_C_REVERSE:
	        if (GetDlgItemInt(dialog, IDC_C_RESERVED, NULL, TRUE) == -1) {
	            SetDlgItemInt(dialog, IDC_C_RESERVED, -256, TRUE);
	            SendMessage(GetDlgItem(dialog, IDC_C_REVERSE), WM_SETTEXT, 0, (LPARAM) _T("Unreverse Condition"));
	        } else {
	            SetDlgItemInt(dialog, IDC_C_RESERVED, -1, TRUE);
	            SendMessage(GetDlgItem(dialog, IDC_C_REVERSE), WM_SETTEXT, 0, (LPARAM) _T("Reverse Condition"));
	        }
	        break;
		case IDC_C_AREA_ALL:
			{
				SetDlgItemInt(dialog, IDC_C_AREAX1, -1, TRUE);
				SetDlgItemInt(dialog, IDC_C_AREAY1, -1, TRUE);
				SetDlgItemInt(dialog, IDC_C_AREAX2, -1, TRUE);
				SetDlgItemInt(dialog, IDC_C_AREAY2, -1, TRUE);
			}
			break;
		case IDC_C_CLEAR:
		    {
			    data->c = Condition();
			    LoadCond(dialog, data);
			}
			break;
	    case IDOK:
	        {
		        bool valid;
		        int ret = IDOK;

		        SaveCond(dialog, data);	//update type
		        valid = data->c.check();

		        if (!valid)
			        ret = MessageBox(dialog, warnInvalidC, "Condition Editor", MB_OKCANCEL);

		        if (ret == IDOK)
		        {
			        SendMessage(data->parent, EC_Closing,
				        MAKELONG(1, valid), reinterpret_cast<LPARAM>(data));
			        DestroyWindow(dialog);
		        }
	        }
	        break;

	    case IDCANCEL:
	        {
		        SendMessage(data->parent, EC_Closing,
			        0, reinterpret_cast<LPARAM>(data));
		        DestroyWindow(dialog);
	        }
	        break;

	    case IDC_C_USEL1:
	        {
		        if (SingleUnitSelDialogBox(dialog, data->players,
			        data->c.object, data->c.object != -1))
		        {
			        SetDlgItemInt(dialog, IDC_C_UIDOBJ, data->c.object, FALSE);
		        }
	        }
		    break;

	    case IDC_C_USEL2:
	        {
		        if (SingleUnitSelDialogBox(dialog, data->players,
			        data->c.u_loc, data->c.u_loc != -1))
		        {
			        SetDlgItemInt(dialog, IDC_C_UIDLOC, data->c.u_loc, FALSE);
		        }
	        }
		    break;

		}
		break;

    case CBN_SELCHANGE:
	    switch (id)
	    {
	    case IDC_C_TYPE:
	        {
		        C_HandleChangeType(dialog, data);
	        }
		    break;
	    }
		break;

	case EN_SETFOCUS:
		C_HandleSetFocus(dialog, id);
		break;

	case EN_KILLFOCUS:
		C_HandleKillFocus(dialog, id);
		break;

	}
}

const char errorNoDataC[] =
"No Data! Functionality will be impaired.";

INT_PTR CALLBACK CondWndProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	switch (msg)
	{

	case WM_INITDIALOG:
		{
			class EditCondition *data = (EditCondition*)lParam;

			C_Init(dialog);

			if (data)
			{
				SetWindowLongPtr(dialog, DWLP_USER, lParam);
				ConditionControls(dialog, data->c.type);
				LoadCond(dialog, data);

				ret = TRUE;
			}
			else
			{
				MessageBox(dialog, errorNoDataC, "Error", MB_ICONWARNING);
				DestroyWindow(dialog);

				ret = FALSE;
			}
		}
		break;

	case WM_COMMAND:
		ret = 0;
		C_HandleCommand(dialog, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;

	case WM_DESTROY:
		{
			class EditCondition *data = (EditCondition*)GetWindowLongPtr(dialog, DWLP_USER);
			delete data;
		}
		break;

	case EC_Update:
		{
			class EditCondition *data = (EditCondition*)GetWindowLongPtr(dialog, DWLP_USER);
			data->index += wParam;
		}
	}

	return ret;
}
