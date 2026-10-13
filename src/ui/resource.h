//{{NO_DEPENDENCIES}}
// Microsoft Developer Studio generated include file.
// Used by mame32.rc
//
#if 0 /* For RC Editors */
#define USE_SCALE_EFFECTS
#define TRANS_UI
#define UI_COLOR_DISPLAY
#define ROM_PATCH
#define JOYSTICK_ID
#define HAS_M68000
#define USE_JOY_MOUSE_MOVE
#define KAILLERA
#define TREE_SHEET
#define SHOW_UIKEY
#endif /* For RC Editors */

#define IDPAUSE                         3
#define IDAPPLY                         4
#define IDS_UI_FILE                     51
#define IDS_UI_VIEW                     52
#define IDS_UI_OPTIONS                  53
#define IDS_UI_HELP                     54
#define IDS_UI_TOOLBAR                  61
#define IDS_UI_SHOWPICTURES             69	// IDS_UI_TOOLBAR + 8
#define IDS_UI_FONT                     75
#define IDS_UI_ARRANGE                  77	// IDS_UI_TOOLBAR + 16
#ifdef UI_COLOR_DISPLAY
#define IDS_UI_LANGUAGE                 83	// IDS_UI_FONT + 8
#else /* UI_COLOR_DISPLAY */
#define IDS_UI_LANGUAGE                 82	// IDS_UI_FONT + 7
#endif /* UI_COLOR_DISPLAY */

#define IDD_MAIN                        101
#define IDA_TAB_KEYS                    106
#define IDI_MAME32_ICON                 107
#define IDD_ABOUT                       108
#define IDR_MAIN_MENU                   109
#define IDR_UI_MENU                     110
#define IDW_STATUS                      112
#define IDB_ABOUT                       113
#define IDD_DIRECTX                     114
#define IDD_AUDIT                       115
#define IDD_DIRECTORIES                 116
#define IDD_CUSTOM_FILE                 117
#define IDR_CONTEXT_MENU                118
#define IDI_WIN_NOROMS                  119
#define IDI_WIN_ROMS                    120
#define IDI_WIN_UNKNOWN                 121
#define IDI_WIN_CLONE                   122
#define IDI_WIN_REDX                    123
#define IDI_FOLDER_OPEN                 124
#define IDI_FOLDER                      125
#define IDI_FOLDER_AVAILABLE            126
#define IDI_FOLDER_MANUFACTURER         127
#define IDI_FOLDER_UNAVAILABLE          128
#define IDI_FOLDER_YEAR                 129
#define IDI_FOLDER_SOURCE               130
#define IDI_MANUFACTURER                131
#define IDI_WORKING                     132
#define IDI_NONWORKING                  133
#define IDI_YEAR                        134
#define IDI_SOUND                       135
#define IDI_CPU                         136
#define IDI_HARDDISK                    137
#define IDI_SOURCE                      138
#define IDR_CONTEXT_HEADER              151
#define IDD_PROP_GAME                   152
#define IDD_PROP_AUDIT                  153
#define IDD_PROP_DISPLAY                154
#define IDD_PROP_SOUND                  155
#define IDD_PROP_ADVANCED               156
#define IDD_PROP_INPUT                  157
#define IDD_PROP_MISC                   158
#define IDD_PROP_VECTOR                 159
#define IDD_COLUMNS                     160
#define IDI_KEYBOARD                    161
#define IDI_JOYSTICK                    162
#define IDI_DISPLAY                     163
#define IDI_CHECKMARK                   164
#define IDC_CURSOR_HSPLIT               165
#define IDB_TOOLBAR                     166
#define IDB_PROP_DISPLAY                167
#define IDR_CONTEXT_TREE                168
#define IDD_FILTERS                     169
#define IDD_RESET                       170
#define IDD_LOAD_PROGRESS               171
#define IDD_INTERFACE_OPTIONS           173
#define IDD_PROP_HISTORY                176
#define IDD_ROM_ERROR                   177
#define IDD_LANGUAGE                    178
#define IDD_ERROR                       179
#ifdef UI_COLOR_DISPLAY
#define IDD_PALETTE                     180
#endif /* UI_COLOR_DISPLAY */
#define IDR_CONTEXT_SCREENSHOT          181
#define IDI_HEADER_UP                   182
#define IDI_HEADER_DOWN                 183
#define IDD_PROP_DIRECT3D               184
#define IDI_SOUNDTAB                    185
#define IDD_PROP_BIOS                   186
#ifdef ROM_PATCH
#define IDD_PROP_IPS                    187
#endif /* ROM_PATCH */
#define IDB_TOOLBAR_US                  190
#define IDB_TOOLBAR_JP                  191
#define IDB_TOOLBAR_CN                  192
#define IDB_TOOLBAR_TW                  193
#define IDB_TOOLBAR_KR                  194
#define IDB_TOOLBAR_FR                  195
#define IDB_TOOLBAR_ES                  196
#define IDB_TOOLBAR_IT                  197
#define IDB_TOOLBAR_DE                  198
#define IDB_TOOLBAR_BR                  199
#define IDB_TOOLBAR_PL                  200
#define IDB_TOOLBAR_HU                  201

