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
void TreeView_AddChild(HWND treeview, LPARAM param, HTREEITEM parent, HTREEITEM after);

//Get the "selected" root item.
inline HTREEITEM GetRootSel(HWND treeview)
{
	HTREEITEM selected, parent;

	selected = TreeView_GetNextItem(treeview, NULL, TVGN_CARET);
	parent = TreeView_GetNextItem(treeview, selected, TVGN_PARENT);

	return parent ? parent : selected;
}

#endif // INC_TREEUTIL_H
