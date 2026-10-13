#include "driver.h"
#include <stdarg.h>
#include <string.h>
#include "audit.h"
#include "harddisk.h"
#include "sound/samples.h"
 
static tAuditRecord *gAudits = NULL;

static const struct GameDriver *chd_gamedrv;

/*-------------------------------------------------
    audit_chd_open - interface for opening
    a hard disk image
-------------------------------------------------*/

struct chd_interface_file *audit_chd_open(const char *filename, const char *mode)
{
	const struct GameDriver *drv;

	/* attempt reading up the chain through the parents */
	for (drv = chd_gamedrv; drv != NULL; drv = drv->clone_of)
	{
		void* file = mame_fopen(drv->name, filename, FILETYPE_IMAGE, 0);

		if (file != NULL)
			return file;
	}

	return NULL;
}



/*-------------------------------------------------
    audit_chd_close - interface for closing
    a hard disk image
-------------------------------------------------*/

void audit_chd_close(struct chd_interface_file *file)
{
	mame_fclose((mame_file *)file);
}



/*-------------------------------------------------
    audit_chd_read - interface for reading
    from a hard disk image
-------------------------------------------------*/

UINT32 audit_chd_read(struct chd_interface_file *file, UINT64 offset, UINT32 count, void *buffer)
{
	mame_fseek((mame_file *)file, offset, SEEK_SET);
	return mame_fread((mame_file *)file, buffer, count);
}



/*-------------------------------------------------
    audit_chd_write - interface for writing
    to a hard disk image
-------------------------------------------------*/

UINT32 audit_chd_write(struct chd_interface_file *file, UINT64 offset, UINT32 count, const void *buffer)
{
	return 0;
}


/*-------------------------------------------------
    audit_chd_length - interface for getting
    the length a hard disk image
-------------------------------------------------*/

UINT64 audit_chd_length(struct chd_interface_file *file)
{
	return mame_fsize((mame_file *)file);
}



static struct chd_interface audit_chd_interface =
{
	audit_chd_open,
	audit_chd_close,
	audit_chd_read,
	audit_chd_write,
	audit_chd_length
};


/* returns 1 if rom is defined in this set */
int RomInSet (const struct GameDriver *gamedrv, const char* hash)
{
	const struct RomModule *region, *rom;

	for (region = rom_first_region(gamedrv); region; region = rom_next_region(region))
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			/* Compare all the available checksums */
			if (hash_data_is_equal(ROM_GETHASHDATA(rom), hash, 0))
				return 1;

	return 0;
}


/* returns nonzero if romset is missing */
int RomsetMissing (int game)
{
	const struct GameDriver *gamedrv = drivers[game];

	if (gamedrv->clone_of)
	{
		tAuditRecord	*aud;
		int				count;
		int 			i;

#if 1
        int cloneRomsFound = 0;
        int uniqueRomsFound = 0;

		if ((count = AuditRomSet (game, &aud)) == 0)
			return 1;

		if (count == -1) return 0;

        /* count number of roms found that are unique to clone */
        for (i = 0; i < count; i++)
			if (!RomInSet (gamedrv->clone_of, aud[i].exphash))
			{
				uniqueRomsFound++;
				if (aud[i].status != AUD_ROM_NOT_FOUND)
					cloneRomsFound++;
			}
#else
		int cloneRomsFound = 0;

		if ((count = AuditRomSet (game, &aud)) == 0)
			return 1;

		if (count == -1) return 0;

		/* count number of roms found that are unique to clone */
		for (i = 0; i < count; i++)
			if (aud[i].status != AUD_ROM_NOT_FOUND)
				if (!RomInSet (gamedrv->clone_of, aud[i].exphash))
					cloneRomsFound++;
#endif

		return !cloneRomsFound;
	}
	else
		return !mame_faccess (gamedrv->name, FILETYPE_ROM);
}

/* Fills in an audit record for each rom in the romset. Sets 'audit' to
   point to the list of audit records. Returns total number of roms
   in the romset (same as number of audit records), 0 if romset missing. */
