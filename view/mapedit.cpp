/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	mapedit.cpp -- Defines functions for Map/Terrain editor.

	VIEW/CONTROLLER
**/

#include "editors.h"
#include "../resource.h"
#include "../util/settings.h"
#include "../util/MemBuffer.h"
#include "LCombo.h"
#include "LinkListBox.h"
#include "mapview.h"
#include "utilui.h"

/*
  propdata.sel0 = Current tile x
  propdata.sel1 = Current tile y
*/

#define NUM_SIZES 8
#define NUM_ELEVS 9

enum CLICK_STATES
{
	CLICK_Default,	//just selects tile on map
	CLICK_MCSet1,	//sets x,y coords for map copy rect from1
	CLICK_MCSet2,	//sets x,y coords for map copy rect from2
	CLICK_MCSetT,	//sets x,y coords for map copy rect to
	CLICK_MMSet1,	//sets x,y coords for map move rect from1
	CLICK_MMSet2,	//sets x,y coords for map move rect from2
	CLICK_MMSetT	//sets x,y coords for map move rect to
} click_state = CLICK_Default;

const char *szMapTitle = "Map Editor";
const char *szMapStatus =
"Click \"Set...\" and then a point on the map to automatically fill values.";

const char *sizes[NUM_SIZES] =
{
	"Tiny (120)", "Small (144)", "Medium (168)", "Normal (200)", "Large (220)", "Giant (240)", "Max (256)", "Ludks (480)"
};
const char *elevs[NUM_ELEVS] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };

const char *warningExceededMaxSize =
"Sorry, you have exceeded the maximum allowed mapsize. I will now set the mapsize to maximum.";
const char *warningSensibleRect =
"Please make a sensible source rectangle. I fixed it for you this time, but\n"
"don\'t expect me to be as nice in the future...";
const char *warningMapCopyOverlap =
"Sorry, I had to cancel your map copy because the source and destination areas\n"
"overlapped. This may be supported in the future.";
const char *infoMapCopySuccess =
"Map copy successful. (As far as I know... as always, check for bugs!)";

void LoadMap(HWND dialog, bool all)
{
	Map::Terrain *tn = &scen.map.terrain[propdata.sel0][propdata.sel1];
	const Link *ait;
	int index = -1, i;

	SendDlgItemMessage(dialog, IDC_TR_ID, LB_SETCURSEL, tn->cnst, 0);
	SendDlgItemMessage(dialog, IDC_TR_ELEV, CB_SETCURSEL, tn->elev, 0);

	if (!all)
		return;

	/* See if selected size is one of the built-ins. */
	for (i = 0; i < NUM_SIZES; i++)
	{
		if (scen.map.x == MapSizes[i])
		{
			SendDlgItemMessage(dialog, IDC_TR_SIZE, CB_SETCURSEL, i, 0);
			break;
		}
	}
	if (i == NUM_SIZES)
		SetDlgItemInt(dialog, IDC_TR_SIZE, scen.map.x, FALSE);
	for (i = 0; i < NUM_SIZES; i++)
	{
		if (scen.map.y == MapSizes[i])
		{
			SendDlgItemMessage(dialog, IDC_TR_SIZE2, CB_SETCURSEL, i, 0);
			break;
		}
	}
	if (i == NUM_SIZES)
		SetDlgItemInt(dialog, IDC_TR_SIZE2, scen.map.y, FALSE);

	ait = esdata.aitypes.getByIdSafe(scen.map.aitype);

	LCombo_Select(dialog, IDC_TR_AITYPE, ait);
}

void SaveMap(HWND dialog)
{
	int w,h;
	Map::Terrain *tn = scen.map.terrain[propdata.sel0] + propdata.sel1;

	//First check standard sizes. If that fails, get the custom size.
	if ((w = SendDlgItemMessage(dialog, IDC_TR_SIZE, CB_GETCURSEL, 0, 0)) != LB_ERR)
		scen.map.x = MapSizes[w];
	else
		scen.map.x = GetDlgItemInt(dialog, IDC_TR_SIZE, NULL, FALSE);

	if ((h = SendDlgItemMessage(dialog, IDC_TR_SIZE2, CB_GETCURSEL, 0, 0)) != LB_ERR)
		scen.map.y = MapSizes[h];
	else
		scen.map.y = GetDlgItemInt(dialog, IDC_TR_SIZE2, NULL, FALSE);

	if (scen.map.x > MAX_MAPSIZE || scen.map.y > MAX_MAPSIZE)
	{
	    if (scen.map.x > MAX_MAPSIZE)
		    scen.map.x = MAX_MAPSIZE;
	    if (scen.map.y > MAX_MAPSIZE)
		    scen.map.y = MAX_MAPSIZE;
		MessageBox(dialog, warningExceededMaxSize, szMapTitle, MB_ICONWARNING);
	}

	//scen.map.y = scen.map.x;	//maps are square

	scen.map.aitype = LCombo_GetSelId(dialog, IDC_TR_AITYPE);
	tn->cnst = static_cast<char>(
		LinkListBox_GetSel(GetDlgItem(dialog, IDC_TR_ID))->id());
	tn->elev = static_cast<char>(
		SendDlgItemMessage(dialog, IDC_TR_ELEV, CB_GETCURSEL, 0, 0));
}

