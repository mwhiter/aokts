/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	trigtextview.cpp -- Allows you to read and search trigtext.

	VIEW/CONTROLLER
**/

#include "editors.h"
#include "../res/resource.h"
#include "../util/settings.h"
#include "../model/scen.h"
#include "../model/TrigXmlVisitor.h"
#include "../model/TrigXmlReader.h"
#include "utilui.h"
#include <commdlg.h>
#include <cstring>

void OnFileTrigWrite(HWND dialog);

std::string tt = std::string("");

const int context_size = 8000;
int bottom;
int top;
unsigned long current_pos;

/** Message-processing Functions **/

void LoadTrigtextView(HWND dialog)
{
    std::ostringstream ss;
	scen.accept(TrigXmlVisitor(ss));
	tt = std::string("");
	tt.append(ss.str());

	current_pos = 0;

	std::string snippet = tt.substr (0,context_size);
    //SetDlgItemText(dialog, IDC_TT_VIEW, ss.str().c_str());
    SetDlgItemText(dialog, IDC_TT_VIEW, snippet.c_str());
}

void TrigtextView_Reset(HWND dialog)
{
    LoadTrigtextView(dialog);
}

void TrigtextFind(HWND dialog)
{
    char searchtext[100];
    int found_length = strlen(searchtext);
    GetDlgItemText(dialog, IDC_TT_SEARCHTEXT, searchtext, 100);

    unsigned long result = tt.find(searchtext, current_pos);

    if (result == tt.npos) {
        result = tt.find(searchtext, 0); // search again from top
    }

    if (result != tt.npos) {
        current_pos = result + 1; // update current pos
        found_length = strlen(searchtext);
    } else {
        current_pos = 0; // No result for sure. Just go to top
        found_length = 0;
    }

    bottom = result - context_size / 2;
    top = result + context_size / 2;
    if (bottom < 0)
        bottom = 0;
    if (top > (int)tt.length())
        top = tt.length();

	std::string snippet = tt.substr (bottom,context_size);
    SetDlgItemText(dialog, IDC_TT_VIEW, snippet.c_str());

    std::ostringstream ss2;
    ss2 << result;
    //MessageBox(dialog, ss2.str().c_str(), "Sup", MB_ICONERROR);

    HWND tt_view = GetDlgItem(dialog, IDC_TT_VIEW);

    SendMessage(tt_view,EM_SETSEL,result - bottom,result - bottom + found_length);

    SendMessage(tt_view,EM_SCROLLCARET,0,0);
}

/* Map Inspector */

/**
 * Handles a WM_COMMAND message sent to the dialog.
 */
INT_PTR Trigtext_HandleCommand(HWND dialog, WORD code, WORD id, HWND)
{
	HWND treeview = GetDlgItem(dialog, IDC_T_TREE);	//all use this

	switch (code) {
	case EN_CHANGE:
		switch (id)
		{
		case IDC_TT_SEARCHTEXT:
		    TrigtextFind(dialog);
		    break;
		}

	case BN_CLICKED:
		switch (id)
		{

	    case IDC_TT_EXPORT:
		    OnFileTrigWrite(dialog);
		    break;

	    case IDC_TT_FINDNEXT:
		    TrigtextFind(dialog);
		    break;

	/* switch (code)

		case IDC_P_TOUP:
			scen.hd_to_up();
			break;

             */
		}
	}

	// "If an application processes this message, it should return zero."
	return 0;
}

INT_PTR CALLBACK TrigtextDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	try
	{
	switch (msg)
	{
	    case WM_INITDIALOG:
	        {
                // This doesn't seem to be the reason the newlines do
                // not appear on xp
	            //HWND control = GetDlgItem(dialog, IDC_TT_VIEW);
	            //SendMessage(control, EM_SETLIMITTEXT, 256, 0);
	            //SendMessage(control, EM_SETLIMITTEXT, 0, 0);
	            //SendMessage(control, EM_SETLIMITTEXT, 200000, 0);

	            HWND control = GetDlgItem(dialog, IDC_TT_SEARCHTEXT);
	            SendMessage(control, EM_SETLIMITTEXT, 100, 0);
	        }
	        break;

		case WM_COMMAND:
			return Trigtext_HandleCommand(
					dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);

		case WM_NOTIFY:
			{
				NMHDR *header = (NMHDR*)lParam;
				switch (header->code)
				{
				case PSN_SETACTIVE:
					TrigtextView_Reset(dialog);
					break;

				case PSN_KILLACTIVE:
					break;
				}
			}
			break;

		case AOKTS_Loading:
			TrigtextView_Reset(dialog);
			break;

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