//#ifdef ZETAMAME32 // Interface
#define IDI_WIN_IMPERFECT               202
//#endif /* ZETAMAME32 */

#define IDD_GAMEFINDS                     205

#define IDC_LIST                        1000
#define IDC_PLAY                        1001
#define IDC_DISPLAYS                    1002
#define IDC_JOYSTICK                    1003
#define IDC_VECTORTEXT                  1004
#define IDC_SIZES                       1005
#define IDC_USE_TRAK                    1006
#define IDC_DISPLAYTYPE                 1007
#define IDC_TRAK_SENSITIVITY            1008
#define IDC_FRAMESKIP                   1009
#define IDC_ROTATE                      1010
#define IDC_HELP_TEXT                   1011
#define IDC_TAB                         1012
#define IDC_SCANLINES                   1013
#define IDC_CHEAT                       1014
#define IDC_LOG                         1015
#define IDC_SLEEP                       1016
#define IDC_BACKDROPS                   1017
#define IDC_ARTWORK_CROP                1018
#define IDC_OVERLAYS                    1019
#define IDC_LEDS                        1020
#define IDC_BEZELS                      1021
#define IDC_DDRAW                       1022
#define IDC_GAMMA                       1023
#define IDC_BRIGHTCORRECT               1024
#define IDC_USE_DEFAULT                 1025
#define IDC_VSCANLINES                  1026
#define IDC_SAMPLERATE                  1027
#define IDC_DEPTH                       1028
#define IDC_ANTIALIAS                   1029
#define IDC_BEAM                        1030
#define IDC_FLICKER                     1031
#define IDC_FLIPX                       1032
#define IDC_FLIPY                       1033
#define IDC_INTENSITY                   1034
#define IDC_DIRECTX_HELP                1035
#define IDB_WEB_PAGE                    1036
#define IDC_ROMS_PROGRESS               1037
#define IDC_ROMS_CORRECT                1038
#define IDC_ROMS_INCORRECT              1039
#define IDC_ROMS_TOTAL                  1040
#define IDC_SAMPLES_PROGRESS            1041
#define IDC_SAMPLES_CORRECT             1042
#define IDC_SAMPLES_INCORRECT           1043
#define IDC_SAMPLES_TOTAL               1044
#define IDC_AUDIT_DETAILS               1045
#define IDC_USE_FM_YM3812               1046
#define IDC_DIR_COMBO                   1047
#define IDC_DIR_LIST                    1048
#define IDC_DIR_BROWSE                  1049
#define IDC_DIR_INSERT                  1050
#define IDC_DIR_DELETE                  1051
#define IDC_FLICKERTEXT                 1052
#define IDC_INTENSITYTEXT               1053
#define IDC_SSFRAME                     1054
#define IDC_SSNONE                      1056
#define IDC_SSPICTURE                   1057
#define IDC_BEAMTEXT                    1058
#define IDC_COLORSTEXT                  1059
#define IDC_SIZETEXT                    1060
#define IDC_TRACKSPINTEXT               1061
#define IDC_INPUTDEVTEXT                1062
#define IDC_RATETEXT                    1063
#define IDC_LISTAVAILABLECOLUMNS        1064
#define IDC_LISTSHOWCOLUMNS             1065
#define IDC_BUTTONADD                   1066
#define IDC_BUTTONREMOVE                1067
#define IDC_BUTTONMOVEDOWN              1068
#define IDC_BUTTONMOVEUP                1069
#define IDC_PROP_SOUND                  1070
#define IDC_PROP_CPU                    1071
#define IDC_PROP_TITLE                  1072
#define IDC_PROP_MANUFACTURED           1073
#define IDC_PROP_STATUS                 1074
#define IDC_PROP_SCREEN                 1075
#define IDC_PROP_COLORS                 1076
#define IDC_PROP_ROMS                   1077
#define IDC_PROP_SAMPLES                1078
#define IDC_PROP_CLONEOF_TEXT           1079
#define IDC_PROP_CLONEOF                1080
#define IDC_TRANSLUCENCY                1081
#define IDC_VERSION                     1082
#define IDC_ARTWORK                     1083
#define IDC_BRIGHTNESS                  1084
#define IDC_GAMMADISP                   1085
#define IDC_BRIGHTNESSDISP              1086
#define IDC_BRIGHTCORRECTDISP           1087
#define IDC_FLICKERDISP                 1088
#define IDC_INTENSITYDISP               1089
#define IDC_BEAMDISP                    1090
#define IDC_AUTOFRAMESKIP               1091
#define IDC_TREE                        1092
#define IDC_WINDOWED                    1093
#define IDC_STEREO                      1094
#define IDC_PROP_RESET                  1095
#define IDC_SPLITTER                    1096
#define IDC_SPLITTER2                   1097
#define IDC_DIVIDER                     1098
#define IDC_FILTER_VECTOR               1099
#define IDC_FILTER_RASTER               1100
#define IDC_FILTER_UNAVAILABLE          1101
#define IDC_FILTER_NONWORKING           1102
#define IDC_FILTER_CLONES               1103
#define IDC_RESET_GAMES                 1105
#define IDC_RESET_UI                    1106
#define IDC_RESET_FILTERS               1107
#define IDC_LOAD_PROGRESS               1108
#define IDC_RESET_DEFAULT               1109
#define IDC_FILTER_ORIGINALS            1110
#define IDC_FILTER_WORKING              1111
#define IDC_FILTER_AVAILABLE            1112
#define IDC_CONFIGURETEXT               1113
#define IDC_JOYMAPTEXT                  1114
#define IDC_START_GAME_CHECK            1115
#define IDC_VOLUME                      1117
#define IDC_VOLUMEDISP                  1118
#define IDC_PROP_HISTORY                1119
#define IDC_HISTORY                     1120
#define IDC_SAMPLES                     1121
#define IDC_ERROR_DETAILS               1122
#define IDC_TRIPLE_BUFFER               1123
#define IDC_LOAD_ROMNAME                1124
#define IDC_DEFAULT_INPUT               1125
#define IDC_LANGUAGECHECK               1126
#define IDC_LANGUAGEBROWSE              1127
#define IDC_LANGUAGEEDIT                1128
#define IDC_FILTER_EDIT                 1129
#define IDC_ABOUT                       1130
#define IDC_PROP_DISPLAY                1131
#define IDC_USE_FILTER                  1132
#define IDC_USE_SOUND                   1133
#define IDC_WAITVSYNC                   1134
#define IDC_HWSTRETCH                   1135
#define IDC_SWITCHRES                   1136
#define IDC_SWITCHBPP                   1137
#define IDC_MAXIMIZE                    1138
#define IDC_KEEPASPECT                  1139
#define IDC_MATCHREFRESH                1140
#define IDC_SYNCREFRESH                 1141
#define IDC_USE_STICKPOINT              1142	// Support Stick-type Pointing Device by miko2u@hotmail.com
#define IDC_USE_MOUSE                   1143
#define IDC_THROTTLE                    1144
#define IDC_RESDEPTH                    1145
#define IDC_VOLUMETEXT                  1146
#define IDC_RESDEPTHTEXT                1147
#define IDC_REFRESH                     1148
#define IDC_REFRESHTEXT                 1149
#define IDC_EFFECT                      1150
#define IDC_BRIGHTNESSTEXT              1151
#define IDC_STEADYKEY                   1152
#define IDC_ASPECTRATIO                 1153
#define IDC_ASPECTRATION                1154
#define IDC_ASPECTRATIOD                1155
#define IDC_ASPECTRATIOTEXT             1156
#define IDC_JOY_GUI                     1157
#define IDC_BROADCAST                   1158
#define IDC_ARTRES                      1159
#define IDC_A2D                         1160
#define IDC_A2DDISP                     1161
#define IDC_A2DTEXT                     1162
#define IDC_ARTRESTEXT                  1163
#define IDC_PLAY_GAME                   1164
#define IDC_LIGHTGUN                    1165
#define IDC_SSTAB                       1167
#define IDC_GAME_ICON                   1168
#define IDC_RANDOM_BG                   1169
#define IDC_CUSTOM_TREE                 1170
#define IDC_CUSTOMFILEBROWSE            1171
#define IDC_CUSTOMFILE_GAME             1172
#define IDC_LOAD_STATUS                 1173
#define IDC_ERROR_TEXT                  1174
#define IDC_ARTMISCTEXT                 1175
#define IDC_SKIP_DISCLAIMER             1176
#define IDC_SKIP_GAME_INFO              1177
#define IDC_PROP_SOURCE                 1178
#define IDC_OLD_TIMING                  1179
#define IDC_PAUSEBRIGHT                 1180
#define IDC_PAUSEBRIGHTDISP             1181
#define IDC_HIGH_PRIORITY               1182
#define IDC_D3D                         1183
#define IDC_D3D_FILTER                  1184
#define IDC_D3D_FILTERTEXT              1185
#define IDC_D3D_TEXTURE_MANAGEMENT      1186
#define IDC_D3D_EFFECT                  1187
#define IDC_D3D_EFFECTTEXT              1188
#define IDC_D3D_PRESCALE                1189
#define IDC_D3D_PRESCALETEXT            1190
#define IDC_D3D_ROTATE_EFFECTS          1191
#define IDC_CLEAN_STRETCH               1192
#define IDC_CLEAN_STRETCHTEXT           1193
#define IDC_AUDIO_LATENCY_DISP          1194
#define IDC_D3D_SCANLINES               1195
#define IDC_D3D_SCANLINES_ENABLE        1196
#define IDC_D3D_SCANLINES_DISP          1197
#define IDC_D3D_FEEDBACK                1201
#define IDC_D3D_FEEDBACK_ENABLE         1202
#define IDC_D3D_FEEDBACK_DISP           1203
#define IDC_ZOOM                        1204
#define IDC_ZOOMTEXT                    1205
#define IDC_ZOOMDIST                    1206
#define IDC_BIOSTEXT                    1207
#define IDC_BIOS                        1208
#define IDC_HIDE_MOUSE                  1209
#define IDC_AUDIO_LATENCY               1210
#define IDC_EFFECTOVERRIDES             1212
#define IDC_AUDIT_DETAILS_PROP          1213
#define IDC_CYCLETIME                   1214
#define IDC_CYCLETIMESEC                1215
#define IDC_CYCLETIMESECTXT             1216
#define IDC_AUDIO_LATENCY_TEXT          1217
#define IDC_STRETCH_SCREENSHOT_LARGER   1218
#define IDC_DUAL_LIGHTGUN               1219
#define IDC_RELOAD                      1220
#define IDC_CONFIRM_QUIT                1221
//ks hcmame s switch m68k core
//#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020)
#define IDC_M68K_CORE                   1222
#define IDC_M68K_CORETEXT               1223
//#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020) */
//ks hcmame e switch m68k core
#ifdef USE_VOLUME_AUTO_ADJUST
#define IDC_VOLUME_ADJUST               1224
#endif /* USE_VOLUME_AUTO_ADJUST */
#define IDC_BROKEN_ICON                 1225
#define IDC_EDIT_BROKEN_COLOR           1226

