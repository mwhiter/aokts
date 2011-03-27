#ifndef INC_LINKLISTBOX_H
#define INC_LINKLISTBOX_H

#include "utilui.h"
#include "../model/esdata.h"

/**
 * LinkListBox: a List Box that stores Link *'s associated with each item.
 */

/**
 * Fills the given listbox with links from the given list.
 */
void LinkListBox_Fill(HWND listbox, Link *list);

/*
 * Appends a Link * to a List Box. Should be used in conjunction with
 * LinkListBox_Get().
 */
LRESULT LinkListBox_Add(HWND listbox, const Link *);

/*
 * Retrieves Link * from the specified index in the listbox. Returns NULL if
 * an error occurs. Should be used in conjunction with LinkListBox_Add().
 */
const Link * LinkListBox_Get(HWND listbox, WPARAM index);

/*
 * Retrieves Link * from the currently-selected item in the listbox.
 */
inline const Link * LinkListBox_GetSel(HWND listbox);

/**
 * Returns index of the item that has the specified associated Link, or
 * UINT_MAX if none found.
 */
inline LRESULT LinkListBox_GetIndex(HWND listbox, Link const * link);

/** Inline function definitions... don't look! **/

const Link * LinkListBox_GetSel(HWND listbox)
{
	return LinkListBox_Get(listbox,
		SendMessage(listbox, LB_GETCURSEL, 0, 0));
}

inline LRESULT LinkListBox_GetIndex(HWND listbox, Link const * link)
{
	return ListBox_Find(listbox, link);
}

#endif // INC_LINKLISTBOX_H