void Map_UpdatePos(HWND dialog, WORD id)
{
	unsigned int value = GetDlgItemInt(dialog, id, NULL, FALSE);
	SendMessage(propdata.mapview, MAP_UnhighlightPoint,
		MAP_UNHIGHLIGHT_ALL, 0);

	if (value < scen.map.x)	//map is square, so we can compare to just map.x
	{
		if (propdata.sel0 >= 0 && propdata.sel1 >= 0)
			SaveMap(dialog);

		if (id == IDC_TR_TX)
			propdata.sel0 = value;
		else
			propdata.sel1 = value;

		SendMessage(propdata.mapview, MAP_HighlightPoint, propdata.sel0, propdata.sel1);
		LoadMap(dialog, false);
	}
}

void Map_HandleMapClick(HWND dialog, short x, short y)
{
	int ctrlx, ctrly;

	switch (click_state)
	{
	case CLICK_Default:
		ctrlx = IDC_TR_TX;
		ctrly = IDC_TR_TY;
		break;
	case CLICK_MCSet1:
		ctrlx = IDC_TR_MCX1;
		ctrly = IDC_TR_MCY1;
		click_state = CLICK_Default;
		break;
	case CLICK_MCSet2:
		ctrlx = IDC_TR_MCX2;
		ctrly = IDC_TR_MCY2;
		click_state = CLICK_Default;
		break;
	case CLICK_MCSetT:
		ctrlx = IDC_TR_MCXT;
		ctrly = IDC_TR_MCYT;
		click_state = CLICK_Default;
		break;
	case CLICK_MMSet1:
		ctrlx = IDC_TR_MMX1;
		ctrly = IDC_TR_MMY1;
		click_state = CLICK_Default;
		break;
	case CLICK_MMSet2:
		ctrlx = IDC_TR_MMX2;
		ctrly = IDC_TR_MMY2;
		click_state = CLICK_Default;
		break;
	case CLICK_MMSetT:
		ctrlx = IDC_TR_MMXT;
		ctrly = IDC_TR_MMYT;
		click_state = CLICK_Default;
		break;
	default:
		return;
	}

	SetDlgItemInt(dialog, ctrlx, x, FALSE);
	SetDlgItemInt(dialog, ctrly, y, FALSE);
}

void Map_HandleMapCopy(HWND dialog)
{
	RECT source;
	int space;
	long temp;
	MapCopyCache *cache = NULL;
	void *clipboardData;
	HGLOBAL clipboardMem;
	bool disp = false;

	if (!OpenClipboard(dialog))
	{
		MessageBox(dialog, errorOpenClipboard, szMapTitle, MB_ICONWARNING);
		return;
	}

	/* Get the source rect */
	source.left   = GetDlgItemInt(dialog, IDC_TR_MCX1, NULL, FALSE);
	source.bottom    = GetDlgItemInt(dialog, IDC_TR_MCY1, NULL, FALSE);	//top and bottom are "normal", reverse from GDI standard
	source.right  = GetDlgItemInt(dialog, IDC_TR_MCX2, NULL, FALSE);
	source.top = GetDlgItemInt(dialog, IDC_TR_MCY2, NULL, FALSE);

	/* We need to make sure it's a sane rectangle. */
	if (source.left > source.right)
	{
		temp = source.left;
		source.left = source.right;
		source.right = temp;
		disp = true;
	}
	if (source.bottom > source.top)
	{
		temp = source.top;
		source.top = source.bottom;
		source.bottom = temp;
		disp = true;
	}
	if (disp)
	{
		MessageBox(dialog, warningSensibleRect, szMapTitle, MB_ICONWARNING);
	}

	EmptyClipboard();

	/* allocate the clipboard memory */
	space = scen.map_size(source, cache);
	clipboardMem = GlobalAlloc(GMEM_MOVEABLE, space);

	if (clipboardMem == NULL)
	{
		MessageBox(dialog, errorAllocFailed, szMapTitle, MB_ICONWARNING);
		goto Cleanup;
	}

	/* actually write the data */
	clipboardData = GlobalLock(clipboardMem);
	if (clipboardData)
	{
		MemBuffer data((char*)clipboardData, space);
		scen.map_copy(data, cache);
		cache = NULL;
	}
	GlobalUnlock(clipboardMem);

	if (!SetClipboardData(propdata.mcformat, clipboardMem))
		MessageBox(dialog, errorSetClipboard, szMapTitle, MB_ICONWARNING);

	EnableWindow(GetDlgItem(dialog, IDC_TR_MCPASTE), true);

Cleanup:
	CloseClipboard();
}