#ifdef JOYSTICK_ID
#define IDC_JOYID1                      1230
#define IDC_JOYID2                      1231
#define IDC_JOYID3                      1232
#define IDC_JOYID4                      1233
#define IDC_JOYID5                      1234
#define IDC_JOYID6                      1235
#define IDC_JOYID7                      1236
#define IDC_JOYID8                      1237
#define IDC_JOYIDTEXT                   1238
#define IDC_JOYID1TEXT                  1239
#define IDC_JOYID2TEXT                  1240
#define IDC_JOYID3TEXT                  1241
#define IDC_JOYID4TEXT                  1242
#define IDC_JOYID5TEXT                  1243
#define IDC_JOYID6TEXT                  1244
#define IDC_JOYID7TEXT                  1245
#define IDC_JOYID8TEXT                  1246
#endif /* JOYSTICK_ID */

#define IDC_BIOS1                       1250
#define IDC_BIOS2                       1251
#define IDC_BIOS3                       1252
#define IDC_BIOS4                       1253
#define IDC_BIOS5                       1254
#define IDC_BIOS6                       1255
#define IDC_BIOS7                       1256
#define IDC_BIOS8                       1257
#define IDC_BIOSTEXT1                   1258
#define IDC_BIOSTEXT2                   1259
#define IDC_BIOSTEXT3                   1260
#define IDC_BIOSTEXT4                   1261
#define IDC_BIOSTEXT5                   1262
#define IDC_BIOSTEXT6                   1263
#define IDC_BIOSTEXT7                   1264
#define IDC_BIOSTEXT8                   1265

