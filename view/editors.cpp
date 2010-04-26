/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	editors.cpp -- Defines functions for non-trigger editors.

	VIEW/CONTROLLER
**/

#include "editors.h"

#include "../util/settings.h"
#include "../model/scen.h"
#include "../resource.h"
#include "utilunit.h"
#include "LCombo.h"
#include "../util/winugly.h"

#include <time.h>
#include <commdlg.h>
#include <stdio.h>
#include <commctrl.h>
#include "utilui.h"
#include "mapview.h"

const char *errorBadReturn =
"An unidentified error occured. Please report this to DiGiT and look\n"
"in aokts.log for any extra information. I would also recommend saving\n"
"to a different file in case any corruption occured.";

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

/* Info / Messages */

int IM_msel = 0;
int IM_csel = 0;

void LoadIM(HWND dialog)
{
	SetDlgItemText(dialog, IDC_M_MSGS, scen.messages[IM_msel].c_str());
	SetDlgItemText(dialog, IDC_M_CINEM, scen.cinem[IM_csel]);
	CheckDlgButton(dialog, IDC_G_ALLTECHS, scen.all_techs);
	SetDlgItemInt(dialog, IDC_M_STABLE, scen.mstrings[IM_msel], TRUE);
}

void SaveM(HWND dialog)
{
	GetWindowText(GetDlgItem(dialog, IDC_M_MSGS), scen.messages[IM_msel]);
	GetDlgItemText(dialog, IDC_M_CINEM, scen.cinem[IM_csel], 0x20);
	scen.all_techs = IsDlgButtonChecked(dialog, IDC_G_ALLTECHS);
	scen.mstrings[IM_msel] = GetDlgItemInt(dialog, IDC_M_STABLE, NULL, TRUE);
}

void ExportBitmap(HWND dialog)
{
	char path[_MAX_PATH];
	OPENFILENAME ofn;
	bool success = true;

	if (scen.bBitmap)
	{
		strcpy(path, scen.cinem[3]);

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = dialog;
		ofn.lpstrFilter = "Bitmap (*.bmp)\0*.bmp\0";
		ofn.lpstrCustomFilter = NULL;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = path;
		ofn.nMaxFile = _MAX_PATH;
		ofn.lpstrFileTitle = NULL;
		ofn.lpstrInitialDir = setts.BasePath;
		ofn.lpstrTitle = "Export Bitmap";
		ofn.Flags = OFN_NOREADONLYRETURN;
		ofn.lpstrDefExt = "bmp";

		if (GetSaveFileName(&ofn))
			success = scen.export_bmp(path);

		if (!success)
			MessageBox(dialog, "Bitmap export failed.", "AOKTS Warning", MB_ICONWARNING);
	}
	else
		MessageBox(dialog, "No bitmap to export. Duh?", "Bitmap Export", MB_OK);
}

