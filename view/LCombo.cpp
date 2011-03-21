#include "LCombo.h"

#include "../model/esdata.h"
#include "../resource.h"
#include "../util/winugly.h"

/*
 * Convenience function to simultaneously add a string and data to a combo box.
 */
LRESULT Combo_AddW(HWND combobox, LPCWSTR string, const void * data)
{
	LRESULT index = Combo_AddStringW(combobox, string);
	Combo_SetItemData(combobox, index, data);

	return index;
}

int LinkComboBox_Fill(HWND combobox, const Link *list, const Link *select,
		const wchar_t * nosel)
{
	int ret = -1;

	SendMessage(combobox, CB_RESETCONTENT, 0, 0);

	if (nosel)
		Combo_AddW(combobox, nosel, NULL);

	for (; list; list = list->next())
	{
		LRESULT index = Combo_AddW(combobox, list->name(), list);

		if (list == select)
		{
			SendMessage(combobox, CB_SETCURSEL, index, 0);
			ret = index;
		}
	}

	if (ret == -1 && nosel)
	{
		SendMessage(combobox, CB_SETCURSEL, 0, 0);
		ret = 0;
	}

	return ret;
}

int LinkComboBox_FillById(HWND combobox, const Link *list, int select,
		const wchar_t * nosel)
{
	int ret = -1;

	SendMessage(combobox, CB_RESETCONTENT, 0, 0);

	if (nosel)
		Combo_AddW(combobox, nosel, NULL);

	for (; list; list = list->next())
	{
		LRESULT index = Combo_AddW(combobox, list->name(), list);
		if (list->id() == select)
		{
			SendMessage(combobox, CB_SETCURSEL, index, 0);
			ret = index;
		}
	}

	if (ret == -1 && nosel)
	{
		SendMessage(combobox, CB_SETCURSEL, 0, 0);
		ret = 0;
	}

	return ret;
}

int LinkComboBox_GetSelId(HWND combobox)
{
	const Link * sel = LinkComboBox_GetSelPtr(combobox);

	return sel ? sel->id() : -1;
}

const Link *LinkComboBox_GetSelPtr(HWND combobox)
{
	LRESULT index;

	// Get current selection
	index = SendMessage(combobox, CB_GETCURSEL, 0, 0);
	if (index == CB_ERR)
		return NULL;

	// Return its data, casted to a Link *
	// See Combo_AddW() above for SetItemData
	return static_cast<const Link *>(Combo_GetItemData_cPtr(combobox, index));
}

int LinkComboBox_Find(HWND combobox, const Link * data)
{
	int ret = -1, i;
	int count;

	count = SendMessage(combobox, CB_GETCOUNT, 0, 0);
	for (i = 0; i < count; i++)
	{
		// See Combo_AddW() above for SetItemData
		if (Combo_GetItemData_cPtr(combobox, i) == data)
		{
			ret = i;
			break;
		}
	}

	return ret;
}

int LCombo_Select(HWND dialog, int id, const Link * item)
{
	// Get the combobox handle
	HWND combobox = GetDlgItem(dialog, id);

	// Lookup the index of the item in the combobox
	int index = LinkComboBox_Find(combobox, item);

	// If the item was not found and is not null, add an entry for it.
	if (index == -1 && item != NULL)
	{
		index = Combo_AddW(combobox, item->name(), item);
	}

	// Set the selection to the item's index
	SendMessage(combobox, CB_SETCURSEL, index, 0);

	return index;
}

int LinkComboBox_SelById(HWND combobox, int x)
{
	int ret = -1, i;
	int count;

	count = SendMessage(combobox, CB_GETCOUNT, 0, 0);
	for (i = 0; i < count; i++)
	{
		const Link * link = static_cast<const Link *>(
			Combo_GetItemData_cPtr(combobox, i));

		// link may be NULL if client provided a nosel
		if (link && link->id() == x)
		{
			SendMessage(combobox, CB_SETCURSEL, i, 0);
			ret = i;
			break;
		}
	}

	return ret;
}
