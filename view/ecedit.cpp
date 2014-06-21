/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	ecedit.cpp -- Editors for effects and conditions.

	VIEW/CONTROLLER
**/

#include "../util/settings.h"
#include "../resource.h"
#include "ecedit.h"
#include "utilui.h"
#include "LCombo.h"
#include "utilunit.h"
#include "mapview.h"
#include "../util/winugly.h"
#include <stdio.h>

/** Shared **/

const wchar_t *noselect = L"<none>";

const char errorNoData[] =
"No Data! Functionality will be impaired.";

/* Editor classes */

EditEffect::EditEffect(Effect &source)
:	e(source)
{}

void EditEffect::update(Trigger *t)
{
	t->effects[index] = e;
}

void EditCondition::update(Trigger *t)
{
	t->conds[index] = c;
}

/** Effect Editor **/

const char warnWeirdResource[] =
"The resource you selected is non-standard and may have unpredictable consequences.";

/*
	MakeUIDString: Fills buffer with a comma-delimited list of UIDs in e.
*/
void MakeUIDString(UID *list, int count, HWND dest)
{
	char buffer[0x30];
	char *pos = buffer;
	int ret;

	for (int i = 0; i < count; i++)
	{
		if (i)
			*pos++ = ',';

		/* this is pretty ugly here, but at least it doesn't crash */
		ret = _snprintf(pos, sizeof(buffer) - (pos - buffer + 1), "%u", list[i]);

		if (ret == -1)
			break;

		pos += ret;
	}
	*pos = '\0';

	SetWindowText(dest, buffer);
}

inline void setloc(HWND dialog, bool point)
{
	ENABLE_WND(IDC_E_LOCX, point);
	ENABLE_WND(IDC_E_LOCY, point);
	ENABLE_WND(IDC_E_LOCUID, !point);
	ENABLE_WND(IDC_E_OPENSEL2, !point);
}

/* E/C control special identifiers */
#define ECC_AREA -1
#define ECC_UNITSEL -2	//as in UCNST, class, and type
#define ECC_LOCATION -3	//effect only
#define ECC_NOPLAYER -4	//effect only

/* controls table: identifies which controls each effect type uses */
const short ectrls[NUM_EFFECTS][5] =
{	//
	{ ECC_NOPLAYER },		//0 = No Effect
	{ IDC_E_TPLAY, IDC_E_DSTATE },
	{ IDC_E_RESEARCH },
	{ IDC_E_SOUND, IDC_E_TEXT },
	/* 0x4 = Play Sound */
	{ IDC_E_SOUND },
	{ IDC_E_TPLAY, IDC_E_AMOUNT, IDC_E_RESTYPE },
	{ IDC_E_UIDS, ECC_NOPLAYER },
	{ IDC_E_UIDS, ECC_NOPLAYER },
	/* 0x8 = Activate Trigger */
	{ IDC_E_TRIG, ECC_NOPLAYER },
	{ IDC_E_TRIG, ECC_NOPLAYER },
	{ IDC_E_AIGOAL },
	{ ECC_LOCATION, IDC_E_UCNST },
	/* 0xC = Task Object */
	{ ECC_LOCATION, IDC_E_UIDS, ECC_UNITSEL, ECC_AREA },
	{ 0 },
	{ ECC_UNITSEL, IDC_E_UIDS, ECC_AREA },
	{ ECC_UNITSEL, IDC_E_UIDS, ECC_AREA },
	/* 0x10 = Change View */
	{ ECC_LOCATION },
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_LOCATION, ECC_AREA },
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_AREA, IDC_E_TPLAY },
	{ IDC_E_UIDS, ECC_LOCATION, ECC_NOPLAYER },
	/* 0x14 = Display Instructions */
	{ IDC_E_SOUND, IDC_E_TEXT, IDC_E_PANEL, IDC_E_DTIME, ECC_NOPLAYER },
	{ IDC_E_PANEL, ECC_NOPLAYER },
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_AREA },
	{ 0 },
	/* 0x18 = Damage Object */
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_AREA, IDC_E_AMOUNT },
	{ IDC_E_UCNST, ECC_LOCATION },
	/* 0x1A = Change Object Name */
	{ IDC_E_UIDS, IDC_E_TEXT, ECC_AREA },
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_AREA, IDC_E_AMOUNT },
	/* 0x1C = Change Object Attack */
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_AREA, IDC_E_AMOUNT },
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_AREA },
	{ ECC_LOCATION },
	{ 0 },
	/* 0x20 = Enable Tech */
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_AREA, IDC_E_AMOUNT },
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_AREA },
	{ ECC_LOCATION },
	{ 0 },
	/* 0x24 = Flash Objects */
	{ IDC_E_UIDS, ECC_UNITSEL, ECC_AREA },
};