#ifdef USE_SCALE_EFFECTS
#define IDC_SCALEEFFECT                 1273
#define IDC_SCALEEFFECTTEXT             1274
#define IDC_USE_VIDEOMEMORY             1275
#endif /* USE_SCALE_EFFECTS */

#define IDC_USE_LIST                    1276

#ifdef UI_COLOR_DISPLAY
#define IDC_PALETTE_COMBO               1279
#define IDC_PALETTE_R                   1280
#define IDC_PALETTE_G                   1281
#define IDC_PALETTE_B                   1282
#define IDC_PALETTE_VIEW                1283
#define IDC_PALETTE_TEXTR               1284
#define IDC_PALETTE_TEXTG               1285
#define IDC_PALETTE_TEXTB               1286
#endif /* UI_COLOR_DISPLAY */

#ifdef TRANS_UI
#define IDC_TRANSUI                     1289
#define IDC_TRANSPARENCY                1290
#define IDC_TRANSPARENCYDISP            1291
#endif /* TRANS_UI */

#define IDC_FILTER_INHERIT              1292
#define IDC_INHERITED                   1293
#define IDC_RESET_PLAYTIME              1294
#define IDC_RESET_PLAYCOUNT             1295
#define IDC_LEDMODE                     1296
#define IDC_NOOFFSET_CLONES             1297
#define IDC_KEY_GUI                     1298
#define IDC_HISTORY_TAB                 1299
#define IDC_GAME_CAPTION                1300
#define IDC_SCREENSHOT_BORDERSIZE       1301
#define IDC_SCREENSHOT_BORDERCOLOR      1302
#define IDC_SCREENSHOT_BORDERSIZETXT    1303
#ifdef ROM_PATCH
#define IDC_IPS1                        1304
#define IDC_IPS2                        1305
#define IDC_IPS3                        1306
#define IDC_IPS4                        1307
#define IDC_IPS5                        1308
#define IDC_IPS6                        1309
#define IDC_IPS7                        1310
#define IDC_IPS8                        1311
#endif /* ROM_PATCH */
#define IDC_SINGLE_PROCESS              1312
#ifdef TREE_SHEET
#define IDB_MISSING_SNAPSHOT			      1313
#endif /*TREE_SHEET*/
//#ifdef MENU_TRANS
#define IDD_PROP_PLUS                   1350
#define IDB_MENUPREVIEW                 1351
#define IDC_BTN_MENUBGCOLOR             1352
#define IDC_STATIC_MENUBGCOLOR          1353
#define IDC_STATIC_MENUPREVIEW          1354
#define IDC_PLUS_MENUTRANSPARENCY       1355
#define IDC_STATIC_MENUTRANSPARENCY     1356
//#endif /*MENU_TRANS*/
//#ifdef LOCK60FPS
#define IDC_PLUS_FIX60FPS               1357
//#endif /*LOCK60FPS*/

