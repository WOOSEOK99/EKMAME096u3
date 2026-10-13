/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

***************************************************************************/

/***************************************************************************

  dialogs.c

  Dialog box procedures go here

***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string.h>

#include "bitmask.h"
#include "TreeView.h"
#include "M32Util.h"
#include "resource.h"
#include "directories.h"
#include "options.h"
#include "splitters.h"
#include "help.h"
#include "audit32.h"
#include "screenshot.h"
#include "mame32.h"
#include "properties.h"
#include "dialogs.h"
#include "translate.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#define FILTERTEXT_LEN 256

static char g_FilterText[FILTERTEXT_LEN];

#define NUM_EXCLUSIONS  9

/* Pairs of filters that exclude each other */
DWORD filterExclusion[NUM_EXCLUSIONS] =
{
	IDC_FILTER_CLONES,      IDC_FILTER_ORIGINALS,
	IDC_FILTER_NONWORKING,  IDC_FILTER_WORKING,
	IDC_FILTER_UNAVAILABLE, IDC_FILTER_AVAILABLE,
	IDC_FILTER_RASTER,      IDC_FILTER_VECTOR
};

static void DisableFilterControls(HWND hWnd, LPFOLDERDATA lpFilterRecord,
								  LPFILTER_ITEM lpFilterItem, DWORD dwFlags);
static void EnableFilterExclusions(HWND hWnd, DWORD dwCtrlID);
static DWORD ValidateFilters(LPFOLDERDATA lpFilterRecord, DWORD dwFlags);
static void OnHScroll(HWND hWnd, HWND hwndCtl, UINT code, int pos);

/***************************************************************************/
static HWND   hMain  = NULL;

const char * GetFilterText(void)
{
	return g_FilterText;
}

INT_PTR CALLBACK ResetDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BOOL resetFilters  = FALSE;
	BOOL resetGames    = FALSE;
	BOOL resetUI	   = FALSE;
	BOOL resetDefaults = FALSE;

	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, FALSE);

		return TRUE;

	case WM_HELP:
		/* User clicked the ? from the upper right on a control */
		HelpFunction(((LPHELPINFO)lParam)->hItemHandle, MAME32CONTEXTHELP, HH_TP_HELP_WM_HELP, GetHelpIDs());
		break;

	case WM_CONTEXTMENU:
		HelpFunction((HWND)wParam, MAME32CONTEXTHELP, HH_TP_HELP_CONTEXTMENU, GetHelpIDs());

		break;

	case WM_COMMAND :
		switch (GET_WM_COMMAND_ID(wParam, lParam))
		{
		case IDOK :
			resetFilters  = Button_GetCheck(GetDlgItem(hDlg, IDC_RESET_FILTERS));
			resetGames	  = Button_GetCheck(GetDlgItem(hDlg, IDC_RESET_GAMES));
			resetDefaults = Button_GetCheck(GetDlgItem(hDlg, IDC_RESET_DEFAULT));
			resetUI 	  = Button_GetCheck(GetDlgItem(hDlg, IDC_RESET_UI));
			if (resetFilters || resetGames || resetUI || resetDefaults)
			{
				char temp[200];
				strcpy(temp, _UI(MAME32NAME " will now reset the selected\n"));
				strcat(temp, _UI("items to the original, installation\n"));
				strcat(temp, _UI("settings then exit.\n\n"));
				strcat(temp, _UI("The new settings will take effect\n"));
				strcat(temp, _UI("the next time "MAME32NAME " is run.\n\n"));
				strcat(temp, _UI("Do you wish to continue?"));

				if (MessageBox(hDlg, _Unicode(temp), _Unicode(_UI("Restore Settings")), IDOK) == IDOK)
				{
					if (resetFilters)
						ResetFilters();

					if (resetUI)
						ResetGUI();

					if (resetDefaults)
						ResetGameDefaults();

					if (resetGames)
						ResetAllGameOptions();

					EndDialog(hDlg, 1);
					return TRUE;
				}
			}
			/* Fall through if no options were selected
			 * or the user hit cancel in the popup dialog.
			 */
		case IDCANCEL :
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return 0;
}

INT_PTR CALLBACK InterfaceDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	char tmp[4];
	int i;
	BOOL bRedrawList = FALSE;
	int nCurSelection = 0;
	int nHistoryTab = 0;
	int nCount = 0;
	int value;

	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, FALSE);

		Button_SetCheck(GetDlgItem(hDlg,IDC_START_GAME_CHECK),GetGameCheck());
		Button_SetCheck(GetDlgItem(hDlg,IDC_JOY_GUI),GetJoyGUI());
		Button_SetCheck(GetDlgItem(hDlg,IDC_KEY_GUI),GetKeyGUI());
		Button_SetCheck(GetDlgItem(hDlg,IDC_BROADCAST),GetBroadcast());
		Button_SetCheck(GetDlgItem(hDlg,IDC_RANDOM_BG),GetRandomBackground());
