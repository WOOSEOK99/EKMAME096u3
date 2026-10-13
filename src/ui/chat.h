#ifdef KAILLERA

void KailleraChatInit(void);
void KailleraChatExit(void);
void KailleraChatUpdate(struct mame_bitmap *pBitmap);
void KailleraChateReceive(char *szText);
BOOL KailleraChatIsActive(void);
int KailleraChatHeight(void);

extern HWND	hChat;
#define KC_BUFFER_MAX    128

#endif /* KAILLERA */
