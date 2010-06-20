#ifndef INC_WINUGLY_H
#define INC_WINUGLY_H

/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	winugly.h -- utility functions to hide ugly casts

	UTIL
**/

#include <windows.h>
/* Go ahead and include commctrl.h here. Some clients don't need it, but it's
 * better than keeping a separate "commugly"... and "utilcomm.h".
 */
#include <commctrl.h>

/**
 * This file exists to bury all of the ugly, dangerous casting required by the
 * Windows API in one place.
 *
 * I use C-style casts because I'm not sure if C++'s reinterpret_cast is
 * guaranteed to do the same thing. (Maybe once I get my hands on Bjarne
 * Stroustrup's book...)
 *
 * All these casts have been checked against the current Windows API docs at
 * time of writing.
 */

/** Windows **/

inline HICON Window_SetIcon(HWND window, WPARAM type, HICON icon)
{
	return (HICON)SendMessageW(
		window, WM_SETICON, type, (LPARAM)icon);
}

inline LRESULT Window_SetTextW(HWND window, LPCWSTR string)
{
	return SendMessageW(window, WM_SETTEXT, 0, (LPARAM)string);
}

inline WNDPROC SetWindowWndProc(HWND window, WNDPROC newproc)
{
	return (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC,
		(LONG_PTR)newproc);
}

inline void SetDialogUserData(HWND dialog, void * data)
{
	SetWindowLongPtr(dialog, DWLP_USER, (LPARAM)data);
}

/**
 * Returns dialog's user data as a ptr. Should be used with the ptr version of
 * SetDialogUserData, of course.
 */
inline void * GetDialogUserData_ptr(HWND dialog)
{
	return (void *)GetWindowLongPtr(dialog, DWLP_USER);
}

/** Buttons **/

/**
 * This is just for convenience, no dangerous casting required.
 */
inline bool Button_IsChecked(HWND button)
{
	return (SendMessage(button, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

/** List boxes **/

inline LRESULT List_AddStringW(HWND control, LPCWSTR string)
{
	return SendMessageW(control, LB_ADDSTRING, 0, (LPARAM)string);
}

inline LRESULT List_InsertStringW(HWND control, WPARAM index, LPCWSTR string)
{
	return SendMessageW(control, LB_INSERTSTRING, index, (LPARAM)string);
}

/*
 * Get a const void * from a LB item's data. Of course, callees should only use
 * this if set with the const void * version of List_SetItemData().
 *
 * @return item data, or NULL if an error occurs
 */
inline const void * List_GetItemData_cPtr(HWND control, WPARAM index)
{
	LRESULT data = SendMessageW(control, LB_GETITEMDATA, index, 0);
	return (data != LB_ERR) ?
		reinterpret_cast<const void *>(data) : NULL;
}

inline LRESULT List_SetItemData(HWND control, WPARAM index, const void * ptr)
{
	// LPARAM is defined as a LONG_PTR by the documentation. Hopefully that
	// won't change.
	return SendMessageW(control, LB_SETITEMDATA, index,
		reinterpret_cast<LPARAM>(ptr));
}

/** Combo boxes **/

inline LRESULT Combo_AddStringA(HWND control, LPCSTR string)
{
	return SendMessageA(control, CB_ADDSTRING, 0, (LPARAM)string);
}

inline LRESULT Combo_AddStringW(HWND control, LPCWSTR string)
{
	return SendMessageW(control, CB_ADDSTRING, 0, (LPARAM)string);
}

/*
 * Get a const void * from a CB item's data. Of course, callees should only use
 * this if set with the const void * version of Combo_SetItemData().
 *
 * @return item data, or NULL if an error occurs
 */
inline const void * Combo_GetItemData_cPtr(HWND control, WPARAM index)
{
	LRESULT data = SendMessageW(control, CB_GETITEMDATA, index, 0);
	return (data != CB_ERR) ?
		reinterpret_cast<const void *>(data) : NULL;
}

inline LRESULT Combo_SetItemData(HWND control, WPARAM index, const void * ptr)
{
	// LPARAM is defined as a LONG_PTR by the documentation. Hopefully that
	// won't change.
	return SendMessageW(control, CB_SETITEMDATA, index,
		reinterpret_cast<LPARAM>(ptr));
}

/** Scroll bars **/

inline LRESULT ScrollBar_SetParts(HWND scrollbar, int parts, LPINT widths)
{
	return SendMessage(scrollbar, SB_SETPARTS, parts, (LPARAM)widths);
}

inline LRESULT ScrollBar_SetText(HWND scrollbar, int part, LPTSTR text)
{
	return SendMessage(scrollbar, SB_SETTEXT, part, (LPARAM)text);
}

/** Tree Views **/

// get rid of MS type-ignoring macro
#undef TreeView_GetNextItem
inline HTREEITEM TreeView_GetNextItem(HWND treeview, HTREEITEM item, UINT flag)
{
	return (HTREEITEM)SendMessage(
		treeview, TVM_GETNEXTITEM, flag, (LPARAM)item);
}

#undef TreeView_GetItem
inline LRESULT TreeView_GetItem(HWND treeview, TVITEM * tvitem)
{
	return SendMessage(treeview, TVM_GETITEM, 0, (LPARAM)tvitem);
}

#undef TreeView_Expand
inline LRESULT TreeView_Expand(HWND treeview, HTREEITEM item, UINT flag)
{
	return SendMessage(treeview, TVM_EXPAND, flag, (LPARAM)item);
}

/** Property Sheets **/

#undef PropSheet_IsDialogMessage
inline LRESULT PropSheet_IsDialogMessage(HWND propsheet, PMSG msg)
{
	return SendMessage(propsheet, PSM_ISDIALOGMESSAGE, 0, (LPARAM)msg);
}

#undef PropSheet_GetCurrentPageHwnd
inline HWND PropSheet_GetCurrentPageHwnd(HWND propsheet)
{
	return (HWND)SendMessage(propsheet, PSM_GETCURRENTPAGEHWND, 0, 0);
}

/** Tool tips **/

inline void ToolTip_FillIDWithHwnd(TOOLINFO &ti, HWND tool)
{
	ti.uId = (UINT_PTR)tool;
}

inline LRESULT ToolTip_AddTool(HWND tooltip, TOOLINFO * ti)
{
	return SendMessage(tooltip, TTM_ADDTOOL, 0, (LPARAM)ti);
}

#endif // INC_WINUGLY_H

