//============================================================
//
//	win_kchat.c - Kaillera chat system
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef __GNUC__
#ifndef NONAMELESSUNION
 #define NONAMELESSUNION
#endif
#endif
#include <ddraw.h>

// standard C headers
#include <math.h>

// MAME headers
#include "driver.h"
#include "window.h"
#include "video.h"
#include "blit.h"

#include "win_kchat.h"

char ChatBuffer[4096] = "";
int	 ChatMessageTimer = 0;
int	 ChatMessageLines = 0;
BOOL bChatFirst = TRUE;
int  nBlinkCounter = 0;
BOOL bShowCursor = TRUE;



//============================================================
//	DIRECTDRAW
//============================================================

HRGN ddraw_create_chatmsg_rgn(LPDIRECTDRAWSURFACE target_surface, LPRECT rcScr)
{
	HDC		hDC;
	HFONT	hFont;
	HRGN	hRgn = NULL;
	RECT	rect;
	BOOL	bCursor;

	if(!kPlay) return hRgn; // NULL

	IDirectDrawSurface_GetDC(target_surface, &hDC);
	if( hDC == NULL ) return hRgn; // NULL

	hFont = (HFONT)SelectObject(hDC, GetKailleraChatFont());

	if( KailleraChatIsActive() ) {
		char	displayBuffer [KC_BUFFER_MAX + 10];
		char	getInputBuffer[KC_BUFFER_MAX];

		displayBuffer[0]  = '\0';
		getInputBuffer[0] = '\0';

		SetRect(&rect, rcScr->left, rcScr->bottom - KailleraChatHeight(), rcScr->right, rcScr->bottom);

		Edit_GetText(hChat, getInputBuffer, KC_BUFFER_MAX - 1);
		strcat(displayBuffer, "Chat:");
		strcat(displayBuffer, getInputBuffer);

		bCursor = bShowCursor;
		if ( (nBlinkCounter+1) > KC_BLINK_SPD)
		{
			bCursor = !bCursor;
		}
		if (bCursor) strcat(displayBuffer, "|");
		else strcat(displayBuffer, " ");

		DrawText(hDC, displayBuffer, -1, &rect, 
				 DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK|DT_CALCRECT);

		hRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
	}

	if( ChatMessageTimer && (nChatDrawMode != 2) ) {
		int	    i, j, top, fontheight;
		char	dispBuffer[KC_BUFFER_MAX + 10];
		HRGN	hTmpRgn;

		top = rcScr->top;

		i = j = 0;
		while( ChatBuffer[i] ) {
			if( ChatBuffer[i] == '\r' ) { i++; continue; }

			if( ChatBuffer[i] != '\n' ) {
				dispBuffer[j] = ChatBuffer[i];
				j++;
			}
			else {
				dispBuffer[j] = '\0';

				SetRect(&rect, rcScr->left, top, rcScr->right, top+KailleraChatHeight() );

				fontheight = DrawText(hDC, dispBuffer, -1, &rect, 
									  DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK|DT_CALCRECT);

				rect.bottom = rect.top + fontheight;
				top += fontheight;

				j = 0;
				//--------------------------------------------------------------
				if( hRgn == NULL ) {
					hRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
				}
				else {
					hTmpRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
					CombineRgn( hRgn, hRgn, hTmpRgn, RGN_OR );
					DeleteObject(hTmpRgn);
				}
				//--------------------------------------------------------------
			}

			i++;
		}

		if( j ) {
			dispBuffer[j] = '\0';

			SetRect(&rect, rcScr->left, top, rcScr->right, top+KailleraChatHeight() );

			fontheight = DrawText(hDC, dispBuffer, -1, &rect, 
								  DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK|DT_CALCRECT);

			rect.bottom = rect.top + fontheight;

			//--------------------------------------------------------------
			if( hRgn == NULL ) {
				hRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
			}
			else {
				hTmpRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
				CombineRgn( hRgn, hRgn, hTmpRgn, RGN_OR );
				DeleteObject(hTmpRgn);
			}
			//--------------------------------------------------------------
		}
	}

	SelectObject(hDC, hFont);
	IDirectDrawSurface_ReleaseDC(target_surface, hDC);

	return hRgn;
}


