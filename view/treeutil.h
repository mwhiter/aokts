#ifndef INC_TREEUTIL_H
#define INC_TREEUTIL_H

/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	treeutil.h -- Utility functions for treeviews.

	VIEW/CONTROLLER
**/

#include "../util/winugly.h"

/*
	Misc Treeview Inline Funcs

  Note: These apply to all treeviews, not just this trigger tree.
*/

/**
 * Subclasses a Tree-View edit control to accept all keyboard input.
 */
void SubclassTreeEditControl(HWND control);

//Get the lParam of a specified item. If which = NULL, get the current selected item.
LPARAM GetItemParam(HWND treeview, HTREEITEM which);

/*
	TreeView_AddChild:
		Adds a child node to the specified parent with an
		lParam of param.
*/
void TreeView_AddChild(HWND treeview, LPARAM param, HTREEITEM parent, HTREEITEM after, int type);

//Get the "selected" root item.
inline HTREEITEM GetRootSel(HWND treeview)
{
	HTREEITEM selected, parent;

	selected = TreeView_GetNextItem(treeview, NULL, TVGN_CARET);
	parent = TreeView_GetNextItem(treeview, selected, TVGN_PARENT);

	return parent ? parent : selected;
}

struct BitmapIcons {
    enum Value{
        TRIG_ON_GOOD  = 0x00,
        TRIG_ON_BAD   = 0x01,
        TRIG_OFF_GOOD = 0x02,
        TRIG_OFF_BAD  = 0x03,
        LOOP_ON_GOOD  = 0x04,
        LOOP_ON_BAD   = 0x05,
        LOOP_OFF_GOOD = 0x06,
        LOOP_OFF_BAD  = 0x07,
        COND_GOOD     = 0x08,
        COND_BAD      = 0x09,
        EFFECT_GOOD   = 0x0a,
        EFFECT_BAD    = 0x0b
    };
};

#endif // INC_TREEUTIL_H