void IMsgs_HandleCommand(HWND dialog, WORD code, WORD id, HWND control)
{
	switch (code)
	{
	case BN_CLICKED:	//and menuitem click!
	case CBN_SELCHANGE:	//and accelerator!
		switch (id)
		{
		case IDC_M_EXPORT:
			ExportBitmap(dialog);
			break;

		case IDC_M_SEL:
			SaveM(dialog);
			IM_msel = (USHORT)SendMessage(control, CB_GETCURSEL, 0, 0);
			LoadIM(dialog);
			break;

		case IDC_M_SELC:
			SaveM(dialog);
			IM_csel = (USHORT)SendMessage(control, CB_GETCURSEL, 0, 0);
			LoadIM(dialog);
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

void IMsgs_Reset(HWND dialog)
{
	char string[20];

	//these aren't gonna change, so load them here
	sprintf(string, "%s (%.2f)", scen.header.version, scen.ver2);
	SetDlgItemText(dialog, IDC_G_VER, string);
	SetDlgItemText(dialog, IDC_G_TIMESTAMP, _ctime32(&scen.header.timestamp));
	SetDlgItemText(dialog, IDC_G_ONAME, scen.origname);
	SetDlgItemFloat(dialog, IDC_G_X, scen.editor_pos[0]);
	SetDlgItemFloat(dialog, IDC_G_Y, scen.editor_pos[1]);

	IM_msel = 0;
	SendDlgItemMessage(dialog, IDC_M_SEL, CB_SETCURSEL, 0, 0);
	IM_csel = 3;	//Bitmap = default
	SendDlgItemMessage(dialog, IDC_M_SELC, CB_SETCURSEL, 3, 0);
	LoadIM(dialog);
}

INT_PTR CALLBACK IMsgsDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			Combo_Fill(dialog, IDC_M_SEL, message_names, NUM_MSGS);
			Combo_Fill(dialog, IDC_M_SELC, cinem_names, NUM_CINEM);

			return TRUE;
		}

	case WM_COMMAND:
		IMsgs_HandleCommand(dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;

	case WM_NOTIFY:
		{
			NMHDR *header = (NMHDR*)lParam;
			switch (header->code)
			{
			case PSN_SETACTIVE:
				IMsgs_Reset(dialog);
				ret = 0;
				break;

			case PSN_KILLACTIVE:
				SaveM(dialog);
				break;
			}
		}
		break;

	case AOKTS_Saving:
		SaveM(dialog);
		break;

	case AOKTS_Loading:
		IMsgs_Reset(dialog);
		ret = 0;
		break;
	}

	return ret;
}

/* Players */

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
	LCombo_FillById(dialog, IDC_P_CIV, (unsigned)-1, esdata.civs);
	Combo_Fill(dialog, IDC_P_SPDIP, Player::names, NUM_PLAYERS);
	SendDlgItemMessage(dialog, IDC_P_SPDIP, CB_SETCURSEL, 0, 0);	//set to player 0, or we get diplomacy[-1]
	LCombo_FillById(dialog, IDC_P_COLOR, (unsigned)-1, esdata.colors);
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

	switch (msg)
	{
	case WM_INITDIALOG:
		ret = Players_Init(dialog);
		break;

	case WM_COMMAND:
		ret = 0;
		Players_HandleCommand(dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;

	case WM_NOTIFY:
		{
			ret = TRUE;

			NMHDR *header = (NMHDR*)lParam;
			switch (header->code)
			{
			case PSN_SETACTIVE:
				CheckRadioButton(dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
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
		CheckRadioButton(dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
		LoadPlayer(dialog);
		return ret;

	case AOKTS_Saving:
		SavePlayer(dialog);
	}

	return ret;
}

/* Global Victory */

void LoadVictory(HWND dialog)
{
	Victory *vict = &scen.vict;
	SetDlgItemInt(dialog, IDC_V_SCORE, vict->score, FALSE);
	SetDlgItemInt(dialog, IDC_V_TIME, vict->time, FALSE);
	SetDlgItemInt(dialog, IDC_V_RELICS, vict->relics, FALSE);
	SetDlgItemInt(dialog, IDC_V_EXPL, vict->expl, FALSE);
	SendDlgItemMessage(dialog, IDC_V_ALL, BM_SETCHECK, vict->all, 0);
	SendDlgItemMessage(dialog, IDC_V_CONQUEST, BM_SETCHECK, vict->conq, 0);
	CheckRadioButton(dialog, IDC_V_SSTD, IDC_V_SCUSTOM, IDC_V_SSTD + vict->mode);
}

void SaveVictory(HWND dialog)
{
	Victory *vict = &scen.vict;
	vict->score = GetDlgItemInt(dialog, IDC_V_SCORE, NULL, FALSE);
	vict->time = GetDlgItemInt(dialog, IDC_V_TIME, NULL, FALSE);
	vict->relics = GetDlgItemInt(dialog, IDC_V_RELICS, NULL, FALSE);
	vict->expl = GetDlgItemInt(dialog, IDC_V_EXPL, NULL, FALSE);
	vict->all = SendDlgItemMessage(dialog, IDC_V_ALL, BM_GETCHECK, 0, 0);
	vict->conq = SendDlgItemMessage(dialog, IDC_V_CONQUEST, BM_GETCHECK, 0, 0);
}

void Vict_DisableControls(HWND dialog, int mode)
{
	scen.vict.mode = mode;
	for (int i = IDC_V_ALL; i <= IDC_V_CONQUEST; i++)
		ENABLE_WND(i, false);
	switch (mode)
	{
	case Victory::MODE_Score:
		ENABLE_WND(IDC_V_SCORE, true);
		break;
	case Victory::MODE_Time:
		ENABLE_WND(IDC_V_TIME, true);
		break;	
	case Victory::MODE_Custom:
		ENABLE_WND(IDC_V_ALL, true);
		ENABLE_WND(IDC_V_RELICS, true);
		ENABLE_WND(IDC_V_EXPL, true);
		ENABLE_WND(IDC_V_CONQUEST, true);
		break;
	}
}

void Victory_HandleCommand(HWND dialog, WORD code, WORD id, HWND)
{
	switch (code)
	{
	case BN_CLICKED:	//or menuitem click
	case 1:				//accelerator
		if (id >= IDC_V_SSTD && id <= IDC_V_SCUSTOM)
			Vict_DisableControls(dialog, id - IDC_V_SSTD);
		else
		switch (id)
		{
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

INT_PTR CALLBACK VictDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		Victory_HandleCommand(dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;

	case WM_NOTIFY:
		{
			NMHDR *header = (NMHDR*)lParam;
			switch (header->code)
			{
			case PSN_SETACTIVE:
				goto Reset;
			case PSN_KILLACTIVE:
				SaveVictory(dialog);
				break;
			}
		}
		break;

	case AOKTS_Loading:
		goto Reset;

	case AOKTS_Saving:
		SaveVictory(dialog);
	}

	return 0;

Reset:
	LoadVictory(dialog);
	Vict_DisableControls(dialog, scen.vict.mode);

	return 0;
}

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

void LB_Fill(HWND listbox, Link *list)
{
	for (; list; list = list->next())
		LinkListBox_Add(listbox, list);
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

		LB_Fill(list_all, esdata.techs);

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

INT_PTR CALLBACK DisDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		Combo_Fill(dialog, IDC_D_SPLY, Player::names, NUM_PLAYERS - 1);
		Combo_Fill(dialog, IDC_D_STYPE, dtypes, NUM_TYPES);
		return TRUE;

	case WM_COMMAND:
		Disables_HandleCommand(dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;

	case WM_NOTIFY:
		{
			NMHDR *header = (NMHDR*)lParam;
			switch (header->code)
			{
			case PSN_SETACTIVE:
				goto Reset;
			case PSN_KILLACTIVE:
				SaveDisables(dialog);
			}
		}
		break;

	case AOKTS_Loading:
		goto Reset;

	case AOKTS_Saving:
		SaveDisables(dialog);
	}

	return 0;

Reset:
	propdata.sel0 = 0;
	LoadDisables(dialog);
	SendDlgItemMessage(dialog, IDC_D_SPLY, CB_SETCURSEL, propdata.pindex, 0);
	SendDlgItemMessage(dialog, IDC_D_STYPE, CB_SETCURSEL, 0, 0);
	return 0;
}

/*
	Terrain / Map

  propdata.sel0 = Current tile x
  propdata.sel1 = Current tile y
*/

#define NUM_SIZES 6
#define NUM_ELEVS 9

enum CLICK_STATES
{
	CLICK_Default,	//just selects tile on map
	CLICK_MCSet1,	//sets x,y coords for map copy rect from1
	CLICK_MCSet2,	//sets x,y coords for map copy rect from2
	CLICK_MCSetT	//sets x,y coords for map copy rect to
} click_state = CLICK_Default;

const char *szMapTitle = "Map Editor";
const char *szMapStatus =
"Click \"Set...\" and then a point on the map to automatically fill values.";

const char *sizes[NUM_SIZES] =
{
	"Tiny (120)", "Small (144)", "Medium (168)", "Normal (200)", "Large (220)", "Giant (240)"
};
const char *elevs[NUM_ELEVS] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };

const char *warningExceededMaxSize =
"Sorry, you have exceeded the maximum allowed mapsize. I will now set the mapsize to maximum.";
const char *warningSensibleRect =
"Please make a sensible source rectangle. I fixed it for you this time, but\n"
"don\'t expect me to be as nice in the future...";
const char *warningMapCopyOverlap =
"Sorry, I had to cancel your map copy because the source and destination areas\n"
"overlapped. This may be supported in the future.";
const char *infoMapCopySuccess =
"Map copy successful. (As far as I know... as always, check for bugs!)";

void LoadMap(HWND dialog, bool all)
{
	Map::Terrain *tn = &scen.map.terrain[propdata.sel0][propdata.sel1];
	const Link *ait;
	int index = -1, i;

	SendDlgItemMessage(dialog, IDC_TR_ID, LB_SETCURSEL, tn->cnst, 0);
	SendDlgItemMessage(dialog, IDC_TR_ELEV, CB_SETCURSEL, tn->elev, 0);

	if (!all)
		return;

	/* See if selected size is one of the built-ins. */
	for (i = 0; i < NUM_SIZES; i++)
	{
		if (scen.map.x == MapSizes[i])
		{
			SendDlgItemMessage(dialog, IDC_TR_SIZE, CB_SETCURSEL, i, 0);
			break;
		}
	}
	if (i == NUM_SIZES)
		SetDlgItemInt(dialog, IDC_TR_SIZE, scen.map.x, FALSE);

	ait = getById(esdata.aitypes, scen.map.aitype);

	if (ait)
	{
		index = LCombo_Find(dialog, IDC_TR_AITYPE, ait);
	}
	else
		printf("WARNING: Unknown ai constant (%d).\n", scen.map.aitype);

	SendDlgItemMessage(dialog, IDC_TR_AITYPE, CB_SETCURSEL, index, 0);
}

void SaveMap(HWND dialog)
{
	int size;
	Map::Terrain *tn = scen.map.terrain[propdata.sel0] + propdata.sel1;

	//First check standard sizes. If that fails, get the custom size.
	if ((size = SendDlgItemMessage(dialog, IDC_TR_SIZE, CB_GETCURSEL, 0, 0)) != LB_ERR)
		scen.map.x = MapSizes[size];
	else
		scen.map.x = GetDlgItemInt(dialog, IDC_TR_SIZE, NULL, FALSE);

	if (scen.map.x > MAX_MAPSIZE)
	{
		scen.map.x = MAX_MAPSIZE;
		MessageBox(dialog, warningExceededMaxSize, szMapTitle, MB_ICONWARNING);
	}

	scen.map.y = scen.map.x;	//maps are square

	scen.map.aitype = LCombo_GetSelId(dialog, IDC_TR_AITYPE);
	tn->cnst = static_cast<char>(
		LinkListBox_GetSel(GetDlgItem(dialog, IDC_TR_ID))->id());
	tn->elev = static_cast<char>(
		SendDlgItemMessage(dialog, IDC_TR_ELEV, CB_GETCURSEL, 0, 0));
}

void Map_UpdatePos(HWND dialog, WORD id)
{
	unsigned int value = GetDlgItemInt(dialog, id, NULL, FALSE);
	SendMessage(propdata.mapview, MAP_UnhighlightPoint,
		MAP_UNHIGHLIGHT_ALL, 0);

	if (value < scen.map.x)	//map is square, so we can compare to just map.x
	{
		if (propdata.sel0 >= 0 && propdata.sel1 >= 0)
			SaveMap(dialog);

		if (id == IDC_TR_TX)
			propdata.sel0 = value;
		else
			propdata.sel1 = value;

		SendMessage(propdata.mapview, MAP_HighlightPoint, propdata.sel0, propdata.sel1);
		LoadMap(dialog, false);
	}
}

void Map_HandleMapClick(HWND dialog, short x, short y)
{
	int ctrlx, ctrly;

	switch (click_state)
	{
	case CLICK_Default:
		ctrlx = IDC_TR_TX;
		ctrly = IDC_TR_TY;
		break;
	case CLICK_MCSet1:
		ctrlx = IDC_TR_MCX1;
		ctrly = IDC_TR_MCY1;
		click_state = CLICK_Default;
		break;
	case CLICK_MCSet2:
		ctrlx = IDC_TR_MCX2;
		ctrly = IDC_TR_MCY2;
		click_state = CLICK_Default;
		break;
	case CLICK_MCSetT:
		ctrlx = IDC_TR_MCXT;
		ctrly = IDC_TR_MCYT;
		click_state = CLICK_Default;
		break;
	default:
		return;
	}

	SetDlgItemInt(dialog, ctrlx, x, FALSE);
	SetDlgItemInt(dialog, ctrly, y, FALSE);
}

void Map_HandleMapCopy(HWND dialog)
{
	RECT source;
	int space;
	long temp;
	MapCopyCache *cache = NULL;
	void *clipboardData;
	HGLOBAL clipboardMem;
	bool disp = false;

	if (!OpenClipboard(dialog))
	{
		MessageBox(dialog, errorOpenClipboard, szMapTitle, MB_ICONWARNING);
		return;
	}

	/* Get the source rect */
	source.left   = GetDlgItemInt(dialog, IDC_TR_MCX1, NULL, FALSE);
	source.bottom    = GetDlgItemInt(dialog, IDC_TR_MCY1, NULL, FALSE);	//top and bottom are "normal", reverse from GDI standard
	source.right  = GetDlgItemInt(dialog, IDC_TR_MCX2, NULL, FALSE);
	source.top = GetDlgItemInt(dialog, IDC_TR_MCY2, NULL, FALSE);

	/* We need to make sure it's a sane rectangle. */
	if (source.left > source.right)
	{
		temp = source.left;
		source.left = source.right;
		source.right = temp;
		disp = true;
	}
	if (source.bottom > source.top)
	{
		temp = source.top;
		source.top = source.bottom;
		source.bottom = temp;
		disp = true;
	}
	if (disp)
	{
		MessageBox(dialog, warningSensibleRect, szMapTitle, MB_ICONWARNING);
	}
	
	EmptyClipboard();

	/* allocate the clipboard memory */
	space = scen.map_size(source, cache);
	clipboardMem = GlobalAlloc(GMEM_MOVEABLE, space);

	if (clipboardMem == NULL)
	{
		MessageBox(dialog, errorAllocFailed, szMapTitle, MB_ICONWARNING);
		goto Cleanup;
	}

	/* actually write the data */
	clipboardData = GlobalLock(clipboardMem);
	if (clipboardData)
	{
		Buffer data((char*)clipboardData, space);
		scen.map_copy(data, cache);
		cache = NULL;
	}
	GlobalUnlock(clipboardMem);

	if (!SetClipboardData(propdata.mcformat, clipboardMem))
		MessageBox(dialog, errorSetClipboard, szMapTitle, MB_ICONWARNING);

	EnableWindow(GetDlgItem(dialog, IDC_TR_MCPASTE), true);
	
Cleanup:
	CloseClipboard();
}

void Map_HandleMapPaste(HWND dialog)
{
	HGLOBAL cbMem;
	POINT target;
	void *cbData;
	size_t size;

	target.x = GetDlgItemInt(dialog, IDC_TR_MCXT, NULL, FALSE);
	target.y = GetDlgItemInt(dialog, IDC_TR_MCYT, NULL, FALSE);

	/*	Check whether map copy data is actually available. (Should be if we've
		gotten this far.
	*/
	if (!IsClipboardFormatAvailable(propdata.mcformat))
	{
		MessageBox(dialog, warningNoFormat, szMapTitle, MB_ICONWARNING);
		return;
	}

	if (!OpenClipboard(dialog))
		MessageBox(dialog, errorOpenClipboard, szMapTitle, MB_ICONWARNING);

	cbMem = GetClipboardData(propdata.mcformat);
	size = GlobalSize(cbMem);
	cbData = GlobalLock(cbMem);
	if (cbData)
	{
		Buffer b((char*)cbData, size);

		scen.map_paste(target, b);
	}
	else
		MessageBox(dialog, "Could not paste clipboard data.",
			szMapTitle, MB_ICONWARNING);
	GlobalUnlock(cbMem);

	CloseClipboard();

	SendMessage(propdata.mapview, MAP_Reset, 0, 0);
}

void Map_HandleSetFocus(HWND, WORD)
{
	EnableMenuItem(propdata.menu, ID_EDIT_COPY, MF_ENABLED);
	EnableMenuItem(propdata.menu, ID_EDIT_CUT, MF_ENABLED);
	if (IsClipboardFormatAvailable(CF_TEXT))
		EnableMenuItem(propdata.menu, ID_EDIT_PASTE, MF_ENABLED);
}

void Map_HandleKillFocus(HWND dialog, WORD id)
{
	EnableMenuItem(propdata.menu, ID_EDIT_COPY, MF_GRAYED);
	EnableMenuItem(propdata.menu, ID_EDIT_CUT, MF_GRAYED);
	EnableMenuItem(propdata.menu, ID_EDIT_PASTE, MF_GRAYED);

	switch (id)
	{
	case IDC_TR_MCX1:
	case IDC_TR_MCY1:
	case IDC_TR_MCX2:
	case IDC_TR_MCY2:
	case IDC_TR_MCXT:
	case IDC_TR_MCYT:
		SendMessage(propdata.mapview, MAP_UnhighlightPoint, MAP_UNHIGHLIGHT_ALL, 0);
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_TR_MCX1, NULL, FALSE),
			GetDlgItemInt(dialog, IDC_TR_MCY1, NULL, FALSE));
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_TR_MCX2, NULL, FALSE),
			GetDlgItemInt(dialog, IDC_TR_MCY2, NULL, FALSE));
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_TR_MCXT, NULL, FALSE),
			GetDlgItemInt(dialog, IDC_TR_MCYT, NULL, FALSE));
		break;
	}
}