LPRGNDATA ddraw_get_chatmsg_rgndata(LPDIRECTDRAWSURFACE target_surface, LPRECT rcScr)
{
	LPRGNDATA region_data = NULL;
	DWORD dwSize;
	HRGN hRgn, hChatRgn;

	hChatRgn = ddraw_create_chatmsg_rgn( target_surface, rcScr );
	if( hChatRgn == NULL ) return region_data; // NULL


	hRgn = CreateRectRgn( rcScr->left, rcScr->top, rcScr->right, rcScr->bottom );

	CombineRgn( hRgn, hRgn, hChatRgn, RGN_DIFF );

	DeleteObject( hChatRgn );

	dwSize = GetRegionData( hRgn, 0, NULL ); // get required data size
	if( dwSize > 0 ) {
		region_data = (LPRGNDATA)malloc( dwSize );
		GetRegionData( hRgn, dwSize, region_data );
	}
	else {
		region_data = NULL;
	}

	DeleteObject( hRgn );

	return region_data;
}


LPDIRECTDRAWCLIPPER ddraw_create_chatclipper(LPDIRECTDRAW lpDDraw, LPDIRECTDRAWSURFACE target_surface, LPRGNDATA lpRgnData )
{
	LPDIRECTDRAWCLIPPER lpDDClipper = NULL;
	HRESULT result;

	result = IDirectDraw_CreateClipper(lpDDraw, 0, &lpDDClipper, NULL);
	if (result == DD_OK) {

		result = IDirectDrawClipper_SetClipList(lpDDClipper, lpRgnData, 0);
		if( result == DD_OK ) {
			result = IDirectDrawSurface_SetClipper(target_surface, lpDDClipper);
			if( result != DD_OK ) {
				logerror( "IDirectDrawSurface_SetClipper() Failed.(Code:%d)\n", (int)result );
				IDirectDrawClipper_SetClipList(lpDDClipper, NULL, 0);
				IDirectDrawClipper_Release(lpDDClipper);
				lpDDClipper = NULL;
			}
		}
		else {
			logerror( "IDirectDrawClipper_SetClipList() Failed.(Code:%d)\n", (int)result );
			IDirectDrawClipper_Release(lpDDClipper);
			lpDDClipper = NULL;
		}
	}
	else {
		logerror( "IDirectDraw_CreateClipper() Failed.(Code:%d)\n", (int)result );
	}

	return lpDDClipper;
}


void ddraw_destroy_chatclipper(LPDIRECTDRAWSURFACE target_surface, LPDIRECTDRAWCLIPPER lpCurClipper, LPDIRECTDRAWCLIPPER lpPrevClipper )
{
	IDirectDrawSurface_SetClipper(target_surface, NULL);

	if( lpCurClipper != NULL ) {
		IDirectDrawClipper_SetClipList(lpCurClipper, NULL, 0); // Delete Clip Lists.
		IDirectDrawClipper_Release(lpCurClipper);
	}

	IDirectDrawSurface_SetClipper(target_surface, lpPrevClipper);
}