#define IDC_HOMEPAGE                    1360

#define IDC_SKIP_VALIDITY_CHECKS        1400
#define IDC_CRC_ONLY                    1401

//#ifdef USE_VIRTUAL_3D_VIEW
#define IDD_PROP_VIRTUAL3D               300
#define IDC_D3D_VIRTUAL_3DVIEW          1500
#define IDC_D3D_ROTATION                1501
#define IDC_D3D_ROTATION_DISP           1502
#define IDC_D3D_ZOOMX                   1503
#define IDC_D3D_ZOOMX_DISP              1504
#define IDC_D3D_ZOOMY                   1505
#define IDC_D3D_ZOOMY_DISP              1506
#define IDC_D3D_KEYSTONERATIOU          1507
#define IDC_D3D_KEYSTONERATIOD          1508
#define IDC_D3D_KEYSTONERATIOTEXT       1509
#define IDC_D3D_ZOOMXTEXT               1510
#define IDC_D3D_ZOOMYTEXT               1511
#define IDC_D3D_ROTATIONTEXT            1512
//#endif /* USE_VIRTUAL_3D_VIEW */

#define IDI_FOLDER_NEOGEO               1600
#define IDI_FOLDER_CPS1                 1601
#define IDI_FOLDER_CPS2                 1602

