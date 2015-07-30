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

/* Players */

//these convert from enum Diplomacy to Win32 Checkbox states and vice-versa
WPARAM d_to_b[4] = { BST_CHECKED, BST_INDETERMINATE, BST_UNCHECKED, BST_UNCHECKED };
enum Diplomacy b_to_d[3] = { DIP_enemy, DIP_ally, DIP_neutral };

//These controls are disabled when user selects GAIA.
int gaia_disables[] =
{
	IDC_P_STABLE, IDC_P_ACTIVE,
	IDC_P_AV, IDC_P_X, IDC_P_Y,
};

void LoadPlayer(HWND dialog)
{
	Player *p = propdata.p;
	bool is_gaia = (propdata.pindex == GAIA_INDEX);

    if (!setts.editall)
	    for (int i = 0; i < sizeof(gaia_disables) / sizeof(int); i++)
		    ENABLE_WND(gaia_disables[i], !is_gaia);

	if (propdata.pindex == PLAYER1_INDEX)
		ENABLE_WND(IDC_P_ACTIVE, false);

	SetDlgItemInt(dialog, IDC_P_STABLE, p->stable, TRUE);
	SetDlgItemInt(dialog, IDC_P_OREX, p->resources[4], FALSE);
	SetDlgItemInt(dialog, IDC_P_OREY, p->resources[5], FALSE);
	SendDlgItemMessage(dialog, IDC_P_ACTIVE, BM_SETCHECK, p->enable, 0);
	SetDlgItemFloat(dialog, IDC_P_X, p->camera[0]);
	SetDlgItemFloat(dialog, IDC_P_Y, p->camera[1]);
	SendDlgItemMessage(dialog, IDC_P_AV, BM_SETCHECK, p->avictory, 0);
	for (int i = 0; i < 9; i++) {
		SendDlgItemMessage(dialog, IDC_P_DSTATE1 + i, BM_SETCHECK, d_to_b[p->diplomacy[i]], 0);
	}
	SetDlgItemFloat(dialog, IDC_P_UF, p->ucount);
	SetDlgItemInt(dialog, IDC_P_US0, p->u1, FALSE);
	SetDlgItemInt(dialog, IDC_P_US1, p->u2, FALSE);
}

void SavePlayer(HWND dialog)
{
	Player *p = propdata.p;
	p->stable = GetDlgItemInt(dialog, IDC_P_STABLE, NULL, TRUE);
	p->resources[4] = GetDlgItemInt(dialog, IDC_P_OREX, NULL, FALSE);
	p->resources[5] = GetDlgItemInt(dialog, IDC_P_OREY, NULL, FALSE);
	p->enable = Button_IsChecked(GetDlgItem(dialog, IDC_P_ACTIVE));
	p->camera[0] = GetDlgItemFloat(dialog, IDC_P_X);
	p->camera[1] = GetDlgItemFloat(dialog, IDC_P_Y);
	p->avictory = (SendDlgItemMessage(dialog, IDC_P_AV, BM_GETCHECK, 0, 0) != 0);
	for (int i = 0; i < 9; i++) {
	    p->diplomacy[i] = b_to_d[SendDlgItemMessage(dialog, IDC_P_DSTATE1 + i, BM_GETCHECK, 0, 0)];
	}
	p->u1 = toshort(GetDlgItemInt(dialog, IDC_P_US0, NULL, FALSE));
	p->u2 = toshort(GetDlgItemInt(dialog, IDC_P_US1, NULL, FALSE));
}

void Player_HandleCommand(HWND dialog, WORD code, WORD id, HWND control)
{
	Player *p = propdata.p;

	switch (code)
	{
	case BN_CLICKED:
	case CBN_SELCHANGE:
		switch (id)
		{
		case IDC_P_ACTIVE:
		    p->enable = Button_IsChecked(GetDlgItem(dialog, IDC_P_ACTIVE));
		    break;
		case IDC_P_SWAPP1:
		    scen.swap_players(propdata.pindex, 0);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP2:
		    scen.swap_players(propdata.pindex, 1);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP3:
		    scen.swap_players(propdata.pindex, 2);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP4:
		    scen.swap_players(propdata.pindex, 3);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP5:
		    scen.swap_players(propdata.pindex, 4);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP6:
		    scen.swap_players(propdata.pindex, 5);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP7:
		    scen.swap_players(propdata.pindex, 6);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP8:
		    scen.swap_players(propdata.pindex, 7);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPGA:
		    scen.swap_players(propdata.pindex, 8);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;

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

BOOL Player_Init(HWND dialog)
{
	SendDlgItemMessage(dialog, IDC_P_X, EM_SETLIMITTEXT, 5, 0);
	SendDlgItemMessage(dialog, IDC_P_Y, EM_SETLIMITTEXT, 5, 0);

	return TRUE;
}

INT_PTR CALLBACK PlayerDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			ret = Player_Init(dialog);
			break;

		case WM_COMMAND:
			ret = 0;
			Player_HandleCommand(
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
			ret = Player_Init(dialog);
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
