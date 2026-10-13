/***************************************************************************
 
  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

 /***************************************************************************

  win32ui.c

  Win32 GUI code.

  Created 8/12/97 by Christopher Kirmse (ckirmse@ricochet.net)
  Additional code November 1997 by Jeff Miller (miller@aa.net)
  More July 1998 by Mike Haaland (mhaaland@hypertech.com)
  Added Spitters/Property Sheets/Removed Tabs/Added Tree Control in
  Nov/Dec 1998 - Mike Haaland

***************************************************************************/
#define MULTISESSION 0

#ifdef _MSC_VER
#ifndef NONAMELESSUNION
#define NONAMELESSUNION 
#endif
#endif

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <stdio.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <commctrl.h>
#include <commdlg.h>
#include <dlgs.h>
#include <string.h>
#include <sys/stat.h>
#include <wingdi.h>
#include <time.h>

#include "mame32.h"
#include "driver.h"
#include "osdepend.h"
#include "unzip.h"
#include "ui_pal.h"
#include "ui_lang.h"
#include "misc.h"


#include "resource.h"
#include "resource.hm"

#include "screenshot.h"
#include "M32Util.h"
#include "file.h"
#include "audit32.h"
#include "Directories.h"
#include "Properties.h"
#include "ColumnEdit.h"
#include "picker.h"
#include "tabview.h"
#include "bitmask.h"
#include "TreeView.h"
#include "Splitters.h"
#include "help.h"
#include "history.h"
#include "options.h"
#include "dialogs.h"
#include "state.h"
#include "windows/input.h"
#include "windows/config.h"
#include "windows/window.h"
#include "PaletteEdit.h"
#include "translate.h"

#include "DirectDraw.h"
#include "DirectInput.h"
#include "DIJoystick.h"     /* For DIJoystick avalibility. */

#ifndef LVS_EX_LABELTIP
#define LVS_EX_LABELTIP         0x00004000 // listview unfolds partly hidden labels if it does not have infotip text
#endif // LVS_EX_LABELTIP

#ifdef MAME_AVI
#include "video.h"	//"Display.h"
#include "avi.h"
#endif /* MAME_AVI */

#ifdef KAILLERA
#include "osdepend.h"
#include "chat.h"
#include "kailleraclient.h"
#include "kcommand.h"

int kPlay = 0;
int kPlayers = 0;
int kPlayerNB = 0;

static HWND hWndLoad = 0;
static HFONT hChatFont = NULL;     /* Font for kaillera chat */

typedef struct
{
	char name[200];
	int index;
} driver_title_type;

static driver_title_type *sorted_titles;
static int DriverTitleCompareFunc(const void *arg1,const void *arg2);
static void KailleraPlayRecordGame(int nGame);
#endif /* KAILLERA */

#ifdef SHOW_UIKEY
#include "win32ui_plus.h"	// SHOW_UIKEY
extern int config_load_default_ui(const char *lpszCfgDir, const struct InputPortDefinition *input_ports_backup, struct InputPortDefinition *input_ports);
extern struct InputPortDefinition *GetDefaultInputPortList(void);	// inptport.c
extern const int GetInputPortCount(void); // inptport.c
#endif /*SHOW_UIKEY*/

#ifdef DARKMAME
static HFONT hHistFont = NULL;     /* Font for history edit control */
#endif /*DARKMAME*/

// fix warning: cast does not match function type
#if defined(__GNUC__) && defined(ListView_CreateDragImage)
#undef ListView_CreateDragImage
#endif

#ifndef ListView_CreateDragImage
#define ListView_CreateDragImage(hwnd, i, lpptUpLeft) \
    (HIMAGELIST)(LRESULT)(int)SendMessage((hwnd), LVM_CREATEDRAGIMAGE, (WPARAM)(int)(i), (LPARAM)(LPPOINT)(lpptUpLeft))
#endif // ListView_CreateDragImage

#ifndef TreeView_EditLabel
#define TreeView_EditLabel(w, i) \
    SNDMSG(w,TVM_EDITLABEL,0,(LPARAM)(i))
#endif // TreeView_EditLabel

#ifndef HDF_SORTUP
#define HDF_SORTUP 0x400
#endif // HDF_SORTUP

#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN 0x200
#endif // HDF_SORTDOWN

#ifndef LVM_SETBKIMAGEA
#define LVM_SETBKIMAGEA         (LVM_FIRST + 68)
#endif // LVM_SETBKIMAGEA

#ifndef LVM_SETBKIMAGEW
#define LVM_SETBKIMAGEW         (LVM_FIRST + 138)
#endif // LVM_SETBKIMAGEW

#ifndef LVM_GETBKIMAGEA
#define LVM_GETBKIMAGEA         (LVM_FIRST + 69)
#endif // LVM_GETBKIMAGEA

#ifndef LVM_GETBKIMAGEW
#define LVM_GETBKIMAGEW         (LVM_FIRST + 139)
#endif // LVM_GETBKIMAGEW

#ifndef LVBKIMAGE

typedef struct tagLVBKIMAGEA
{
	ULONG ulFlags;
	HBITMAP hbm;
	LPSTR pszImage;
	UINT cchImageMax;
	int xOffsetPercent;
	int yOffsetPercent;
} LVBKIMAGEA, *LPLVBKIMAGEA;

typedef struct tagLVBKIMAGEW
{
	ULONG ulFlags;
	HBITMAP hbm;
	LPWSTR pszImage;
	UINT cchImageMax;
	int xOffsetPercent;
	int yOffsetPercent;
} LVBKIMAGEW, *LPLVBKIMAGEW;

#ifdef UNICODE
#define LVBKIMAGE               LVBKIMAGEW
#define LPLVBKIMAGE             LPLVBKIMAGEW
#define LVM_SETBKIMAGE          LVM_SETBKIMAGEW
#define LVM_GETBKIMAGE          LVM_GETBKIMAGEW
#else
#define LVBKIMAGE               LVBKIMAGEA
#define LPLVBKIMAGE             LPLVBKIMAGEA
#define LVM_SETBKIMAGE          LVM_SETBKIMAGEA
#define LVM_GETBKIMAGE          LVM_GETBKIMAGEA
#endif
#endif

#ifndef LVBKIF_SOURCE_NONE
#define LVBKIF_SOURCE_NONE      0x00000000
#endif // LVBKIF_SOURCE_NONE

#ifndef LVBKIF_SOURCE_HBITMAP
#define LVBKIF_SOURCE_HBITMAP   0x00000001
#endif

#ifndef LVBKIF_SOURCE_URL
#define LVBKIF_SOURCE_URL       0x00000002
#endif // LVBKIF_SOURCE_URL

#ifndef LVBKIF_SOURCE_MASK
#define LVBKIF_SOURCE_MASK      0x00000003
#endif // LVBKIF_SOURCE_MASK

#ifndef LVBKIF_STYLE_NORMAL
#define LVBKIF_STYLE_NORMAL     0x00000000
#endif // LVBKIF_STYLE_NORMAL

#ifndef LVBKIF_STYLE_TILE
#define LVBKIF_STYLE_TILE       0x00000010
#endif // LVBKIF_STYLE_TILE

#ifndef LVBKIF_STYLE_MASK
#define LVBKIF_STYLE_MASK       0x00000010
#endif // LVBKIF_STYLE_MASK

#ifndef ListView_SetBkImageA
#define ListView_SetBkImageA(hwnd, plvbki) \
    (BOOL)SNDMSG((hwnd), LVM_SETBKIMAGEA, 0, (LPARAM)(plvbki))
#endif // ListView_SetBkImageA

#ifndef ListView_GetBkImageA
#define ListView_GetBkImageA(hwnd, plvbki) \
    (BOOL)SNDMSG((hwnd), LVM_GETBKIMAGEA, 0, (LPARAM)(plvbki))
#endif // ListView_GetBkImageA

#define MM_PLAY_GAME (WM_APP + 15000)

#define JOYGUI_MS 100

#define JOYGUI_TIMER 1
#define SCREENSHOT_TIMER 2
#if MULTISESSION
#define GAMEWND_TIMER 3
#endif

/* Max size of a sub-menu */
#define DBU_MIN_WIDTH  292
#define DBU_MIN_HEIGHT 190

int MIN_WIDTH  = DBU_MIN_WIDTH;
int MIN_HEIGHT = DBU_MIN_HEIGHT;

/* Max number of bkground picture files */
#define MAX_BGFILES 100

#define NO_FOLDER -1
#define STATESAVE_VERSION 1

typedef BOOL (WINAPI *common_file_dialog_procW)(LPOPENFILENAMEW lpofn);
typedef BOOL (WINAPI *common_file_dialog_procA)(LPOPENFILENAMEA lpofn);

#ifdef MAME_AVI
#include "AVI.h"
#include "WAV.h"
static void     MamePlayGameAVI(void);
static void     MamePlayBackGameAVI(void);
static BOOL     CommonFileDialogAVI(common_file_dialog_procA cfd,char *filename);
static char		last_directory_avi[MAX_PATH];
int				_nAviNo = 0;
extern void *record;
extern void *playback;
#endif /* MAME_AVI */

#ifdef USE_SAVE_GAMELIST
static void SaveGameListToFile(char *szFile, int Formatted);
static void SaveRomsListToFile(char *szFile);
#endif /* USE_SAVE_GAMELIST */

/***************************************************************************
 externally defined global variables
 ***************************************************************************/
#if 0
extern const char g_szPlayGameString[] = "&Play %s";
extern const char g_szGameCountString[] = "%d games";
extern const char *history_filename;
extern const char *mameinfo_filename;
#endif

/***************************************************************************
    function prototypes
 ***************************************************************************/

static BOOL             Win32UI_init(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow);
static void             Win32UI_exit(void);

static BOOL             PumpMessage(void);
static BOOL             OnIdle(HWND hWnd);
static void             OnSize(HWND hwnd, UINT state, int width, int height);
static long WINAPI      MameWindowProc(HWND hwnd,UINT message,UINT wParam,LONG lParam);

static void             SetView(int menu_id);
static void             ResetListView(void);
static void             UpdateGameList(void);
static void             DestroyIcons(void);
static void             ReloadIcons(void);
static void             PollGUIJoystick(void);
#if 0
static void             PressKey(HWND hwnd, UINT vk);
#endif
static BOOL             MameCommand(HWND hwnd,int id, HWND hwndCtl, UINT codeNotify);
static void             KeyboardKeyDown(int syskey, int vk_code, int special);
static void             KeyboardKeyUp(int syskey, int vk_code, int special);
static void             KeyboardStateClear(void);

static void             UpdateStatusBar(void);
static BOOL             PickerHitTest(HWND hWnd);
static BOOL             TreeViewNotify(NMHDR *nm);

static void             ResetBackground(char *szFile);
static void             RandomSelectBackground(void);
static void             LoadBackgroundBitmap(void);
static void             PaintBackgroundImage(HWND hWnd, HRGN hRgn, int x, int y);

static int CLIB_DECL    DriverDataCompareFunc(const void *arg1,const void *arg2);
static int              GamePicker_Compare(HWND hwndPicker, int index1, int index2, int sort_subitem);

static void             DisableSelection(void);
static void             EnableSelection(int nGame);

static int              GetSelectedPick(void);
static HICON            GetSelectedPickItemIcon(void);
static void             SetRandomPickItem(void);
static void             PickColor(COLORREF *cDefault);

#if 1 //INHERIT
static LPTREEFOLDER     GetSelectedFolder(void);
static HICON            GetSelectedFolderIcon(void);
#endif /* INHERIT */

static LRESULT CALLBACK HistoryWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK PictureFrameWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK PictureWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//static INT_PTR CALLBACK LanguageDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

//static BOOL             SelectLanguageFile(HWND hWnd, TCHAR* filename);
static void             ChangeLanguage(int id);
static void             MamePlayRecordGame(void);
static void             MamePlayBackGame(void);
static void             MamePlayRecordWave(void);
static void             MameLoadState(void);
static BOOL             CommonFileDialogW(common_file_dialog_procW cfd,char *filename, int filetype);
static BOOL             CommonFileDialogA(common_file_dialog_procA cfd,char *filename, int filetype);
static BOOL             CommonFileDialog(BOOL open_for_write,char *filename, int filetype);
static void             MamePlayGame(void);
static void             MamePlayGameWithOptions(int nGame);
static INT_PTR CALLBACK LoadProgressDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
static int              UpdateLoadProgress(const char* name, const struct rom_load_data *romdata);
static BOOL             GameCheck(void);
static BOOL             FolderCheck(void);

static void             ToggleScreenShot(void);
static void             AdjustMetrics(void);
static void             EnablePlayOptions(int nIndex, options_type *o);

/* Icon routines */
static DWORD            GetShellLargeIconSize(void);
static DWORD            GetShellSmallIconSize(void);
static void             CreateIcons(void);
static int              GetIconForDriver(int nItem);
static void             AddDriverIcon(int nItem,int default_icon_index);

// Context Menu handlers
static void             UpdateMenu(HMENU hMenu);
static void             InitTreeContextMenu(HMENU hTreeMenu);
static void             ToggleShowFolder(int folder);
static BOOL             HandleTreeContextMenu( HWND hWnd, WPARAM wParam, LPARAM lParam);
static BOOL             HandleScreenShotContextMenu( HWND hWnd, WPARAM wParam, LPARAM lParam);
static void             GamePicker_OnHeaderContextMenu(POINT pt, int nColumn);
static void             GamePicker_OnBodyContextMenu(POINT pt);

static void             InitListView(void);
/* Re/initialize the ListView header columns */
static void             ResetColumnDisplay(BOOL first_time);

static void             CopyToolTipTextW(LPTOOLTIPTEXTW lpttt);
static void             CopyToolTipTextA(LPTOOLTIPTEXTA lpttt);

static void             ProgressBarShow(void);
static void             ProgressBarHide(void);
static void             ResizeProgressBar(void);
static void             ProgressBarStep(void);
static void             ProgressBarStepParam(int iGameIndex, int nGameCount);

static HWND             InitProgressBar(HWND hParent);
static HWND             InitToolbar(HWND hParent);
static HWND             InitStatusBar(HWND hParent);

static LRESULT          Statusbar_MenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam);

static void             UpdateHistory(void);

#ifdef SMOOTH_SCROLL 
/* Custom Draw item */
static void             SmoothScroll_DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
#endif /* SMOOTH_SCROLL */

void RemoveCurrentGameCustomFolder(void);
void RemoveGameCustomFolder(int driver_index);

void BeginListViewDrag(NM_LISTVIEW *pnmv);
void MouseMoveListViewDrag(POINTS pt);
void ButtonUpListViewDrag(POINTS p);

void CalculateBestScreenShotRect(HWND hWnd, RECT *pRect, BOOL restrict_height);

BOOL MouseHasBeenMoved(void);
void SwitchFullScreenMode(void);

// Game Window Communication Functions
#if MULTISESSION
BOOL SendMessageToEmulationWindow(UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL SendIconToEmulationWindow(int nGameIndex);
HWND GetGameWindow(void);
#else
void SendMessageToProcess(LPPROCESS_INFORMATION lpProcessInformation,
                                             UINT Msg, WPARAM wParam, LPARAM lParam);
void SendIconToProcess(LPPROCESS_INFORMATION lpProcessInformation, int nGameIndex);
HWND GetGameWindow(LPPROCESS_INFORMATION lpProcessInformation);
#endif

static BOOL CALLBACK EnumWindowCallBack(HWND hwnd, LPARAM lParam);
/***************************************************************************
    External variables
 ***************************************************************************/

/***************************************************************************
    Internal structures
 ***************************************************************************/

/*
 * These next two structs represent how the icon information
 * is stored in an ICO file.
 */
typedef struct
{
	BYTE    bWidth;               /* Width of the image */
	BYTE    bHeight;              /* Height of the image (times 2) */
	BYTE    bColorCount;          /* Number of colors in image (0 if >=8bpp) */
	BYTE    bReserved;            /* Reserved */
	WORD    wPlanes;              /* Color Planes */
	WORD    wBitCount;            /* Bits per pixel */
	DWORD   dwBytesInRes;         /* how many bytes in this resource? */
	DWORD   dwImageOffset;        /* where in the file is this image */
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
	UINT            Width, Height, Colors; /* Width, Height and bpp */
	LPBYTE          lpBits;                /* ptr to DIB bits */
	DWORD           dwNumBytes;            /* how many bytes? */
	LPBITMAPINFO    lpbi;                  /* ptr to header */
	LPBYTE          lpXOR;                 /* ptr to XOR image bits */
	LPBYTE          lpAND;                 /* ptr to AND image bits */
} ICONIMAGE, *LPICONIMAGE;

/* Which edges of a control are anchored to the corresponding side of the parent window */
#define RA_LEFT     0x01
#define RA_RIGHT    0x02
#define RA_TOP      0x04
#define RA_BOTTOM   0x08
#define RA_ALL      0x0F

#define RA_END  0
#define RA_ID   1
#define RA_HWND 2

typedef struct
{
	int         type;       /* Either RA_ID or RA_HWND, to indicate which member of u is used; or RA_END
				   to signify last entry */
	union                   /* Can identify a child window by control id or by handle */
	{
		int     id;         /* Window control id */
		HWND    hwnd;       /* Window handle */
	} u;
	BOOL		setfont;	/* Do we set this item's font? */
	int         action;     /* What to do when control is resized */
	void        *subwindow; /* Points to a Resize structure for this subwindow; NULL if none */
} ResizeItem;

typedef struct
{
	RECT        rect;       /* Client rect of window; must be initialized before first resize */
	ResizeItem* items;      /* Array of subitems to be resized */
} Resize;

static void ResizeWindow(HWND hParent, Resize *r);
static void SetAllWindowsFont(HWND hParent, const Resize *r, HFONT hFont, BOOL bRedraw);

/* List view Icon defines */
#define LG_ICONMAP_WIDTH    GetSystemMetrics(SM_CXICON)
#define LG_ICONMAP_HEIGHT   GetSystemMetrics(SM_CYICON)
#define ICONMAP_WIDTH       GetSystemMetrics(SM_CXSMICON)
#define ICONMAP_HEIGHT      GetSystemMetrics(SM_CYSMICON)

/*
typedef struct tagPOPUPSTRING
{
	HMENU hMenu;
	UINT uiString;
} POPUPSTRING;

#define MAX_MENUS 3
*/

#define SPLITTER_WIDTH	4
#define MIN_VIEW_WIDTH	10

// Struct needed for Game Window Communication

typedef struct
{
	LPPROCESS_INFORMATION ProcessInfo;
	HWND hwndFound;
} FINDWINDOWHANDLE;

/***************************************************************************
    Internal variables
 ***************************************************************************/

static HWND   hMain  = NULL;
static HACCEL hAccel = NULL;

static HWND hwndList  = NULL;
static HWND hTreeView = NULL;
static HWND hProgWnd  = NULL;
static HWND hTabCtrl  = NULL;

static BOOL g_bAbortLoading = FALSE; /* doesn't work right */
static BOOL g_bCloseLoading = FALSE;

static HINSTANCE hInst = NULL;

static HFONT hFont = NULL;     /* Font for list view */

static int game_count = 0;

/* global data--know where to send messages */
static BOOL in_emulation;

/* idle work at startup */
static BOOL idle_work;

static int  game_index;
static int  progBarStep;

static BOOL bDoGameCheck = FALSE;

/* Tree control variables */
static BOOL bShowTree      = 1;
static BOOL bShowToolBar   = 1;
static BOOL bShowStatusBar = 1;
static BOOL bShowTabCtrl   = 1;
static BOOL bProgressShown = FALSE;
static BOOL bListReady     = FALSE;

/* use a joystick subsystem in the gui? */
static struct OSDJoystick* g_pJoyGUI = NULL;

/* store current keyboard state (in internal codes) here */
static input_code_t keyboard_state[ __code_max ]; /* __code_max #defines the number of internal key_codes */

/* table copied from windows/inputs.c */
// table entry indices
#define MAME_KEY		0
#define DI_KEY			1
#define VIRTUAL_KEY		2
#define ASCII_KEY		3

typedef struct
{
	char		name[40];	    // functionality name (optional)
	input_seq_t	is;				// the input sequence (the keys pressed)
	UINT		func_id;        // the identifier
	input_seq_t* (*getiniptr)(void);// pointer to function to get the value from .ini file
} GUISequence;

static GUISequence GUISequenceControl[]=
{
	{"gui_key_up",                SEQ_DEF_0,    ID_UI_UP,           Get_ui_key_up },
	{"gui_key_down",              SEQ_DEF_0,    ID_UI_DOWN,         Get_ui_key_down },
	{"gui_key_left",              SEQ_DEF_0,    ID_UI_LEFT,         Get_ui_key_left },
	{"gui_key_right",             SEQ_DEF_0,    ID_UI_RIGHT,        Get_ui_key_right },
	{"gui_key_start",             SEQ_DEF_0,    ID_UI_START,        Get_ui_key_start },
	{"gui_key_pgup",              SEQ_DEF_0,    ID_UI_PGUP,         Get_ui_key_pgup },
	{"gui_key_pgdwn",             SEQ_DEF_0,    ID_UI_PGDOWN,       Get_ui_key_pgdwn },
	{"gui_key_home",              SEQ_DEF_0,    ID_UI_HOME,         Get_ui_key_home },
	{"gui_key_end",               SEQ_DEF_0,    ID_UI_END,          Get_ui_key_end },
	{"gui_key_ss_change",         SEQ_DEF_0,    IDC_SSFRAME,        Get_ui_key_ss_change },
	{"gui_key_history_up",        SEQ_DEF_0,    ID_UI_HISTORY_UP,   Get_ui_key_history_up },
	{"gui_key_history_down",      SEQ_DEF_0,    ID_UI_HISTORY_DOWN, Get_ui_key_history_down },
	
	{"gui_key_context_filters",    SEQ_DEF_0,    ID_CONTEXT_FILTERS,       Get_ui_key_context_filters },
	{"gui_key_select_random",      SEQ_DEF_0,    ID_CONTEXT_SELECT_RANDOM, Get_ui_key_select_random },
	{"gui_key_game_audit",         SEQ_DEF_0,    ID_GAME_AUDIT,            Get_ui_key_game_audit },
	{"gui_key_game_properties",    SEQ_DEF_0,    ID_GAME_PROPERTIES,       Get_ui_key_game_properties },
	{"gui_key_help_contents",      SEQ_DEF_0,    ID_HELP_CONTENTS,         Get_ui_key_help_contents },
	{"gui_key_update_gamelist",    SEQ_DEF_0,    ID_UPDATE_GAMELIST,       Get_ui_key_update_gamelist },
	{"gui_key_view_folders",       SEQ_DEF_0,    ID_VIEW_FOLDERS,          Get_ui_key_view_folders },
	{"gui_key_view_fullscreen",    SEQ_DEF_0,    ID_VIEW_FULLSCREEN,       Get_ui_key_view_fullscreen },
	{"gui_key_view_pagetab",       SEQ_DEF_0,    ID_VIEW_PAGETAB,          Get_ui_key_view_pagetab },
	{"gui_key_view_picture_area",  SEQ_DEF_0,    ID_VIEW_PICTURE_AREA,     Get_ui_key_view_picture_area },
	{"gui_key_view_status",        SEQ_DEF_0,    ID_VIEW_STATUS,           Get_ui_key_view_status },
	{"gui_key_view_toolbars",      SEQ_DEF_0,    ID_VIEW_TOOLBARS,         Get_ui_key_view_toolbars },

	{"gui_key_view_tab_cabinet",     SEQ_DEF_0,  ID_VIEW_TAB_CABINET,       Get_ui_key_view_tab_cabinet },
	{"gui_key_view_tab_cpanel",      SEQ_DEF_0,  ID_VIEW_TAB_CONTROL_PANEL, Get_ui_key_view_tab_cpanel },
	{"gui_key_view_tab_flyer",       SEQ_DEF_0,  ID_VIEW_TAB_FLYER,         Get_ui_key_view_tab_flyer },
	{"gui_key_view_tab_history",     SEQ_DEF_0,  ID_VIEW_TAB_HISTORY,       Get_ui_key_view_tab_history },
	{"gui_key_view_tab_marquee",     SEQ_DEF_0,  ID_VIEW_TAB_MARQUEE,       Get_ui_key_view_tab_marquee },
	{"gui_key_view_tab_screenshot",  SEQ_DEF_0,  ID_VIEW_TAB_SCREENSHOT,    Get_ui_key_view_tab_screenshot },
	{"gui_key_view_tab_title",       SEQ_DEF_0,  ID_VIEW_TAB_TITLE,         Get_ui_key_view_tab_title },
	{"gui_key_quit",                 SEQ_DEF_0,  ID_FILE_EXIT,              Get_ui_key_quit },
};


#define NUM_GUI_SEQUENCES (sizeof(GUISequenceControl) / sizeof(GUISequenceControl[0]))


static UINT    lastColumnClick   = 0;
static WNDPROC g_lpHistoryWndProc = NULL;
static WNDPROC g_lpPictureFrameWndProc = NULL;
static WNDPROC g_lpPictureWndProc = NULL;

/*
static POPUPSTRING popstr[MAX_MENUS + 1];
*/

static int *parent_index = NULL;

/* Tool and Status bar variables */
static HWND hStatusBar = 0;
static HWND hToolBar   = 0;

/* Column Order as Displayed */
static BOOL oldControl = FALSE;
static BOOL xpControl = FALSE;

/* Used to recalculate the main window layout */
static int  bottomMargin;
static int  topMargin;
static int  have_history = FALSE;

static BOOL have_selection = FALSE;

static HBITMAP hMissing_bitmap;

/* Icon variables */
static HIMAGELIST   hLarge = NULL;
static HIMAGELIST   hSmall = NULL;
static HIMAGELIST   hHeaderImages = NULL;
static int          *icon_index = NULL; /* for custom per-game icons */

static TBBUTTON tbb[] =
{
	{0, ID_VIEW_FOLDERS,     TBSTATE_ENABLED, TBSTYLE_CHECK,      {0, 0}, 0, 0},
	{1, ID_VIEW_PICTURE_AREA,TBSTATE_ENABLED, TBSTYLE_CHECK,      {0, 0}, 0, 1},
	{0, 0,                   TBSTATE_ENABLED, TBSTYLE_SEP,        {0, 0}, 0, 0},
	{2, ID_VIEW_LARGE_ICON,  TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 2},
	{3, ID_VIEW_SMALL_ICON,  TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 3},
	{4, ID_VIEW_LIST_MENU,   TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 4},
	{5, ID_VIEW_DETAIL,      TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 5},
	{6, ID_VIEW_GROUPED,     TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 6},
	{0, 0,                   TBSTATE_ENABLED, TBSTYLE_SEP,        {0, 0}, 0, 0},
	{7, ID_OPTIONS_DEFAULTS, TBSTATE_ENABLED, TBSTYLE_BUTTON,     {0, 0}, 0, 7},
	{8, ID_OPTIONS_DIR,      TBSTATE_ENABLED, TBSTYLE_BUTTON,    {0, 0}, 0,8},
	{9, ID_OPTIONS_VIEW_UIKEY,TBSTATE_ENABLED, TBSTYLE_BUTTON,    {0, 0}, 0,9},
	{10, ID_GAME_FILTERS,      TBSTATE_ENABLED, TBSTYLE_BUTTON,    {0, 0}, 0,8},
	{11, IDC_USE_LIST,        TBSTATE_ENABLED, TBSTYLE_CHECK,      {0, 0}, 0, 10},
	{0, 0,                   TBSTATE_ENABLED, TBSTYLE_SEP,        {0, 0}, 0, 0},
	{12, ID_HELP_ABOUT,       TBSTATE_ENABLED, TBSTYLE_BUTTON,     {0, 0}, 0, 11},
	{13, ID_HELP_CONTENTS,    TBSTATE_ENABLED, TBSTYLE_BUTTON,     {0, 0}, 0, 12}
};

#define NUM_TOOLBUTTONS (sizeof(tbb) / sizeof(tbb[0]))

#define NUM_TOOLTIPS 13 + 1

static char szTbStrings[NUM_TOOLTIPS + 1][30] =
{
	"Toggle Folder List",
	"Toggle Screen Shot",
	"Large Icons",
	"Small Icons",
	"List",
	"Details",
	"Grouped",
	"Default Game Options",
	"Directories...",
	"View in-game UI key list",
	"Game Find",
	"Use Local Language Game List",
	"About",
	"Help"
};

static int CommandToString[] =
{
	ID_VIEW_FOLDERS,
	ID_VIEW_PICTURE_AREA,
	ID_VIEW_LARGE_ICON,
	ID_VIEW_SMALL_ICON,
	ID_VIEW_LIST_MENU,
	ID_VIEW_DETAIL,
	ID_VIEW_GROUPED,
	ID_OPTIONS_DEFAULTS,
	ID_OPTIONS_DIR,
	ID_OPTIONS_VIEW_UIKEY,	
	ID_GAME_FILTERS,
	IDC_USE_LIST,
	ID_HELP_ABOUT,
	ID_HELP_CONTENTS,
	-1
};

/* How to resize main window */
static ResizeItem main_resize_items[] =
{
	{ RA_HWND, { 0 },            FALSE, RA_LEFT  | RA_RIGHT  | RA_TOP,     NULL },
	{ RA_HWND, { 0 },            FALSE, RA_LEFT  | RA_RIGHT  | RA_BOTTOM,  NULL },
	{ RA_ID,   { IDC_DIVIDER },  FALSE, RA_LEFT  | RA_RIGHT  | RA_TOP,     NULL },
	{ RA_ID,   { IDC_TREE },     TRUE,	RA_LEFT  | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_LIST },     TRUE,	RA_ALL,                            NULL },
	{ RA_ID,   { IDC_SPLITTER }, FALSE,	RA_LEFT  | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_SPLITTER2 },FALSE,	RA_RIGHT | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_SSFRAME },  FALSE,	RA_RIGHT | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_SSPICTURE },FALSE,	RA_RIGHT | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_HISTORY },  TRUE,	RA_RIGHT | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_SSTAB },    FALSE,	RA_RIGHT | RA_TOP,                 NULL },
	{ RA_END,  { 0 },            FALSE, 0,                                 NULL }
};

static Resize main_resize = { {0, 0, 0, 0}, main_resize_items };

/* our dialog/configured options */
static options_type playing_game_options;

/* last directory for common file dialogs */
static char last_directory[MAX_PATH];

/* system-wide window message sent out with an ATOM of the current game name
   each time it changes */
static UINT g_mame32_message = 0;
static BOOL g_bDoBroadcast   = FALSE;

static BOOL use_gui_romloading = FALSE;

static BOOL g_listview_dragging = FALSE;
HIMAGELIST himl_drag;
int game_dragged; /* which game started the drag */
HTREEITEM prev_drag_drop_target; /* which tree view item we're currently highlighting */

static BOOL g_in_treeview_edit = FALSE;

typedef struct
{
	const char *name;
	int index;
} driver_data_type;
static driver_data_type *sorted_drivers;

static char * g_pRecordName = NULL;
static char * g_pPlayBkName = NULL;
static char * g_pSaveStateName = NULL;
static char * g_pRecordWaveName = NULL;
static char * override_playback_directory = NULL;
static char * override_savestate_directory = NULL;

/***************************************************************************
    Global variables
 ***************************************************************************/

/* Background Image handles also accessed from TreeView.c */
static HPALETTE         hPALbg   = 0;
static HBITMAP          hBackground  = 0;
static MYBITMAPINFO     bmDesc;

#ifdef DARKMAME	// HISTORY_BG
// Folder background
static HPALETTE         hFolderPALbg   = 0;
static HBITMAP          hFolderBackground  = 0;
static MYBITMAPINFO     bmFolderDesc;
// Snapshot background
static HPALETTE         hSnapPALbg   = 0;
static HBITMAP          hSnapBackground  = 0;
static MYBITMAPINFO     bmSnapDesc;
#endif /*DARKMAME*/

/* List view Column text */
const char* column_names[COLUMN_MAX] =
{
	"Game",
	"ROMs",
	"Samples",
	"Directory",
	"Type",
	"Trackball",
	"Played",
	"Manufacturer",
	"Year",
	"Clone Of",
	"Source",
	"Play Time"
};

/***************************************************************************
    Message Macros
 ***************************************************************************/

#ifndef StatusBar_GetItemRect
#define StatusBar_GetItemRect(hWnd, iPart, lpRect) \
    SendMessage(hWnd, SB_GETRECT, (WPARAM) iPart, (LPARAM) (LPRECT) lpRect)
#endif

#ifndef ToolBar_CheckButton
#define ToolBar_CheckButton(hWnd, idButton, fCheck) \
    SendMessage(hWnd, TB_CHECKBUTTON, (WPARAM)idButton, (LPARAM)MAKELONG(fCheck, 0))
#endif

/***************************************************************************
    External functions
 ***************************************************************************/

static int GetTotalPathLen(void)
{
	int len = 0;

	len += strlen(GetRomDirs());
	len += strlen(GetSampleDirs());
	len += strlen(GetIniDir());
	len += strlen(GetCfgDir());
	len += strlen(GetNvramDir());
	len += strlen(GetMemcardDir());

	if (override_playback_directory != NULL)
		len += strlen(override_playback_directory);
	else
		len += strlen(GetInpDir());

	len += strlen(GetHiDir());
	len += strlen(GetStateDir());
	len += strlen(GetArtDir());
	len += strlen(GetImgDir());
	len += strlen(GetDiffDir());
	len += strlen(GetCheatFileName());
	len += strlen(GetHistoryFileName());
	len += strlen(GetMAMEInfoFileName());
	len += strlen(GetHiscoreFileName());
	len += strlen(GetCtrlrDir());
#ifdef ROM_PATCH
	len += strlen(GetPatchDir());
#endif /* ROM_PATCH */
	len += strlen(GetLocalizedDataDir());

	if (g_pPlayBkName != NULL)
		len += strlen(g_pPlayBkName);
	if (g_pRecordName != NULL)
		len += strlen(g_pRecordName);

	return len;
}

#ifdef ZETAMAME32 // Interface
INLINE int IsGamePerfect(int idx)
{
    return !(drivers[idx]->flags & (
			GAME_NOT_WORKING
			| GAME_UNEMULATED_PROTECTION
#ifndef _DEBUG
			| GAME_WRONG_COLORS
			| GAME_IMPERFECT_COLORS
			| GAME_IMPERFECT_GRAPHICS
			| GAME_IMPERFECT_SOUND
#endif
#ifdef GAME_UNSIMULATED_HARDWARE // MAGE
			| GAME_UNSIMULATED_HARDWARE
#endif
		));
}

INLINE int IsGameImperfect(int idx)
{
    return (drivers[idx]->flags & (
			GAME_NOT_WORKING
			| GAME_UNEMULATED_PROTECTION
			| GAME_WRONG_COLORS
			| GAME_IMPERFECT_COLORS
			| GAME_IMPERFECT_GRAPHICS
			| GAME_NO_COCKTAIL	/* screen flip support is missing */
			| GAME_NO_SOUND
			| GAME_IMPERFECT_SOUND
#ifdef GAME_UNSIMULATED_HARDWARE // MAGE
			| GAME_UNSIMULATED_HARDWARE
#endif
		));
}

INLINE int IsGameWorking(int idx)
{
    return !(drivers[idx]->flags & (
			GAME_NOT_WORKING
			| GAME_UNEMULATED_PROTECTION
#ifdef GAME_UNSIMULATED_HARDWARE // MAGE
			| GAME_UNSIMULATED_HARDWARE
#endif
		));
}
#endif /* ZETAMAME32 */

static void CreateCommandLine(int nGameIndex, char* pCmdLine)
{
	char pModule[_MAX_PATH];
	options_type* pOpts;

	// this command line can grow too long for win9x, so we try to not send
	// some default values

	GetModuleFileNameA(GetModuleHandle(NULL), pModule, _MAX_PATH);

	pOpts = GetGameOptions(nGameIndex);

	sprintf(pCmdLine, "%s %s -norc", pModule, drivers[nGameIndex]->name);

	sprintf(&pCmdLine[strlen(pCmdLine)], " -rp \"%s\"",            GetRomDirs());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -sp \"%s\"",         GetSampleDirs());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -inipath \"%s\"",			GetIniDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -cfg_directory \"%s\"",      GetCfgDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -nvram_directory \"%s\"",    GetNvramDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -memcard_directory \"%s\"",  GetMemcardDir());

	if (override_playback_directory != NULL)
	{
		sprintf(&pCmdLine[strlen(pCmdLine)], " -input_directory \"%s\"",    override_playback_directory);
	}
	else
		sprintf(&pCmdLine[strlen(pCmdLine)], " -input_directory \"%s\"",    GetInpDir());

	sprintf(&pCmdLine[strlen(pCmdLine)], " -hiscore_directory \"%s\"",  GetHiDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -state_directory \"%s\"",    GetStateDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -artwork_directory \"%s\"",  GetArtDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -snapshot_directory \"%s\"", GetImgDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -diff_directory \"%s\"",     GetDiffDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -cheat_file \"%s\"",         GetCheatFileName());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -history_file \"%s\"",       GetHistoryFileName());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -mameinfo_file \"%s\"",      GetMAMEInfoFileName());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -hiscore_file \"%s\"",       GetHiscoreFileName());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -ctrlr_directory \"%s\"",    GetCtrlrDir());
#ifdef ROM_PATCH
	sprintf(&pCmdLine[strlen(pCmdLine)], " -ips_directory \"%s\"",      GetPatchDir());
#endif /* ROM_PATCH */
#ifdef LOG_WAV
	sprintf(&pCmdLine[strlen(pCmdLine)], " -wav_directory \"%s\"",      GetWavDir());
#endif /*LOG_WAV*/
	sprintf(&pCmdLine[strlen(pCmdLine)], " -localized_datapath \"%s\"", GetLocalizedDataDir());

	/* video */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sautoframeskip",           pOpts->autoframeskip   ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -frameskip %d",              pOpts->frameskip);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%swaitvsync",               pOpts->wait_vsync      ? "" : "no");
	if (pOpts->use_triplebuf)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%striplebuffer",            pOpts->use_triplebuf   ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%swindow",                  pOpts->window_mode     ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sddraw",                   pOpts->use_ddraw       ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%shwstretch",               pOpts->ddraw_stretch   ? "" : "no");
	if (pOpts->screen_name)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -screen_name %s",            pOpts->screen_name);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -resolution %s",             pOpts->resolution);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -refresh %d",                pOpts->gfx_refresh);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sscanlines",               pOpts->scanlines       ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sswitchres",               pOpts->switchres       ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sswitchbpp",               pOpts->switchbpp       ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%smaximize",                pOpts->maximize        ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%skeepaspect",              pOpts->keepaspect      ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%smatchrefresh",            pOpts->matchrefresh    ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssyncrefresh",             pOpts->syncrefresh     ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sthrottle",                pOpts->throttle        ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -full_screen_brightness %f", pOpts->gfx_brightness);
//	sprintf(&pCmdLine[strlen(pCmdLine)], " -frames_to_run %d",          pOpts->frames_to_display);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -effect %s",                 pOpts->effect);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -screen_aspect %s",          pOpts->aspect);

	sprintf(&pCmdLine[strlen(pCmdLine)], " -cleanstretch %s",           pOpts->clean_stretch);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -zoom %i", pOpts->zoom);
#ifdef USE_SCALE_EFFECTS
#ifdef USE_VIDEO_MEMORY
	if (pOpts->scale_effect)
	{ 
	sprintf(&pCmdLine[strlen(pCmdLine)], " -scale_effect %s",           pOpts->scale_effect);
	   if (pOpts->use_ddraw && pOpts->use_videomemory)
			   sprintf(&pCmdLine[strlen(pCmdLine)], " -%svm",                  pOpts->use_videomemory ? "" : "no");
	}
#else	/*USE_VIDEO_MEMORY*/
  sprintf(&pCmdLine[strlen(pCmdLine)], " -scale_effect %s",           pOpts->scale_effect);
#endif /*USE_VIDEO_MEMORY*/
#endif /* USE_SCALE_EFFECTS */

	// d3d
	if (pOpts->use_d3d)
	{
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3d");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dfilter %i",      pOpts->d3d_filter);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sd3dtexmanage",    pOpts->d3d_texture_management ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sd3deffectrotate", pOpts->d3d_rotate_effects ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3deffect %s",      pOpts->d3d_effect);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dprescale %s",    pOpts->d3d_prescale);

		if (pOpts->d3d_scanlines_enable)
			sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dscan %i",pOpts->d3d_scanlines);
		if (pOpts->d3d_feedback_enable)
			sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dfeedback %i",pOpts->d3d_feedback);
	}
	else
		sprintf(&pCmdLine[strlen(pCmdLine)], " -nod3d");

#ifdef UI_COLOR_DISPLAY
	sprintf(&pCmdLine[strlen(pCmdLine)], " -font_blank \"%s\"",         GetUIPaletteString(FONT_COLOR_BLANK));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -font_normal \"%s\"",        GetUIPaletteString(FONT_COLOR_NORMAL));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -font_special \"%s\"",       GetUIPaletteString(FONT_COLOR_SPECIAL));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -system_background \"%s\"",  GetUIPaletteString(SYSTEM_COLOR_BACKGROUND));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -system_framemedium \"%s\"", GetUIPaletteString(SYSTEM_COLOR_FRAMEMEDIUM));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -system_framelight \"%s\"",  GetUIPaletteString(SYSTEM_COLOR_FRAMELIGHT));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -system_framedark \"%s\"",   GetUIPaletteString(SYSTEM_COLOR_FRAMEDARK));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -osdbar_framemedium \"%s\"", GetUIPaletteString(OSDBAR_COLOR_FRAMEMEDIUM));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -osdbar_framelight \"%s\"",  GetUIPaletteString(OSDBAR_COLOR_FRAMELIGHT));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -osdbar_framedark \"%s\"",   GetUIPaletteString(OSDBAR_COLOR_FRAMEDARK));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -osdbar_defaultbar \"%s\"",  GetUIPaletteString(OSDBAR_COLOR_DEFAULTBAR));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_red \"%s\"",         GetUIPaletteString(BUTTON_COLOR_RED));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_yellow \"%s\"",      GetUIPaletteString(BUTTON_COLOR_YELLOW));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_green \"%s\"",       GetUIPaletteString(BUTTON_COLOR_GREEN));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_blue \"%s\"",        GetUIPaletteString(BUTTON_COLOR_BLUE));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_purple \"%s\"",      GetUIPaletteString(BUTTON_COLOR_PURPLE));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_pink \"%s\"",        GetUIPaletteString(BUTTON_COLOR_PINK));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_aqua \"%s\"",        GetUIPaletteString(BUTTON_COLOR_AQUA));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_silver \"%s\"",      GetUIPaletteString(BUTTON_COLOR_SILVER));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_navy \"%s\"",        GetUIPaletteString(BUTTON_COLOR_NAVY));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_lime \"%s\"",        GetUIPaletteString(BUTTON_COLOR_LIME));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -cursor \"%s\"",             GetUIPaletteString(CURSOR_COLOR));
#endif /* UI_COLOR_DISPLAY */
#ifdef USE_VIRTUAL_3D_VIEW
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%svirtual3d",        pOpts->d3d_virtual3d ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3drotation %d",     pOpts->d3d_rotation);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dzoomx %f",        pOpts->d3d_zoomx);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dzoomy %f",        pOpts->d3d_zoomy);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dkeystoneratio %s",pOpts->d3d_keystone);
#endif /* USE_VIRTUAL_3D_VIEW */

	/* input */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%smouse",                   pOpts->use_mouse       ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sjoystick",                pOpts->use_joystick    ? "" : "no");
	if (pOpts->use_joystick)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -a2d %f",                    pOpts->f_a2d);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssteadykey",               pOpts->steadykey       ? "" : "no");

	if (pOpts->lightgun)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%slightgun",                pOpts->lightgun        ? "" : "no");
	if (pOpts->dual_lightgun)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sdual_lightgun",           pOpts->dual_lightgun ? "" : "no");
	if (pOpts->offscreen_reload)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%soffscreen_reload",        pOpts->offscreen_reload ? "" : "no");

#ifdef USE_JOY_MOUSE_MOVE
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sstickpoint",              pOpts->use_stickpoint  ? "" : "no");
#endif /* USE_JOY_MOUSE_MOVE */
	if (strlen(pOpts->ctrlr) > 0)
	sprintf(&pCmdLine[strlen(pCmdLine)], " -ctrlr \"%s\"",              pOpts->ctrlr);
#ifdef JOYSTICK_ID
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid1 %d",                 pOpts->joyid[0]);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid2 %d",                 pOpts->joyid[1]);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid3 %d",                 pOpts->joyid[2]);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid4 %d",                 pOpts->joyid[3]);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid5 %d",                 pOpts->joyid[4]);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid6 %d",                 pOpts->joyid[5]);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid7 %d",                 pOpts->joyid[6]);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid8 %d",                 pOpts->joyid[7]);
#endif /* JOYSTICK_ID */

	/* core video */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -brightness %f",             pOpts->f_bright_correct); 
	sprintf(&pCmdLine[strlen(pCmdLine)], " -pause_brightness %f",       pOpts->f_pause_bright); 

	if (pOpts->norotate)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%snorotate",pOpts->norotate ? "" : "no");
	if (pOpts->ror)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sror",pOpts->ror ? "" : "no");
	if (pOpts->rol)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%srol",pOpts->rol ? "" : "no");
	if (pOpts->auto_ror)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -autoror");
	if (pOpts->auto_rol)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -autorol");
	if (pOpts->flipx)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sflipx",pOpts->flipx ? "" : "no");
	if (pOpts->flipy)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sflipy",pOpts->flipy ? "" : "no");
	if (strcmp(pOpts->debugres,"auto") != 0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -debug_resolution %s",       pOpts->debugres); 
	sprintf(&pCmdLine[strlen(pCmdLine)], " -gamma %f",                  pOpts->f_gamma_correct);

	/* vector */
	if (DriverIsVector(nGameIndex))
	{
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%santialias",               pOpts->antialias       ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%stranslucency",            pOpts->translucency    ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -beam %f",                   pOpts->f_beam);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -flicker %f",                pOpts->f_flicker);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -intensity %f",              pOpts->f_intensity);
	}
	/* sound */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -samplerate %d",             pOpts->samplerate);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssamples",                 pOpts->use_samples     ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sresamplefilter",          pOpts->use_filter      ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssound",                   pOpts->enable_sound    ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -volume %d",                 pOpts->attenuation);
#ifdef USE_VOLUME_AUTO_ADJUST
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%svolume_adjust",           pOpts->use_volume_adjust ? "" : "no");
#endif /* USE_VOLUME_AUTO_ADJUST */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -audio_latency %i",          pOpts->audio_latency);
	/* misc artwork options */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sartwork",                 pOpts->use_artwork     ? "" : "no");

	if (pOpts->use_artwork == TRUE)
	{
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sbackdrop",                pOpts->backdrops       ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%soverlay",                 pOpts->overlays        ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sbezel",                   pOpts->bezels          ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sartcrop",                 pOpts->artwork_crop    ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -artres %d",                 pOpts->artres);
	}

	/* misc */
	if (pOpts->cheat)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%scheat",                   pOpts->cheat           ? "" : "no");
	if (pOpts->mame_debug)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sdebug",                   pOpts->mame_debug      ? "" : "no");
	if (g_pPlayBkName != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -playback \"%s\"",           g_pPlayBkName);
	if (g_pRecordName != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -record \"%s\"",             g_pRecordName);
	if (g_pRecordWaveName != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -wavwrite \"%s\"",           g_pRecordWaveName);
	if (g_pSaveStateName != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -state \"%s\"",              g_pSaveStateName);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%slog",                     pOpts->errorlog        ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -maxlogsize %d",             pOpts->maxlogsize);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%soslog",                   pOpts->errorlog        ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssleep",                   pOpts->sleep           ? "" : "no");
	if (pOpts->old_timing)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -rdtsc");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sleds",                    pOpts->leds            ? "" : "no");

	if (DriverHasOptionalBIOS(nGameIndex))
		sprintf(&pCmdLine[strlen(pCmdLine)], " -bios %s",pOpts->bios);		

	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sskip_disclaimer",         pOpts->skip_disclaimer ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sskip_gameinfo",           pOpts->skip_gameinfo   ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sskip_validitychecks",     pOpts->skip_validitychecks ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%scrconly",                 pOpts->crc_only        ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%shigh_priority",           pOpts->high_priority   ? "" : "no");
	/* confirm quit */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sconfirm_quit",            pOpts->confirm_quit    ? "" : "no");
#ifdef ROM_PATCH
	if (pOpts->patchname != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -ips_patch %s",          pOpts->patchname);
#endif /* ROM_PATCH */
#ifdef TRANS_UI
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%suse_trans_ui",            pOpts->use_transui     ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -ui_transparency %d",        pOpts->ui_transparency);
#endif /* TRANS_UI */
//ks hcmame s switch m68k core
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020)
	sprintf(&pCmdLine[strlen(pCmdLine)], " -m68k_core %d",              pOpts->m68k_core);
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020) */
//ks hcmame e switch m68k core
#ifdef MENU_TRANS
	sprintf(&pCmdLine[strlen(pCmdLine)], " -menu_color %lu",	pOpts->menu_color);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -menu_trans %d",		pOpts->menu_trans);
#endif
#ifdef LOCK60FPS
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%slock60fps",   	pOpts->bLock60Fps	  ?	"" : "no");
#endif /*LOCK60FPS*/

	/* langcode */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -language %s",               ui_lang_info[GetLangcode()].name);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%suse_lang_list",           UseLangList()          ? "" : "no");

#ifdef USE_PSXPLUGIN
	/* use external gpu plug-ins */
	//sprintf(&pCmdLine[strlen(pCmdLine)], " -%susegpu",		pOpts->bUseGpuPlugIn ?	"" : "no" );
	//if( pOpts->bUseGpuPlugIn ) {
		/* make another windows for gpu plug-ins */
		//sprintf(&pCmdLine[strlen(pCmdLine)], " -%smkgpuwin",	pOpts->bMakeGpuGameWindow ?	"" : "no" );
		/* gpu plug-ins name */
		//sprintf(&pCmdLine[strlen(pCmdLine)], " -gpuname \"%s\"",  pOpts->szGpuPlugIn );

		// PSX_GPU_EXTRA
		sprintf(&pCmdLine[strlen(pCmdLine)], " -gpuscr_size %d",	pOpts->nPsxGpuScreenSize );
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sgpufullscr",		pOpts->bPsxGpuFullScreen  ?	"" : "no" );
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sgpufps",		pOpts->bPsxGpuShowFps  ?	"" : "no" );
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sgpusl",		pOpts->bPsxGpuScanline ?	"" : "no" );
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sgpubld",		pOpts->bPsxGpuBlending ?	"" : "no" );
	//}
#endif

	//dprintf("Launching MAME32:");
	//dprintf("%s",pCmdLine);
}

static BOOL WaitWithMessageLoop(HANDLE hEvent)
{
	DWORD dwRet;
	MSG   msg;

	while (1)
	{
		dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, INFINITE, QS_ALLINPUT);

		if (dwRet == WAIT_OBJECT_0)
			return TRUE;

		if (dwRet != WAIT_OBJECT_0 + 1)
			break;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
				return TRUE;
		}
	}
	return FALSE;
}

static int RunMAME(int nGameIndex)
{
	DWORD               dwExitCode = 0;
	STARTUPINFOA        si;
	PROCESS_INFORMATION pi;
	char *pCmdLine;
	time_t start, end;
	double elapsedtime;

	pCmdLine = malloc(GetTotalPathLen() + 4800);
	if (!pCmdLine)
	{
		OutputDebugStringA("Out of memory.");
		return 1;
	}

	CreateCommandLine(nGameIndex, pCmdLine);

	if (
#ifdef KAILLERA
		kPlay ||
#endif /* KAILLERA */
		(GetLaunchGameInSameProcess() 
		&& IsGamePerfect(nGameIndex)) 
		&& IsAuditResultYes(GetRomAuditResults(nGameIndex)))
	{
		char* argv[400];
		int argc = 0;
		char* p = pCmdLine;

		while (*p) {
			while (*p == ' ') p++;
			if (!*p) break;
						
			if (*p == '\"') {
				argv[argc++] = ++p;
				while (*p && *p != '\"') p++;
			}
			else {
				argv[argc++] = p;
				while (*p && *p != ' ') p++;
			}

			if (!*p) break;
			*p++ = 0;
		}
		
		ShowWindow(hMain, SW_HIDE);
		time(&start);
		dwExitCode = main_(argc, argv);
		time(&end);
		ShowWindow(hMain, SW_SHOW);
		
		if (
#ifdef KAILLERA
			!kPlay && 
#endif /* KAILLERA */
			dwExitCode == 0 ) 
		{
			/*Check the exitcode before incrementing Playtime*/
			IncrementPlayTime(nGameIndex, end - start);
			ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
		}
		
		free(pCmdLine);
		return dwExitCode;
	}

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	if (!CreateProcessA(NULL,
	                    pCmdLine,
	                    NULL,		  /* Process handle not inheritable. */
	                    NULL,		  /* Thread handle not inheritable. */
	                    TRUE,		  /* Handle inheritance.  */
	                    0,			  /* Creation flags. */
	                    NULL,		  /* Use parent's environment block.  */
	                    NULL,		  /* Use parent's starting directory.  */
	                    &si,		  /* STARTUPINFO */
	                    &pi))		  /* PROCESS_INFORMATION */
	{
		dprintf("CreateProcess failed.");
		dwExitCode = GetLastError();
	}
	else
	{
		ShowWindow(hMain, SW_HIDE);
		SendIconToProcess(&pi, nGameIndex);
		time(&start);

		// Wait until child process exits.
		WaitWithMessageLoop(pi.hProcess);

		GetExitCodeProcess(pi.hProcess, &dwExitCode);

		time(&end);
		elapsedtime = end - start;
		if( dwExitCode == 0 )
		{
			// Check the exitcode before incrementing Playtime
			IncrementPlayTime(nGameIndex, elapsedtime);
			ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
		}

		ShowWindow(hMain, SW_SHOW);
		// Close process and thread handles.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	free(pCmdLine);

	return dwExitCode;
}

#undef WinMainInDLL
#ifndef DONT_USE_DLL
#ifdef _MSC_VER
#define WinMainInDLL
#endif /* _MSC_VER */
#endif /* !DONT_USE_DLL */

#ifdef WinMainInDLL
SHAREDOBJ_FUNC(int) WinMain_(HINSTANCE    hInstance,
#else
int WinMain_(HINSTANCE    hInstance,
#endif
                   HINSTANCE    hPrevInstance,
                   LPSTR        lpCmdLine,
                   int          nCmdShow)
{
	dprintf("MAME32 starting");

	options.gui_host = 1;
	use_gui_romloading = TRUE;

	/* set up language for windows */
	assign_msg_catategory(UI_MSG_OSD0, "windows");
	assign_msg_catategory(UI_MSG_OSD1, "ui");

	osd_display_loading_rom_message_ = osd_display_loading_rom_message;

#ifdef KAILLERA
	kailleraInit();
#endif /* KAILLERA */

	if (__argc != 1)
	{
		/* Rename main because gcc will use it instead of WinMain even with -mwindows */
//		extern int DECL_SPEC main_(int, char**);
		exit(main_(__argc, __argv));
	}
	if (!Win32UI_init(hInstance, lpCmdLine, nCmdShow))
		return 1;

	// pump message, but quit on WM_QUIT
	while(PumpMessage())
		;

	Win32UI_exit();
	return 0;
}


HWND GetMainWindow(void)
{
	return hMain;
}

HWND GetTreeView(void)
{
	return hTreeView;
}

void GetRealColumnOrder(int order[])
{
	int tmpOrder[COLUMN_MAX];
	int nColumnMax;
	int i;

	nColumnMax = Picker_GetNumColumns(hwndList);

	/* Get the Column Order and save it */
	if (!oldControl)
	{
		ListView_GetColumnOrderArray(hwndList, nColumnMax, tmpOrder);

		for (i = 0; i < nColumnMax; i++)
		{
			order[i] = Picker_GetRealColumnFromViewColumn(hwndList, tmpOrder[i]);
		}
	}
}

/*
 * PURPOSE: Format raw data read from an ICO file to an HICON
 * PARAMS:  PBYTE ptrBuffer  - Raw data from an ICO file
 *          UINT nBufferSize - Size of buffer ptrBuffer
 * RETURNS: HICON - handle to the icon, NULL for failure
 * History: July '95 - Created
 *          March '00- Seriously butchered from MSDN for mine own
 *          purposes, sayeth H0ek.
 */
HICON FormatICOInMemoryToHICON(PBYTE ptrBuffer, UINT nBufferSize)
{
	ICONIMAGE           IconImage;
	LPICONDIRENTRY      lpIDE = NULL;
	UINT                nNumImages;
	UINT                nBufferIndex = 0;
	HICON               hIcon = NULL;
	int                 i;

	/* Is there a WORD? */
	if (nBufferSize < sizeof(WORD))
	{
		return NULL;
	}

	/* Was it 'reserved' ?	 (ie 0) */
	if ((WORD)(ptrBuffer[nBufferIndex]) != 0)
	{
		return NULL;
	}

	nBufferIndex += sizeof(WORD);

	/* Is there a WORD? */
	if (nBufferSize - nBufferIndex < sizeof(WORD))
	{
		return NULL;
	}

	/* Was it type 1? */
	if ((WORD)(ptrBuffer[nBufferIndex]) != 1)
	{
		return NULL;
	}

	nBufferIndex += sizeof(WORD);

	/* Is there a WORD? */
	if (nBufferSize - nBufferIndex < sizeof(WORD))
	{
		return NULL;
	}

	/* Then that's the number of images in the ICO file */
	nNumImages = (WORD)(ptrBuffer[nBufferIndex]);

	/* Is there at least one icon in the file? */
	if ( nNumImages < 1 )
	{
		return NULL;
	}

	nBufferIndex += sizeof(WORD);

	/* Is there enough space for the icon directory entries? */
	if ((nBufferIndex + nNumImages * sizeof(ICONDIRENTRY)) > nBufferSize)
	{
		return NULL;
	}

	/* Assign icon directory entries from buffer */
	lpIDE = (LPICONDIRENTRY)(&ptrBuffer[nBufferIndex]);
	nBufferIndex += nNumImages * sizeof (ICONDIRENTRY);

	/* Search large icon index to load */
	for (i = 0; i < nNumImages; i++)
	{
		int width;
		int height;

		IconImage.dwNumBytes = lpIDE[i].dwBytesInRes;

		/* Seek to beginning of this image */
		if ( lpIDE[i].dwImageOffset > nBufferSize )
		{
			return NULL;
		}

		nBufferIndex = lpIDE[i].dwImageOffset;

		/* Read it in */
		if ((nBufferIndex + lpIDE[i].dwBytesInRes) > nBufferSize)
		{
			return NULL;
		}

		IconImage.lpBits = &ptrBuffer[nBufferIndex];
		nBufferIndex += lpIDE[i].dwBytesInRes;

		width = (*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biWidth;
		height = (*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biHeight / 2;

		if (width == GetSystemMetrics(SM_CXCURSOR) && height == GetSystemMetrics(SM_CYCURSOR))
			break;
	}

	/* If not found, use first one */
	if (i == nNumImages)
		i = 0;

	IconImage.dwNumBytes = lpIDE[i].dwBytesInRes;
	nBufferIndex = lpIDE[i].dwImageOffset;

	IconImage.lpBits = &ptrBuffer[nBufferIndex];
	nBufferIndex += lpIDE[i].dwBytesInRes;

	hIcon = CreateIconFromResourceEx(IconImage.lpBits, IconImage.dwNumBytes, TRUE, 0x00030000,
			(*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biWidth, (*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biHeight/2, 0 );

	/* It failed, odds are good we're on NT so try the non-Ex way */
	if (hIcon == NULL)
	{
		/* We would break on NT if we try with a 16bpp image */
		if (((LPBITMAPINFO)IconImage.lpBits)->bmiHeader.biBitCount != 16)
		{
			hIcon = CreateIconFromResource(IconImage.lpBits, IconImage.dwNumBytes, TRUE, 0x00030000);
		}
	}

	return hIcon;
}

HICON LoadIconFromFile(const char *iconname)
{
	HICON       hIcon = 0;
	struct stat file_stat;
	char        tmpStr[MAX_PATH];
	char        tmpIcoName[MAX_PATH];
	const char* sDirName = GetImgDir();
	PBYTE       bufferPtr;
	UINT        bufferLen;

	sprintf(tmpStr, "%s/%s.ico", GetIconsDir(), iconname);
	if (stat(tmpStr, &file_stat) != 0
	|| (hIcon = ExtractIconA(hInst, tmpStr, 0)) == 0)
	{
		sprintf(tmpStr, "%s/%s.ico", sDirName, iconname);
		if (stat(tmpStr, &file_stat) != 0
		|| (hIcon = ExtractIconA(hInst, tmpStr, 0)) == 0)
		{
			sprintf(tmpStr, "%s/icons.zip", GetIconsDir());
			sprintf(tmpIcoName, "%s.ico", iconname);
			if (stat(tmpStr, &file_stat) == 0)
			{
				SetCorePathList(FILETYPE_ARTWORK, GetIconsDir());
				if (load_zipped_file(FILETYPE_ARTWORK, 0, "icons.zip", tmpIcoName, &bufferPtr, &bufferLen) == 0)
				{
					hIcon = FormatICOInMemoryToHICON(bufferPtr, bufferLen);
					free(bufferPtr);
				}
			}
		}
	}
	return hIcon;
}

/* Return the number of games currently displayed */
int GetNumGames()
{
	return game_count;
}

/* Sets the treeview and listviews sizes in accordance with their visibility and the splitters */
static void ResizeTreeAndListViews(BOOL bResizeHidden)
{
	int i;
	int nLastWidth = 0;
	int nLastWidth2 = 0;
	int nLeftWindowWidth = 0;
	RECT rect;
	BOOL bVisible;

	/* Size the List Control in the Picker */
	GetClientRect(hMain, &rect);

	if (bShowStatusBar)
		rect.bottom -= bottomMargin;
	if (bShowToolBar)
		rect.top += topMargin;

	/* Tree control */
	ShowWindow(GetDlgItem(hMain, IDC_TREE), bShowTree ? SW_SHOW : SW_HIDE);

	for (i = 0; g_splitterInfo[i].nSplitterWindow; i++)
	{
		bVisible = GetWindowLong(GetDlgItem(hMain, g_splitterInfo[i].nLeftWindow), GWL_STYLE) & WS_VISIBLE ? TRUE : FALSE;
		if (bResizeHidden || bVisible)
		{
			nLeftWindowWidth = nSplitterOffset[i] - SPLITTER_WIDTH/2 - nLastWidth;

			/* special case for the rightmost pane when the screenshot is gone */
			if (!GetShowScreenShot() && !g_splitterInfo[i+1].nSplitterWindow)
				nLeftWindowWidth = rect.right - nLastWidth;

			/* woah?  are we overlapping ourselves? */
			if (nLeftWindowWidth < MIN_VIEW_WIDTH)
			{
				nLastWidth = nLastWidth2;
				nLeftWindowWidth = nSplitterOffset[i] - MIN_VIEW_WIDTH - (SPLITTER_WIDTH*3/2) - nLastWidth;
				i--;
			}

			MoveWindow(GetDlgItem(hMain, g_splitterInfo[i].nLeftWindow), nLastWidth, rect.top + 2,
				nLeftWindowWidth, (rect.bottom - rect.top) - 4 , TRUE);

			MoveWindow(GetDlgItem(hMain, g_splitterInfo[i].nSplitterWindow), nSplitterOffset[i], rect.top + 2,
				SPLITTER_WIDTH, (rect.bottom - rect.top) - 4, TRUE);
		}

		if (bVisible)
		{
			nLastWidth2 = nLastWidth;
			nLastWidth += nLeftWindowWidth + SPLITTER_WIDTH; 
		}
	}
}

/* Adjust the list view and screenshot button based on GetShowScreenShot() */
void UpdateScreenShot(void)
{
	RECT rect;
	int  nWidth;
	RECT fRect;
	POINT p = {0, 0};

	/* first time through can't do this stuff */
	if (hwndList == NULL)
		return;

	/* Size the List Control in the Picker */
	GetClientRect(hMain, &rect);

	if (bShowStatusBar)
		rect.bottom -= bottomMargin;
	if (bShowToolBar)
		rect.top += topMargin;

	if (GetShowScreenShot())
	{
		nWidth = nSplitterOffset[GetSplitterCount() - 1];
		CheckMenuItem(GetMenu(hMain),ID_VIEW_PICTURE_AREA, MF_CHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_PICTURE_AREA, MF_CHECKED);
	}
	else
	{
		nWidth = rect.right;
		CheckMenuItem(GetMenu(hMain),ID_VIEW_PICTURE_AREA, MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_PICTURE_AREA, MF_UNCHECKED);
	}

	ResizeTreeAndListViews(FALSE);

	FreeScreenShot();

	if (have_selection)
	{
		// load and set image, or empty it if we don't have one
		LoadScreenShot(Picker_GetSelectedItem(hwndList), TabView_GetCurrentTab(hTabCtrl));
	}

	// figure out if we have a history or not, to place our other windows properly
	UpdateHistory();

	// setup the picture area

	if (GetShowScreenShot())
	{
		DWORD dwStyle;
		DWORD dwStyleEx;
		BOOL showing_history;

		ClientToScreen(hMain, &p);
		GetWindowRect(GetDlgItem(hMain, IDC_SSFRAME), &fRect);
		OffsetRect(&fRect, -p.x, -p.y);

		// show history on this tab IFF
		// - we have history for the game
		// - we're on the first tab
		// - we DON'T have a separate history tab
		showing_history = (have_history && (TabView_GetCurrentTab(hTabCtrl) == GetHistoryTab() || GetHistoryTab() == TAB_ALL ) &&
		                   GetShowTab(TAB_HISTORY) == FALSE);
		CalculateBestScreenShotRect(GetDlgItem(hMain, IDC_SSFRAME), &rect,showing_history);

		dwStyle   = GetWindowLong(GetDlgItem(hMain, IDC_SSPICTURE), GWL_STYLE);
		dwStyleEx = GetWindowLong(GetDlgItem(hMain, IDC_SSPICTURE), GWL_EXSTYLE);

		AdjustWindowRectEx(&rect, dwStyle, FALSE, dwStyleEx);
		MoveWindow(GetDlgItem(hMain, IDC_SSPICTURE),
		           fRect.left  + rect.left,
		           fRect.top   + rect.top,
		           rect.right  - rect.left,
		           rect.bottom - rect.top,
		           TRUE);
		
		ShowWindow(GetDlgItem(hMain,IDC_SSPICTURE),
				   (TabView_GetCurrentTab(hTabCtrl) != TAB_HISTORY) ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hMain,IDC_SSFRAME),SW_SHOW);
		ShowWindow(GetDlgItem(hMain,IDC_SSTAB),bShowTabCtrl ? SW_SHOW : SW_HIDE);

		InvalidateRect(GetDlgItem(hMain,IDC_SSPICTURE),NULL,FALSE);
	}
	else
	{
		ShowWindow(GetDlgItem(hMain,IDC_SSPICTURE),SW_HIDE);
		ShowWindow(GetDlgItem(hMain,IDC_SSFRAME),SW_HIDE);
		ShowWindow(GetDlgItem(hMain,IDC_SSTAB),SW_HIDE);
	}

}

void ResizePickerControls(HWND hWnd)
{
	RECT frameRect;
	RECT rect, sRect;
	int  nListWidth, nScreenShotWidth;
	static BOOL firstTime = TRUE;
	int  doSSControls = TRUE;
	int i, nSplitterCount;

	nSplitterCount = GetSplitterCount();

	/* Size the List Control in the Picker */
	GetClientRect(hWnd, &rect);

	/* Calc the display sizes based on g_splitterInfo */
	if (firstTime)
	{
		RECT rWindow;

		for (i = 0; i < nSplitterCount; i++)
			nSplitterOffset[i] = rect.right * g_splitterInfo[i].dPosition;

		GetWindowRect(hStatusBar, &rWindow);
		bottomMargin = rWindow.bottom - rWindow.top;
		GetWindowRect(hToolBar, &rWindow);
		topMargin = rWindow.bottom - rWindow.top+10;
		/*buttonMargin = (sRect.bottom + 4); */

		firstTime = FALSE;
	}
	else
	{
		doSSControls = GetShowScreenShot();
	}

	if (bShowStatusBar)
		rect.bottom -= bottomMargin;

	if (bShowToolBar)
		rect.top += topMargin;

	MoveWindow(GetDlgItem(hWnd, IDC_DIVIDER), rect.left, rect.top - 4, rect.right, 2, TRUE);

	ResizeTreeAndListViews(TRUE);

	nListWidth = nSplitterOffset[nSplitterCount-1];
	nScreenShotWidth = (rect.right - nListWidth) - 4;

	/* Screen shot Page tab control */
	if (bShowTabCtrl)
	{
		MoveWindow(GetDlgItem(hWnd, IDC_SSTAB), nListWidth + 4, rect.top + 2,
			nScreenShotWidth - 2, rect.top + 20, doSSControls);
		rect.top += 20;
	}

	/* resize the Screen shot frame */
	MoveWindow(GetDlgItem(hWnd, IDC_SSFRAME), nListWidth + 4, rect.top + 2,
		nScreenShotWidth - 2, (rect.bottom - rect.top) - 4, doSSControls);

	/* The screen shot controls */
	GetClientRect(GetDlgItem(hWnd, IDC_SSFRAME), &frameRect);

	/* Text control - game history */
	sRect.left = nListWidth + 14;
	sRect.right = sRect.left + (nScreenShotWidth - 22);
	

	if (GetShowTab(TAB_HISTORY))
	{
		// We're using the new mode, with the history filling the entire tab (almost)
		sRect.top = rect.top + 14;
		sRect.bottom = (rect.bottom - rect.top) - 30;   
	}
	else
	{
		// We're using the original mode, with the history beneath the SS picture
		sRect.top = rect.top + 264;
		sRect.bottom = (rect.bottom - rect.top) - 278;
	}

	MoveWindow(GetDlgItem(hWnd, IDC_HISTORY),
		sRect.left, sRect.top,
		sRect.right - sRect.left, sRect.bottom, doSSControls);

	/* the other screen shot controls will be properly placed in UpdateScreenshot() */

}

static int modify_separator_len(const char *str)
{
	int n;

	switch (*str)
	{
	case ' ':
		if (!strncmp(str, " - ", 3))
			return 3;

		if ((n = modify_separator_len(str + 1)) != 0)
			return n + 1;

		break;

	case ':':
	case '/':
	case ',':
	case '(':
	case ')':
	case '!':
		for (n = 1; str[n]; n++)
			if (str[n] != ' ')
				break;
		return modify_separator_len(str + n) + n;
	}

	return 0;
}

char *ModifyThe(const char *str)
{
	static int  bufno = 0;
	static char buffer[4][255];
	int modified = 0;
	char *ret = (char *)str;
	char *t;

	t = buffer[bufno];

	while (*str)
	{
		char *p = t;

		while (!modify_separator_len(str))
		{
			if ((*p++ = *str++) == '\0')
				break;
		}

		*p = '\0';
#if 0
		if (strncmp(t, "The ", 4) == 0)
		{
			char temp[255];

			strcpy(temp, t + 4);
			strcat(temp, ", The");

			strcpy(t, temp);
			p++;
			modified = 1;
		}
#endif
		t = p + modify_separator_len(str);
		while (p < t)
			*p++ = *str++;
		*p = '\0';
	}

	if (modified)
	{
		ret = buffer[bufno];
		bufno = (bufno + 1) % 4;
	}

	return ret;
}

HBITMAP GetBackgroundBitmap(void)
{
	return hBackground;
}

HPALETTE GetBackgroundPalette(void)
{
	return hPALbg;
}

MYBITMAPINFO * GetBackgroundInfo(void)
{
	return &bmDesc;
}

#ifdef DARKMAME	// HISTORY_BG
HBITMAP GetFolderBackgroundBitmap(void) 	 { return hFolderBackground; }
HPALETTE GetFolderBackgroundPalette(void)	 { return hFolderPALbg; }
MYBITMAPINFO * GetFolderBackgroundInfo(void) { return &bmFolderDesc; }

HBITMAP GetSnapBackgroundBitmap(void) 	 	{ return hSnapBackground; }
HPALETTE GetSnapBackgroundPalette(void)	 	{ return hSnapPALbg; }
MYBITMAPINFO * GetSnapBackgroundInfo(void) { return &bmSnapDesc; }
#endif

BOOL GetUseOldControl(void)
{
	return oldControl;
}

BOOL GetUseXPControl(void)
{
	return xpControl;
}

int GetMinimumScreenShotWindowWidth(void)
{
	BITMAP bmp;
	GetObject(hMissing_bitmap,sizeof(BITMAP),&bmp);

	return bmp.bmWidth + 6; // 6 is for a little breathing room
}


int GetDriverIndex(const struct GameDriver *driver)
{
	return GetGameNameIndex(driver->name);
}

int GetGameNameIndex(const char *name)
{
	driver_data_type *driver_index_info;
	driver_data_type key;
	key.name = name;

	// uses our sorted array of driver names to get the index in log time
	driver_index_info = bsearch(&key,sorted_drivers,game_count,sizeof(driver_data_type),
	                            DriverDataCompareFunc);

	if (driver_index_info == NULL)
		return -1;

	return driver_index_info->index;

}

int GetIndexFromSortedIndex(int sorted_index)
{
	return sorted_drivers[sorted_index].index;
}

/***************************************************************************
    Internal functions
 ***************************************************************************/

static char **game_readings;

static void build_game_readings(void)
{
	int i;

	if (game_readings)
		free (game_readings);

	game_readings = malloc (sizeof (char *) * game_count);
	if (!game_readings)
		return;

	for (i = 0; i < game_count; i++)
	{
		char *l,*r;
		l = _LST(drivers[i]->description);
		r = _READINGS(drivers[i]->description);

		game_readings[i] = (drivers[i]->description == r) ? l : r;
	}
}

static void ChangeLanguage(int id)
{
	int nGame = Picker_GetSelectedItem(hwndList);
	int i;

	if (id)
		SetLangcode(id - ID_LANGUAGE_ENGLISH_US);

	for (i = 0; i < UI_LANG_MAX; i++)
	{
		UINT cp = ui_lang_info[i].codepage;

		CheckMenuItem(GetMenu(hMain), i + ID_LANGUAGE_ENGLISH_US, i == GetLangcode() ? MF_CHECKED : MF_UNCHECKED);
		if (OnNT())
			EnableMenuItem(GetMenu(hMain), i + ID_LANGUAGE_ENGLISH_US, IsValidCodePage(cp) ? MF_ENABLED : MF_GRAYED);
		else
			EnableMenuItem(GetMenu(hMain), i + ID_LANGUAGE_ENGLISH_US, (i == UI_LANG_EN_US || cp == GetACP()) ? MF_ENABLED : MF_GRAYED);
	}

	if (id)
	{
		LOGFONTA logfont;

		if (hFont != NULL)
			DeleteObject(hFont);

		GetTranslatedFont(&logfont);

		SetListFont(&logfont);

		hFont = TranslateCreateFont(&logfont);
	}

	build_game_readings();

	if (id && hToolBar != NULL)
	{
		DestroyWindow(hToolBar);
		hToolBar = InitToolbar(hMain);
		main_resize_items[0].u.hwnd = hToolBar;
		ToolBar_CheckButton(hToolBar, ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, IDC_USE_LIST, UseLangList() ^ (GetLangcode() == UI_LANG_EN_US) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_PICTURE_AREA, GetShowScreenShot() ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_STATUS, (bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_LARGE_ICON + Picker_GetViewID(hwndList), MF_CHECKED);
		ShowWindow(hToolBar, (bShowToolBar) ? SW_SHOW : SW_HIDE);
	}

	TranslateDialog(hMain, 0, TRUE);
	TranslateMenu(GetMenu(hMain), 0);

	DrawMenuBar(hMain);

	TranslateTreeFolders(hTreeView);

	/* Reset the font */
	if (hFont != NULL)
		SetAllWindowsFont(hMain, &main_resize, hFont, FALSE);

	TabView_Reset(hTabCtrl);
	TabView_UpdateSelection(hTabCtrl);
	UpdateHistory();

	ResetColumnDisplay(FALSE);

	Picker_SetSelectedItem(hwndList, nGame);
}

// used for our sorted array of game names
int CLIB_DECL DriverDataCompareFunc(const void *arg1,const void *arg2)
{
	return strcmp( ((driver_data_type *)arg1)->name, ((driver_data_type *)arg2)->name );
}
#ifdef KAILLERA
int DriverTitleCompareFunc(const void *arg1,const void *arg2)
{
	return strcmp( ((driver_title_type *)arg1)->name, ((driver_title_type *)arg2)->name );
}
#endif /* KAILLERA */

static void ResetBackground(char *szFile)
{
	char szDestFile[MAX_PATH];

	/* The MAME core load the .png file first, so we only need replace this file */
	sprintf(szDestFile, "%s\\bkground.png", GetBgDir());
	SetFileAttributesA(szDestFile, FILE_ATTRIBUTE_NORMAL);
	CopyFileA(szFile, szDestFile, FALSE);
}

static void RandomSelectBackground(void)
{
	struct _finddata_t c_file;
	long hFile;
	char szFile[MAX_PATH];
	int count=0;
	const char *szDir=GetBgDir();
	char *buf=malloc(_MAX_FNAME * MAX_BGFILES);

	if (buf == NULL)
		return;

	sprintf(szFile, "%s\\*.bmp", szDir);
	hFile = _findfirst(szFile, &c_file);
	if (hFile != -1L)
	{
		int Done = 0;
		while (!Done && count < MAX_BGFILES)
		{
			memcpy(buf + count * _MAX_FNAME, c_file.name, _MAX_FNAME);
			count++;
			Done = _findnext(hFile, &c_file);
		}
		_findclose(hFile);
	}
	sprintf(szFile, "%s\\*.png", szDir);
	hFile = _findfirst(szFile, &c_file);
	if (hFile != -1L)
	{
		int Done = 0;
		while (!Done && count < MAX_BGFILES)
		{
			memcpy(buf + count * _MAX_FNAME, c_file.name, _MAX_FNAME);
			count++;
			Done = _findnext(hFile, &c_file);
		}
		_findclose(hFile);
	}

	if (count)
	{
		srand( (unsigned)time( NULL ) );
		sprintf(szFile, "%s\\%s", szDir, buf + (rand() % count) * _MAX_FNAME);
		ResetBackground(szFile);
	}

	free(buf);
}

void SetMainTitle(void)
{
	char version[50];
	char buffer[100];

	sscanf(build_version,"%s",version);
#ifdef EKMAMELITE	
  sprintf(buffer,"EKMAMElite %s ",version);
#else  
	sprintf(buffer,"EKMAME %s ",version);
#endif
	SetWindowText(hMain,_Unicode(buffer));
}

static BOOL Win32UI_init(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS	wndclass;
	RECT		rect;
	int i, j = 0, nSplitterCount;
	LONG common_control_version = GetCommonControlVersion();

	srand((unsigned)time(NULL));

	begin_resource_tracking();

	// Count the number of games
	game_count = 0;
	while (drivers[game_count] != 0)
		game_count++;

	// Create parent index
	parent_index = auto_malloc(sizeof(*parent_index) * game_count);
	if (!parent_index)
		return FALSE;
	for (i = 0; i < game_count; i++)
	{
		parent_index[i] = -1;
		if (drivers[i]->clone_of && !(drivers[i]->clone_of->flags & NOT_A_DRIVER))
		{
			if (drivers[i]->clone_of == drivers[j])
			{
				parent_index[i] = j;
			}
			else
			{
				for (j = 0; j < game_count; j++)
				{
					if (drivers[i]->clone_of == drivers[j])
					{
						parent_index[i] = j;
						break;
					}
				}
				j = 0;
			}
		}
	}

	/* custom per-game icons */
	icon_index = auto_malloc(sizeof(int) * game_count);
	if (!icon_index)
		return FALSE;
	ZeroMemory(icon_index,sizeof(int) * game_count);

	/* sorted list of drivers by name */
	sorted_drivers = (driver_data_type *) auto_malloc(sizeof(driver_data_type) * game_count);
#ifdef KAILLERA
	sorted_titles = (driver_title_type *) auto_malloc(sizeof(driver_title_type) * game_count);
#endif /* KAILLERA */

	if (!sorted_drivers)
		return FALSE;
	for (i=0;i<game_count;i++)
	{
		sorted_drivers[i].name = drivers[i]->name;
		sorted_drivers[i].index = i;
#ifdef KAILLERA
		strcpy(sorted_titles[i].name, ModifyThe(drivers[i]->description));
		sorted_titles[i].index = i;
#endif
	}
	qsort(sorted_drivers,game_count,sizeof(driver_data_type),DriverDataCompareFunc );

#ifdef KAILLERA
	qsort(sorted_titles,game_count,sizeof(driver_title_type),DriverTitleCompareFunc );
#endif /* KAILLERA */

	/* initialize cpu information */
	cpuintrf_init();

	/* initialize sound information */
	sndintrf_init();
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = MameWindowProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = DLGWINDOWEXTRA;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAME32_ICON));
	wndclass.hCursor       = NULL;
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wndclass.lpszMenuName  = MAKEINTRESOURCE(IDR_UI_MENU);
	wndclass.lpszClassName = TEXT("MainClass");

	RegisterClass(&wndclass);

	InitCommonControls();

	// Are we using an Old comctl32.dll?
	dprintf("common controlversion %i %i",common_control_version >> 16,
	        common_control_version & 0xffff);
			 
	oldControl = (common_control_version < PACKVERSION(4,71));
	xpControl = (common_control_version >= PACKVERSION(6,0));
	if (oldControl)
	{
		char buf[] = MAME32NAME " has detected an old version of comctl32.dll\n\n"
					 "Game Properties, many configuration options and\n"
					 "features are not available without an updated DLL\n\n"
					 "Please install the common control update found at:\n\n"
					 "http://www.microsoft.com/msdownload/ieplatform/ie/comctrlx86.asp\n\n"
					 "Would you like to continue without using the new features?\n";

		if (IDNO == MessageBoxA(0, buf, MAME32NAME " Outdated comctl32.dll Warning", MB_YESNO | MB_ICONWARNING))
			return FALSE;
	}

	dprintf("about to init options");
	OptionsInit();
	dprintf("options loaded");

	/* USE LANGUAGE LIST */
	build_game_readings();

	g_mame32_message = RegisterWindowMessage(TEXT_MAME32NAME);
	g_bDoBroadcast = GetBroadcast();

	HelpInit();

	strcpy(last_directory,GetInpDir());
#ifdef MAME_AVI
	strcpy(last_directory_avi, GetAviDir());
	mkdir(last_directory_avi);	// 디렉토리 생성 시도
#endif
  
	hMain = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, NULL);
	if (hMain == NULL)
	{
		dprintf("error creating main dialog, aborting");
		return FALSE;
	}

	SetMainTitle();
	hTabCtrl = GetDlgItem(hMain, IDC_SSTAB);
	
	{
		struct TabViewOptions opts;

		static struct TabViewCallbacks s_tabviewCallbacks =
		{
			GetShowTabCtrl,			// pfnGetShowTabCtrl
			SetCurrentTab,			// pfnSetCurrentTab
			GetCurrentTab,			// pfnGetCurrentTab
			SetShowTab,				// pfnSetShowTab
			GetShowTab,				// pfnGetShowTab

			GetImageTabShortName,	// pfnGetTabShortName
			GetImageTabLongName,	// pfnGetTabLongName
			UpdateScreenShot		// pfnOnSelectionChanged
		};

		memset(&opts, 0, sizeof(opts));
		opts.pCallbacks = &s_tabviewCallbacks;
		opts.nTabCount = MAX_TAB_TYPES;

		if (!SetupTabView(hTabCtrl, &opts))
			return FALSE;
	}

	/* subclass history window */
	g_lpHistoryWndProc = (WNDPROC)(LONG)(int)GetWindowLong(GetDlgItem(hMain, IDC_HISTORY), GWL_WNDPROC);
	SetWindowLong(GetDlgItem(hMain, IDC_HISTORY), GWL_WNDPROC, (LONG)HistoryWndProc);

	/* subclass picture frame area */
	g_lpPictureFrameWndProc = (WNDPROC)(LONG)(int)GetWindowLong(GetDlgItem(hMain, IDC_SSFRAME), GWL_WNDPROC);
	SetWindowLong(GetDlgItem(hMain, IDC_SSFRAME), GWL_WNDPROC, (LONG)PictureFrameWndProc);

	/* subclass picture area */
	g_lpPictureWndProc = (WNDPROC)(LONG)(int)GetWindowLong(GetDlgItem(hMain, IDC_SSPICTURE), GWL_WNDPROC);
	SetWindowLong(GetDlgItem(hMain, IDC_SSPICTURE), GWL_WNDPROC, (LONG)PictureWndProc);

	/* Load the pic for the default screenshot. */
	hMissing_bitmap = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_ABOUT));

	/* Stash hInstance for later use */
	hInst = hInstance;

	hToolBar   = InitToolbar(hMain);
	hStatusBar = InitStatusBar(hMain);
	hProgWnd   = InitProgressBar(hStatusBar);

	main_resize_items[0].u.hwnd = hToolBar;
	main_resize_items[1].u.hwnd = hStatusBar;

	/* In order to handle 'Large Fonts' as the Windows
	 * default setting, we need to make the dialogs small
	 * enough to fit in our smallest window size with
	 * large fonts, then resize the picker, tab and button
	 * controls to fill the window, no matter which font
	 * is currently set.  This will still look like bad
	 * if the user uses a bigger default font than 125%
	 * (Large Fonts) on the Windows display setting tab.
	 *
	 * NOTE: This has to do with Windows default font size
	 * settings, NOT our picker font size.
	 */

	GetClientRect(hMain, &rect);

	hTreeView = GetDlgItem(hMain, IDC_TREE);
	hwndList  = GetDlgItem(hMain, IDC_LIST);

	//history_filename = strdup(GetHistoryFileName());
	//mameinfo_filename = strdup(GetMAMEInfoFileName());

	if (!InitSplitters())
		return FALSE;

	nSplitterCount = GetSplitterCount();
	for (i = 0; i < nSplitterCount; i++)
	{
		HWND hWnd;
		HWND hWndLeft;
		HWND hWndRight;

		hWnd = GetDlgItem(hMain, g_splitterInfo[i].nSplitterWindow);
		hWndLeft = GetDlgItem(hMain, g_splitterInfo[i].nLeftWindow);
		hWndRight = GetDlgItem(hMain, g_splitterInfo[i].nRightWindow);

		AddSplitter(hWnd, hWndLeft, hWndRight, g_splitterInfo[i].pfnAdjust);
	}

	/* Initial adjustment of controls on the Picker window */
	ResizePickerControls(hMain);

	TabView_UpdateSelection(hTabCtrl);
#ifdef KAILLERA
	{
		LOGFONTA logfont;

		GetChatFont(&logfont);
		hChatFont = CreateFontIndirectA(&logfont);
	}
#endif /* KAILLERA */

	bDoGameCheck = GetGameCheck();
	idle_work    = TRUE;
	game_index   = 0;

	bShowTree      = GetShowFolderList();
	bShowToolBar   = GetShowToolBar();
	bShowStatusBar = GetShowStatusBar();
	bShowTabCtrl   = GetShowTabCtrl();

	CheckMenuItem(GetMenu(hMain), ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
	ToolBar_CheckButton(hToolBar, ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMain), ID_VIEW_TOOLBARS, (bShowToolBar) ? MF_CHECKED : MF_UNCHECKED);
	ShowWindow(hToolBar, (bShowToolBar) ? SW_SHOW : SW_HIDE);
	CheckMenuItem(GetMenu(hMain), ID_VIEW_STATUS, (bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED);
	ShowWindow(hStatusBar, (bShowStatusBar) ? SW_SHOW : SW_HIDE);
	CheckMenuItem(GetMenu(hMain), ID_VIEW_PAGETAB, (bShowTabCtrl) ? MF_CHECKED : MF_UNCHECKED);
	ToolBar_CheckButton(hToolBar, IDC_USE_LIST, UseLangList() ^ (GetLangcode() == UI_LANG_EN_US) ? MF_CHECKED : MF_UNCHECKED);

	if (oldControl)
	{
		EnableMenuItem(GetMenu(hMain), ID_CUSTOMIZE_FIELDS,  MF_GRAYED);
		EnableMenuItem(GetMenu(hMain), ID_GAME_PROPERTIES,   MF_GRAYED);
		EnableMenuItem(GetMenu(hMain), ID_SOURCE_PROPERTIES, MF_GRAYED);
		EnableMenuItem(GetMenu(hMain), ID_OPTIONS_DEFAULTS,  MF_GRAYED);
	}

	/* Init DirectDraw */
	if (!DirectDraw_Initialize())
	{
		DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DIRECTX), NULL, DirectXDialogProc);
		return FALSE;
	}

	if (GetRandomBackground())
		RandomSelectBackground();

	LoadBackgroundBitmap();

	dprintf("about to init tree");
	InitTree(g_folderData, g_filterList);
	dprintf("did init tree");

	PropertiesInit();

	/* Initialize listview columns */
	InitListView();
	SetFocus(hwndList);

	/* Reset the font */
	{
		LOGFONTA logfont;

		GetListFont(&logfont);
		hFont = TranslateCreateFont(&logfont);
		if (hFont != NULL)
			SetAllWindowsFont(hMain, &main_resize, hFont, FALSE);

#ifdef DARKMAME
		GetHistFont(&logfont);
		hHistFont = TranslateCreateFont(&logfont);
#endif

#ifdef KAILLERA
		GetChatFont(&logfont);
		hChatFont = CreateFontIndirect(&logfont);
#endif			
	}

	/* Init DirectInput */
	if (!DirectInputInitialize())
	{
		DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DIRECTX), NULL, DirectXDialogProc);
		return FALSE;
	}

	AdjustMetrics();
	UpdateScreenShot();

	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDA_TAB_KEYS));

	/* clear keyboard state */
	KeyboardStateClear();

	for (i = 0; i < NUM_GUI_SEQUENCES; i++)
	{
		input_seq_t *is1;
		input_seq_t *is2;
		is1 = &(GUISequenceControl[i].is);
		is2 = GUISequenceControl[i].getiniptr();
		seq_copy(is1, is2);
		//dprintf("seq =%s is: %4i %4i %4i %4i\n",GUISequenceControl[i].name, (*is1)[0], (*is1)[1], (*is1)[2], (*is1)[3]);
	}

	if (GetJoyGUI() == TRUE)
	{
		g_pJoyGUI = &DIJoystick;
		if (g_pJoyGUI->init() != 0)
			g_pJoyGUI = NULL;
		else
			SetTimer(hMain, JOYGUI_TIMER, JOYGUI_MS, NULL);
	}
	else
		g_pJoyGUI = NULL;

	ChangeLanguage(0);

	if (GetHideMouseOnStartup())
	{    
		/*  For some reason the mouse is centered when a game is exited, which of
			course causes a WM_MOUSEMOVE event that shows the mouse. So we center
			it now, before the startup coords are initilized, and that way the mouse
			will still be hidden when exiting from a game (i hope) :)
		*/      
		SetCursorPos(GetSystemMetrics(SM_CXSCREEN)/2,GetSystemMetrics(SM_CYSCREEN)/2);
		
		// Then hide it
		ShowCursor(FALSE);
	}

	dprintf("about to show window");

	nCmdShow = GetWindowState();
	if (nCmdShow == SW_HIDE || nCmdShow == SW_MINIMIZE || nCmdShow == SW_SHOWMINIMIZED)
	{
		nCmdShow = SW_RESTORE;
	}

	if (GetRunFullScreen())
	{ 
		LONG lMainStyle;

		// Remove menu
		SetMenu(hMain,NULL); 

		// Frameless dialog (fake fullscreen)
		lMainStyle = GetWindowLong(hMain, GWL_STYLE);
		lMainStyle = lMainStyle & (WS_BORDER ^ 0xffffffff);
		SetWindowLong(hMain, GWL_STYLE, lMainStyle);

		nCmdShow = SW_MAXIMIZE;
	}

	ShowWindow(hMain, nCmdShow);


	switch (GetViewMode())
	{
	case VIEW_LARGE_ICONS :
		SetView(ID_VIEW_LARGE_ICON);
		break;
	case VIEW_SMALL_ICONS :
		SetView(ID_VIEW_SMALL_ICON);
		break;
	case VIEW_INLIST :
		SetView(ID_VIEW_LIST_MENU);
		break;
	case VIEW_REPORT :
		SetView(ID_VIEW_DETAIL);
		break;
	case VIEW_GROUPED :
	default :
		SetView(ID_VIEW_GROUPED);
		break;
	}

	if (GetCycleScreenshot() > 0)
	{
		SetTimer(hMain, SCREENSHOT_TIMER, GetCycleScreenshot()*1000, NULL); //scale to Seconds
	}

#ifdef MAME_DEBUG
	if (mame_validitychecks())
	{
		MessageBox(hMain, MAMENAME " has failed its validity checks.  The GUI will "
			"still work, but emulations will fail to execute", MAMENAME, MB_OK);
	}
#endif // MAME_DEBUG

	return TRUE;
}

static void Win32UI_exit(void)
{
#ifdef KAILLERA
	kailleraShutdown();
#endif /* KAILLERA */

	if (g_bDoBroadcast == TRUE)
	{
        ATOM a = GlobalAddAtomA("");
		SendMessage(HWND_BROADCAST, g_mame32_message, a, a);
		GlobalDeleteAtom(a);
	}

	if (g_pJoyGUI != NULL)
		g_pJoyGUI->exit();

	/* Free GDI resources */

	if (hMissing_bitmap)
	{
		DeleteObject(hMissing_bitmap);
		hMissing_bitmap = NULL;
	}

	if (hBackground)
	{
		DeleteObject(hBackground);
		hBackground = NULL;
	}
	
	if (hPALbg)
	{
		DeleteObject(hPALbg);
		hPALbg = NULL;
	}
	
	if (hFont)
	{
		DeleteObject(hFont);
		hFont = NULL;
	}

#ifdef DARKMAME	// HISTORY_BG
	if (hFolderBackground)	DeleteObject(hFolderBackground);
	if (hFolderPALbg)		DeleteObject(hFolderPALbg);
	if (hSnapBackground)	DeleteObject(hSnapBackground);
	if (hSnapPALbg)			DeleteObject(hSnapPALbg);
	
	hFolderBackground = NULL;
	hFolderPALbg = NULL;
	hSnapBackground = NULL;
	hSnapPALbg = NULL;
	
	if (hHistFont) DeleteObject(hHistFont);
	hHistFont = NULL;
#endif

#ifdef KAILLERA
	if (hChatFont)
	{
		DeleteObject(hChatFont);
		hChatFont = NULL;
	}
#endif /* KAILLERA */

	DestroyIcons();

	DestroyAcceleratorTable(hAccel);

	DirectInputClose();
	DirectDraw_Close();

	SetSavedFolderID(GetCurrentFolderID());

	SaveOptions();

	FreeFolders();

	/* DestroyTree(hTreeView); */

	FreeScreenShot();

	OptionsExit();

	HelpExit();

	end_resource_tracking();
}

static long WINAPI MameWindowProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	MINMAXINFO	*mminfo;
#if MULTISESSION
	int nGame;
	HWND hGameWnd;
	long lGameWndStyle;
#endif // MULTISESSION
	int 		i;
	char		szClass[128];

	switch (message)
	{
	case WM_CTLCOLORSTATIC:
#ifdef DARKMAME	// HISTORY_BG
		if ((HWND)lParam == GetDlgItem(hMain, IDC_HISTORY))
		{
			HDC hDC=(HDC)wParam;
			static HBRUSH hBrush=0;
			LOGBRUSH lb;

			if (hBrush) DeleteObject(hBrush);
			lb.lbStyle  = BS_HOLLOW;
			hBrush = CreateBrushIndirect(&lb);
			
			if (hSnapBackground)
			{
				SetBkMode(hDC, TRANSPARENT);
			}
			else {
				SetBkMode(hDC, OPAQUE);
				SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
			}
			SetTextColor(hDC, GetHistFontColor());
			return (LRESULT) hBrush;
		}
#else /*DARKMAME*/
		if (hBackground && (HWND)lParam == GetDlgItem(hMain, IDC_HISTORY))
		{
			static HBRUSH hBrush=0;
			HDC hDC=(HDC)wParam;
			LOGBRUSH lb;

			if (hBrush)
				DeleteObject(hBrush);

			if (hBackground)	// Always true?
			{
				lb.lbStyle  = BS_HOLLOW;
				hBrush = CreateBrushIndirect(&lb);
				SetBkMode(hDC, TRANSPARENT);
			}
			else
			{
				hBrush = GetSysColorBrush(COLOR_BTNFACE);
				SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
				SetBkMode(hDC, OPAQUE);
			}
			SetTextColor(hDC, GetListFontColor());
			return (LRESULT) hBrush;
		}
#endif 	/*DARKMAME*/
		break;

	case WM_INITDIALOG:
		TranslateDialog(hWnd, lParam, FALSE);

		/* Initialize info for resizing subitems */
		GetClientRect(hWnd, &main_resize.rect);
		return TRUE;

	case WM_SETFOCUS:
		SetFocus(hwndList);
		break;

	case WM_SETTINGCHANGE:
		AdjustMetrics();
		return 0;

	case WM_SIZE:
		OnSize(hWnd, wParam, LOWORD(lParam), HIWORD(wParam));
		return TRUE;

	case WM_MENUSELECT:
		return Statusbar_MenuSelect(hWnd, wParam, lParam);

	case MM_PLAY_GAME:
		MamePlayGame();
		return TRUE;

	case WM_INITMENUPOPUP:
		UpdateMenu(GetMenu(hWnd));
		break;

	case WM_CONTEXTMENU:
		if (HandleTreeContextMenu(hWnd, wParam, lParam)
		||	HandleScreenShotContextMenu(hWnd, wParam, lParam))
			return FALSE;
		break;

	case WM_COMMAND:
		return MameCommand(hWnd,(int)(LOWORD(wParam)),(HWND)(lParam),(UINT)HIWORD(wParam));

	case WM_GETMINMAXINFO:
		/* Don't let the window get too small; it can break resizing */
		mminfo = (MINMAXINFO *) lParam;
		mminfo->ptMinTrackSize.x = MIN_WIDTH;
		mminfo->ptMinTrackSize.y = MIN_HEIGHT;
		return 0;

	case WM_TIMER:
		switch (wParam)
		{
		case JOYGUI_TIMER:
			PollGUIJoystick();
			break;
		case SCREENSHOT_TIMER:
			TabView_CalculateNextTab(hTabCtrl);
			UpdateScreenShot();
			TabView_UpdateSelection(hTabCtrl);
			break;
#if MULTISESSION
		case GAMEWND_TIMER:
			nGame = Picker_GetSelectedItem(hwndList);
			if( ! GetGameCaption() )
			{
				hGameWnd = GetGameWindow();
				if( hGameWnd )
				{
					lGameWndStyle = GetWindowLong(hGameWnd, GWL_STYLE);
					lGameWndStyle = lGameWndStyle & (WS_BORDER ^ 0xffffffff);
					SetWindowLong(hGameWnd, GWL_STYLE, lGameWndStyle);
					SetWindowPos(hGameWnd,0,0,0,0,0,SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			if( SendIconToEmulationWindow(nGame)== TRUE);
				KillTimer(hMain, GAMEWND_TIMER);
#endif // MULTISESSION
			break;
		default:
			break;
		}
		return TRUE;

	case WM_CLOSE:
		{
			/* save current item */
			RECT rect;
			AREA area;
			int widths[COLUMN_MAX];
			int order[COLUMN_MAX];
			int shown[COLUMN_MAX];
			int tmpOrder[COLUMN_MAX];
			int nItem;
			int nColumnMax = 0;
			WINDOWPLACEMENT wndpl;
			UINT state;

			wndpl.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hMain, &wndpl);
			state = wndpl.showCmd;

			/* Restore the window before we attempt to save parameters,
			 * This fixed the lost window on startup problem, among other problems
			 */
			if (state == SW_MINIMIZE || state == SW_SHOWMINIMIZED || state == SW_MAXIMIZE)
			{
				if( wndpl.flags & WPF_RESTORETOMAXIMIZED || state == SW_MAXIMIZE)
					state = SW_MAXIMIZE;
				else
				{
					state = SW_RESTORE;
					ShowWindow(hWnd, SW_RESTORE);
				}
			}
			for (i = 0; i < GetSplitterCount(); i++)
				SetSplitterPos(i, nSplitterOffset[i]);
			SetWindowState(state);

			GetColumnOrder(order);
			GetColumnShown(shown);
			GetColumnWidths(widths);

			// switch the list view to LVS_REPORT style so column widths reported correctly
			SetWindowLong(hwndList,GWL_STYLE,
			              (GetWindowLong(hwndList, GWL_STYLE) & ~LVS_TYPEMASK) | LVS_REPORT);

			nColumnMax = Picker_GetNumColumns(hwndList);

			if (oldControl)
			{
				for (i = 0; i < nColumnMax; i++)
				{
					widths[Picker_GetRealColumnFromViewColumn(hwndList, i)] =
						ListView_GetColumnWidth(hwndList, i);
				}
			}
			else
			{
				/* Get the Column Order and save it */
				ListView_GetColumnOrderArray(hwndList, nColumnMax, tmpOrder);

				for (i = 0; i < nColumnMax; i++)
				{
					widths[Picker_GetRealColumnFromViewColumn(hwndList, i)]
						= ListView_GetColumnWidth(hwndList, i);
					order[i] = Picker_GetRealColumnFromViewColumn(hwndList, tmpOrder[i]);
				}
			}

			SetColumnWidths(widths);
			SetColumnOrder(order);

			GetWindowRect(hWnd, &rect);
			area.x		= rect.left;
			area.y		= rect.top;
			area.width	= rect.right  - rect.left;
			area.height = rect.bottom - rect.top;
			SetWindowArea(&area);

			/* Save the users current game options and default game */
			nItem = Picker_GetSelectedItem(hwndList);
			SetDefaultGame(drivers[nItem]->name);

			/* hide window to prevent orphan empty rectangles on the taskbar */
			/* ShowWindow(hWnd,SW_HIDE); */
			DestroyWindow( hWnd );

			/* PostQuitMessage(0); */
			break;
		}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_LBUTTONDOWN:
		OnLButtonDown(hWnd, (UINT)wParam, MAKEPOINTS(lParam));
		break;

		/*
		  Check to see if the mouse has been moved by the user since
		  startup. I'd like this checking to be done only in the
		  main WinProc (here), but somehow the WM_MOUSEDOWN messages
		  are eaten up before they reach MameWindowProc. That's why
		  there is one check for each of the subclassed windows too.
    
		  POSSIBLE BUGS:
		  I've included this check in the subclassed windows, but a 
		  mose move in either the title bar, the menu, or the
		  toolbar will not generate a WM_MOUSEOVER message. At least
		  not one that I know how to pick up. A solution could maybe
		  be to subclass those too, but that's too much work :)
		*/

	case WM_MOUSEMOVE:
		if (MouseHasBeenMoved())
			ShowCursor(TRUE);

		if (g_listview_dragging)
			MouseMoveListViewDrag(MAKEPOINTS(lParam));
		else
			/* for splitters */
			OnMouseMove(hWnd, (UINT)wParam, MAKEPOINTS(lParam));
		break;

	case WM_LBUTTONUP:
		if (g_listview_dragging)
			ButtonUpListViewDrag(MAKEPOINTS(lParam));
		else
			/* for splitters */
			OnLButtonUp(hWnd, (UINT)wParam, MAKEPOINTS(lParam));
		break;

	case WM_NOTIFY:
		/* Where is this message intended to go */
		{
			LPNMHDR lpNmHdr = (LPNMHDR)lParam;

			/* Fetch tooltip text */
			if (lpNmHdr->code == TTN_NEEDTEXTW)
			{
				LPTOOLTIPTEXTW lpttt = (LPTOOLTIPTEXTW)lParam;
				CopyToolTipTextW(lpttt);
			}
			if (lpNmHdr->code == TTN_NEEDTEXTA)
			{
				LPTOOLTIPTEXTA lpttt = (LPTOOLTIPTEXTA)lParam;
				CopyToolTipTextA(lpttt);
			}

			if (lpNmHdr->hwndFrom == hTreeView)
				return TreeViewNotify(lpNmHdr);

			GetClassNameA(lpNmHdr->hwndFrom, szClass, sizeof(szClass) / sizeof(szClass[0]));
			if (!strcmp(szClass, "SysListView32"))
				return Picker_HandleNotify(lpNmHdr);	
			if (!strcmp(szClass, "SysTabControl32"))
				return TabView_HandleNotify(lpNmHdr);
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpDis = (LPDRAWITEMSTRUCT)lParam;

			GetClassNameA(lpDis->hwndItem, szClass, sizeof(szClass) / sizeof(szClass[0]));
			if (!strcmp(szClass, "SysListView32"))
#ifdef SMOOTH_SCROLL 
				SmoothScroll_DrawItem(lpDis);
#else
				Picker_HandleDrawItem(GetDlgItem(hMain, lpDis->CtlID), lpDis);
#endif /* SMOOTH_SCROLL */	
		}
		break;

	case WM_MEASUREITEM :
	{
		LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;

		// tell the list view that each row (item) should be just taller than our font

		//DefWindowProc(hWnd, message, wParam, lParam);
		//dprintf("default row height calculation gives %u\n",lpmis->itemHeight);

		TEXTMETRIC tm;
		HDC hDC = GetDC(NULL);
		HFONT hFontOld = (HFONT)SelectObject(hDC,hFont);

		GetTextMetrics(hDC,&tm);
		
		lpmis->itemHeight = tm.tmHeight + tm.tmExternalLeading + 1;
		if (lpmis->itemHeight < 17)
			lpmis->itemHeight = 17;
		//dprintf("we would do %u\n",tm.tmHeight + tm.tmExternalLeading + 1);
		SelectObject(hDC,hFontOld);
		ReleaseDC(NULL,hDC);

		return TRUE;
	}

	default:

		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

static int HandleKeyboardGUIMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	switch (message)
	{
		case WM_CHAR: /* List-View controls use this message for searching the items "as user types" */
			//MessageBox(NULL,"wm_char message arrived","TitleBox",MB_OK);
			return TRUE;

		case WM_KEYDOWN:
			KeyboardKeyDown(0, wParam, lParam);
			return TRUE;

		case WM_KEYUP:
			KeyboardKeyUp(0, wParam, lParam);
			return TRUE;

		case WM_SYSKEYDOWN:
			KeyboardKeyDown(1, wParam, lParam);
			return TRUE;

		case WM_SYSKEYUP:
			KeyboardKeyUp(1, wParam, lParam);
			return TRUE;
	}

	return FALSE;	/* message not processed */
}

static BOOL PumpMessage(void)
{
	MSG msg;

	if (!GetMessage(&msg, NULL, 0, 0))
	{
		return FALSE;
	}

	if (IsWindow(hMain))
	{
		BOOL absorbed_key = FALSE;
		if (GetKeyGUI())
			absorbed_key = HandleKeyboardGUIMessage(msg.hwnd, msg.message, 
			                                        msg.wParam, msg.lParam);
		else
			absorbed_key = TranslateAccelerator(hMain, hAccel, &msg);

		if (!absorbed_key)
		{
			if (!IsDialogMessage(hMain, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return TRUE;
}

static BOOL FolderCheck(void)
{
	char *pDescription = NULL;
	int nGameIndex = 0;
	int i=0;
	int iStep = 0;
	LV_FINDINFO lvfi;
	int nCount = ListView_GetItemCount(hwndList);
	BOOL changed = FALSE;

	MSG msg;
	for(i=0; i<nCount;i++)
	{
		LV_ITEM lvi;

		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.mask	 = LVIF_PARAM;
		ListView_GetItem(hwndList, &lvi);
		nGameIndex  = lvi.lParam;
		SetRomAuditResults(nGameIndex, UNKNOWN);
		SetSampleAuditResults(nGameIndex, UNKNOWN);
	}
	if( nCount > 0)
		ProgressBarShow();
	else
		return FALSE;
	if( nCount < 100 )
		iStep = 100 / nCount;
	else
		iStep = nCount/100;
	UpdateListView();
	UpdateWindow(hMain);
	for(i=0; i<nCount;i++)
	{
		LV_ITEM lvi;

		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.mask	 = LVIF_PARAM;
		ListView_GetItem(hwndList, &lvi);
		nGameIndex  = lvi.lParam;
		if (GetRomAuditResults(nGameIndex) == UNKNOWN)
		{
			Mame32VerifyRomSet(nGameIndex);
			changed = TRUE;
		}

		if (GetSampleAuditResults(nGameIndex) == UNKNOWN)
		{
			Mame32VerifySampleSet(nGameIndex);
			changed = TRUE;
		}

		lvfi.flags	= LVFI_PARAM;
		lvfi.lParam = nGameIndex;

		i = ListView_FindItem(hwndList, -1, &lvfi);
		if (changed && i != -1);
		{
			ListView_RedrawItems(hwndList, i, i);
			while( PeekMessage( &msg, hwndList, 0, 0, PM_REMOVE ) != 0)
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			}
		}
		changed = FALSE;
		if ((i % iStep) == 0)
			ProgressBarStepParam(i, nCount);
	}
	ProgressBarHide();
	pDescription = UseLangList() ? _LST(drivers[Picker_GetSelectedItem(hwndList)]->description) : ModifyThe(drivers[Picker_GetSelectedItem(hwndList)]->description);
	SetStatusBarText(0, pDescription);
	UpdateStatusBar();
	return TRUE;
}

static BOOL GameCheck(void)
{
	LV_FINDINFO lvfi;
	int i;
	BOOL changed = FALSE;

	if (game_index == 0)
		ProgressBarShow();

	if (game_index >= game_count)
	{
		bDoGameCheck = FALSE;
		ProgressBarHide();
		ResetWhichGamesInFolders();
#if 0
#ifdef FAST_AUDIT
		{
			extern const char *GetFastAuditResultFile(void);	/* src/audit.c */
			extern int FinalizeAudit(void);	/* src/audit.c */

			if(FinalizeAudit()) {
#ifdef KOREAN
				i = MessageBox(hMain, "롬파일 검사가 완료되었습니다.\n\n검사 결과를 보시겠습니까?", "검사 완료", MB_YESNO|MB_ICONQUESTION);
#else
				i = MessageBoxA(hMain, "ROM and samples audit completed. Do you want to see result?", "Audit Complete", MB_YESNO|MB_ICONQUESTION);
#endif
				if(i==IDYES) {
					char szCurDir[256];
					
					GetCurrentDirectoryA(256, szCurDir);
					ShellExecuteA(hMain, NULL, GetFastAuditResultFile(), NULL, szCurDir, SW_SHOW);
				}
			}
		}
#endif /*FAST_AUDIT*/
#endif
		return FALSE;
	}

	if (GetRomAuditResults(game_index) == UNKNOWN)
	{
#ifdef FAST_AUDIT
		Mame32FastVerifyRomSet(game_index);
#else
		Mame32VerifyRomSet(game_index);
#endif		
		changed = TRUE;
	}

	if (GetSampleAuditResults(game_index) == UNKNOWN)
	{
		Mame32VerifySampleSet(game_index);
		changed = TRUE;
	}

	lvfi.flags	= LVFI_PARAM;
	lvfi.lParam = game_index;

	i = ListView_FindItem(hwndList, -1, &lvfi);
	if (changed && i != -1);
		ListView_RedrawItems(hwndList, i, i);
	if ((game_index % progBarStep) == 0)
		ProgressBarStep();
	game_index++;

	return changed;
}

static BOOL OnIdle(HWND hWnd)
{
	static int bFirstTime = TRUE;
	static int bResetList = TRUE;

	char *pDescription;
	int driver_index;

	if (bFirstTime)
	{
		bResetList = FALSE;
		bFirstTime = FALSE;
		if (bListReady)
			Picker_ResetHeaderSortIcon(hwndList);
	}
	if (bDoGameCheck)
	{
		bResetList |= GameCheck();
		return idle_work;
	}
	// NPW 17-Jun-2003 - Commenting this out because it is redundant
	// and it causes the game to reset back to the original game after an F5 
	// refresh
	//driver_index = GetGameNameIndex(GetDefaultGame());
	//Picker_SetSelectedItem(hwndList, driver_index);

	// in case it's not found, get it back
	driver_index = Picker_GetSelectedItem(hwndList);

	pDescription = UseLangList() ? _LST(drivers[driver_index]->description) : ModifyThe(drivers[driver_index]->description);
	SetStatusBarText(0, pDescription);
	if (bResetList || (GetViewMode() == VIEW_LARGE_ICONS))
	{
		ResetWhichGamesInFolders();
		ResetListView();
	}
	idle_work = FALSE;
	UpdateStatusBar();
	bFirstTime = TRUE;

	if (!idle_work)
		PostMessage(GetMainWindow(),WM_COMMAND, MAKEWPARAM(ID_VIEW_LINEUPICONS, TRUE),(LPARAM)NULL);
	return idle_work;
}

static void OnSize(HWND hWnd, UINT nState, int nWidth, int nHeight)
{
	static BOOL firstTime = TRUE;

	if (nState != SIZE_MAXIMIZED && nState != SIZE_RESTORED)
		return;

	ResizeWindow(hWnd, &main_resize);
	ResizeProgressBar();
	if (firstTime == FALSE)
		OnSizeSplitter(hMain);
	//firstTime = FALSE;
	/* Update the splitters structures as appropriate */
	RecalcSplitters();
	if (firstTime == FALSE)
		ResizePickerControls(hMain);
	firstTime = FALSE;
	UpdateScreenShot();
}



static HWND GetResizeItemWindow(HWND hParent, const ResizeItem *ri)
{
	HWND hControl;
	if (ri->type == RA_ID)
		hControl = GetDlgItem(hParent, ri->u.id);
	else
		hControl = ri->u.hwnd;
	return hControl;
}



static void SetAllWindowsFont(HWND hParent, const Resize *r, HFONT hTheFont, BOOL bRedraw)
{
	int i;
	HWND hControl;

	for (i = 0; r->items[i].type != RA_END; i++)
	{
		if (r->items[i].setfont)
		{
			hControl = GetResizeItemWindow(hParent, &r->items[i]);
			SetWindowFont(hControl, hTheFont, bRedraw);
		}
	}

	hControl = GetDlgItem(hwndList, 0);
	if (hControl)
		TranslateControl(hControl);
}



static void ResizeWindow(HWND hParent, Resize *r)
{
	int cmkindex = 0, dx, dy, dx1, dtempx;
	HWND hControl;
	RECT parent_rect, rect;
	ResizeItem *ri;
	POINT p = {0, 0};

	if (hParent == NULL)
		return;

	/* Calculate change in width and height of parent window */
	GetClientRect(hParent, &parent_rect);
	//dx = parent_rect.right - r->rect.right;
	dtempx = parent_rect.right - r->rect.right;
	dy = parent_rect.bottom - r->rect.bottom;
	dx = dtempx/2;
	dx1 = dtempx - dx;
	ClientToScreen(hParent, &p);

	while (r->items[cmkindex].type != RA_END)
	{
		ri = &r->items[cmkindex];
		if (ri->type == RA_ID)
			hControl = GetDlgItem(hParent, ri->u.id);
		else
			hControl = ri->u.hwnd;

		if (hControl == NULL)
		{
			cmkindex++;
			continue;
		}

		/* Get control's rectangle relative to parent */
		GetWindowRect(hControl, &rect);
		OffsetRect(&rect, -p.x, -p.y);

		if (!(ri->action & RA_LEFT))
			rect.left += dx;

		if (!(ri->action & RA_TOP))
			rect.top += dy;

		if (ri->action & RA_RIGHT)
			rect.right += dx;

		if (ri->action & RA_BOTTOM)
			rect.bottom += dy;

		MoveWindow(hControl, rect.left, rect.top,
		           (rect.right - rect.left),
		           (rect.bottom - rect.top), TRUE);

		/* Take care of subcontrols, if appropriate */
		if (ri->subwindow != NULL)
			ResizeWindow(hControl, ri->subwindow);

		cmkindex++;
	}

	/* Record parent window's new location */
	memcpy(&r->rect, &parent_rect, sizeof(RECT));
}

static void ProgressBarShow(void)
{
	RECT rect;
	int  widths[2] = {250, -1};

	if (game_count < 100)
		progBarStep = 100 / game_count;
	else
		progBarStep = (game_count / 100);

	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)2, (LPARAM)(LPINT)widths);
	SendMessage(hProgWnd, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, game_count));
	SendMessage(hProgWnd, PBM_SETSTEP, (WPARAM)progBarStep, 0);
	SendMessage(hProgWnd, PBM_SETPOS, 0, 0);

	StatusBar_GetItemRect(hStatusBar, 1, &rect);

	MoveWindow(hProgWnd, rect.left, rect.top,
	           rect.right - rect.left,
	           rect.bottom - rect.top, TRUE);

	bProgressShown = TRUE;
}

static void ProgressBarHide(void)
{
	RECT rect;
	int  widths[4];
	HDC  hDC;
	SIZE size;
	int  numParts = 4;

	if (hProgWnd == NULL)
	{
		return;
	}

	hDC = GetDC(hProgWnd);

	ShowWindow(hProgWnd, SW_HIDE);

	GetTextExtentPoint32A(hDC, "MMX", 3 , &size);
	widths[3] = size.cx;
	GetTextExtentPoint32A(hDC, "MMMM games", 10, &size);
	widths[2] = size.cx;
	//Just specify 24 instead of 30, gives us sufficient space to display the message, and saves some space
	GetTextExtentPoint32(hDC, TEXT("Screen flip support is missing"), 24, &size);
	widths[1] = 400; // 게임상태가 표시되는곳
//	widths[1] = size.cx;

	ReleaseDC(hProgWnd, hDC);

	widths[0] = -1;
	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)1, (LPARAM)(LPINT)widths);
	StatusBar_GetItemRect(hStatusBar, 0, &rect);

	widths[0] = (rect.right - rect.left) - (widths[1] + widths[2] + widths[3]);
	widths[1] += widths[0];
	widths[2] += widths[1];
	widths[3] = -1;

	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)numParts, (LPARAM)(LPINT)widths);
	UpdateStatusBar();

	bProgressShown = FALSE;
}

static void ResizeProgressBar(void)
{
	if (bProgressShown)
	{
		RECT rect;
		int  widths[2] = {250, -1};

		SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)2, (LPARAM)(LPINT)widths);
		StatusBar_GetItemRect(hStatusBar, 1, &rect);
		MoveWindow(hProgWnd, rect.left, rect.top,
		           rect.right  - rect.left,
		           rect.bottom - rect.top, TRUE);
	}
	else
	{
		ProgressBarHide();
	}
}

static void ProgressBarStepParam(int iGameIndex, int nGameCount)
{
	SetStatusBarTextF(0, _UI("Game search %d%% complete"),
	                  ((iGameIndex + 1) * 100) / nGameCount);
	if (iGameIndex == 0)
		ShowWindow(hProgWnd, SW_SHOW);
	SendMessage(hProgWnd, PBM_STEPIT, 0, 0);
}

static void ProgressBarStep(void)
{
	ProgressBarStepParam(game_index, game_count);
}

static HWND InitProgressBar(HWND hParent)
{
	RECT rect;

	StatusBar_GetItemRect(hStatusBar, 0, &rect);

	rect.left += 250;

	return CreateWindowEx(WS_EX_STATICEDGE,
			PROGRESS_CLASS,
			TEXT("Progress Bar"),
			WS_CHILD | PBS_SMOOTH,
			rect.left,
			rect.top,
			rect.right	- rect.left,
			rect.bottom - rect.top,
			hParent,
			NULL,
			hInst,
			NULL);
}

static void CopyToolTipTextW(LPTOOLTIPTEXTW lpttt)
{
	int   i;
	int   id = lpttt->hdr.idFrom;
	int   iButton = lpttt->hdr.idFrom;
	static char String[1024];
	BOOL bConverted = FALSE;
	//LPWSTR pDest = lpttt->lpszText;
	/* Map command ID to string index */
	for (i = 0; CommandToString[i] != -1; i++)
	{
		if (CommandToString[i] == id)
		{
			iButton = i;
			bConverted = TRUE;
			break;
		}
	}

	if( bConverted )
	{
		/* Check for valid parameter */
		if (iButton > NUM_TOOLTIPS)
		{
			strcpy(String, _UI("Invalid Button Index"));
		}
		else
		{
			strcpy(String, (id==IDC_USE_LIST && GetLangcode()==UI_LANG_EN_US) ?
			       "Modify 'The'" : _UI(szTbStrings[iButton]));
		}
	}
	else if (iButton <= 2 )
	{
		//Statusbar
		SendMessage(lpttt->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 200);
		if (iButton != 1)
			SendMessage(hStatusBar, SB_GETTEXTA, (WPARAM)iButton, (LPARAM)(LPSTR) &String );
		else
			//for first pane we get the Status directly, to get the line breaks
			strcpy(String, _String(GameInfoStatus(Picker_GetSelectedItem(hwndList), FALSE)) );
	}
	else
		strcpy(String, _UI("Invalid Button Index"));

	lstrcpy(lpttt->lpszText, _Unicode(String));
}

static void CopyToolTipTextA(LPTOOLTIPTEXTA lpttt)
{
	int   i;
	int   id = lpttt->hdr.idFrom;
	int   iButton = lpttt->hdr.idFrom;
	static char String[1024];
	BOOL bConverted = FALSE;
	//LPSTR pDest = lpttt->lpszText;
	/* Map command ID to string index */
	for (i = 0; CommandToString[i] != -1; i++)
	{
		if (CommandToString[i] == id)
		{
			iButton = i;
			bConverted = TRUE;
			break;
		}
	}

	if( bConverted )
	{
		/* Check for valid parameter */
		if (iButton > NUM_TOOLTIPS)
		{
			strcpy(String, _UI("Invalid Button Index"));
		}
		else
		{
			strcpy(String, (id==IDC_USE_LIST && GetLangcode()==UI_LANG_EN_US) ?
			       "Modify 'The'" : _UI(szTbStrings[iButton]));
		}
	}
	else if (iButton <= 2 )
	{
		//Statusbar
		SendMessage(lpttt->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 140);
		if (iButton != 1)
			SendMessage(hStatusBar, SB_GETTEXTA, (WPARAM)iButton, (LPARAM)(LPSTR) &String );
		else
			//for first pane we get the Status directly, to get the line breaks
			strcpy(String, _String(GameInfoStatus(Picker_GetSelectedItem(hwndList), FALSE)) );
	}
	else
		strcpy(String, _UI("Invalid Button Index"));

	strcpy(lpttt->lpszText, String);
}

static HWND InitToolbar(HWND hParent)
{
	return CreateToolbarEx(hParent,
	                       WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
	                       CCS_TOP | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
	                       1,
	                       8,
	                       hInst,
//	                       IDB_TOOLBAR_US + GetLangcode(),
												 IDB_TOOLBAR,	
	                       tbb,
	                       NUM_TOOLBUTTONS,
	                       24,
	                       24,
	                       0,
	                       0,
	                       sizeof(TBBUTTON));
}

static HWND InitStatusBar(HWND hParent)
{
#if 0
	HMENU hMenu = GetMenu(hParent);

	popstr[0].hMenu    = 0;
	popstr[0].uiString = 0;
	popstr[1].hMenu    = hMenu;
	popstr[1].uiString = IDS_UI_FILE;
	popstr[2].hMenu    = GetSubMenu(hMenu, 1);
	popstr[2].uiString = IDS_UI_TOOLBAR;
	popstr[3].hMenu    = 0;
	popstr[3].uiString = 0;
#endif

	return CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
	                          CCS_BOTTOM | SBARS_SIZEGRIP | SBT_TOOLTIPS,
	                          _Unicode(_UI("Ready")),
	                          hParent,
	                          2);
}


static LRESULT Statusbar_MenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
#if 0
	UINT  fuFlags	= (UINT)HIWORD(wParam);
	HMENU hMainMenu = NULL;
	int   iMenu 	= 0;

	/* Handle non-system popup menu descriptions. */
	if (  (fuFlags & MF_POPUP)
	&&	(!(fuFlags & MF_SYSMENU)))
	{
		for (iMenu = 1; iMenu < MAX_MENUS; iMenu++)
		{
			if ((HMENU)lParam == popstr[iMenu].hMenu)
			{
				hMainMenu = (HMENU)lParam;
				break ;
			}
		}
	}

	if (hMainMenu)
	{
		/* Display helpful text in status bar */
		MenuHelp(WM_MENUSELECT, wParam, lParam, hMainMenu, hInst,
				 hStatusBar, (UINT *)&popstr[iMenu]);
	}
	else
	{
		UINT nZero = 0;
		MenuHelp(WM_MENUSELECT, wParam, lParam, NULL, hInst,
				 hStatusBar, &nZero);
	}
#endif
	char *p = TranslateMenuHelp((HMENU)lParam, (UINT)LOWORD(wParam), HIWORD(wParam) & MF_POPUP);
	StatusBarSetTextA(hStatusBar, 0, p);

	return 0;
}

static void UpdateStatusBar(void)
{
	LPTREEFOLDER lpFolder = GetCurrentFolder();
	int 		 games_shown = 0;
	int 		 i = -1;

	if (!lpFolder)
		return;

	while (1)
	{
		i = FindGame(lpFolder,i+1);
		if (i == -1)
			break;

		if (!GameFiltered(i, lpFolder->m_dwFlags))
			games_shown++;
	}

	/* Show number of games in the current 'View' in the status bar */
	SetStatusBarTextF(2, _UI("%d games"), games_shown);

	i = Picker_GetSelectedItem(hwndList);

	if (games_shown == 0)
		DisableSelection();
	else
		SetStatusBarTextW(1, GameInfoStatus(i, FALSE));
}

static void UpdateHistory(void)
{
	HDC hDC;
	RECT rect;
	TEXTMETRIC     tm ;
	int nLines, nLineHeight;
	//DWORD dwStyle = GetWindowLong(GetDlgItem(hMain, IDC_HISTORY), GWL_STYLE);
	have_history = FALSE;

	if (GetSelectedPick() >= 0)
	{
		LPCWSTR histText = GetGameHistory(Picker_GetSelectedItem(hwndList));

		have_history = (histText && histText[0]) ? TRUE : FALSE;
		Edit_SetText(GetDlgItem(hMain, IDC_HISTORY), histText);
	}

	if (have_history && GetShowScreenShot()
		&& ((TabView_GetCurrentTab(hTabCtrl) == TAB_HISTORY) || 
			(TabView_GetCurrentTab(hTabCtrl) == GetHistoryTab() && GetShowTab(TAB_HISTORY) == FALSE) ||
	        (TAB_ALL == GetHistoryTab() && GetShowTab(TAB_HISTORY) == FALSE) ))
	{
		Edit_GetRect(GetDlgItem(hMain, IDC_HISTORY),&rect);
		nLines = Edit_GetLineCount(GetDlgItem(hMain, IDC_HISTORY) );
		hDC = GetDC(GetDlgItem(hMain, IDC_HISTORY));
		GetTextMetrics (hDC, &tm);
		nLineHeight = tm.tmHeight - tm.tmInternalLeading;
		if( ( (rect.bottom - rect.top) / nLineHeight) < (nLines) )
		{
			//more than one Page, so show Scrollbar
			SetScrollRange(GetDlgItem(hMain, IDC_HISTORY), SB_VERT, 0, nLines, TRUE); 
		}
		else
		{
			//hide Scrollbar
			SetScrollRange(GetDlgItem(hMain, IDC_HISTORY),SB_VERT, 0, 0, TRUE);

		}
 		ShowWindow(GetDlgItem(hMain, IDC_HISTORY), SW_SHOW);
	}
	else
		ShowWindow(GetDlgItem(hMain, IDC_HISTORY), SW_HIDE);

}


static void DisableSelection(void)
{
	MENUITEMINFO	mmi;
	HMENU			hMenu = GetMenu(hMain);
	BOOL			prev_have_selection = have_selection;

	mmi.cbSize         = sizeof(mmi);
	mmi.fMask          = MIIM_TYPE;
	mmi.fType          = MFT_STRING;
	mmi.dwTypeData     = _Unicode(_UI("&Play"));
	mmi.cch            = lstrlen(mmi.dwTypeData);
	SetMenuItemInfo(hMenu, ID_FILE_PLAY, FALSE, &mmi);

	mmi.cbSize         = sizeof(mmi);
	mmi.fMask          = MIIM_TYPE;
	mmi.fType          = MFT_STRING;
	mmi.dwTypeData     = _Unicode(_UI("Source Proper&ties"));
	mmi.cch            = lstrlen(mmi.dwTypeData);
	SetMenuItemInfo(hMenu, ID_SOURCE_PROPERTIES, FALSE, &mmi);

	EnableMenuItem(hMenu, ID_FILE_PLAY, 		   MF_GRAYED);
	EnableMenuItem(hMenu, ID_FILE_PLAY_RECORD,	   MF_GRAYED);
	EnableMenuItem(hMenu, ID_GAME_PROPERTIES,	   MF_GRAYED);
	EnableMenuItem(hMenu, ID_SOURCE_PROPERTIES,	   MF_GRAYED);

#ifdef KAILLERA
	EnableMenuItem(hMenu, ID_FILE_NETPLAY,         MF_GRAYED);
#endif /* KAILLERA */
#ifdef MAME_AVI
  EnableMenuItem(hMenu, ID_FILE_PLAY_BACK_AVI,   MF_GRAYED);
	EnableMenuItem(hMenu, ID_FILE_PLAY_WITH_AVI,   MF_GRAYED);	// NEW_AVI
#endif

	SetStatusBarText(0, _UI("No Selection"));
	SetStatusBarText(1, "");
	SetStatusBarText(3, "");

	have_selection = FALSE;

	if (prev_have_selection != have_selection)
		UpdateScreenShot();
}

static void EnableSelection(int nGame)
{
	char		buf[200];
	const char *	pText;
	MENUITEMINFO	mmi;
	HMENU		hMenu = GetMenu(hMain);

	
	snprintf(buf, sizeof(buf) / sizeof(buf[0]), _UI("&Play %s"),
	         ConvertAmpersandString(UseLangList() ?
	                                _LST(drivers[nGame]->description) :
	                                ModifyThe(drivers[nGame]->description)));
	mmi.cbSize         = sizeof(mmi);
	mmi.fMask          = MIIM_TYPE;
	mmi.fType          = MFT_STRING;
	mmi.dwTypeData     = _Unicode(buf);
	mmi.cch            = lstrlen(mmi.dwTypeData);
	SetMenuItemInfo(hMenu, ID_FILE_PLAY, FALSE, &mmi);

	snprintf(buf, sizeof(buf) / sizeof(buf[0]), _UI("Proper&ties of %s"),
	         GetDriverFilename(nGame));
	mmi.cbSize         = sizeof(mmi);
	mmi.fMask          = MIIM_TYPE;
	mmi.fType          = MFT_STRING;
	mmi.dwTypeData     = _Unicode(buf);
	mmi.cch            = lstrlen(mmi.dwTypeData);
	SetMenuItemInfo(hMenu, ID_SOURCE_PROPERTIES, FALSE, &mmi);

	pText = UseLangList() ?
		_LST(drivers[nGame]->description) :
		ModifyThe(drivers[nGame]->description);
	SetStatusBarText(0, pText);
	/* Add this game's status to the status bar */
	SetStatusBarTextW(1, GameInfoStatus(nGame, FALSE));
	SetStatusBarText(3, "");

	/* If doing updating game status and the game name is NOT pacman.... */

	EnableMenuItem(hMenu, ID_FILE_PLAY, 		   MF_ENABLED);
	EnableMenuItem(hMenu, ID_FILE_PLAY_RECORD,	   MF_ENABLED);

#ifdef KAILLERA
	EnableMenuItem(hMenu, ID_FILE_NETPLAY,     MF_ENABLED);
#endif /* KAILLERA */
#ifdef MAME_AVI
  EnableMenuItem(hMenu, ID_FILE_PLAY_BACK_AVI,   MF_ENABLED);
	EnableMenuItem(hMenu, ID_FILE_PLAY_WITH_AVI,   MF_ENABLED); // NEW_AVI
#endif

	if (!oldControl)
	{
		EnableMenuItem(hMenu, ID_GAME_PROPERTIES,          MF_ENABLED);
		EnableMenuItem(hMenu, ID_SOURCE_PROPERTIES,	   MF_ENABLED);
	}

	if (bProgressShown && bListReady == TRUE)
	{
		SetDefaultGame(drivers[nGame]->name);
	}
	have_selection = TRUE;

	UpdateScreenShot();
}

static void PaintBackgroundImage(HWND hWnd, HRGN hRgn, int x, int y)
{
	RECT		rcClient;
	HRGN		rgnBitmap;
	HPALETTE	hPAL;
	HDC 		hDC = GetDC(hWnd);
	int 		i, j;
	HDC 		htempDC;
	HBITMAP 	oldBitmap;

	/* x and y are offsets within the background image that should be at 0,0 in hWnd */

	/* So we don't paint over the control's border */
	GetClientRect(hWnd, &rcClient);

	htempDC = CreateCompatibleDC(hDC);
#ifdef DARKMAME	// HISTORY_BG
	oldBitmap = SelectObject(htempDC, hSnapBackground);
#else	 /*DARKMAME*/
	oldBitmap = SelectObject(htempDC, hBackground);
#endif /*DARKMAME*/

	if (hRgn == NULL)
	{
		/* create a region of our client area */
		rgnBitmap = CreateRectRgnIndirect(&rcClient);
		SelectClipRgn(hDC, rgnBitmap);
		DeleteObject(rgnBitmap);
	}
	else
	{
		/* use the passed in region */
		SelectClipRgn(hDC, hRgn);
	}

#ifdef DARKMAME	// HISTORY_BG
	hPAL = GetSnapBackgroundPalette();
#else	/*DARKMAME*/
	hPAL = GetBackgroundPalette();
#endif /*DARKMAME*/

	if (hPAL == NULL)
		hPAL = CreateHalftonePalette(hDC);

	if (GetDeviceCaps(htempDC, RASTERCAPS) & RC_PALETTE && hPAL != NULL)
	{
		SelectPalette(htempDC, hPAL, FALSE);
		RealizePalette(htempDC);
	}

#ifdef DARKMAME
	for (i = rcClient.left-x; i < rcClient.right; i += bmSnapDesc.bmWidth)
		for (j = rcClient.top-y; j < rcClient.bottom; j += bmSnapDesc.bmHeight)
			BitBlt(hDC, i, j, bmSnapDesc.bmWidth, bmSnapDesc.bmHeight, htempDC, 0, 0, SRCCOPY);
#else	/*DARKMAME*/
	for (i = rcClient.left-x; i < rcClient.right; i += bmDesc.bmWidth)
		for (j = rcClient.top-y; j < rcClient.bottom; j += bmDesc.bmHeight)
			BitBlt(hDC, i, j, bmDesc.bmWidth, bmDesc.bmHeight, htempDC, 0, 0, SRCCOPY);
#endif /*DARKMAME*/
	
	SelectObject(htempDC, oldBitmap);
	DeleteDC(htempDC);

#ifdef DARKMAME	// HISTORY_BG
	if (GetSnapBackgroundPalette == NULL)
#else	/*DARKMAME*/
	if (GetBackgroundPalette() == NULL)
#endif /*DARKMAME*/	
	{
		DeleteObject(hPAL);
		hPAL = NULL;
	}

	ReleaseDC(hWnd, hDC);
}

static LPCSTR GetCloneParentName(int nItem)
{
	if (DriverIsClone(nItem) == TRUE)
	{
		if (UseLangList())
			return _LST(drivers[nItem]->clone_of->description);
		else
			return ModifyThe(drivers[nItem]->clone_of->description);
	}
	return "";
}

static BOOL PickerHitTest(HWND hWnd)
{
	RECT			rect;
	POINTS			p;
	DWORD			res = GetMessagePos();
	LVHITTESTINFO	htInfo;

	ZeroMemory(&htInfo,sizeof(LVHITTESTINFO));
	p = MAKEPOINTS(res);
	GetWindowRect(hWnd, &rect);
	htInfo.pt.x = p.x - rect.left;
	htInfo.pt.y = p.y - rect.top;
	ListView_HitTest(hWnd, &htInfo);

	return (! (htInfo.flags & LVHT_NOWHERE));
}

static BOOL TreeViewNotify(LPNMHDR nm)
{
	switch (nm->code)
	{
	case TVN_SELCHANGEDW :
	case TVN_SELCHANGEDA:
	    {
		    HTREEITEM hti = TreeView_GetSelection(hTreeView);
		    TVITEM	  tvi;
    
		    tvi.mask  = TVIF_PARAM | TVIF_HANDLE;
		    tvi.hItem = hti;
		    
		    if (TreeView_GetItem(hTreeView, &tvi))
		    {
			    SetCurrentFolder((LPTREEFOLDER)tvi.lParam);
			    if (bListReady)
			    {
				    ResetListView();
				    UpdateScreenShot();
			    }
		    }
		    return TRUE;
	    }
	case TVN_BEGINLABELEDITW :
	case TVN_BEGINLABELEDITA :
	    {
		    TV_DISPINFO *ptvdi = (TV_DISPINFO *)nm;
		    LPTREEFOLDER folder = (LPTREEFOLDER)ptvdi->item.lParam;
    
		    if (folder->m_dwFlags & F_CUSTOM)
		    {
			    // user can edit custom folder names
			    g_in_treeview_edit = TRUE;
			    return FALSE;
		    }
		    // user can't edit built in folder names
		    return TRUE;
	    }
	case TVN_ENDLABELEDITW :
	case TVN_ENDLABELEDITA :
	    {
		    TV_DISPINFO *ptvdi = (TV_DISPINFO *)nm;
		    LPTREEFOLDER folder = (LPTREEFOLDER)ptvdi->item.lParam;
    
		    g_in_treeview_edit = FALSE;
    
			if (ptvdi->item.pszText == NULL || lstrlen(ptvdi->item.pszText) == 0)
			    return FALSE;
    
			return TryRenameCustomFolder(folder,_String(ptvdi->item.pszText));
	    }
	}
	return FALSE;
}



static void GamePicker_OnHeaderContextMenu(POINT pt, int nColumn)
{
	// Right button was clicked on header
	HMENU hMenuLoad;
	HMENU hMenu;

	hMenuLoad = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT_HEADER));
	hMenu = GetSubMenu(hMenuLoad, 0);
	TranslateMenu(hMenu, ID_SORT_ASCENDING);
	lastColumnClick = nColumn;
	TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,0,hMain,NULL);

	DestroyMenu(hMenuLoad);
}



char* ConvertAmpersandString(const char *s)
{
	/* takes a string and changes any ampersands to double ampersands,
	   for setting text of window controls that don't allow us to disable
	   the ampersand underlining.
	 */
	/* returns a static buffer--use before calling again */

	static char buf[200];
	char *ptr;

	ptr = buf;
	while (*s)
	{
		if (*s == '&')
			*ptr++ = *s;
		*ptr++ = *s++;
	}
	*ptr = 0;

	return buf;
}

static int GUI_seq_pressed(input_code_t *code)
{
	int j;
	int res = 1;
	int invert = 0;
	int count = 0;

	for(j=0;j<SEQ_MAX;++j)
	{
		switch (code[j])
		{
			case CODE_NONE :
				return res && count;
			case CODE_OR :
				if (res && count)
					return 1;
				res = 1;
				count = 0;
				break;
			case CODE_NOT :
				invert = !invert;
				break;
			default:
				if (res)
				{
					int pressed = keyboard_state[code[j]];
					if ((pressed != 0) == invert)
						res = 0;
				}
				invert = 0;
				++count;
		}
	}
	return res && count;
}

static void check_for_GUI_action(void)
{
	int i;

	for (i = 0; i < NUM_GUI_SEQUENCES; i++)
	{
		input_seq_t *is = &(GUISequenceControl[i].is);

		if (GUI_seq_pressed(is->code))
		{
			dprintf("seq =%s pressed\n", GUISequenceControl[i].name);
			switch (GUISequenceControl[i].func_id)
			{
			case ID_GAME_AUDIT:
			case ID_GAME_PROPERTIES:
			case ID_CONTEXT_FILTERS:
			case ID_GAME_FILTERS:
			case ID_UI_START:
				KeyboardStateClear(); /* beacuse whe won't receive KeyUp mesage when we loose focus */
				break;
			default:
				break;
			}
			SendMessage(hMain, WM_COMMAND, GUISequenceControl[i].func_id, 0);
		}
	}
}

static void KeyboardStateClear(void)
{
	int i;

	for (i = 0; i < __code_max; i++)
		keyboard_state[i] = 0;

	dprintf("keyboard gui state cleared.\n");
}


static void KeyboardKeyDown(int syskey, int vk_code, int special)
{
	int i, found = 0;
	input_code_t icode = 0;
	int special_code = (special >> 24) & 1;
	int scancode = (special>>16) & 0xff;

	if ((vk_code==VK_MENU) || (vk_code==VK_CONTROL) || (vk_code==VK_SHIFT))
	{
		found = 1;

		/* a hack for right shift - it's better to use Direct X for keyboard input it seems......*/
		if (vk_code==VK_SHIFT)
			if (scancode>0x30) /* on my keyboard left shift scancode is 0x2a, right shift is 0x36 */
				special_code = 1;

		if (special_code) /* right hand keys */
		{
			switch(vk_code)
			{
			case VK_MENU:
				icode = KEYCODE_RALT;
				break;
			case VK_CONTROL:
				icode = KEYCODE_RCONTROL;
				break;
			case VK_SHIFT:
				icode = KEYCODE_RSHIFT;
				break;
			}
		}
		else /* left hand keys */
		{
			switch(vk_code)
			{
			case VK_MENU:
				icode = KEYCODE_LALT;
				break;
			case VK_CONTROL:
				icode = KEYCODE_LCONTROL;
				break;
			case VK_SHIFT:
				icode = KEYCODE_LSHIFT;
				break;
			}
		}
	}
	else
	{
		for (i = 0; i < __code_max; i++)
		{
			if ( vk_code == win_key_trans_table[i][VIRTUAL_KEY])
			{
				icode = win_key_trans_table[i][MAME_KEY];
				found = 1;
				break;
			}
		}
	}
	if (!found)
	{
		dprintf("VK_code pressed not found =  %i\n",vk_code);
		//MessageBox(NULL,"keydown message arrived not processed","TitleBox",MB_OK);
		return;
	}
	dprintf("VK_code pressed found =  %i, sys=%i, mame_keycode=%i special=%08x\n", vk_code, syskey, icode, special);
	keyboard_state[icode] = 1;
	check_for_GUI_action();
}

static void KeyboardKeyUp(int syskey, int vk_code, int special)
{
	int i, found = 0;
	input_code_t icode = 0;
	int special_code = (special >> 24) & 1;
	int scancode = (special>>16) & 0xff;

	if ((vk_code==VK_MENU) || (vk_code==VK_CONTROL) || (vk_code==VK_SHIFT))
	{
		found = 1;

		/* a hack for right shift - it's better to use Direct X for keyboard input it seems......*/
		if (vk_code==VK_SHIFT)
			if (scancode>0x30) /* on my keyboard left shift scancode is 0x2a, right shift is 0x36 */
				special_code = 1;

		if (special_code) /* right hand keys */
		{
			switch(vk_code)
			{
			case VK_MENU:
				icode = KEYCODE_RALT;
				break;
			case VK_CONTROL:
				icode = KEYCODE_RCONTROL;
				break;
			case VK_SHIFT:
				icode = KEYCODE_RSHIFT;
				break;
			}
		}
		else /* left hand keys */
		{
			switch(vk_code)
			{
			case VK_MENU:
				icode = KEYCODE_LALT;
				break;
			case VK_CONTROL:
				icode = KEYCODE_LCONTROL;
				break;
			case VK_SHIFT:
				icode = KEYCODE_LSHIFT;
				break;
			}
		}
	}
	else
	{
		for (i = 0; i < __code_max; i++)
		{
			if (vk_code == win_key_trans_table[i][VIRTUAL_KEY])
			{
				icode = win_key_trans_table[i][MAME_KEY];
				found = 1;
				break;
			}
		}
	}
	if (!found)
	{
		dprintf("VK_code released not found =  %i",vk_code);
		//MessageBox(NULL,"keyup message arrived not processed","TitleBox",MB_OK);
		return;
	}
	keyboard_state[icode] = 0;
	dprintf("VK_code released found=  %i, sys=%i, mame_keycode=%i special=%08x\n", vk_code, syskey, icode, special );
	check_for_GUI_action();
}

static void PollGUIJoystick(void)
{
	// For the exec timer, will keep track of how long the button has been pressed  
	static int exec_counter = 0;

	if (in_emulation)
		return;

	if (g_pJoyGUI == NULL)
		return;

	g_pJoyGUI->poll_joysticks();


	// User pressed UP
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyUp(0), GetUIJoyUp(1), GetUIJoyUp(2),GetUIJoyUp(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_UP, 0);
	}

	// User pressed DOWN
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyDown(0), GetUIJoyDown(1), GetUIJoyDown(2),GetUIJoyDown(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_DOWN, 0);
	}

	// User pressed LEFT
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyLeft(0), GetUIJoyLeft(1), GetUIJoyLeft(2),GetUIJoyLeft(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_LEFT, 0);
	}

	// User pressed RIGHT
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyRight(0), GetUIJoyRight(1), GetUIJoyRight(2),GetUIJoyRight(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_RIGHT, 0);
	}

	// User pressed START GAME
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyStart(0), GetUIJoyStart(1), GetUIJoyStart(2),GetUIJoyStart(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_START, 0);
	}

	// User pressed PAGE UP
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyPageUp(0), GetUIJoyPageUp(1), GetUIJoyPageUp(2),GetUIJoyPageUp(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_PGUP, 0);
	}

	// User pressed PAGE DOWN
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyPageDown(0), GetUIJoyPageDown(1), GetUIJoyPageDown(2),GetUIJoyPageDown(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_PGDOWN, 0);
	}

	// User pressed HOME
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyHome(0), GetUIJoyHome(1), GetUIJoyHome(2),GetUIJoyHome(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_HOME, 0);
	}

	// User pressed END
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyEnd(0), GetUIJoyEnd(1), GetUIJoyEnd(2),GetUIJoyEnd(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_END, 0);
	}

	// User pressed CHANGE SCREENSHOT
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoySSChange(0), GetUIJoySSChange(1), GetUIJoySSChange(2),GetUIJoySSChange(3))))
	{
		SendMessage(hMain, WM_COMMAND, IDC_SSFRAME, 0);
	}

	// User pressed SCROLL HISTORY UP
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyHistoryUp(0), GetUIJoyHistoryUp(1), GetUIJoyHistoryUp(2),GetUIJoyHistoryUp(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_HISTORY_UP, 0);
	}

	// User pressed SCROLL HISTORY DOWN
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyHistoryDown(0), GetUIJoyHistoryDown(1), GetUIJoyHistoryDown(2),GetUIJoyHistoryDown(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_HISTORY_DOWN, 0);
	}

	// User pressed EXECUTE COMMANDLINE
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyExec(0), GetUIJoyExec(1), GetUIJoyExec(2),GetUIJoyExec(3))))
	{
		if (++exec_counter >= GetExecWait()) // Button has been pressed > exec timeout
		{
			PROCESS_INFORMATION pi;
			STARTUPINFOA si;

			// Reset counter
			exec_counter = 0;

			ZeroMemory( &si, sizeof(si) );
			ZeroMemory( &pi, sizeof(pi) );
			si.dwFlags = STARTF_FORCEONFEEDBACK;
			si.cb = sizeof(si);

			CreateProcessA(NULL, GetExecCommand(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

			// We will not wait for the process to finish cause it might be a background task
			// The process won't get closed when MAME32 closes either.

			// But close the handles cause we won't need them anymore. Will not close process.
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}
	else
	{
		// Button has been released within the timeout period, reset the counter
		exec_counter = 0;
	}
}

#if 0
static void PressKey(HWND hwnd, UINT vk)
{
	SendMessage(hwnd, WM_KEYDOWN, vk, 0);
	SendMessage(hwnd, WM_KEYUP,   vk, 0xc0000000);
}
#endif

static void SetView(int menu_id)
{
	BOOL force_reset = FALSE;

	// first uncheck previous menu item, check new one
	CheckMenuRadioItem(GetMenu(hMain), ID_VIEW_LARGE_ICON, ID_VIEW_GROUPED, menu_id, MF_CHECKED);
	ToolBar_CheckButton(hToolBar, menu_id, MF_CHECKED);

	if (Picker_GetViewID(hwndList) == VIEW_GROUPED || menu_id == ID_VIEW_GROUPED)
	{
		// this changes the sort order, so redo everything
		force_reset = TRUE;
	}

	Picker_SetViewID(hwndList, menu_id - ID_VIEW_LARGE_ICON);

	if (force_reset)
	{
		Picker_Sort(hwndList);
	}
}

static void ResetListView(void)
{
	int 	i;
	int 	current_game;
	LV_ITEM lvi;
	BOOL	no_selection = FALSE;
	LPTREEFOLDER lpFolder = GetCurrentFolder();

	if (!lpFolder)
	{
		return;
	}

	/* If the last folder was empty, no_selection is TRUE */
	if (have_selection == FALSE)
	{
		no_selection = TRUE;
	}

	current_game = Picker_GetSelectedItem(hwndList);

	SetWindowRedraw(hwndList,FALSE);

	ListView_DeleteAllItems(hwndList);

	// hint to have it allocate it all at once
	ListView_SetItemCount(hwndList,game_count);

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.stateMask = 0;

	i = -1;

	do
	{
		/* Add the games that are in this folder */
		if ((i = FindGame(lpFolder, i + 1)) != -1)
		{
			if (GameFiltered(i, lpFolder->m_dwFlags))
				continue;

			lvi.iItem	 = i;
			lvi.iSubItem = 0;
			lvi.lParam	 = i;
			lvi.pszText  = LPSTR_TEXTCALLBACK;
			lvi.iImage	 = I_IMAGECALLBACK;
			ListView_InsertItem(hwndList, &lvi);
		}
	} while (i != -1);

	Picker_Sort(hwndList);

	if (bListReady)
	{
		/* If last folder was empty, select the first item in this folder */
		if (no_selection)
		    Picker_SetSelectedPick(hwndList, 0);
		else
		    Picker_SetSelectedItem(hwndList, current_game);
	}

	/*RS Instead of the Arrange Call that was here previously on all Views
		 We now need to set the ViewMode for SmallIcon again,
		 for an explanation why, see SetView*/
	if (GetViewMode() == VIEW_SMALL_ICONS)
		SetView(ID_VIEW_SMALL_ICON);

	SetWindowRedraw(hwndList, TRUE);

	UpdateStatusBar();

}

static void UpdateGameList(void)
{
	int i;

	for (i = 0; i < game_count; i++)
	{
		SetRomAuditResults(i, UNKNOWN);
		SetSampleAuditResults(i, UNKNOWN);
	}

	idle_work    = TRUE;
	bDoGameCheck = TRUE;
	game_index   = 0;

	ReloadIcons();

	// Let REFRESH also load new background if found
	LoadBackgroundBitmap();
	InvalidateRect(hMain,NULL,TRUE);
	Picker_ResetIdle(hwndList);
}

UINT_PTR CALLBACK CFHookProc(
  HWND hdlg,      // handle to dialog box
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
)
{
	int iIndex, i;
	COLORREF cCombo, cList;
	switch (uiMsg)
	{
		case WM_INITDIALOG:
			SendDlgItemMessageA(hdlg, cmb4, CB_ADDSTRING, 0, (LPARAM)(const void *)_UI("Custom"));
			iIndex = SendDlgItemMessage(hdlg, cmb4, CB_GETCOUNT, 0, 0);
			cList = GetListFontColor();
			SendDlgItemMessage(hdlg, cmb4, CB_SETITEMDATA,(WPARAM)iIndex-1,(LPARAM)cList );
			for( i = 0; i< iIndex; i++)
			{
				cCombo = SendDlgItemMessage(hdlg, cmb4, CB_GETITEMDATA,(WPARAM)i,0 );
				if( cList == cCombo)
				{
					SendDlgItemMessage(hdlg, cmb4, CB_SETCURSEL,(WPARAM)i,0 );
					break;
				}
			}
			break;
		case WM_COMMAND:
			if( LOWORD(wParam) == cmb4)
			{
				switch (HIWORD(wParam))
				{
					case CBN_SELCHANGE:  // The color ComboBox changed selection
						iIndex = (int)SendDlgItemMessage(hdlg, cmb4,
													  CB_GETCURSEL, 0, 0L);
						if( iIndex == SendDlgItemMessage(hdlg, cmb4, CB_GETCOUNT, 0, 0)-1)
						{
							//Custom color selected
 							cList = GetListFontColor();
 							PickColor(&cList);
							SendDlgItemMessage(hdlg, cmb4, CB_DELETESTRING, iIndex, 0);
							SendDlgItemMessageA(hdlg, cmb4, CB_ADDSTRING, 0, (LPARAM)(const void *)_UI("Custom"));
							SendDlgItemMessage(hdlg, cmb4, CB_SETITEMDATA,(WPARAM)iIndex,(LPARAM)cList);
							SendDlgItemMessage(hdlg, cmb4, CB_SETCURSEL,(WPARAM)iIndex,0 );
							return TRUE;
						}
				}
			}
			break;
	}
	return FALSE;
}

static void PickFont(void)
{
	LOGFONTA font;
	CHOOSEFONTA cf;
	HWND hWnd;

	GetListFont(&font);
	font.lfQuality = DEFAULT_QUALITY;

	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner   = hMain;
	cf.lpLogFont   = &font;
	cf.lpfnHook = &CFHookProc;
	cf.rgbColors   = GetListFontColor();
	cf.Flags	   = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_ENABLEHOOK;
	if (!ChooseFontA(&cf))
		return;

	SetListFont(&font);
	if (hFont != NULL)
		DeleteObject(hFont);
	hFont = TranslateCreateFont(&font);
	if (hFont != NULL)
	{
		COLORREF textColor = cf.rgbColors;

		if (textColor == RGB(255,255,255))
		{
			textColor = RGB(240, 240, 240);
		}

		SetAllWindowsFont(hMain, &main_resize, hFont, TRUE);

		hWnd = GetWindow(hMain, GW_CHILD);
		while(hWnd)
		{
			char szClass[128];
			if (GetClassNameA(hWnd, szClass, sizeof(szClass) / sizeof(szClass[0])))
			{
				if (!strcmp(szClass, "SysListView32"))
				{
					ListView_SetTextColor(hWnd, textColor);
				}
				else if (!strcmp(szClass, "SysTreeView32"))
				{
					TreeView_SetTextColor(hTreeView, textColor);
				}
			}
			hWnd = GetWindow(hWnd, GW_HWNDNEXT);
		}
		SetListFontColor(cf.rgbColors);
		ResetListView();
	}
}

static void PickColor(COLORREF *cDefault)
{
	CHOOSECOLOR cc;
	COLORREF choice_colors[16];
	int i;
	
	for (i=0;i<16;i++)
		choice_colors[i] = GetCustomColor(i);
 
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner   = hMain;
	cc.rgbResult   = *cDefault;
	cc.lpCustColors = choice_colors;
	cc.Flags       = CC_ANYCOLOR | CC_RGBINIT | CC_SOLIDCOLOR;
	if (!ChooseColor(&cc))
		return;
	for (i=0;i<16;i++)
		SetCustomColor(i,choice_colors[i]);
	*cDefault = cc.rgbResult;
}

static void PickCloneColor(void)
{
	COLORREF cClonecolor;
	cClonecolor = GetListCloneColor();
	PickColor( &cClonecolor);
	SetListCloneColor(cClonecolor);
	InvalidateRect(hwndList,NULL,FALSE);
}


#ifdef DARKMAME
static void PickHistoryFont(void)
{
	LOGFONTA font;
	CHOOSEFONTA cf;

	GetHistFont(&font);
	font.lfQuality = DEFAULT_QUALITY;

	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner   = hMain;
	cf.lpLogFont   = &font;
	cf.rgbColors   = GetHistFontColor();
	cf.Flags	   = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
	if (!ChooseFontA(&cf))
		return;

	SetHistFont(&font);
	if (hHistFont != NULL) DeleteObject(hHistFont);
	hHistFont = TranslateCreateFont(&font);	
	
	if (hHistFont != NULL)
	{
		COLORREF textColor = cf.rgbColors;

		if (textColor == RGB(255,255,255))
		{
			textColor = RGB(240,240,240);
		}

		SetHistFontColor(cf.rgbColors);
		SetWindowFont(GetDlgItem(hMain, IDC_HISTORY), hHistFont, TRUE);
	}
}

static void UpdateFontAndColorInfo(void)
{
	LOGFONTA font;
	BOOL	bNeedUpdate;
	HWND	hWnd;
	char	szClass[128];
	
	
	bNeedUpdate = FALSE;
	
	// PickFont() 함수 참고
	GetListFont(&font);

	if (hFont != NULL) DeleteObject(hFont);						
	hFont = TranslateCreateFont(&font);
	if (hFont != NULL)
	{
		COLORREF textColor = GetListFontColor();

		if (textColor == RGB(255,255,255))
        {
			textColor = RGB(240, 240, 240);
		}
		
		hWnd = GetWindow(hMain, GW_CHILD);
		while(hWnd)
		{
			if (GetClassNameA(hWnd, szClass, sizeof(szClass) / sizeof(szClass[0])))
			{
				if (!strcmp(szClass, "SysListView32"))
				{
					ListView_SetTextColor(hWnd, textColor);
				}
				else if (!strcmp(szClass, "SysTreeView32"))
				{
					TreeView_SetTextColor(hTreeView, textColor);
				}
			}
			hWnd = GetWindow(hWnd, GW_HWNDNEXT);
		}
		
		SetAllWindowsFont(hMain, &main_resize, hFont, TRUE);
	}

	// History Font Information
	GetHistFont(&font);

	if (hHistFont != NULL) DeleteObject(hHistFont);
	hHistFont = TranslateCreateFont(&font);
	
	if (hHistFont != NULL)
	{
		SetWindowFont(GetDlgItem(hMain, IDC_HISTORY), hHistFont, TRUE);
	}
	
#ifdef KAILLERA
	// Kaillera Chat Font
	GetChatFont(&font);

	if (hChatFont != NULL) DeleteObject(hChatFont);
	hChatFont = CreateFontIndirect(&font);
#endif	
}

#endif /* DARKMAME */

#ifdef KAILLERA
static void PickChatFont(void)
{
	LOGFONTA font;
	CHOOSEFONTA cf;

	GetChatFont(&font);
	font.lfQuality = DEFAULT_QUALITY;

	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner   = hMain;
	cf.lpLogFont   = &font;
	cf.rgbColors   = GetChatFontColor();
	cf.Flags       = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
	if (!ChooseFontA(&cf))
		return;

	SetChatFont(&font);
	if (hChatFont != NULL)
		DeleteObject(hChatFont);
	hChatFont = CreateFontIndirectA(&font);
	if (hChatFont != NULL)
	{
		COLORREF textColor = cf.rgbColors;

		if (textColor == RGB(255,255,255))
		{
			textColor = RGB(240, 240, 240);
		}

		SetChatFontColor(cf.rgbColors);
//		SetWindowFont(GetDlgItem(hMain, IDC_HISTORY), hChatFont, FALSE);
	}
}

int WINAPI kGameCallback(char *game, int player, int numplayers)
{
	int         i;
	options_type *o;

	for (i = 0; i < game_count; i++) {
		if (strcmp(game, sorted_titles[i].name) == 0) break;
	}

	if (i >= game_count)
	{
		kailleraEndGame();
		return 0;
	}

	srand(0xbadc0de);

	/* sound HAS to be enabled otherwise some games (a lot actually) in MAME will desynch */
	o = GetDefaultOptions();
	if (!o->enable_sound)
	{
		o->enable_sound = 1;
		o->samplerate = 11025;
	}

	o = GetGameOptions(sorted_titles[i].index);
	if (!o->enable_sound)
	{
		o->enable_sound = 1;
		o->samplerate = 11025;
	}

	kPlayers = numplayers;
	kPlayerNB = player;
	kPlay = 1;

	if(HIBYTE(GetAsyncKeyState(VK_SHIFT)) && HIBYTE(GetAsyncKeyState(VK_CONTROL)))
	{
		KailleraPlayRecordGame(sorted_titles[i].index);
	}
	else
	{
		g_pPlayBkName = NULL;
		g_pRecordName = NULL;
		MamePlayGameWithOptions(sorted_titles[i].index);
	}

	kailleraEndGame();
	kPlay = 0;
	kPlayers = 0;
	kPlayerNB = 0;

	return 0;
}

void WINAPI kChatCallback(char *nick, char *text)
{
	char tmp[512];
	if (text[0] == KCOMMAND_CHAR)
	{
		KailleraCommand(text);
	} else {
		sprintf(tmp,"<%s> %s",nick,text);
		KailleraChateReceive(tmp);
	}
}

void WINAPI kDropCallback(char *nick, int playernb)
{
	char tmp[512];
	sprintf(tmp,"* Player %i (%s) dropped from the current game.",playernb,nick);
	KailleraChateReceive(tmp);
	kPlayers--;
	if(ksavecounter) {
		ksavecounter = 0;
		usrintf_showmessage("Autosaving disabled");
	}
}

void WINAPI kInfosCallback(char *gamename)
{
	int i, f = -1;

	for (i = 0; i < game_count && f == -1; i++)
		if (!strcmp(ModifyThe(drivers[i]->description), gamename)) f = i;

	if (f != -1)
	{
		char tmpurl[512];

		sprintf(tmpurl, "http://www.mame.dk/gameinfo/%s/", drivers[f]->name);
		ShellExecuteA(NULL, "open", tmpurl, NULL, ".", 0);
	}
}

static kailleraInfos kInfos=
{
	NULL,
	NULL,
	kGameCallback,
	kChatCallback,
	kDropCallback,
	kInfosCallback
};
#endif /* KAILLERA */

static BOOL MameCommand(HWND hwnd,int id, HWND hwndCtl, UINT codeNotify)
{
	int i;

	if ((id >= ID_LANGUAGE_ENGLISH_US) && (id < ID_LANGUAGE_ENGLISH_US + UI_LANG_MAX))
	{
		ChangeLanguage(id);
		return TRUE;
	}

	switch (id)
	{
	case ID_FILE_PLAY:
		MamePlayGame();
		return TRUE;

	case ID_FILE_PLAY_RECORD:
		MamePlayRecordGame();
		return TRUE;

	case ID_FILE_PLAY_BACK:
		MamePlayBackGame();
		return TRUE;

#ifdef MAME_AVI
    case ID_FILE_PLAY_BACK_AVI:
        MamePlayBackGameAVI();
        return TRUE;

	case ID_FILE_PLAY_WITH_AVI:	// NEW_AVI
		MamePlayGameAVI();
		return TRUE;
#endif

	case ID_FILE_PLAY_RECORD_WAVE:
		MamePlayRecordWave();
		return TRUE;

	case ID_FILE_LOADSTATE :
		MameLoadState();
		return TRUE;
		
#ifdef KAILLERA
	case ID_FILE_KAILLERA_PLAY_BACK:
		kPlay = 1;
		MamePlayBackGame();
		kPlay = 0;
		return TRUE;

	case ID_FILE_NETPLAY:
	{
		char mameVer[512];
		char *tmpGames = NULL;
            int gamesSize  = 6000;
		int j, s;

		tmpGames = (char *)malloc(gamesSize);
		tmpGames[0] = 0;
		tmpGames[1] = 0;
		s = 0;

		// Get a list of avaliable game titles
		for (j = 0; j < game_count; j++)
		{
			char *p;
			char tmptxt[512];
			int l;

		        if (IsGameWorking(sorted_titles[j].index)
					&& IsAuditResultYes(GetRomAuditResults(sorted_titles[j].index)))
			{
				strcpy(tmptxt, sorted_titles[j].name);
				while ((s + strlen(tmptxt) + 4) > gamesSize)
				{
					gamesSize *= 2;
					tmpGames = (char *)realloc(tmpGames, gamesSize);
				}
				p = tmpGames + s;
				strcpy(p, tmptxt);
				l = strlen(p) + 1;
				p[l] = 0;
				s += l;
			}
		}

		{
			char *p;
			char tmptxt[512];
			int l;

			strcpy(tmptxt, "*Chat (not game)");
			while ((s + strlen(tmptxt) + 4) > gamesSize)
			{
				gamesSize *= 2;
				tmpGames = (char *)realloc(tmpGames, gamesSize);
			}
			p = tmpGames + s;
			strcpy(p, tmptxt);
			l = strlen(p) + 1;
			p[l] = 0;
			s += l;

			strcpy(tmptxt, "*Away (leave messages)");
			while ((s + strlen(tmptxt) + 4) > gamesSize)
			{
				gamesSize *= 2;
				tmpGames = (char *)realloc(tmpGames, gamesSize);
			}
			p = tmpGames + s;
			strcpy(p, tmptxt);
			l = strlen(p) + 1;
			p[l] = 0;
			s += l;
		}

		sprintf(mameVer, "EKMAME %s", GetVersionString());

		kInfos.appName  = mameVer;
		kInfos.gameList = tmpGames;
		kailleraSetInfos(&kInfos);
		free(tmpGames);

		kailleraSelectServerDialog(hMain);
	}
	return TRUE;
#endif /* KAILLERA */

	case ID_FILE_AUDIT:
		AuditDialog(hMain);
		ResetWhichGamesInFolders();
		ResetListView();
		SetFocus(hwndList);
		return TRUE;

#ifdef USE_SAVE_GAMELIST
	case ID_FILE_GAMELIST:
	{
		BOOL bIsTrue = FALSE;
		OPENFILENAMEA OpenFileName;
		char szFile[MAX_PATH]   = "\0";
		char szCurDir[MAX_PATH] = "\1";

		// Save current directory (avoids mame file creation further failure)
		if ( GetCurrentDirectoryA(MAX_PATH, szCurDir) > MAX_PATH )
		{
			// If path is too large than Null
			szCurDir[0] = 0;
		}

		OpenFileName.lStructSize       = sizeof(OPENFILENAME);
		OpenFileName.hwndOwner         = hMain;
		OpenFileName.hInstance         = 0;
		OpenFileName.lpstrFilter       = "Formatted text file (*.txt)\0*.txt\0Tabuled text file (*.txt)\0*.txt\0";
		OpenFileName.lpstrCustomFilter = NULL;
		OpenFileName.nMaxCustFilter    = 0;
		OpenFileName.nFilterIndex      = 1;
		OpenFileName.lpstrFile         = szFile;
		OpenFileName.nMaxFile          = sizeof(szFile);
		OpenFileName.lpstrFileTitle    = NULL;
		OpenFileName.nMaxFileTitle     = 0;
		OpenFileName.lpstrInitialDir   = NULL;
		OpenFileName.lpstrTitle        = "Pick a file name to save the game list";
		OpenFileName.nFileOffset       = 0;
		OpenFileName.nFileExtension    = 0;
		OpenFileName.lpstrDefExt       = ".txt";
		OpenFileName.lCustData         = 0;
		OpenFileName.lpfnHook          = NULL;
		OpenFileName.lpTemplateName    = NULL;
		OpenFileName.Flags             = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;


		while( !bIsTrue )
		{
			if ( GetOpenFileNameA(&OpenFileName) )
			{
				if ( GetFileAttributesA(szFile) != -1 )
				{
					if ( MessageBoxA(GetMainWindow(), "File already exists, overwrite ?" , MAME32NAME, MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK )
						continue;
					else
						bIsTrue = TRUE;

					SetFileAttributesA(szFile, FILE_ATTRIBUTE_NORMAL);
				}

				SaveGameListToFile(szFile, (OpenFileName.nFilterIndex==2 ? 0 : 1));

				bIsTrue = TRUE;
			}
			else
				break;
		}

		// Restore current file path
		if ( szCurDir[0] != 0 )
			SetCurrentDirectoryA(szCurDir);

		if ( bIsTrue )
			return TRUE;

		break;
	}

	case ID_FILE_ROMSLIST:
	{
		BOOL bIsTrue = FALSE;
		OPENFILENAMEA OpenFileName;
		char szFile[MAX_PATH]   = "\0";
		char szCurDir[MAX_PATH] = "\1";

		// Save current directory (avoids mame file creation further failure)
		if ( GetCurrentDirectoryA(MAX_PATH, szCurDir) > MAX_PATH )
		{
			// If path is too large than Null
			szCurDir[0] = 0;
		}

		OpenFileName.lStructSize       = sizeof(OPENFILENAME);
		OpenFileName.hwndOwner         = hMain;
		OpenFileName.hInstance         = 0;
		OpenFileName.lpstrFilter       = "Standard text file (*.txt)\0*.txt\0";
		OpenFileName.lpstrCustomFilter = NULL;
		OpenFileName.nMaxCustFilter    = 0;
		OpenFileName.nFilterIndex      = 1;
		OpenFileName.lpstrFile         = szFile;
		OpenFileName.nMaxFile          = sizeof(szFile);
		OpenFileName.lpstrFileTitle    = NULL;
		OpenFileName.nMaxFileTitle     = 0;
		OpenFileName.lpstrInitialDir   = NULL;
		OpenFileName.lpstrTitle        = "Pick a file name to save the ROMs list";
		OpenFileName.nFileOffset       = 0;
		OpenFileName.nFileExtension    = 0;
		OpenFileName.lpstrDefExt       = ".txt";
		OpenFileName.lCustData         = 0;
		OpenFileName.lpfnHook          = NULL;
		OpenFileName.lpTemplateName    = NULL;
		OpenFileName.Flags             = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

		while( !bIsTrue )
		{
			if ( GetOpenFileNameA(&OpenFileName) )
			{
				if ( GetFileAttributesA(szFile) != -1 )
				{
					if ( MessageBoxA(GetMainWindow(), "File already exists, overwrite ?" , MAME32NAME, MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK )
						continue;
					else
						bIsTrue = TRUE;

					SetFileAttributesA(szFile, FILE_ATTRIBUTE_NORMAL);
				}

				SaveRomsListToFile(szFile);

				bIsTrue = TRUE;
			}
			else
				break;
		}

		// Restore current file path
		if ( szCurDir[0] != 0 )
			SetCurrentDirectoryA(szCurDir);
		
		if ( bIsTrue )
			return TRUE;

		break;
	}
#endif /* USE_SAVE_GAMELIST */

	case ID_FILE_EXIT:
		PostMessage(hMain, WM_CLOSE, 0, 0);
		return TRUE;

	case ID_VIEW_LARGE_ICON:
		SetView(ID_VIEW_LARGE_ICON);
		return TRUE;

	case ID_VIEW_SMALL_ICON:
		SetView(ID_VIEW_SMALL_ICON);
		ResetListView();
		return TRUE;

	case ID_VIEW_LIST_MENU:
		SetView(ID_VIEW_LIST_MENU);
		return TRUE;

	case ID_VIEW_DETAIL:
		SetView(ID_VIEW_DETAIL);
		return TRUE;

	case ID_VIEW_GROUPED:
		SetView(ID_VIEW_GROUPED);
		return TRUE;

	/* Arrange Icons submenu */
	case ID_VIEW_BYGAME:
		SetSortReverse(FALSE);
		SetSortColumn(COLUMN_GAMES);
		Picker_Sort(hwndList);
		break;

	case ID_VIEW_BYDIRECTORY:
		SetSortReverse(FALSE);
		SetSortColumn(COLUMN_DIRECTORY);
		Picker_Sort(hwndList);
		break;

	case ID_VIEW_BYMANUFACTURER:
		SetSortReverse(FALSE);
		SetSortColumn(COLUMN_MANUFACTURER);
		Picker_Sort(hwndList);
		break;

	case ID_VIEW_BYTIMESPLAYED:
		SetSortReverse(FALSE);
		SetSortColumn(COLUMN_PLAYED);
		Picker_Sort(hwndList);
		break;

	case ID_VIEW_BYTYPE:
		SetSortReverse(FALSE);
		SetSortColumn(COLUMN_TYPE);
		Picker_Sort(hwndList);
		break;

	case ID_VIEW_BYYEAR:
		SetSortReverse(FALSE);
		SetSortColumn(COLUMN_YEAR);
		Picker_Sort(hwndList);
		break;

	case ID_VIEW_FOLDERS:
		bShowTree = !bShowTree;
		SetShowFolderList(bShowTree);
		CheckMenuItem(GetMenu(hMain), ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
		UpdateScreenShot();
		break;

	case ID_VIEW_TOOLBARS:
		bShowToolBar = !bShowToolBar;
		SetShowToolBar(bShowToolBar);
		CheckMenuItem(GetMenu(hMain), ID_VIEW_TOOLBARS, (bShowToolBar) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_TOOLBARS, (bShowToolBar) ? MF_CHECKED : MF_UNCHECKED);
		ShowWindow(hToolBar, (bShowToolBar) ? SW_SHOW : SW_HIDE);
		ResizePickerControls(hMain);
		UpdateScreenShot();
		break;

	case ID_VIEW_STATUS:
		bShowStatusBar = !bShowStatusBar;
		SetShowStatusBar(bShowStatusBar);
		CheckMenuItem(GetMenu(hMain), ID_VIEW_STATUS, (bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_STATUS, (bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED);
		ShowWindow(hStatusBar, (bShowStatusBar) ? SW_SHOW : SW_HIDE);
		ResizePickerControls(hMain);
		UpdateScreenShot();
		break;

	case ID_VIEW_PAGETAB:
		bShowTabCtrl = !bShowTabCtrl;
		SetShowTabCtrl(bShowTabCtrl);
		ShowWindow(hTabCtrl, (bShowTabCtrl) ? SW_SHOW : SW_HIDE);
		ResizePickerControls(hMain);
		UpdateScreenShot();
		InvalidateRect(hMain,NULL,TRUE);
		break;

		/*
		  Switches to fullscreen mode. No check mark handeling 
		  for this item cause in fullscreen mode the menu won't
		  be visible anyways.
		*/    
	case ID_VIEW_FULLSCREEN:
		SwitchFullScreenMode();
		break;

	case IDC_USE_LIST:
		SetUseLangList(!UseLangList());
		ToolBar_CheckButton(hToolBar, IDC_USE_LIST, UseLangList() ^ (GetLangcode() == UI_LANG_EN_US) ? MF_CHECKED : MF_UNCHECKED);
		ResetListView();
		UpdateHistory();
		break;

	case ID_GAME_AUDIT:
		InitGameAudit(0);
		if (!oldControl)
		{
			InitPropertyPageToPage(hInst, hwnd, Picker_GetSelectedItem(hwndList), GetSelectedPickItemIcon(), AUDIT_PAGE, NULL);
			SaveGameOptions(Picker_GetSelectedItem(hwndList));
		}
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;

	/* ListView Context Menu */
	case ID_CONTEXT_ADD_CUSTOM:
	{
		int  nResult;

		nResult = DialogBoxParam(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_CUSTOM_FILE),
								 hMain,AddCustomFileDialogProc,Picker_GetSelectedItem(hwndList));
		SetFocus(hwndList);
		break;
	}

	case ID_CONTEXT_REMOVE_CUSTOM:
	{
		RemoveCurrentGameCustomFolder();
		break;
	}

	/* Tree Context Menu */
	case ID_GAME_FILTERS:
		if (DialogBox(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_GAMEFINDS), hMain, FilterDialogProc) == TRUE)
			ResetListView();
		SetFocus(hwndList);
		return TRUE;
	case ID_CONTEXT_FILTERS:
		if (DialogBox(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_FILTERS), hMain, FilterDialogProc) == TRUE)
			ResetListView();
		SetFocus(hwndList);
		return TRUE;

	// ScreenShot Context Menu
	// select current tab
	case ID_VIEW_TAB_SCREENSHOT :
	case ID_VIEW_TAB_FLYER :
	case ID_VIEW_TAB_CABINET :
	case ID_VIEW_TAB_MARQUEE :
	case ID_VIEW_TAB_TITLE :
	case ID_VIEW_TAB_CONTROL_PANEL :
	case ID_VIEW_TAB_HISTORY :
		if (id == ID_VIEW_TAB_HISTORY && GetShowTab(TAB_HISTORY) == FALSE)
			break;

		TabView_SetCurrentTab(hTabCtrl, id - ID_VIEW_TAB_SCREENSHOT);
		UpdateScreenShot();
		TabView_UpdateSelection(hTabCtrl);
		break;

		// toggle tab's existence
	case ID_TOGGLE_TAB_SCREENSHOT :
	case ID_TOGGLE_TAB_FLYER :
	case ID_TOGGLE_TAB_CABINET :
	case ID_TOGGLE_TAB_MARQUEE :
	case ID_TOGGLE_TAB_TITLE :
	case ID_TOGGLE_TAB_CONTROL_PANEL :
	case ID_TOGGLE_TAB_HISTORY :
	{
		int toggle_flag = id - ID_TOGGLE_TAB_SCREENSHOT;

		if (AllowedToSetShowTab(toggle_flag,!GetShowTab(toggle_flag)) == FALSE)
		{
			// attempt to hide the last tab
			// should show error dialog? hide picture area? or ignore?
			break;
		}

		SetShowTab(toggle_flag,!GetShowTab(toggle_flag));

		TabView_Reset(hTabCtrl);

		if (TabView_GetCurrentTab(hTabCtrl) == toggle_flag && GetShowTab(toggle_flag) == FALSE)
		{
			// we're deleting the tab we're on, so go to the next one
			TabView_CalculateNextTab(hTabCtrl);
		}


		// Resize the controls in case we toggled to another history
		// mode (and the history control needs resizing).

		ResizePickerControls(hMain);
		UpdateScreenShot();

		TabView_UpdateSelection(hTabCtrl);

		break;
	}

	/* Header Context Menu */
	case ID_SORT_ASCENDING:
		SetSortReverse(FALSE);
		SetSortColumn(Picker_GetRealColumnFromViewColumn(hwndList, lastColumnClick));
		Picker_Sort(hwndList);
		break;

	case ID_SORT_DESCENDING:
		SetSortReverse(TRUE);
		SetSortColumn(Picker_GetRealColumnFromViewColumn(hwndList, lastColumnClick));
		Picker_Sort(hwndList);
		break;

	case ID_CUSTOMIZE_FIELDS:
		if (DialogBox(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_COLUMNS), hMain, ColumnDialogProc) == TRUE)
			ResetColumnDisplay(FALSE);
		SetFocus(hwndList);
		return TRUE;

	/* View Menu */
	case ID_VIEW_LINEUPICONS:
		if( codeNotify == FALSE)
			ResetListView();
		else
		{
			/*it was sent after a refresh (F5) was done, we only reset the View if "available" is the selected folder
			  as it doesn't affect the others*/
			LPTREEFOLDER folder = GetSelectedFolder();
			if( folder )
			{
				if (folder->m_nFolderId == FOLDER_AVAILABLE )
					ResetListView();

			}
		}
		break;

	//fixme: inconsistent w/ official
	case ID_GAME_PROPERTIES:
		if (!oldControl)
		{
			InitPropertyPage(hInst, hwnd, Picker_GetSelectedItem(hwndList), GetSelectedPickItemIcon(), NULL);
			SaveGameOptions(Picker_GetSelectedItem(hwndList));
		}
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;

	case ID_SOURCE_PROPERTIES:
		if (!oldControl)
		{
			LPTREEFOLDER lpFolder = GetSourceFolder(Picker_GetSelectedItem(hwndList));
			int icon_id = GetTreeViewIconIndex(lpFolder->m_nIconId);
			HICON hIcon = ImageList_GetIcon(NULL, icon_id, ILD_TRANSPARENT);
			const char *name = lpFolder->m_lpTitle;

			if (lpFolder->m_lpOriginalTitle)
				name = lpFolder->m_lpOriginalTitle;

			InitPropertyPage(hInst, hwnd, -2, hIcon, name);
			SaveFolderOptions(name);
		}
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;

	//fixme: inconsistent w/ official
	case ID_FOLDER_PROPERTIES:
		if (!oldControl)
		{
			LPTREEFOLDER lpFolder = GetSelectedFolder();
			const char *name = lpFolder->m_lpTitle;

			if (lpFolder->m_lpOriginalTitle)
				name = lpFolder->m_lpOriginalTitle;

			InitPropertyPage(hInst, hwnd, -2, GetSelectedFolderIcon(), name);
			SaveFolderOptions(name);
		}
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;

	case ID_FOLDER_AUDIT:
		FolderCheck();
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;

	case ID_VIEW_PICTURE_AREA :
		ToggleScreenShot();
		break;

	case ID_UPDATE_GAMELIST:
		UpdateGameList();
		break;

	case ID_OPTIONS_FONT:
		PickFont();
		UpdateHistory();
		return TRUE;

#ifdef DARKMAME
	case ID_OPTIONS_FONT_HISTORY:
		PickHistoryFont();
		return TRUE;
#endif /*DARKMAME*/

#ifdef KAILLERA
	case ID_OPTIONS_FONT_K:
		PickChatFont();
		return TRUE;
#endif /* KAILLERA */

	case ID_OPTIONS_CLONE_COLOR:
		PickCloneColor();
		return TRUE;

	case ID_OPTIONS_DEFAULTS:
		/* Check the return value to see if changes were applied */
		if (!oldControl)
		{
			InitDefaultPropertyPage(hInst, hwnd);
			SaveDefaultOptions();
		}
		SetFocus(hwndList);
		return TRUE;

	case ID_OPTIONS_DIR:
		{
			int  nResult;
			BOOL bUpdateRoms;
			BOOL bUpdateSamples;

			nResult = DialogBox(GetModuleHandle(NULL),
			                    MAKEINTRESOURCE(IDD_DIRECTORIES),
			                    hMain,
			                    DirectoriesDialogProc);

			SaveDefaultOptions();

			bUpdateRoms    = ((nResult & DIRDLG_ROMS)	 == DIRDLG_ROMS)	? TRUE : FALSE;
			bUpdateSamples = ((nResult & DIRDLG_SAMPLES) == DIRDLG_SAMPLES) ? TRUE : FALSE;

			/* update game list */
			if (bUpdateRoms == TRUE || bUpdateSamples == TRUE)
				UpdateGameList();

			SetFocus(hwndList);
		}
		return TRUE;

	case ID_OPTIONS_RESET_DEFAULTS:
		if (DialogBox(GetModuleHandle(NULL),
		              MAKEINTRESOURCE(IDD_RESET), hMain, ResetDialogProc) == TRUE)
		{
			// these may have been changed
			SaveDefaultOptions();
			DestroyWindow(hwnd);
			PostQuitMessage(0);
		}
		return TRUE;

	case ID_OPTIONS_INTERFACE:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INTERFACE_OPTIONS),
		          hMain, InterfaceDialogProc);
		SaveDefaultOptions();

		KillTimer(hMain, SCREENSHOT_TIMER);
		if( GetCycleScreenshot() > 0)
		{
			SetTimer(hMain, SCREENSHOT_TIMER, GetCycleScreenshot()*1000, NULL ); // Scale to seconds
		}

		return TRUE;
		
#ifdef DARKMAME
	case ID_OPTIONS_INTERFACE_EX:
		if( DialogBox(GetModuleHandle(NULL), 
				MAKEINTRESOURCE(IDD_INTERFACE_EX_OPTIONS),
				hMain, InterfaceExDialogProc) == 1 ) 
		{
			// 선택한 폰트의 종류와 크기를 적용
			UpdateFontAndColorInfo();

			LoadBackgroundBitmap();
			InvalidateRect(hMain, NULL, TRUE);
		}
		return TRUE;
#endif /*DARKMAME*/

#ifdef SHOW_UIKEY
	case ID_OPTIONS_VIEW_UIKEY:
		DialogBox(GetModuleHandle(NULL), 
				MAKEINTRESOURCE(IDD_INGAME_UIKEY),
				hMain, InGameUIKeyProc);
		return TRUE;
#endif /*SHOW_UIKEY*/		
	case ID_OPTIONS_BG:
		{
			OPENFILENAMEA OpenFileName;
			static char szFile[MAX_PATH] = "\0";

			OpenFileName.lStructSize       = sizeof(OPENFILENAME);
			OpenFileName.hwndOwner         = hMain;
			OpenFileName.hInstance         = 0;
			OpenFileName.lpstrFilter       = "Image Files (*.png, *.bmp)\0*.PNG;*.BMP\0";
			OpenFileName.lpstrCustomFilter = NULL;
			OpenFileName.nMaxCustFilter    = 0;
			OpenFileName.nFilterIndex      = 1;
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = sizeof(szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = GetBgDir();
			OpenFileName.lpstrTitle        = _UI("Select a Background Image");
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0;
			OpenFileName.lpstrDefExt       = NULL;
			OpenFileName.lCustData         = 0;
			OpenFileName.lpfnHook          = NULL;
			OpenFileName.lpTemplateName    = NULL;                                    
			OpenFileName.Flags             = OFN_NOCHANGEDIR | OFN_SHOWHELP | OFN_EXPLORER;

			if (GetOpenFileNameA(&OpenFileName))
			{
				ResetBackground(szFile);
				LoadBackgroundBitmap();
				InvalidateRect(hMain, NULL, TRUE);
				return TRUE;
			}
		}
		break;

#ifdef UI_COLOR_DISPLAY
	case ID_OPTIONS_PALETTE:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PALETTE),
				  hMain, PaletteDialogProc);
		return TRUE;
#endif /* UI_COLOR_DISPLAY */

#if 0
	case ID_OPTIONS_LANGUAGE:
		DialogBox(GetModuleHandle(NULL),
				  MAKEINTRESOURCE(IDD_LANGUAGE),
				  hMain,
				  LanguageDialogProc);
		return TRUE;
#endif

	case ID_HELP_ABOUT:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT),
				  hMain, AboutDialogProc);
		SetFocus(hwndList);
		return TRUE;

	case IDOK :
		/* cmk -- might need to check more codes here, not sure */
		if (codeNotify != EN_CHANGE && codeNotify != EN_UPDATE)
		{
			/* enter key */
			if (g_in_treeview_edit)
			{
				TreeView_EndEditLabelNow(hTreeView, FALSE);
				return TRUE;
			}
			else 
				if (have_selection)
					MamePlayGame();
		}
		break;

	case IDCANCEL : /* esc key */
		if (g_in_treeview_edit)
			TreeView_EndEditLabelNow(hTreeView, TRUE);
		break;

	case IDC_PLAY_GAME :
		if (have_selection)
			MamePlayGame();
		break;

	case ID_UI_START:
		SetFocus(hwndList);
		MamePlayGame();
		break;

	case ID_UI_UP:
		Picker_SetSelectedPick(hwndList, GetSelectedPick() - 1);
		break;

	case ID_UI_DOWN:
		Picker_SetSelectedPick(hwndList, GetSelectedPick() + 1);
		break;

	case ID_UI_PGUP:
		Picker_SetSelectedPick(hwndList, GetSelectedPick() - ListView_GetCountPerPage(hwndList));
		break;

	case ID_UI_PGDOWN:
		if ( (GetSelectedPick() + ListView_GetCountPerPage(hwndList)) < ListView_GetItemCount(hwndList) )
			Picker_SetSelectedPick(hwndList,  GetSelectedPick() + ListView_GetCountPerPage(hwndList) );
		else
			Picker_SetSelectedPick(hwndList,  ListView_GetItemCount(hwndList)-1 );
		break;

	case ID_UI_HOME:
		Picker_SetSelectedPick(hwndList, 0);
		break;

	case ID_UI_END:
		Picker_SetSelectedPick(hwndList,  ListView_GetItemCount(hwndList)-1 );
		break;
	case ID_UI_LEFT:
		/* hmmmmm..... */
		SendMessage(hwndList,WM_HSCROLL, SB_LINELEFT, 0);
		break;

	case ID_UI_RIGHT:
		/* hmmmmm..... */
		SendMessage(hwndList,WM_HSCROLL, SB_LINERIGHT, 0);
		break;
	case ID_UI_HISTORY_UP:
		/* hmmmmm..... */
		{
			HWND hHistory = GetDlgItem(hMain, IDC_HISTORY);
			SendMessage(hHistory, EM_SCROLL, SB_PAGEUP, 0);
		}
		break;

	case ID_UI_HISTORY_DOWN:
		/* hmmmmm..... */
		{
			HWND hHistory = GetDlgItem(hMain, IDC_HISTORY);
			SendMessage(hHistory, EM_SCROLL, SB_PAGEDOWN, 0);
		}
		break;

	case IDC_SSFRAME:
		TabView_CalculateNextTab(hTabCtrl);
		UpdateScreenShot();
		TabView_UpdateSelection(hTabCtrl);
		break;

	case ID_CONTEXT_SELECT_RANDOM:
		SetRandomPickItem();
		break;

	case ID_CONTEXT_RESET_PLAYTIME:
		ResetPlayTime( Picker_GetSelectedItem(hwndList) );
		ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
		break;

	case ID_CONTEXT_RESET_PLAYCOUNT:
		ResetPlayCount( Picker_GetSelectedItem(hwndList) );
		ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
		break;

	case ID_CONTEXT_RENAME_CUSTOM :
		TreeView_EditLabel(hTreeView,TreeView_GetSelection(hTreeView));
		break;

	default:
		if (id >= ID_CONTEXT_SHOW_FOLDER_START && id < ID_CONTEXT_SHOW_FOLDER_END)
		{
			ToggleShowFolder(id - ID_CONTEXT_SHOW_FOLDER_START);
			break;
		}
		for (i = 0; g_helpInfo[i].nMenuItem > 0; i++)
		{
			if (g_helpInfo[i].nMenuItem == id)
			{
				if (g_helpInfo[i].bIsHtmlHelp)
					HelpFunction(hMain, g_helpInfo[i].lpFile, HH_DISPLAY_TOPIC, 0);
				else
					DisplayTextFile(hMain, g_helpInfo[i].lpFile);
				return FALSE;
			}
		}
		break;
	}

	return FALSE;
}

static void LoadBackgroundBitmap(void)
{
	HGLOBAL hDIBbg;
	char*	pFileName = 0;

	if (hBackground)
	{
		DeleteObject(hBackground);
		hBackground = 0;
	}

	if (hPALbg)
	{
		DeleteObject(hPALbg);
		hPALbg = 0;
	}


#ifdef DARKMAME	// HISTORY_BG
	if (hFolderBackground)	DeleteObject(hFolderBackground);
	if (hFolderPALbg)		DeleteObject(hFolderPALbg);
	if (hSnapBackground)	DeleteObject(hSnapBackground);
	if (hSnapPALbg)			DeleteObject(hSnapPALbg);
	
	hFolderBackground = 0;
	hFolderPALbg = 0;
	hSnapBackground = 0;
	hSnapPALbg = 0;
#endif /*DARKMAME*/

	/* Pick images based on number of colors avaliable. */
	if (GetDepth(hwndList) <= 8)
	{
		pFileName = (char *)"bkgnd16";
		/*nResource = IDB_BKGROUND16;*/
	}
	else
	{
		pFileName = (char *)"bkground";
		/*nResource = IDB_BKGROUND;*/
	}

	if (LoadDIB(pFileName, &hDIBbg, &hPALbg, BACKGROUND))
	{
		HDC hDC = GetDC(hwndList);
		hBackground = DIBToDDB(hDC, hDIBbg, &bmDesc);
		GlobalFree(hDIBbg);
		ReleaseDC(hwndList, hDC);
	}

#ifdef DARKMAME	// HISTORY_BG
	pFileName = (char *)"bg_folder";
	if (LoadDIB(pFileName, &hDIBbg, &hFolderPALbg, BACKGROUND))
	{
		HDC hDC = GetDC(hwndList);
		hFolderBackground = DIBToDDB(hDC, hDIBbg, &bmFolderDesc);
		GlobalFree(hDIBbg);
		ReleaseDC(hwndList, hDC);
	}

	pFileName = (char *)"bg_snap";
	if (LoadDIB(pFileName, &hDIBbg, &hSnapPALbg, BACKGROUND))
	{
		HDC hDC = GetDC(hwndList);
		hSnapBackground = DIBToDDB(hDC, hDIBbg, &bmSnapDesc);
		GlobalFree(hDIBbg);
		ReleaseDC(hwndList, hDC);
	}
#endif /*DARKMAME*/	
}

static void ResetColumnDisplay(BOOL first_time)
{
	int driver_index;

	if (!first_time)
		Picker_ResetColumnDisplay(hwndList);

	ResetListView();

	driver_index = GetGameNameIndex(GetDefaultGame());
	Picker_SetSelectedItem(hwndList, driver_index);
}

int GamePicker_GetItemImage(HWND hwndPicker, int nItem)
{
	return GetIconForDriver(nItem);
}

int GamePicker_GetUseBrokenColor(void)
{
	return !GetUseBrokenIcon();
}

const char *GamePicker_GetItemString(HWND hwndPicker, int nItem, int nColumn)
{
	const char *s = "";

	switch(nColumn)
	{
		case COLUMN_GAMES:
			/* Driver description */
			s = UseLangList() ?
				_LST(drivers[nItem]->description):
				ModifyThe(drivers[nItem]->description);
			break;

		case COLUMN_ROMS:
			/* Has Roms */
			s = GetAuditString(GetRomAuditResults(nItem));
			break;

		case COLUMN_SAMPLES:
			/* Samples */
			if (DriverUsesSamples(nItem))
				s = GetAuditString(GetSampleAuditResults(nItem));
			break;

		case COLUMN_DIRECTORY:
			/* Driver name (directory) */
			s = drivers[nItem]->name;
			break;

		case COLUMN_SRCDRIVERS:
			/* Source drivers */
			s = GetDriverFilename(nItem);
			break;

		case COLUMN_PLAYTIME:
			/* total play time */
			{
				static char buf[100];

				GetTextPlayTime(nItem, buf);
				s = buf;
			}
			break;

		case COLUMN_TYPE:
		{
			struct InternalMachineDriver drv;
			expand_machine_driver(drivers[nItem]->drv,&drv);

			/* Vector/Raster */
			if (drv.video_attributes & VIDEO_TYPE_VECTOR)
				s = _UI("Vector");
			else
				s = _UI("Raster");
			break;
		}
		case COLUMN_TRACKBALL:
			/* Trackball */
			if (DriverUsesTrackball(nItem))
				s = _UI("Yes");
			else
				s = _UI("No");
			break;

		case COLUMN_PLAYED:
			/* times played */
			{
				static char buf[100];

				sprintf(buf, "%i", GetPlayCount(nItem));
				s = buf;
			}
			break;

		case COLUMN_MANUFACTURER:
			/* Manufacturer */
			if (UseLangList())
				s = _MANUFACT(drivers[nItem]->manufacturer);
			else
				s = (char *)drivers[nItem]->manufacturer;
			break;

		case COLUMN_YEAR:
			/* Year */
			s = drivers[nItem]->year;
			break;

		case COLUMN_CLONE:
			s = GetCloneParentName(nItem);
			break;
	}
	return s;
}

static void GamePicker_LeavingItem(HWND hwndPicker, int nItem)
{
	// leaving item
	// printf("leaving %s\n",drivers[nItem]->name);
#ifdef MESS
	MessWriteMountedSoftware(nItem);
#endif	
}

static void GamePicker_EnteringItem(HWND hwndPicker, int nItem)
{
	// printf("entering %s\n",drivers[nItem]->name);
	if (g_bDoBroadcast == TRUE)
	{
		ATOM a = GlobalAddAtom(_Unicode(drivers[nItem]->description));
		SendMessage(HWND_BROADCAST, g_mame32_message, a, a);
		GlobalDeleteAtom(a);
	}

	EnableSelection(nItem);
#ifdef MESS
	MessReadMountedSoftware(nItem);
#endif
}

static int GamePicker_FindItemParent(HWND hwndPicker, int nItem)
{
	return parent_index[nItem];
}

static int GamePicker_CheckItemBroken(HWND hwndPicker, int nItem)
{
	return DriverIsBroken(nItem);
}

/* Initialize the Picker and List controls */
static void InitListView(void)
{
	LVBKIMAGEA bki;
	char path[MAX_PATH];

	static const struct PickerCallbacks s_gameListCallbacks =
	{
		SetSortColumn,			/* pfnSetSortColumn */
		GetSortColumn,			/* pfnGetSortColumn */
		SetSortReverse,			/* pfnSetSortReverse */
		GetSortReverse,			/* pfnGetSortReverse */
		SetViewMode,			/* pfnSetViewMode */
		GetViewMode,			/* pfnGetViewMode */
		SetColumnWidths,		/* pfnSetColumnWidths */
		GetColumnWidths,		/* pfnGetColumnWidths */
		SetColumnOrder,			/* pfnSetColumnOrder */
		GetColumnOrder,			/* pfnGetColumnOrder */
		SetColumnShown,			/* pfnSetColumnShown */
		GetColumnShown,			/* pfnGetColumnShown */
		GetOffsetClones,		/* pfnGetOffsetChildren */
		GamePicker_GetUseBrokenColor,	/* pfnGetUseBrokenColor */

		GamePicker_Compare,		/* pfnCompare */
		MamePlayGame,			/* pfnDoubleClick */
		GamePicker_GetItemString,	/* pfnGetItemString */
		GamePicker_GetItemImage,	/* pfnGetItemImage */
		GamePicker_LeavingItem,		/* pfnLeavingItem */
		GamePicker_EnteringItem,	/* pfnEnteringItem */
		BeginListViewDrag,		/* pfnBeginListViewDrag */
		GamePicker_FindItemParent,	/* pfnFindItemParent */
		GamePicker_CheckItemBroken,	/* pfnCheckItemBroken */
		OnIdle,				/* pfnIdle */
		GamePicker_OnHeaderContextMenu,	/* pfnOnHeaderContextMenu */
		GamePicker_OnBodyContextMenu	/* pfnOnBodyContextMenu */
	};

	struct PickerOptions opts;

	// subclass the list view
	memset(&opts, 0, sizeof(opts));
	opts.pCallbacks = &s_gameListCallbacks;
	opts.nColumnCount = COLUMN_MAX;
	opts.ppszColumnNames = column_names;
	SetupPicker(hwndList, &opts);

	ListView_SetTextBkColor(hwndList, CLR_NONE);
	ListView_SetBkColor(hwndList, CLR_NONE);
	sprintf(path, "%s\\bkground.png", GetBgDir() );
	bki.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_TILE;
	bki.pszImage = path;
	if( hBackground )	
		ListView_SetBkImageA(hwndList, &bki);

	CreateIcons();

	ResetColumnDisplay(TRUE);

	// Allow selection to change the default saved game
	bListReady = TRUE;
}

static void AddDriverIcon(int nItem,int default_icon_index)
{
	HICON hIcon = 0;

	/* if already set to rom or clone icon, we've been here before */
	if (icon_index[nItem] == 1 || icon_index[nItem] == 3)
		return;

	hIcon = LoadIconFromFile((char *)drivers[nItem]->name);
	if (hIcon == NULL && drivers[nItem]->clone_of != NULL)
	{
		hIcon = LoadIconFromFile((char *)drivers[nItem]->clone_of->name);
		if (hIcon == NULL && drivers[nItem]->clone_of->clone_of != NULL)
			hIcon = LoadIconFromFile((char *)drivers[nItem]->clone_of->clone_of->name);
	}

	if (hIcon != NULL)
	{
		int nIconPos = ImageList_AddIcon(hSmall, hIcon);
		ImageList_AddIcon(hLarge, hIcon);
		if (nIconPos != -1)
			icon_index[nItem] = nIconPos;
	}
	if (icon_index[nItem] == 0)
		icon_index[nItem] = default_icon_index;
}

static void DestroyIcons(void)
{
	if (hSmall != NULL)
	{
		//FIXME: ImageList_Destroy(hSmall);
		hSmall = NULL;
	}

	if (icon_index != NULL)
	{
		int i;
		for (i=0;i<game_count;i++)
			icon_index[i] = 0; // these are indices into hSmall
	}

	if (hLarge != NULL)
	{
		//FIXME: ImageList_Destroy(hLarge);
		hLarge = NULL;
	}

	if (hHeaderImages != NULL)
	{
		//FIXME: ImageList_Destroy(hHeaderImages);
		hHeaderImages = NULL;
	}

}

static void ReloadIcons(void)
{
	HICON hIcon;
	INT i;

	// clear out all the images
	ImageList_Remove(hSmall,-1);
	ImageList_Remove(hLarge,-1);

	if (icon_index != NULL)
	{
		for (i=0;i<game_count;i++)
			icon_index[i] = 0; // these are indices into hSmall
	}

	for (i = 0; g_iconData[i].icon_name; i++)
	{
		hIcon = LoadIconFromFile((char *) g_iconData[i].icon_name);
		if (hIcon == NULL)
			hIcon = LoadIcon(hInst, MAKEINTRESOURCE(g_iconData[i].resource));

		ImageList_AddIcon(hSmall, hIcon);
		ImageList_AddIcon(hLarge, hIcon);
	}
}

static DWORD GetShellLargeIconSize(void)
{
	DWORD  dwSize, dwLength = 512, dwType = REG_SZ;
	char   szBuffer[512];
	HKEY   hKey;

	/* Get the Key */
	RegOpenKeyA(HKEY_CURRENT_USER, "Control Panel\\desktop\\WindowMetrics", &hKey);
	/* Save the last size */
	RegQueryValueExA(hKey, "Shell Icon Size", NULL, &dwType, (LPBYTE)szBuffer, &dwLength);
	dwSize = atol(szBuffer);
	if (dwSize < 32)
		dwSize = 32;

	if (dwSize > 48)
		dwSize = 48;

	/* Clean up */
	RegCloseKey(hKey);
	return dwSize;
}

static DWORD GetShellSmallIconSize(void)
{
	DWORD dwSize = ICONMAP_WIDTH;

	if (dwSize < 48)
	{
		if (dwSize < 32)
			dwSize = 16;
		else
			dwSize = 32;
	}
	else
	{
		dwSize = 48;
	}
	return dwSize;
}

// create iconlist for Listview control
static void CreateIcons(void)
{
	DWORD dwLargeIconSize = GetShellLargeIconSize();
	HICON hIcon;
	int icon_count;
	DWORD dwStyle;

	icon_count = 0;
	while(g_iconData[icon_count].icon_name)
		icon_count++;

	// the current window style affects the sizing of the rows when changing
	// between list views, so put it in small icon mode temporarily while we associate
	// our image list
	//
	// using large icon mode instead kills the horizontal scrollbar when doing
	// full refresh, which seems odd (it should recreate the scrollbar when
	// set back to report mode, for example, but it doesn't).

	dwStyle = GetWindowLong(hwndList,GWL_STYLE);
	SetWindowLong(hwndList,GWL_STYLE,(dwStyle & ~LVS_TYPEMASK) | LVS_ICON);

	hSmall = ImageList_Create(GetShellSmallIconSize(),GetShellSmallIconSize(),
	                          ILC_COLORDDB | ILC_MASK, icon_count, 500);
	hLarge = ImageList_Create(dwLargeIconSize, dwLargeIconSize,
	                          ILC_COLORDDB | ILC_MASK, icon_count, 500);

	ReloadIcons();

	// Associate the image lists with the list view control.
	ListView_SetImageList(hwndList, hSmall, LVSIL_SMALL);
	ListView_SetImageList(hwndList, hLarge, LVSIL_NORMAL);

	// restore our view
	SetWindowLong(hwndList,GWL_STYLE,dwStyle);

	// Now set up header specific stuff
	hHeaderImages = ImageList_Create(16,16,ILC_COLORDDB | ILC_MASK,2,2);
	hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_HEADER_UP));
	ImageList_AddIcon(hHeaderImages,hIcon);
	hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_HEADER_DOWN));
	ImageList_AddIcon(hHeaderImages,hIcon);

	Picker_SetHeaderImageList(hwndList, hHeaderImages);
}



static int GamePicker_Compare(HWND hwndPicker, int index1, int index2, int sort_subitem)
{
	int value;
	const char *name1 = NULL;
	const char *name2 = NULL;
	int nTemp1, nTemp2;

	struct InternalMachineDriver drv1;
	struct InternalMachineDriver drv2;
	expand_machine_driver(drivers[index1]->drv, &drv1);
	expand_machine_driver(drivers[index2]->drv, &drv2);

#ifdef DEBUG
	if (strcmp(drivers[index1]->name,"1941") == 0 && strcmp(drivers[index2]->name,"1942") == 0)
	{
		dprintf("comparing 1941, 1942");
	}

	if (strcmp(drivers[index1]->name,"1942") == 0 && strcmp(drivers[index2]->name,"1941") == 0)
	{
		dprintf("comparing 1942, 1941");
	}
#endif

	switch (sort_subitem)
	{
	case COLUMN_GAMES:
		if (UseLangList())
			value = stricmp(game_readings[index1], game_readings[index2]);
		else
			value = stricmp(ModifyThe(drivers[index1]->description),
			                ModifyThe(drivers[index2]->description));
		break;

	case COLUMN_ROMS:
		nTemp1 = GetRomAuditResults(index1);
		nTemp2 = GetRomAuditResults(index2);

		if (IsAuditResultKnown(nTemp1) == FALSE && IsAuditResultKnown(nTemp2) == FALSE)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		if (IsAuditResultKnown(nTemp1) == FALSE)
		{
			value = 1;
			break;
		}

		if (IsAuditResultKnown(nTemp2) == FALSE)
		{
			value = -1;
			break;
		}

		// ok, both are known

		if (IsAuditResultYes(nTemp1) && IsAuditResultYes(nTemp2))
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);
		
		if (IsAuditResultNo(nTemp1) && IsAuditResultNo(nTemp2))
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		if (IsAuditResultYes(nTemp1) && IsAuditResultNo(nTemp2))
			value = -1;
		else
			value = 1;
		break;

	case COLUMN_SAMPLES:
		nTemp1 = -1;
		if (DriverUsesSamples(index1))
		{
			int audit_result = GetSampleAuditResults(index1);
			if (IsAuditResultKnown(audit_result))
			{
				if (IsAuditResultYes(audit_result))
					nTemp1 = 1;
				else 
					nTemp1 = 0;
			}
			else
				nTemp1 = 2;
		}

		nTemp2 = -1;
		if (DriverUsesSamples(index2))
		{
			int audit_result = GetSampleAuditResults(index2);
			if (IsAuditResultKnown(audit_result))
			{
				if (IsAuditResultYes(audit_result))
					nTemp2 = 1;
				else 
					nTemp2 = 0;
			}
			else
				nTemp2 = 2;
		}

		if (nTemp1 == nTemp2)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = nTemp2 - nTemp1;
		break;

	case COLUMN_DIRECTORY:
		value = stricmp(drivers[index1]->name, drivers[index2]->name);
		break;

	case COLUMN_SRCDRIVERS:
		if (stricmp(GetDriverFilename(index1), GetDriverFilename(index2)) == 0)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = stricmp(GetDriverFilename(index1), GetDriverFilename(index2));
		break;

	case COLUMN_PLAYTIME:
	   value = GetPlayTime(index1) - GetPlayTime(index2);
	   if (value == 0)
		  return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

	   break;

	case COLUMN_TYPE:
	{
		if ((drv1.video_attributes & VIDEO_TYPE_VECTOR) ==
			(drv2.video_attributes & VIDEO_TYPE_VECTOR))
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = (drv1.video_attributes & VIDEO_TYPE_VECTOR) -
				(drv2.video_attributes & VIDEO_TYPE_VECTOR);
		break;
	}
	case COLUMN_TRACKBALL:
		if (DriverUsesTrackball(index1) == DriverUsesTrackball(index2))
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = DriverUsesTrackball(index1) - DriverUsesTrackball(index2);
		break;

	case COLUMN_PLAYED:
		value = GetPlayCount(index1) - GetPlayCount(index2);
		if (value == 0)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		break;

	case COLUMN_MANUFACTURER:
		if (stricmp(drivers[index1]->manufacturer, drivers[index2]->manufacturer) == 0)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = stricmp(drivers[index1]->manufacturer, drivers[index2]->manufacturer);
		break;

	case COLUMN_YEAR:
		if (stricmp(drivers[index1]->year, drivers[index2]->year) == 0)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = stricmp(drivers[index1]->year, drivers[index2]->year);
		break;

	case COLUMN_CLONE:
		name1 = GetCloneParentName(index1);
		name2 = GetCloneParentName(index2);

		if (*name1 == '\0')
			name1 = NULL;
		if (*name2 == '\0')
			name2 = NULL;

		if (name1 == name2)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		if (name2 == NULL)
			value = -1;
		else if (name1 == NULL)
			value = 1;
		else
			value = stricmp(name1, name2);
		break;

	default :
		return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);
	}

#ifdef DEBUG
	if ((strcmp(drivers[index1]->name,"1941") == 0 && strcmp(drivers[index2]->name,"1942") == 0) ||
		(strcmp(drivers[index1]->name,"1942") == 0 && strcmp(drivers[index2]->name,"1941") == 0))
		dprintf("result: %i",value);
#endif

	return value;
}

static int GetSelectedPick()
{
	/* returns index of listview selected item */
	/* This will return -1 if not found */
	return ListView_GetNextItem(hwndList, -1, LVIS_SELECTED | LVIS_FOCUSED);
}

static HICON GetSelectedPickItemIcon()
{
	LV_ITEM lvi;

	lvi.iItem = GetSelectedPick();
	lvi.iSubItem = 0;
	lvi.mask     = LVIF_IMAGE;
	ListView_GetItem(hwndList, &lvi);

	return ImageList_GetIcon(hLarge, lvi.iImage, ILD_TRANSPARENT);
}

static void SetRandomPickItem(void)
{
	int nListCount;

	nListCount = ListView_GetItemCount(hwndList);

	if (nListCount > 0)
	{
		Picker_SetSelectedPick(hwndList, rand() % nListCount);
	}
}

enum
{
	FILETYPE_INPUT_FILES = 1,
	FILETYPE_SAVESTATE_FILES = 2,
	FILETYPE_WAVE_FILES = 3,
#ifdef MAME_AVI
	FILETYPE_AVI_FILES = 4,
#endif	
};
static BOOL CommonFileDialogW(common_file_dialog_procW cfd, char *filename, int filetype)
{
	BOOL success;

	OPENFILENAMEW of;
	WCHAR fn[MAX_PATH];
	WCHAR *p, buf[256];
	const char *s = NULL;

	lstrcpy(fn, _Unicode(filename));

	of.lStructSize       = sizeof(of);
	of.hwndOwner         = hMain;
	of.hInstance         = NULL;

	switch (filetype)
	{
	case FILETYPE_INPUT_FILES :
		s   = MAMENAME " input files (*.inp,*.zip)\0*.inp;*.zip\0All files (*.*)\0*.*\0";
		break;
	case FILETYPE_SAVESTATE_FILES :
		s   = MAMENAME " savestate files (*.sta)\0*.sta;\0All files (*.*)\0*.*\0";
		break;
	case FILETYPE_WAVE_FILES :
		s   = "Sounds (*.wav)\0*.wav;\0All files (*.*)\0*.*\0";
		break;
#ifdef MAME_AVI
	case FILETYPE_AVI_FILES :
		s   = "AVI files (*.avi)\0*.avi;\0All files (*.*)\0*.*\0";
		break;
#endif
	}

	for (p = buf; *s; s += strlen(s) + 1)
	{
		lstrcpy(p, _Unicode(_UI(s)));
		p += lstrlen(p) + 1;
	}
	*p = '\0';
	of.lpstrFilter   = buf;

	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter    = 0;
	of.nFilterIndex      = 1;
	of.lpstrFile         = fn;
	of.nMaxFile          = MAX_PATH;
	of.lpstrFileTitle    = NULL;
	of.nMaxFileTitle     = 0;
	if (filetype == FILETYPE_SAVESTATE_FILES)
		of.lpstrInitialDir = _Unicode(GetStateDir());
#ifdef MAME_AVI
	else if (filetype == FILETYPE_AVI_FILES)
		of.lpstrInitialDir = _Unicode(GetAviDir());
#endif
	else
		of.lpstrInitialDir = _Unicode(last_directory);
	of.lpstrTitle        = NULL;
	of.Flags             = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	of.nFileOffset       = 0;
	of.nFileExtension    = 0;
	switch (filetype)
	{
	case FILETYPE_INPUT_FILES :
		of.lpstrDefExt       = TEXT("inp");
		break;
	case FILETYPE_SAVESTATE_FILES :
		of.lpstrDefExt       = TEXT("sta");
		break;
	case FILETYPE_WAVE_FILES :
		of.lpstrDefExt       = TEXT("wav");
		break;
#ifdef MAME_AVI
	case FILETYPE_AVI_FILES :
		of.lpstrDefExt       = TEXT("avi");
		break;
#endif	
	}
	of.lpstrDefExt       = NULL;
	of.lCustData         = 0;
	of.lpfnHook          = NULL;
	of.lpTemplateName    = NULL;

	success = cfd(&of);
	if (success)
	{
		strcpy(filename, _String(fn));
		//dprintf("got filename %s nFileExtension %u\n",filename,_String(of.nFileExtension));
		/*GetDirectory(filename,last_directory,sizeof(last_directory));*/
	}

	return success;
}

static BOOL CommonFileDialogA(common_file_dialog_procA cfd, char *filename, int filetype)
{
	BOOL success;

	OPENFILENAMEA of;
	char fn[MAX_PATH];
	char *p, buf[256];
	const char *s = NULL;

	strcpy(fn, filename);

	of.lStructSize       = sizeof(of);
	of.hwndOwner         = hMain;
	of.hInstance         = NULL;

	switch (filetype)
	{
	case FILETYPE_INPUT_FILES :
		s   = MAMENAME " input files (*.inp,*.zip)\0*.inp;*.zip\0All files (*.*)\0*.*\0";
		break;
	case FILETYPE_SAVESTATE_FILES :
		s   = MAMENAME " savestate files (*.sta)\0*.sta;\0All files (*.*)\0*.*\0";
		break;
	case FILETYPE_WAVE_FILES :
		s   = "Sounds (*.wav)\0*.wav;\0All files (*.*)\0*.*\0";
		break;
#ifdef MAME_AVI
	case FILETYPE_AVI_FILES :
		s   = "AVI files (*.avi)\0*.avi;\0All files (*.*)\0*.*\0";
		break;
#endif
	}

	for (p = buf; *s; s += strlen(s) + 1)
	{
		strcpy(p, _UI(s));
		p += strlen(p) + 1;
	}
	*p = '\0';
	of.lpstrFilter   = buf;

	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter    = 0;
	of.nFilterIndex      = 1;
	of.lpstrFile         = fn;
	of.nMaxFile          = MAX_PATH;
	of.lpstrFileTitle    = NULL;
	of.nMaxFileTitle     = 0;
	if (filetype == FILETYPE_SAVESTATE_FILES)
		of.lpstrInitialDir = GetStateDir();
#ifdef MAME_AVI
	else if (filetype == FILETYPE_AVI_FILES)
		of.lpstrInitialDir = GetAviDir();
#endif
	else
		of.lpstrInitialDir = last_directory;
	of.lpstrTitle        = NULL;
	of.Flags             = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	of.nFileOffset       = 0;
	of.nFileExtension    = 0;
	switch (filetype)
	{
	case FILETYPE_INPUT_FILES :
		of.lpstrDefExt       = "inp";
		break;
	case FILETYPE_SAVESTATE_FILES :
		of.lpstrDefExt       = "sta";
		break;
	case FILETYPE_WAVE_FILES :
		of.lpstrDefExt       = "wav";
		break;
#ifdef MAME_AVI
	case FILETYPE_AVI_FILES :
		of.lpstrDefExt       = "avi";
		break;
#endif
	}
	of.lpstrDefExt       = NULL;
	of.lCustData         = 0;
	of.lpfnHook          = NULL;
	of.lpTemplateName    = NULL;

	success = cfd(&of);
	if (success)
	{
		strcpy(filename, fn);
		//dprintf("got filename %s nFileExtension %u\n",filename,of.nFileExtension);
		/*GetDirectory(filename,last_directory,sizeof(last_directory));*/
	}

	return success;
}

static BOOL CommonFileDialog(BOOL open_for_write, char *filename, int filetype)
{
	if (OnNT())
		return CommonFileDialogW(open_for_write ? GetSaveFileNameW : GetOpenFileNameW, filename, filetype);
	else
		return CommonFileDialogA(open_for_write ? GetSaveFileNameA : GetOpenFileNameA, filename, filetype);
}


#if 0
static BOOL SelectLanguageFile(HWND hWnd, TCHAR* filename)
{
	OPENFILENAME of;

	of.lStructSize       = sizeof(of);
	of.hwndOwner         = hWnd;
	of.hInstance         = NULL;
	of.lpstrFilter       = MAMENAME " Language files (*.lng)\0*.lng\0";
	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter    = 0;
	of.nFilterIndex      = 1;
	of.lpstrFile         = filename;
	of.nMaxFile          = MAX_PATH;
	of.lpstrFileTitle    = NULL;
	of.nMaxFileTitle     = 0;
	of.lpstrInitialDir   = NULL;
	of.lpstrTitle        = NULL;
	of.Flags             = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	of.nFileOffset       = 0;
	of.nFileExtension    = 0;
	of.lpstrDefExt       = "lng";
	of.lCustData         = 0;
	of.lpfnHook          = NULL;
	of.lpTemplateName    = NULL;

	return GetOpenFileName(&of);
}

static INT_PTR CALLBACK LanguageDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR pLangFile[MAX_PATH];
	DWORD dwHelpIDs[] = { IDC_LANGUAGECHECK, HIDC_LANGUAGECHECK,
	                      IDC_LANGUAGEEDIT,  HIDC_LANGUAGEEDIT,
	                      0, 0};

	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, FALSE);

		{
			const char* pLang = GetLanguage();
			if (pLang == NULL || *pLang == '\0')
			{
				Edit_SetText(GetDlgItem(hDlg, IDC_LANGUAGEEDIT), "");
				Button_SetCheck(GetDlgItem(hDlg, IDC_LANGUAGECHECK), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGEEDIT),   FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGEBROWSE), FALSE);
			}
			else
			{
				Edit_SetText(GetDlgItem(hDlg, IDC_LANGUAGEEDIT), pLang);
				Button_SetCheck(GetDlgItem(hDlg, IDC_LANGUAGECHECK), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGEEDIT),   TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGEBROWSE), TRUE);
			}

			return TRUE;
		}

	case WM_HELP:
		HelpFunction(((LPHELPINFO)lParam)->hItemHandle, MAME32CONTEXTHELP, HH_TP_HELP_WM_HELP, (DWORD)dwHelpIDs);
		break;

	case WM_CONTEXTMENU:
		HelpFunction((HWND)wParam, MAME32CONTEXTHELP, HH_TP_HELP_CONTEXTMENU, (DWORD)dwHelpIDs);
		break;

	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam, lParam))
		{
		case IDOK:
			Edit_GetText(GetDlgItem(hDlg, IDC_LANGUAGEEDIT), pLangFile, MAX_PATH);
			SetLanguage(pLangFile);

		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;

		case IDC_LANGUAGECHECK:
		{
			BOOL bCheck = Button_GetCheck(GetDlgItem(hDlg, IDC_LANGUAGECHECK));
			EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGEEDIT),   bCheck);
			EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGEBROWSE), bCheck);
			if (bCheck == FALSE)
				Edit_SetText(GetDlgItem(hDlg, IDC_LANGUAGEEDIT), "");
			return TRUE;
		}

		case IDC_LANGUAGEBROWSE:
			Edit_GetText(GetDlgItem(hDlg, IDC_LANGUAGEEDIT), pLangFile, MAX_PATH);
			if (SelectLanguageFile(hDlg, pLangFile) == TRUE)
				Edit_SetText(GetDlgItem(hDlg, IDC_LANGUAGEEDIT), pLangFile);
			break;
		}
		break;
	}
	return 0;
}
#endif

void SetStatusBarText(int part_index, const char *message)
{
	StatusBarSetTextA(hStatusBar, part_index, message);
}

void SetStatusBarTextW(int part_index, LPWSTR message)
{
	StatusBarSetTextW(hStatusBar, part_index, message);
}

void SetStatusBarTextF(int part_index, const char *fmt, ...)
{
	char buf[256];
	va_list va;

	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);

	SetStatusBarText(part_index, buf);
}

static void MameMessageBox(const char *fmt, ...)
{
	char buf[2048];
	va_list va;

	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	MessageBox(GetMainWindow(), _Unicode(buf), TEXT_MAME32NAME, MB_OK | MB_ICONERROR);
	va_end(va);
}

static void MamePlayBackGame(void)
{
	int nGame;
	char filename[MAX_PATH];

	*filename = 0;

	nGame = Picker_GetSelectedItem(hwndList);
	if (nGame != -1)
		strcpy(filename, drivers[nGame]->name);

	if (CommonFileDialog(FALSE, filename, FILETYPE_INPUT_FILES))
	{
		mame_file* pPlayBack;
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char bare_fname[_MAX_FNAME];
		char ext[_MAX_EXT];

		char path[MAX_PATH];
		char fname[MAX_PATH];

		_splitpath(filename, drive, dir, bare_fname, ext);

		sprintf(path,"%s%s",drive,dir);
		sprintf(fname,"%s",bare_fname); // don't add extension for zip support.
		if (path[strlen(path)-1] == '\\')
			path[strlen(path)-1] = 0; // take off trailing back slash

		SetCorePathList(FILETYPE_INPUTLOG, path);
#ifdef KAILLERA
		SetCorePathList(FILETYPE_INPUTLOG2,path);
#endif /* KAILLERA */

		pPlayBack = mame_fopen(fname,NULL,FILETYPE_INPUTLOG,0);
		if (pPlayBack == NULL)
		{
			MameMessageBox(_UI("Could not open '%s' as a valid input file."), filename);
			return;
		}

		// check for game name embedded in .inp header
		if (pPlayBack)
		{
			INP_HEADER inp_header;

			// read playback header
			mame_fread(pPlayBack, &inp_header, sizeof(INP_HEADER));

			if (!isalnum(inp_header.name[0])) // If first byte is not alpha-numeric
				mame_fseek(pPlayBack, 0, SEEK_SET); // old .inp file - no header
			else
			{
				int i;
				for (i = 0; drivers[i] != 0; i++) // find game and play it
				{
					if (strcmp(drivers[i]->name, inp_header.name) == 0)
					{
						nGame = i;
						break;
					}
				}
			}
		}
		mame_fclose(pPlayBack);

		g_pPlayBkName = fname;
		override_playback_directory = path;
		MamePlayGameWithOptions(nGame);
		g_pPlayBkName = NULL;
		override_playback_directory = NULL;
	}
}

static void MameLoadState(void)
{
	int nGame;
	char filename[MAX_PATH];
	char selected_filename[MAX_PATH];

	*filename = 0;

	nGame = Picker_GetSelectedItem(hwndList);
	if (nGame != -1)
	{
		strcpy(filename, drivers[nGame]->name);
		strcpy(selected_filename, drivers[nGame]->name);
	}
	if (CommonFileDialog(FALSE, filename, FILETYPE_SAVESTATE_FILES))
	{
		mame_file* pSaveState;
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char ext[_MAX_EXT];

		char path[MAX_PATH];
		char fname[MAX_PATH];
		char bare_fname[_MAX_FNAME];
		char *state_fname;
		int rc;

		_splitpath(filename, drive, dir, bare_fname, ext);

		// parse path
		sprintf(path,"%s%s",drive,dir);
		sprintf(fname,"%s%s",bare_fname,ext);
		if (path[strlen(path)-1] == '\\')
			path[strlen(path)-1] = 0; // take off trailing back slash

#ifdef MESS
		{
			state_fname = filename;
			return;
		}
#else // !MESS
		{
			char *cPos=0;
			int  iPos=0;
			char romname[MAX_PATH];

			cPos = strchr(bare_fname, '-' );
			iPos = cPos ? cPos - bare_fname : strlen(bare_fname);
			strncpy(romname, bare_fname, iPos );
			romname[iPos] = '\0';
			if( strcmp(selected_filename,romname) != 0 )
			{
				MameMessageBox(_UI("'%s' is not a valid savestate file for game '%s'."), filename, selected_filename);
				return;
			}
			SetCorePathList(FILETYPE_STATE, path);
			state_fname = fname;
		}
#endif // MESS

		pSaveState = mame_fopen(NULL,state_fname,FILETYPE_STATE,0);
		if (pSaveState == NULL)
		{
			MameMessageBox(_UI("Could not open '%s' as a valid savestate file."), filename);
			return;
		}

		// call the MAME core function to check the save state file
		rc = state_save_check_file(pSaveState, selected_filename, MameMessageBox);
		mame_fclose(pSaveState);
		if (rc)
			return;

#ifdef MESS
		g_pSaveStateName = state_fname;
#else
		{
			char *cPos;
			cPos = strrchr(bare_fname, '-' );
			cPos = cPos+1;
			if( strlen(cPos) >0)
			{
				g_pSaveStateName = cPos;
				override_savestate_directory = path;
			}
		}
#endif

		MamePlayGameWithOptions(nGame);
		g_pSaveStateName = NULL;
		override_savestate_directory = NULL;
	}
}

static void MamePlayRecordGame(void)
{
	int  nGame;
	char filename[MAX_PATH];
	*filename = 0;

	nGame = Picker_GetSelectedItem(hwndList);
	strcpy(filename, drivers[nGame]->name);

	if (CommonFileDialog(TRUE, filename, FILETYPE_INPUT_FILES))
	{
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		char path[MAX_PATH];

		_splitpath(filename, drive, dir, fname, ext);

		sprintf(path,"%s%s",drive,dir);
		if (path[strlen(path)-1] == '\\')
			path[strlen(path)-1] = 0; // take off trailing back slash

		g_pRecordName = fname;
		override_playback_directory = path;
		MamePlayGameWithOptions(nGame);
		g_pRecordName = NULL;
		override_playback_directory = NULL;
	}
}

#ifdef KAILLERA
static void KailleraPlayRecordGame(int nGame)
{
	char filename[MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	char path[MAX_PATH];
	int num_record = 0;

	for (;;) {
		FILE *fp;

		sprintf(filename, "%s\\%s_%02d.inp", GetInpDir(), drivers[nGame]->name, num_record);

		num_record++;
		fp = fopen(filename, "r");
		if (!fp) break;
		fclose(fp);
	}

	_splitpath(filename, drive, dir, fname, ext);

	sprintf(path,"%s%s",drive,dir);
	if (path[strlen(path)-1] == '\\')
		path[strlen(path)-1] = 0; // take off trailing back slash

	g_pRecordName = fname;
	override_playback_directory = path;
	MamePlayGameWithOptions(nGame);
	g_pRecordName = NULL;
	override_playback_directory = NULL;
}
#endif /* KAILLERA */
static void MamePlayGame(void)
{
	int nGame;

	nGame = Picker_GetSelectedItem(hwndList);

	g_pPlayBkName = NULL;
	g_pRecordName = NULL;

	MamePlayGameWithOptions(nGame);
}

static void MamePlayRecordWave(void)
{
	int  nGame;
	char filename[MAX_PATH];
	*filename = 0;

	nGame = Picker_GetSelectedItem(hwndList);
	strcpy(filename, drivers[nGame]->name);

	if (CommonFileDialog(TRUE, filename, FILETYPE_WAVE_FILES))
	{
		g_pRecordWaveName = filename;
		MamePlayGameWithOptions(nGame);
		g_pRecordWaveName = NULL;
	}	
}

static void MamePlayGameWithOptions(int nGame)
{
	memcpy(&playing_game_options, GetGameOptions(nGame), sizeof(options_type));

	/* Deal with options that can be disabled. */
	EnablePlayOptions(nGame, &playing_game_options);

	if (g_pJoyGUI != NULL)
		KillTimer(hMain, JOYGUI_TIMER);
	if (GetCycleScreenshot() > 0)
		KillTimer(hMain, SCREENSHOT_TIMER);

	g_bAbortLoading = FALSE;

	in_emulation = TRUE;

	if (RunMAME(nGame) == 0)
	{
		IncrementPlayCount(nGame);
		ResetWhichGamesInFolders();
		ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
	}
	else
	{
		ShowWindow(hMain, SW_SHOW);
	}

	in_emulation = FALSE;

#ifdef KAILLERA
	hWndLoad = NULL;
	while (ShowCursor(TRUE) < 0) ;
#endif /* KAILLERA */

	// re-sort if sorting on # of times played
	if (GetSortColumn() == COLUMN_PLAYED)
		Picker_Sort(hwndList);

	UpdateStatusBar();

	ShowWindow(hMain, SW_SHOW);
	SetFocus(hwndList);

	if (g_pJoyGUI != NULL)
		SetTimer(hMain, JOYGUI_TIMER, JOYGUI_MS, NULL);
	if (GetCycleScreenshot() > 0)
		SetTimer(hMain, SCREENSHOT_TIMER, GetCycleScreenshot()*1000, NULL); //scale to seconds
}

/* Toggle ScreenShot ON/OFF */
static void ToggleScreenShot(void)
{
	BOOL showScreenShot = GetShowScreenShot();

	SetShowScreenShot((showScreenShot) ? FALSE : TRUE);
	UpdateScreenShot();

	/* Redraw list view */
	if (hBackground != NULL && showScreenShot)
		InvalidateRect(hwndList, NULL, FALSE);
}

static void AdjustMetrics(void)
{
	HDC hDC;
	TEXTMETRIC tm;
	int xtraX, xtraY;
	AREA area;
	int  offX, offY;
	int  maxX, maxY;
	COLORREF textColor;
	HWND hWnd;

	/* WM_SETTINGCHANGE also */
	xtraX  = GetSystemMetrics(SM_CXFIXEDFRAME); /* Dialog frame width */
	xtraY  = GetSystemMetrics(SM_CYFIXEDFRAME); /* Dialog frame height */
	xtraY += GetSystemMetrics(SM_CYMENUSIZE);   /* Menu height */
	xtraY += GetSystemMetrics(SM_CYCAPTION);    /* Caption Height */
	maxX   = GetSystemMetrics(SM_CXSCREEN);     /* Screen Width */
	maxY   = GetSystemMetrics(SM_CYSCREEN);     /* Screen Height */

	hDC = GetDC(hMain);
	GetTextMetrics (hDC, &tm);

	/* Convert MIN Width/Height from Dialog Box Units to pixels. */
	MIN_WIDTH  = (int)((tm.tmAveCharWidth / 4.0) * DBU_MIN_WIDTH)  + xtraX;
	MIN_HEIGHT = (int)((tm.tmHeight       / 8.0) * DBU_MIN_HEIGHT) + xtraY;
	ReleaseDC(hMain, hDC);

	if ((textColor = GetListFontColor()) == RGB(255, 255, 255))
		textColor = RGB(240, 240, 240);

	hWnd = GetWindow(hMain, GW_CHILD);
	while(hWnd)
	{
		char szClass[128];

		if (GetClassNameA(hWnd, szClass, sizeof(szClass) / sizeof(szClass[0])))
		{
			if (!strcmp(szClass, "SysListView32"))
			{
				ListView_SetBkColor(hWnd, GetSysColor(COLOR_WINDOW));
				ListView_SetTextColor(hWnd, textColor);
			}
			else if (!strcmp(szClass, "SysTreeView32"))
			{
				TreeView_SetBkColor(hTreeView, GetSysColor(COLOR_WINDOW));
				TreeView_SetTextColor(hTreeView, textColor);
			}
		}
		hWnd = GetWindow(hWnd, GW_HWNDNEXT);
	}

	GetWindowArea(&area);

	offX = area.x + area.width;
	offY = area.y + area.height;

	if (offX > maxX)
	{
		offX = maxX;
		area.x = (offX - area.width > 0) ? (offX - area.width) : 0;
	}
	if (offY > maxY)
	{
		offY = maxY;
		area.y = (offY - area.height > 0) ? (offY - area.height) : 0;
	}

	SetWindowArea(&area);
	SetWindowPos(hMain, 0, area.x, area.y, area.width, area.height, SWP_NOZORDER | SWP_SHOWWINDOW);
}


/* Adjust options - tune them to the currently selected game */
static void EnablePlayOptions(int nIndex, options_type *o)
{
	if (!DIJoystick.Available())
		o->use_joystick = FALSE;
}

static int FindIconIndex(int nIconResource)
{
	int i;
	for(i = 0; g_iconData[i].icon_name; i++)
	{
		if (g_iconData[i].resource == nIconResource)
			return i;
	}
	return -1;
}

static BOOL UseBrokenIcon(int type)
{
	//if ((GetViewMode() != VIEW_GROUPED) && (GetViewMode() != VIEW_DETAILS))
	//	return TRUE;
	if (type == 4 && !GetUseBrokenIcon())
		return FALSE;
	return TRUE;
}

static int GetIconForDriver(int nItem)
{
	int iconRoms;

	if (DriverUsesRoms(nItem))
	{
		int audit_result = GetRomAuditResults(nItem);
		if (IsAuditResultKnown(audit_result) == FALSE)
			return 2;

		if (IsAuditResultYes(audit_result))
			iconRoms = 1;
		else
			iconRoms = 0;
	}
	else
		iconRoms = 1;

	// iconRoms is now either 0 (no roms), 1 (roms), or 2 (unknown)

	/* these are indices into icon_names, which maps into our image list
	 * also must match IDI_WIN_NOROMS + iconRoms
	 */

	// Show Red-X if the ROMs are present and flagged as NOT WORKING
	if (iconRoms == 1 && DriverIsBroken(nItem))
		iconRoms = FindIconIndex(IDI_WIN_REDX);

#ifdef ZETAMAME32 // Interface
	// Show imperfect if the ROMs are present and is imperfect
	if (iconRoms == 1 && IsGameImperfect(nItem))
		iconRoms = FindIconIndex(IDI_WIN_IMPERFECT);
#endif /* ZETAMAME32 */

	// show clone icon if we have roms and game is working
	if (iconRoms == 1 && DriverIsClone(nItem))
		iconRoms = FindIconIndex(IDI_WIN_CLONE);

	// if we have the roms, then look for a custom per-game icon to override
#ifdef ZETAMAME32 // Interface
	if (iconRoms == 1 || iconRoms == 3 || iconRoms == 5 || !UseBrokenIcon(iconRoms))
#else
	if (iconRoms == 1 || iconRoms == 3 || !UseBrokenIcon(iconRoms))
#endif /* ZETAMAME32 */
	{
		if (icon_index[nItem] == 0)
			AddDriverIcon(nItem,iconRoms);
		iconRoms = icon_index[nItem];
	}

	return iconRoms;
}

static BOOL HandleTreeContextMenu(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HMENU hTreeMenu;
	HMENU hMenu;
	TVHITTESTINFO hti;
	POINT pt;

	if ((HWND)wParam != GetDlgItem(hWnd, IDC_TREE))
		return FALSE;

	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);
	if (pt.x < 0 && pt.y < 0)
		GetCursorPos(&pt);

	/* select the item that was right clicked or shift-F10'ed */
	hti.pt = pt;
	ScreenToClient(hTreeView,&hti.pt);
	TreeView_HitTest(hTreeView,&hti);
	if ((hti.flags & TVHT_ONITEM) != 0)
		TreeView_SelectItem(hTreeView,hti.hItem);

	hTreeMenu = LoadMenu(hInst,MAKEINTRESOURCE(IDR_CONTEXT_TREE));

	InitTreeContextMenu(hTreeMenu);

	hMenu = GetSubMenu(hTreeMenu, 0);

	TranslateMenu(hMenu, ID_CONTEXT_RENAME_CUSTOM);
	UpdateMenu(hMenu);

	TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,0,hWnd,NULL);

	DestroyMenu(hTreeMenu);

	return TRUE;
}

static void GamePicker_OnBodyContextMenu(POINT pt)
{
	HMENU hMenuLoad;
	HMENU hMenu;

	hMenuLoad = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT_MENU));
	hMenu = GetSubMenu(hMenuLoad, 0);
	TranslateMenu(hMenu, ID_FILE_PLAY);

	UpdateMenu(hMenu);

	TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,0,hMain,NULL);

	DestroyMenu(hMenuLoad);
}



static BOOL HandleScreenShotContextMenu(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenuLoad;
	HMENU hMenu;
	POINT pt;

	if ((HWND)wParam != GetDlgItem(hWnd, IDC_SSPICTURE) && (HWND)wParam != GetDlgItem(hWnd, IDC_SSFRAME))
		return FALSE;

	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);
	if (pt.x < 0 && pt.y < 0)
		GetCursorPos(&pt);

	hMenuLoad = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT_SCREENSHOT));
	hMenu = GetSubMenu(hMenuLoad, 0);
	TranslateMenu(hMenu, ID_VIEW_PAGETAB);

	UpdateMenu(hMenu);

	TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,0,hWnd,NULL);

	DestroyMenu(hMenuLoad);

	return TRUE;
}

static void UpdateMenu(HMENU hMenu)
{
	char			buf[200];
	MENUITEMINFO	mItem;
	int 			nGame = Picker_GetSelectedItem(hwndList);
	LPTREEFOLDER lpFolder = GetCurrentFolder();
	int i;

	if (have_selection)
	{
		snprintf(buf, sizeof(buf), _UI("&Play %s"),
		         ConvertAmpersandString(UseLangList() ?
		                                _LST(drivers[nGame]->description) :
		                                ModifyThe(drivers[nGame]->description)));

		mItem.cbSize     = sizeof(mItem);
		mItem.fMask      = MIIM_TYPE;
		mItem.fType      = MFT_STRING;
		mItem.dwTypeData = _Unicode(buf);
		mItem.cch        = lstrlen(mItem.dwTypeData);

		SetMenuItemInfo(hMenu, ID_FILE_PLAY, FALSE, &mItem);

#ifdef KAILLERA
		EnableMenuItem(hMenu, ID_FILE_NETPLAY, MF_ENABLED);
#endif /* KAILLERA */
#ifdef MAME_AVI
    EnableMenuItem(hMenu, ID_FILE_PLAY_BACK_AVI,    MF_ENABLED);
    EnableMenuItem(hMenu, ID_FILE_PLAY_WITH_AVI, 	MF_ENABLED);
#endif

		snprintf(buf, sizeof(buf) / sizeof(buf[0]), _UI("Proper&ties of %s"),
		         GetDriverFilename(nGame));

		mItem.cbSize     = sizeof(mItem);
		mItem.fMask      = MIIM_TYPE;
		mItem.fType      = MFT_STRING;
		mItem.dwTypeData = _Unicode(buf);
		mItem.cch        = lstrlen(mItem.dwTypeData);

		SetMenuItemInfo(hMenu, ID_SOURCE_PROPERTIES, FALSE, &mItem);
		EnableMenuItem(hMenu, ID_CONTEXT_SELECT_RANDOM, MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hMenu, ID_FILE_PLAY,             MF_GRAYED);
		EnableMenuItem(hMenu, ID_FILE_PLAY_RECORD,      MF_GRAYED);
		EnableMenuItem(hMenu, ID_GAME_PROPERTIES,       MF_GRAYED);
		EnableMenuItem(hMenu, ID_SOURCE_PROPERTIES,     MF_GRAYED);
#ifdef KAILLERA
		EnableMenuItem(hMenu, ID_FILE_NETPLAY,          MF_GRAYED);
#endif /* KAILLERA */
#ifdef MAME_AVI
    EnableMenuItem(hMenu, ID_FILE_PLAY_WITH_AVI,    MF_GRAYED);	// NEW_AVI
#endif
		EnableMenuItem(hMenu, ID_CONTEXT_SELECT_RANDOM, MF_GRAYED);
	}

	if (oldControl)
	{
		EnableMenuItem(hMenu, ID_CUSTOMIZE_FIELDS,  MF_GRAYED);
		EnableMenuItem(hMenu, ID_GAME_PROPERTIES,   MF_GRAYED);
		EnableMenuItem(hMenu, ID_SOURCE_PROPERTIES, MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPTIONS_DEFAULTS,  MF_GRAYED);
	}

	if (lpFolder->m_dwFlags & F_CUSTOM)
	{
		EnableMenuItem(hMenu,ID_CONTEXT_REMOVE_CUSTOM,MF_ENABLED);
		EnableMenuItem(hMenu,ID_CONTEXT_RENAME_CUSTOM,MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hMenu,ID_CONTEXT_REMOVE_CUSTOM,MF_GRAYED);
		EnableMenuItem(hMenu,ID_CONTEXT_RENAME_CUSTOM,MF_GRAYED);
	}

	if (lpFolder && (IsSourceFolder(lpFolder) || IsVectorFolder(lpFolder)))
		EnableMenuItem(hMenu,ID_FOLDER_PROPERTIES,MF_ENABLED);
	else
		EnableMenuItem(hMenu,ID_FOLDER_PROPERTIES,MF_GRAYED);

	CheckMenuRadioItem(hMenu, ID_VIEW_TAB_SCREENSHOT, ID_VIEW_TAB_HISTORY,
					   ID_VIEW_TAB_SCREENSHOT + TabView_GetCurrentTab(hTabCtrl), MF_BYCOMMAND);

	// set whether we're showing the tab control or not
	if (bShowTabCtrl)
		CheckMenuItem(hMenu,ID_VIEW_PAGETAB,MF_BYCOMMAND | MF_CHECKED);
	else
		CheckMenuItem(hMenu,ID_VIEW_PAGETAB,MF_BYCOMMAND | MF_UNCHECKED);


	for (i=0; i < MAX_TAB_TYPES; i++)
	{
		// disable menu items for tabs we're not currently showing
		if (GetShowTab(i))
			EnableMenuItem(hMenu,ID_VIEW_TAB_SCREENSHOT + i,MF_BYCOMMAND | MF_ENABLED);
		else
			EnableMenuItem(hMenu,ID_VIEW_TAB_SCREENSHOT + i,MF_BYCOMMAND | MF_GRAYED);

		// check toggle menu items 
		if (GetShowTab(i))
			CheckMenuItem(hMenu, ID_TOGGLE_TAB_SCREENSHOT + i,MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_TOGGLE_TAB_SCREENSHOT + i,MF_BYCOMMAND | MF_UNCHECKED);
	}

	for (i=0; i < MAX_FOLDERS; i++)
	{
		if (GetShowFolder(i))
			CheckMenuItem(hMenu,ID_CONTEXT_SHOW_FOLDER_START + i,MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem(hMenu,ID_CONTEXT_SHOW_FOLDER_START + i,MF_BYCOMMAND | MF_UNCHECKED);
	}

}

void InitTreeContextMenu(HMENU hTreeMenu)
{
	MENUITEMINFOA mii;
	HMENU hMenu;
	int i;

	ZeroMemory(&mii,sizeof(mii));
	mii.cbSize = sizeof(mii);

	mii.wID = -1;
	mii.fMask = MIIM_SUBMENU | MIIM_ID;

	hMenu = GetSubMenu(hTreeMenu, 0);

	if (GetMenuItemInfoA(hMenu,3,TRUE,&mii) == FALSE)
	{
		dprintf("can't find show folders context menu");
		return;
	}

	if (mii.hSubMenu == NULL)
	{
		dprintf("can't find submenu for show folders context menu");
		return;
	}

	hMenu = mii.hSubMenu;

	for (i=0; g_folderData[i].m_lpTitle != NULL; i++)
	{
		mii.fMask = MIIM_TYPE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.dwTypeData = (char *)g_folderData[i].m_lpTitle;
		mii.cch = strlen(mii.dwTypeData);
		mii.wID = ID_CONTEXT_SHOW_FOLDER_START + g_folderData[i].m_nFolderId;


		// menu in resources has one empty item (needed for the submenu to setup properly)
		// so overwrite this one, append after
		if (i == 0)
			SetMenuItemInfoA(hMenu,ID_CONTEXT_SHOW_FOLDER_START,FALSE,&mii);
		else
			InsertMenuItemA(hMenu,i,FALSE,&mii);
	}

}

void ToggleShowFolder(int folder)
{
	int current_id = GetCurrentFolderID();

	SetWindowRedraw(hwndList,FALSE);

	SetShowFolder(folder,!GetShowFolder(folder));

	ResetTreeViewFolders();
	SelectTreeViewFolder(current_id);

	SetWindowRedraw(hwndList,TRUE);
}

static LRESULT CALLBACK HistoryWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef DARKMAME	// HISTORY_BG
	if (hSnapBackground)
#else /*DARKMAME*/
	if (hBackground)
#endif /*DARKMAME*/
	{
		switch (uMsg)
		{
		case WM_MOUSEMOVE:
			if (MouseHasBeenMoved())
				ShowCursor(TRUE);
			break;

		case WM_ERASEBKGND:
			return TRUE;
		case WM_PAINT:
		{
			POINT p = { 0, 0 };
			
			/* get base point of background bitmap */
			MapWindowPoints(hWnd,hTreeView,&p,1);
			PaintBackgroundImage(hWnd, NULL, p.x, p.y);
			/* to ensure our parent procedure repaints the whole client area */
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		}
	}
	return CallWindowProc(g_lpHistoryWndProc, hWnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK PictureFrameWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_MOUSEMOVE:
		if (MouseHasBeenMoved())
			ShowCursor(TRUE);
		break;

	case WM_NCHITTEST :
	{
		POINT pt;
		RECT  rect;
		HWND hHistory = GetDlgItem(hMain, IDC_HISTORY);

		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		GetWindowRect(hHistory, &rect);
		// check if they clicked on the picture area (leave 6 pixel no man's land
		// by the history window to reduce mistaken clicks)
		// no more no man's land, the Cursor changes when Edit control is left, should be enough feedback
		if (have_history &&        
			( ( (TabView_GetCurrentTab(hTabCtrl) == TAB_HISTORY) || 
			 (TabView_GetCurrentTab(hTabCtrl) == GetHistoryTab() && GetShowTab(TAB_HISTORY) == FALSE) ||
		        (TAB_ALL == GetHistoryTab() && GetShowTab(TAB_HISTORY) == FALSE) ) &&
//		        (rect.top - 6) < pt.y && pt.y < (rect.bottom + 6) ) )
		        PtInRect( &rect, pt ) ) )
		{
			return HTTRANSPARENT;
		}
		else
		{
			return HTCLIENT;
		}

		break;
	}

	case WM_CONTEXTMENU:
		if ( HandleScreenShotContextMenu(hWnd, wParam, lParam))
			return FALSE;
		break;
	}
	
#ifdef DARKMAME	// HISTORY_BG
	if (hSnapBackground)
#else /*DARKMAME*/
	if (hBackground)
#endif /*DARKMAME*/
	{
		switch (uMsg)
		{
		case WM_ERASEBKGND :
			return TRUE;
		case WM_PAINT :
		{
			RECT rect,nodraw_rect;
			HRGN region,nodraw_region;
			POINT p = { 0, 0 };

			/* get base point of background bitmap */
			MapWindowPoints(hWnd,hTreeView,&p,1);

			/* get big region */
			GetClientRect(hWnd,&rect);
			region = CreateRectRgnIndirect(&rect);

			if (IsWindowVisible(GetDlgItem(hMain,IDC_HISTORY)))
			{
				/* don't draw over this window */
				GetWindowRect(GetDlgItem(hMain,IDC_HISTORY),&nodraw_rect);
				MapWindowPoints(HWND_DESKTOP,hWnd,(LPPOINT)&nodraw_rect,2);
				nodraw_region = CreateRectRgnIndirect(&nodraw_rect);
				CombineRgn(region,region,nodraw_region,RGN_DIFF);
				DeleteObject(nodraw_region);
			}
			if (IsWindowVisible(GetDlgItem(hMain,IDC_SSPICTURE)))
			{
				/* don't draw over this window */
				GetWindowRect(GetDlgItem(hMain,IDC_SSPICTURE),&nodraw_rect);
				MapWindowPoints(HWND_DESKTOP,hWnd,(LPPOINT)&nodraw_rect,2);
				nodraw_region = CreateRectRgnIndirect(&nodraw_rect);
				CombineRgn(region,region,nodraw_region,RGN_DIFF);
				DeleteObject(nodraw_region);
			}

			PaintBackgroundImage(hWnd,region,p.x,p.y);

			DeleteObject(region);

			/* to ensure our parent procedure repaints the whole client area */
			InvalidateRect(hWnd, NULL, FALSE);

			break;
		}
		}
	}
	return CallWindowProc(g_lpPictureFrameWndProc, hWnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK PictureWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ERASEBKGND :
		return TRUE;
	case WM_PAINT :
	{
		PAINTSTRUCT ps;
		HDC	hdc,hdc_temp;
		RECT rect;
		HBITMAP old_bitmap;

		int width,height;

		RECT rect2;
		HBRUSH hBrush;
		HBRUSH holdBrush;
		HRGN region1, region2;
		int nBordersize;
		nBordersize = GetScreenshotBorderSize();
		hBrush = CreateSolidBrush(GetScreenshotBorderColor());

		hdc = BeginPaint(hWnd,&ps);

		hdc_temp = CreateCompatibleDC(hdc);
		if (ScreenShotLoaded())
		{
			width = GetScreenShotWidth();
			height = GetScreenShotHeight();

			old_bitmap = SelectObject(hdc_temp,GetScreenShotHandle());
		}
		else
		{
			BITMAP bmp;

			GetObject(hMissing_bitmap,sizeof(BITMAP),&bmp);
			width = bmp.bmWidth;
			height = bmp.bmHeight;

			old_bitmap = SelectObject(hdc_temp,hMissing_bitmap);
		}

		GetClientRect(hWnd,&rect);

		rect2 = rect;
		//Configurable Borders around images
		rect.bottom -= nBordersize;
		if( rect.bottom < 0)
			rect.bottom = rect2.bottom;
		rect.right -= nBordersize;
		if( rect.right < 0)
			rect.right = rect2.right;
		rect.top += nBordersize;
		if( rect.top > rect.bottom )
			rect.top = rect2.top;
		rect.left += nBordersize;
		if( rect.left > rect.right )
			rect.left = rect2.left;
		region1 = CreateRectRgnIndirect(&rect);
		region2 = CreateRectRgnIndirect(&rect2);
		CombineRgn(region2,region2,region1,RGN_DIFF);
		holdBrush = SelectObject(hdc, hBrush); 

		FillRgn(hdc,region2, hBrush );
		SelectObject(hdc, holdBrush); 
		DeleteObject(hBrush); 

		SetStretchBltMode(hdc,STRETCH_HALFTONE);
		StretchBlt(hdc,nBordersize,nBordersize,rect.right-rect.left,rect.bottom-rect.top,
		           hdc_temp,0,0,width,height,SRCCOPY);
		SelectObject(hdc_temp,old_bitmap);
		DeleteDC(hdc_temp);

		EndPaint(hWnd,&ps);

		return TRUE;
	}
	}

	return CallWindowProc(g_lpPictureWndProc, hWnd, uMsg, wParam, lParam);
}

// replaces function in src/windows/fileio.c:
int osd_display_loading_rom_message(const char *name,struct rom_load_data *romdata)
{
	int retval;

	if (use_gui_romloading)
	{
		options.gui_host = 1;
		retval = UpdateLoadProgress(name,romdata);
	}
	else
	{
		if (name)
			fprintf(stdout, _WINDOWS("loading %-12s\r"), name);
		else
			fprintf(stdout, "%30s\r", "");
		fflush (stdout);

		retval = 0;
	}

	return retval;
}

static INT_PTR CALLBACK LoadProgressDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG :
	{
		char buf[256];
		
		TranslateDialog(hDlg, lParam, TRUE);
		sprintf(buf, _UI("Loading %s"), options.use_lang_list ?
			_LST(Machine->gamedrv->description):Machine->gamedrv->description);
		SetWindowText(hDlg, _Unicode(buf));
		
		g_bCloseLoading = FALSE;
		g_bAbortLoading = FALSE;

		return 1;
	}

	case WM_CLOSE:
		if (!g_bCloseLoading)
			g_bAbortLoading = TRUE;
		EndDialog(hDlg, 0);
		return 1;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			g_bAbortLoading = TRUE;
			EndDialog(hDlg, IDCANCEL);
			return 1;
		}
		if (LOWORD(wParam) == IDOK)
		{
			g_bCloseLoading = TRUE;
			EndDialog(hDlg, IDOK);
			return 1;
		}
	}
	return 0;
}

int UpdateLoadProgress(const char* name, const struct rom_load_data *romdata)
{
#ifndef KAILLERA
	static HWND hWndLoad = 0;
#endif /* KAILLERA */
	MSG Msg;

	int current = romdata->romsloaded;
	int total = romdata->romstotal;

	//dprintf("updateloadprogress %s %u %u %08x\n",name,current,total,hWndLoad);

	if (hWndLoad == NULL)
	{
		InitTranslator(options.langcode);
		hWndLoad = CreateDialog(GetModuleHandle(NULL),
		                        MAKEINTRESOURCE(IDD_LOAD_PROGRESS),
		                        hMain,
		                        LoadProgressDialogProc);

		EnableWindow(GetDlgItem(hWndLoad,IDOK),FALSE);
		EnableWindow(GetDlgItem(hWndLoad,IDCANCEL),TRUE);

		ShowWindow(hWndLoad,SW_SHOW);
	}

	SetWindowText(GetDlgItem(hWndLoad, IDC_LOAD_STATUS),
				  _Unicode(ConvertToWindowsNewlines(romdata->errorbuf)));

	SendDlgItemMessage(hWndLoad, IDC_LOAD_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, total));
	SendDlgItemMessage(hWndLoad, IDC_LOAD_PROGRESS, PBM_SETPOS, current, 0);

	if (name == NULL)
	{
		// final call to us
		SetWindowText(GetDlgItem(hWndLoad, IDC_LOAD_ROMNAME), TEXT(""));
		if (romdata->errors > 0 || romdata->warnings > 0)
		{

			/*
			  Shows the load progress dialog if there is an error while
			  loading the game.
			*/

			ShowWindow(hWndLoad, SW_SHOW);

			EnableWindow(GetDlgItem(hWndLoad,IDOK),TRUE);
			if (romdata->errors != 0)
				EnableWindow(GetDlgItem(hWndLoad,IDCANCEL),FALSE);
			SetFocus(GetDlgItem(hWndLoad,IDOK));
			if (romdata->errors)
				SetWindowText(GetDlgItem(hWndLoad,IDC_ERROR_TEXT),
							  _Unicode(_UI("ERROR: required files are missing, the game cannot be run.")));
			else
				SetWindowText(GetDlgItem(hWndLoad,IDC_ERROR_TEXT),
							  _Unicode(_UI("WARNING: the game might not run correctly.")));
		}
	}
	else
		SetWindowText(GetDlgItem(hWndLoad, IDC_LOAD_ROMNAME), _Unicode(name));

	if (name == NULL && (romdata->errors > 0 || romdata->warnings > 0))
	{
		while (GetMessage(&Msg, NULL, 0, 0))
		{
			if (!IsDialogMessage(hWndLoad, &Msg))
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}

			// make sure the user clicks-through on an error/warning
			if (g_bCloseLoading || g_bAbortLoading)
				break;
#ifdef KAILLERA
			if (kPlay) break;
#endif /* KAILLERA */
		}

	}

	if (name == NULL)
	{
		DestroyWindow(hWndLoad);
		hWndLoad = NULL;
	}

	// take care of any pending messages
	while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!IsDialogMessage(hWndLoad, &Msg))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

	// if abort with a warning, gotta exit abruptly
	if (name == NULL && g_bAbortLoading && romdata->errors == 0)
#ifdef KAILLERA
		return 1;
#else /* KAILLERA */
		exit(1);
#endif /* KAILLERA */

	// if abort along the way, tell 'em
	if (g_bAbortLoading && name != NULL)
	{
#ifdef KAILLERA
		return 1;
#else /* KAILLERA */
		exit(1);
#endif /* KAILLERA */
	}

	return 0;
}

void RemoveCurrentGameCustomFolder(void)
{
	RemoveGameCustomFolder(Picker_GetSelectedItem(hwndList));
}

void RemoveGameCustomFolder(int driver_index)
{
	int i;
	TREEFOLDER **folders;
	int num_folders;

	GetFolders(&folders,&num_folders);
	
	for (i=0;i<num_folders;i++)
	{
		if (folders[i]->m_dwFlags & F_CUSTOM && folders[i]->m_nFolderId == GetCurrentFolderID())
		{
			int current_pick_index;

			RemoveFromCustomFolder(folders[i],driver_index);

			if (driver_index == Picker_GetSelectedItem(hwndList))
			{
				/* if we just removed the current game,
				  move the current selection so that when we rebuild the listview it
				  leaves the cursor on next or previous one */
			
				current_pick_index = GetSelectedPick();
				Picker_SetSelectedPick(hwndList, GetSelectedPick() + 1);
				if (current_pick_index == GetSelectedPick()) /* we must have deleted the last item */
					Picker_SetSelectedPick(hwndList, GetSelectedPick() - 1);
			}

			ResetListView();
			return;
		}
	}
	MessageBox(GetMainWindow(), _Unicode(_UI("Error searching for custom folder")), TEXT_MAME32NAME, MB_OK | MB_ICONERROR);

}


void BeginListViewDrag(NM_LISTVIEW *pnmv)
{
	LV_ITEM lvi;
	POINT pt;

	lvi.iItem = pnmv->iItem;
	lvi.mask	 = LVIF_PARAM;
	ListView_GetItem(hwndList, &lvi);

	game_dragged = lvi.lParam;

	pt.x = 0;
	pt.y = 0;

	/* Tell the list view control to create an image to use 
	   for dragging. */
	himl_drag = ListView_CreateDragImage(hwndList,pnmv->iItem,&pt);
 
	/* Start the drag operation. */
	ImageList_BeginDrag(himl_drag, 0, 0, 0); 

	pt = pnmv->ptAction;
	ClientToScreen(hwndList,&pt);
	ImageList_DragEnter(GetDesktopWindow(),pt.x,pt.y);

	/* Hide the mouse cursor, and direct mouse input to the 
	   parent window. */
	SetCapture(hMain);

	prev_drag_drop_target = NULL;

	g_listview_dragging = TRUE; 

}

void MouseMoveListViewDrag(POINTS p)
{
	HTREEITEM htiTarget;
	TV_HITTESTINFO tvht;

	POINT pt;
	pt.x = p.x;
	pt.y = p.y;

	ClientToScreen(hMain,&pt);

	ImageList_DragMove(pt.x,pt.y);

	MapWindowPoints(GetDesktopWindow(),hTreeView,&pt,1);

	tvht.pt = pt;
	htiTarget = TreeView_HitTest(hTreeView,&tvht);

	if (htiTarget != prev_drag_drop_target)
	{
		ImageList_DragShowNolock(FALSE);
		if (htiTarget != NULL)
			TreeView_SelectDropTarget(hTreeView,htiTarget);
		else
			TreeView_SelectDropTarget(hTreeView,NULL);
		ImageList_DragShowNolock(TRUE);

		prev_drag_drop_target = htiTarget;
	}
}

void ButtonUpListViewDrag(POINTS p)
{
	POINT pt;
	HTREEITEM htiTarget;
	TV_HITTESTINFO tvht;
	TVITEM tvi;
	
	ReleaseCapture();

	ImageList_DragLeave(hwndList);
	ImageList_EndDrag();
	ImageList_Destroy(himl_drag);

	TreeView_SelectDropTarget(hTreeView,NULL);

	g_listview_dragging = FALSE;

	/* see where the game was dragged */

	pt.x = p.x;
	pt.y = p.y;

	MapWindowPoints(hMain,hTreeView,&pt,1);

	tvht.pt = pt;
	htiTarget = TreeView_HitTest(hTreeView,&tvht);
	if (htiTarget == NULL)
	{
		LVHITTESTINFO lvhtti;
		LPTREEFOLDER folder;
		RECT rcList;

		/* the user dragged a game onto something other than the treeview */
		/* try to remove if we're in a custom folder */

		/* see if it was dragged within the list view; if so, ignore */

		MapWindowPoints(hTreeView,hwndList,&pt,1);
		lvhtti.pt = pt;
		GetWindowRect(hwndList, &rcList);
		ClientToScreen(hwndList, &pt);
		if( PtInRect(&rcList, pt) != 0 )
			return;

		folder = GetCurrentFolder();
		if (folder->m_dwFlags & F_CUSTOM)
		{
			/* dragged out of a custom folder, so let's remove it */
			RemoveCurrentGameCustomFolder();
		}
		return;
	}


	tvi.lParam = 0;
	tvi.mask  = TVIF_PARAM | TVIF_HANDLE;
	tvi.hItem = htiTarget;

	if (TreeView_GetItem(hTreeView, &tvi))
	{
		LPTREEFOLDER folder = (LPTREEFOLDER)tvi.lParam;
		AddToCustomFolder(folder,game_dragged);
	}

}

static LPTREEFOLDER GetSelectedFolder(void)
{
	HTREEITEM htree;
	TVITEM tvi;
	htree = TreeView_GetSelection(hTreeView);
	if(htree != NULL)
	{
		tvi.hItem = htree;
		tvi.mask = TVIF_PARAM;
		TreeView_GetItem(hTreeView,&tvi);
		return (LPTREEFOLDER)tvi.lParam;
	}
	return NULL;
}

static HICON GetSelectedFolderIcon(void)
{
	HTREEITEM htree;
	TVITEM tvi;
	HIMAGELIST hSmall_icon;
	LPTREEFOLDER folder;
	htree = TreeView_GetSelection(hTreeView);

	if (htree != NULL)
	{
		tvi.hItem = htree;
		tvi.mask = TVIF_PARAM;
		TreeView_GetItem(hTreeView,&tvi);
		
		folder = (LPTREEFOLDER)tvi.lParam;
		//hSmall_icon = TreeView_GetImageList(hTreeView,(int)tvi.iImage);
		hSmall_icon = NULL;
		return ImageList_GetIcon(hSmall_icon, tvi.iImage, ILD_TRANSPARENT);
	}
	return NULL;
}

/* Updates all currently displayed Items in the List with the latest Data*/
void UpdateListView(void)
{
	if( (GetViewMode() == VIEW_GROUPED) || (GetViewMode() == VIEW_DETAILS ) )
	    ListView_RedrawItems(hwndList,ListView_GetTopIndex(hwndList),
	                         ListView_GetTopIndex(hwndList)+ ListView_GetCountPerPage(hwndList) );
}

void CalculateBestScreenShotRect(HWND hWnd, RECT *pRect, BOOL restrict_height)
{
	int 	destX, destY;
	int 	destW, destH;
	int	nBorder;
	RECT	rect;
	/* for scaling */		 
	int x, y;
	int rWidth, rHeight;
	double scale;
	BOOL bReduce = FALSE;

	GetClientRect(hWnd, &rect);

	// Scale the bitmap to the frame specified by the passed in hwnd
	if (ScreenShotLoaded())
	{
		x = GetScreenShotWidth();
		y = GetScreenShotHeight();
	}
	else
	{
		BITMAP bmp;
		GetObject(hMissing_bitmap,sizeof(BITMAP),&bmp);

		x = bmp.bmWidth;
		y = bmp.bmHeight;
	}
	rWidth	= (rect.right  - rect.left);
	rHeight = (rect.bottom - rect.top);

	/* Limit the screen shot to max height of 264 */
	if (restrict_height == TRUE && rHeight > 264)
	{
		rect.bottom = rect.top + 264;
		rHeight = 264;
	}

	/* If the bitmap does NOT fit in the screenshot area */
	if (x > rWidth - 10 || y > rHeight - 10)
	{
		rect.right	-= 10;
		rect.bottom -= 10;
		rWidth	-= 10;
		rHeight -= 10;
		bReduce = TRUE;
		/* Try to scale it properly */
		/*	assumes square pixels, doesn't consider aspect ratio */
		if (x > y)
			scale = (double)rWidth / x;
		else
			scale = (double)rHeight / y;

		destW = (int)(x * scale);
		destH = (int)(y * scale);

		/* If it's still too big, scale again */
		if (destW > rWidth || destH > rHeight)
		{
			if (destW > rWidth)
				scale = (double)rWidth	/ destW;
			else
				scale = (double)rHeight / destH;

			destW = (int)(destW * scale);
			destH = (int)(destH * scale);
		}
	}
	else
	{
		if (GetStretchScreenShotLarger())
		{
			rect.right	-= 10;
			rect.bottom -= 10;
			rWidth	-= 10;
			rHeight -= 10;
			bReduce = TRUE;
			// Try to scale it properly
			// assumes square pixels, doesn't consider aspect ratio
			if (x < y)
				scale = (double)rWidth / x;
			else
				scale = (double)rHeight / y;
			
			destW = (int)(x * scale);
			destH = (int)(y * scale);
			
			// If it's too big, scale again
			if (destW > rWidth || destH > rHeight)
			{
				if (destW > rWidth)
					scale = (double)rWidth	/ destW;
				else
					scale = (double)rHeight / destH;
				
				destW = (int)(destW * scale);
				destH = (int)(destH * scale);
			}
		}
		else
		{
			// Use the bitmaps size if it fits
			destW = x;
			destH = y;
		}

	}


	destX = ((rWidth  - destW) / 2);
	destY = ((rHeight - destH) / 2);

	if (bReduce)
	{
		destX += 5;
		destY += 5;
	}
	nBorder = GetScreenshotBorderSize();
	if( destX > nBorder+1)
		pRect->left   = destX - nBorder;
	else
		pRect->left   = 2;
	if( destY > nBorder+1)
		pRect->top    = destY - nBorder;
	else
		pRect->top    = 2;
	if( rWidth >= destX + destW + nBorder)
		pRect->right  = destX + destW + nBorder;
	else
		pRect->right  = rWidth - pRect->left;
	if( rHeight >= destY + destH + nBorder)
		pRect->bottom = destY + destH + nBorder;
	else
		pRect->bottom = rHeight - pRect->top;
}

/*
  Switches to either fullscreen or normal mode, based on the
  current mode.

  POSSIBLE BUGS:
  Removing the menu might cause problems later if some
  function tries to poll info stored in the menu. Don't
  know if you've done that, but this was the only way I
  knew to remove the menu dynamically. 
*/

void SwitchFullScreenMode(void)
{
	LONG lMainStyle;
	
	if (GetRunFullScreen())
	{
		// Return to normal

		// Restore the menu
		SetMenu(hMain, LoadMenu(hInst,MAKEINTRESOURCE(IDR_UI_MENU)));
		TranslateMenu(GetMenu(hMain), 0);

		// Refresh the checkmarks
		CheckMenuItem(GetMenu(hMain), ID_VIEW_FOLDERS, GetShowFolderList() ? MF_CHECKED : MF_UNCHECKED); 
		CheckMenuItem(GetMenu(hMain), ID_VIEW_TOOLBARS, GetShowToolBar() ? MF_CHECKED : MF_UNCHECKED);    
		CheckMenuItem(GetMenu(hMain), ID_VIEW_STATUS, GetShowStatusBar() ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(GetMenu(hMain), ID_VIEW_PAGETAB, GetShowTabCtrl() ? MF_CHECKED : MF_UNCHECKED);

		// Add frame to dialog again
		lMainStyle = GetWindowLong(hMain, GWL_STYLE);
		lMainStyle = lMainStyle | WS_BORDER;
		SetWindowLong(hMain, GWL_STYLE, lMainStyle);

		// Show the window maximized
		if( GetWindowState() == SW_MAXIMIZE )
		{
			ShowWindow(hMain, SW_NORMAL);
			ShowWindow(hMain, SW_MAXIMIZE);
		}
		else
			ShowWindow(hMain, SW_RESTORE);

		SetRunFullScreen(FALSE);
	}
	else
	{
		// Set to fullscreen

		// Remove menu
		SetMenu(hMain,NULL); 

		// Frameless dialog (fake fullscreen)
		lMainStyle = GetWindowLong(hMain, GWL_STYLE);
		lMainStyle = lMainStyle & (WS_BORDER ^ 0xffffffff);
		SetWindowLong(hMain, GWL_STYLE, lMainStyle);
		if( IsMaximized(hMain) )
		{
			ShowWindow(hMain, SW_NORMAL);
			SetWindowState( SW_MAXIMIZE );
		}
		ShowWindow(hMain, SW_MAXIMIZE);

		SetRunFullScreen(TRUE);
	}
}

/*
  Checks to see if the mouse has been moved since this func
  was first called (which is at startup). The reason for 
  storing the startup coordinates of the mouse is that when
  a window is created it generates WM_MOUSEOVER events, even
  though the user didn't actually move the mouse. So we need
  to know when the WM_MOUSEOVER event is user-triggered.

  POSSIBLE BUGS:
  Gets polled at every WM_MOUSEMOVE so it might cause lag,
  but there's probably another way to code this that's 
  way better?
  
*/

BOOL MouseHasBeenMoved(void)
{
	static int mouse_x = -1;
	static int mouse_y = -1;
	POINT p;

	GetCursorPos(&p);

	if (mouse_x == -1) // First time
	{
		mouse_x = p.x;
		mouse_y = p.y;
	}
	
	return (p.x != mouse_x || p.y != mouse_y);       
}

/*
	The following two functions enable us to send Messages to the Game Window
*/
#if MULTISESSION

BOOL SendIconToEmulationWindow(int nGameIndex)
{
	HICON hIcon; 
	hIcon = LoadIconFromFile(drivers[nGameIndex]->name); 
	if( hIcon == NULL ) 
	{ 
		//Check if clone, if so try parent icon first 
		if( DriverIsClone(nGameIndex) ) 
		{ 
			hIcon = LoadIconFromFile(drivers[nGameIndex]->clone_of->name); 
			if( hIcon == NULL) 
			{ 
				hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAME32_ICON)); 
			} 
		} 
		else 
		{ 
			hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAME32_ICON)); 
		} 
	} 
	if( SendMessageToEmulationWindow( WM_SETICON, ICON_SMALL, (LPARAM)hIcon ) == FALSE)
		return FALSE;
	if( SendMessageToEmulationWindow( WM_SETICON, ICON_BIG, (LPARAM)hIcon ) == FALSE)
		return FALSE;
	return TRUE;
}

BOOL SendMessageToEmulationWindow(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	FINDWINDOWHANDLE fwhs;
	fwhs.ProcessInfo = NULL;
	fwhs.hwndFound  = NULL;

	EnumWindows(EnumWindowCallBack, (LPARAM)&fwhs);
	if( fwhs.hwndFound )
	{
		SendMessage(fwhs.hwndFound, Msg, wParam, lParam);
		//Fix for loosing Focus, we reset the Focus on our own Main window
		SendMessage(fwhs.hwndFound, WM_KILLFOCUS,(WPARAM) hMain, (LPARAM) NULL);
		return TRUE;
	}
	return FALSE;
}


static BOOL CALLBACK EnumWindowCallBack(HWND hwnd, LPARAM lParam) 
{ 
	FINDWINDOWHANDLE * pfwhs = (FINDWINDOWHANDLE * )lParam; 
	DWORD ProcessId, ProcessIdGUI; 
	char buffer[MAX_PATH]; 

	GetWindowThreadProcessId(hwnd, &ProcessId);
	GetWindowThreadProcessId(hMain, &ProcessIdGUI);

	// cmk--I'm not sure I believe this note is necessary
	// note: In order to make sure we have the MainFrame, verify that the title 
	// has Zero-Length. Under Windows 98, sometimes the Client Window ( which doesn't 
	// have a title ) is enumerated before the MainFrame 

	GetWindowText(hwnd, buffer, sizeof(buffer));
	if (ProcessId  == ProcessIdGUI &&
		 strncmp(buffer,MAMENAME,strlen(MAMENAME)) == 0 &&
		 hwnd != hMain) 
	{ 
		pfwhs->hwndFound = hwnd; 
		return FALSE; 
	} 
	else 
	{ 
		// Keep enumerating 
		return TRUE; 
	} 
}

HWND GetGameWindow(void)
{
	FINDWINDOWHANDLE fwhs;
	fwhs.ProcessInfo = NULL;
	fwhs.hwndFound  = NULL;

	EnumWindows(EnumWindowCallBack, (LPARAM)&fwhs);
	return fwhs.hwndFound;
}


#else

void SendIconToProcess(LPPROCESS_INFORMATION pi, int nGameIndex)
{
	HICON hIcon; 
	hIcon = LoadIconFromFile(drivers[nGameIndex]->name); 
	if( hIcon == NULL ) 
	{ 
		//Check if clone, if so try parent icon first 
		if( DriverIsClone(nGameIndex) ) 
		{ 
			hIcon = LoadIconFromFile(drivers[nGameIndex]->clone_of->name); 
			if( hIcon == NULL) 
			{ 
				hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAME32_ICON)); 
			} 
		} 
		else 
		{ 
			hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAME32_ICON)); 
		} 
	} 
	WaitForInputIdle( pi->hProcess, INFINITE ); 
	SendMessageToProcess( pi, WM_SETICON, ICON_SMALL, (LPARAM)hIcon ); 
	SendMessageToProcess( pi, WM_SETICON, ICON_BIG, (LPARAM)hIcon ); 
}

void SendMessageToProcess(LPPROCESS_INFORMATION lpProcessInformation, 
                                      UINT Msg, WPARAM wParam, LPARAM lParam)
{
	FINDWINDOWHANDLE fwhs;
	fwhs.ProcessInfo = lpProcessInformation;
	fwhs.hwndFound  = NULL;

	EnumWindows(EnumWindowCallBack, (LPARAM)&fwhs);

	SendMessage(fwhs.hwndFound, Msg, wParam, lParam);
	//Fix for loosing Focus, we reset the Focus on our own Main window
	SendMessage(fwhs.hwndFound, WM_KILLFOCUS,(WPARAM) hMain, (LPARAM) NULL);
}

static BOOL CALLBACK EnumWindowCallBack(HWND hwnd, LPARAM lParam) 
{ 
	FINDWINDOWHANDLE * pfwhs = (FINDWINDOWHANDLE * )lParam; 
	DWORD ProcessId; 
	char buffer[MAX_PATH]; 

	GetWindowThreadProcessId(hwnd, &ProcessId);

	// cmk--I'm not sure I believe this note is necessary
	// note: In order to make sure we have the MainFrame, verify that the title 
	// has Zero-Length. Under Windows 98, sometimes the Client Window ( which doesn't 
	// have a title ) is enumerated before the MainFrame 

	GetWindowTextA(hwnd, buffer, sizeof(buffer));
	if (ProcessId  == pfwhs->ProcessInfo->dwProcessId &&
		 strncmp(buffer,MAMENAME,strlen(MAMENAME)) == 0) 
	{ 
		pfwhs->hwndFound = hwnd; 
		return FALSE; 
	} 
	else 
	{ 
		// Keep enumerating 
		return TRUE; 
	} 
}

HWND GetGameWindow(LPPROCESS_INFORMATION lpProcessInformation)
{
	FINDWINDOWHANDLE fwhs;
	fwhs.ProcessInfo = lpProcessInformation;
	fwhs.hwndFound  = NULL;

	EnumWindows(EnumWindowCallBack, (LPARAM)&fwhs);
	return fwhs.hwndFound;
}

#endif

#ifdef SMOOTH_SCROLL
// fix warning: cast does not match function type
#if defined(__GNUC__) && defined(ListView_CreateDragImage)
#undef ListView_GetImageList
#define ListView_GetImageList(w,i) (HIMAGELIST)(LRESULT)(int)SendMessage((w),LVM_GETIMAGELIST,(i),0)
#endif

static LPCTSTR MakeShortString(HDC hDC, LPCTSTR lpszLong, int nColumnLen, int nOffset)
{
//	static const CHAR szThreeDots[] = "...";
  const char *szThreeDots = "...";
	static WCHAR szShort[MAX_PATH];
	int nStringLen = lstrlen(lpszLong);
	int nAddLen;
	SIZE size;
	int i;

	GetTextExtentPoint32(hDC, lpszLong, nStringLen, &size);
	if (nStringLen == 0 || size.cx + nOffset <= nColumnLen)
		return lpszLong;

	lstrcpy(szShort, lpszLong);
	GetTextExtentPoint32(hDC, _Unicode(_UI(szThreeDots)), sizeof(_Unicode(_UI(szThreeDots))), &size);
	nAddLen = size.cx;

	for (i = nStringLen - 1; i > 0; i--)
	{
		szShort[i] = 0;
		GetTextExtentPoint32(hDC, szShort, i, &size);
		if (size.cx + nOffset + nAddLen <= nColumnLen)
			break;
	}

	lstrcat(szShort, _Unicode(_UI(szThreeDots)));

	return szShort;
}


/* SmoothScroll_DrawItem for ListView */
static void SmoothScroll_DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	HDC         hDC = lpDrawItemStruct->hDC;
	RECT        rcItem = lpDrawItemStruct->rcItem;
	UINT        uiFlags = ILD_TRANSPARENT;
	HIMAGELIST  hImageList;
	int         nItem = lpDrawItemStruct->itemID;
	COLORREF    clrTextSave = 0;
	COLORREF    clrBkSave = 0;
	COLORREF    clrImage = GetSysColor(COLOR_WINDOW);
	static WCHAR szBuff[MAX_PATH];
	BOOL        bFocus = (GetFocus() == hwndList);
	LPCTSTR     pszText;
	UINT        nStateImageMask;
	BOOL        bSelected;
	LV_COLUMN   lvc;
	LV_ITEM     lvi;
	RECT        rcAllLabels;
	RECT        rcLabel;
	RECT        rcIcon;
	int         offset;
	SIZE        size;
	int         i, j;
	int         nColumn;
	int         nColumnMax = 0;
	int         order[COLUMN_MAX];
	BOOL draw_as_clone;
	int indent_space;
	BOOL         bColourClone;

	nColumnMax = Picker_GetNumColumns(hwndList);

	if (oldControl)
	{
		GetColumnOrder(order);
	}
	else
	{
		/* Get the Column Order and save it */
		ListView_GetColumnOrderArray(hwndList, nColumnMax, order);

		/* Disallow moving column 0 */
		if (order[0] != 0)
		{
			for (i = 0; i < nColumnMax; i++)
			{
				if (order[i] == 0)
				{
					order[i] = order[0];
					order[0] = 0;
				}
			}
			ListView_SetColumnOrderArray(hwndList, nColumnMax, order);
		}
	}

	/* Labels are offset by a certain amount */
	/* This offset is related to the width of a space character */
	GetTextExtentPoint32(hDC, _Unicode(_UI(" ")), 1 , &size);
	offset = size.cx;

	lvi.mask	   = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
	lvi.iItem	   = nItem;
	lvi.iSubItem   = order[0];
	lvi.pszText    = szBuff;
	lvi.cchTextMax = sizeof(szBuff);
	lvi.stateMask  = 0xFFFF;	   /* get all state flags */
	ListView_GetItem(hwndList, &lvi);

	bSelected = ((lvi.state & LVIS_DROPHILITED) || ( (lvi.state & LVIS_SELECTED)
		&& ((bFocus) || (GetWindowLong(hwndList, GWL_STYLE) & LVS_SHOWSELALWAYS))));

	/* figure out if we indent and draw grayed */
	draw_as_clone = (GetViewMode() == VIEW_GROUPED && DriverIsClone(lvi.lParam));
	bColourClone = draw_as_clone;

	if (GetOffsetClones() && draw_as_clone)
	{
		BOOL	bParentFound = FALSE;
#if 0

	/* only indent if parent is also in this view */
		for( i=0;i<ListView_GetItemCount(hwndList);i++)
		{
			WCHAR cDesc[256];
			char *pDesc2 = NULL;

			ListView_GetItemText(hwndList,i,0,cDesc, 255 );
			pDesc2 = ModifyThe(drivers[lvi.lParam]->clone_of->description);
			if( strcmp(pDesc2, cDesc ) == 0 )
			{
				bParentFound = TRUE;
				break;
			}
		}
#else
		LPTREEFOLDER lpFolder = GetSelectedFolder();
		if (lpFolder)
		{
			int parent_index = GetGameNameIndex(drivers[lvi.lParam]->clone_of->name);

			if (parent_index != -1 &&
			    TestBit(lpFolder->m_lpGameBits, parent_index) &&
			    !GameFiltered(parent_index, lpFolder->m_dwFlags))
				bParentFound = TRUE;
	}
#endif

		if (!bParentFound)
		{
			/*Reset it, as no Parent is there*/
			draw_as_clone = FALSE;
		}
	}

	ListView_GetItemRect(hwndList, nItem, &rcAllLabels, LVIR_BOUNDS);

	ListView_GetItemRect(hwndList, nItem, &rcLabel, LVIR_LABEL);
	rcAllLabels.left = rcLabel.left;

	if (hBackground != NULL)
	{
		RECT		rcClient;
		HRGN		rgnBitmap;
		RECT		rcTmpBmp = rcItem;
		RECT		rcFirstItem;
		HPALETTE	hPAL;
		HDC 		htempDC;
		HBITMAP 	oldBitmap;

		htempDC = CreateCompatibleDC(hDC);

		oldBitmap = SelectObject(htempDC, hBackground);

		GetClientRect(hwndList, &rcClient);
		rcTmpBmp.right = rcClient.right;
		/* We also need to check whether it is the last item
		   The update region has to be extended to the bottom if it is */
		if (nItem == ListView_GetItemCount(hwndList) - 1)
			rcTmpBmp.bottom = rcClient.bottom;

		rgnBitmap = CreateRectRgnIndirect(&rcTmpBmp);
		SelectClipRgn(hDC, rgnBitmap);
		DeleteObject(rgnBitmap);

		hPAL = GetBackgroundPalette();
		if (hPAL == NULL)
			hPAL = CreateHalftonePalette(hDC);

		if (GetDeviceCaps(htempDC, RASTERCAPS) & RC_PALETTE && hPAL != NULL)
		{
			SelectPalette(htempDC, hPAL, FALSE);
			RealizePalette(htempDC);
		}

		ListView_GetItemRect(hwndList, 0, &rcFirstItem, LVIR_BOUNDS);

		for (i = rcFirstItem.left; i < rcClient.right; i += bmDesc.bmWidth)
			for (j = rcFirstItem.top; j < rcClient.bottom; j += bmDesc.bmHeight)
				BitBlt(hDC, i, j, bmDesc.bmWidth, bmDesc.bmHeight, htempDC, 0, 0, SRCCOPY);

		SelectObject(htempDC, oldBitmap);
		DeleteDC(htempDC);

		if (GetBackgroundPalette() == NULL)
		{
			DeleteObject(hPAL);
			hPAL = NULL;
		}
	}

	indent_space = 0;

	if (draw_as_clone)
	{
		RECT rect;
		ListView_GetItemRect(hwndList, nItem, &rect, LVIR_ICON);

		/* indent width of icon + the space between the icon and text
		 * so left of clone icon starts at text of parent
         */
		indent_space = rect.right - rect.left + offset;
	}

	rcAllLabels.left += indent_space;

	if (bSelected)
	{
		HBRUSH hBrush;
		HBRUSH hOldBrush;

		if (bFocus)
		{
			clrTextSave = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			clrBkSave	= SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
			hBrush		= CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
		}
		else
		{
			clrTextSave = SetTextColor(hDC, GetSysColor(COLOR_BTNTEXT));
			clrBkSave	= SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
			hBrush		= CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
		}

		hOldBrush = SelectObject(hDC, hBrush);
		FillRect(hDC, &rcAllLabels, hBrush);
		SelectObject(hDC, hOldBrush);
		DeleteObject(hBrush);
	}
	else
	{
		if (hBackground == NULL)
		{
			HBRUSH hBrush;
			
			hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
			FillRect(hDC, &rcAllLabels, hBrush);
			DeleteObject(hBrush);
		}
		
		if (bColourClone)
				clrTextSave = SetTextColor(hDC,GetListCloneColor());
			else
				clrTextSave = SetTextColor(hDC,GetListFontColor());

		clrBkSave = SetBkColor(hDC,GetSysColor(COLOR_WINDOW));
	}


	if (lvi.state & LVIS_CUT)
	{
		clrImage = GetSysColor(COLOR_WINDOW);
		uiFlags |= ILD_BLEND50;
	}
	else
	if (bSelected)
	{
		if (bFocus)
			clrImage = GetSysColor(COLOR_HIGHLIGHT);
		else
			clrImage = GetSysColor(COLOR_BTNFACE);

		uiFlags |= ILD_BLEND50;
	}

	nStateImageMask = lvi.state & LVIS_STATEIMAGEMASK;

	if (nStateImageMask)
	{
		int nImage = (nStateImageMask >> 12) - 1;
		hImageList = ListView_GetImageList(hwndList, LVSIL_STATE);
		if (hImageList)
			ImageList_Draw(hImageList, nImage, hDC, rcItem.left, rcItem.top, ILD_TRANSPARENT);
	}

	ListView_GetItemRect(hwndList, nItem, &rcIcon, LVIR_ICON);

	rcIcon.left += indent_space;

	ListView_GetItemRect(hwndList, nItem, &rcItem, LVIR_LABEL);

	hImageList = ListView_GetImageList(hwndList, LVSIL_SMALL);
	if (hImageList)
	{
		UINT nOvlImageMask = lvi.state & LVIS_OVERLAYMASK;
		if (rcIcon.left + 16 + indent_space < rcItem.right)
		{
			ImageList_DrawEx(hImageList, lvi.iImage, hDC, rcIcon.left, rcIcon.top, 16, 16,
							 GetSysColor(COLOR_WINDOW), clrImage, uiFlags | nOvlImageMask);
		}
	}

	ListView_GetItemRect(hwndList, nItem, &rcItem, LVIR_LABEL);

	pszText = MakeShortString(hDC, szBuff, rcItem.right - rcItem.left, 2*offset + indent_space);

	rcLabel = rcItem;
	rcLabel.left  += offset + indent_space;
	rcLabel.right -= offset;

	DrawText(hDC, pszText, -1, &rcLabel,
			 DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

	for (nColumn = 1; nColumn < nColumnMax; nColumn++)
	{
		int 	nRetLen;
		UINT	nJustify;
		LV_ITEM lvItem;

		lvc.mask = LVCF_FMT | LVCF_WIDTH;
		ListView_GetColumn(hwndList, order[nColumn], &lvc);

		lvItem.mask 	  = LVIF_TEXT;
		lvItem.iItem	  = nItem;
		lvItem.iSubItem   = order[nColumn];
		lvItem.pszText	  = szBuff;
		lvItem.cchTextMax = sizeof(szBuff);

		if (ListView_GetItem(hwndList, &lvItem) == FALSE)
			continue;

		rcItem.left   = rcItem.right;
		rcItem.right += lvc.cx;
#define BUFF
#ifdef BUFF		
#undef static WCHAR szBuff[MAX_PATH]
    static szBuff[MAX_PATH];
    char szBuff
		nRetLen = strlen(szBuff);
		if (nRetLen == 0)
			continue;
#endif
		pszText = MakeShortString(hDC, szBuff, rcItem.right - rcItem.left, 2 * offset);

		nJustify = DT_LEFT;

		if (pszText == szBuff)
		{
			switch (lvc.fmt & LVCFMT_JUSTIFYMASK)
			{
			case LVCFMT_RIGHT:
				nJustify = DT_RIGHT;
				break;

			case LVCFMT_CENTER:
				nJustify = DT_CENTER;
				break;

			default:
				break;
			}
		}

		rcLabel = rcItem;
		rcLabel.left  += offset;
		rcLabel.right -= offset;
		DrawText(hDC, pszText, -1, &rcLabel,
				 nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);
	}

	if (lvi.state & LVIS_FOCUSED && bFocus)
		DrawFocusRect(hDC, &rcAllLabels);

	SetTextColor(hDC, clrTextSave);
	SetBkColor(hDC, clrBkSave);
}
#endif /* SMOOTH_SCROLL */

#ifdef KAILLERA
extern unsigned int crc32(unsigned int crc, const UINT8 *buf, unsigned int len);
static int checksum_file2(const char* file, unsigned char **p, unsigned int *size, unsigned int *crc)
{
	int length;
	unsigned char *data;
	FILE* f;

	f = fopen(file,"rb");
	if (!f)
	{
		return -1;
	}

	/* determine length of file */
	if (fseek (f, 0L, SEEK_END) != 0)
	{
		fclose(f);
		return -1;
	}

	length = ftell(f);
	if (length == -1L)
	{
		fclose(f);
		return -1;
	}

	/* allocate space for entire file */
	data = (unsigned char*)malloc(length);
	if (!data)
	{
		fclose(f);
		return -1;
	}

	/* read entire file into memory */
	if (fseek(f, 0L, SEEK_SET) != 0)
	{
		free(data);
		fclose(f);
		return -1;
	}

	if (fread(data, sizeof (unsigned char), length, f) != (size_t)length)
	{
		free(data);
		fclose(f);
		return -1;
	}

	*size = length;
	*crc = crc32(0L, data, length);
	if (p)
		*p = data;
	else
		free(data);

	fclose(f);

	return 0;
}

unsigned int fgetsavecrc(const char *gamename, const char *filename)
{
	char name[FILENAME_MAX];
		unsigned int size;
	UINT32 crc;

	sprintf(name, "%s/%s.st%s", GetStateDir(), gamename, filename);

	if(checksum_file2(name, NULL, &size, &crc)) return 0;

	return crc;
}

int fmovestate(const char *gamename, const char *src, const char *dest)
{
	char name_dest[FILENAME_MAX];
	char name_src[FILENAME_MAX];

	sprintf(name_dest, "%s/%s.st%s", GetStateDir(), gamename, dest);
	DeleteFileA(name_dest);

	sprintf(name_src, "%s/%s.st%s", GetStateDir(), gamename, src);
	MoveFileA(name_src, name_dest);

	return 0;
}

HFONT GetKailleraChatFont(void)
{
	return hChatFont;
}
#endif /* KAILLERA */

#ifdef MAME_AVI

#include <math.h>

static struct MAME_AVI_STATUS AviStatus;

void AviDialogProcRefresh(HWND hDlg)
{
	// '프레임 압축'에 체크된 경우
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP)) &&
		AviStatus.depth == 16	)
	{
		Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP_PRE), TRUE);
		Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP_FEW), TRUE);
	}
	else
	{
		Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP_PRE), FALSE);
		Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP_FEW), FALSE);
	}

	// '16bit -> 24bit변환'에 체크된 경우
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24)))
	{
		Button_Enable(GetDlgItem(hDlg, IDC_INTERLACE), TRUE);
	} else
	{
		Button_Enable(GetDlgItem(hDlg, IDC_INTERLACE), FALSE);
	}

	// '16bit->24bit변환' 과 '인터레이스드 방식'에 체크가 된 경우
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_INTERLACE)) &&
		Button_GetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24))	)
	{	
		Button_Enable(GetDlgItem(hDlg, IDC_INTERLACE_ODD),	 TRUE);
		Button_Enable(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_X), TRUE);
		Button_Enable(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_Y), TRUE);

		Button_Enable(GetDlgItem(hDlg, IDC_AVISIZE_WIDTH),		TRUE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVISIZE_HEIGHT),		TRUE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVI_LEFT),			TRUE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVI_WIDTH),			TRUE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVI_TOP),			TRUE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVI_HEIGHT),			TRUE);
	
		Button_Enable(GetDlgItem(hDlg, IDC_AVI_POS_CENTER),			TRUE);
		Static_SetTextA(GetDlgItem(hDlg, IDC_TEXT_FPS_DIV2),        "/2");
		Static_SetTextA(GetDlgItem(hDlg, IDC_TEXT_AVI_TOP_MUL2),    "x2");
		Static_SetTextA(GetDlgItem(hDlg, IDC_TEXT_AVI_HEIGHT_MUL2), "x2");
	}
	else
	{
		Button_Enable(GetDlgItem(hDlg, IDC_INTERLACE_ODD), FALSE);
		Button_Enable(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_X), FALSE);
		Button_Enable(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_Y), FALSE);

		Button_Enable(GetDlgItem(hDlg, IDC_AVISIZE_WIDTH),		FALSE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVISIZE_HEIGHT),		FALSE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVI_LEFT),			FALSE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVI_WIDTH),			FALSE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVI_TOP),			FALSE);
		Button_Enable(GetDlgItem(hDlg, IDC_AVI_HEIGHT),			FALSE);

		Button_Enable(GetDlgItem(hDlg, IDC_AVI_POS_CENTER),			FALSE);
		Static_SetTextA(GetDlgItem(hDlg, IDC_TEXT_FPS_DIV2),        "");
		Static_SetTextA(GetDlgItem(hDlg, IDC_TEXT_AVI_TOP_MUL2),    "");
		Static_SetTextA(GetDlgItem(hDlg, IDC_TEXT_AVI_HEIGHT_MUL2), "");
	}
}

INT_PTR CALLBACK AviDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{

    switch (Msg)
    {
    case WM_INITDIALOG:
		// 다이얼로그 초기화
		{
			char buf[64];

			HWND hCtrl = GetDlgItem(hDlg, IDC_AVIFRAMESKIP);
			AviStatus = (*(GetAviStatus()));
			if (hCtrl)
			{
				int i;
				char fmt[64];
#ifdef KOREAN
				const char *szFmtStr = "%d/12 프레임 생략";
				ComboBox_AddString(hCtrl, "프레임 생략 안함  @");
#else
				const char *szFmtStr = "Skip %d of 12 frames";
				ComboBox_AddString(hCtrl, "Draw every frame  @");
#endif

				for (i = 1; i < 12; i++)
				{
					sprintf( fmt, szFmtStr, i );
					if (i==6 || i==8 || i==9)
						sprintf(buf, "%s @", fmt);
					else
						sprintf(buf, "%s", fmt);

					ComboBox_AddString(hCtrl, buf);
				}
				ComboBox_SetCurSel(hCtrl, AviStatus.frame_skip);
			}

			hCtrl = GetDlgItem(hDlg, IDC_FPS);
			if (hCtrl)
			{
				sprintf(buf, "%5.6f", AviStatus.fps);
				ComboBox_AddString(hCtrl, buf);

				ComboBox_AddString(hCtrl, "60");
				ComboBox_AddString(hCtrl, "59.94");
				ComboBox_AddString(hCtrl, "53.333333");
				ComboBox_AddString(hCtrl, "48");
				ComboBox_AddString(hCtrl, "40");
				ComboBox_AddString(hCtrl, "30");
				ComboBox_AddString(hCtrl, "29.97");
				ComboBox_AddString(hCtrl, "24");
				ComboBox_AddString(hCtrl, "20");
				ComboBox_AddString(hCtrl, "15");
				ComboBox_AddString(hCtrl, "12");
				ComboBox_AddString(hCtrl, "10");
				ComboBox_SetCurSel(hCtrl, 0);

				ComboBox_LimitText(hCtrl, 10);
				Edit_SetTextA(hCtrl, buf);

			}

			hCtrl = GetDlgItem(hDlg, IDC_AVISIZE_WIDTH);
			if (hCtrl)
			{
				sprintf(buf, "%u", AviStatus.avi_width);
				ComboBox_AddString(hCtrl, buf);
				ComboBox_AddString(hCtrl, "720");
				ComboBox_AddString(hCtrl, "640");
				ComboBox_AddString(hCtrl, "512");
				ComboBox_AddString(hCtrl, "480");
				ComboBox_AddString(hCtrl, "384");
				ComboBox_AddString(hCtrl, "352");
				ComboBox_AddString(hCtrl, "320");
				ComboBox_AddString(hCtrl, "304");
				ComboBox_AddString(hCtrl, "256");
				ComboBox_AddString(hCtrl, "240");
				ComboBox_SetCurSel(hCtrl, 0);

				ComboBox_LimitText(hCtrl, 5);
				sprintf(buf, "%u", AviStatus.avi_width);
				Edit_SetTextA(hCtrl, buf);
			}

			hCtrl = GetDlgItem(hDlg, IDC_AVISIZE_HEIGHT);
			if (hCtrl)
			{
				char buf[32];
				sprintf(buf, "%u", AviStatus.avi_height);
				ComboBox_AddString(hCtrl, buf);
				ComboBox_AddString(hCtrl, "480");
				ComboBox_AddString(hCtrl, "384");
				ComboBox_AddString(hCtrl, "320");
				ComboBox_AddString(hCtrl, "240");
				ComboBox_AddString(hCtrl, "232");
				ComboBox_AddString(hCtrl, "224");
				ComboBox_SetCurSel(hCtrl, 0);

				ComboBox_LimitText(hCtrl, 5);
				sprintf(buf, "%u", AviStatus.avi_height);
				Edit_SetTextA(hCtrl, buf);
			}

			sprintf(buf, "%lu", AviStatus.avi_rect.m_Left);
			Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_LEFT),   buf);
			sprintf(buf, "%lu", AviStatus.avi_rect.m_Top);
			Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_TOP),    buf);
			sprintf(buf, "%lu", AviStatus.avi_rect.m_Width);
			Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_WIDTH),  buf);
			sprintf(buf, "%lu", AviStatus.avi_rect.m_Height);
			Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_HEIGHT), buf);
			
			hCtrl = GetDlgItem(hDlg, IDC_AVI_FILESIZE);
			if (hCtrl)
			{
				char buf[32];
				sprintf(buf, "%u", AviStatus.avi_filesize);
				ComboBox_AddString(hCtrl, buf);
				ComboBox_AddString(hCtrl, "2000");
				ComboBox_AddString(hCtrl, "1500");
				ComboBox_AddString(hCtrl, "1000");
				ComboBox_AddString(hCtrl, "500");
				ComboBox_AddString(hCtrl, "100");
				ComboBox_SetCurSel(hCtrl, 0);

				ComboBox_LimitText(hCtrl, 7);
				sprintf(buf, "%u", AviStatus.avi_filesize);
				Edit_SetTextA(hCtrl, buf);
			}
			hCtrl = GetDlgItem(hDlg, IDC_AVI_FILESIZE_CHECK_FRAME);
			if (hCtrl)
			{
				char buf[32];
				ComboBox_AddString(hCtrl, "60");
				ComboBox_AddString(hCtrl, "30");
				ComboBox_AddString(hCtrl, "20");
				ComboBox_AddString(hCtrl, "15");
				ComboBox_AddString(hCtrl, "12");
				ComboBox_AddString(hCtrl, "10");
				ComboBox_AddString(hCtrl, "6");
				ComboBox_AddString(hCtrl, "5");
				ComboBox_AddString(hCtrl, "4");
				ComboBox_AddString(hCtrl, "3");
				ComboBox_AddString(hCtrl, "2");
				ComboBox_AddString(hCtrl, "1");
				ComboBox_SetCurSel(hCtrl, 0);

				ComboBox_LimitText(hCtrl, 11);
				sprintf(buf, "%u", AviStatus.avi_filesizecheck_frame);
				Edit_SetTextA(hCtrl, buf);
			}

			hCtrl = GetDlgItem(hDlg, IDC_AUDIO_RECORD_TYPE);
			if (hCtrl)
			{
#ifdef KOREAN
				ComboBox_AddString(hCtrl, "녹음하지 않음 ");
				ComboBox_AddString(hCtrl, "WAVE파일에 녹음");
				ComboBox_AddString(hCtrl, "AVI파일에 녹음");
#else
				ComboBox_AddString(hCtrl, "Do not record sound");
				ComboBox_AddString(hCtrl, "Record as WAV file");
				ComboBox_AddString(hCtrl, "Record to AVI");
#endif
				ComboBox_SetCurSel(hCtrl, AviStatus.avi_audio_record_type);
			}

			Edit_LimitText(	GetDlgItem(hDlg, IDC_HOUR),   3);
			Edit_LimitText(	GetDlgItem(hDlg, IDC_MINUTE), 3);
			Edit_LimitText(	GetDlgItem(hDlg, IDC_SECOND), 3);

			{
				char buf[32];

				sprintf(buf, "%d x %d x %d bit", AviStatus.width, AviStatus.height, AviStatus.depth);
				Static_SetTextA(GetDlgItem(hDlg, IDC_BITMAP_SIZE),        buf);

				if (AviStatus.audio_type == 0)
				{
#ifdef KOREAN
					Static_SetText(GetDlgItem(hDlg, IDC_AUDIO_SRC_FORMAT),	"무음");
#else
					Static_SetTextA(GetDlgItem(hDlg, IDC_AUDIO_SRC_FORMAT),	"No sound");
#endif
					Static_SetTextA(GetDlgItem(hDlg, IDC_AUDIO_DEST_FORMAT),	"");

					Button_Enable(GetDlgItem(hDlg, IDC_AUDIO_RECORD_TYPE), FALSE);

				}else
				{
#ifdef KOREAN
					const char *lpszStereo = "스테레오";
					const char *lpszMono   = "모노";
#else
					const char *lpszStereo = "Stereo";
					const char *lpszMono   = "Mono";
#endif					
					sprintf(buf, "%uHz %ubit %s", AviStatus.audio_samples_per_sec, AviStatus.audio_bitrate, (AviStatus.audio_channel == 2) ? lpszStereo:lpszMono);
					Static_SetTextA(GetDlgItem(hDlg, IDC_AUDIO_SRC_FORMAT),        buf);
					sprintf(buf, "%uHz %ubit %s", AviStatus.avi_audio_samples_per_sec, AviStatus.avi_audio_bitrate, (AviStatus.avi_audio_channel == 2) ? lpszStereo:lpszMono);
					Static_SetTextA(GetDlgItem(hDlg, IDC_AUDIO_DEST_FORMAT),        buf);
					Button_Enable(GetDlgItem(hDlg, IDC_AUDIO_RECORD_TYPE), TRUE);
				}


				sprintf(buf, "%d", AviStatus.hour);
				Edit_SetTextA(	GetDlgItem(hDlg, IDC_HOUR), buf);
				sprintf(buf, "%d", AviStatus.minute);
				Edit_SetTextA(	GetDlgItem(hDlg, IDC_MINUTE), buf);
				sprintf(buf, "%d", AviStatus.second);
				Edit_SetTextA(	GetDlgItem(hDlg, IDC_SECOND), buf);
			}
			

			Button_SetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP),      AviStatus.frame_cmp);
			Button_SetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP_PRE),  AviStatus.frame_cmp_pre15);
			Button_SetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP_FEW),  AviStatus.frame_cmp_few);
			Button_SetCheck(GetDlgItem(hDlg, IDC_WAVE_RECORD),    AviStatus.wave_record);
			Button_SetCheck(GetDlgItem(hDlg, IDC_INTERLACE),      AviStatus.interlace);
			Button_SetCheck(GetDlgItem(hDlg, IDC_INTERLACE_ODD),	AviStatus.interlace_odd_number_field);

			Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_FORCEFLIPY),     AviStatus.force_flip_y);

			Button_SetCheck(GetDlgItem(hDlg, IDC_AVI_SAVEFILE_PAUSE),	AviStatus.avi_savefile_pause);
			Button_SetCheck(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_X),		AviStatus.avi_smooth_resize_x);
			Button_SetCheck(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_Y),		AviStatus.avi_smooth_resize_y);
			
			Button_SetCheck(GetDlgItem(hDlg, IDC_AUDIO_16BIT),    (AviStatus.avi_audio_bitrate>8) ? TRUE:FALSE);
			Button_SetCheck(GetDlgItem(hDlg, IDC_AUDIO_STEREO),   (AviStatus.avi_audio_channel==2) ? TRUE:FALSE);

	
			if (AviStatus.depth == 16)
			{
				if (AviStatus.avi_depth == 8)	Button_SetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO8),TRUE);
				else							Button_SetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO8),FALSE);
				if (AviStatus.avi_depth == 24)	Button_SetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24),TRUE);
				else							Button_SetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24),FALSE);
			
				Button_Enable(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO8), TRUE);
				Button_Enable(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24), TRUE);

				Button_Enable(GetDlgItem(hDlg, IDC_INTERLACE), TRUE);
				Button_Enable(GetDlgItem(hDlg, IDC_SET_TV_DISPLAY_SETTING), TRUE);
				

			} else
			{
				Button_SetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO8),FALSE);
				Button_SetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24),FALSE);
				Button_Enable(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO8), FALSE);
				Button_Enable(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24), FALSE);
				Button_Enable(GetDlgItem(hDlg, IDC_INTERLACE), FALSE);
				Button_Enable(GetDlgItem(hDlg, IDC_SET_TV_DISPLAY_SETTING), FALSE);
			}
			

			if (AviStatus.depth == 16)
			{
				Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP), TRUE);
				Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP_PRE), TRUE);
				Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP_FEW), TRUE);
			}else 
			{
				Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP), FALSE);
				Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP_PRE), FALSE);
				Button_Enable(GetDlgItem(hDlg, IDC_FRAME_CMP_FEW), FALSE);
			}

			Edit_LimitText(GetDlgItem(hDlg, IDC_LEFT), 4);
			Edit_SetTextA(GetDlgItem(hDlg, IDC_LEFT),	"0");
			SendDlgItemMessage(hDlg, IDC_LEFT_SPIN, UDM_SETRANGE, 0,
										(LPARAM)MAKELONG(AviStatus.width-1, 0));
		    SendDlgItemMessage(hDlg, IDC_LEFT_SPIN, UDM_SETPOS, 0,
										(LPARAM)MAKELONG(AviStatus.rect.m_Left, 0));

			Edit_LimitText(GetDlgItem(hDlg, IDC_TOP), 4);
			Edit_SetTextA(GetDlgItem(hDlg, IDC_TOP),	"0");
			SendDlgItemMessage(hDlg, IDC_TOP_SPIN, UDM_SETRANGE, 0,
										(LPARAM)MAKELONG(AviStatus.height-1, 0));
		    SendDlgItemMessage(hDlg, IDC_TOP_SPIN, UDM_SETPOS, 0,
										(LPARAM)MAKELONG(AviStatus.rect.m_Top, 0));

			Edit_LimitText(GetDlgItem(hDlg, IDC_WIDTH), 4);
			SendDlgItemMessage(hDlg, IDC_WIDTH_SPIN, UDM_SETRANGE, 0,
										(LPARAM)MAKELONG(AviStatus.width, 1));
		    SendDlgItemMessage(hDlg, IDC_WIDTH_SPIN, UDM_SETPOS, 0,
										(LPARAM)MAKELONG(AviStatus.rect.m_Width, 0));

			Edit_LimitText(GetDlgItem(hDlg, IDC_HEIGHT), 4);
			SendDlgItemMessage(hDlg, IDC_HEIGHT_SPIN, UDM_SETRANGE, 0,
										(LPARAM)MAKELONG(AviStatus.height, 1));
		    SendDlgItemMessage(hDlg, IDC_HEIGHT_SPIN, UDM_SETPOS, 0,
										(LPARAM)MAKELONG(AviStatus.rect.m_Height, 0));


		}

		AviDialogProcRefresh(hDlg);
        return TRUE;

    case WM_HELP:
        break;

    case WM_CONTEXTMENU:
        break;

    case WM_COMMAND :
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
		case IDC_FRAME_CMP:
			AviDialogProcRefresh(hDlg);
			break;
		case IDC_FRAME_CMP_PRE:
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP_PRE)))
				Button_SetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP_FEW),      FALSE);
			break;
		case IDC_FRAME_CMP_FEW:
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP_FEW)))
				Button_SetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP_PRE),      FALSE);
			break;
		case IDC_INTERLACE:
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_INTERLACE)) == TRUE)
			{	
				char buf[32];
				sprintf(buf, "%lu", AviStatus.rect.m_Height*2);
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVISIZE_HEIGHT),        buf);
			}
			else
			{
				char buf[32];
				sprintf(buf, "%lu", AviStatus.rect.m_Height);
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVISIZE_HEIGHT),        buf);
			}
			AviDialogProcRefresh(hDlg);
			break;
		case IDC_COLOR_CNV_16TO8:
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO8)))
				Button_SetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24),      FALSE);
			AviDialogProcRefresh(hDlg);
			break;
		case IDC_COLOR_CNV_16TO24:
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24)))
			{
				Button_SetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO8),      FALSE);
			}
			AviDialogProcRefresh(hDlg);
			break;
		case IDC_SET_TV_DISPLAY_SETTING:
			{
				char buf[100];
				unsigned int x,y,width,height;
				unsigned int width_src,height_src;

				Button_SetCheck(GetDlgItem(hDlg, IDC_INTERLACE),			TRUE);
				Button_SetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24),		TRUE);
				Button_SetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP),			FALSE);
				Button_SetCheck(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_Y),		TRUE);

				Edit_SetTextA(GetDlgItem(hDlg, IDC_FPS),					"59.94");
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVISIZE_WIDTH),		"720");
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVISIZE_HEIGHT),		"480");


				Edit_GetTextA(GetDlgItem(hDlg, IDC_WIDTH), buf, 100);
				sscanf(buf,"%u", &width);	if (width == 0)		width = AviStatus.rect.m_Width;
				Edit_GetTextA(GetDlgItem(hDlg, IDC_HEIGHT), buf, 100);
				sscanf(buf,"%u", &height);	if (height == 0)	height = AviStatus.rect.m_Height;

				width_src = width; 
				height_src = height;

				width *= 2;
				if (height > 240) {height = 240; width = 480*0.75;}
				y = (240 - height)/2;
				x = (720 - width)/2;
				if (width > 670) {width = 670; x = 22;}

				sprintf(buf, "%u", x);
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_LEFT),        buf);
				sprintf(buf, "%u", y);
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_TOP),        buf);
				sprintf(buf, "%u", width);
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_WIDTH),        buf);	
				sprintf(buf, "%u", height);
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_HEIGHT),        buf);
				
				if (width % width_src)		AviStatus.avi_smooth_resize_x = TRUE;
				else						AviStatus.avi_smooth_resize_x = FALSE;
				if (height % height_src)	AviStatus.avi_smooth_resize_y = TRUE;
				else						AviStatus.avi_smooth_resize_y = FALSE;

				Button_SetCheck(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_X),		AviStatus.avi_smooth_resize_x);
				Button_SetCheck(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_Y),		AviStatus.avi_smooth_resize_y);
				
				ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_AVIFRAMESKIP), 0);

			}
			AviDialogProcRefresh(hDlg);
			break;
		case IDC_AVI_POS_CENTER:
			{
				char buf[100];
				int x,y,width,height;
				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVI_WIDTH), buf, 100);
				sscanf(buf,"%u", &width);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVI_HEIGHT), buf, 100);
				sscanf(buf,"%u", &height);

				y = (240 - height)/2;
				x = (720 - width)/2;

				if (x<0) x=0;
				if (y<0) y=0;

				sprintf(buf, "%u", x);
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_LEFT),        buf);
				sprintf(buf, "%u", y);
				Edit_SetTextA(GetDlgItem(hDlg, IDC_AVI_TOP),        buf);
			}
			break;
			
        case IDOK :
			{
				char buf[100];

				Edit_GetTextA(GetDlgItem(hDlg, IDC_FPS), buf, 100);

				{
					int i,j;
					int di,df;
					int di2,df2,j2;

					AviStatus.fps = 0;
					j=-1;
					for(i=0; i<100; i++)
					{
						if (buf[i] == 0 ) break;
						if (buf[i] == '.')
						{
							buf[i] = 0;
							j=i+1;
							break;
						}
					}
					sscanf(buf,"%d", &di);
					df = 0;
					if (j!=-1) sscanf(&buf[j],"%d", &df);
					j=strlen(&buf[j]);
					AviStatus.fps = (double)df / pow(10,j);
					AviStatus.fps += (double)di;

									
					sprintf(buf, "%5.6f", AviStatus.def_fps);
					j2=-1;
					for(i=0; i<100; i++)
					{
						if (buf[i] == 0 ) break;
						if (buf[i] == '.')
						{
							buf[i] = 0;
							j2=i+1;
							break;
						}
					}
					sscanf(buf,"%d", &di2);
					df2 = 0;
					if (j2!=-1) sscanf(&buf[j2],"%d", &df2);
					j2=strlen(&buf[j2]);
					if ( AviStatus.fps == (double)df2 / pow(10,j2) + (double)di2 ) AviStatus.fps = AviStatus.def_fps;

				}

				if (AviStatus.fps <= 0) AviStatus.fps = AviStatus.def_fps;

				AviStatus.frame_skip = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_AVIFRAMESKIP));

				AviStatus.frame_cmp       = Button_GetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP));
				AviStatus.frame_cmp_pre15 = Button_GetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP_PRE));
				AviStatus.frame_cmp_few   = Button_GetCheck(GetDlgItem(hDlg, IDC_FRAME_CMP_FEW));
				AviStatus.wave_record     = Button_GetCheck(GetDlgItem(hDlg, IDC_WAVE_RECORD));
				AviStatus.interlace       = Button_GetCheck(GetDlgItem(hDlg, IDC_INTERLACE));
				AviStatus.interlace_odd_number_field = Button_GetCheck(GetDlgItem(hDlg, IDC_INTERLACE_ODD));

				AviStatus.force_flip_y    = Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_FORCEFLIPY));
				
				AviStatus.avi_savefile_pause  = Button_GetCheck(GetDlgItem(hDlg, IDC_AVI_SAVEFILE_PAUSE));
				AviStatus.avi_smooth_resize_x = Button_GetCheck(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_X));
				AviStatus.avi_smooth_resize_y = Button_GetCheck(GetDlgItem(hDlg, IDC_SMOOTH_RESIZE_Y));

				AviStatus.avi_audio_record_type = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_AUDIO_RECORD_TYPE));
				AviStatus.avi_audio_bitrate = (Button_GetCheck(GetDlgItem(hDlg, IDC_AUDIO_16BIT))==TRUE) ? 16:8;
				AviStatus.avi_audio_channel = (Button_GetCheck(GetDlgItem(hDlg, IDC_AUDIO_STEREO))==TRUE) ? 2:1;

				AviStatus.bmp_16to8_cnv = Button_GetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO8));
				AviStatus.avi_depth = AviStatus.depth;
				if (AviStatus.depth == 16)
				{
					if (Button_GetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO8)) == TRUE) AviStatus.avi_depth = 8;
					if (Button_GetCheck(GetDlgItem(hDlg, IDC_COLOR_CNV_16TO24)) == TRUE) AviStatus.avi_depth = 24;
				}


				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVI_FILESIZE), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%d", &AviStatus.avi_filesize);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVI_FILESIZE_CHECK_FRAME), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%d", &AviStatus.avi_filesizecheck_frame);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVISIZE_WIDTH), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%d", &AviStatus.avi_width);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVISIZE_HEIGHT), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%d", &AviStatus.avi_height);

				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVI_LEFT), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%lu", &AviStatus.avi_rect.m_Left);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVI_TOP), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%lu", &AviStatus.avi_rect.m_Top);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVI_WIDTH), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%lu", &AviStatus.avi_rect.m_Width);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_AVI_HEIGHT), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%lu", &AviStatus.avi_rect.m_Height);

				Edit_GetTextA(GetDlgItem(hDlg, IDC_LEFT), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%lu", &AviStatus.rect.m_Left);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_TOP), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%lu", &AviStatus.rect.m_Top);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_WIDTH), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%lu", &AviStatus.rect.m_Width);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_HEIGHT), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%lu", &AviStatus.rect.m_Height);
				
				Edit_GetTextA(GetDlgItem(hDlg, IDC_HOUR), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%d", &AviStatus.hour);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_MINUTE), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%d", &AviStatus.minute);
				Edit_GetTextA(GetDlgItem(hDlg, IDC_SECOND), buf, 100);
				if (buf[0] != 0)	sscanf(buf,"%d", &AviStatus.second);

				if (AviStatus.rect.m_Width	> AviStatus.width)	AviStatus.rect.m_Width	= AviStatus.width;
				if (AviStatus.rect.m_Height	> AviStatus.height)	AviStatus.rect.m_Height	= AviStatus.height;
				if (AviStatus.rect.m_Left+AviStatus.rect.m_Width	> AviStatus.width)	AviStatus.rect.m_Left	= AviStatus.width - AviStatus.rect.m_Width;
				if (AviStatus.rect.m_Top+AviStatus.rect.m_Height	> AviStatus.height) AviStatus.rect.m_Top	= AviStatus.height - AviStatus.rect.m_Height;

				SetAviStatus(&AviStatus);
			}
            /* Fall through */

			EndDialog(hDlg, 1);
			return TRUE;

        case IDCANCEL :
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;
    }
    return 0;
}


static void set_mame_mixer_wfm(int drvindex)
{
	extern int mame_mixer_wave_cnvnmb;
	extern struct WAV_WAVEFORMAT		mame_mixer_dstwfm, mame_mixer_srcwfm;
	extern struct WAV_SAMPLES_RESIZE	mame_mixer_wsr;
	//extern struct WAV_SAMPLES_RESAMPLE	mame_mixer_wsre;

	struct InternalMachineDriver drv;
    expand_machine_driver(drivers[drvindex]->drv,&drv);

	//mame_mixer_srcwfm.samplespersec = get_fixedsample_rate(drivers[drvindex]->name, playing_game_options.sample_rate);
	mame_mixer_srcwfm.samplespersec = playing_game_options.samplerate;
	//mame_mixer_srcwfm.channel = (drv.sound_attributes & SOUND_SUPPORTS_STEREO) ? 2:1;
	mame_mixer_srcwfm.channel = 2;		// changed from 0.93 (Aaron's big sound system change) - DarkCoder
	mame_mixer_srcwfm.bitrate = 16;

	mame_mixer_dstwfm = mame_mixer_srcwfm;

	mame_mixer_wave_cnvnmb = wav_convert_select(&mame_mixer_dstwfm, &mame_mixer_srcwfm, 
												&mame_mixer_wsr, NULL ); //&mame_mixer_wsre );
		

	//if ( mame_mixer_wave_cnvnmb != -1 && (mame_mixer_wave_cnvnmb & 0x30) >= 0x20 )
	//	mame_mixer_wave_cnvnmb |= 0x40;	// 부동소수점 연산
}

static BOOL CommonFileDialogAVI(common_file_dialog_procA cfd, char *filename)
{
    BOOL success;

    OPENFILENAMEA of;

    of.lStructSize       = sizeof(of);
    of.hwndOwner         = hMain;
    of.hInstance         = NULL;
#ifdef KOREAN
    of.lpstrFilter       = "AVI 파일(*.avi)\0*.avi\0모든 파일 (*.*)\0*.*\0";
#else
    of.lpstrFilter       = "AVI files (*.avi)\0*.avi\0All files (*.*)\0*.*\0";
#endif
    of.lpstrCustomFilter = NULL;
    of.nMaxCustFilter    = 0;
    of.nFilterIndex      = 1;
    of.lpstrFile         = filename;
    of.nMaxFile          = MAX_PATH;
    of.lpstrFileTitle    = NULL;
    of.nMaxFileTitle     = 0;

	of.lpstrInitialDir   = last_directory_avi;
	of.lpstrTitle        = "저장할 AVI파일";
	
    of.Flags             = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    of.nFileOffset       = 0;
    of.nFileExtension    = 0;
    of.lpstrDefExt       = "avi";
    of.lCustData         = 0;
    of.lpfnHook          = NULL;
    of.lpTemplateName    = NULL;

    success = cfd(&of);
    if (success)
    {
        GetDirectory(filename, last_directory_avi, sizeof(last_directory_avi));
    }
    return success;
}

static void SetupAviStatus(int nGame)
{
	struct MAME_AVI_STATUS *OldAviStatus;	//kt
	struct InternalMachineDriver drv;
    expand_machine_driver(drivers[nGame]->drv, &drv);

	AviStatus.source_file = (char*)drivers[nGame]->source_file;
	AviStatus.index = nGame + 1;

	AviStatus.def_fps = drv.frames_per_second; // fps
	AviStatus.fps     = AviStatus.def_fps;
	AviStatus.depth   = 16; //playing_game_options.depth;	// 색상(auto/16bit/32bit)
	AviStatus.flags   = drivers[nGame]->flags;

	//if (drivers[nGame]->drv->video_attributes & VIDEO_RGB_DIRECT)
	//	AviStatus.depth = 16;

	//if (AviStatus.depth > 8) AviStatus.depth = 16;

	AviStatus.orientation = AviStatus.flags & ORIENTATION_MASK;

	if (playing_game_options.ror == TRUE)
	{
		/* if only one of the components is inverted, switch them */
		if ((AviStatus.orientation & ROT180) == ORIENTATION_FLIP_X ||
			(AviStatus.orientation & ROT180) == ORIENTATION_FLIP_Y) 
		{
			AviStatus.orientation ^= ROT180;
		}
		AviStatus.orientation ^= ROT90;
	}
	else if(playing_game_options.rol == TRUE)
	{
		/* if only one of the components is inverted, switch them */
		if ((AviStatus.orientation & ROT180) == ORIENTATION_FLIP_X ||
			(AviStatus.orientation & ROT180) == ORIENTATION_FLIP_Y) 
		{
			AviStatus.orientation ^= ROT180;
		}
		AviStatus.orientation ^= ROT270;
	}

	if (playing_game_options.flipx)
		AviStatus.orientation ^= ORIENTATION_FLIP_X;
	if (playing_game_options.flipy)
		AviStatus.orientation ^= ORIENTATION_FLIP_Y;

	AviStatus.frame_skip		= 0;
	AviStatus.frame_cmp			= TRUE;
	AviStatus.frame_cmp_pre15	= FALSE;
	AviStatus.frame_cmp_few		= FALSE;
	AviStatus.wave_record		= FALSE;
	AviStatus.bmp_16to8_cnv		= FALSE;

	AviStatus.force_flip_y		= FALSE;

	AviStatus.avi_filesize				= 1800;
	AviStatus.avi_filesizecheck_frame	= 10;
	AviStatus.avi_savefile_pause		= FALSE;


	AviStatus.interlace	= FALSE;
	AviStatus.interlace_odd_number_field = FALSE;

	{
		extern struct WAV_WAVEFORMAT mame_mixer_dstwfm, mame_mixer_srcwfm;
		set_mame_mixer_wfm(nGame);

		AviStatus.audio_channel         = mame_mixer_srcwfm.channel;
		AviStatus.audio_samples_per_sec	= mame_mixer_srcwfm.samplespersec;
		AviStatus.audio_bitrate			= mame_mixer_srcwfm.bitrate;
		
		AviStatus.avi_audio_channel			= mame_mixer_dstwfm.channel;
		AviStatus.avi_audio_samples_per_sec = mame_mixer_dstwfm.samplespersec;
		AviStatus.avi_audio_bitrate			= mame_mixer_dstwfm.bitrate;

		AviStatus.audio_type			= playing_game_options.enable_sound;
		AviStatus.avi_audio_record_type	= (AviStatus.audio_type!=0) ? 2:0;
	}

	AviStatus.hour   = 0;
	AviStatus.minute = 0;
	AviStatus.second = 0;

	if (AviStatus.orientation & ORIENTATION_SWAP_XY)
	{
		AviStatus.width  = drv.screen_height;
		AviStatus.height = drv.screen_width;
		AviStatus.rect.m_Left   = drv.default_visible_area.min_y;
		AviStatus.rect.m_Top    = drv.default_visible_area.min_x;
		AviStatus.rect.m_Width  = drv.default_visible_area.max_y - drv.default_visible_area.min_y + 1;
		AviStatus.rect.m_Height = drv.default_visible_area.max_x - drv.default_visible_area.min_x + 1;
	}
	else
	{
		AviStatus.width  = drv.screen_width;
		AviStatus.height = drv.screen_height;
		AviStatus.rect.m_Left   = drv.default_visible_area.min_x;
		AviStatus.rect.m_Top    = drv.default_visible_area.min_y;
		AviStatus.rect.m_Width  = drv.default_visible_area.max_x - drv.default_visible_area.min_x + 1;
		AviStatus.rect.m_Height = drv.default_visible_area.max_y - drv.default_visible_area.min_y + 1;
	}

	AviStatus.avi_width			= AviStatus.rect.m_Width;
	AviStatus.avi_height		= AviStatus.rect.m_Height;
	AviStatus.avi_depth			= 16;
	AviStatus.avi_rect.m_Left	= 0;
	AviStatus.avi_rect.m_Top	= 0;
	AviStatus.avi_rect.m_Width	= AviStatus.rect.m_Width;
	AviStatus.avi_rect.m_Height	= AviStatus.rect.m_Height;
	AviStatus.avi_smooth_resize_x	= FALSE;
	AviStatus.avi_smooth_resize_y	= FALSE;

	if (AviStatus.avi_rect.m_Width < AviStatus.rect.m_Width ||
		(int)((double)(AviStatus.avi_rect.m_Width<<16) / (double)AviStatus.rect.m_Width) & 0xffff)	
		AviStatus.avi_smooth_resize_x	= TRUE;

	if (AviStatus.avi_rect.m_Height < AviStatus.rect.m_Height ||
		(int)((double)(AviStatus.avi_rect.m_Height<<16) / (double)AviStatus.rect.m_Height) & 0xffff)
		AviStatus.avi_smooth_resize_y	= TRUE;		


	OldAviStatus = GetAviStatus();

	if (OldAviStatus->source_file)
	if (!strcmp(AviStatus.source_file, OldAviStatus->source_file))
	{
		if (AviStatus.def_fps == OldAviStatus->def_fps)
			AviStatus.fps = OldAviStatus->fps;

		AviStatus.frame_skip = OldAviStatus->frame_skip;
		AviStatus.frame_cmp = OldAviStatus->frame_cmp;
		AviStatus.frame_cmp_pre15 = OldAviStatus->frame_cmp_pre15;
		AviStatus.frame_cmp_few = OldAviStatus->frame_cmp_few;
		AviStatus.wave_record = OldAviStatus->wave_record;
		AviStatus.bmp_16to8_cnv = OldAviStatus->bmp_16to8_cnv;

		AviStatus.avi_depth = OldAviStatus->avi_depth;
		AviStatus.interlace = OldAviStatus->interlace;
		AviStatus.interlace_odd_number_field = OldAviStatus->interlace_odd_number_field;
		AviStatus.avi_filesize = OldAviStatus->avi_filesize;
		AviStatus.avi_filesizecheck_frame = OldAviStatus->avi_filesizecheck_frame;
		AviStatus.avi_savefile_pause = OldAviStatus->avi_savefile_pause;


		if (AviStatus.audio_type == OldAviStatus->audio_type)
			AviStatus.avi_audio_record_type	= OldAviStatus->avi_audio_record_type;
		
		AviStatus.hour = OldAviStatus->hour;
		AviStatus.minute = OldAviStatus->minute;
		AviStatus.second = OldAviStatus->second;

		if ((AviStatus.flags & ORIENTATION_SWAP_XY) == (OldAviStatus->flags & ORIENTATION_SWAP_XY) &&
				AviStatus.orientation == OldAviStatus->orientation)
		{
			AviStatus.rect = OldAviStatus->rect;
			AviStatus.avi_width = OldAviStatus->avi_width;
			AviStatus.avi_height = OldAviStatus->avi_height;
			AviStatus.avi_rect = OldAviStatus->avi_rect;
			AviStatus.avi_smooth_resize_x = OldAviStatus->avi_smooth_resize_x;
			AviStatus.avi_smooth_resize_y = OldAviStatus->avi_smooth_resize_y;
		}
	}

	SetAviStatus(&AviStatus);
}

//#ifdef MAME_AVI (NEW_AVI)
void get_autofilename(int nGame, char *avidir, char *avifilename, char *ext)
{
	char szdir[MAX_PATH];
	char sztmpfile[MAX_PATH];
	int  i;

	sprintf( szdir, "%s", avidir );
	i = strlen(szdir); if(i) i--;
	if( szdir[i] == '\\' || szdir[i] == '/' ) szdir[i] = 0; // 끝의 디렉토리 구분자 삭제

	/* "gamename.avi" 가 있는지 조사 */
	sprintf( sztmpfile, "%s\\%s.%s", avidir, drivers[nGame]->name, ext );
	if( access(sztmpfile, 0) != -1 ) {
		do
		{
			/* "nameNNNN.avi" 방식을 사용 */
			sprintf(sztmpfile,"%s\\%.4s%04d.%s", avidir, drivers[nGame]->name, _nAviNo++, ext);
		} while (access(sztmpfile, 0) != -1);
	}

	sprintf( avifilename, "%s", sztmpfile );
}

static void MamePlayGameAVI(void)
{
	int nGame, hr;
	char filename_avi[MAX_PATH];
	char filename_wav[MAX_PATH];

	nGame = Picker_GetSelectedItem(hwndList);

	g_pPlayBkName = NULL;
	g_pRecordName = NULL;
#ifdef KAILLERA
	record   = NULL;
	trace    = NULL;
	playback = NULL;
	playback_trc = NULL;
#endif

	// 추후에 MamePlayGameWithOptions()을 호출하면 playing_game_options 블럭이
	// 셋업되지만 지금 AviStatus 셋업을 위해 호출한다.
	memcpy(&playing_game_options, GetGameOptions(nGame), sizeof(options_type));

	// AviStatus 블럭 설정
	SetupAviStatus(nGame);
	
	// AVI설정 옵션 다이얼로그
	hr = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AVI_STATUS),
				   hMain, AviDialogProc);

	if (hr != 1) return;

	AviStatus = (*(GetAviStatus()));

	// 저장할 AVI파일이름 획득
	get_autofilename(nGame, last_directory_avi, filename_avi, (char*)"avi" );

	strcpy(filename_wav, "");
	if (AviStatus.avi_audio_record_type)
	{
		extern struct WAV_WAVEFORMAT mame_mixer_dstwfm;

		AviStatus.wav_filename = filename_wav;

		if (AviStatus.avi_audio_record_type == 1)
		{
			char drive[_MAX_DRIVE];
			char dir  [_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext  [_MAX_EXT];

			_splitpath(filename_avi, drive, dir, fname, ext);
			_makepath(filename_wav, drive, dir, fname, "wav");

			if (wav_start_log_wave(filename_wav, &mame_mixer_dstwfm) == 0)
			{
				wav_stop_log_wave();
			} 
			else
			{
				char buf[1024];
#ifdef KOREAN
				sprintf(buf, "'%s' 파일을 열 수 없습니다.", filename_wav);
#else
				sprintf(buf, "Could not open '%s' as a valid input file.", filename_wav);
#endif
				MessageBoxA(hMain, buf, MAME32NAME, MB_OK | MB_ICONERROR);
				hr = 0;
				AviStatus.avi_audio_record_type = 0;
			}
		}
	}

	if ( hr == 1 )
	{
		int width, height, depth;
		tRect rect;
		int avi_depth;
		int fcmp;

		extern int			nAviFrameSkip;
		extern unsigned int	nAviFrameCount;
		extern unsigned int	nAviFrameCountStart;
		extern int			nAviAudioRecord;

		width  = AviStatus.width;
		height = AviStatus.height;
		depth  = AviStatus.depth;

		rect = AviStatus.rect;

		avi_depth = depth;
		if (AviStatus.bmp_16to8_cnv == TRUE) avi_depth = 8;

		fcmp=0;
		if (AviStatus.frame_cmp == TRUE)
		{
			fcmp=3;
			if (AviStatus.frame_cmp_pre15 == TRUE)	fcmp=1;
			if (AviStatus.frame_cmp_few == TRUE)	fcmp=2;
		}
		if (AviStatus.fps == AviStatus.def_fps)		fcmp=0;

		nAviFrameSkip = AviStatus.frame_skip;

		nAviFrameCount = 0;
		nAviFrameCountStart = (unsigned int)(((AviStatus.hour*60 + AviStatus.minute)*60 + AviStatus.second) * AviStatus.def_fps);

		nAviAudioRecord = AviStatus.avi_audio_record_type;

		if (AviStartCapture(hMain, filename_avi, &AviStatus))
		{
			// AVI녹화 마지막 안내문
			char buf[1024];
#ifdef KOREAN
			sprintf(buf, "AVI녹화모드로 게임을 실행합니다.\n\n" \
						 "게임 실행 도중에 'AVI 녹화(또는 Record AVI)'에 해당하는 단축키\n" \
						 "(기본값:Shift+F12)를 누르면 AVI녹화를 시작/중단할 수 있습니다." );
#else
			sprintf(buf, "Use 'Record AVI' key(default:Shift+F12) to toggle start/stop AVI recording.");
#endif
			MessageBoxA(hMain, buf, MAME32NAME, MB_OK | MB_ICONEXCLAMATION );
				
			MamePlayGameWithOptions(nGame);
			AviEndCapture();
		}
		else 
		{
			// 마지막 단계(코덱선택)에서 취소한 경우
			if( _nAviNo ) _nAviNo--;
		}
	}
}
//#endif //MAME_AVI (NEW_AVI)

// KAILLERA should be defined!!! (trc사용을 위해 반드시 정의되어 있어야함.)
static void MamePlayBackGameAVI()
{
	int nGame;
	char filename[MAX_PATH];
	char filename_avi[MAX_PATH];
	static char filename_wav[MAX_PATH];
	int	hr;

	*filename = 0;

	nGame = Picker_GetSelectedItem(hwndList);
	if (nGame != -1)
		strcpy(filename, drivers[nGame]->name);

	if (CommonFileDialogA(GetOpenFileNameA, filename, FILETYPE_INPUT_FILES))
	{
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char bare_fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		char path[MAX_PATH];
		char fname[MAX_PATH];

		_splitpath(filename, drive, dir, bare_fname, ext);

		sprintf(path,"%s%s",drive,dir);
		sprintf(fname,"%s%s",bare_fname,".inp");
		if (path[strlen(path)-1] == '\\')
			path[strlen(path)-1] = 0; // take off trailing back slash

		set_pathlist(FILETYPE_INPUTLOG, path);

		if (!strcmp(ext, ".zip")) {
			playback     = mame_fopen(bare_fname,fname, FILETYPE_INPUTLOG, 0);
		} else {
			playback     = mame_fopen(NULL,fname, FILETYPE_INPUTLOG, 0);
		}
		if (playback == NULL)
		{
			char buf[MAX_PATH + 64];
#ifdef KOREAN
			sprintf(buf, "'%s' 파일을 정상적인 입력파일로 열 수 없습니다.", fname);
#else
			sprintf(buf, "Could not open '%s' as a valid input file.", fname);
#endif
			MessageBoxA(NULL, buf, MAME32NAME, MB_OK | MB_ICONERROR);
			return;
		}

		// check for game name embedded in .inp header
		if (playback)
		{
			INP_HEADER inp_header;

			// read playback header
			mame_fread(playback, &inp_header, sizeof(INP_HEADER));

			if (!isalnum(inp_header.name[0])) // If first byte is not alpha-numeric
				mame_fseek(playback, 0, SEEK_SET); // old .inp file - no header
			else
			{
				int i;
				for (i = 0; drivers[i] != 0; i++) // find game and play it
				{
					if (strcmp(drivers[i]->name, inp_header.name) == 0)
					{
						nGame = i;
						break;
					}
				}
			}
		}


		record = NULL;

		// 추후에 MamePlayGameWithOptions()을 호출하면 playing_game_options 블럭이
		// 셋업되지만 지금 AviStatus 셋업을 위해 호출한다.
		memcpy(&playing_game_options, GetGameOptions(nGame), sizeof(options_type));

		// AviStatus 블럭 설정
		SetupAviStatus(nGame);
		
		// AVI설정 옵션 다이얼로그
		hr = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AVI_STATUS),
					   hMain, AviDialogProc);

		if (hr == 1)
		{
			AviStatus = (*(GetAviStatus()));

			if (playback)
			{
				char fname[_MAX_FNAME];

				_splitpath(filename, NULL, NULL, fname, NULL);
				strcpy(filename_avi, fname);
			} else
				strcpy(filename_avi, drivers[nGame]->name);

			// 저장할 AVI파일
			if (!CommonFileDialogAVI(GetOpenFileNameA, filename_avi))
				hr = 0;
		}

		if (hr == 1) 
		{
			strcpy(filename_wav, "");
			if (AviStatus.avi_audio_record_type)
			{
				extern struct WAV_WAVEFORMAT mame_mixer_dstwfm;

				AviStatus.wav_filename = filename_wav;

				if (AviStatus.avi_audio_record_type == 1)
				{
					char drive[_MAX_DRIVE];
					char dir[_MAX_DIR];
					char fname[_MAX_FNAME];
					char ext[_MAX_EXT];

					_splitpath(filename_avi, drive, dir, fname, ext);
					_makepath(filename_wav, drive, dir, fname, "wav");

					if (wav_start_log_wave(filename_wav, &mame_mixer_dstwfm) == 0)
					{
						wav_stop_log_wave();
					} 
					else
					{
						char buf[1024];
#ifdef KOREAN
						sprintf(buf, "'%s' 파일을 열 수 없습니다.", filename_wav);
#else
						sprintf(buf, "Could not open '%s' as a valid input file.", filename_wav);
#endif
						MessageBoxA(hMain, buf, MAME32NAME, MB_OK | MB_ICONERROR);
						hr = 0;
						AviStatus.avi_audio_record_type = 0;
					}
				}
			}
		}

		if ( hr == 1 )
		{
			int width, height, depth;
			tRect rect;
			int avi_depth;
			int fcmp;
			extern int	nAviFrameSkip;
			extern unsigned int				nAviFrameCount;
			extern unsigned int				nAviFrameCountStart;
			extern int						nAviAudioRecord;


			width  = AviStatus.width;
			height = AviStatus.height;
			depth  = AviStatus.depth;

			rect = AviStatus.rect;

			avi_depth = depth;
			if (AviStatus.bmp_16to8_cnv == TRUE) avi_depth = 8;

			fcmp=0;
			if (AviStatus.frame_cmp == TRUE)
			{
				fcmp=3;
				if (AviStatus.frame_cmp_pre15 == TRUE)	fcmp=1;
				if (AviStatus.frame_cmp_few == TRUE)	fcmp=2;
			}
			if (AviStatus.fps == AviStatus.def_fps)		fcmp=0;

			nAviFrameSkip = AviStatus.frame_skip;

			nAviFrameCount = 0;
			nAviFrameCountStart = (unsigned int)(((AviStatus.hour*60 + AviStatus.minute)*60 + AviStatus.second) * AviStatus.def_fps);

			nAviAudioRecord = AviStatus.avi_audio_record_type;

			/* Auto mode. Find out what depth is ok for this game. */
			if (AviStartCapture(hMain, filename_avi, &AviStatus))
			{
				// AVI녹화 마지막 안내문
				char buf[1024];
#ifdef KOREAN
				sprintf(buf, "AVI녹화모드로 게임을 실행합니다.\n\n게임 실행 도중에 단축키 Shift+F12를 누르면 AVI녹화를 시작/중단할 수 있습니다." );
#else
				sprintf(buf, "Use 'Shift+F12' Key to toggle AVI recoding start/stop");
#endif
				MessageBoxA(hMain, buf, MAME32NAME, MB_OK | MB_ICONEXCLAMATION );
			
				MamePlayGameWithOptions(nGame);
				AviEndCapture();
			}
		}

		mame_fclose(playback);

		playback = NULL;

	}
}
#endif /* MAME_AVI */

#ifdef USE_SAVE_GAMELIST
static void SaveGameListToFile(char *szFile, int Formatted)
{

	int Order[COLUMN_MAX];
	int Size[COLUMN_MAX] = {70, 4, 8, 9, 6, 9, 6, 30, 4, 70, 12, 9};
	int nColumnMax = Picker_GetNumColumns(hwndList);
	int nListCount = ListView_GetItemCount(hwndList);
	int nIndex = 0, nGameIndex = 0;
	int i, j = 0;

#ifdef USE_FIXED_CODE // CHD_FILTER: Added psx
#define MAX_FILTERS	9
	const char *Filters[MAX_FILTERS] = { "Clones", "Non-Working", "Unvailable", "Vector Graphics", "Raster Graphics", "Originals", "Working", "Available", "Hard Disk" };
#else
#define MAX_FILTERS	8
	const char *Filters[MAX_FILTERS] = { "Clones", "Non-Working", "Unvailable", "Vector Graphics", "Raster Graphics", "Originals", "Working", "Available"};
#endif /* USE_FIXED_CODE */
	char *CrLf;
	char Buf[300];

	LPTREEFOLDER lpFolder = GetCurrentFolder();
	LV_ITEM lvi;

	FILE *fp = fopen(szFile, "wt");

	// No interline with tabuled format
	if ( Formatted )
		CrLf = (char *)"\n\n";
	else
		CrLf = (char *)"\n";
	
	if (fp == NULL)
	{
		MessageBoxA(GetMainWindow(), "Error : unable to access file", MAME32NAME, MB_OK | MB_ICONERROR);
		return;
	}

	GetRealColumnOrder(Order);

	// Title
	sprintf( Buf, "%s %s.%s", MAME32NAME, GetVersionString(), CrLf );
	fwrite( Buf, strlen(Buf), 1, fp);
	sprintf( Buf, "This is the current list of games, as displayed in the GUI (%s view mode).%s", ((GetViewMode() == VIEW_GROUPED)?"grouped":"detail"), CrLf );
	fwrite( Buf, strlen(Buf), 1, fp);

	// Current folder
	sprintf( Buf, "Current folder : <" );
	if ( lpFolder->m_nParent != -1 )
	{
		// Shows only 2 levels (last and previous)
		LPTREEFOLDER lpF = GetFolder( lpFolder->m_nParent );

#if 0 // Changed </Folder name> ==> <Folder name>
		if ( lpF->m_nParent == -1 )
		{
			strcat( Buf, "/" );
		}
#endif
		strcat( Buf, lpF->m_lpTitle );
		strcat( Buf, " / " );
	}
#if 0 // Changed </Folder name> ==> <Folder name>
	else
	{
		strcat( Buf, "/" );
	}
#endif
	sprintf( &Buf[strlen(Buf)], "%s>%s.%s", lpFolder->m_lpTitle, (lpFolder->m_dwFlags&F_CUSTOM?" (custom folder)":""), CrLf );
	fwrite( Buf, strlen(Buf), 1, fp);

	// Filters
	sprintf(Buf, "Additional excluding filter(s) : ");
	for (i=0,j=0; i<MAX_FILTERS; i++ )
	{
		if ( lpFolder->m_dwFlags & (1<<i) )
		{
			if ( j > 0)
			{
				strcat( Buf, ", ");
			}

			strcat(Buf, Filters[i]);

			j++;
		}
	}
	if ( j == 0)
	{
		strcat(Buf, "none");
	}
	strcat(Buf, ".");
	strcat(Buf, CrLf);
	fwrite( Buf, strlen(Buf), 1, fp);

	// Sorting
	if ( GetSortColumn() > 0 )
	{
		sprintf( Buf, "Sorted by <%s> descending order", column_names[GetSortColumn()] );
	}
	else
	{
		sprintf( Buf, "Sorted by <%s> ascending order", column_names[-GetSortColumn()] );
	}
	sprintf( &Buf[strlen(Buf)], ", %d game(s) found.%s", nListCount, CrLf );
	fwrite( Buf, strlen(Buf), 1, fp);

	if ( Formatted )
	{
		// Separator
		memset( Buf, '-', sizeof(Buf) );
		Buf[0] = '+';
		for (i=0,j=0; i<nColumnMax; i++ )
		{
			j += Size[Order[i]]+3;
			Buf[j] = '+';
		}
		Buf[j+1] = '\0';
		strcat( Buf, "\n");
		fwrite( Buf, strlen(Buf), 1, fp);

		// Left side of columns title
		Buf[0] = '|';
		Buf[1] = '\0';
	}
	else
		Buf[0] = '\0';

	// Title of columns
	for (i=0; i<nColumnMax; i++ )
	{
		if ( Formatted )
			sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], column_names[Order[i]] );
		else
		{
			if ( i )
				sprintf( &Buf[strlen(Buf)], "\t%s", column_names[Order[i]] );
			else
				sprintf( &Buf[strlen(Buf)], "%s", column_names[Order[i]] );
		}
	}
	strcat( Buf, "\n");
	fwrite( Buf, strlen(Buf), 1, fp);

	// Separator
	if ( Formatted )
	{
		memset( Buf, '-', sizeof(Buf) );
		Buf[0] = '+';
		for (i=0,j=0; i<nColumnMax; i++ )
		{
			// Fixed Real Column Size
			int size = (int)strlen(column_names[Order[i]]);
			if ( Size[Order[i]] < size )
				Size[Order[i]] = size;
			j += Size[Order[i]]+3;
			Buf[j] = '+';
		}
		Buf[j+1] = '\0';
		strcat( Buf, "\n");
		fwrite( Buf, strlen(Buf), 1, fp);
	}

	// Games
	for ( nIndex=0; nIndex<nListCount; nIndex++ )
	{
		lvi.iItem = nIndex;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM;

		if ( ListView_GetItem(hwndList, &lvi) )
		{
			nGameIndex  = lvi.lParam;

			if ( Formatted )
			{
				Buf[0] = '|';
				Buf[1] = '\0';
			}
			else
				Buf[0] = '\0';

			// lvi.lParam contains the absolute game index
			for (i=0; i<nColumnMax; i++ )
			{
				if ((i > 1) && (! Formatted))
					strcat(&Buf[strlen(Buf)], "\t");

				switch( Order[i] )
				{
					case  0: // Name
						if ( Formatted )
						{
							if ( DriverIsClone(nGameIndex) && (GetViewMode() == VIEW_GROUPED) )
							{
								sprintf( &Buf[strlen(Buf)], "    %-*.*s |", Size[Order[i]]-3, Size[Order[i]]-3, ModifyThe(drivers[nGameIndex]->description) );
							}
							else
							{
								sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], ModifyThe(drivers[nGameIndex]->description) );
							}
						}
						else
							sprintf( &Buf[strlen(Buf)], "%s", ModifyThe(drivers[nGameIndex]->description) );
						break;

					case  1: // ROMs
						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], (GetRomAuditResults(nGameIndex)==TRUE?"no":"yes") );
						else
							sprintf( &Buf[strlen(Buf)], "%s", (GetRomAuditResults(nGameIndex)==TRUE?"no":"yes") );
						break;

					case  2: // Samples
						if (DriverUsesSamples(nGameIndex))
						{
							if ( Formatted )
								sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], (GetSampleAuditResults(nGameIndex)?"no":"yes") );
							else
								sprintf( &Buf[strlen(Buf)], "%s", (GetSampleAuditResults(nGameIndex)?"no":"yes") );
						}
						else
						{
							if ( Formatted )
								sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], "" );
						}
						break;

					case  3: // Directory
						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], drivers[nGameIndex]->name );
						else
							sprintf( &Buf[strlen(Buf)], "%s", drivers[nGameIndex]->name );
						break;

					case  4: // Type
						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], (DriverIsVector(nGameIndex)?"Vector":"Raster") );
						else
							sprintf( &Buf[strlen(Buf)], "%s", (DriverIsVector(nGameIndex)?"Vector":"Raster") );
						break;

					case  5: // Trackball
						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], (DriverUsesTrackball(nGameIndex)?"yes":"no") );
						else
							sprintf( &Buf[strlen(Buf)], "%s", (DriverUsesTrackball(nGameIndex)?"yes":"no") );
						break;

					case  6: // Played
						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*d |", Size[Order[i]], GetPlayCount(nGameIndex) );
						else
							sprintf( &Buf[strlen(Buf)], "%d", GetPlayCount(nGameIndex) );
						break;

					case  7: // Manufacturer
						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], drivers[nGameIndex]->manufacturer );
						else
							sprintf( &Buf[strlen(Buf)], "%s", drivers[nGameIndex]->manufacturer );
						break;

					case  8: // Year
						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], drivers[nGameIndex]->year );
						else
							sprintf( &Buf[strlen(Buf)], "%s", drivers[nGameIndex]->year );
						break;

					case  9: // Clone of
						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], GetCloneParentName(nGameIndex) );
						else
							sprintf( &Buf[strlen(Buf)], "%s", GetCloneParentName(nGameIndex) );
						break;

					case 10: // Source
						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], GetDriverFilename(nGameIndex) );
						else
							sprintf( &Buf[strlen(Buf)], "%s", GetDriverFilename(nGameIndex) );
						break;

					case 11: // Play time
					{
						char Tmp[20];
						
						GetTextPlayTime(nGameIndex, Tmp);

						if ( Formatted )
							sprintf( &Buf[strlen(Buf)], " %-*.*s |", Size[Order[i]], Size[Order[i]], Tmp );
						else
							sprintf( &Buf[strlen(Buf)], "%s", Tmp );

						break;
					}
				}
			}
			strcat( Buf, "\n");
			fwrite( Buf, strlen(Buf), 1, fp);
		}
	}

	// Last separator
	if ( Formatted && (nListCount > 0) )
	{
		memset( Buf, '-', sizeof(Buf) );
		Buf[0] = '+';
		for (i=0,j=0; i<nColumnMax; i++ )
		{
			j += Size[Order[i]]+3;
			Buf[j] = '+';
		}
		Buf[j+1] = '\0';
		strcat( Buf, "\n");
		fwrite( Buf, strlen(Buf), 1, fp);
	}

	fclose(fp);
	
	SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)0, (LPARAM)"File created");
}

static void SaveRomsListToFile(char *szFile)
{
	int Order[COLUMN_MAX];
	int nColumnMax = Picker_GetNumColumns(hwndList);
	int nListCount = ListView_GetItemCount(hwndList);
	int nIndex = 0, nGameIndex = 0;
	int i = 0;

	char *CrLf;
	char Buf[300];

	LPTREEFOLDER lpFolder = GetCurrentFolder();
	LV_ITEM lvi;

	FILE *fp = fopen(szFile, "wt");

	CrLf = (char *)"\n\n";
	
	if (fp == NULL)
	{
		MessageBoxA(GetMainWindow(), "Error : unable to access file", MAME32NAME, MB_OK | MB_ICONERROR);
		return;
	}

	GetRealColumnOrder(Order);

	// Title
	sprintf( Buf, "%s %s.%s", MAME32NAME, GetVersionString(), CrLf );
	fwrite( Buf, strlen(Buf), 1, fp);
	sprintf( Buf, "This is the current list of ROMs.%s", CrLf );
	fwrite( Buf, strlen(Buf), 1, fp);

	// Current folder
	sprintf( Buf, "Current folder : <" );
	if ( lpFolder->m_nParent != -1 )
	{
		// Shows only 2 levels (last and previous)
		LPTREEFOLDER lpF = GetFolder( lpFolder->m_nParent );

#if 0 // Changed </Folder name> ==> <Folder name>
		if ( lpF->m_nParent == -1 )
		{
			strcat( Buf, "/" );
		}
#endif
		strcat( Buf, lpF->m_lpTitle );
		strcat( Buf, " / " );
	}
#if 0 // Changed </Folder name> ==> <Folder name>
	else
	{
		strcat( Buf, "/" );
	}
#endif
	sprintf( &Buf[strlen(Buf)], "%s>%s.%s", lpFolder->m_lpTitle, (lpFolder->m_dwFlags&F_CUSTOM?" (custom folder)":""), CrLf );
	fwrite( Buf, strlen(Buf), 1, fp);

	// Sorting
	if ( GetSortColumn() > 0 )
	{
		sprintf( Buf, "Sorted by <%s> descending order", column_names[GetSortColumn()] );
	}
	else
	{
		sprintf( Buf, "Sorted by <%s> ascending order", column_names[-GetSortColumn()] );
	}
	sprintf( &Buf[strlen(Buf)], ", %d game(s) found.%s", nListCount, CrLf );
	fwrite( Buf, strlen(Buf), 1, fp);

	Buf[0] = '\0';

	// Games
	for ( nIndex=0; nIndex<nListCount; nIndex++ )
	{
		lvi.iItem = nIndex;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM;

		if ( ListView_GetItem(hwndList, &lvi) )
		{
			nGameIndex  = lvi.lParam;
			Buf[0] = '\0';

			// lvi.lParam contains the absolute game index
			for (i=0; i<nColumnMax; i++ )
			{
				switch( Order[i] )
				{
					case  3: // ROMs
						sprintf( &Buf[strlen(Buf)], "%s", drivers[nGameIndex]->name );
						break;
				}
			}
			strcat( Buf, "\n");
			fwrite( Buf, strlen(Buf), 1, fp);
		}
	}

	fclose(fp);
	
	SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)0, (LPARAM)"File created");
}
#endif /* USE_SAVE_GAMELIST */

/* End of source file */