//		Button_SetCheck(GetDlgItem(hDlg,IDC_SKIP_DISCLAIMER),GetSkipDisclaimer());
//		Button_SetCheck(GetDlgItem(hDlg,IDC_SKIP_GAME_INFO),GetSkipGameInfo());
//		Button_SetCheck(GetDlgItem(hDlg,IDC_HIGH_PRIORITY),GetHighPriority());
		Button_SetCheck(GetDlgItem(hDlg,IDC_GAME_CAPTION),GetGameCaption());

		Button_SetCheck(GetDlgItem(hDlg,IDC_HIDE_MOUSE),GetHideMouseOnStartup());
		Button_SetCheck(GetDlgItem(hDlg,IDC_SINGLE_PROCESS),GetLaunchGameInSameProcess());

		// Get the current value of the control
		SendDlgItemMessageA(hDlg, IDC_CYCLETIMESEC, TBM_SETRANGE,
					(WPARAM)FALSE,
					(LPARAM)MAKELONG(0, 60)); /* [0, 60] */
		value = GetCycleScreenshot();
		SendDlgItemMessageA(hDlg,IDC_CYCLETIMESEC, TBM_SETPOS, TRUE, value);
		itoa(value,tmp,10);
		SendDlgItemMessageA(hDlg,IDC_CYCLETIMESECTXT,WM_SETTEXT,0, (WPARAM)tmp);

		Button_SetCheck(GetDlgItem(hDlg,IDC_STRETCH_SCREENSHOT_LARGER),
						GetStretchScreenShotLarger());
		Button_SetCheck(GetDlgItem(hDlg,IDC_FILTER_INHERIT),
						GetFilterInherit());
		Button_SetCheck(GetDlgItem(hDlg,IDC_NOOFFSET_CLONES),
						GetOffsetClones());
		Button_SetCheck(GetDlgItem(hDlg,IDC_BROKEN_ICON),
						!GetUseBrokenIcon());
		ComboBox_AddStringA(GetDlgItem(hDlg, IDC_HISTORY_TAB), _UI("Snapshot"));
		ComboBox_SetItemData(GetDlgItem(hDlg, IDC_HISTORY_TAB), nCount++, TAB_SCREENSHOT);
		ComboBox_AddStringA(GetDlgItem(hDlg, IDC_HISTORY_TAB), _UI("Flyer"));
		ComboBox_SetItemData(GetDlgItem(hDlg, IDC_HISTORY_TAB), nCount++, TAB_FLYER);
		ComboBox_AddStringA(GetDlgItem(hDlg, IDC_HISTORY_TAB), _UI("Cabinet"));
		ComboBox_SetItemData(GetDlgItem(hDlg, IDC_HISTORY_TAB), nCount++, TAB_CABINET);
		ComboBox_AddStringA(GetDlgItem(hDlg, IDC_HISTORY_TAB), _UI("Marquee"));
		ComboBox_SetItemData(GetDlgItem(hDlg, IDC_HISTORY_TAB), nCount++, TAB_MARQUEE);
		ComboBox_AddStringA(GetDlgItem(hDlg, IDC_HISTORY_TAB), _UI("Title"));
		ComboBox_SetItemData(GetDlgItem(hDlg, IDC_HISTORY_TAB), nCount++, TAB_TITLE);
		ComboBox_AddStringA(GetDlgItem(hDlg, IDC_HISTORY_TAB), _UI("Control Panel"));
		ComboBox_SetItemData(GetDlgItem(hDlg, IDC_HISTORY_TAB), nCount++, TAB_CONTROL_PANEL);
		ComboBox_AddStringA(GetDlgItem(hDlg, IDC_HISTORY_TAB), _UI("All"));
		ComboBox_SetItemData(GetDlgItem(hDlg, IDC_HISTORY_TAB), nCount++, TAB_ALL);
		ComboBox_AddStringA(GetDlgItem(hDlg, IDC_HISTORY_TAB), _UI("None"));
		ComboBox_SetItemData(GetDlgItem(hDlg, IDC_HISTORY_TAB), nCount++, TAB_NONE);
		if( GetHistoryTab() < MAX_TAB_TYPES )
			ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_HISTORY_TAB), GetHistoryTab());
		else
			ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_HISTORY_TAB), GetHistoryTab()-TAB_SUBTRACT);

		SendDlgItemMessageA(hDlg, IDC_SCREENSHOT_BORDERSIZE, TBM_SETRANGE,
					(WPARAM)FALSE,
					(LPARAM)MAKELONG(0, 100)); /* [0, 100] */
		value = GetScreenshotBorderSize();
		SendDlgItemMessageA(hDlg,IDC_SCREENSHOT_BORDERSIZE, TBM_SETPOS, TRUE, value);
		itoa(value,tmp,10);
		SendDlgItemMessageA(hDlg,IDC_SCREENSHOT_BORDERSIZETXT,WM_SETTEXT,0, (WPARAM)tmp);
		//return TRUE;
		break;

	case WM_HELP:
		/* User clicked the ? from the upper right on a control */
		HelpFunction(((LPHELPINFO)lParam)->hItemHandle, MAME32CONTEXTHELP, HH_TP_HELP_WM_HELP, GetHelpIDs());
		break;

	case WM_CONTEXTMENU:
		HelpFunction((HWND)wParam, MAME32CONTEXTHELP, HH_TP_HELP_CONTEXTMENU, GetHelpIDs());
		break;

	case WM_HSCROLL:
		HANDLE_WM_HSCROLL(hDlg, wParam, lParam, OnHScroll);
		break;

	case WM_COMMAND :
		switch (GET_WM_COMMAND_ID(wParam, lParam))
		{
		case IDC_SCREENSHOT_BORDERCOLOR:
		{
			CHOOSECOLOR cc;
			COLORREF choice_colors[16];

			for (i=0;i<16;i++)
				choice_colors[i] = GetCustomColor(i);

			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner   = hDlg;
			cc.rgbResult   = GetScreenshotBorderColor();
			cc.lpCustColors = choice_colors;
			cc.Flags       = CC_ANYCOLOR | CC_RGBINIT | CC_SOLIDCOLOR;
			if (!ChooseColor(&cc))
				return TRUE;
 			for (i=0;i<16;i++)
 				SetCustomColor(i,choice_colors[i]);
			SetScreenshotBorderColor(cc.rgbResult);
			UpdateScreenShot();
			return TRUE;
		}

		case IDC_EDIT_BROKEN_COLOR:
		{
			CHOOSECOLOR cc;
			COLORREF choice_colors[16];

			for (i=0;i<16;i++)
				choice_colors[i] = GetCustomColor(i);

			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner   = hDlg;
			cc.rgbResult   = GetListBrokenColor();
			cc.lpCustColors = choice_colors;
			cc.Flags       = CC_ANYCOLOR | CC_RGBINIT | CC_SOLIDCOLOR;
			if (!ChooseColor(&cc))
				return TRUE;
 			for (i=0;i<16;i++)
 				SetCustomColor(i,choice_colors[i]);
			SetListBrokenColor(cc.rgbResult);
			// LineUpIcons does just a ResetListView(), which is what we want here
			PostMessage(GetMainWindow(),WM_COMMAND, MAKEWPARAM(ID_VIEW_LINEUPICONS, FALSE),(LPARAM)NULL);
			return TRUE;
		}

		case IDOK :
		{
			BOOL checked = FALSE;

			SetGameCheck(Button_GetCheck(GetDlgItem(hDlg, IDC_START_GAME_CHECK)));
			SetJoyGUI(Button_GetCheck(GetDlgItem(hDlg, IDC_JOY_GUI)));
			SetKeyGUI(Button_GetCheck(GetDlgItem(hDlg, IDC_KEY_GUI)));
			SetBroadcast(Button_GetCheck(GetDlgItem(hDlg, IDC_BROADCAST)));
			SetRandomBackground(Button_GetCheck(GetDlgItem(hDlg, IDC_RANDOM_BG)));
//			SetSkipDisclaimer(Button_GetCheck(GetDlgItem(hDlg, IDC_SKIP_DISCLAIMER)));
//			SetSkipGameInfo(Button_GetCheck(GetDlgItem(hDlg, IDC_SKIP_GAME_INFO)));
//			SetHighPriority(Button_GetCheck(GetDlgItem(hDlg, IDC_HIGH_PRIORITY)));
			SetGameCaption(Button_GetCheck(GetDlgItem(hDlg, IDC_GAME_CAPTION)));
			
			SetHideMouseOnStartup(Button_GetCheck(GetDlgItem(hDlg,IDC_HIDE_MOUSE)));
			SetLaunchGameInSameProcess(Button_GetCheck(GetDlgItem(hDlg, IDC_SINGLE_PROCESS)));

			if( Button_GetCheck(GetDlgItem(hDlg,IDC_RESET_PLAYCOUNT ) ) )
			{
				ResetPlayCount( -1 );
				bRedrawList = TRUE;
			}
			if( Button_GetCheck(GetDlgItem(hDlg,IDC_RESET_PLAYTIME ) ) )
			{
				ResetPlayTime( -1 );
				bRedrawList = TRUE;
			}
			value = SendDlgItemMessageA(hDlg,IDC_CYCLETIMESEC, TBM_GETPOS, 0, 0);
			if( GetCycleScreenshot() != value )
			{
				SetCycleScreenshot(value);
			}
			value = SendDlgItemMessageA(hDlg,IDC_SCREENSHOT_BORDERSIZE, TBM_GETPOS, 0, 0);
			if( GetScreenshotBorderSize() != value )
			{
				SetScreenshotBorderSize(value);
				UpdateScreenShot();
			}
			checked = Button_GetCheck(GetDlgItem(hDlg,IDC_STRETCH_SCREENSHOT_LARGER));
			if (checked != GetStretchScreenShotLarger())
			{
				SetStretchScreenShotLarger(checked);
				UpdateScreenShot();
			}

			checked = Button_GetCheck(GetDlgItem(hDlg,IDC_FILTER_INHERIT));
			if (checked != GetFilterInherit())
			{
				SetFilterInherit(checked);
				// LineUpIcons does just a ResetListView(), which is what we want here
				PostMessage(GetMainWindow(),WM_COMMAND, MAKEWPARAM(ID_VIEW_LINEUPICONS, FALSE),(LPARAM)NULL);
 			}
			checked = Button_GetCheck(GetDlgItem(hDlg,IDC_NOOFFSET_CLONES));
			if (checked != GetOffsetClones())
			{
				SetOffsetClones(checked);
				// LineUpIcons does just a ResetListView(), which is what we want here
				PostMessage(GetMainWindow(),WM_COMMAND, MAKEWPARAM(ID_VIEW_LINEUPICONS, FALSE),(LPARAM)NULL);
 			}
			checked = !Button_GetCheck(GetDlgItem(hDlg,IDC_BROKEN_ICON));
			if (checked != GetUseBrokenIcon())
			{
				SetUseBrokenIcon(checked);
				// LineUpIcons does just a ResetListView(), which is what we want here
				PostMessage(GetMainWindow(),WM_COMMAND, MAKEWPARAM(ID_VIEW_LINEUPICONS, FALSE),(LPARAM)NULL);
 			}
			nCurSelection = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_HISTORY_TAB));
			if (nCurSelection != CB_ERR)
				nHistoryTab = ComboBox_GetItemData(GetDlgItem(hDlg,IDC_HISTORY_TAB), nCurSelection);
			EndDialog(hDlg, 0);
			if( GetHistoryTab() != nHistoryTab )
			{
				SetHistoryTab(nHistoryTab, TRUE);
				ResizePickerControls(GetMainWindow());
				UpdateScreenShot();
			}
			if( bRedrawList )
			{
				UpdateListView();
			}
			return TRUE;
		}
		case IDCANCEL :
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return 0;
}