void EffectControls(HWND dialog, int type)
{
	HWND control;
	int id;

	//disable all controls except source player
	for (id = IDC_E_SOUND; id <= IDC_E_UTYPE; id++)
		ENABLE_WND(id, false);

	ENABLE_WND(IDC_E_SPLAY, true);

	const short *controls = ectrls[type];
	int count = 5;

	//re-enable those that are needed
	while (count-- && (id = *controls++))
	{
		switch (id)
		{
		case ECC_NOPLAYER:
			ENABLE_WND(IDC_E_SPLAY, false);
			break;

		case ECC_AREA:
			ENABLE_WND(IDC_E_AREAX1, true);
			ENABLE_WND(IDC_E_AREAY1, true);
			ENABLE_WND(IDC_E_AREAX2, true);
			ENABLE_WND(IDC_E_AREAY2, true);
			break;

		case ECC_LOCATION:
			ENABLE_WND(IDC_E_LOCX, true);
			ENABLE_WND(IDC_E_LOCY, true);
			ENABLE_WND(IDC_E_LOCM, true);
			break;

		case ECC_UNITSEL:
			ENABLE_WND(IDC_E_UCNST, true);
			ENABLE_WND(IDC_E_GROUP, true);
			ENABLE_WND(IDC_E_UTYPE, true);
			break;

		case IDC_E_TEXT:
			ENABLE_WND(IDC_E_UCNST, true);
			ENABLE_WND(IDC_E_GROUP, true);
			ENABLE_WND(IDC_E_STRINGID, true);	//both go with Display Instructions

		default:
			ENABLE_WND(id, true);
		}

		if (id == IDC_E_UIDS)
			ENABLE_WND(IDC_E_OPENSEL, true);	//IDC_E_OPENSEL modifies IDC_E_UIDS, so...
	}

	control = GetDlgItem(dialog, IDC_E_TEXT);
	if (type == EFFECT_SendChat)
		SendMessage(control, EM_SETLIMITTEXT, 64, 0);
	else if (type == EFFECT_DisplayInstructions)
		SendMessage(control, EM_SETLIMITTEXT, 256, 0);
}

const char *dnames[3] = { "Ally", "Neutral", "Enemy" };
const char *pnames[3] = { "Pan. 0", "Pan. 1", "Pan. 2" };

void E_Init(HWND dialog)
{
	Combo_Fill(dialog, IDC_E_TYPE, Effect::types, NUM_EFFECTS);

	Combo_Fill(dialog, IDC_E_SPLAY, players_ec, EC_NUM_PLAYERS);
	Combo_Fill(dialog, IDC_E_TPLAY, players_ec, EC_NUM_PLAYERS);
	Combo_Fill(dialog, IDC_E_DSTATE, dnames, 3);
	Combo_Fill(dialog, IDC_E_PANEL, pnames, 3);
	LCombo_Fill(dialog, IDC_E_RESEARCH, esdata.techs.head());
	LCombo_Fill(dialog, IDC_E_RESTYPE, esdata.resources.head());
	Combo_PairFill(GetDlgItem(dialog, IDC_E_GROUP), NUM_GROUPS, groups);
	Combo_PairFill(GetDlgItem(dialog, IDC_E_UTYPE), NUM_UTYPES, utypes);
	LCombo_Fill(dialog, IDC_E_UCNST, esdata.units.head(), noselect);
}

