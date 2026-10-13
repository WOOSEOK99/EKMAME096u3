#ifdef KAILLERA
#include <driver.h>
#include <windows.h>
#include <windowsx.h>
#include <imm.h>
#include "kailleraclient.h"
#include "windows/window.h"
#include "options.h"
#include "chat.h"

/***************************************************************************
    Macros and definitions
 ***************************************************************************/

typedef BOOL (WINAPI *ime_proc)(HWND hWnd, BOOL bflag);

/***************************************************************************
    Function prototypes
 ***************************************************************************/

static LRESULT CALLBACK EditProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void             KailleraChatCloseChat(void);
void KailleraChatAdjustWindow(int width, int height);
extern void			osd_chat_message(const char *str);

/***************************************************************************
    External variables
 ***************************************************************************/

int nChatDrawMode;
int nEndChat;
HWND	hChat		= NULL;

/***************************************************************************
    Internal variables
 ***************************************************************************/

static HANDLE          hUser32DLL;
static int	nChatHeight	= 0;

static BOOL            bChatActive;
static BOOL            bShowLog;

static char            szRecvBuf[KC_BUFFER_MAX * 4];
static char            szInputBuf[KC_BUFFER_MAX];

static ime_proc        _WINNLSEnableIME;
static WNDPROC         DefaultWindowProc;

/***************************************************************************
    External functions
 ***************************************************************************/

void KailleraChatInit(void)
{
	extern HFONT GetKailleraChatFont(void);

	HFONT		hFont = GetKailleraChatFont();
	HFONT		hOldFont;
	HDC 		hDC;
	TEXTMETRIC	tm;
	UINT error_mode;

	bChatActive   = FALSE;
	nChatDrawMode = GetChatBG();
	bShowLog      = FALSE;
	nEndChat	  = 0;

	ZeroMemory(szRecvBuf, KC_BUFFER_MAX * 4);
	ZeroMemory(szInputBuf, KC_BUFFER_MAX);

	hChat = CreateWindowEx(WS_EX_TOPMOST,
						   "EDIT", "",
						   WS_CHILD | ES_AUTOHSCROLL,
						   0, 0, 0, 0,
						   win_video_window,
						   NULL,
						   GetModuleHandle(NULL),
						   NULL);

	SetWindowFont(hChat, hFont, FALSE);

	hDC = GetDC(hChat);
	hOldFont = SelectObject(hDC, hFont);
	GetTextMetrics(hDC, &tm);
	SelectObject(hDC, hOldFont);
	ReleaseDC(hChat, hDC);

	nChatHeight = tm.tmHeight + tm.tmExternalLeading + 2;

	Edit_LimitText(hChat, KC_BUFFER_MAX - 1);
	Edit_Enable(hChat, FALSE);

	error_mode = SetErrorMode(0);
	hUser32DLL = LoadLibrary("user32.dll");
	SetErrorMode(error_mode);

	if (hUser32DLL != NULL)
		_WINNLSEnableIME = (ime_proc)GetProcAddress(hUser32DLL, "WINNLSEnableIME");

	if (_WINNLSEnableIME == NULL)
	{
		FreeLibrary(hUser32DLL);
		hUser32DLL = NULL;
	}

	if (_WINNLSEnableIME)
		_WINNLSEnableIME(win_video_window, FALSE);

	DefaultWindowProc = (WNDPROC)GetWindowLongPtr(hChat, GWL_WNDPROC);
	SetWindowLongPtr(hChat, GWL_WNDPROC, (LONG)EditProc);
}

void KailleraChatExit(void)
{
	bChatActive   = FALSE;
	bShowLog      = FALSE;
	nEndChat	  = 0;

	if(hChat) {
		if(DefaultWindowProc) {
			SetWindowLongPtr(hChat, GWL_WNDPROC, (LONG)DefaultWindowProc);
			DefaultWindowProc = NULL;
		}

		DestroyWindow(hChat);
		hChat = NULL;
		nChatHeight = 0;
	}

	if (_WINNLSEnableIME)
		_WINNLSEnableIME(win_video_window, TRUE);

	if (hUser32DLL)
	{
		FreeLibrary(hUser32DLL);
		hUser32DLL = NULL;
	}
}

int KailleraChatHeight(void)
{
	if(hChat == NULL) return 0;

	return nChatHeight;
}

void KailleraChatUpdate(struct mame_bitmap *pBitmap)
{
    if(!bChatActive && input_ui_pressed(IPT_UI_CHAT_OPEN))
    {
        Edit_Enable(hChat, TRUE);
        SetFocus(hChat);

	  if (_WINNLSEnableIME)
		_WINNLSEnableIME(hChat, TRUE);

	  schedule_full_refresh();

        bChatActive = TRUE;
    }

    if (szRecvBuf[0])
    {
        osd_chat_message(szRecvBuf);
        szRecvBuf[0] = '\0';
    }

    if(bChatActive)
    {
        SetFocus(hChat);
    }
    else if (input_ui_pressed(IPT_UI_CHAT_CHANGE_BG))
    {
        nChatDrawMode++;
        if (nChatDrawMode == 3)
            nChatDrawMode = 0;
        SetChatBG(nChatDrawMode);
    }
}

void KailleraChateReceive(char *szText)
{
    if (szRecvBuf[0])
        strcat(szRecvBuf,"\n");
    strcat(szRecvBuf, szText);
}

BOOL KailleraChatIsActive(void)
{
    return bChatActive;
}

/***************************************************************************
    Internal functions
 ***************************************************************************/

static LRESULT CALLBACK EditProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(hWnd == hChat && nChatHeight) {
		switch (Msg)
		{
		case WM_CHAR:
			switch (wParam)
			{
			case 0x0d: /* Enter */
				if (bChatActive == TRUE)
				{
					Edit_GetText(hChat, szInputBuf, KC_BUFFER_MAX - 1);

					if (szInputBuf[0])
					{
						kailleraChatSend(szInputBuf);
					}
					KailleraChatCloseChat();
				}
				return TRUE; /* disable beep */

			case 0x1b: /* Esc */
				KailleraChatCloseChat();
				nEndChat = 1;
				return TRUE; /* disable beep */
			}
			break;

		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_HOME:
				return TRUE;

			case VK_END:
				return TRUE;

			case VK_LEFT:
				return TRUE;

			case VK_RIGHT:
				return TRUE;

			case VK_INSERT:
				return TRUE;

			case VK_DELETE:
				return TRUE;
			}
			break;
		
		case WM_IME_STARTCOMPOSITION:
			if (hWnd == hChat)
			{
				LOGFONTA	logfont;
				HIMC hImc = ImmGetContext(hWnd);

				GetChatFont(&logfont);
				ImmSetCompositionFont(hImc, &logfont);
				ImmReleaseContext(hWnd, hImc);
			}
			break;

		case WM_IME_NOTIFY:
			if (wParam == 0x04 /*IMN_CLOSECANDIDATE*/)
			{
				schedule_full_refresh();
				InvalidateRect(hChat, NULL, TRUE);
			}
			break;

		case WM_CONTEXTMENU:
			return TRUE;
		}
	}

	return CallWindowProc(DefaultWindowProc, hWnd, Msg, wParam, lParam);
}

static void KailleraChatCloseChat(void)
{
    Edit_Enable(hChat, FALSE);
    Edit_SetText(hChat, "");
    szInputBuf[0] = '\0';
    bChatActive = FALSE;
    SetFocus(win_video_window);

    if (_WINNLSEnableIME)
	    _WINNLSEnableIME(hChat, FALSE);

    schedule_full_refresh();
}
#endif /* KAILLERA */