int AuditRomSet (int game, tAuditRecord **audit)
{
#define CHECK_SELECTED_BIOS_ONLY

	const struct RomModule *region, *rom, *chunk;
	const char *name;
	const struct GameDriver *gamedrv;

	int count = 0;
	tAuditRecord *aud;
	int	err;

#ifdef CHECK_SELECTED_BIOS_ONLY
	int system_bios = determine_bios_rom(drivers[game]->bios);
#endif /* CHECK_SELECTED_BIOS_ONLY */

	if (!gAudits)
	{
		gAudits = (tAuditRecord *)malloc (AUD_MAX_ROMS * sizeof (tAuditRecord));

		// Make sure the memory is cleared - it's needed by the hashing
		//  engine
		memset(gAudits, 0, AUD_MAX_ROMS * sizeof(tAuditRecord));
	}

	if (gAudits)
		*audit = aud = gAudits;
	else
		return 0;

	gamedrv = drivers[game];

	if (!gamedrv->rom) return -1;

	/* check for existence of romset */
	if (!mame_faccess (gamedrv->name, FILETYPE_ROM))
	{
		/* if the game is a clone, check for parent */
		if (gamedrv->clone_of == 0 || (gamedrv->clone_of->flags & NOT_A_DRIVER) ||
				!mame_faccess(gamedrv->clone_of->name,FILETYPE_ROM))
			return 0;
	}

	for (region = rom_first_region(gamedrv); region; region = rom_next_region(region))
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			if (ROMREGION_ISROMDATA(region))
			{
#ifdef CHECK_SELECTED_BIOS_ONLY
			if (!ROM_GETBIOSFLAGS(rom) || (ROM_GETBIOSFLAGS(rom) == (system_bios+1))) /* alternate bios sets */
			{
#endif /* CHECK_SELECTED_BIOS_ONLY */
				const struct GameDriver *drv;

				name = ROM_GETNAME(rom);
				strcpy (aud->rom, name);
				aud->explength = 0;
				aud->length = 0;
				aud->exphash = ROM_GETHASHDATA(rom);

				/* Copy into the variable we pass to the functions
                   to support load-by-checksum */
				hash_data_copy(aud->hash, aud->exphash);

				count++;

				/* obtain hash checksums and length of ROM file */
				drv = gamedrv;
				do
				{
					err = mame_fchecksum(drv->name, name, &aud->length, aud->hash);
					drv = drv->clone_of;
				} while (err && drv);

				/* spin through ROM_CONTINUEs, totaling length */
				for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
					aud->explength += ROM_GETLENGTH(chunk);

				if (err)
				{
					if (hash_data_has_info(aud->exphash, HASH_INFO_NO_DUMP))
					{
						/* not found but it's not good anyway */
						aud->status = AUD_NOT_AVAILABLE;
					}
					else if (ROM_ISOPTIONAL(rom))
					{
						/* optional ROM not found */
						aud->status = AUD_OPTIONAL_ROM_NOT_FOUND;
					}
					else
					{
						/* not found */
						aud->status = AUD_ROM_NOT_FOUND;

						drv = gamedrv->clone_of;

						/* If missing ROM is also present in a parent set, indicate that */
						while (drv)
						{
							if (RomInSet (drv, aud->exphash))
							{
								if (drv->flags & NOT_A_DRIVER)
								{
									aud->status = AUD_ROM_NOT_FOUND_BIOS;
									break;
								}
								else
									aud->status = AUD_ROM_NOT_FOUND_PARENT;
							}

							// Walk up the inheritance list. If this ROM is a clone of a set which
							// contains a BIOS that is missing, we can correctly mark it as
							// such.
							drv = drv->clone_of;
						}
					}
				}
				/* all cases below assume the ROM was at least found */
				else if (aud->explength != aud->length)
					aud->status = AUD_LENGTH_MISMATCH;
				else if (hash_data_has_info(aud->exphash, HASH_INFO_NO_DUMP))
						aud->status = AUD_ROM_NEED_DUMP; /* new case - found but not known to be dumped */
				else if (!hash_data_is_equal(aud->exphash, aud->hash, 0))
				{
					/* non-matching hash */
						aud->status = AUD_BAD_CHECKSUM;
				}
				else
				{
					/* matching hash */
					if (hash_data_has_info(aud->exphash, HASH_INFO_BAD_DUMP))
						aud->status = AUD_ROM_NEED_REDUMP;
					else
					aud->status = AUD_ROM_GOOD;
				}

				aud++;
#ifdef CHECK_SELECTED_BIOS_ONLY
			}
#endif /* CHECK_SELECTED_BIOS_ONLY */
			}
			else if (ROMREGION_ISDISKDATA(region))
			{
				const UINT8 nullhash[HASH_BUF_SIZE] = { 0 };
				void *source;
				struct chd_header header;

				name = ROM_GETNAME(rom);
				strcpy (aud->rom, name);
				aud->explength = 0;
				aud->length = 0;
				aud->exphash = ROM_GETHASHDATA(rom);
				hash_data_clear(aud->hash);
				count++;

				chd_gamedrv = gamedrv;
				chd_set_interface(&audit_chd_interface);
				source = chd_open( name, 0, NULL );
				if( source == NULL )
				{
					err = chd_get_last_error();
					if( err == CHDERR_OUT_OF_MEMORY )
					{
						aud->status = AUD_MEM_ERROR;
					}
					else if (hash_data_has_info(aud->exphash, HASH_INFO_NO_DUMP))
					{
						/* not found but it's not good anyway */
						aud->status = AUD_DISK_NOT_AVAILABLE;
					}
					else
					{
						/* not found */
						aud->status = AUD_DISK_NOT_FOUND;
					}
				}
				else
				{
					header = *chd_get_header(source);

					if (memcmp(nullhash, header.md5, sizeof(header.md5)))
						hash_data_insert_binary_checksum(aud->hash, HASH_MD5, header.md5);
					if (memcmp(nullhash, header.sha1, sizeof(header.sha1)))
						hash_data_insert_binary_checksum(aud->hash, HASH_SHA1, header.sha1);

					 if (hash_data_has_info(aud->exphash, HASH_INFO_NO_DUMP))
					{
						aud->status = AUD_DISK_NEED_DUMP;
					}
					else if (!hash_data_is_equal(aud->exphash, aud->hash, 0))
					{
						aud->status = AUD_DISK_BAD_MD5;
					}
					else
					{
						aud->status = AUD_DISK_GOOD;
					}

					chd_close( source );
				}

				aud++;
			}
		}

        #ifdef MESS
        if (!count)
                return -1;
        else
        #endif
	return count;
}

static void VerifyDumpHashData(const char* hash, const char* head, verify_printf_proc verify_printf)
{
	char buf[512];

	verify_printf("\t%s", head ? head : "");
	hash_data_print(hash, 0, buf);
	verify_printf("%s\n", buf);
}

#ifdef FAST_AUDIT
#include <io.h>
#include "unzip.h"

typedef struct {
	UINT32		status;
	const char*	szMsg;
} STATSTRING;

typedef struct {
	int		iType;
	UINT32	crc32;
	char	szROM[256];
} GAMEROMLIST;

typedef struct tagGAMEROMINFO {
	char 	szROM[256];
	char 	szDesc[256];
	char 	szParentROM[256];
	struct tagGAMEROMINFO *next;
} GAMEROMINFO;

GAMEROMLIST*	gRomList    = 0;
GAMEROMINFO		gBadRomInfo, *ptBadRom;

int				gDuplicated = 0;
FILE*			_audfp      = 0;
int				_AuditInit  = 0;