void LoadEffect(HWND dialog, EditEffect *data)
{
	Effect *e = &data->e;

	// Refresh trigger combo box.
	SendMessageW(GetDlgItem(dialog, IDC_E_TRIG), CB_RESETCONTENT, 0, 0);
	data->TrigCallback(GetDlgItem(dialog, IDC_E_TRIG), e->trig_index);

	SendDlgItemMessage(dialog, IDC_E_TYPE, CB_SETCURSEL, e->type, 0);
	SetDlgItemText(dialog, IDC_E_SOUND, e->sound.c_str());
	SendDlgItemMessage(dialog, IDC_E_PANEL, CB_SETCURSEL, e->panel, 0);
	SetDlgItemText(dialog, IDC_E_TEXT, e->text.c_str());
	SetDlgItemInt(dialog, IDC_E_DTIME, e->disp_time, TRUE);
	SetDlgItemInt(dialog, IDC_E_STRINGID, e->stringid, TRUE);
	SendDlgItemMessage(dialog, IDC_E_SPLAY, CB_SETCURSEL, e->s_player, 0);
	SendDlgItemMessage(dialog, IDC_E_TPLAY, CB_SETCURSEL, e->t_player, 0);
	SendDlgItemMessage(dialog, IDC_E_DSTATE, CB_SETCURSEL, e->diplomacy, 0);
	SetDlgItemInt(dialog, IDC_E_LOCX, e->location.x, TRUE);
	SetDlgItemInt(dialog, IDC_E_LOCY, e->location.y, TRUE);

	if (e->num_sel > 0)
		MakeUIDString(e->uids, e->num_sel, GetDlgItem(dialog, IDC_E_UIDS));
	else
		SetDlgItemText(dialog, IDC_E_UIDS, "");

	SetDlgItemInt(dialog, IDC_E_LOCUID, e->uid_loc, TRUE);
	SetDlgItemInt(dialog, IDC_E_AREAX1, e->area.left, TRUE);
	SetDlgItemInt(dialog, IDC_E_AREAY1, e->area.bottom, TRUE);
	SetDlgItemInt(dialog, IDC_E_AREAX2, e->area.right, TRUE);
	SetDlgItemInt(dialog, IDC_E_AREAY2, e->area.top, TRUE);
	SetDlgItemInt(dialog, IDC_E_AIGOAL, e->ai_goal, TRUE);
	LCombo_Select(dialog, IDC_E_UCNST, e->pUnit);
	Combo_SelectByData(GetDlgItem(dialog, IDC_E_GROUP), e->group);
	Combo_SelectByData(GetDlgItem(dialog, IDC_E_UTYPE), e->utype);
	LCombo_Select(dialog, IDC_E_RESEARCH, e->pTech);
	SetDlgItemInt(dialog, IDC_E_AMOUNT, e->amount, TRUE);
	LCombo_SelById(dialog, IDC_E_RESTYPE, e->res_type);
	//trig_index selected above
	SendDlgItemMessage(dialog, IDC_E_LOCM, BM_SETCHECK,
		(e->uid_loc != -1) ? BST_CHECKED : BST_UNCHECKED, 0);

	if (e->u2 != -1)
		MessageBox(dialog, "OMG! An unknown had a meaningful value! You must report this!",
			"Effect load", MB_OK);
}

void SaveEffect(HWND dialog, EditEffect *data)
{
	Effect *e = &data->e;

	e->type =	SendDlgItemMessage(dialog, IDC_E_TYPE, CB_GETCURSEL, 0, 0);
	GetWindowText(GetDlgItem(dialog, IDC_E_SOUND), e->sound);
	e->panel =	SendDlgItemMessage(dialog, IDC_E_PANEL, CB_GETCURSEL, 0, 0);
	GetWindowText(GetDlgItem(dialog, IDC_E_TEXT), e->text);
	e->disp_time =	GetDlgItemInt(dialog, IDC_E_DTIME, NULL, TRUE);
	e->pUnit = (UnitLink*)LCombo_GetSelPtr(dialog, IDC_E_UCNST);
	e->stringid = GetDlgItemInt(dialog, IDC_E_STRINGID, NULL, TRUE);
	e->s_player = SendDlgItemMessage(dialog, IDC_E_SPLAY, CB_GETCURSEL, 0, 0);
	e->t_player = SendDlgItemMessage(dialog, IDC_E_TPLAY, CB_GETCURSEL, 0, 0);
	e->diplomacy = (enum Diplomacy)SendDlgItemMessage(dialog, IDC_E_DSTATE, CB_GETCURSEL, 0, 0);
	e->area.left = GetDlgItemInt(dialog, IDC_E_AREAX1, NULL, TRUE);
	e->area.bottom = GetDlgItemInt(dialog, IDC_E_AREAY1, NULL, TRUE);
	e->area.right = GetDlgItemInt(dialog, IDC_E_AREAX2, NULL, TRUE);
	e->area.top = GetDlgItemInt(dialog, IDC_E_AREAY2, NULL, TRUE);
	e->pTech = (TechLink*)LCombo_GetSelPtr(dialog, IDC_E_RESEARCH);
	e->ai_goal = GetDlgItemInt(dialog, IDC_E_AIGOAL, NULL, TRUE);
	e->amount = GetDlgItemInt(dialog, IDC_E_AMOUNT, NULL, TRUE);
	e->res_type = LCombo_GetSelId(dialog, IDC_E_RESTYPE);

	if (IsDlgButtonChecked(dialog, IDC_E_LOCM) == BST_CHECKED)
	{
		e->uid_loc = GetDlgItemInt(dialog, IDC_E_LOCUID, NULL, TRUE);
		e->location.x = -1;
		e->location.y = -1;
	}
	else
	{
		e->uid_loc = (unsigned)-1;
		e->location.x = GetDlgItemInt(dialog, IDC_E_LOCX, NULL, TRUE);
		e->location.y = GetDlgItemInt(dialog, IDC_E_LOCY, NULL, TRUE);
	}

	//get the data, not the index, for these
	e->trig_index = Combo_GetSelData(GetDlgItem(dialog, IDC_E_TRIG));
	e->group = Combo_GetSelData(GetDlgItem(dialog, IDC_E_GROUP));
	e->utype = Combo_GetSelData(GetDlgItem(dialog, IDC_E_UTYPE));
}