void Map_HandleCommand(HWND dialog, WORD code, WORD id, HWND)
{
	switch (code)
	{
	case EN_CHANGE:
		switch (id)
		{
		case IDC_TR_TX:
		case IDC_TR_TY:
			Map_UpdatePos(dialog, id);
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

	case BN_CLICKED:
		switch (id)
		{
		case IDC_TR_MCSET1:
			click_state = CLICK_MCSet1;
			break;

		case IDC_TR_MCSET2:
			click_state = CLICK_MCSet2;
			break;

		case IDC_TR_MCSETT:
			click_state = CLICK_MCSetT;
			break;

		case IDC_TR_MCCOPY:
			Map_HandleMapCopy(dialog);
			break;

		case IDC_TR_MCPASTE:
			Map_HandleMapPaste(dialog);
			break;
		}

	case EN_SETFOCUS:
		Map_HandleSetFocus(dialog, id);
		break;

	case EN_KILLFOCUS:
		Map_HandleKillFocus(dialog, id);
		break;
	}
}

void Map_Reset(HWND dialog)
{
	propdata.sel0 = -1;
	propdata.sel1 = -1;
	SetDlgItemInt(dialog, IDC_TR_TX, 0, FALSE);
	SetDlgItemInt(dialog, IDC_TR_TY, 0, FALSE);
	LoadMap(dialog, true);
	SetWindowText(propdata.statusbar, szMapStatus);
}

INT_PTR CALLBACK MapDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;	//default: doesn't process message

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			LB_Fill(GetDlgItem(dialog, IDC_TR_ID), esdata.terrains);

			LCombo_FillById(dialog, IDC_TR_AITYPE, (unsigned)-1, esdata.aitypes);
			Combo_Fill(dialog, IDC_TR_SIZE, sizes, NUM_SIZES);
			Combo_Fill(dialog, IDC_TR_ELEV, elevs, NUM_ELEVS);

			ret = TRUE;
		}
		break;

	case WM_COMMAND:
		Map_HandleCommand(dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;

	case WM_NOTIFY:
		{
			NMHDR *header = (NMHDR*)lParam;
			switch (header->code)
			{
			case PSN_SETACTIVE:
				Map_Reset(dialog);
				break;
			case PSN_KILLACTIVE:
				SaveMap(dialog);
				break;
			}

			ret = TRUE;
		}
		break;

	case MAP_Click:
		Map_HandleMapClick(dialog, LOWORD(lParam), HIWORD(lParam));
		break;

	case AOKTS_Loading:
		Map_Reset(dialog);
		break;

	case AOKTS_Saving:
		SaveMap(dialog);
		break;
	}

	return ret;
}
