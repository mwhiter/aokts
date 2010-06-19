/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	utilui.h -- Custom controls and utility UI functions & macros.

	VIEW/CONTROLLER
**/

#include "../model/esdata.h"
#include <windows.h>
#include <commctrl.h>
#include <climits>

/* Shared msgbox strings */

const char errorOpenClipboard[] =
"Could not open clipboard. Most likely another window is currently using it.";
const char errorSetClipboard[] =
"Could not set clipboard data.";
const char errorAllocFailed[] =
"Could not allocate the memory for this operation. This could be a sign that you are out of memory, "
"but usually indicates a bug. Please contact DiGiT about this error.";
const char warningNoFormat[] =
"Specified clipboard data did not exist.\nPlease report this error to cyan.spam@gmail.com.";


/** Common functions **/

#define ENABLE_WND(id, en) EnableWindow(::GetDlgItem(dialog, id), en)

/*
	GetWindowText: Retrieves text from a window and stores it in an SString class.
*/
void GetWindowText(HWND wnd, SString &value);

void SetWindowText(HWND wnd, int value);

/** Combo Box Utility Functions **/

/**
 * Fills a combo box with the specified strings.
 */
void Combo_Fill(HWND dialog, int id, char const * * strings, size_t count);

/*
	FillCB: Fills a combobox with strings with associated data and selects one.
*/
void FillCB(HWND combobox, unsigned count, const struct PAIR *pairs, unsigned select);

/*
 * Retrieves item data from the currently-selected item in the combo box.
 */
inline LRESULT Combo_GetSelData(HWND combobox);

/** List Box Utility Functions **/

/**
 * @return the index of the item with the associated cPtr, or UINT_MAX if none
 * found.
 */
unsigned ListBox_Find(HWND listbox, const void *);

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

/** Dialog Utility Functions **/

/**
 * Convert an int to a short, since many UI functions just deal with ints.
 */
inline short toshort(int n)
{
	return (n > SHRT_MAX) ? SHRT_MAX : static_cast<short>(n);
}

/*
	GetDlgItemFloat: Return a float from a dialog box edit control.

	dialog: The owner dialog.
	id: The identifier of the control.

  Note: Text longer than 32 characters will be truncated.
*/
float GetDlgItemFloat(HWND dialog, UINT id);

/*
	SetDlgItemFloat: Set a dialog box edit control's text to a floating-point value.

	dialog: The owner dialog.
	id: The identifier of the control.
	value: The value to display.
*/
void SetDlgItemFloat(HWND dialog, UINT id, double value);

/*
	GetCheckedRadio: Returns the ID of the checked radio button from a range of radio buttons.
*/
UINT GetCheckedRadio(HWND dialog, UINT first, UINT last);

/** Common Dialog Utility Functions **/

/**
 * Presents an "Open File" dialog box to the user. The box:
 * - requires the user to select an existing file
 * - has no file type filters
 *
 * The full path is returned in /path/.
 *
 * @return true if the user clicked "OK", false otherwise
 */
bool GetOpenFileNameA(HWND owner, char * path, DWORD maxPath);

/**
 * Presents a "Save File" dialog box to the user. The box:
 * - has no file type filters
 *
 * @param owner desired owner window of the dialog box
 * @param path in/out: contains full path upon return
 * @param maxPath specifies maximum number of characters to store in path
 *
 * @return true if the user clicked "OK", false otherwise
 */
bool GetSaveFileNameA(HWND owner, char * path, DWORD maxPath);
// TODO: allow pre-filled path

#ifdef TOOLTIPS_CLASS

/* Tooltips */
void TooltipInit(HWND tt);
BOOL AddTool(HWND dialog, int ctrlId, LPTSTR text);

/* Special Edit control subclass */
LRESULT CALLBACK TVEditWndProc(HWND control, UINT msg, WPARAM wParam, LPARAM lParam);

#endif //TOOLTIPS_CLASS

/** Inline function definitions **/

LRESULT Combo_GetSelData(HWND combobox)
{
	return SendMessage(combobox, CB_GETITEMDATA,
			SendMessage(combobox, CB_GETCURSEL, 0, 0), 0);
}

const Link * LinkListBox_GetSel(HWND listbox)
{
	return LinkListBox_Get(listbox,
		SendMessage(listbox, LB_GETCURSEL, 0, 0));
}