/*	Should be:

	Patrol, Task Object, Remove Object, Rename, Change HP, Change Attack, Freeze Unit,
	Stop Unit, Kill Object, Damage Object.
*/
const bool multiSelEffect[] =
{
	false,
	false,
	false,
	false,
	false,
	false,
	false, //maybe, UnlockGate
	false, //maybe, LockGate
	false,
	false,
	false,
	false,
	true,  //TaskObject
	false,
	true,  //KillObject
	true,  //RemoveObject
	false,
	false,
	false,
	true,  //Patrol
	false,
	false,
	true,  //FreezeUnit
	false,
	true,  //DamageObject
	false,
	true,  //ChangeObjectName
	true,  //ChangeObjectHP
	true,  //ChangeObjectAttack
	true,  //StopUnit
	false,
	false,
	false,
	false,
	false,
	false,
	false
};

void OnOpenSel(HWND dialog, EditEffect *data)
{
	INT_PTR ret;
	struct UnitEdit ue =
	{ data->players, data->e.s_player, data->e.num_sel };
	if (data->e.num_sel > 0)
		memcpy(ue.ids, data->e.uids, sizeof(UID) * data->e.num_sel);

	ret = UnitSelDialogBox(
		GetModuleHandle(NULL), dialog, ue, multiSelEffect[data->e.type]);

	if (ret)
	{
		data->e.s_player = ue.player;
		data->e.num_sel = ue.count;
		memcpy(data->e.uids, ue.ids, sizeof(UID) * ue.count);

		/* Update controls */
		MakeUIDString(data->e.uids, data->e.num_sel, GetDlgItem(dialog, IDC_E_UIDS));
		SendDlgItemMessage(dialog, IDC_E_SPLAY, CB_SETCURSEL, data->e.s_player, 0);
	}
}

void E_HandleSetFocus(HWND dialog, WORD id)
{
	EditEffect * data =
		static_cast<EditEffect*>(GetDialogUserData_ptr(dialog));

	if (data->mapview)
	{
		switch (id)
		{
		case IDC_E_LOCX:
		case IDC_E_LOCY:
			SendMessage(data->mapview, MAP_HighlightPoint,
				GetDlgItemInt(dialog, IDC_E_LOCX, NULL, TRUE),
				GetDlgItemInt(dialog, IDC_E_LOCY, NULL, TRUE));
			break;

		case IDC_E_AREAX1:
		case IDC_E_AREAY1:
		case IDC_E_AREAX2:
		case IDC_E_AREAY2:
			SendMessage(data->mapview, MAP_HighlightPoint,
				GetDlgItemInt(dialog, IDC_E_AREAX1, NULL, TRUE),
				GetDlgItemInt(dialog, IDC_E_AREAY1, NULL, TRUE));
			SendMessage(data->mapview, MAP_HighlightPoint,
				GetDlgItemInt(dialog, IDC_E_AREAX2, NULL, TRUE),
				GetDlgItemInt(dialog, IDC_E_AREAY2, NULL, TRUE));
			break;
		}
	}
}

void E_HandleKillFocus(HWND dialog, WORD)
{
	EditEffect * data =
		static_cast<EditEffect*>(GetDialogUserData_ptr(dialog));

	if (data->mapview)
		SendMessage(data->mapview, MAP_UnhighlightPoint,
			MAP_UNHIGHLIGHT_ALL, 0);
}

void E_HandleChangeType(HWND dialog, EditEffect *data)
{
	static const char *promptClearEffect =
		"Would you like to clear the effect? (Recommended)";

	if (data->e.type != EFFECT_None &&
		MessageBox(dialog, promptClearEffect, "Effect Editor", MB_YESNO) == IDYES)
	{
		data->e = Effect();
	}

	int newtype = SendDlgItemMessage(dialog, IDC_E_TYPE, CB_GETCURSEL, 0, 0);
	data->e.type = newtype;
	EffectControls(dialog, newtype);
	LoadEffect(dialog, data);
}