int				_FastAuditTotal 	= 0;
int				_FastAuditGood		= 0;
int				_FastAuditNotFound	= 0;
int				_FastAuditBad		= 0;
int				_FastAuditMissing	= 0;

#ifdef KOREAN
STATSTRING gStatString[] = {
	{ 0,							"*알 수 없는 상태*"						},
	{ AUD_ROM_NOT_FOUND,			"없음(찾지못함)"						},
	{ AUD_OPTIONAL_ROM_NOT_FOUND,	"없음 (옵션)"							},
	{ AUD_NOT_AVAILABLE,			"없음 - 양호한 롬이 알려지지 않았음"	},
	{ AUD_ROM_NEED_DUMP,			"있지만 양호한 롬이 알려지지 않았음"	},
	{ AUD_BAD_CHECKSUM,				"잘못된 체크섬(CRC):"					},
	{ AUD_ROM_NEED_REDUMP,			"재덤프가 필요한 롬"					},
	{ AUD_MEM_ERROR,				"롬을 읽기 위한 메모리가 부족"			},
	{ AUD_LENGTH_MISMATCH,			"잘못된 파일크기:"						},
	{ AUD_ROM_GOOD,					"양호한 롬"								},
	{ AUD_DISK_GOOD,				"양호"									},
	{ AUD_DISK_NOT_FOUND,			"없음(찾지못함)"						},
	{ AUD_DISK_BAD_MD5,				"잘못된 체크섬(CRC):"					},
	{ AUD_DISK_NOT_AVAILABLE,		"없음 - 양호한 디스크가 알려지지 않았음"},
	{ AUD_DISK_NEED_DUMP,			"있지만 양호한 디스크가 알려지지 않았음"},
	{ 0,							0 }
};
const char *gszFastAuditResultFile = "FastAudit.txt";

const char	*gszExpected   = "예상값:";
const char	*gszFound      = "실제값:";
const char	*gszDuplicated = "한군데 이상의 폴더에서 발견되었습니다.";
#else
STATSTRING gStatString[] = {
	{ 0,							"*UNKNOWN STATUS*"					},
	{ AUD_ROM_NOT_FOUND,			"NOT FOUND"							},
	{ AUD_OPTIONAL_ROM_NOT_FOUND,	"NOT FOUND BUT OPTIONAL"			},
	{ AUD_NOT_AVAILABLE,			"NOT FOUND - NO GOOD DUMP KNOWN"	},
	{ AUD_ROM_NEED_DUMP,			"FOUND BUT NO GOOD DUMP KNOWN"		},
	{ AUD_BAD_CHECKSUM,				"INCORRECT CHECKSUM:"				},
	{ AUD_ROM_NEED_REDUMP,			"ROM NEEDS REDUMP"					},
	{ AUD_MEM_ERROR,				"Out of memory reading ROM"			},
	{ AUD_LENGTH_MISMATCH,			"INCORRECT LENGTH:"					},
	{ AUD_ROM_GOOD,					"ROM GOOD"							},
	{ AUD_DISK_GOOD,				"GOOD"								},
	{ AUD_DISK_NOT_FOUND,			"NOT FOUND"							},
	{ AUD_DISK_BAD_MD5,				"INCORRECT CHECKSUM:"				},
	{ AUD_DISK_NOT_AVAILABLE,		"NO GOOD DUMP KNOWN"				},
	{ AUD_DISK_NEED_DUMP,			"FOUND BUT NO GOOD DUMP KNOWN"		},
	{ 0,							0 }
};
const char *gszFastAuditResultFile = "FastAudit.txt";

const char	*gszExpected   = "EXPECTED:";
const char	*gszFound      = "   FOUND:";
const char	*gszDuplicated = "This rom/disk found in more than one directory.";
#endif

const char *FindStatusString(UINT32 status)
{
	int i = 1;
	while( gStatString[i].status != 0 ) {
		if( status == gStatString[i].status ) {
			return gStatString[i].szMsg;
		}
		i++;
	}

	return gStatString[0].szMsg;
}
#endif /*FAST_AUDIT*/

/* Generic function for evaluating a romset. Some platforms may wish to
   call AuditRomSet() instead and implement their own reporting (like MacMAME). */
