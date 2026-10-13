/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 UI Plus Copyright (C) 2005 DarkCoder

  This file is part of Customized MAME32, and may only be used, modified 
  and distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef __WIN32UI_PLUS__
#define __WIN32UI_PLUS__

void CreateUIKeyList(void);

INT_PTR CALLBACK InGameUIKeyProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

#endif //__WIN32UI_PLUS__
