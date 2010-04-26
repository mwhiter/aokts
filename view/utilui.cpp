/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	utilui.cpp -- Definitions of Utility Functions for UI

	VIEW/CONTROLLER
**/

#include "../model/scen_const.h"
#include "utilui.h"
#include "../util/winugly.h"

#include <stdio.h>
#include <stdlib.h>
#include <commdlg.h>

/* Options */
const size_t CONV_BUFF_SIZE = 32; // size of string conversion buffers

/* Globals */
HWND tooltip;	//set with TooltipInit()
/* Interface */

void Combo_Fill(HWND dialog, int id, char const * * strings, size_t count)
{
	HWND control = GetDlgItem(dialog, id);
	for (size_t i = 0; i < count; ++i)
		Combo_AddStringA(control, *strings++);
}

LRESULT LinkListBox_Add(HWND listbox, const Link *link)
{
	WPARAM index = List_AddStringW(listbox, link->name());
	List_SetItemData(listbox, index, link);

	return index;
}

const Link * LinkListBox_Get(HWND listbox, WPARAM index)
{
	return static_cast<const Link *>(
		List_GetItemData_cPtr(listbox, index));
}

void GetWindowText(HWND wnd, SString &value)
{
	int len = GetWindowTextLengthA(wnd);
	if (len)
	{
		char *cstr = value.unlock(++len);
		GetWindowTextA(wnd, cstr, len);
		value.lock();
	}
	else
		value.erase();
}

void SetWindowText(HWND wnd, int value)
{
	wchar_t buffer[CONV_BUFF_SIZE];
	swprintf(buffer, CONV_BUFF_SIZE, L"%d", value);
	SetWindowTextW(wnd, buffer);
}

float GetDlgItemFloat(HWND dialog, UINT id)
{
	// We use Unicode buffers since it's what Windows uses internally and we
	// pay nothing for using it here.

	// get the text
	wchar_t buffer[CONV_BUFF_SIZE];
	GetDlgItemTextW(dialog, id, buffer, CONV_BUFF_SIZE);

	// scan the text
	float ret = 0.0; // default to 0.0
	swscanf(buffer, L"%f", &ret);

	return ret;
}

void SetDlgItemFloat(HWND dialog, UINT id, double value)
{
	// See GetDlgItemFloat() for "why Unicode?".

	wchar_t buffer[CONV_BUFF_SIZE];
	swprintf(buffer, CONV_BUFF_SIZE, L"%.1f", value);
	SetDlgItemTextW(dialog, id, buffer);
}

UINT GetCheckedRadio(HWND dialog, UINT first, UINT last)
{
	while (first <= last)
	{
		if (SendDlgItemMessage(dialog, first, BM_GETCHECK, 0, 0))
			return first;
		first++;
	}
	return 0;
}

void FillCB(HWND combobox, unsigned count, const struct PAIR *pairs, unsigned select)
{
	unsigned index, type;

	while (count--)
	{
		type = pairs->index;
		index = SendMessage(combobox, CB_ADDSTRING, 0, (LPARAM)pairs->name);
		SendMessage(combobox, CB_SETITEMDATA, index, type);
		if (type == select)
			SendMessage(combobox, CB_SETCURSEL, index, 0);

		pairs++;
	}
}

bool GetOpenFileNameA(HWND owner, char * path, DWORD maxPath)
{
	OPENFILENAMEA ofn;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFilter = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = path;
	ofn.nMaxFile = maxPath;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = NULL;

	*path = '\0';

	return GetOpenFileNameA(&ofn) == TRUE;
}

bool GetSaveFileNameA(HWND owner, char * path, DWORD maxPath)
{
	OPENFILENAMEA ofn;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFilter = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = path;
	ofn.nMaxFile = maxPath;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = 0;
	ofn.lpstrDefExt = NULL;

	*path = '\0';

	return GetSaveFileNameA(&ofn) == TRUE;
}

/* Tooltips */

void TooltipInit(HWND tt)
{
	tooltip = tt;
}

BOOL AddTool(HWND dialog, int ctrlId, LPTSTR text)
{
	TOOLINFO ti;

	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = dialog;
	ToolTip_FillIDWithHwnd(ti, GetDlgItem(dialog, ctrlId));
	ti.lpszText = text;

	return ToolTip_AddTool(tooltip, &ti);
}