int VerifyRomSet (int game, verify_printf_proc verify_printf)
{
	tAuditRecord			*aud;
	int						count;
	int						archive_status = 0;
	const struct GameDriver *gamedrv = drivers[game];

	if ((count = AuditRomSet (game, &aud)) == 0)
		return NOTFOUND;

	if (count == -1) return CORRECT;

#if 1
    if (gamedrv->clone_of)
    {
        int i;
        int cloneRomsFound = 0;
        int uniqueRomsFound = 0;

        /* count number of roms found that are unique to clone */
        for (i = 0; i < count; i++)
			if (!RomInSet (gamedrv->clone_of, aud[i].exphash))
			{
				uniqueRomsFound++;
				if (aud[i].status != AUD_ROM_NOT_FOUND)
					cloneRomsFound++;
			}
        #ifndef MESS
        /* Different MESS systems can use the same ROMs */
        if (uniqueRomsFound && !cloneRomsFound)
            return CLONE_NOTFOUND;
        #endif
    }
#else
	if (gamedrv->clone_of)
	{
		int i;
		int cloneRomsFound = 0;

		/* count number of roms found that are unique to clone */
		for (i = 0; i < count; i++)
			if (aud[i].status != AUD_ROM_NOT_FOUND)
				if (!RomInSet (gamedrv->clone_of, aud[i].exphash))
					cloneRomsFound++;

                #ifndef MESS
                /* Different MESS systems can use the same ROMs */
		if (cloneRomsFound == 0)
			return CLONE_NOTFOUND;
                #endif
	}
#endif

	while (count--)
	{
		archive_status |= aud->status;

		switch (aud->status)
		{
			case AUD_ROM_NOT_FOUND:
				verify_printf (_("%-8s: %-12s %7d bytes NOT FOUND\n"),
					drivers[game]->name, aud->rom, aud->explength);
				VerifyDumpHashData(aud->exphash, NULL, verify_printf);
				break;
			case AUD_ROM_NOT_FOUND_PARENT:
				verify_printf ("%-8s: %-12s %7d bytes NOT FOUND (shared with parent)\n",
					drivers[game]->name, aud->rom, aud->explength);
				VerifyDumpHashData(aud->exphash, NULL, verify_printf);
				break;
			case AUD_ROM_NOT_FOUND_BIOS:
				verify_printf ("%-8s: %-12s %7d bytes NOT FOUND (BIOS)\n",
					drivers[game]->name, aud->rom, aud->explength);
				VerifyDumpHashData(aud->exphash, NULL, verify_printf);
				break;
			case AUD_OPTIONAL_ROM_NOT_FOUND:
				verify_printf (_("%-8s: %-12s %7d bytes NOT FOUND BUT OPTIONAL\n"),
					drivers[game]->name, aud->rom, aud->explength);
				VerifyDumpHashData(aud->exphash, NULL, verify_printf);
				break;
			case AUD_NOT_AVAILABLE:
				verify_printf (_("%-8s: %-12s %7d bytes NOT FOUND - NO GOOD DUMP KNOWN\n"),
					drivers[game]->name, aud->rom, aud->explength);
				break;
			case AUD_ROM_NEED_DUMP:
				verify_printf (_("%-8s: %-12s %7d bytes FOUND BUT NO GOOD DUMP KNOWN\n"),
					drivers[game]->name, aud->rom, aud->explength);
				break;
			case AUD_BAD_CHECKSUM:
				verify_printf (_("%-8s: %-12s %7d bytes INCORRECT CHECKSUM:\n"),
					drivers[game]->name, aud->rom, aud->explength);
				VerifyDumpHashData(aud->exphash, _("EXPECTED: "), verify_printf);
				VerifyDumpHashData(aud->hash,    _("   FOUND: "), verify_printf);
				break;
			case AUD_ROM_NEED_REDUMP:
				verify_printf (_("%-8s: %-12s %7d bytes ROM NEEDS REDUMP\n"),
					drivers[game]->name, aud->rom, aud->explength);
				break;
			case AUD_MEM_ERROR:
				verify_printf (_("Out of memory reading ROM %s\n"), aud->rom);
				break;
			case AUD_LENGTH_MISMATCH:
				verify_printf (_("%-8s: %-12s %7d bytes INCORRECT LENGTH: %8d\n"),
					drivers[game]->name, aud->rom, aud->explength, aud->length);
				VerifyDumpHashData(aud->exphash, NULL, verify_printf);
				break;
			case AUD_ROM_GOOD:
#if 0    /* if you want a full accounting of roms */
				verify_printf (_("%-8s: %-12s %7d bytes ROM GOOD\n"),
					drivers[game]->name, aud->rom, aud->explength);
				VerifyDumpHashData(aud->hash, NULL, verify_printf);
#endif
				break;
			case AUD_DISK_GOOD:
#if 0    /* if you want a full accounting of roms */
				verify_printf (_("%-8s: %-12s GOOD\n"),
					drivers[game]->name, aud->rom);
				VerifyDumpHashData(aud->hash, NULL, verify_printf);
#endif
				break;
			case AUD_DISK_NOT_FOUND:
				verify_printf (_("%-8s: %-12s NOT FOUND\n"),
					drivers[game]->name, aud->rom);
				VerifyDumpHashData(aud->exphash, NULL, verify_printf);
				break;
			case AUD_DISK_BAD_MD5:
				verify_printf (_("%-8s: %-12s INCORRECT CHECKSUM:\n"),
					drivers[game]->name, aud->rom);
				VerifyDumpHashData(aud->exphash, _("EXPECTED: "), verify_printf);
				VerifyDumpHashData(aud->hash,    _("   FOUND: "), verify_printf);
				break;
			case AUD_DISK_NOT_AVAILABLE:
				verify_printf (_("%-8s: %-12s NO GOOD DUMP KNOWN\n"),
					drivers[game]->name, aud->rom);
				break;
			case AUD_DISK_NEED_DUMP:
				verify_printf (_("%-8s: %-12s FOUND BUT NO GOOD DUMP KNOWN\n"),
					drivers[game]->name, aud->rom);
				VerifyDumpHashData(aud->hash, NULL, verify_printf);
				break;
		}
		aud++;
	}

	if (archive_status & (AUD_ROM_NOT_FOUND|AUD_ROM_NOT_FOUND_PARENT|AUD_ROM_NOT_FOUND_BIOS|AUD_BAD_CHECKSUM|AUD_MEM_ERROR|AUD_LENGTH_MISMATCH|AUD_DISK_NOT_FOUND|AUD_DISK_BAD_MD5))
		return INCORRECT;
	if (archive_status & (AUD_ROM_NEED_DUMP|AUD_ROM_NEED_REDUMP|AUD_NOT_AVAILABLE|AUD_DISK_NOT_AVAILABLE|AUD_DISK_NEED_DUMP))
		return BEST_AVAILABLE;
	if (archive_status & (AUD_OPTIONAL_ROM_NOT_FOUND))
		return MISSING_OPTIONAL;

	return CORRECT;

}

#ifdef FAST_AUDIT
const char *GetFastAuditResultFile(void)
{
	return gszFastAuditResultFile;
}