void Map_HandleMapPaste(HWND dialog)
{
	HGLOBAL cbMem;
	POINT target;
	void *cbData;
	size_t size;

	target.x = GetDlgItemInt(dialog, IDC_TR_MCXT, NULL, FALSE);
	target.y = GetDlgItemInt(dialog, IDC_TR_MCYT, NULL, FALSE);

	/*	Check whether map copy data is actually available. (Should be if we've
		gotten this far.
	*/
	if (!IsClipboardFormatAvailable(propdata.mcformat))
	{
		MessageBox(dialog, warningNoFormat, szMapTitle, MB_ICONWARNING);
		return;
	}

	if (!OpenClipboard(dialog))
		MessageBox(dialog, errorOpenClipboard, szMapTitle, MB_ICONWARNING);

	cbMem = GetClipboardData(propdata.mcformat);
	size = GlobalSize(cbMem);
	cbData = GlobalLock(cbMem);
	if (cbData)
	{
		MemBuffer b((char*)cbData, size);

		scen.map_paste(target, b);
	}
	else
		MessageBox(dialog, "Could not paste clipboard data.",
			szMapTitle, MB_ICONWARNING);
	GlobalUnlock(cbMem);

	CloseClipboard();

	SendMessage(propdata.mapview, MAP_Reset, 0, 0);
}

void Map_HandleMapMove(HWND dialog)
{
	bool disp = false;
	RECT source;
	POINT target;
	long temp;

	/* Get the source rect */
	source.left   = GetDlgItemInt(dialog, IDC_TR_MMX1, NULL, FALSE);
	source.bottom    = GetDlgItemInt(dialog, IDC_TR_MMY1, NULL, FALSE);	//top and bottom are "normal", reverse from GDI standard
	source.right  = GetDlgItemInt(dialog, IDC_TR_MMX2, NULL, FALSE);
	source.top = GetDlgItemInt(dialog, IDC_TR_MMY2, NULL, FALSE);

	/* Get the target point */
	target.x = GetDlgItemInt(dialog, IDC_TR_MMXT, NULL, FALSE);
	target.y = GetDlgItemInt(dialog, IDC_TR_MMYT, NULL, FALSE);

	/* We need to make sure it's a sane rectangle. */
	if (source.left > source.right)
	{
		temp = source.left;
		source.left = source.right;
		source.right = temp;
		disp = true;
	}
	if (source.bottom > source.top)
	{
		temp = source.top;
		source.top = source.bottom;
		source.bottom = temp;
		disp = true;
	}
	if (disp) {
		MessageBox(dialog, warningSensibleRect, szMapTitle, MB_ICONWARNING);
	} else {
	    scen.map_move(source, target);
	}

	SendMessage(propdata.mapview, MAP_Reset, 0, 0);
}

void Map_HandleSetFocus(HWND, WORD)
{
	EnableMenuItem(propdata.menu, ID_EDIT_COPY, MF_ENABLED);
	EnableMenuItem(propdata.menu, ID_EDIT_CUT, MF_ENABLED);
	if (IsClipboardFormatAvailable(CF_TEXT))
		EnableMenuItem(propdata.menu, ID_EDIT_PASTE, MF_ENABLED);
}