const char warnInvalidE[] =
"Watch out, this effect appears to be invalid.";

void E_HandleCommand(HWND dialog, WORD id, WORD code, HWND control)
{
	/* Most of them use this */
	class EditEffect *data;

	switch (code)
	{
	case BN_CLICKED:
		switch (id)
		{
		case IDC_BUTTON_AREA_ALL:
			{
				SetDlgItemInt(dialog, IDC_E_AREAX1, -1, TRUE);
				SetDlgItemInt(dialog, IDC_E_AREAY1, -1, TRUE);
				SetDlgItemInt(dialog, IDC_E_AREAX2, -1, TRUE);
				SetDlgItemInt(dialog, IDC_E_AREAY2, -1, TRUE);
			}
			break;
		}
	case CBN_SELCHANGE:
		data = (EditEffect*)GetWindowLongPtr(dialog, DWLP_USER);
		switch (id)
		{
		case IDOK:
			{
				bool valid;
				int ret = IDOK;

				SaveEffect(dialog, data);
				valid = data->e.check();

				if (!valid)
					ret = MessageBox(dialog, warnInvalidE, "Effect Editor", MB_OKCANCEL);

				if (ret == IDOK)
				{
					SendMessage(data->parent, EC_Closing,
						MAKELONG(1, valid), reinterpret_cast<LPARAM>(data));
					DestroyWindow(dialog);
				}
			}
			break;

		case IDCANCEL:
			SendMessage(data->parent, EC_Closing,
				0, reinterpret_cast<LPARAM>(data));
			DestroyWindow(dialog);
			break;

		case IDC_E_TYPE:
			E_HandleChangeType(dialog, data);
			break;

		case IDC_E_RESTYPE:
			if (SendMessage(control, CB_GETCURSEL, 0, 0) >= NUM_STYPES &&
				setts.warnbits & WARN_WEIRDRESOURCE)
				MessageBox(dialog, warnWeirdResource, "Warning", MB_ICONWARNING);
			break;

		case IDC_E_OPENSEL:
			OnOpenSel(dialog, data);
			break;

		case IDC_E_OPENSEL2:
			if (SingleUnitSelDialogBox(dialog, data->players,
				data->e.uid_loc, (data->e.uid_loc != -1)))
			{
				SetDlgItemInt(dialog, IDC_E_LOCUID, data->e.uid_loc, FALSE);
			}
			break;

		case IDC_E_LOCM:
			if (IsDlgButtonChecked(dialog, IDC_E_LOCM) == BST_CHECKED)
				setloc(dialog, false);
			else
				setloc(dialog, true);
		}
		break;

	case EN_SETFOCUS:
		E_HandleSetFocus(dialog, id);
		break;

	case EN_KILLFOCUS:
		E_HandleKillFocus(dialog, id);
		break;
	}
}

char noteTrigDeleted[] =
"Selected trigger deleted.";

INT_PTR CALLBACK EffectWndProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;
	class EditEffect *data;	//most use it, but we don't know where it comes from yet

	switch (msg)
	{
	case WM_INITDIALOG:
		data = (EditEffect*)lParam;

		E_Init(dialog);

		if (!data)
		{
			MessageBox(dialog, errorNoData, "Error", MB_ICONWARNING);
			DestroyWindow(dialog);
			ret = FALSE;
			break;
		}

		LoadEffect(dialog, data);

		SetDialogUserData(dialog, data);
		EffectControls(dialog, data->e.type);

		if (data->e.location.x == -1 && data->e.uid_loc != -1)
		{
			CheckDlgButton(dialog, IDC_E_LOCM, BST_CHECKED);
			setloc(dialog, false);
		}
		else
			CheckDlgButton(dialog, IDC_E_LOCM, BST_UNCHECKED);

		ret = TRUE;
		break;

	case WM_COMMAND:
		ret = 0;
		E_HandleCommand(dialog, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;

	case WM_DESTROY:
		data = (EditEffect*)GetWindowLongPtr(dialog, DWLP_USER);
		delete data;
		break;

	case EC_Update:
		data = (EditEffect*)GetWindowLongPtr(dialog, DWLP_USER);
		data->index += wParam;
		break;

	case EC_RefreshTriggers:
		{
			HWND combobox = GetDlgItem(dialog, IDC_E_TRIG);
			data = (EditEffect*)GetWindowLongPtr(dialog, DWLP_USER);

			SendMessage(combobox, CB_RESETCONTENT, 0, 0);
			data->TrigCallback(combobox, data->e.trig_index);

			if (wParam == data->e.trig_index)
				MessageBox(dialog, noteTrigDeleted, "Effect Editor Notice", MB_OK);
		}
		break;
	}

	return ret;
}