void FastAuditOutput(const char*lpszFmt,...)
{
	char szBuf[2048];
	va_list vargs;

	va_start(vargs,lpszFmt);
	vsprintf(szBuf, lpszFmt, vargs);
	va_end(vargs);

	if( !_audfp ) {
		_audfp = fopen( GetFastAuditResultFile(), "wt" );
		if( _audfp ) _AuditInit = 0;
	}		
	if( !_audfp ) return;
		
	fprintf( _audfp, "%s", szBuf );
}

void CloseFastAuditOutput(void)
{
	if( !_audfp ) return;
	fclose( _audfp );
	_audfp = 0;
	_AuditInit = 0;
}

int get_game_rom_list(const char *gamename, GAMEROMLIST *lpRomList, int iListBaseIndex, int *lpiDuplicated)
{
	int pathcount, pathindex, pathstart, pathstop, pathinc, iCount, iDup;
	char szoldpath[256];
	GAMEROMLIST *lpList;

	pathcount = osd_get_path_count(FILETYPE_ROM);

	pathstart = 0;
	pathstop  = pathcount;
	pathinc   = 1;

	iDup	  = 0;
	iCount	  = 0;
	lpList	  = &lpRomList[iListBaseIndex];

	getcwd( szoldpath, 256 );

	/* loop over paths */
	for (pathindex = pathstart; pathindex != pathstop; pathindex += pathinc)
	{
		char fullpath[1024];

		osd_compose_path(fullpath, FILETYPE_ROM, pathindex, gamename);

		/* if the directory exists, proceed */
		if (chdir(fullpath) == 0)
		{
			struct _finddata_t c_file;
			long hFile;

			if( (hFile = _findfirst("*.*", &c_file)) != -1L )
			{
				while(1) {
					lpList[iCount].iType = 0;
					lpList[iCount].crc32 = 0;
					strcpy( lpList[iCount].szROM, c_file.name );
					iCount++;

					if( _findnext( hFile, &c_file ) != 0 ) break;
				}
				_findclose(hFile);
			}

			chdir(szoldpath);
			iDup++;
		}

		/* first look for path/gamename.zip */
		sprintf( fullpath, "%s.zip", gamename );

		/* if the ZIP file exists, proceed */
		if (osd_get_path_info(FILETYPE_ROM, pathindex, fullpath) == PATH_IS_FILE)
		{
			ZIP* zip;
			struct zipent* ent;

			zip = openzip(FILETYPE_ROM, pathindex, fullpath);
			if (zip) {
				while (readzip(zip)) {
					ent = &(zip->ent);

					lpList[iCount].iType = 1;
					lpList[iCount].crc32 = ent->crc32;
					strcpy( lpList[iCount].szROM, ent->name );
					iCount++;
				}

				suspendzip(zip);
				iDup++;
			}
		}
	}

	if(lpiDuplicated) *lpiDuplicated = iDup;

	return iCount;
}

