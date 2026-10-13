#ifdef KAILLERA

#include <windows.h>
#include "mame32.h"
#include "kcommand.h"
#include "kailleraclient.h"

extern int fmovestate(const char *gamename, const char *src, const char *dest);

//#define	KCOMMAND_CHAR	'\a'

/*
	Kaillera command	= prefix command [separator params]
	prefix				= KCOMMAND_CHAR
	command				= 1*letter
	separator			= ':'
	params				= 1*noctrl
	letter				= %x41-5A			; A-Z
	noctrl				= %x20-ff
*/

int kdipcounter;
int kgotdip;
int kcrccounter;
int kgotcrc;
int ksavecounter;
int kaillera_schedule;

const char *kcommand[] =
{
	"CLOSE",		// Close and quit emulation
	"VERSION",		// Version check
	"DIP",			// Send dip switch settings
	"GOTDIP",		// Response to DIP command
	"SAVECRC",		// Verify save state CRC to check desync
	NULL
};

#define	SEPALATOR	':'

int KailleraCommand(const char *text)
{
	char	command[256];
	char	params[256];
	const char	*src;
	char	*dest;
	int		i;
	int		command_num = KC_unknown;

	src = &text[1];
	dest = command;
	while(*src != '\0' && *src != SEPALATOR)
		*dest++ = *src++;
	*dest = '\0';

	if(*src == SEPALATOR) {
		src++;
		lstrcpy(params, src);
	}

	for(i = 0; kcommand[i] != NULL; i++) {
		if(lstrcmp(command, kcommand[i]) == 0) {
			command_num = i;
			break;
		}
	}

	switch(command_num)
	{
	case KC_close:
		kaillera_schedule = KS_CLOSE;
		break;

	case KC_version:
		{
			char	buf[256];
			sprintf(buf,"%s %s", MAME32NAME, build_version);
			kailleraChatSend(buf);
		}
		break;

	case KC_dip:
		{
			extern unsigned short kDipSwitch[MAX_INPUT_PORTS];
			extern int	kUpdateDip;
			int	len = lstrlen(params) / 4;
                        int j;
                        for(j = 0; j < len; j++) {
				char buf[] = "0000";
                                memcpy(buf, &params[j * 4], 4);
                                kDipSwitch[j] = strtoul(buf, NULL, 16);
			}
			kUpdateDip = 1;
			SendKailleraCommand(KC_gotdip, NULL);
		}
		break;

	case KC_gotdip:
		if(kdipcounter) {
			extern int kPlayers;
			kgotdip++;
			if(kgotdip == kPlayers) {
				kdipcounter = 0;
				kgotdip = 0;
				kaillera_schedule = KS_DIP;
			}
		}
		break;

	case KC_savecrc:
		if(kcrccounter) {
			static UINT32 savecrc = 0;
			UINT32 crc = strtoul(params, NULL, 16);
			extern int kPlayers;

			if(kgotcrc == 0) {
				savecrc = crc;
			} else {
				if(crc != savecrc) {
					usrintf_showmessage("CRC mismatch, game may desync");
					kcrccounter = 0;
					kgotcrc = 0;
					break;
				}
			}
			kgotcrc++;
			if(kgotcrc == kPlayers) {
				fmovestate(Machine->gamedrv->name, "~", "@");
				kcrccounter = 0;
				kgotcrc = 0;
			}
		}
		break;
	}
        return 0;
}

void SendKailleraCommand(int command_num, const char *params)
{
	char	buf[256];

	if(command_num <= KC_unknown || command_num >= KC_last_entry) {
		logerror("SendKailleraCommand() unknown command %d,%s\n", command_num, params);
		return;
	}

	buf[0] = KCOMMAND_CHAR;
	buf[1] = '\0';

	lstrcpy(&buf[1], kcommand[command_num]);

	if(params && params[0]) {
		lstrcat(buf, ":");
		lstrcat(buf, params);
	}

	kailleraChatSend(buf);
}

#endif /* KAILLERA */