/* Condition Editor */

//controls table: identifies which controls each cond type uses
const short cctrls[NUM_CONDS][5] =
{
	{ 0 },	//0x0 = No Effect
	{ IDC_C_UIDOBJ, ECC_AREA },
	{ IDC_C_UIDOBJ, IDC_C_UIDLOC },
	{ ECC_UNITSEL, IDC_C_AMOUNT, IDC_C_PLAYER },
	{ ECC_UNITSEL, IDC_C_AMOUNT, IDC_C_PLAYER, ECC_AREA },	//0x4 = Own Fewer Objects
	{ ECC_UNITSEL, IDC_C_AMOUNT, IDC_C_PLAYER, ECC_AREA },
	{ IDC_C_UIDOBJ },
	{ IDC_C_PLAYER, IDC_C_UIDOBJ },
	{ IDC_C_PLAYER, IDC_C_AMOUNT, IDC_C_RESTYPE },	//0x8 = Accumulate Attribute
	{ IDC_C_PLAYER, IDC_C_RESEARCH },
	{ IDC_C_TIMER },
	{ IDC_C_UIDOBJ },
	{ IDC_C_AISIG },	//0xC = AI Signal
	{ IDC_C_PLAYER },
	{ IDC_C_UIDOBJ, IDC_C_UIDLOC, ECC_UNITSEL },
	{ IDC_C_UIDOBJ },
	{ IDC_C_UIDOBJ },	//0x10 = Object Not Visible
	{ IDC_C_PLAYER, IDC_C_RESEARCH },
	{ IDC_C_UIDOBJ, IDC_C_AMOUNT },
	{ IDC_C_AMOUNT },

	/* 0x13 = OwnFewerFoundations */
	{ ECC_UNITSEL, IDC_C_AMOUNT, IDC_C_PLAYER },
	{ ECC_UNITSEL, IDC_C_AMOUNT, IDC_C_PLAYER, ECC_AREA },
	{ ECC_UNITSEL, IDC_C_AMOUNT, IDC_C_PLAYER, ECC_AREA },
	{ IDC_C_PLAYER }
};

void ConditionControls(HWND dialog, int type)
{
	int id;

	for (id = IDC_C_AMOUNT; id <= IDC_C_USEL2; id++)
		ENABLE_WND(id, false);

	const short *controls = cctrls[type];	//access this table row through pointer
	int count = 5;

	while (count-- && (id = *controls++))
	{
		ENABLE_WND(IDC_C_U1, true);
		if (id == ECC_AREA)
		{
			ENABLE_WND(IDC_C_AREAX1, true);
			ENABLE_WND(IDC_C_AREAY1, true);
			ENABLE_WND(IDC_C_AREAX2, true);
			ENABLE_WND(IDC_C_AREAY2, true);
		}
		else if (id == ECC_UNITSEL)
		{
			ENABLE_WND(IDC_C_UCNST, true);
			ENABLE_WND(IDC_C_GROUP, true);
			ENABLE_WND(IDC_C_UTYPE, true);
		}
		else
			ENABLE_WND(id, true);

		if (id == IDC_C_UIDOBJ)
			ENABLE_WND(IDC_C_USEL1, true);
		else if (id == IDC_C_UIDLOC)
			ENABLE_WND(IDC_C_USEL2, true);
	}
}

void C_Init(HWND dialog)
{
	Combo_Fill(dialog, IDC_C_TYPE, Condition::types, NUM_CONDS);
	Combo_Fill(dialog, IDC_C_PLAYER, players_ec, EC_NUM_PLAYERS);
	LCombo_Fill(dialog, IDC_C_RESEARCH, esdata.techs.head());
	LCombo_Fill(dialog, IDC_C_RESTYPE, esdata.resources.head());
	Combo_PairFill(GetDlgItem(dialog, IDC_C_GROUP), NUM_GROUPS, groups);
	Combo_PairFill(GetDlgItem(dialog, IDC_C_UTYPE), NUM_UTYPES, utypes);
	LCombo_Fill(dialog, IDC_C_UCNST, esdata.units.head(), noselect);
}

