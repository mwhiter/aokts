#ifndef INC_LCOMBO_H
#define INC_LCOMBO_H

#include <windows.h>
#include "../model/esdata.h"

/**
 * LinkComboBox: a combo box that stores Link *'s associated with each item.
 */

/* Clears contents and fills the combobox with the provided items, selecting
 * the item specified by lParam.
 *
 * @param nosel If non-NULL, adds an item to the combo box representing "no
 * selection".
 *
 * @return index of selection
 */
int LinkComboBox_Fill(HWND combobox, const Link *list, const Link *select,
		const wchar_t * nosel);

inline LRESULT LCombo_Fill(HWND dialog, int id, const Link * sel_link,
		const Link * list, const wchar_t * nosel = NULL)
{
	return LinkComboBox_Fill(GetDlgItem(dialog, id), list, sel_link, nosel);
}

/* Clears contents and fills the combobox with the provided items, selecting
 * the item specified by lParam.
 *
 * @param nosel If non-NULL, adds an item to the combo box representing "no
 * selection".
 * 
 * return: index of selection
 */
int LinkComboBox_FillById(HWND combobox, const Link *list, int select,
		const wchar_t * nosel);

inline LRESULT LCombo_FillById(HWND dialog, int id, WPARAM sel_index, const
		Link * list, const wchar_t * nosel = NULL)
{
	return LinkComboBox_FillById(GetDlgItem(dialog, id), list, sel_index,
		nosel);
}

/*
 * @return Link::id() of current selection
 */
int LinkComboBox_GetSelId(HWND combobox);

inline int LCombo_GetSelId(HWND dialog, int id)
{
	return LinkComboBox_GetSelId(GetDlgItem(dialog, id));
}

/*
 * @return Link* of current selection
 */
const Link *LinkComboBox_GetSelPtr(HWND combobox);

inline const Link * LCombo_GetSelPtr(HWND dialog, int id)
{
	return LinkComboBox_GetSelPtr(GetDlgItem(dialog, id));
}

/*
 * return: index of deisred item
 */
int LinkComboBox_Find(HWND combobox, const Link * data);

inline LRESULT LCombo_Find(HWND dialog, int id, const Link * item)
{
	return LinkComboBox_Find(GetDlgItem(dialog, id), item);
}

int LinkComboBox_SelById(HWND combobox, int x);

/*	
 * return: index of selected item
 */
inline int LCombo_SelById(HWND dialog, int id, int x)
{
	return LinkComboBox_SelById(GetDlgItem(dialog, id), x);
}

#endif // INC_LCOMBO_H