int FastAuditRomSet (int game, tAuditRecord **audit)
{
#define CHECK_SELECTED_BIOS_ONLY

	const struct RomModule *region, *rom;
	const char *name;
	const struct GameDriver *gamedrv;
	const struct GameDriver *drv;
	UINT8  crcs[4];
	UINT32 crc32;

#ifdef CHECK_SELECTED_BIOS_ONLY
	int system_bios = determine_bios_rom(drivers[game]->bios);
#endif /* CHECK_SELECTED_BIOS_ONLY */

	int i, iRomExist, iRomCount, count, iDuplicated;
	tAuditRecord *aud;
	int	err;

	if (!gAudits)
	{
		gAudits = (tAuditRecord *)malloc (AUD_MAX_ROMS * sizeof (tAuditRecord));

		// Make sure the memory is cleared - it's needed by the hashing
		//  engine
		memset(gAudits, 0, AUD_MAX_ROMS * sizeof(tAuditRecord));
	}

	if( !gRomList)
	{
		gRomList = (GAMEROMLIST *)malloc (AUD_MAX_ROMS * sizeof (GAMEROMLIST));
		memset(gRomList, 0, AUD_MAX_ROMS * sizeof(GAMEROMLIST));
	}

	if (gAudits)
		*audit = aud = gAudits;
	else
		return 0;

	gamedrv = drivers[game];

	if (!gamedrv->rom) return -1;

	iRomCount = 0;

	/* check for existence of romset */
	iRomExist = mame_faccess(gamedrv->name, FILETYPE_ROM);
	if (!iRomExist )
	{
		/* if the game is a clone, check for parent */
		if (gamedrv->clone_of == 0 || (gamedrv->clone_of->flags & NOT_A_DRIVER) ||
				!mame_faccess(gamedrv->clone_of->name,FILETYPE_ROM))
			return 0;
	}
	else {
		i = get_game_rom_list( gamedrv->name, gRomList, iRomCount, &iDuplicated );
		iRomCount += i;
	}

	/* if the game is a clone, check for parent */
	drv = gamedrv->clone_of;
	while(drv)
	{
		if (drv->name[0] && mame_faccess (drv->name, FILETYPE_ROM))
		{
			i = get_game_rom_list( drv->name, gRomList, iRomCount, 0 );
			iRomCount += i;
		}
		drv = drv->clone_of;
	}

#if 0
	logmsg( "<%s> search result ---------\n", gamedrv->name );
	for(i=0; i<iRomCount; i++) {
		logmsg( "%2d) %d:<%-12s>:%08x\n", i, gRomList[i].iType, gRomList[i].szROM, gRomList[i].crc32 );
	}
	logmsg( "\n" );
#endif

	if( !iRomCount ) return 0;

	count = 0;

	for (region = rom_first_region(gamedrv); region; region = rom_next_region(region)) {
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			if (ROMREGION_ISROMDATA(region))
			{
#ifdef CHECK_SELECTED_BIOS_ONLY
			if (!ROM_GETBIOSFLAGS(rom) || (ROM_GETBIOSFLAGS(rom) == (system_bios+1))) /* alternate bios sets */
			{
#endif /* CHECK_SELECTED_BIOS_ONLY */
				name = ROM_GETNAME(rom);
				strcpy (aud->rom, name);
				aud->explength = 0;
				aud->length	   = 0;
				aud->status    = AUD_ROM_GOOD;
				aud->exphash   = ROM_GETHASHDATA(rom);
				hash_data_copy(aud->hash, aud->exphash);
				count++;

				crc32 = 0;
				if (hash_data_extract_binary_checksum(aud->hash, HASH_CRC, crcs) != 0)
				{
					/* Store the CRC in a single DWORD */
					crc32 = ((unsigned long)crcs[0] << 24) | ((unsigned long)crcs[1] << 16) |
						    ((unsigned long)crcs[2] <<  8) | ((unsigned long)crcs[3] <<  0);
				}
				aud->explength = crc32;

				err = 1;
				for(i=0; i<iRomCount; i++) {
					if( gRomList[i].iType == 1 ) // ZIP format
					{
						
						if(!stricmp(gRomList[i].szROM, name)) 
						{ 
							// 파일명으로 찾았으나 CRC가 다른 경우
							if(crc32 && (gRomList[i].crc32 != crc32)) {
								aud->length = gRomList[i].crc32;
								err = 2;
							}
							else 
								err = 0; 

							break; 
						}

						// 파일명은 다르나 CRC가 같은 경우
						if(crc32 && (gRomList[i].crc32 == crc32)) 
						{ 	err = 0; break; }
					}
					else {
						// ZIP포맷이 아닌 경우 파일명으로만 체크
						if(!stricmp(gRomList[i].szROM, name)) { err = 0; break; }
					}
				}

				switch(err) {
				case 1:
					if (hash_data_has_info(aud->exphash, HASH_INFO_NO_DUMP))
					{
						/* not found but it's not good anyway */
						aud->status = AUD_NOT_AVAILABLE;
					}
					else if( ROM_ISOPTIONAL(rom) ) {
						aud->status = AUD_OPTIONAL_ROM_NOT_FOUND;
					}
					else {
						aud->status = AUD_ROM_NOT_FOUND;
					}
					break;
				case 2:
					aud->status = AUD_BAD_CHECKSUM;
					break;
				case 0: // GOOD
					/* matching hash */
					if (hash_data_has_info(aud->exphash, HASH_INFO_NO_DUMP))
						aud->status = AUD_ROM_NEED_DUMP; /* new case - found but not known to be dumped */
					else if (hash_data_has_info(aud->exphash, HASH_INFO_BAD_DUMP))
						aud->status = AUD_ROM_NEED_REDUMP;
					break;
				}

				if( !iRomExist ) aud->status |= 0x80000000;

				aud++;
#ifdef CHECK_SELECTED_BIOS_ONLY
			}
#endif /* CHECK_SELECTED_BIOS_ONLY */	
			}
			else if (ROMREGION_ISDISKDATA(region))
			{
				const UINT8 nullhash[HASH_BUF_SIZE] = { 0 };
				void *source;
				struct chd_header header;

				name = ROM_GETNAME(rom);
				strcpy (aud->rom, name);
				aud->explength = 0;
				aud->length = 0;
				aud->exphash = ROM_GETHASHDATA(rom);
				hash_data_clear(aud->hash);
				count++;

				chd_gamedrv = gamedrv;
				chd_set_interface(&audit_chd_interface);
				source = chd_open( name, 0, NULL );
				if( source == NULL )
				{
					err = chd_get_last_error();
					if( err == CHDERR_OUT_OF_MEMORY )
					{
						aud->status = AUD_MEM_ERROR;
					}
					else if (hash_data_has_info(aud->exphash, HASH_INFO_NO_DUMP))
					{
						/* not found but it's not good anyway */
						aud->status = AUD_DISK_NOT_AVAILABLE;
					}
					else
					{
						/* not found */
						aud->status = AUD_DISK_NOT_FOUND;
					}
				}
				else
				{
					header = *chd_get_header(source);

					if (memcmp(nullhash, header.md5, sizeof(header.md5)))
						hash_data_insert_binary_checksum(aud->hash, HASH_MD5, header.md5);
					if (memcmp(nullhash, header.sha1, sizeof(header.sha1)))
						hash_data_insert_binary_checksum(aud->hash, HASH_SHA1, header.sha1);
					
					 if (hash_data_has_info(aud->exphash, HASH_INFO_NO_DUMP))
					{
						aud->status = AUD_DISK_NEED_DUMP;
					}
					else if (!hash_data_is_equal(aud->exphash, aud->hash, 0))
					{
						aud->status = AUD_DISK_BAD_MD5;
					}
					else
					{
						aud->status = AUD_DISK_GOOD;
					}

					chd_close( source );
				}

				if( !iRomExist ) aud->status |= 0x80000000;

				aud++;
			}
		}
	}

	gDuplicated = iDuplicated;

	return count;
}