#ifdef DARKMAME
typedef struct {
	char			szFileName[MAX_PATH];
	HPALETTE		hPal;
	HBITMAP			hBmp;
	MYBITMAPINFO	info;
	RECT			rc;
	int				iXOffs;
	int				iYOffs;
	int				iDrawWidth;
	int				iDrawHeight;
} BGINFO;

enum {
	BGINFO_FOLDER = 0,
	BGINFO_LIST,
	BGINFO_SNAPSHOT,
	
	BGINFO_MAX
};

	
// [인터페이스] 글꼴 및 배경이미지 다이얼로그
typedef struct {
	LOGFONTA		listFont;	// ListView font
	COLORREF	ListColor;	// font color
	COLORREF	CloneColor;	// clone color
	RECT		rcList;
	RECT		rcClone;
	//-----------------------------------------
	LOGFONTA		histFont;	// Histroy Font
	COLORREF	histColor;	// Histroy font color
	RECT		rcHist;
#ifdef KAILLERA	
	//-----------------------------------------
	LOGFONTA		chatFont;	// Kaillera Chat Font
	COLORREF	chatColor;	// Kaillera Chat font color
	RECT		rcChat;
#endif	
	//-----------------------------------------
	BGINFO		bgData[BGINFO_MAX];
	//-----------------------------------------
} INTERFACE_EX;

INTERFACE_EX _ExUIOpt;

extern UINT_PTR CALLBACK CFHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam,LPARAM lParam);

static void PickUIFont(HWND hWnd, int type)
{
	LOGFONTA 	font;
	COLORREF	crTextColor, crTemp;
	RECT		rcArea;
	CHOOSEFONTA  cf;
	char 		szText[256];
	int			i;
	
	switch(type) {
	default:
	case 0:	// ListView Font
		memcpy( &font, &_ExUIOpt.listFont, sizeof(LOGFONT) );
		crTextColor = _ExUIOpt.ListColor;
		memcpy( &rcArea, &_ExUIOpt.rcList, sizeof(RECT) );
		break;
	case 1: // History Font
		memcpy( &font, &_ExUIOpt.histFont, sizeof(LOGFONT) );
		crTextColor = _ExUIOpt.histColor;
		memcpy( &rcArea, &_ExUIOpt.rcHist, sizeof(RECT) );
		break;
#if 0
	case 2: // Kaillera Chat Font
		memcpy( &font, &_ExUIOpt.chatFont, sizeof(LOGFONT) );
		crTextColor = _ExUIOpt.chatColor;
		memcpy( &rcArea, &_ExUIOpt.rcChat, sizeof(RECT) );
		break;
#endif
	}

	font.lfQuality = DEFAULT_QUALITY;

	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner   = hWnd;
	cf.lpLogFont   = &font;
	cf.lpfnHook    = &CFHookProc;
	cf.rgbColors   = crTextColor;
	cf.Flags	   = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_ENABLEHOOK;

	// 후킹 프로시저로 사용하는 CFHookProc에서 폰트다이얼로그의 기본선택색상을
	// 판단하기 위해 GetListFontColor()함수를 이용하여 현재의 선택색상값을 읽어온다.
	// 따라서 이를 위해 임시적으로 여기서 ListFontColor를 바꿔준다.
	crTemp = GetListFontColor();
	SetListFontColor(crTextColor);
	
	i = ChooseFontA(&cf);
	
	// 바꾼 ListFontColor를 복구
	SetListFontColor(crTemp);
	crTemp = GetListFontColor();
	
	if (!i) return;
	
	sprintf( szText, "%s, (%ld %ld)", font.lfFaceName, font.lfHeight, font.lfWidth );
	switch(type) {
	default:
	case 0:	// ListView Font
		memcpy( &_ExUIOpt.listFont, &font, sizeof(LOGFONT) );
		_ExUIOpt.ListColor = cf.rgbColors;
		SetDlgItemTextA(hWnd, IDC_STATIC_MAINFONT, szText);
		break;
	case 1: // History Font
		memcpy( &_ExUIOpt.histFont, &font, sizeof(LOGFONT) );
		_ExUIOpt.histColor = cf.rgbColors;
		SetDlgItemTextA(hWnd, IDC_STATIC_HISTORYFONT, szText);
		break;
#if 0
	case 2: // Kaillera Chat Font
		memcpy( &_ExUIOpt.chatFont, &font, sizeof(LOGFONT) );
		_ExUIOpt.chatColor = cf.rgbColors;
		SetDlgItemText(hWnd, IDC_STATIC_CHATFONT, szText);
		break;
#endif
	}
		
	InvalidateRect( hWnd, &rcArea, FALSE );
}

static void	ResetUIBgBitmapInfo(int idx)
{
	if(idx == -1) {
		for(int i=0; i<BGINFO_MAX; i++) 
		{
			if (_ExUIOpt.bgData[i].hBmp) DeleteObject(_ExUIOpt.bgData[i].hBmp);
			if (_ExUIOpt.bgData[i].hPal) DeleteObject(_ExUIOpt.bgData[i].hPal);
			
			_ExUIOpt.bgData[i].hBmp = 0;
			_ExUIOpt.bgData[i].hPal = 0;
		}
	}
	else {
		if (_ExUIOpt.bgData[idx].hBmp) DeleteObject(_ExUIOpt.bgData[idx].hBmp);
		if (_ExUIOpt.bgData[idx].hPal) DeleteObject(_ExUIOpt.bgData[idx].hPal);
		
		_ExUIOpt.bgData[idx].hBmp = 0;
		_ExUIOpt.bgData[idx].hPal = 0;
	}
}

