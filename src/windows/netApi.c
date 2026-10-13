#ifdef KAILLERA

#include <windows.h>
#include "netAPI.h"

// --------------------------------------------------------------------------
// By DarkCoder
typedef int	(WINAPI *PROC_KGETVERSION		)(char *version);
typedef int	(WINAPI *PROC_KINIT				)(void);
typedef int	(WINAPI *PROC_KSHUTDOWN			)(void);
typedef int	(WINAPI *PROC_KSETINFOS			)(netApiCallbacks *infos);
typedef int	(WINAPI *PROC_KSELECTSERVERDLG	)(HWND parent);
typedef int	(WINAPI *PROC_KMODIFYPLAYVALUES	)(void *values, int size);
typedef int	(WINAPI *PROC_KCHATSEND			)(char *text);
typedef int	(WINAPI *PROC_KENDGAME			)(void);

typedef struct {
	int		iIsLoaded;
	HMODULE	hLib;
	
	// function pointer
	PROC_KGETVERSION		pfnKGetVersion;
	PROC_KINIT				pfnKInit;
	PROC_KSHUTDOWN			pfnKShutDown;
	PROC_KSETINFOS			pfnKSetInfos;
	PROC_KSELECTSERVERDLG	pfnKSelectServerDlg;
	PROC_KMODIFYPLAYVALUES	pfnModifyPlayValues;
	PROC_KCHATSEND			pfnChatSend;
	PROC_KENDGAME			pfnKEndGame;
} NETMODLIB;

NETMODLIB _netAPI = { 0, };

static const char *_gszFakeMsg[] = {
	"Unable to initialize wsock32",
	"Connection lost from the host. (error code:10060)",
	"Server list request timed out.",
	"404 Not Found - Unable to find server host or DNS error.",
	"Ok... It's fake. :)",
	"Sorry, there's no network play feature",
	"Hey, Can't you read English? NO network support!!",
	"Unable to find proper DLL module to load.",
	NULL
};
static int _fakeMsgIdx = 0;
	
void net_ShowFakeMsg(void)
{
	MessageBox(NULL, _gszFakeMsg[_fakeMsgIdx], "ERROR", MB_OK|MB_ICONSTOP);

	_fakeMsgIdx++;
	if(	_gszFakeMsg[_fakeMsgIdx] == NULL ) _fakeMsgIdx = 0;
	return;
}

int net_IsLoaded(void) { return _netAPI.iIsLoaded; }

int net_LoadModule(void)
{
	HMODULE hLib;
	
	_netAPI.iIsLoaded = 0;
	_netAPI.hLib	 = NULL;

	hLib = LoadLibrary( "kclient.dll" );
	if( hLib == NULL ) {
		return 0;
	}
	
	_netAPI.pfnKGetVersion		= (PROC_KGETVERSION		 )GetProcAddress( hLib, "_kailleraGetVersion@4"			);
	_netAPI.pfnKInit			= (PROC_KINIT			 )GetProcAddress( hLib, "_kailleraInit@0"				);
	_netAPI.pfnKShutDown		= (PROC_KSHUTDOWN		 )GetProcAddress( hLib, "_kailleraShutdown@0"			);
	_netAPI.pfnKSetInfos		= (PROC_KSETINFOS		 )GetProcAddress( hLib, "_kailleraSetInfos@4"			);
	_netAPI.pfnKSelectServerDlg	= (PROC_KSELECTSERVERDLG )GetProcAddress( hLib, "_kailleraSelectServerDialog@4"	);
	_netAPI.pfnModifyPlayValues	= (PROC_KMODIFYPLAYVALUES)GetProcAddress( hLib, "_kailleraModifyPlayValues@8"	);
	_netAPI.pfnChatSend			= (PROC_KCHATSEND		 )GetProcAddress( hLib, "_kailleraChatSend@4"			);
	_netAPI.pfnKEndGame			= (PROC_KENDGAME		 )GetProcAddress( hLib, "_kailleraEndGame@0"			);

	if( !_netAPI.pfnKGetVersion		 ||
		!_netAPI.pfnKInit			 ||
		!_netAPI.pfnKShutDown		 ||
		!_netAPI.pfnKSetInfos		 ||
		!_netAPI.pfnKSelectServerDlg ||
		!_netAPI.pfnModifyPlayValues ||
		!_netAPI.pfnChatSend		 ||
		!_netAPI.pfnKEndGame		 )
	{
		/*
		logmsg("LoadModule: GetProcAddress Failed! (GV:%p I:%p SD:%p SI:%p SSD:%p MPV:%p CS:%p EG:%p)\n",
			_netAPI.pfnKGetVersion,
			_netAPI.pfnKInit,
			_netAPI.pfnKShutDown,
			_netAPI.pfnKSetInfos,
			_netAPI.pfnKSelectServerDlg,
			_netAPI.pfnModifyPlayValues,
			_netAPI.pfnChatSend,
			_netAPI.pfnKEndGame		 );
		*/
		FreeLibrary( hLib );
		return 0;
	}

	_netAPI.iIsLoaded = 1;
	_netAPI.hLib = hLib;
	
	return 1;
}

int net_GetVersion(char *version)
{
	if(!net_IsLoaded()) net_LoadModule();
	if(!net_IsLoaded()) { net_ShowFakeMsg(); return 0; }

	return _netAPI.pfnKGetVersion(version);
}

int net_Init(void)
{
	if(!net_IsLoaded()) net_LoadModule();
	if(!net_IsLoaded()) return 0;

	return _netAPI.pfnKInit();
}

int net_ShutDown(void)
{
	int i;
	if(!net_IsLoaded()) return 0;
	i = _netAPI.pfnKShutDown();
	
	FreeLibrary( _netAPI.hLib );
	_netAPI.iIsLoaded = 0;
	_netAPI.hLib	  = NULL;
	
	return i;
}

int net_SetInfos(netApiCallbacks *infos)
{
	if(!net_IsLoaded()) return 0;
	return _netAPI.pfnKSetInfos(infos);
}

int net_SelectServerDialog(HWND parent)
{
	if(!net_IsLoaded()) { net_ShowFakeMsg(); return 0; }
	return _netAPI.pfnKSelectServerDlg(parent);
}

int net_ModifyPlayValues(void *values, int size)
{
	if(!net_IsLoaded()) return 0;
	return _netAPI.pfnModifyPlayValues(values,size);
}

int net_ChatSend(char *text)
{
	if(!net_IsLoaded()) return 0;
	return _netAPI.pfnChatSend(text);
}

int net_EndGame(void)
{
	if(!net_IsLoaded()) return 0;
	return _netAPI.pfnKEndGame();
}

#endif /* KAILLERA */
