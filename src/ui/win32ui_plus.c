/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 UI Plus Copyright (C) 2005 DarkCoder

  This file is part of Customized MAME32, and may only be used, modified 
  and distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

 /***************************************************************************

  win32ui_plus.c

	2005/02/20 - Created by DarkCoder

***************************************************************************/
#define MULTISESSION 0

#ifdef _MSC_VER
#ifndef NONAMELESSUNION
#define NONAMELESSUNION 
#endif
#endif

#define WIN32_LEAN_AND_MEAN
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

#include <driver.h>
#include <osdepend.h>
#include <unzip.h>

#include "bitmask.h"	// LPBITS
#include "options.h"
#include "m32util.h"
#include "osdepend.h"

#ifdef KAILLERA
#include "chat.h"
#include "netApi.h"
#include "kcommand.h"
#endif

#include "input.h"
#include "inptport.h"

#include "resource.h"

///////////////////////////////////////////////////////////////////////////////

/* src/config.c */
extern int  config_load_default_ui(const char *lpszCfgDir, const struct InputPortDefinition *input_ports_backup, struct InputPortDefinition *input_ports);
extern void config_save_default_ui(const char *lpszCfgDir, const struct InputPortDefinition *input_ports_backup, const struct InputPortDefinition *input_ports);

/* src/inptport.c */
struct InputPortDefinition *GetDefaultInputPortList(void);
const int GetInputPortCount(void);
struct InputPortDefinition *get_input_port_list(void);			/* not defined in inptport.h */
struct InputPortDefinition *get_input_port_list_backup(void);	/* not defined in inptport.h */

#ifndef LVS_EX_LABELTIP
#define LVS_EX_LABELTIP         0x00004000 // listview unfolds partly hidden labels if it does not have infotip text
#endif // LVS_EX_LABELTIP

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

///////////////////////////////////////////////////////////////////////////////


void input_FindKeyName(int iCode, char *pOut, int maxlen)
{
	input_seq_t seq;
	
	seq.code[0] = iCode;
	seq.code[1] = CODE_NONE;
		
	seq_to_string(&seq, pOut, maxlen);	// input.h/input.c
}

void myListCtrl_InsertColumn(HWND hWnd, int nCol, const char* lpszColumnHeading, int nFormat, int nWidth, int nSubItem)
{
	LVCOLUMN column;
	column.mask = LVCF_TEXT|LVCF_FMT;
	column.pszText = (LPTSTR)lpszColumnHeading;
	column.fmt = nFormat;
	if (nWidth != -1)
	{
		column.mask |= LVCF_WIDTH;
		column.cx = nWidth;
	}
	if (nSubItem != -1)
	{
		column.mask |= LVCF_SUBITEM;
		column.iSubItem = nSubItem;
	}
	
	SendMessage(hWnd, LVM_INSERTCOLUMN, nCol, (LPARAM)&column);
}

int myListCtrl_SetupItem(HWND hWnd, UINT nMask, int nItem, int nSubItem, const char *lpszItem, 
							UINT nState, UINT nStateMask, int nImage, LPARAM lParam)
{
	LVITEM lvi;
	lvi.mask 	  = nMask;
	lvi.iItem 	  = nItem;
	lvi.iSubItem  = nSubItem;
	lvi.pszText   = (char *)lpszItem;
	lvi.state 	  = nState;
	lvi.stateMask = nStateMask;
	lvi.iImage 	  = nImage;
	lvi.lParam	  = lParam;
	
	if(nSubItem == 0) 
		return SendMessage(hWnd, LVM_INSERTITEM, 0, (LPARAM)&lvi);
	
	return SendMessage(hWnd, LVM_SETITEM, 0, (LPARAM)&lvi);
}

void ModifyKeyName(const char *lpIn, char *lpOut)
{
	if(!strncmp(lpIn, "KEYCODE_", 8)) {
		strcpy( lpOut, &lpIn[8] );
		return;
	}

	if(!strncmp(lpIn, "JOYCODE_", 8)) {
		char cPlayer;
		
		cPlayer = lpIn[8];
		sprintf(lpOut, "(JOY%c)%s", cPlayer, &lpIn[10]);
		return;
	}

	strcpy( lpOut, lpIn );
}