// 지정한 배경이미지 정보를 토대로 다이얼로그 위에 그려질 위치와 크기를 계산
static void	CalcBgDrawSize(int idx)
{
	int iWidth, iHeight, iImgWidth, iImgHeight;
	BGINFO	*lpInfo;

	if((idx<0) || (idx>=BGINFO_MAX)) return;
	
	lpInfo = &_ExUIOpt.bgData[idx];
	
	// 그려질 영역의 크기
	iWidth  = lpInfo->rc.right - lpInfo->rc.left;
	iHeight = lpInfo->rc.bottom - lpInfo->rc.top;
	
	// 이미지의 크기
	iImgWidth  = lpInfo->info.bmWidth;
	iImgHeight = lpInfo->info.bmHeight;

	// 잘못된 이미지 정보
	if(!iImgWidth || !iImgHeight) return;
	
	if((iWidth >= iImgWidth) && (iHeight >= iImgHeight)) {
		// 이미지의 크기가 영역보다 작은 경우
		lpInfo->iXOffs = (iWidth - iImgWidth) / 2;
		lpInfo->iYOffs = (iHeight - iImgHeight) / 2;
		lpInfo->iDrawWidth  = iImgWidth;
		lpInfo->iDrawHeight = iImgHeight;
	}
	else {
		int		iDrawWidth, iDrawHeight;
		double	dXRatio, dYRatio;

		dXRatio = (double)iWidth  / (double)iImgWidth;
		dYRatio = (double)iHeight / (double)iImgHeight;
		
		if(dXRatio > dYRatio) {
			iDrawWidth  = (int)((iImgWidth  * dYRatio) + 0.5);
			iDrawHeight = (int)((iImgHeight * dYRatio) + 0.5);
		}
		else {
			iDrawWidth  = (int)((iImgWidth  * dXRatio) + 0.5);
			iDrawHeight = (int)((iImgHeight * dXRatio) + 0.5);
		}

		lpInfo->iXOffs = (iWidth  - iDrawWidth)  / 2;
		lpInfo->iYOffs = (iHeight - iDrawHeight) / 2;
		lpInfo->iDrawWidth  = iDrawWidth;
		lpInfo->iDrawHeight = iDrawHeight;
	}
}

// 배경이미지 로드 및 정보세팅
static void	LoadUIBgBitmap(HWND hWnd, int idx)
{
	HGLOBAL hDIBbg;
	BGINFO	*lpInfo;
	HDC		hDC;

	if((idx<0) || (idx>=BGINFO_MAX)) return;
	
	hDC = GetDC(hWnd);
	
	ResetUIBgBitmapInfo(idx);
	
	lpInfo = &_ExUIOpt.bgData[idx];
	
	if (LoadDIB(lpInfo->szFileName, &hDIBbg, &lpInfo->hPal, BACKGROUND))
	{
		lpInfo->hBmp = DIBToDDB(hDC, hDIBbg, &lpInfo->info);
		GlobalFree(hDIBbg);
		
		if(lpInfo->hBmp) CalcBgDrawSize(idx);
	}
	else {
		lpInfo->hBmp = 0;
		lpInfo->hPal = 0;
	}
	
	ReleaseDC(hWnd, hDC);
}

static void ChangeBackgroundImage(HWND hWnd, int idx)
{
	OPENFILENAMEA OpenFileName;
	static char szFile[MAX_PATH] = "\0";

	if((idx<0) || (idx>=BGINFO_MAX)) return;
	
	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner         = hWnd;
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
#ifdef KOREAN
	OpenFileName.lpstrTitle        = "배경이미지 선택";
#else	
	OpenFileName.lpstrTitle        = "Select a Background Image";
#endif	
	OpenFileName.nFileOffset       = 0;
	OpenFileName.nFileExtension    = 0;
	OpenFileName.lpstrDefExt       = NULL;
	OpenFileName.lCustData         = 0;                                                 
	OpenFileName.lpfnHook 		   = NULL;
	OpenFileName.lpTemplateName    = NULL;                                    
	OpenFileName.Flags             = OFN_NOCHANGEDIR | OFN_SHOWHELP | OFN_EXPLORER;

	if (GetOpenFileNameA(&OpenFileName))
	{
		char szDestFile[MAX_PATH];

		sprintf(szDestFile, "%s\\%s.png", GetBgDir(), _ExUIOpt.bgData[idx].szFileName );
		SetFileAttributesA(szDestFile, FILE_ATTRIBUTE_NORMAL);
		CopyFileA(szFile, szDestFile, FALSE);

		LoadUIBgBitmap(hWnd, idx);
		
		InvalidateRect(hWnd, NULL, TRUE);
	}
}

static int DeleteBackgroundImage(HWND hWnd, int idx)
{
	char szDestFile[MAX_PATH];
	int  i;
	
	if((idx<0) || (idx>=BGINFO_MAX)) return 0;

	if(!_ExUIOpt.bgData[idx].hBmp) return 0;
	
#ifdef KOREAN
	i = MessageBox(hWnd, "해당 배경이미지를 삭제하시겠습니까?\n\n'예'를 선택하면 파일이 완전히 삭제됩니다.", "배경이미지 삭제", MB_YESNO|MB_ICONQUESTION);
#else	
	i = MessageBoxA(hWnd, "Remove background image. Are you sure?\n\nThe background image file will be removed from your disks.", "Delete Background image", MB_YESNO|MB_ICONQUESTION);
#endif
	if(i != IDYES) return 0;
	
	sprintf(szDestFile, "%s\\%s.png", GetBgDir(), _ExUIOpt.bgData[idx].szFileName );
	SetFileAttributesA(szDestFile, FILE_ATTRIBUTE_NORMAL);
	DeleteFileA(szDestFile);

#ifdef KOREAN
	MessageBox(hWnd, "삭제하였습니다.", "안내", MB_OK);
#else	
	MessageBoxA(hWnd, "The background image file deleted.", "NOTICE", MB_OK);
#endif
	return 1;
}