void LoadCond(HWND dialog, EditCondition *data)
{
	Condition *c = &data->c;

	SendDlgItemMessage(dialog, IDC_C_TYPE, CB_SETCURSEL, c->type, 0);
	SendDlgItemMessage(dialog, IDC_C_PLAYER, CB_SETCURSEL, c->player, 0);
	SetDlgItemInt(dialog, IDC_C_UIDOBJ, c->object, TRUE);
	SetDlgItemInt(dialog, IDC_C_UIDLOC, c->u_loc, TRUE);
	LCombo_Select(dialog, IDC_C_UCNST, c->pUnit);
	Combo_SelectByData(GetDlgItem(dialog, IDC_C_GROUP), c->group);
	Combo_SelectByData(GetDlgItem(dialog, IDC_C_UTYPE), c->utype);
	SetDlgItemInt(dialog, IDC_C_AREAX1, c->area.left, TRUE);
	SetDlgItemInt(dialog, IDC_C_AREAY1, c->area.bottom, TRUE);
	SetDlgItemInt(dialog, IDC_C_AREAX2, c->area.right, TRUE);
	SetDlgItemInt(dialog, IDC_C_AREAY2, c->area.top, TRUE);
	SetDlgItemInt(dialog, IDC_C_TIMER, c->timer, TRUE);
	SetDlgItemInt(dialog, IDC_C_AISIG, c->ai_signal, TRUE);
	LCombo_Select(dialog, IDC_C_RESEARCH, c->pTech);
	SetDlgItemInt(dialog, IDC_C_U1, c->u1, TRUE); //Controls Condition Reversing
	SetDlgItemInt(dialog, IDC_C_AMOUNT, c->amount, TRUE);
	LCombo_SelById(dialog, IDC_C_RESTYPE, c->res_type);

	/* if (c->u1 != -1)
		MessageBox(dialog, "OMG! An unknown member had a meaningful value! You must report this!",
			"Condition Load", MB_OK); */ //Code for detecting Value of the unknown field. Since UP uses it, I decided to disable this code
}

void SaveCond(HWND dialog, EditCondition *data)
{
	Condition *c = &data->c;

	c->type =		SendDlgItemMessage(dialog, IDC_C_TYPE, CB_GETCURSEL, 0, 0);
	c->pUnit =		(UnitLink*)LCombo_GetSelPtr(dialog, IDC_C_UCNST);
	c->player =		SendDlgItemMessage(dialog, IDC_C_PLAYER, CB_GETCURSEL, 0, 0);
	c->object =		GetDlgItemInt(dialog, IDC_C_UIDOBJ, NULL, TRUE);
	c->u_loc =		GetDlgItemInt(dialog, IDC_C_UIDLOC, NULL, TRUE);
	c->group =		SendDlgItemMessage(dialog, IDC_C_GROUP, CB_GETCURSEL, 0, 0);
	c->area.left =	GetDlgItemInt(dialog, IDC_C_AREAX1, NULL, TRUE);
	c->area.bottom =	GetDlgItemInt(dialog, IDC_C_AREAY1, NULL, TRUE);
	c->area.right =	GetDlgItemInt(dialog, IDC_C_AREAX2, NULL, TRUE);
	c->area.top =	GetDlgItemInt(dialog, IDC_C_AREAY2, NULL, TRUE);
	c->timer =		GetDlgItemInt(dialog, IDC_C_TIMER, NULL, TRUE);
	c->ai_signal =	GetDlgItemInt(dialog, IDC_C_AISIG, NULL, TRUE);
	c->u1 =			GetDlgItemInt(dialog, IDC_C_U1, NULL, TRUE); //Controls Condition Reversing
	c->pTech =		(TechLink*)LCombo_GetSelPtr(dialog, IDC_C_RESEARCH);
	c->amount =		GetDlgItemInt(dialog, IDC_C_AMOUNT, NULL, TRUE);
	c->res_type =	LCombo_GetSelId(dialog, IDC_C_RESTYPE);

	c->group = Combo_GetSelData(GetDlgItem(dialog, IDC_C_GROUP));
	c->utype = Combo_GetSelData(GetDlgItem(dialog, IDC_C_UTYPE));
}

const char warnInvalidC[] =
"Watch out, this condition appears to be invalid.";

void C_HandleSetFocus(HWND dialog, WORD id)
{
	EditEC * data =
		static_cast<EditEC*>(GetDialogUserData_ptr(dialog));

	switch (id)
	{
	case IDC_C_AREAX1:
	case IDC_C_AREAY1:
	case IDC_C_AREAX2:
	case IDC_C_AREAY2:
		SendMessage(data->mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_C_AREAX1, NULL, TRUE),
			GetDlgItemInt(dialog, IDC_C_AREAY1, NULL, TRUE));
		SendMessage(data->mapview, MAP_HighlightPoint,
			GetDlgItemInt(dialog, IDC_C_AREAX2, NULL, TRUE),
			GetDlgItemInt(dialog, IDC_C_AREAY2, NULL, TRUE));
		break;
	}
}

