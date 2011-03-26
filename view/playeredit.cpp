/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	playeredit.cpp -- Defines functions for Players editor.

	VIEW/CONTROLLER
**/

#include "editors.h"
#include "../resource.h"
#include "utilui.h"
#include "../util/settings.h"
#include "LCombo.h"

#include <commdlg.h>

/* Players */

#if (GAME == 1)

const wchar_t * FOOD_STRING = L"Food:";
const wchar_t * WOOD_STRING = L"Wood:";
const wchar_t * GOLD_STRING = L"Gold:";
const wchar_t * STONE_STRING= L"Stone:";
const wchar_t * OREX_STRING = L"Ore X:";

#elif (GAME == 2)

const wchar_t * FOOD_STRING = L"Food:";
const wchar_t * WOOD_STRING = L"Carbon:";
const wchar_t * GOLD_STRING = L"Nova:";
const wchar_t * STONE_STRING= L"Ore:";
const wchar_t * OREX_STRING = L"Ore X:";

#endif

//these convert from enum Diplomacy to Win32 Checkbox states and vice-versa
WPARAM d_to_b[4] = { BST_CHECKED, BST_INDETERMINATE, BST_UNCHECKED, BST_UNCHECKED };
enum Diplomacy b_to_d[3] = { DIP_enemy, DIP_ally, DIP_neutral };

//These controls are disabled when user selects GAIA.
int gaia_disables[] =
{
	IDC_P_NAME, IDC_P_STABLE, IDC_P_HUMAN, IDC_P_COLOR, IDC_P_SPDIP, IDC_P_ACTIVE,
	IDC_P_DSTATE, IDC_P_AV, IDC_P_X, IDC_P_Y, IDC_P_AI, IDC_P_EXAI, IDC_P_IMAI
};

void LoadPlayer(HWND dialog)
{
	Player *p = propdata.p;
	int count = sizeof(gaia_disables) / sizeof(int);
	bool is_gaia = (propdata.pindex == GAIA_INDEX);

	for (int i = 0; i < count; i++)
		ENABLE_WND(gaia_disables[i], !is_gaia);

	if (propdata.pindex == PLAYER1_INDEX)
		ENABLE_WND(IDC_P_ACTIVE, false);

	SetDlgItemText(dialog, IDC_P_NAME, p->name);
	LCombo_SelById(dialog, IDC_P_CIV, p->civ);
	SetDlgItemInt(dialog, IDC_P_STABLE, p->stable, TRUE);
	SetDlgItemInt(dialog, IDC_P_GOLD, p->resources[0], FALSE);
	SetDlgItemInt(dialog, IDC_P_WOOD, p->resources[1], FALSE);
	SetDlgItemInt(dialog, IDC_P_FOOD, p->resources[2], FALSE);
	SetDlgItemInt(dialog, IDC_P_STONE, p->resources[3], FALSE);
	SetDlgItemInt(dialog, IDC_P_OREX, p->resources[4], FALSE);
	SendDlgItemMessage(dialog, IDC_P_ACTIVE, BM_SETCHECK, p->enable, 0);
	SendDlgItemMessage(dialog, IDC_P_HUMAN, BM_SETCHECK, p->human, 0);
	SetDlgItemInt(dialog, IDC_P_POP, (int)p->pop, FALSE);
	SetDlgItemFloat(dialog, IDC_P_X, p->camera[0]);
	SetDlgItemFloat(dialog, IDC_P_Y, p->camera[1]);
	SendDlgItemMessage(dialog, IDC_P_COLOR, CB_SETCURSEL, p->color, 0);	//assuming in order
	SendDlgItemMessage(dialog, IDC_P_AV, BM_SETCHECK, p->avictory, 0);
	SendDlgItemMessage(dialog, IDC_P_DSTATE, BM_SETCHECK, d_to_b[p->diplomacy[propdata.sel0]], 0);
	SendDlgItemMessage(dialog, IDC_P_AGE, CB_SETCURSEL, p->age, 0);
	SetDlgItemFloat(dialog, IDC_P_UF, p->ucount);
	SetDlgItemInt(dialog, IDC_P_US0, p->u1, FALSE);
	SetDlgItemInt(dialog, IDC_P_US1, p->u2, FALSE);

	/* AI is special */
	SendDlgItemMessage(dialog, IDC_P_AIMODE, BM_SETCHECK, p->aimode == AI_standard, 0);
	SetDlgItemText(dialog, IDC_P_AI, p->ai);
	EnableWindow(GetDlgItem(dialog, IDC_P_AI), p->aimode != AI_standard);
}