void Map_HandleKillFocus(HWND dialog, WORD id)
{
	EnableMenuItem(propdata.menu, ID_EDIT_COPY, MF_GRAYED);
	EnableMenuItem(propdata.menu, ID_EDIT_CUT, MF_GRAYED);
	EnableMenuItem(propdata.menu, ID_EDIT_PASTE, MF_GRAYED);

	switch (id)
	{
	case IDC_TR_MCX1:
	case IDC_TR_MCY1:
	case IDC_TR_MCX2:
	case IDC_TR_MCY2:
	case IDC_TR_MCXT:
	case IDC_TR_MCYT:
		SendMessage(propdata.mapview, MAP_UnhighlightPoint, MAP_UNHIGHLIGHT_ALL, 0);
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_TR_MCX1, NULL, FALSE),
			GetDlgItemInt(dialog, IDC_TR_MCY1, NULL, FALSE));
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_TR_MCX2, NULL, FALSE),
			GetDlgItemInt(dialog, IDC_TR_MCY2, NULL, FALSE));
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_TR_MCXT, NULL, FALSE),
			GetDlgItemInt(dialog, IDC_TR_MCYT, NULL, FALSE));
		break;
	case IDC_TR_MMX1:
	case IDC_TR_MMY1:
	case IDC_TR_MMX2:
	case IDC_TR_MMY2:
	case IDC_TR_MMXT:
	case IDC_TR_MMYT:
		SendMessage(propdata.mapview, MAP_UnhighlightPoint, MAP_UNHIGHLIGHT_ALL, 0);
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_TR_MMX1, NULL, FALSE),
			GetDlgItemInt(dialog, IDC_TR_MMY1, NULL, FALSE));
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_TR_MMX2, NULL, FALSE),
			GetDlgItemInt(dialog, IDC_TR_MMY2, NULL, FALSE));
		SendMessage(propdata.mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_TR_MMXT, NULL, FALSE),
			GetDlgItemInt(dialog, IDC_TR_MMYT, NULL, FALSE));
		break;
	}
}

void Map_HandleCommand(HWND dialog, WORD code, WORD id, HWND)
{
	switch (code)
	{
	case EN_CHANGE:
		switch (id)
		{
		case IDC_TR_TX:
		case IDC_TR_TY:
			Map_UpdatePos(dialog, id);
			break;

		case ID_EDIT_COPY:
			SendMessage(GetFocus(), WM_COPY, 0, 0);
			break;

		case ID_EDIT_CUT:
			SendMessage(GetFocus(), WM_CUT, 0, 0);
			break;

		case ID_EDIT_PASTE:
			SendMessage(GetFocus(), WM_PASTE, 0, 0);
			break;
		}
		break;

	case BN_CLICKED:
		switch (id)
		{
		case IDC_TR_MCSET1:
			click_state = CLICK_MCSet1;
			break;

		case IDC_TR_MCSET2:
			click_state = CLICK_MCSet2;
			break;

		case IDC_TR_MCSETT:
			click_state = CLICK_MCSetT;
			break;

		case IDC_TR_MMSET1:
			click_state = CLICK_MMSet1;
			break;

		case IDC_TR_MMSET2:
			click_state = CLICK_MMSet2;
			break;

		case IDC_TR_MMSETT:
			click_state = CLICK_MMSetT;
			break;

		case IDC_TR_MCCOPY:
			Map_HandleMapCopy(dialog);
			break;

		case IDC_TR_MCPASTE:
			Map_HandleMapPaste(dialog);
			break;

		case IDC_TR_MMMOVE:
			Map_HandleMapMove(dialog);
			break;
		}

	case EN_SETFOCUS:
		Map_HandleSetFocus(dialog, id);
		break;

	case EN_KILLFOCUS:
		Map_HandleKillFocus(dialog, id);
		break;
	}
}

void Map_Reset(HWND dialog)
{
	propdata.sel0 = -1;
	propdata.sel1 = -1;
	SetDlgItemInt(dialog, IDC_TR_TX, 0, FALSE);
	SetDlgItemInt(dialog, IDC_TR_TY, 0, FALSE);
	LoadMap(dialog, true);
	SetWindowText(propdata.statusbar, szMapStatus);
}

INT_PTR CALLBACK MapDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;	//default: doesn't process message

	try
	{
		switch (msg)
		{
			case WM_INITDIALOG:
				{
					LinkListBox_Fill(
						GetDlgItem(dialog, IDC_TR_ID), esdata.terrains.head());

					LCombo_Fill(dialog, IDC_TR_AITYPE, esdata.aitypes.head());
					Combo_Fill(dialog, IDC_TR_SIZE, sizes, NUM_SIZES);
					Combo_Fill(dialog, IDC_TR_SIZE2, sizes, NUM_SIZES);
					Combo_Fill(dialog, IDC_TR_ELEV, elevs, NUM_ELEVS);

					ret = TRUE;
				}
				break;

			case WM_COMMAND:
				Map_HandleCommand(
						dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
				break;

			case WM_NOTIFY:
				{
					NMHDR *header = (NMHDR*)lParam;
					switch (header->code)
					{
						case PSN_SETACTIVE:
							Map_Reset(dialog);
							break;
						case PSN_KILLACTIVE:
							SaveMap(dialog);
							break;
					}

					ret = TRUE;
				}
				break;

			case MAP_Click:
				Map_HandleMapClick(dialog, LOWORD(lParam), HIWORD(lParam));
				break;

			case AOKTS_Loading:
				Map_Reset(dialog);
				break;

			case AOKTS_Saving:
				SaveMap(dialog);
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
