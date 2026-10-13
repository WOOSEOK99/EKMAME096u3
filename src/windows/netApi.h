#ifdef KAILLERA

#ifndef __NETAPI_H__
#define __NETAPI_H__

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *appName;
	char *gameList;
	
	int (WINAPI *gameCallback)(char *game, int player, int numplayers);
	
	void (WINAPI *chatReceivedCallback)(char *nick, char *text);
	void (WINAPI *clientDroppedCallback)(char *nick, int playernb);
	
	void (WINAPI *moreInfosCallback)(char *gamename);
} netApiCallbacks;

#ifdef __cplusplus
};
#endif

int net_IsLoaded(void);

int net_GetVersion(char *version);
int net_Init(void);
int net_ShutDown(void);
int net_SetInfos(netApiCallbacks *infos);
int net_SelectServerDialog(HWND parent);
int net_ModifyPlayValues(void *values, int size);
int net_ChatSend(char *text);
int net_EndGame(void);

#endif /* __NETAPI_H__ */

#endif /* KAILLERA */