//#ifdef USE_PSXPLUGIN
#define IDD_PROP_PSXPLUGIN              1331
#define IDC_USE_GPUPLUGIN               1332
#define IDC_MAKE_GPUGAMEWINDOW          1333
#define	IDC_COMBO_GPUPLUGIN				1334
#define IDC_COMBO_PSXGPU_SCREENSIZE     1337
#define IDC_PSXGPU_FULLSCREEN           1338
#define IDC_PSXGPU_SHOWFPS              1339
#define IDC_PSXGPU_SCANLINE             1340
#define IDC_PSXGPU_BLENDING             1341
#define IDI_FOLDER_PSXGAME				1342
#define IDC_BTN_GPUCFG					1343
//#endif //USE_PSXPLUGIN

#define ID_CONTEXT_SHOW_FOLDER_START    39800
#define ID_CONTEXT_SHOW_FOLDER_END      40000

#define ID_FILE_EXIT                    40001
#define ID_ABOUT                        40003
#define ID_FILE_PLAY                    40004
#define ID_HELP_ABOUT                   40006
#define ID_VIEW_LARGE_ICON              40007
#define ID_VIEW_SMALL_ICON              40008
#define ID_VIEW_LIST_MENU               40009
#define ID_VIEW_DETAIL                  40010
#define ID_VIEW_GROUPED                 40011
#define ID_OPTIONS_FONT                 40012
#define ID_OPTIONS_CLONE_COLOR          40013
#define ID_OPTIONS_DIR                  40014
#define ID_OPTIONS_DEFAULTS             40015
#define ID_OPTIONS_RESET_DEFAULTS       40016
#define ID_OPTIONS_INTERFACE            40017
#define ID_FILE_AUDIT                   40018
#define ID_FILE_PLAY_RECORD             40019
#define ID_FILE_PLAY_BACK               40020
#define ID_FILE_PLAY_RECORD_WAVE        40021
#define ID_VIEW_PICTURE_AREA            40023
#define ID_HELP_CONTENTS                40025
#define ID_HELP_WHATS_NEW32             40026
#define ID_HELP_TROUBLE                 40027
#define ID_HELP_RELEASE                 40028
#define ID_HELP_WHATS_NEW               40029
#define ID_HELP_CHEATS                  40030
#define ID_VIEW_BYGAME                  40034
#define ID_VIEW_BYDIRECTORY             40035
#define ID_VIEW_BYMANUFACTURER          40036
#define ID_VIEW_BYYEAR                  40037
#define ID_VIEW_BYTYPE                  40038
#define ID_VIEW_BYTIMESPLAYED           40039
#define ID_VIEW_LINEUPICONS             40040
#define ID_UPDATE_GAMELIST              40042
#define ID_SORT_ASCENDING               40045
#define ID_SORT_DESCENDING              40046
#define ID_CUSTOMIZE_FIELDS             40047
#define ID_GAME_PROPERTIES              40048
#define ID_VIEW_PAGETAB                 40049
#define ID_VIEW_FOLDERS                 40050
#define ID_VIEW_SHOWPICTURES            40051	// ID_VIEW_PAGETAB + 2
#define ID_VIEW_TOOLBARS                40052
#define ID_VIEW_STATUS                  40053
#define ID_CONTEXT_ADD_CUSTOM           40054
#define ID_CONTEXT_REMOVE_CUSTOM        40055
#define ID_GAME_AUDIT                   40056
#define ID_CONTEXT_SELECT_RANDOM        40057
#define ID_CONTEXT_RESET_PLAYTIME       40058
#define ID_CONTEXT_RESET_PLAYCOUNT      40059
#define ID_CONTEXT_RENAME_CUSTOM        40060
#define ID_CONTEXT_FILTERS              40061
#define ID_OPTIONS_LANGUAGE             40062
#define ID_CONTEXT_SHOW_FOLDER          40063	// ID_CONTEXT_RENAME_CUSTOM + 3, ID_CONTEXT_FILTERS + 2
#define ID_VIEW_TAB_SCREENSHOT          40064
#define ID_VIEW_TAB_FLYER               40065
#define ID_VIEW_TAB_CABINET             40066
#define ID_VIEW_TAB_MARQUEE             40067
#define ID_VIEW_TAB_TITLE               40068
#define ID_VIEW_TAB_CONTROL_PANEL       40069
#define ID_VIEW_TAB_HISTORY             40070
#define ID_TOGGLE_TAB_SCREENSHOT        40071
#define ID_TOGGLE_TAB_FLYER             40072
#define ID_TOGGLE_TAB_CABINET           40073
#define ID_TOGGLE_TAB_MARQUEE           40074
#define ID_TOGGLE_TAB_TITLE             40075
#define ID_TOGGLE_TAB_CONTROL_PANEL     40076
#define ID_TOGGLE_TAB_HISTORY           40077
#define ID_VIEW_FULLSCREEN              40078
#define ID_OPTIONS_BG                   40079
#ifdef UI_COLOR_DISPLAY
#define ID_OPTIONS_PALETTE              40080
#endif /* UI_COLOR_DISPLAY */
#define ID_FOLDER_PROPERTIES            40081
#define ID_FOLDER_AUDIT                 40082
#define ID_UI_UP                        40083
#define ID_UI_DOWN                      40084
#define ID_UI_LEFT                      40085
#define ID_UI_RIGHT                     40086
#define ID_UI_START                     40087
#define ID_UI_PGUP                      40088
#define ID_UI_PGDOWN                    40089
#define ID_UI_HOME                      40090
#define ID_UI_END                       40091
#define ID_UI_HISTORY_UP                40092
#define ID_UI_HISTORY_DOWN              40093
#define ID_FILE_LOADSTATE               40094
#define ID_SOURCE_PROPERTIES            40095