void GetKeySeqName(input_seq_t *seq, char *pOut)
{
	int codenum, skipkey;
	char szKeyDefault[256]; 
	char szKeyName[256];

	pOut[0] = 0;
	codenum = 0;
	szKeyName[0] = 0;
	skipkey = 0;
	while((codenum < SEQ_MAX) && (seq->code[codenum] != CODE_NONE)) 
	{
		if(skipkey) { skipkey=0; codenum++; continue; }
		if(seq->code[codenum] == CODE_NOT) { skipkey=1; codenum++;	continue; }
		
		input_FindKeyName(seq->code[codenum], szKeyDefault, 256); 
		if(szKeyDefault[0]) { 
			ModifyKeyName(szKeyDefault, szKeyName); 
		} 
	else { 
			sprintf(szKeyName, "*n/a(%d)*", seq->code[codenum]);
		}
		strcat(pOut, szKeyName); 
		
		codenum++;
		
		/* special */
		switch(seq->code[codenum]) {
		case CODE_NONE:						/* no code, also marker of sequence end */
		case CODE_OTHER_DIGITAL:			/* OS code not mapped to any other code */
		case CODE_OTHER_ANALOG_ABSOLUTE:	/* OS code not mapped to any other code */
		case CODE_OTHER_ANALOG_RELATIVE:	/* OS code not mapped to any other code */
			break;

		case CODE_DEFAULT:					/* special for input port definitions */
#ifdef KOREAN		
			strcat(pOut, "*기본값*"); 
#else
			strcat(pOut, "*DEFAULT*"); 
#endif			
			break;
			
		case CODE_NOT: 						/* operators for sequences */
			skipkey=1; codenum++;
			break;

		case CODE_OR:  strcat(pOut, " or "); codenum++;	break;	/* operators for sequences */
		default:	   strcat(pOut, " + ");	break;
		}
	}
	
	return;
}