void C_HandleKillFocus(HWND dialog, WORD)
{
	EditEC * data =
		static_cast<EditEC*>(GetDialogUserData_ptr(dialog));

	if (data->mapview)
	{
		SendMessage(data->mapview, MAP_UnhighlightPoint,
			MAP_UNHIGHLIGHT_ALL, 0);
	}
}

void C_HandleChangeType(HWND dialog, EditCondition *data)
{
	static const char *promptClearCondition =
		"Would you like to clear the condition? (Recommended)";

	if (data->c.type != EFFECT_None &&
		MessageBox(dialog, promptClearCondition, "Condition Editor", MB_YESNO) == IDYES)
	{
		// reset to default-constructor state
		data->c = Condition();
	}

	int newtype = SendDlgItemMessage(dialog, IDC_C_TYPE, CB_GETCURSEL, 0, 0);
	data->c.type = newtype;
	ConditionControls(dialog, newtype);
	LoadCond(dialog, data);
}

void C_HandleCommand(HWND dialog, WORD id, WORD code, HWND)
{
	EditCondition *data = (EditCondition*)GetWindowLongPtr(dialog, DWLP_USER);
	switch (code)
		{
		case BN_CLICKED:
			switch (id)
			{
			case IDC_BUTTON_AREA_ALL_COND:
				{
					SetDlgItemInt(dialog, IDC_C_AREAX1, -1, TRUE);
					SetDlgItemInt(dialog, IDC_C_AREAY1, -1, TRUE);
					SetDlgItemInt(dialog, IDC_C_AREAX2, -1, TRUE);
					SetDlgItemInt(dialog, IDC_C_AREAY2, -1, TRUE);
				}
				break;
			}
	}
	if (id == IDOK)
	{
		bool valid;
		int ret = IDOK;

		SaveCond(dialog, data);	//update type
		valid = data->c.check();

		if (!valid)
			ret = MessageBox(dialog, warnInvalidC, "Condition Editor", MB_OKCANCEL);

		if (ret == IDOK)
		{
			SendMessage(data->parent, EC_Closing,
				MAKELONG(1, valid), reinterpret_cast<LPARAM>(data));
			DestroyWindow(dialog);
		}
	}
	else if (id == IDCANCEL)
	{
		SendMessage(data->parent, EC_Closing,
			0, reinterpret_cast<LPARAM>(data));
		DestroyWindow(dialog);
	}
	else if (id == IDC_C_TYPE && code == CBN_SELCHANGE)
	{
		C_HandleChangeType(dialog, data);
	}
	else if (id == IDC_C_USEL1)
	{
		if (SingleUnitSelDialogBox(dialog, data->players,
			data->c.object, data->c.object != -1))
		{
			SetDlgItemInt(dialog, IDC_C_UIDOBJ, data->c.object, FALSE);
		}
	}
	else if (id == IDC_C_USEL2)
	{
		if (SingleUnitSelDialogBox(dialog, data->players,
			data->c.u_loc, data->c.u_loc != -1))
		{
			SetDlgItemInt(dialog, IDC_C_UIDLOC, data->c.u_loc, FALSE);
		}
	}
	else if (code == EN_SETFOCUS)
		C_HandleSetFocus(dialog, id);

	else if (code == EN_KILLFOCUS)
		C_HandleKillFocus(dialog, id);
}

INT_PTR CALLBACK CondWndProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	switch (msg)
	{

	case WM_INITDIALOG:
		{
			class EditCondition *data = (EditCondition*)lParam;

			C_Init(dialog);

			if (data)
			{
				SetWindowLongPtr(dialog, DWLP_USER, lParam);
				ConditionControls(dialog, data->c.type);
				LoadCond(dialog, data);

				ret = TRUE;
			}
			else
			{
				MessageBox(dialog, errorNoData, "Error", MB_ICONWARNING);
				DestroyWindow(dialog);

				ret = FALSE;
			}
		}
		break;

	case WM_COMMAND:
		ret = 0;
		C_HandleCommand(dialog, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;

	case WM_DESTROY:
		{
			class EditCondition *data = (EditCondition*)GetWindowLongPtr(dialog, DWLP_USER);
			delete data;
		}
		break;

	case EC_Update:
		{
			class EditCondition *data = (EditCondition*)GetWindowLongPtr(dialog, DWLP_USER);
			data->index += wParam;
		}
	}

	return ret;
}