#ifdef KAILLERA
#define ID_FILE_NETPLAY                 40096
#define ID_OPTIONS_FONT_K               40097
#define ID_FILE_KAILLERA_PLAY_BACK      40098
#endif /* KAILLERA */

#define ID_LANGUAGE_ENGLISH_US          40100
#define ID_LANGUAGE_JAPANESE            40101
#define ID_LANGUAGE_SMPL_CHINESE        40102
#define ID_LANGUAGE_TRAD_CHINESE        40103
#define ID_LANGUAGE_KOREAN              40104
#define ID_LANGUAGE_FRENCH              40105
#define ID_LANGUAGE_SPANISH             40106
#define ID_LANGUAGE_ITALIAN             40107
#define ID_LANGUAGE_GERMAN              40108
#define ID_LANGUAGE_PORTUGUESE_BRA      40109
#define ID_LANGUAGE_POLISH              40110
#define ID_LANGUAGE_HUNGARIAN           40111

//#ifdef SHOW_UIKEY
#define ID_OPTIONS_VIEW_UIKEY			      40112
#define IDD_INGAME_UIKEY				        1800
#define IDC_BTN_RESET										1801
#define IDC_LIST_KEY                    1802
//#endif /*SHOW_UIKEY*/

#define ID_GAME_FILTERS                 40113

#define ID_OPTIONS_INTERFACE_EX         40114
#define ID_OPTIONS_FONT_HISTORY         40115