INT_PTR CALLBACK InterfaceExDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG:
		CenterWindow(hDlg);
		
		{
			char szText[256];
			int  i;
			RECT rc;
			
			GetListFont(&_ExUIOpt.listFont);
			_ExUIOpt.ListColor  = GetListFontColor();
			_ExUIOpt.CloneColor = GetListCloneColor();
			
			GetHistFont(&_ExUIOpt.histFont);
			_ExUIOpt.histColor  = GetHistFontColor();
#if 0			
			GetChatFont(&_ExUIOpt.chatFont);
			_ExUIOpt.chatColor  = GetChatFontColor();
#endif			
			GetWindowRect( GetDlgItem(hDlg,IDC_STATIC_FONTCOLOR),  &rc );
			rc.left+=2; rc.top+=2; rc.right-=3; rc.bottom-=3;
			ScreenToClient(hDlg, (LPPOINT)&rc);	ScreenToClient(hDlg, ((LPPOINT)&rc)+1);
			memcpy( &_ExUIOpt.rcList, &rc, sizeof(RECT) );

			GetWindowRect( GetDlgItem(hDlg,IDC_STATIC_CLONECOLOR), &rc );
			rc.left+=2; rc.top+=2; rc.right-=3; rc.bottom-=3;
			ScreenToClient(hDlg, (LPPOINT)&rc);	ScreenToClient(hDlg, ((LPPOINT)&rc)+1);
			memcpy( &_ExUIOpt.rcClone, &rc, sizeof(RECT) );

			sprintf( szText, "%s, (%ld %ld)", _ExUIOpt.listFont.lfFaceName, _ExUIOpt.listFont.lfHeight, _ExUIOpt.listFont.lfWidth );
			SetDlgItemTextA(hDlg, IDC_STATIC_MAINFONT, szText);
			
			GetWindowRect( GetDlgItem(hDlg,IDC_STATIC_HIST_FONTCOLOR), &rc );
			rc.left+=2; rc.top+=2; rc.right-=3; rc.bottom-=3;
			ScreenToClient(hDlg, (LPPOINT)&rc);	ScreenToClient(hDlg, ((LPPOINT)&rc)+1);
			memcpy( &_ExUIOpt.rcHist, &rc, sizeof(RECT) );

			sprintf( szText, "%s, (%ld %ld)", _ExUIOpt.histFont.lfFaceName, _ExUIOpt.histFont.lfHeight, _ExUIOpt.histFont.lfWidth );
			SetDlgItemTextA(hDlg, IDC_STATIC_HISTORYFONT, szText);

#ifdef KAILLERA
			GetWindowRect( GetDlgItem(hDlg,IDC_STATIC_CHAT_FONTCOLOR), &rc );
			rc.left+=2; rc.top+=2; rc.right-=3; rc.bottom-=3;
			ScreenToClient(hDlg, (LPPOINT)&rc);	ScreenToClient(hDlg, ((LPPOINT)&rc)+1);
			memcpy( &_ExUIOpt.rcChat, &rc, sizeof(RECT) );

			sprintf( szText, "%s, (%ld %ld)", _ExUIOpt.chatFont.lfFaceName, _ExUIOpt.chatFont.lfHeight, _ExUIOpt.chatFont.lfWidth );
			SetDlgItemText(hDlg, IDC_STATIC_CHATFONT, szText);
#endif
			for(i=0; i<BGINFO_MAX; i++) {
				memset( &_ExUIOpt.bgData[i], 0, sizeof(BGINFO) );
			}
			
			sprintf(_ExUIOpt.bgData[BGINFO_FOLDER].szFileName, 	"bg_folder");
			sprintf(_ExUIOpt.bgData[BGINFO_LIST].szFileName, 	"bkground");
			sprintf(_ExUIOpt.bgData[BGINFO_SNAPSHOT].szFileName,"bg_snap");

			GetWindowRect( GetDlgItem(hDlg,IDC_STATIC_BG_FOLDER), &rc  );
			rc.left+=2; rc.top+=2; rc.right-=3; rc.bottom-=3;
			ScreenToClient(hDlg, (LPPOINT)&rc);	ScreenToClient(hDlg, ((LPPOINT)&rc)+1);
			memcpy( &_ExUIOpt.bgData[BGINFO_FOLDER].rc, &rc, sizeof(RECT) );

			GetWindowRect( GetDlgItem(hDlg,IDC_STATIC_BG_LISTVIEW), &rc  );
			rc.left+=2; rc.top+=2; rc.right-=3; rc.bottom-=3;
			ScreenToClient(hDlg, (LPPOINT)&rc);	ScreenToClient(hDlg, ((LPPOINT)&rc)+1);
			memcpy( &_ExUIOpt.bgData[BGINFO_LIST].rc, &rc, sizeof(RECT) );

			GetWindowRect( GetDlgItem(hDlg,IDC_STATIC_BG_SNAPSHOT), &rc  );
			rc.left+=2; rc.top+=2; rc.right-=3; rc.bottom-=3;
			ScreenToClient(hDlg, (LPPOINT)&rc);	ScreenToClient(hDlg, ((LPPOINT)&rc)+1);
			memcpy( &_ExUIOpt.bgData[BGINFO_SNAPSHOT].rc, &rc, sizeof(RECT) );
			
			for(i=0; i<BGINFO_MAX; i++) LoadUIBgBitmap(hDlg, i);
		}
		//return TRUE;
		break;

	case WM_HELP:
		/* User clicked the ? from the upper right on a control */
		break;

	case WM_CONTEXTMENU:
		break;

	case WM_HSCROLL:
		HANDLE_WM_HSCROLL(hDlg, wParam, lParam, OnHScroll);
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC			hDC, hMemDC;
			RECT		rc;
			int			i;
			COLORREF	crTemp;

			// 페인트 시작	
			BeginPaint (hDlg, &ps);
			hDC = ps.hdc;
			
			memcpy( &rc, &_ExUIOpt.rcList, sizeof(RECT) );
			crTemp = SetBkColor(hDC, _ExUIOpt.ListColor);
			ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

			//---------
			memcpy( &rc, &_ExUIOpt.rcClone, sizeof(RECT) );
			SetBkColor(hDC, _ExUIOpt.CloneColor);
			ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

			//---------
			memcpy( &rc, &_ExUIOpt.rcHist, sizeof(RECT) );
			SetBkColor(hDC, _ExUIOpt.histColor);
			ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

#ifdef KAILLERA
			//---------
			memcpy( &rc, &_ExUIOpt.rcChat, sizeof(RECT) );
			SetBkColor(hDC, _ExUIOpt.chatColor);
			ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
