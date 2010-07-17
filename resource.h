/**
	AOK Trigger Studio
	WINDOWS VERSION.
	resource.h -- Defines version constants and resource IDs

	VIEW/CONTROLLER
**/

#define VER_MAJOR 0x0001
#define VER_MINOR 0x00000
#define VER_BUGFIX 0x0001
#define VER_BUILD 0x0000
#define VER_STRING "1.0.1"

/* Custom controls */
#define LCOMBO "LinkedCombo"
#define LLISTB "LinkedListB"

/* Custom control styles */
#define LCBS_ALLOWNOSELECT CBS_HASSTRINGS	//Style to allow for no selection.


/* Icons */
#define IDI_LOGO			16	//0x10

/* Menus */
#define IDM_MAIN			16	//0x10

/* Accelerators */
#define IDA_MAIN			32	//0x20

/* Status Bars */
#define IDS_MAIN			48	//0x30

/* Dialogs */
#define IDD_ABOUT			101
#define IDD_STATS			102
#define IDD_UNITSEL1		103
#define IDD_UNITSEL2		104
#define IDD_MSGS			110
#define IDD_PLAYERS			111
#define IDD_VICTORY			112
#define IDD_DISABLES		113
#define IDD_TERRAIN			114
#define IDD_UNITS			115
#define IDD_TRIGGERS		116
#define IDD_EFFECT			118
#define IDD_COND			119

/* Bitmaps */
#define IDB_LOGO			200
#define IDB_GOOD			201
#define IDB_EVIL			202

/* Info/Messages Controls */
#define IDC_M_MSGS			1001
#define IDC_M_CINEM			1002
#define IDC_M_STABLE        1003
#define IDC_M_EXPORT		1009
#define IDC_M_SEL			1014
#define IDC_M_SELC			1016
#define IDC_G_VER			1010
#define IDC_G_TIMESTAMP		1011
#define IDC_G_ONAME			1012
#define IDC_G_ALLTECHS		1018
#define IDC_G_X				1019
#define IDC_G_Y				1020

/* Statistics Controls */
#define IDC_S_PLAYERS		1050
#define IDC_S_TRIGGERS		1052
#define IDC_S_CONDITIONS	1053
#define IDC_S_EFFECTS		1054
#define IDC_S_MAPSIZE		1055
#define IDC_S_UNITS			1060
#define IDC_S_UNITS1		1061
#define IDC_S_UNITS2		1062
#define IDC_S_UNITS3		1063
#define IDC_S_UNITS4		1064
#define IDC_S_UNITS5		1065
#define IDC_S_UNITS6		1066
#define IDC_S_UNITS7		1067
#define IDC_S_UNITS8		1068
#define IDC_S_UNITS9		1069
#define IDC_S_DISABLE1		1071
#define IDC_S_DISABLE2		1072
#define IDC_S_DISABLE3		1073
#define IDC_S_DISABLE4		1074
#define IDC_S_DISABLE5		1075
#define IDC_S_DISABLE6		1076
#define IDC_S_DISABLE7		1077
#define IDC_S_DISABLE8		1078

/* Player Controls */
#define IDC_P_SP1			1100
#define IDC_P_SP2			1101
#define IDC_P_SP3			1102
#define IDC_P_SP4			1103
#define IDC_P_SP5			1104
#define IDC_P_SP6			1105
#define IDC_P_SP7			1106
#define IDC_P_SP8			1107
#define IDC_P_SG			1108
#define IDC_P_NAME			1111
#define IDC_P_STABLE		1112
#define IDC_P_ACTIVE		1113
#define IDC_P_HUMAN			1114
#define IDC_P_AI			1115
#define IDC_P_CIV			1116
#define IDC_P_POP			1117
#define IDC_P_X				1118
#define IDC_P_Y				1119
#define IDC_P_COLOR			1120
#define IDC_P_EXAI			1121
#define IDC_P_FOOD			1122
#define IDC_P_WOOD			1123
#define IDC_P_GOLD			1124
#define IDC_P_STONE			1125
#define IDC_P_OREX			1126
#define IDC_P_SPDIP			1130
#define IDC_P_DSTATE		1131
#define IDC_P_AV			1132
#define IDC_P_IMAI			1133
#define IDC_P_AGE			1134
#define IDC_P_AIMODE		1140
#define IDC_P_UF			1141
#define IDC_P_US0			1147
#define IDC_P_US1			1148
#define IDC_P_LBL_FOOD      1160
#define IDC_P_LBL_WOOD      1161
#define IDC_P_LBL_GOLD      1162
#define IDC_P_LBL_STONE     1163
#define IDC_P_LBL_OREX      1164

/* Trigger Controls */
#define IDC_T_TREE				1200
#define IDC_T_NAME				1201
#define IDC_T_STATE				1202
#define IDC_T_LOOP				1203
#define IDC_T_OBJ				1204
#define IDC_T_ORDER				1205
#define IDC_T_DESC				1206
#define IDC_T_EDIT				1207
#define IDC_T_NCOND				1208
#define IDC_T_NEFFECT			1209
#define IDC_T_DEL				1210
#define IDC_T_ADD				1211
#define IDC_T_DUPP				1212

/* Global Victory Controls */
#define IDC_V_SSTD				1300
#define IDC_V_SCONQ				1301
#define IDC_V_SSCORE			1302
#define IDC_V_STIME				1303
#define IDC_V_SCUSTOM			1304
#define IDC_V_ALL				1305
#define IDC_V_SCORE				1306
#define IDC_V_TIME				1307
#define IDC_V_RELICS			1308
#define IDC_V_EXPL				1309
#define IDC_V_CONQUEST			1310