int FastVerifyRomSet (int game, verify_printf_proc verify_printf)
{
	tAuditRecord			*aud;
	int						count, i, isBad;
	int						archive_status = 0;
	char					szbuf[256];
	const char				*str;

	if( _AuditInit == 0 ) {
		int pathcount, i;
		char fullpath[1024];

		FastAuditOutput("MAME %s\n\n", build_version );

		getcwd( szbuf, 256 );
		FastAuditOutput("current MAME Directories: %s\n", szbuf );
		
		pathcount = osd_get_path_count(FILETYPE_ROM);
		FastAuditOutput("List of Directories: (%d)\n", pathcount );
		for(i=0; i<pathcount; i++) {
			osd_compose_path(fullpath, FILETYPE_ROM, i, " ");
			FastAuditOutput("    : %s\n",fullpath );
		}
		FastAuditOutput("------------------------------------------------------------------------\n");
		_AuditInit = 1;

		_FastAuditTotal 	= 0;
		_FastAuditNotFound	= 0;
		_FastAuditBad		= 0;
		_FastAuditMissing	= 0;
		_FastAuditGood		= 0;
		
		gBadRomInfo.next = 0;
		ptBadRom = &gBadRomInfo;
	}
	
	_FastAuditTotal++;
	
	if ((count = FastAuditRomSet (game, &aud)) == 0) {
		_FastAuditNotFound++;
		return NOTFOUND;
	}

	if (count == -1) {
		_FastAuditGood++;
		return CORRECT;
	}

	for(i=0; i<count; i++) {
		archive_status |= aud[i].status;
	}

	if( !(archive_status & 0x80000000) ) {

		if( (archive_status != AUD_ROM_GOOD) || (gDuplicated > 1) ) {
			FastAuditOutput("\n%-8s: \"%s\"\n", drivers[game]->name, drivers[game]->description );
			if( gDuplicated > 1 ) {
				FastAuditOutput("    %s\n", gszDuplicated );
			}
		}

		isBad = 0;
		for(i=0; i<count; i++)
		{
			str = FindStatusString(aud[i].status);
			switch (aud[i].status)
			{
				case AUD_ROM_GOOD:
				case AUD_DISK_GOOD:
#if 0    /* if you want a full accounting of roms */
					FastAuditOutput("    %-12s: %s\n", aud[i].rom, str);
#endif
					break;
				case AUD_ROM_NOT_FOUND:
				case AUD_OPTIONAL_ROM_NOT_FOUND:
				case AUD_NOT_AVAILABLE:
				case AUD_ROM_NEED_DUMP:
				case AUD_ROM_NEED_REDUMP:
				case AUD_MEM_ERROR:
				case AUD_LENGTH_MISMATCH:
				case AUD_DISK_NOT_FOUND:
				case AUD_DISK_NOT_AVAILABLE:
				case AUD_DISK_NEED_DUMP:
					FastAuditOutput("    %-12s: %s\n", aud[i].rom, str);
					break;
				case AUD_BAD_CHECKSUM:
					FastAuditOutput("    %-12s %s\n", aud[i].rom, str);
					FastAuditOutput("        %s CRC(%08x)\n", gszExpected, aud[i].explength );
					FastAuditOutput("        %s CRC(%08x)\n", gszFound, aud[i].length );
					break;
				case AUD_DISK_BAD_MD5:
					FastAuditOutput("    %-12s %s\n", aud[i].rom, str);
					hash_data_print(aud[i].exphash, 0, szbuf);
					FastAuditOutput("        %s %s\n", gszExpected, szbuf );
					hash_data_print(aud[i].hash,    0, szbuf);
					FastAuditOutput("        %s %s\n", gszFound, szbuf );
					break;
				default:
					FastAuditOutput("    %-12s State:%08x\n", aud[i].rom, aud[i].status);
					break;
			}

			switch (aud[i].status)
			{
				case AUD_ROM_NOT_FOUND:
				case AUD_MEM_ERROR:
				case AUD_LENGTH_MISMATCH:
				case AUD_DISK_NOT_FOUND:
				case AUD_BAD_CHECKSUM:
				case AUD_DISK_BAD_MD5:
					isBad++;
					break;
			}
			
		}

		if(isBad) {
			GAMEROMINFO	*tmpInfo;
			
			tmpInfo = (GAMEROMINFO *)malloc(sizeof(GAMEROMINFO));
		
			strcpy( tmpInfo->szROM,		drivers[game]->name );
			strcpy( tmpInfo->szDesc,	drivers[game]->description );
			tmpInfo->szParentROM[0] = 0;
			if(drivers[game]->clone_of) 
				strcpy( tmpInfo->szParentROM,	drivers[game]->clone_of->name );
			
			tmpInfo->next = NULL;

			if(ptBadRom) ptBadRom->next = tmpInfo;
			ptBadRom = tmpInfo;			

			_FastAuditBad++;
		}
	}
	else {
		archive_status ^= 0x80000000; // clear bit
	}

	if (archive_status & (AUD_ROM_NOT_FOUND|AUD_BAD_CHECKSUM|AUD_MEM_ERROR|AUD_LENGTH_MISMATCH|AUD_DISK_NOT_FOUND|AUD_DISK_BAD_MD5)) {
		return INCORRECT;
	}

	if (archive_status & (AUD_ROM_NEED_DUMP|AUD_ROM_NEED_REDUMP|AUD_NOT_AVAILABLE|AUD_DISK_NOT_AVAILABLE|AUD_DISK_NEED_DUMP)) {
		_FastAuditGood++;
		return BEST_AVAILABLE;
	}
	
	if (archive_status & (AUD_OPTIONAL_ROM_NOT_FOUND)) {
		FastAuditOutput("    [MIS] %-8s: \"%s\"\n", drivers[game]->name, drivers[game]->description );
		_FastAuditMissing++;
		return MISSING_OPTIONAL;
	}

	_FastAuditGood++;
	return CORRECT;

}

static int CLIB_DECL BadRomInfoCompareFunc(const void *arg1,const void *arg2)
{
    return strcmp( ((GAMEROMINFO *)arg1)->szROM, ((GAMEROMINFO *)arg2)->szROM );
}

