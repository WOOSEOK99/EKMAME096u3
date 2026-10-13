//============================================================
//
//	win_kchat.h - Kaillera chat system
//
//============================================================

#ifdef KAILLERA
#ifndef __WIN_KCHAT_H__
#define __WIN_KCHAT_H__

#define KC_BLINK_SPD			20
#include <windowsx.h>
#include "ui/chat.h"
#include "ui/options.h"
#include "ui/win32ui.h"

#include <ddraw.h>
#include <d3d.h>

extern int	kPlay;
extern char ChatBuffer[4096];
extern int	ChatMessageTimer;
extern int	ChatMessageLines;
extern BOOL	bChatFirst;
extern int	nBlinkCounter;
extern BOOL bShowCursor;

extern int nChatDrawMode;
extern HFONT GetKailleraChatFont(void);



//============================================================
//	DIRECTDRAW
//============================================================

LPRGNDATA ddraw_get_chatmsg_rgndata(LPDIRECTDRAWSURFACE target_surface, LPRECT rcScr);
LPDIRECTDRAWCLIPPER ddraw_create_chatclipper(LPDIRECTDRAW lpDDraw, LPDIRECTDRAWSURFACE target_surface, LPRGNDATA lpRgnData );
void ddraw_destroy_chatclipper(LPDIRECTDRAWSURFACE target_surface, LPDIRECTDRAWCLIPPER lpCurClipper, LPDIRECTDRAWCLIPPER lpPrevClipper );
void ddraw_draw_kchatmsg(LPDIRECTDRAWSURFACE target_surface, LPRECT dst, BOOL bUseBackSurface );



//============================================================
//	DIRECTDRAW
//============================================================

LPRGNDATA d3d_get_chatmsg_rgndata(LPDIRECTDRAWSURFACE7 target_surface, LPRECT rcScr);
LPDIRECTDRAWCLIPPER d3d_create_chatclipper(LPDIRECTDRAW7 lpDDraw7, LPDIRECTDRAWSURFACE7 target_surface, LPRGNDATA lpRgnData );
void d3d_destroy_chatclipper(LPDIRECTDRAWSURFACE7 target_surface, LPDIRECTDRAWCLIPPER lpCurClipper, LPDIRECTDRAWCLIPPER lpPrevClipper );
void d3d_draw_kchatmsg(LPDIRECTDRAWSURFACE7 target_surface, LPRECT dst, BOOL bUseBackSurface );



//============================================================
//	OSD
//============================================================

void init_kchat(void);
void osd_chat_message(const char *str);

#endif//__WIN_KCHAT_H__
#endif// KAILLERA