/* Map/Terrain Controls */
#define IDC_TR_AITYPE			1402
#define IDC_TR_SIZE				1403
#define IDC_TR_TX				1404
#define IDC_TR_TY				1405
#define IDC_TR_ID				1406
#define IDC_TR_ELEV				1407
#define IDC_TR_MCX1				1410
#define IDC_TR_MCY1				1411
#define IDC_TR_MCX2				1412
#define IDC_TR_MCY2				1413
#define IDC_TR_MCXT				1414
#define IDC_TR_MCYT				1415
#define IDC_TR_MCSET1			1416
#define IDC_TR_MCSET2			1417
#define IDC_TR_MCSETT			1418
#define IDC_TR_MCPASTE			1419
#define IDC_TR_MCCOPY			1420

/* Units Controls */
#define IDC_U_SELU			1500
#define IDC_U_SELP			1501
#define IDC_U_TYPE			1502
#define IDC_U_UNIT			1503
#define IDC_U_CONST			1504
#define IDC_U_X				1505
#define IDC_U_Y				1506
#define IDC_U_ROTATE		1507
#define IDC_U_FRAME			1508
#define IDC_U_ADD			1509
#define IDC_U_DEL			1510
#define IDC_U_GARRISON		1511
#define IDC_U_SORT			1512

/* Disables Controls */
#define IDC_D_SPLY			1600
#define IDC_D_STYPE			1601
#define IDC_D_ALL			1602
#define IDC_D_SEL			1603
#define IDC_D_CLR			1604
#define IDC_D_DEL			1605
#define IDC_D_ADD			1606
#define IDC_D_FILL			1607

/* Effect Controls */
#define IDC_E_TYPE				1800
#define IDC_E_SPLAY				1801
#define IDC_E_SOUND				1802
#define IDC_E_TEXT				1803
#define IDC_E_AIGOAL			1804
#define IDC_E_TPLAY				1805
#define IDC_E_AMOUNT			1806
#define IDC_E_RESTYPE			1807
#define IDC_E_DSTATE			1808
#define IDC_E_RESEARCH			1809
#define IDC_E_PANEL				1810
#define IDC_E_LOCX				1811
#define IDC_E_LOCY				1812
#define IDC_E_AREAX1			1813
#define IDC_E_AREAY1			1814
#define IDC_E_AREAX2			1815
#define IDC_E_AREAY2			1816
#define IDC_E_DTIME				1817
#define IDC_E_UIDS				1818
#define IDC_E_LOCUID			1819
#define IDC_E_UCNST				1820
#define IDC_E_LOCM				1821
#define IDC_E_GROUP				1822
#define IDC_E_TRIG				1823
#define IDC_E_OPENSEL			1824
#define IDC_E_OPENSEL2			1825
#define IDC_E_STRINGID          1826
#define IDC_E_UTYPE				1827

/* Condition Controls */
#define IDC_C_TYPE				1900
#define IDC_C_AMOUNT			1901
#define IDC_C_RESTYPE			1902
#define IDC_C_UIDOBJ			1903
#define IDC_C_UIDLOC			1904
#define IDC_C_UCNST				1905
#define IDC_C_GROUP				1906
#define IDC_C_PLAYER			1907
#define IDC_C_RESEARCH			1908
#define IDC_C_TIMER				1909
#define IDC_C_AISIG				1910
#define IDC_C_U1				1911
#define IDC_C_UTYPE				1912
#define IDC_C_AREAX1			1914
#define IDC_C_AREAY1			1915
#define IDC_C_AREAX2			1916
#define IDC_C_AREAY2			1917
#define IDC_C_USEL1             1920
#define IDC_C_USEL2             1921

/* Unit Selection Controls */
#define IDC_US_UNITS			1200
#define IDC_US_SORT				1201
#define IDC_US_PLAYER			1202
#define IDC_US_NOTE				1203

/* Menu Items */
#define ID_FILE_NEW				0xE100
#define ID_FILE_OPEN			0xE101
#define ID_FILE_CLOSE			0xE102
#define ID_FILE_SAVE			0xE103
#define ID_FILE_SAVE_AS			0xE104
#define ID_FILE_SAVE_AS2		0xE105
#define ID_FILE_DUMP			0xE106
#define ID_FILE_TRIGWRITE       0xE107
#define ID_FILE_TRIGREAD        0xE108
#define ID_FILE_RECENT1			0xE110
#define ID_FILE_RECENT2			0xE111
#define ID_FILE_RECENT3			0xE112
#define ID_FILE_RECENT4			0xE113
#define ID_EDIT_RENAME          0xE120
#define ID_EDIT_COPY			0xE122
#define ID_EDIT_CUT				0xE123
#define ID_EDIT_PASTE			0xE125
#define ID_EDIT_DELETE			0xE126
#define ID_EDIT_UNDO			0xE12B
#define ID_VIEW_STATUS_BAR		0xE131
#define ID_VIEW_STATISTICS		0xE132
#define ID_VIEW_MAP				0xE133
#define ID_APP_ABOUT			0xE140
#define ID_APP_EXIT				0xE141
#define ID_HELP					0xE146
#define ID_TOOLS_COMPRESS		0xE150
#define ID_TOOLS_DECOMPRESS		0xE151