#define IDD_INTERFACE_EX_OPTIONS        1410
#define IDC_BTN_CHANGEFONT              1411
#define IDC_BTN_CHANGE_CLONECOLOR       1412
#define IDC_STATIC_CLONECOLOR           1413
#define IDC_STATIC_FONTCOLOR            1414
#define IDC_STATIC_HIST_FONTCOLOR       1415
#define IDC_BTN_HIST_CHANGEFONT         1416
#define IDC_BTN_CHANGE_BGFOLDER         1417
#define IDC_BTN_CHANGE_BGLISTVIEW       1418
#define IDC_BTN_CHANGE_BGSNAPSHOT       1419
#define IDC_STATIC_BG_FOLDER            1420
#define IDC_STATIC_BG_LISTVIEW          1421
#define IDC_STATIC_BG_SNAPSHOT          1422
#define IDC_STATIC_MAINFONT             1423
#define IDC_STATIC_HISTORYFONT          1424
#define IDC_STATIC_HISTORYFONTINFO		  1425
#define IDC_BTN_DEL_FOLDERBG            1426
#define IDC_BTN_DEL_LISTBG              1427
#define IDC_BTN_DEL_SNAPSHOTBG          1428

#define IDC_DOKDO                       2000

//#ifdef MAME_AVI
#define IDD_AVI_STATUS                  1400
#define ID_FILE_PLAY_WITH_AVI           40200
#define ID_FILE_PLAY_BACK_AVI           40201
#define IDC_INIT                        40202
#define IDC_FRAME_CMP                   40203
#define IDC_EDIT2                       40204
#define IDC_SPIN1                       40205
#define IDC_WAVE_RECORD                 40206
#define IDC_LEFT                        40207
#define IDC_TOP                         40208
#define IDC_WIDTH                       40209
#define IDC_HEIGHT                      40210
#define IDC_LEFT_SPIN                   40211
#define IDC_WIDTH_SPIN                  40212
#define IDC_TOP_SPIN                    40213
#define IDC_HEIGHT_SPIN                 40214
#define IDC_AVIFRAMESKIP                40215
#define IDC_BITMAP_SIZE                 40216
#define IDC_HOUR                        40217
#define IDC_MINUTE                      40218
#define IDC_SECOND                      40219
#define IDC_FRAME_CMP_PRE               40220
#define IDC_FRAME_CMP_FEW               40221
#define IDC_COLOR_CNV_16TO8             40222
#define IDC_COLOR_CNV_16TO24            40223
#define IDC_CHECK2                      40224
#define IDC_INTERLACE                   40225
#define IDC_INTERLACE_ODD               40226
#define IDC_SET_TV_DISPLAY_SETTING      40227
#define IDC_AVI_LEFT                    40228
#define IDC_AVI_WIDTH                   40229
#define IDC_AVI_TOP                     40230
#define IDC_AVI_HEIGHT                  40231
#define IDC_AVISIZE_WIDTH               40232
#define IDC_AVISIZE_HEIGHT              40233
#define IDC_AVI_FILESIZE                40234
#define IDC_AVI_FILESIZE_CHECK_FRAME    40235
#define IDC_CHECK_FORCEFLIPY            40236
#define IDC_SMOOTH_RESIZE_X             40239
#define IDC_SMOOTH_RESIZE_Y             40240
#define IDC_AUDIO_RECORD_TYPE           40243
#define IDC_AUDIO_16BIT                 40244
#define IDC_AUDIO_STEREO                40245
#define IDC_AUDIO_SAMPLE_PER_SEC        40246
#define IDC_AUDIO_CMP                   40247
#define IDC_AUDIO_SRC_FORMAT            40248
#define IDC_AUDIO_DEST_FORMAT           40249
#define IDC_TEXT_AVI_HEIGHT_MUL2        40250
#define IDC_TEXT_AVI_TOP_MUL2           40251
#define IDC_TEXT_FPS_DIV2               40252
#define IDC_AVI_SAVEFILE_PAUSE          40253
#define IDC_AVI_POS_CENTER              40254
#define IDC_FPS                         40255
//#endif

//#ifdef USE_SAVE_GAMELIST
#define ID_FILE_GAMELIST                40300
#define ID_FILE_ROMSLIST                40310
//#endif /* USE_SAVE_GAMELIST */

#define IDC_STATIC                      -1

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NO_MFC                     1
#define _APS_3D_CONTROLS                1
#define _APS_NEXT_RESOURCE_VALUE        202
#define _APS_NEXT_COMMAND_VALUE         40300
#define _APS_NEXT_CONTROL_VALUE         1500
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