void  ddraw_draw_kchatmsg(LPDIRECTDRAWSURFACE target_surface, LPRECT dst, BOOL bUseBackSurface)
{
	HDC		hDC;
	RECT	rect;
	HFONT	hFont;
	LPDIRECTDRAWSURFACE work_surface;

	if(!kPlay) return; // not network play mode
	if(!ChatMessageTimer && !KailleraChatIsActive()) return; // nothing to draw

	if(bUseBackSurface) {
		DDSCAPS caps = { DDSCAPS_BACKBUFFER };
		HRESULT result;

		result = IDirectDrawSurface_GetAttachedSurface(target_surface, &caps, &work_surface);
		if( result != DD_OK ) return;
	}
	else {
		work_surface = target_surface;
	}

	IDirectDrawSurface_GetDC(work_surface, &hDC);
	if( hDC == NULL ) return;

	hFont = (HFONT)SelectObject(hDC, GetKailleraChatFont());

	if(ChatMessageTimer) {
		if(nChatDrawMode == 0) {
			SetTextColor(hDC, GetChatFontColor());
			SetBkColor(hDC, RGB(255, 255, 255));
		} else if(nChatDrawMode == 1) {
			SetTextColor(hDC, GetChatFontColor());
			SetBkColor(hDC, RGB(0, 0, 0));
		} else {
			SetTextColor(hDC, GetChatFontColor());
			SetBkColor(hDC, RGB(0, 0, 0));
			SetBkMode(hDC, TRANSPARENT);
		}
		SetRect(&rect, dst->left, dst->top, dst->right, dst->bottom);
		DrawText(hDC, ChatBuffer, -1, &rect, DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK);

		ChatMessageTimer--;
		if(ChatMessageTimer == 0) {ChatMessageLines = 0; ChatBuffer[0] = '\0'; bChatFirst = TRUE;}
	}

	if( KailleraChatIsActive() ) {
		char	displayBuffer [KC_BUFFER_MAX + 10];
		char	getInputBuffer[KC_BUFFER_MAX];

		displayBuffer[0] = '\0';
		getInputBuffer[0] = '\0';

		SetTextColor(hDC, RGB(0, 0, 0));
		SetBkColor(hDC, RGB(255, 255, 255));
		SetRect(&rect, dst->left, dst->bottom - KailleraChatHeight(), dst->right, dst->bottom);

		Edit_GetText(hChat, getInputBuffer, KC_BUFFER_MAX - 1);
		strcat(displayBuffer, "Chat:");
		strcat(displayBuffer, getInputBuffer);
		nBlinkCounter++;
		if (nBlinkCounter > KC_BLINK_SPD)
		{
			nBlinkCounter = 0;
			bShowCursor = !bShowCursor;
		}
		if (bShowCursor) strcat(displayBuffer, "|");
		else strcat(displayBuffer, " ");

		DrawText(hDC, displayBuffer, -1, &rect, DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK);
	}

	SelectObject(hDC, hFont);
    IDirectDrawSurface_ReleaseDC(work_surface, hDC);
}



//============================================================
//	DIRECT3D
//============================================================

HRGN d3d_create_chatmsg_rgn(LPDIRECTDRAWSURFACE7 target_surface, LPRECT rcScr)
{
	HDC		hDC;
	HFONT	hFont;
	HRGN	hRgn = NULL;
	RECT	rect;
	BOOL	bCursor;

	if(!kPlay) return hRgn; // NULL

	IDirectDrawSurface_GetDC(target_surface, &hDC);
	if( hDC == NULL ) return hRgn; // NULL

	hFont = (HFONT)SelectObject(hDC, GetKailleraChatFont());

	if( KailleraChatIsActive() ) {
		char	displayBuffer [KC_BUFFER_MAX + 10];
		char	getInputBuffer[KC_BUFFER_MAX];

		displayBuffer[0] = '\0';
		getInputBuffer[0] = '\0';

		SetRect(&rect, rcScr->left, rcScr->bottom - KailleraChatHeight(), rcScr->right, rcScr->bottom);

		Edit_GetText(hChat, getInputBuffer, KC_BUFFER_MAX - 1);
		strcat(displayBuffer, "Chat:");
		strcat(displayBuffer, getInputBuffer);

		bCursor = bShowCursor;
		if ( (nBlinkCounter+1) > KC_BLINK_SPD)
		{
			bCursor = !bCursor;
		}
		if (bCursor) strcat(displayBuffer, "|");
		else strcat(displayBuffer, " ");

		DrawText(hDC, displayBuffer, -1, &rect, 
				 DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK|DT_CALCRECT);

		hRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
	}

    if( ChatMessageTimer && (nChatDrawMode != 2) ) {
		int	    i, j, top, fontheight;
        char	dispBuffer[KC_BUFFER_MAX + 10];
		HRGN	hTmpRgn;

		top = rcScr->top;

		i = j = 0;
		while( ChatBuffer[i] ) {
			if( ChatBuffer[i] == '\r' ) { i++; continue; }

			if( ChatBuffer[i] != '\n' ) {
				dispBuffer[j] = ChatBuffer[i];
				j++;
			}
			else {
				dispBuffer[j] = '\0';

				SetRect(&rect, rcScr->left, top, rcScr->right, top+KailleraChatHeight() );

				fontheight = DrawText(hDC, dispBuffer, -1, &rect, 
									  DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK|DT_CALCRECT);

				rect.bottom = rect.top + fontheight;
				top += fontheight;

				j = 0;
				//--------------------------------------------------------------
				if( hRgn == NULL ) {
					hRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
				}
				else {
					hTmpRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
					CombineRgn( hRgn, hRgn, hTmpRgn, RGN_OR );
					DeleteObject(hTmpRgn);
				}
				//--------------------------------------------------------------
			}

			i++;
		}

		if( j ) {
			dispBuffer[j] = '\0';

			SetRect(&rect, rcScr->left, top, rcScr->right, top+KailleraChatHeight() );

			fontheight = DrawText(hDC, dispBuffer, -1, &rect, 
								DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK|DT_CALCRECT);

			rect.bottom = rect.top + fontheight;

			//--------------------------------------------------------------
			if( hRgn == NULL ) {
				hRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
			}
			else {
				hTmpRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
				CombineRgn( hRgn, hRgn, hTmpRgn, RGN_OR );
				DeleteObject(hTmpRgn);
			}
			//--------------------------------------------------------------
		}
	}

	SelectObject(hDC, hFont);
	IDirectDrawSurface_ReleaseDC(target_surface, hDC);

	return hRgn;
}


