/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	playeredit.cpp -- Defines functions for Players editor.

	VIEW/CONTROLLER
**/

#include "editors.h"
#include "../res/resource.h"
#include "utilui.h"
#include "../util/settings.h"
#include "LCombo.h"
#include "mapview.h"
#include "../util/winugly.h"

#include <commdlg.h>

#define NUM_MAX_TEAMS        3
#define NUM_MAX_PLAYER_NAMES 9

const char *max_teams_names[] = { "2 Teams", "3 Teams", "4 Teams" };
const char *num_players_names[] = { "INVALID", "1", "2", "3", "4", "5", "6", "7", "8" };
const char *players_number_names[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/* Players */

void LoadMaxTeams(HWND dialog)
{
    if (scen.max_teams == 0) {
        SendDlgItemMessage(dialog, IDC_P_MAX_TEAMS, CB_SETCURSEL, 0, 0);
    } else {
        SendDlgItemMessage(dialog, IDC_P_MAX_TEAMS, CB_SETCURSEL, scen.max_teams - 2, 0);
    }
}

void SaveMaxTeams(HWND dialog)
{
    if (scen.game == AOHD4 || scen.game == AOF4 || scen.game == AOHD6 || scen.game == AOF6) {
        int cur = SendDlgItemMessage(dialog, IDC_P_MAX_TEAMS, CB_GETCURSEL, 0, 0);
        if (cur == 0) {
            scen.max_teams = cur;
        } else {
            scen.max_teams = cur + 2;
        }
    }
}

void LoadActive(HWND dialog)
{
    int num_players = scen.get_number_active_players();
    if (num_players == ERR_combination) {
        SendDlgItemMessage(dialog, IDC_P_NUM_PLAYERS, CB_SETCURSEL, 0, 0);
    } else {
        SendDlgItemMessage(dialog, IDC_P_NUM_PLAYERS, CB_SETCURSEL, num_players, 0);
    }
}

void LoadPlayers(HWND dialog)
{
    for (int i = 0; i < 9; i++) {
	    SendDlgItemMessage(dialog, IDC_P_P1_NUM + i, CB_SETCURSEL, scen.players[i].player_number, 0);
	    SetDlgItemText(dialog, IDC_P_NAME1 + i, scen.players[i].name);
	    LCombo_SelById(dialog, IDC_P_CIV1 + i, scen.players[i].civ);
	    SetDlgItemInt(dialog, IDC_P_GOLD1 + i, scen.players[i].resources[0], FALSE);
	    SetDlgItemInt(dialog, IDC_P_WOOD1 + i, scen.players[i].resources[1], FALSE);
	    SetDlgItemInt(dialog, IDC_P_FOOD1 + i, scen.players[i].resources[2], FALSE);
	    SetDlgItemInt(dialog, IDC_P_STONE1 + i, scen.players[i].resources[3], FALSE);
	    SendDlgItemMessage(dialog, IDC_P_HUMAN1 + i, BM_SETCHECK, scen.players[i].human, 0);
	    SetDlgItemInt(dialog, IDC_P_POP1 + i, (int)scen.players[i].pop, FALSE);
	    SendDlgItemMessage(dialog, IDC_P_COLOR1 + i, CB_SETCURSEL, scen.players[i].color, 0);	//assuming in order
	    SendDlgItemMessage(dialog, IDC_P_AGE1 + i, CB_SETCURSEL, scen.players[i].age + 1, 0);
    }
}

void SavePlayers(HWND dialog)
{
	for (int i = 0; i < 9; i++) {
	    scen.players[i].resources[0] = GetDlgItemInt(dialog, IDC_P_GOLD1 + i, NULL, FALSE);
	    scen.players[i].resources[1] = GetDlgItemInt(dialog, IDC_P_WOOD1 + i, NULL, FALSE);
	    scen.players[i].resources[2] = GetDlgItemInt(dialog, IDC_P_FOOD1 + i, NULL, FALSE);
	    scen.players[i].resources[3] = GetDlgItemInt(dialog, IDC_P_STONE1 + i, NULL, FALSE);
	    GetDlgItemText(dialog, IDC_P_NAME1 + i, scen.players[i].name, 30);
	    scen.players[i].civ = LCombo_GetSelId(dialog, IDC_P_CIV1 + i);
	    scen.players[i].human = Button_IsChecked(GetDlgItem(dialog, IDC_P_HUMAN1 + i));
	    scen.players[i].pop = static_cast<float>(GetDlgItemInt(dialog, IDC_P_POP1 + i, NULL, FALSE));
	    scen.players[i].color = LCombo_GetSelId(dialog, IDC_P_COLOR1 + i);
	    scen.players[i].age = SendDlgItemMessage(dialog, IDC_P_AGE1 + i, CB_GETCURSEL, 0, 0) - 1;
	    scen.players[i].player_number = SendDlgItemMessage(dialog, IDC_P_P1_NUM + i, CB_GETCURSEL, 0, 0);
	}
}

void Players_HandleCommand(HWND dialog, WORD code, WORD id, HWND control)
{
	Player *p = propdata.p;

	switch (code)
	{
	case BN_CLICKED:
	case CBN_SELCHANGE:
		switch (id)
		{
		case IDC_P_NUM_PLAYERS:
		    {
		        int h;
		        if ((h = SendDlgItemMessage(dialog, IDC_P_NUM_PLAYERS, CB_GETCURSEL, 0, 0)) != LB_ERR) {
		            scen.set_number_active_players(h);
		            LoadPlayers(dialog);
	                LoadActive(dialog);
	                LoadMaxTeams(dialog);
		        }
		    }
		    break;

		case IDC_PS_SP1:
		case IDC_PS_SP2:
		case IDC_PS_SP3:
		case IDC_PS_SP4:
		case IDC_PS_SP5:
		case IDC_PS_SP6:
		case IDC_PS_SP7:
		case IDC_PS_SP8:
		case IDC_PS_SG:
			SavePlayers(dialog);
	        SaveMaxTeams(dialog);
			propdata.pindex = id - IDC_PS_SP1;
			propdata.p = &scen.players[propdata.pindex];
			LoadPlayers(dialog);
	        LoadActive(dialog);
	        LoadMaxTeams(dialog);
			break;

		case ID_TS_EDIT_COPY:
			SendMessage(GetFocus(), WM_COPY, 0, 0);
			break;

		case ID_TS_EDIT_CUT:
			SendMessage(GetFocus(), WM_CUT, 0, 0);
			break;

		case ID_TS_EDIT_PASTE:
			SendMessage(GetFocus(), WM_PASTE, 0, 0);
			break;
		}
		break;

	case EN_SETFOCUS:
		EnableMenuItem(propdata.menu, ID_TS_EDIT_COPY, MF_ENABLED);
		EnableMenuItem(propdata.menu, ID_TS_EDIT_CUT, MF_ENABLED);
		if (IsClipboardFormatAvailable(CF_TEXT))
			EnableMenuItem(propdata.menu, ID_TS_EDIT_PASTE, MF_ENABLED);
		break;

	case EN_KILLFOCUS:
		EnableMenuItem(propdata.menu, ID_TS_EDIT_COPY, MF_GRAYED);
		EnableMenuItem(propdata.menu, ID_TS_EDIT_CUT, MF_GRAYED);
		EnableMenuItem(propdata.menu, ID_TS_EDIT_PASTE, MF_GRAYED);
		break;
	}
}

BOOL Players_Init(HWND dialog)
{
    ENABLE_WND(IDC_P_MAX_TEAMS, scen.game == AOHD4 || scen.game == AOF4 || scen.game == AOHD6 || scen.game == AOF6);

	Combo_Fill(dialog, IDC_P_MAX_TEAMS, max_teams_names, NUM_MAX_TEAMS);
	Combo_Fill(dialog, IDC_P_NUM_PLAYERS, num_players_names, NUM_MAX_PLAYER_NAMES);

    for (int i = 0; i < 9; i++) {
        Combo_Fill(dialog, IDC_P_P1_NUM + i, players_number_names, 9);
	    SendDlgItemMessage(dialog, IDC_P_P1_NUM + i, CB_SETCURSEL, scen.players[i].player_number, 0);
	    LCombo_Fill(dialog, IDC_P_CIV1 + i, esdata.civs.head());
	    SendDlgItemMessage(dialog, IDC_P_CIV1 + i, CB_SETCURSEL, scen.players[i].civ, 0);
	    LCombo_Fill(dialog, IDC_P_COLOR1 + i, esdata.colors.head());
	    SendDlgItemMessage(dialog, IDC_P_COLOR1 + i, CB_SETCURSEL, scen.players[i].color, 0);
	    Combo_PairFill(GetDlgItem(dialog, IDC_P_AGE1 + i), NUM_AGES, ages);
	    SendDlgItemMessage(dialog, IDC_P_AGE1 + i, CB_SETCURSEL, scen.players[i].age, 0);
    }

    if (scen.game == AOHD4 || scen.game == AOF4 || scen.game == AOHD6 || scen.game == AOF6) {
	    for (int i = 0; i < 9; i++) {
	        ENABLE_WND(IDC_P_P1_NUM + i, true);
	    }
	} else {
	    for (int i = 0; i < 9; i++) {
	        ENABLE_WND(IDC_P_P1_NUM + i, false);
	    }
	}

	/* Set resource names per game */
	if (scen.game == SWGB) {
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_FOOD), L"Food:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_WOOD), L"Carbon:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_GOLD), L"Nova:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_STONE), L"Ore:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_OREX), L"Ore X:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_OREY), L"Goods:");
	} else {
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_FOOD), L"Food:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_WOOD), L"Wood:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_GOLD), L"Gold:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_STONE), L"Stone:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_OREX), L"Ore X:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_OREY), L"Goods:");
	}

	return TRUE;
}

INT_PTR CALLBACK PlayersDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
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

		case AOKTS_Loading:
			ret = Players_Init(dialog);
			CheckRadioButton(
				dialog, IDC_PS_SP1, IDC_PS_SG, IDC_PS_SP1 + propdata.pindex);
			LoadPlayers(dialog);
	        LoadActive(dialog);
	        LoadMaxTeams(dialog);
			return ret;

		case AOKTS_Saving:
			SavePlayers(dialog);
	        SaveMaxTeams(dialog);
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
