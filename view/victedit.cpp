/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	victedit.cpp -- Defines functions for Victory editor.

	VIEW/CONTROLLER
**/

#include "editors.h"
#include "../resource.h"
#include "utilui.h"

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

/**
 * Resets the dialog when new data is available, such as when activating the
 * dialog.
 */
void reset(HWND dialog)
{
	LoadVictory(dialog);
	Vict_DisableControls(dialog, scen.vict.mode);
}

INT_PTR CALLBACK VictDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			Victory_HandleCommand(
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
					SaveVictory(dialog);
					break;
				}
			}
			break;

		case AOKTS_Loading:
			reset(dialog);
			return 0;

		case AOKTS_Saving:
			SaveVictory(dialog);
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
