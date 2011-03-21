#include "LinkListBox.h"

#include "../util/winugly.h"

void LinkListBox_Fill(HWND listbox, Link *list)
{
	for (; list; list = list->next())
		LinkListBox_Add(listbox, list);
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