void SavePlayer(HWND dialog)
{
	Player *p = propdata.p;
	GetDlgItemText(dialog, IDC_P_NAME, p->name, 30);
	p->civ = LCombo_GetSelId(dialog, IDC_P_CIV);
	p->stable = GetDlgItemInt(dialog, IDC_P_STABLE, NULL, TRUE);
	p->resources[0] = GetDlgItemInt(dialog, IDC_P_GOLD, NULL, FALSE);
	p->resources[1] = GetDlgItemInt(dialog, IDC_P_WOOD, NULL, FALSE);
	p->resources[2] = GetDlgItemInt(dialog, IDC_P_FOOD, NULL, FALSE);
	p->resources[3] = GetDlgItemInt(dialog, IDC_P_STONE, NULL, FALSE);
	p->resources[4] = GetDlgItemInt(dialog, IDC_P_OREX, NULL, FALSE);
	p->enable = Button_IsChecked(GetDlgItem(dialog, IDC_P_ACTIVE));
	p->human = Button_IsChecked(GetDlgItem(dialog, IDC_P_HUMAN));
	p->pop = static_cast<float>(GetDlgItemInt(dialog, IDC_P_POP, NULL, FALSE));
	p->camera[0] = GetDlgItemFloat(dialog, IDC_P_X);
	p->camera[1] = GetDlgItemFloat(dialog, IDC_P_Y);
	p->color = LCombo_GetSelId(dialog, IDC_P_COLOR);
	p->avictory = (::SendDlgItemMessage(dialog, IDC_P_AV, BM_GETCHECK, 0, 0) != 0);
	p->diplomacy[propdata.sel0] = b_to_d[SendDlgItemMessage(dialog, IDC_P_DSTATE, BM_GETCHECK, 0, 0)];
	p->age = SendDlgItemMessage(dialog, IDC_P_AGE, CB_GETCURSEL, 0, 0);
	p->u1 = toshort(GetDlgItemInt(dialog, IDC_P_US0, NULL, FALSE));
	p->u2 = toshort(GetDlgItemInt(dialog, IDC_P_US1, NULL, FALSE));
	
	p->aimode = (SendDlgItemMessage(dialog, IDC_P_AIMODE, BM_GETCHECK, 0, 0) != 0);
	if (p->aimode != AI_standard)
	{
		GetDlgItemText(dialog, IDC_P_AI, p->ai, _MAX_FNAME);
		if (*p->ai)
			p->aimode = AI_custom;
	}
	else
		strcpy(p->ai, scen.StandardAI);
}

const char errorImpExpFail[] =
"Error. Please make sure the file you selected is valid.";

// FIXME: separate this alternate cohesion crap
void Players_ManageAI(HWND dialog, bool import)
{
	char path[_MAX_PATH], dir[_MAX_PATH];
	OPENFILENAME ofn;

	//shared init
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = dialog;
	ofn.lpstrFilter = "AI Scripts (*.per)\0*.per\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrCustomFilter = NULL;	//user should not change filter
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	strcpy(dir, setts.BasePath);
	strcat(dir, "ai");
	ofn.lpstrInitialDir = dir;

	if (import)
	{
		*path = '\0';
		ofn.lpstrFileTitle = propdata.p->ai;
		ofn.nMaxFileTitle = _MAX_FNAME;
		ofn.lpstrTitle = "Import AI Script";
		ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = "per";

		if (GetOpenFileName(&ofn))
		{
			propdata.p->ai
				[ofn.nFileExtension - ofn.nFileOffset - 1]	//offset to extension '.'
				= '\0';
			//do it
			if (propdata.p->import_ai(path))
				SetWindowText(propdata.statusbar, "AI successfully imported.");
			else
				MessageBox(dialog, errorImpExpFail, "AI Import", MB_OK);
		}
	}
	else if (propdata.p->aifile.length()) //export, so check for existence
	{
		strcpy(path, propdata.p->ai);
		ofn.lpstrFileTitle = NULL;
		ofn.lpstrTitle = "Export AI Script";
		ofn.Flags = OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = "per";

		if (GetSaveFileName(&ofn))
		{
			//do it
			if (propdata.p->export_ai(path))
				SetWindowText(propdata.statusbar, "AI successfully exported.");
			else
				MessageBox(dialog, errorImpExpFail, "AI Export Warning", MB_ICONWARNING);
		}
	}
	else
		MessageBox(dialog, "Sorry, that player doesn't have an AI File.", "AI Export", MB_OK);
}