#endif			
			SetBkColor(hDC, crTemp);

			hMemDC = CreateCompatibleDC(hDC);
			for(i=0; i<BGINFO_MAX; i++) 
			{
				BGINFO *lpInfo = &_ExUIOpt.bgData[i];
				
				if (lpInfo->hBmp)
				{
					HBITMAP hOldBmp = SelectObject(hMemDC, lpInfo->hBmp);
					
					SetStretchBltMode( hDC,STRETCH_HALFTONE );
					StretchBlt( hDC, 
								(lpInfo->rc.left + lpInfo->iXOffs),
								(lpInfo->rc.top + lpInfo->iYOffs),
								lpInfo->iDrawWidth, 
								lpInfo->iDrawHeight,
							    hMemDC,0,0,
							    lpInfo->info.bmWidth, lpInfo->info.bmHeight, SRCCOPY);
					
					SelectObject(hMemDC, hOldBmp);
				}
				else {
					crTemp = SetBkColor(hDC, GetSysColor(COLOR_BTNFACE) );
					ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &lpInfo->rc, NULL, 0, NULL);
					SetBkColor(hDC, crTemp);
				}
			}
			DeleteDC(hMemDC);
			
			// 페인트 끝
			EndPaint (hDlg, &ps); 
		}
		break;
		
	case WM_COMMAND :
		switch (GET_WM_COMMAND_ID(wParam, lParam))
		{
		case IDC_BTN_CHANGEFONT:	// 기본 폰트 및 색상
		{
			PickUIFont(hDlg, 0);
			return TRUE;
		}
		case IDC_BTN_CHANGE_CLONECOLOR:	// 클론 색상
		{
		 	CHOOSECOLOR cc;
		 	COLORREF choice_colors[16];
		 	int i;
			
		 	for (i=0;i<16;i++) choice_colors[i] = GetCustomColor(i);
		 
		 	cc.lStructSize  = sizeof(CHOOSECOLOR);
		 	cc.hwndOwner    = hDlg;
		 	cc.rgbResult    = _ExUIOpt.CloneColor;
		 	cc.lpCustColors = choice_colors;
		 	cc.Flags        = CC_ANYCOLOR | CC_RGBINIT | CC_SOLIDCOLOR | CC_FULLOPEN;
		 	
		 	if (ChooseColor(&cc)) 
		 	{
		 		for (i=0;i<16;i++) SetCustomColor(i,choice_colors[i]);
		 		_ExUIOpt.CloneColor = cc.rgbResult;
	
				InvalidateRect( hDlg, &_ExUIOpt.rcClone, FALSE );
		 	}
			
			return TRUE;
		}
		case IDC_BTN_HIST_CHANGEFONT:	// HISTORY 폰트 및 색상
		{
			PickUIFont(hDlg, 1);
			return TRUE;
		}
#ifdef KAILLERA		
		case IDC_BTN_CHAT_CHANGEFONT:	// KAILLERA CHAT 폰트 및 색상
		{
			PickUIFont(hDlg, 2);
			return TRUE;
		}
#endif

		case IDC_BTN_CHANGE_BGFOLDER:	// Folder 배경이미지 변경
		{
			ChangeBackgroundImage(hDlg, BGINFO_FOLDER);
			return TRUE;
		}
		case IDC_BTN_CHANGE_BGLISTVIEW:	// 게임목록 배경이미지 변경		
		{
			ChangeBackgroundImage(hDlg, BGINFO_LIST);
			return TRUE;
		}
		case IDC_BTN_CHANGE_BGSNAPSHOT:	// Snapshot 배경이미지 변경
		{
			ChangeBackgroundImage(hDlg, BGINFO_SNAPSHOT);
			return TRUE;
		}

		case IDC_BTN_DEL_FOLDERBG:	// [삭제] Folder 배경이미지 변경
			if( DeleteBackgroundImage(hDlg, BGINFO_FOLDER) )
			{
				LoadUIBgBitmap(hDlg, BGINFO_FOLDER);
				InvalidateRect(hDlg, NULL, TRUE);
			}
			return TRUE;
			
		case IDC_BTN_DEL_LISTBG:	// [삭제] 게임목록 배경이미지 변경		
			if( DeleteBackgroundImage(hDlg, BGINFO_LIST) )
			{
				LoadUIBgBitmap(hDlg, BGINFO_LIST);
				InvalidateRect(hDlg, NULL, TRUE);
			}
			return TRUE;
		case IDC_BTN_DEL_SNAPSHOTBG:// [삭제] Snapshot 배경이미지 변경
			if( DeleteBackgroundImage(hDlg, BGINFO_SNAPSHOT) )
			{
				LoadUIBgBitmap(hDlg, BGINFO_SNAPSHOT);
				InvalidateRect(hDlg, NULL, TRUE);
			}
			return TRUE;
		
		case IDOK :
		{
			SetListFont(&_ExUIOpt.listFont);
			SetListFontColor(_ExUIOpt.ListColor);
			SetListCloneColor(_ExUIOpt.CloneColor);

			SetHistFont(&_ExUIOpt.histFont);
			SetHistFontColor(_ExUIOpt.histColor);

#ifdef KAILLERA		
			SetChatFont(&_ExUIOpt.chatFont);
			SetChatFontColor(_ExUIOpt.chatColor);
#endif

			ResetUIBgBitmapInfo(-1);
			EndDialog(hDlg, 1);
			return TRUE;
		}
		case IDCANCEL :
			ResetUIBgBitmapInfo(-1);
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return 0;
}
#endif /* DARKMAME */