LPRGNDATA d3d_get_chatmsg_rgndata(LPDIRECTDRAWSURFACE7 target_surface, LPRECT rcScr)
{
	LPRGNDATA region_data = NULL;
	DWORD dwSize;
	HRGN hRgn, hChatRgn;

	hChatRgn = d3d_create_chatmsg_rgn( target_surface, rcScr );
	if( hChatRgn == NULL ) return region_data; // NULL


	hRgn = CreateRectRgn( rcScr->left, rcScr->top, rcScr->right, rcScr->bottom );

	CombineRgn( hRgn, hRgn, hChatRgn, RGN_DIFF );

	DeleteObject( hChatRgn );

	dwSize = GetRegionData( hRgn, 0, NULL ); // get required data size
	if( dwSize > 0 ) {
		region_data = (LPRGNDATA)malloc( dwSize );
		GetRegionData( hRgn, dwSize, region_data );
	}
	else {
		region_data = NULL;
	}

	DeleteObject( hRgn );

	return region_data;
}


LPDIRECTDRAWCLIPPER d3d_create_chatclipper(LPDIRECTDRAW7 lpDDraw7, LPDIRECTDRAWSURFACE7 target_surface, LPRGNDATA lpRgnData )
{
	LPDIRECTDRAWCLIPPER lpDDClipper = NULL;
	HRESULT result;

	result = IDirectDraw7_CreateClipper(lpDDraw7, 0, &lpDDClipper, NULL);
	if (result == DD_OK) {

		result = IDirectDrawClipper_SetClipList(lpDDClipper, lpRgnData, 0);
		if( result == DD_OK ) {
			result = IDirectDrawSurface7_SetClipper(target_surface, lpDDClipper);
			if( result != DD_OK ) {
				logerror( "[D3D] IDirectDrawSurface_SetClipper() Failed.(Code:%d)\n", (int)result );
				IDirectDrawClipper_SetClipList(lpDDClipper, NULL, 0);
				IDirectDrawClipper_Release(lpDDClipper);
				lpDDClipper = NULL;
			}
		}
		else {
			logerror( "[D3D] IDirectDrawClipper_SetClipList() Failed.(Code:%d)\n", (int)result );
			IDirectDrawClipper_Release(lpDDClipper);
			lpDDClipper = NULL;
		}
	}
	else {
		logerror( "[D3D] IDirectDraw_CreateClipper() Failed.(Code:%d)\n", (int)result );
	}

	return lpDDClipper;
}


void d3d_destroy_chatclipper(LPDIRECTDRAWSURFACE7 target_surface, LPDIRECTDRAWCLIPPER lpCurClipper, LPDIRECTDRAWCLIPPER lpPrevClipper )
{
	IDirectDrawSurface7_SetClipper(target_surface, NULL);
	
	if( lpCurClipper != NULL ) {
		IDirectDrawClipper_SetClipList(lpCurClipper, NULL, 0); // Delete Clip Lists.
		IDirectDrawClipper_Release(lpCurClipper);
	}

	IDirectDrawSurface7_SetClipper(target_surface, lpPrevClipper);
}