int FinalizeAudit(void)
{
	GAMEROMINFO *tmpInfo, *SortInfo;
	int i, iCnt;
	
	if(gAudits)  free(gAudits);
	if(gRomList) free(gRomList);

	gAudits  = 0;
	gRomList = 0;

	// 현재까지 output한게 없으면 그냥 0리턴
	if( !_audfp ) return 0;
	
	if(_FastAuditTotal > 0) {
		int iTemp;
		
		iTemp = _FastAuditTotal - (_FastAuditNotFound + _FastAuditGood);
		FastAuditOutput("------------------------------------------------------------------------\n");
		FastAuditOutput("    [Audit Resilt]   \n");
		FastAuditOutput("    All Roms       : %d\n", _FastAuditTotal );
		FastAuditOutput("    NotFound Roms  : %d + %d\n", _FastAuditNotFound, iTemp );
		FastAuditOutput("    Correct Roms   : %d\n", _FastAuditGood );
		FastAuditOutput("    Bad Roms       : %d\n", _FastAuditBad );
		FastAuditOutput("    Missing Roms   : %d (Missing Optinal)\n\n", _FastAuditMissing );
	}

	// 잘못된 롬리스트 정보 출력 -----------------------------
	iCnt = 0;
	ptBadRom = gBadRomInfo.next;
	while(ptBadRom != NULL) { iCnt++; ptBadRom = ptBadRom->next; }
	
	if( iCnt > 0) {
		// 정렬을 위한 새버퍼 할당
		SortInfo = (GAMEROMINFO *)malloc( iCnt * sizeof(GAMEROMINFO) );

		// 새버퍼로 자료을 옮기면서 기존버퍼 해제
		i = 0;
		ptBadRom = gBadRomInfo.next;
		while(ptBadRom != NULL) { 
			
			memcpy(&SortInfo[i], ptBadRom, sizeof(GAMEROMINFO) );
			
			i++;
			tmpInfo = ptBadRom->next; 
			free(ptBadRom);
			ptBadRom = tmpInfo;
		}
		gBadRomInfo.next = NULL;

		// 정렬
		qsort(SortInfo, iCnt, sizeof(GAMEROMINFO), BadRomInfoCompareFunc );

		// 정보 출력
		FastAuditOutput("    Bad Roms List (%d):\n", iCnt );
		ptBadRom = gBadRomInfo.next;
		for(i=0; i<iCnt; i++) {
			FastAuditOutput("        %-8s: \"%s\" ", SortInfo[i].szROM, SortInfo[i].szDesc);
			if( strlen(SortInfo[i].szParentROM) > 0 ) 
				FastAuditOutput("[%s]\n", SortInfo[i].szParentROM );
			else
				FastAuditOutput("\n" );
		}

		// 새버퍼 해제
		free(SortInfo);

		FastAuditOutput("\n ※ Warnning: Some of Missing Roms may be caused by BIOS of Game-Roms \n");
	}
	
	FastAuditOutput("\nEnd.\n" );

	CloseFastAuditOutput();
	
	return 1;
}
#endif /*FAST_AUDIT*/

static tMissingSample *gMissingSamples = NULL;

/* Builds a list of every missing sample. Returns total number of missing
   samples, or -1 if no samples were found. Sets audit to point to the
   list of missing samples. */
int AuditSampleSet (int game, tMissingSample **audit)
{
	struct InternalMachineDriver drv;
	int skipfirst;
	mame_file *f;
	const char **samplenames, *sharedname;
	int exist;
	static const struct GameDriver *gamedrv;
	int j;
	int count = 0;
	tMissingSample *aud;

	gamedrv = drivers[game];
	expand_machine_driver(gamedrv->drv, &drv);

	samplenames = NULL;
#if (HAS_SAMPLES || HAS_VLM5030)
	for( j = 0; drv.sound[j].sound_type && j < MAX_SOUND; j++ )
	{
#if (HAS_SAMPLES)
		if( drv.sound[j].sound_type == SOUND_SAMPLES )
			samplenames = ((struct Samplesinterface *)drv.sound[j].config)->samplenames;
#endif
	}
#endif
    /* does the game use samples at all? */
	if (samplenames == 0 || samplenames[0] == 0)
		return 0;

	/* take care of shared samples */
	if (samplenames[0][0] == '*')
	{
		sharedname=samplenames[0]+1;
		skipfirst = 1;
	}
	else
	{
		sharedname = NULL;
		skipfirst = 0;
	}

	/* do we have samples for this game? */
	exist = mame_faccess (gamedrv->name, FILETYPE_SAMPLE);

	/* try shared samples */
	if (!exist && skipfirst)
		exist = mame_faccess (sharedname, FILETYPE_SAMPLE);

	/* if still not found, we're done */
	if (!exist)
		return -1;

	/* allocate missing samples list (if necessary) */
	if (!gMissingSamples)
		gMissingSamples = (tMissingSample *)malloc (AUD_MAX_SAMPLES * sizeof (tMissingSample));

	if (gMissingSamples)
		*audit = aud = gMissingSamples;
	else
		return 0;

	for (j = skipfirst; samplenames[j] != 0; j++)
	{
		/* skip empty definitions */
		if (strlen (samplenames[j]) == 0)
			continue;
		f = mame_fopen (gamedrv->name, samplenames[j], FILETYPE_SAMPLE, 0);
		if (f == NULL && skipfirst)
			f = mame_fopen (sharedname, samplenames[j], FILETYPE_SAMPLE, 0);

		if (f)
			mame_fclose(f);
		else
		{
			strcpy (aud->name, samplenames[j]);
			count++;
			aud++;
		}
	}
	return count;
}


/* Generic function for evaluating a sampleset. Some platforms may wish to
   call AuditSampleSet() instead and implement their own reporting (like MacMAME). */
int VerifySampleSet (int game, verify_printf_proc verify_printf)
{
	tMissingSample	*aud;
	int				count;

	count = AuditSampleSet (game, &aud);
	if (count==-1)
		return NOTFOUND;
	else if (count==0)
		return CORRECT;

	/* list missing samples */
	while (count--)
	{
		verify_printf (_("%-8s: %s NOT FOUND\n"), drivers[game]->name, aud->name);
		aud++;
	}

	return INCORRECT;
}