INT_PTR CALLBACK FilterDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static DWORD			dwFilters;
	static DWORD			dwpFilters;
	static LPFOLDERDATA		lpFilterRecord;
	char strText[250];
	int 					i;

	switch (Msg)
	{
	case WM_INITDIALOG:
	{
		LPTREEFOLDER folder = GetCurrentFolder();
		LPTREEFOLDER lpParent = NULL;
		LPFILTER_ITEM g_lpFilterList = GetFilterList();

		TranslateDialog(hDlg, lParam, FALSE);
		dwFilters = 0;

		if (folder != NULL)
		{
			char tmp[80];
			
			Edit_SetText(GetDlgItem(hDlg, IDC_FILTER_EDIT), _Unicode(g_FilterText));
			Edit_SetSel(GetDlgItem(hDlg, IDC_FILTER_EDIT), 0, -1);
			// Mask out non filter flags
			dwFilters = folder->m_dwFlags & F_MASK;
			// Display current folder name in dialog titlebar
			snprintf(tmp,sizeof(tmp), _UI("Filters for %s Folder"),folder->m_lpTitle);
			SetWindowText(hDlg, _Unicode(tmp));

			if ( GetFilterInherit() )
			{
				if( folder )
				{
					BOOL bShowExplanation = FALSE;
					lpParent = GetFolder( folder->m_nParent );
					if( lpParent )
					{
						/* Check the Parent Filters and inherit them on child,
						 * No need to promote all games to parent folder, works as is */
						dwpFilters = lpParent->m_dwFlags & F_MASK;
						/*Check all possible Filters if inherited solely from parent, e.g. not being set explicitly on our folder*/
						if( (dwpFilters & F_CLONES) && !(dwFilters & F_CLONES) )
						{
							/*Add a Specifier to the Checkbox to show it was inherited from the parent*/
							Edit_GetTextA(GetDlgItem(hDlg, IDC_FILTER_CLONES), strText, 250);
							strcat(strText, " (*)");
							Edit_SetTextA(GetDlgItem(hDlg, IDC_FILTER_CLONES), strText);
							bShowExplanation = TRUE;
						}
						if( (dwpFilters & F_NONWORKING) && !(dwFilters & F_NONWORKING) )
						{
							/*Add a Specifier to the Checkbox to show it was inherited from the parent*/
							Edit_GetTextA(GetDlgItem(hDlg, IDC_FILTER_NONWORKING), strText, 250);
							strcat(strText, " (*)");
							Edit_SetTextA(GetDlgItem(hDlg, IDC_FILTER_NONWORKING), strText);
							bShowExplanation = TRUE;
						}
						if( (dwpFilters & F_UNAVAILABLE) && !(dwFilters & F_UNAVAILABLE) )
						{
							/*Add a Specifier to the Checkbox to show it was inherited from the parent*/
							Edit_GetTextA(GetDlgItem(hDlg, IDC_FILTER_UNAVAILABLE), strText, 250);
							strcat(strText, " (*)");
							Edit_SetTextA(GetDlgItem(hDlg, IDC_FILTER_UNAVAILABLE), strText);
							bShowExplanation = TRUE;
						}
						if( (dwpFilters & F_VECTOR) && !(dwFilters & F_VECTOR) )
						{
							/*Add a Specifier to the Checkbox to show it was inherited from the parent*/
							Edit_GetTextA(GetDlgItem(hDlg, IDC_FILTER_VECTOR), strText, 250);
							strcat(strText, " (*)");
							Edit_SetTextA(GetDlgItem(hDlg, IDC_FILTER_VECTOR), strText);
							bShowExplanation = TRUE;
						}
						if( (dwpFilters & F_RASTER) && !(dwFilters & F_RASTER) )
						{
							/*Add a Specifier to the Checkbox to show it was inherited from the parent*/
							Edit_GetTextA(GetDlgItem(hDlg, IDC_FILTER_RASTER), strText, 250);
							strcat(strText, " (*)");
							Edit_SetTextA(GetDlgItem(hDlg, IDC_FILTER_RASTER), strText);
							bShowExplanation = TRUE;
						}
						if( (dwpFilters & F_ORIGINALS) && !(dwFilters & F_ORIGINALS) )
						{
							/*Add a Specifier to the Checkbox to show it was inherited from the parent*/
							Edit_GetTextA(GetDlgItem(hDlg, IDC_FILTER_ORIGINALS), strText, 250);
							strcat(strText, " (*)");
							Edit_SetTextA(GetDlgItem(hDlg, IDC_FILTER_ORIGINALS), strText);
							bShowExplanation = TRUE;
						}
						if( (dwpFilters & F_WORKING) && !(dwFilters & F_WORKING) )
						{
							/*Add a Specifier to the Checkbox to show it was inherited from the parent*/
							Edit_GetTextA(GetDlgItem(hDlg, IDC_FILTER_WORKING), strText, 250);
							strcat(strText, " (*)");
							Edit_SetTextA(GetDlgItem(hDlg, IDC_FILTER_WORKING), strText);
							bShowExplanation = TRUE;
						}
						if( (dwpFilters & F_AVAILABLE) && !(dwFilters & F_AVAILABLE) )
						{
							/*Add a Specifier to the Checkbox to show it was inherited from the parent*/
							Edit_GetTextA(GetDlgItem(hDlg, IDC_FILTER_AVAILABLE), strText, 250);
							strcat(strText, " (*)");
							Edit_SetTextA(GetDlgItem(hDlg, IDC_FILTER_AVAILABLE), strText);
							bShowExplanation = TRUE;
						}
						/*Do not or in the Values of the parent, so that the values of the folder still can be set*/
						//dwFilters |= dwpFilters;
					}
					if( ! bShowExplanation )
					{
						ShowWindow(GetDlgItem(hDlg, IDC_INHERITED), FALSE );
					}
				}
			}
			else
			{
				ShowWindow(GetDlgItem(hDlg, IDC_INHERITED), FALSE );
			}

			// Find the matching filter record if it exists
			lpFilterRecord = FindFilter(folder->m_nFolderId);

			// initialize and disable appropriate controls
			for (i = 0; g_lpFilterList[i].m_dwFilterType; i++)
			{
				DisableFilterControls(hDlg, lpFilterRecord, &g_lpFilterList[i], dwFilters);
			}
		}
		SetFocus(GetDlgItem(hDlg, IDC_FILTER_EDIT));
		return FALSE;
	}
	case WM_HELP:
		// User clicked the ? from the upper right on a control
		HelpFunction(((LPHELPINFO)lParam)->hItemHandle, MAME32CONTEXTHELP,
					 HH_TP_HELP_WM_HELP, GetHelpIDs());
		break;

	case WM_CONTEXTMENU:
		HelpFunction((HWND)wParam, MAME32CONTEXTHELP, HH_TP_HELP_CONTEXTMENU, GetHelpIDs());
		break;

	case WM_COMMAND:
	{
		WORD wID		 = GET_WM_COMMAND_ID(wParam, lParam);
		WORD wNotifyCode = GET_WM_COMMAND_CMD(wParam, lParam);
		LPTREEFOLDER folder = GetCurrentFolder();
		LPFILTER_ITEM g_lpFilterList = GetFilterList();

		switch (wID)
		{
		case IDOK:
			dwFilters = 0;
			
			{
				WCHAR buf[FILTERTEXT_LEN];
				Edit_GetText(GetDlgItem(hDlg, IDC_FILTER_EDIT), buf, FILTERTEXT_LEN);
				strncpy(g_FilterText, _String(buf), FILTERTEXT_LEN);
			}
			
			// see which buttons are checked
			for (i = 0; g_lpFilterList[i].m_dwFilterType; i++)
			{
				if (Button_GetCheck(GetDlgItem(hDlg, g_lpFilterList[i].m_dwCtrlID)))
					dwFilters |= g_lpFilterList[i].m_dwFilterType;
			}

			// Mask out invalid filters
			dwFilters = ValidateFilters(lpFilterRecord, dwFilters);

			// Keep non filter flags
			folder->m_dwFlags &= ~F_MASK;

			// put in the set filters
			folder->m_dwFlags |= dwFilters;

		    // Save the filters to the ini file
		    SetFolderFlags(folder->m_lpOriginalTitle?
				               folder->m_lpOriginalTitle:
				               folder->m_lpTitle, dwFilters);

			EndDialog(hDlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
			
		default:
			// Handle unchecking mutually exclusive filters
			if (wNotifyCode == BN_CLICKED)
				EnableFilterExclusions(hDlg, wID);
		}
	}
	break;
	}
	return 0;
}

INT_PTR CALLBACK AboutDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, FALSE);

		{
			HBITMAP hBmp;
			hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL),
									  MAKEINTRESOURCE(IDB_ABOUT),
									  IMAGE_BITMAP, 0, 0, LR_SHARED);
			SendDlgItemMessageA(hDlg, IDC_ABOUT, STM_SETIMAGE,
						(WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
			Static_SetText(GetDlgItem(hDlg, IDC_VERSION), _Unicode(GetVersionString()));
		}
		return 1;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_HOMEPAGE)
		ShellExecuteA(hMain,NULL,"http://cafe.naver.com/emulk",NULL,NULL,SW_SHOWNORMAL);
		else		
		EndDialog(hDlg, 0);
		return 1;
	}
	return 0;
}