void d3d_draw_kchatmsg(LPDIRECTDRAWSURFACE7 target_surface, LPRECT dst, BOOL bUseBackSurface )
{
	HDC		hDC;
	RECT	rect;
	HFONT	hFont;
	LPDIRECTDRAWSURFACE7 work_surface;

	if( !kPlay ) return; // not network play mode
    if(!ChatMessageTimer && !KailleraChatIsActive()) return; // nothing to draw

	if(bUseBackSurface) {
		DDSCAPS2 caps = { DDSCAPS_BACKBUFFER };
		HRESULT result;

		result = IDirectDrawSurface7_GetAttachedSurface(target_surface, &caps, &work_surface);
		if( result != DD_OK ) return;
	}
	else {
		work_surface = target_surface;
	}

	IDirectDrawSurface7_GetDC(work_surface, &hDC);
	if( hDC == NULL ) return;

	hFont = (HFONT)SelectObject(hDC, GetKailleraChatFont());

	if( ChatMessageTimer ) {
		if(nChatDrawMode == 0) {
			SetTextColor(hDC, GetChatFontColor());
			SetBkColor(hDC, RGB(255, 255, 255));
		} else if(nChatDrawMode == 1) {
			SetTextColor(hDC, GetChatFontColor());
			SetBkColor(hDC, RGB(0, 0, 0));
		} else {
			SetTextColor(hDC, GetChatFontColor());
			SetBkColor(hDC, RGB(0, 0, 0));
			SetBkMode(hDC, TRANSPARENT);
		}
		SetRect(&rect, dst->left, dst->top, dst->right, dst->bottom);
		DrawText(hDC, ChatBuffer, -1, &rect, DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK);

		ChatMessageTimer--;
		if(ChatMessageTimer == 0) {ChatMessageLines = 0; ChatBuffer[0] = '\0'; bChatFirst = TRUE;}
	}

	if( KailleraChatIsActive() ) {
		char	displayBuffer [KC_BUFFER_MAX + 10];
		char	getInputBuffer[KC_BUFFER_MAX];

		displayBuffer[0]  = '\0';
		getInputBuffer[0] = '\0';

		SetTextColor(hDC, RGB(0, 0, 0));
		SetBkColor(hDC, RGB(255, 255, 255));
		SetRect(&rect, dst->left, dst->bottom - KailleraChatHeight(), dst->right, dst->bottom);

		Edit_GetText(hChat, getInputBuffer, KC_BUFFER_MAX - 1);
		strcat(displayBuffer, "Chat:");
		strcat(displayBuffer, getInputBuffer);
		nBlinkCounter++;
		if (nBlinkCounter > KC_BLINK_SPD)
		{
			nBlinkCounter = 0;
			bShowCursor = !bShowCursor;
		}
		if (bShowCursor) strcat(displayBuffer, "|");
		else strcat(displayBuffer, " ");

		DrawText(hDC, displayBuffer, -1, &rect, DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_WORDBREAK);
	}

	SelectObject(hDC, hFont);
	IDirectDrawSurface7_ReleaseDC(work_surface, hDC);
}



//============================================================
//	OSD
//============================================================

void init_kchat(void)
{
	ChatMessageTimer = 0;
	ChatMessageLines = 0;
	ChatBuffer[0] = '\0';
	bChatFirst = TRUE;
}


void osd_chat_message(const char *str)
{
	ChatMessageTimer = 500;
	if(str == NULL || *str == 0) return;
	ChatMessageLines++;
	if(ChatMessageLines > 4) {
		char	*ptr = ChatBuffer;
		while(*ptr != '\n' && *ptr != '\0') ptr++;
		memmove(ChatBuffer, ptr + 1, strlen(ChatBuffer) - (ptr - ChatBuffer));
		ChatMessageLines = 4;
	}
	if(!bChatFirst) strcat(ChatBuffer, "\r\n");
	bChatFirst = FALSE;
	strcat(ChatBuffer, str);
}
