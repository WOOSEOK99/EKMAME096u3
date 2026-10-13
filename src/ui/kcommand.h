#ifdef KAILLERA

#define	KCOMMAND_CHAR	'\a'

#define AUTOSAVEPERIOD	60*60

enum {
	KC_unknown = -1,

	KC_close,
	KC_version,
	KC_dip,
	KC_gotdip,
	KC_savecrc,

	KC_last_entry
};

enum {
	KS_NONE = 0,
	KS_RESET,
	KS_SAVE,
	KS_CLOSE,
	KS_CRC,
	KS_DIP
};

int KailleraCommand(const char *text);
void SendKailleraCommand(int command_num, const char *params);

extern int kdipcounter;
extern int kgotdip;
extern int ksavecounter;
extern int kgotcrc;
extern int kcrccounter;
extern int ksavecounter;
extern int kaillera_schedule;

#endif /* KAILLERA */