INT_PTR CALLBACK AddCustomFileDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    static LPTREEFOLDER default_selection = NULL;
	static int driver_index;

	switch (Msg)
	{
	case WM_INITDIALOG:
	{
	    TREEFOLDER **folders;
		int num_folders;
		int i;
		TVINSERTSTRUCT tvis;
		TVITEM tvi;
		BOOL first_entry = TRUE;
		HIMAGELIST treeview_icons = GetTreeViewIconList();
		static HFONT hFont;
		LOGFONTA logfontA;

		TranslateDialog(hDlg, lParam, TRUE);

		if (hFont != NULL)
			DeleteObject(hFont);

		GetTranslatedFont(&logfontA);
		hFont = TranslateCreateFont(&logfontA);

		SetWindowFont(GetDlgItem(hDlg,IDC_CUSTOM_TREE), hFont, FALSE);

		// current game passed in using DialogBoxParam()
		driver_index = lParam;

		TreeView_SetImageList(GetDlgItem(hDlg,IDC_CUSTOM_TREE), treeview_icons, LVSIL_NORMAL);

		GetFolders(&folders,&num_folders);

		// should add "New..."

		// insert custom folders into our tree view
		for (i=0;i<num_folders;i++)
		{
		    if (folders[i]->m_dwFlags & F_CUSTOM)
			{
			    HTREEITEM hti;
				int jj;

				if (folders[i]->m_nParent == -1)
				{
				    tvis.hParent = TVI_ROOT;
					tvis.hInsertAfter = TVI_SORT;
					tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					tvi.pszText = _Unicode(folders[i]->m_lpTitle);
					tvi.lParam = (LPARAM)folders[i];
					tvi.iImage = GetTreeViewIconIndex(folders[i]->m_nIconId);
					tvi.iSelectedImage = 0;
#if !defined(NONAMELESSUNION)
					tvis.item = tvi;
#else
					tvis.DUMMYUNIONNAME.item = tvi;
#endif
					
					hti = TreeView_InsertItem(GetDlgItem(hDlg,IDC_CUSTOM_TREE),&tvis);

					/* look for children of this custom folder */
					for (jj=0;jj<num_folders;jj++)
					{
					    if (folders[jj]->m_nParent == i)
						{
						    HTREEITEM hti_child;
						    tvis.hParent = hti;
							tvis.hInsertAfter = TVI_SORT;
							tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
							tvi.pszText = _Unicode(folders[jj]->m_lpTitle);
							tvi.lParam = (LPARAM)folders[jj];
							tvi.iImage = GetTreeViewIconIndex(folders[jj]->m_nIconId);
							tvi.iSelectedImage = 0;
#if !defined(NONAMELESSUNION)
					        tvis.item = tvi;
#else
					        tvis.DUMMYUNIONNAME.item = tvi;
#endif							
							hti_child = TreeView_InsertItem(GetDlgItem(hDlg,IDC_CUSTOM_TREE),&tvis);
							if (folders[jj] == default_selection)
							    TreeView_SelectItem(GetDlgItem(hDlg,IDC_CUSTOM_TREE),hti_child);
						}
					}

					/*TreeView_Expand(GetDlgItem(hDlg,IDC_CUSTOM_TREE),hti,TVE_EXPAND);*/
					if (first_entry || folders[i] == default_selection)
					{
					    TreeView_SelectItem(GetDlgItem(hDlg,IDC_CUSTOM_TREE),hti);
						first_entry = FALSE;
					}

				}
				
			}
		}

		SetWindowText(GetDlgItem(hDlg,IDC_CUSTOMFILE_GAME),
		              _Unicode(UseLangList() ? _LST(drivers[driver_index]->description)
		                                     : ModifyThe(drivers[driver_index]->description)));

		return TRUE;
	}
	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam, lParam))
		{
		case IDOK:
		{
		   TVITEM tvi;
		   tvi.hItem = TreeView_GetSelection(GetDlgItem(hDlg,IDC_CUSTOM_TREE));
		   tvi.mask = TVIF_PARAM;
		   if (TreeView_GetItem(GetDlgItem(hDlg,IDC_CUSTOM_TREE),&tvi) == TRUE)
		   {
			  /* should look for New... */
		   
			  default_selection = (LPTREEFOLDER)tvi.lParam; /* start here next time */

			  AddToCustomFolder((LPTREEFOLDER)tvi.lParam,driver_index);
		   }

		   EndDialog(hDlg, 0);
		   return TRUE;

		   break;
		}
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;

		}
		break;
	}
	return 0;
}

INT_PTR CALLBACK DirectXDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HWND hEdit;

	const char *directx_help =
		MAME32NAME " requires DirectX version 3 or later, which is a set of operating\r\n"
		"system extensions by Microsoft for Windows 9x, NT and 2000.\r\n\r\n"
		"Visit Microsoft's DirectX web page at http://www.microsoft.com/directx\r\n"
		"download DirectX, install it, and then run " MAME32NAME " again.\r\n";

	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, FALSE);

		hEdit = GetDlgItem(hDlg, IDC_DIRECTX_HELP);
		if (OnNT())
		Edit_SetSel(hEdit, Edit_GetTextLength(hEdit), Edit_GetTextLength(hEdit));
		else
			Edit_SetSelA(hEdit, Edit_GetTextLengthA(hEdit), Edit_GetTextLengthA(hEdit));
		Edit_ReplaceSel(hEdit, _Unicode(_UI(directx_help)));
		return 1;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDB_WEB_PAGE)
			ShellExecuteA(GetMainWindow(), NULL, "http://www.microsoft.com/directx",
						 NULL, NULL, SW_SHOWNORMAL);

		if (LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDB_WEB_PAGE)
			EndDialog(hDlg, 0);
		return 1;
	}
	return 0;
}

/***************************************************************************
    private functions
 ***************************************************************************/

static void DisableFilterControls(HWND hWnd, LPFOLDERDATA lpFilterRecord,
								  LPFILTER_ITEM lpFilterItem, DWORD dwFlags)
{
	HWND  hWndCtrl = GetDlgItem(hWnd, lpFilterItem->m_dwCtrlID);
	DWORD dwFilterType = lpFilterItem->m_dwFilterType;

	/* Check the appropriate control */
	if (dwFilterType & dwFlags)
		Button_SetCheck(hWndCtrl, MF_CHECKED);

	/* No special rules for this folder? */
	if (!lpFilterRecord)
		return;

	/* If this is an excluded filter */
	if (lpFilterRecord->m_dwUnset & dwFilterType)
	{
		/* uncheck it and disable the control */
		Button_SetCheck(hWndCtrl, MF_UNCHECKED);
		EnableWindow(hWndCtrl, FALSE);
	}

	/* If this is an implied filter, check it and disable the control */
	if (lpFilterRecord->m_dwSet & dwFilterType)
	{
		Button_SetCheck(hWndCtrl, MF_CHECKED);
		EnableWindow(hWndCtrl, FALSE);
	}
}

// Handle disabling mutually exclusive controls
static void EnableFilterExclusions(HWND hWnd, DWORD dwCtrlID)
{
	int 	i;
	DWORD	id;

	for (i = 0; i < NUM_EXCLUSIONS; i++)
	{
		// is this control in the list?
		if (filterExclusion[i] == dwCtrlID)
		{
			// found the control id
			break;
		}
	}

	// if the control was found
	if (i < NUM_EXCLUSIONS)
	{
		// find the opposing control id
		if (i % 2)
			id = filterExclusion[i - 1];
		else
			id = filterExclusion[i + 1];

		// Uncheck the other control
		Button_SetCheck(GetDlgItem(hWnd, id), MF_UNCHECKED);
	}
}

// Validate filter setting, mask out inappropriate filters for this folder
static DWORD ValidateFilters(LPFOLDERDATA lpFilterRecord, DWORD dwFlags)
{
	DWORD dwFilters;

	if (lpFilterRecord != (LPFOLDERDATA)0)
	{
		// Mask out implied and excluded filters
		dwFilters = lpFilterRecord->m_dwSet | lpFilterRecord->m_dwUnset;
		return dwFlags & ~dwFilters;
	}

	// No special cases - all filters apply
	return dwFlags;
}

static void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	int value;
	char tmp[4];
	if (hwndCtl == GetDlgItem(hwnd, IDC_CYCLETIMESEC))
	{
		value = SendDlgItemMessageA(hwnd,IDC_CYCLETIMESEC, TBM_GETPOS, 0, 0);
		itoa(value,tmp,10);
		SendDlgItemMessageA(hwnd,IDC_CYCLETIMESECTXT,WM_SETTEXT,0, (WPARAM)tmp);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_SCREENSHOT_BORDERSIZE))
	{
		value = SendDlgItemMessageA(hwnd,IDC_SCREENSHOT_BORDERSIZE, TBM_GETPOS, 0, 0);
		itoa(value,tmp,10);
		SendDlgItemMessageA(hwnd,IDC_SCREENSHOT_BORDERSIZETXT,WM_SETTEXT,0, (WPARAM)tmp);
	}
}