void Players_HandleCommand(HWND dialog, WORD code, WORD id, HWND control)
{
	switch (code)
	{
	case BN_CLICKED:
	case CBN_SELCHANGE:
		switch (id)
		{
		case IDC_P_SP1:
		case IDC_P_SP2:
		case IDC_P_SP3:
		case IDC_P_SP4:
		case IDC_P_SP5:
		case IDC_P_SP6:
		case IDC_P_SP7:
		case IDC_P_SP8:
		case IDC_P_SG:
			SavePlayer(dialog);
			propdata.pindex = id - IDC_P_SP1;
			propdata.p = &scen.players[propdata.pindex];
			LoadPlayer(dialog);
			break;

		case IDC_P_EXAI:
			Players_ManageAI(dialog, false);
			break;

		case IDC_P_IMAI:
			Players_ManageAI(dialog, true);
			//we don't need a full LoadPlayer() for this
			SetDlgItemText(dialog, IDC_P_AI, propdata.p->ai);
			break;

		case IDC_P_SPDIP:
			propdata.p->diplomacy[propdata.sel0] = b_to_d[SendDlgItemMessage(dialog, IDC_P_DSTATE, BM_GETCHECK, 0, 0)];
			propdata.sel0 = SendMessage(control, CB_GETCURSEL, 0 ,0);
			SendDlgItemMessage(dialog, IDC_P_DSTATE, BM_SETCHECK, d_to_b[propdata.p->diplomacy[propdata.sel0]], 0);
			break;

		case IDC_P_AIMODE:
			EnableWindow(GetDlgItem(dialog, IDC_P_AI),
				SendMessage(control, BM_GETCHECK, 0, 0) == BST_UNCHECKED);
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

char ttAI[] =
"Leave blank to have no AI.";

BOOL Players_Init(HWND dialog)
{
	/* Fill Combo Boxes */
	LCombo_Fill(dialog, IDC_P_CIV, esdata.civs.head());
	Combo_Fill(dialog, IDC_P_SPDIP, Player::names, NUM_PLAYERS);
	SendDlgItemMessage(dialog, IDC_P_SPDIP, CB_SETCURSEL, 0, 0);	//set to player 0, or we get diplomacy[-1]
	LCombo_Fill(dialog, IDC_P_COLOR, esdata.colors.head());
	Combo_Fill(dialog, IDC_P_AGE, ages, NUM_AGES);

	/* Set resource names per game */
	Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_FOOD), FOOD_STRING);
	Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_WOOD), WOOD_STRING);
	Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_GOLD), GOLD_STRING);
	Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_STONE), STONE_STRING);
	Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_OREX), OREX_STRING);

	SendDlgItemMessage(dialog, IDC_P_X, EM_SETLIMITTEXT, 5, 0);
	SendDlgItemMessage(dialog, IDC_P_Y, EM_SETLIMITTEXT, 5, 0);

	AddTool(dialog, IDC_P_AI, ttAI);

	return TRUE;
}

INT_PTR CALLBACK PlyDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			ret = Players_Init(dialog);
			break;

		case WM_COMMAND:
			ret = 0;
			Players_HandleCommand(
				dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
			break;

		case WM_NOTIFY:
			{
				ret = TRUE;

				NMHDR *header = (NMHDR*)lParam;
				switch (header->code)
				{
				case PSN_SETACTIVE:
					CheckRadioButton(
						dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
					LoadPlayer(dialog);
					return ret;

				case PSN_KILLACTIVE:
					SavePlayer(dialog);
					break;
				}
			}
			break;

		case AOKTS_Loading:
			ret = TRUE;
			CheckRadioButton(
				dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
			LoadPlayer(dialog);
			return ret;

		case AOKTS_Saving:
			SavePlayer(dialog);
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