int CreateUIKeyList(HWND hWndList)
{
	struct	InputPortDefinition *lpPortList, *lpPortBackup, *lpDefault, *in, *indef;
	int		datasize, iItem, iIsValid, iPortCount, seqnum, cnt;
	char 	szStr[1024];
	char 	szCurSeqName[256];
	char 	szDefSeqName[256];
	BOOL	bIsSame, bLoaded = TRUE;
	
	lpPortList 		= get_input_port_list();
	lpPortBackup	= get_input_port_list_backup();

	/* M32K_PLUS에서 추가된 함수를 이용 */
	lpDefault 		= GetDefaultInputPortList();
	iPortCount		= GetInputPortCount();
	
	datasize  = sizeof(struct InputPortDefinition) * iPortCount;

	/* inptport.c 참조
	   -------------------------------------------------------------------------------
	   XML파서가 사용하는 token_to_port_type()함수가 inptport.c에 있는
	   inputport_list[]의 값에 의존하고 있다. 따라서 이 구조체변수를 미리
	   세팅해야 한다. inputport_list[]는 inptport.c에 있는 get_input_port_list()함수를
	   이용해서 포인터를 얻을 수 있다. 
	*/
	memcpy(lpPortBackup, lpDefault, datasize);
	//osd_customize_inputport_list(lpPortBackup);

	memcpy(lpPortList, lpPortBackup, datasize);
	/* ------------------------------------------------------------------------------ */

	szStr[0] = 0;
	if(strlen(GetCfgDir()) != 0) {
		osd_compose_path(szStr, FILETYPE_CONFIG, 0, "test" );
		/*
		 cfg폴더이름은 설정되어 있지만 osd_compose_path()에서 조합되지 않는 경우
		 : 이는 아직 한번도 게임을 실행하지 않아서 windows/fileio.c에 있는 pathlist[]
		   정보가 세팅되어 있지 않는 경우이다.
		*/
		if( !strcmp(szStr, "test") ) sprintf( szStr, "%s", GetCfgDir() );
		else szStr[0] = 0;
	}
	if( config_load_default_ui( szStr, NULL, lpPortList) == 0 ) {
		/* 생성된 default정보가 없음 */
		bLoaded = FALSE;
	}
	
	iItem = 0;
	in = lpPortList;
	
	/* Remove All Items in ListControl */
	SendMessage(hWndList, LVM_DELETEALLITEMS, 0, 0);
	
	cnt = 0;
	while ((in->type != IPT_END) && (cnt < iPortCount))
	{
		if((in->name != 0) && (in->group == IPG_UI)) {
			
			/* UI키 이름(설명) */
			sprintf( szStr, "%s", in->name );
			
			/* 현재의 키설정값 */
			GetKeySeqName( &in->defaultseq, szCurSeqName );
			
			/* 해당키에 해당하는 기본값 키 검색 */
			indef = lpDefault;
			while ((indef->type != IPT_END) && ((indef->group != IPG_UI) || (indef->type != in->type))) indef++;
			if((indef->group == IPG_UI) && (indef->type == in->type)) {
				
				GetKeySeqName( &indef->defaultseq, szDefSeqName );
				
				/* 현재 설정키와 기본값이 동일한지 조사 */
				bIsSame = TRUE;
				for (seqnum = 0; seqnum < SEQ_MAX; seqnum++) 
				{
					if(in->defaultseq.code[seqnum] != indef->defaultseq.code[seqnum]) 
					{
						bIsSame = FALSE; 
						break;
					}
					
					if( (in->defaultseq.code[seqnum] == CODE_NONE) ||
						(indef->defaultseq.code[seqnum] == CODE_NONE) ) break;
				}
				
				if(bIsSame == TRUE)
					strcpy( szCurSeqName, "*" );	/* 기본키와 같음 */
			}
			else {
#ifdef KOREAN
				sprintf( szDefSeqName, "%s", "*알 수 없음*" );
#else
				sprintf( szDefSeqName, "%s", "*unknown*" );
#endif				
			}

			iIsValid = 1;
			
#ifdef KAILLERA	
			if(!net_IsLoaded() && 
			   ((in->type == IPT_UI_CHAT_OPEN) || 
				(in->type == IPT_UI_CHAT_CHANGE_BG) ||
				(in->type == IPT_UI_CHAT_LOG) ||
				(in->type == IPT_UI_TOGGLE_AUTOSAVE) ||
				(in->type == IPT_UI_LOAD_AUTOSAVE) ||
				(in->type == IPT_UI_SEND_DIP) ||
				(in->type == IPT_UI_VERSION) ||
				(in->type == IPT_UI_P1_SHIFT_DOWN) ||
				(in->type == IPT_UI_P1_SHIFT_UP) ||
				(in->type == IPT_UI_SWAP_CONTROL)) ) 
			{
				iIsValid = 0;
			}
#endif /* KAILLERA*/
			
			if(iIsValid) {
				myListCtrl_SetupItem(hWndList, LVIF_TEXT, iItem, 0, szStr,  	  0, 0, 0, 0);
				myListCtrl_SetupItem(hWndList, LVIF_TEXT, iItem, 1, szCurSeqName, 0, 0, 0, 0);	
				myListCtrl_SetupItem(hWndList, LVIF_TEXT, iItem, 2, szDefSeqName, 0, 0, 0, 0);
				
				iItem++;
			}
		}
		cnt++;
		in++;
	}
	
	/* 해당 컬럼의폭을 내용물의 크기에 맞게 자동으로 조절 */
	ListView_SetColumnWidth(hWndList, 0, LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(hWndList, 1, LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(hWndList, 2, LVSCW_AUTOSIZE);
	
	iItem = ListView_GetColumnWidth(hWndList, 1);
	if(iItem < 80) ListView_SetColumnWidth(hWndList, 1, 80);
	
	return (int)bLoaded;
}

void ResetGameUIKeyConfig(void)
{
	struct	InputPortDefinition *lpPortList, *lpPortBackup, *lpDefault, *in, *indef;
	int		datasize, iPortCount, cnt;
	char 	szCfgDir[1024];
	
	lpPortList 		= get_input_port_list();
	lpPortBackup	= get_input_port_list_backup();

	/* M32K_PLUS에서 추가된 함수를 이용 */
	lpDefault 		= GetDefaultInputPortList();
	iPortCount		= GetInputPortCount();
	
	datasize  = sizeof(struct InputPortDefinition) * iPortCount;

	/* inptport.c 참조
	   -------------------------------------------------------------------------------
	   XML파서가 사용하는 token_to_port_type()함수가 inptport.c에 있는
	   inputport_list[]의 값에 의존하고 있다. 따라서 이 구조체변수를 미리
	   세팅해야 한다. inputport_list[]는 inptport.c에 있는 get_input_port_list()함수를
	   이용해서 포인터를 얻을 수 있다. 
	*/
	memcpy(lpPortBackup, lpDefault, datasize);
	//osd_customize_inputport_list(lpPortBackup);

	memcpy(lpPortList, lpPortBackup, datasize);
	/* ------------------------------------------------------------------------------ */
	
	szCfgDir[0] = 0;
	if(strlen(GetCfgDir()) != 0) {
		osd_compose_path(szCfgDir, FILETYPE_CONFIG, 0, "test" );
		/*
		 cfg폴더이름은 설정되어 있지만 osd_compose_path()에서 조합되지 않는 경우
		 : 이는 아직 한번도 게임을 실행하지 않아서 windows/fileio.c에 있는 pathlist[]
		   정보가 세팅되어 있지 않는 경우이다.
		*/
		if( !strcmp(szCfgDir, "test") ) sprintf( szCfgDir, "%s", GetCfgDir() );
		else szCfgDir[0] = 0;
	}
	if( config_load_default_ui( szCfgDir, NULL, lpPortList) == 0 ) {
		// 생성된 default정보가 없음
		return;
	}
	
	cnt = 0;
	in = lpPortList;
	
	while ((in->type != IPT_END) && (cnt < iPortCount))
	{
		if((in->name != 0) && (in->group == IPG_UI)) {
			
			/* 해당키에 해당하는 기본값 키 검색 */
			indef = lpDefault;
			while ((indef->type != IPT_END) && ((indef->group != IPG_UI) || (indef->type != in->type))) indef++;

			if((indef->group == IPG_UI) && (indef->type == in->type)) {
				memcpy( &in->defaultseq, &indef->defaultseq, sizeof(input_seq_t));
			}
		}
		cnt++;
		in++;
	}
	
	/* ------------------------------------------------------------------------------ */
	/* save default(reset) value */
	config_save_default_ui( szCfgDir, lpPortBackup, lpPortList );
	/* ------------------------------------------------------------------------------ */
	
	return;
}

INT_PTR CALLBACK InGameUIKeyProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG:
	{
		HWND hWnd;
		
		CenterWindow(hDlg);
		
		/* ListView Control 초기화 */		
		hWnd = GetDlgItem(hDlg, IDC_LIST_KEY);
		
		SendMessage(hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM) LVS_EX_FULLROWSELECT);
		
#ifdef KOREAN
		myListCtrl_InsertColumn(hWnd, 0, "UI 키 설명",		LVCFMT_LEFT, 165, 0);
		myListCtrl_InsertColumn(hWnd, 1, "현재 설정값", 	LVCFMT_LEFT, 130, 0);
		myListCtrl_InsertColumn(hWnd, 2, "기본값", 			LVCFMT_LEFT, 130, 0);
#else
		myListCtrl_InsertColumn(hWnd, 0, "UI Key Description",		LVCFMT_LEFT, 160, 0);
		myListCtrl_InsertColumn(hWnd, 1, "Current Config", 			LVCFMT_LEFT, 100, 0);
		myListCtrl_InsertColumn(hWnd, 2, "Default Value", 			LVCFMT_LEFT, 100, 0);
#endif		
		EnableWindow( GetDlgItem(hDlg, IDC_BTN_RESET), CreateUIKeyList(hWnd) );
		
		return FALSE;
	}
	case WM_COMMAND:
	{
		WORD wID		 = GET_WM_COMMAND_ID(wParam, lParam);
		//WORD wNotifyCode = GET_WM_COMMAND_CMD(wParam, lParam);

		switch (wID)
		{
		case IDC_BTN_RESET:	/* 초기화 버튼 */
			{
				int iResult;
#ifdef KOREAN
				iResult= MessageBox(hDlg, "현재의 키 설정값을 기본값으로 되돌립니다.\n\n계속 하시겠습니까?",
								 			"키 설정값 초기화",
								 			MB_YESNO|MB_ICONQUESTION);
#else
				iResult= MessageBox(hDlg, "Reset all key settings to default value.\n\nAre you sure?",
								 			"Reset Key Config",
								 			MB_YESNO|MB_ICONQUESTION);
#endif								 			
								 			
				if(iResult == IDYES) {
					ResetGameUIKeyConfig();
					CreateUIKeyList( GetDlgItem(hDlg, IDC_LIST_KEY) );
				}
			}
			return TRUE;
			
		case IDOK:
			EndDialog(hDlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
	}
	break;
	}
	return 0;
}

/* End of source file */
