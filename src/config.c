/***************************************************************************

    config.c

    Configuration file I/O.

***************************************************************************/

#include "driver.h"
#include "config.h"
#include "common.h"
#include "expat.h"


#define DEBUG_CONFIG		0

#ifdef KAILLERA
extern int kPlay;
#endif /* KAILLERA */

/*************************************
 *
 *  Constants
 *
 *************************************/

#define CONFIG_VERSION			10
#define TEMP_BUFFER_SIZE		4096
#define MAX_DEPTH				8
#define MAX_MIXER_CHANNELS		100

enum
{
	FILE_TYPE_GAME,				/* game-specific config file */
	FILE_TYPE_DEFAULT,			/* default keys config file */
	FILE_TYPE_CONTROLLER		/* controller-specific config file */
};



/*************************************
 *
 *  Type definitions
 *
 *************************************/

struct config_port
{
	struct config_port *	next;				/* next in the list */
	int						type;				/* type attribute */
	int						player;				/* player attribute */
	int						mask;				/* mask for the port */
	int						defvalue;			/* default value of the port */
	int						value;				/* value of the port */
	int						keydelta;			/* keydelta attribute */
	int						centerdelta;		/* centerdelta attribute */
	int						sensitivity;		/* sensitivity attribute */
	int						reverse;			/* reverse attribute */
	input_seq_t				defseq[3];			/* default sequences */
	input_seq_t				newseq[3];			/* current sequences */
	int						autofire;		/* autofire mode */
#ifdef USE_CUSTOM_BUTTON
	int						custom;			/* button mapping */
#endif /* USE_CUSTOM_BUTTON */
};


struct config_counters
{
	int						coins[COIN_COUNTERS];/* array of coin counter values */
	int						tickets;			/* tickets dispensed */
};


struct config_mixer
{
	float					deflevel[MAX_MIXER_CHANNELS];/* array of mixer values */
	float					newlevel[MAX_MIXER_CHANNELS];/* array of mixer values */
#ifdef USE_VOLUME_AUTO_ADJUST
	INT32					volume_multiplier_final;	/* volume multiplier value for final output */
	INT32					volume_multiplier_final_max;	/* volume multiplier maximum value for final output */
	INT32					volume_multiplier_mixer;	/* volume multiplier value for mixer */
	INT32					volume_multiplier_mixer_max;	/* volume multiplier maximum value for mixer */
#endif /* USE_VOLUME_AUTO_ADJUST */
};


struct config_mameplus
{
	int			autofire_delay;
	INT16			cpu_clocks[MAX_CPU];
};


struct config_data
{
	int						version;			/* from the <mameconfig> field */
	UINT8					ignore_game;		/* are we ignoring this game entry? */
	UINT8					loaded_count;		/* number of systems we loaded */
	input_code_t			remap[__code_max];	/* remap table */
	struct config_port *	port;				/* from the <port> field */
	struct config_counters  counters;			/* from the <counters> field */
	struct config_mixer		mixer;				/* from the <mixer> field */
	struct config_mameplus	mameplus;			/* from the <mameplus> field */
};


struct config_parse
{
	XML_Char				tag[1024];			/* combined tags */
	XML_Char				data[1024];			/* accumulated data on the current tag */
	int						datalen;			/* accumulated data length */
	int						seq_index;			/* current index in the sequence array */
};


struct config_file
{
	mame_file *				file;				/* handle to the file */
	int						filetype;			/* what type of config file is this? */
	struct config_data		data;				/* the accumulated data */
	struct config_parse		parse;				/* parsing info */
};



/*************************************
 *
 *  Global variables
 *
 *************************************/

static struct config_file curfile;

static const char *seqtypestrings[] = { "standard", "decrement", "increment" };

extern short cpu_clocks[MAX_CPU];



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void cleanup_file(void);

static int config_load_xml(void);
static int config_save_xml(void);

static int apply_ports(const struct InputPort *input_ports_default, struct InputPort *input_ports);
static int apply_default_ports(const struct InputPortDefinition *input_ports_default_backup, struct InputPortDefinition *input_ports_default);
static int apply_counters(void);
static int apply_mixer(void);
static int apply_mameplus(void);

static int build_ports(const struct InputPort *input_ports_default, const struct InputPort *input_ports);
static int build_default_ports(const struct InputPortDefinition *input_ports_default_backup, const struct InputPortDefinition *input_ports_default);
static int build_counters(void);
static int build_mixer(void);
static int build_mameplus(void);



/*************************************
 *
 *	Token encoder/decoder
 *
 *************************************/

static void utf8_decode_string(const unsigned char *s, unsigned char *buffer, int maxlen)
{
	char *d = &buffer[0];

	maxlen--;

	if (s)
	{
		// decode UTF-8
		while (*s && maxlen > 0)
		{
			// 3 bytes -> 2 bytes
			if ((*s & 0xf0) == 0xe0)
			{
				if ((s[1] & 0xc0) != 0x80)
					break;

				if ((s[2] & 0xc0) != 0x80)
					break;

				if (maxlen < 2)
					break;

				*d = (*s++ & 0x0f) << 4;
				*d++ |= (*s & 0x3c) >> 2;

				*d = (*s++ & 0x03) << 6;
				*d++ |= (*s++ & 0x3f);
			}

			// 2 bytes -> 1 or 2 bytes
			else if ((*s & 0xd0) == 0xc0)
			{
				unsigned char high;

				if ((s[1] & 0xc0) != 0x80)
					break;

				high = (*s & 0x1c) >> 2;

				if (high)
				{
					if (maxlen < 2)
						break;
				
					*d++ = high;
				}
				else if (maxlen < 1)
					break;

				*d = (*s++ & 0x3) << 6;
				*d++ |= (*s++ & 0x3f);
			}

			// 1 bytes
			else if ((*s & 0x80) == 0x00)
			{
				if (maxlen < 1)
					break;

				*d++ = *s++;
			}

			// Surrogate: not supproted (never used??)
			else
			{
				break;
			}
		}
	}
	*d++ = 0;
}

static void xml_encode_string(const unsigned char *s, unsigned char *buffer, int maxlen)
{
	char *d = &buffer[0];

	maxlen--;

	if (s)
	{
		while (*s && maxlen > 0)
		{
			char buf[256];
			int len = 0;

			switch (*s)
			{
				case '\"' : len = sprintf(buf, "&quot;"); break;
				case '&'  : len = sprintf(buf, "&amp;"); break;
				case '<'  : len = sprintf(buf, "&lt;"); break;
				case '>'  : len = sprintf(buf, "&gt;"); break;
				default:
					if (*s>=' ' && *s<='~')
						len = sprintf(buf, "%c", *s);
					else
						len = sprintf(buf, "&#%d;", (unsigned)(unsigned char)*s);
			}
			if (len > maxlen)
				break;

			strcpy(d, buf);
			d += len;
			maxlen -= len;

			++s;
		}
	}
	*d++ = 0;
}

static int xml_string_to_seq(const char *string, input_seq_t *seq)
{
	char buffer[256];

	utf8_decode_string(string, buffer, sizeof buffer);
	return string_to_seq(buffer, seq);
}

static void xml_seq_to_string(const input_seq_t *seq, char *string, int maxlen)
{
	char buffer[256];

	seq_to_string(seq, buffer, sizeof buffer);
	xml_encode_string(buffer, string, maxlen);
}



/*************************************
 *
 *  Game data save/load
 *
 *************************************/

int config_load(const struct InputPort *input_ports_default, struct InputPort *input_ports)
{
	int result;

	/* clear current file data to zero */
	memset(&curfile, 0, sizeof(curfile));

	/* open the config file */
	curfile.file = mame_fopen(Machine->gamedrv->name, 0, FILETYPE_CONFIG, 0);
	if (!curfile.file)
		return 0;
	curfile.filetype = FILE_TYPE_GAME;

	/* load the XML */
	result = config_load_xml();
	mame_fclose(curfile.file);
	if (!result)
	{
		cleanup_file();
		return 0;
	}

	/* first apply counters and mixer data; it's okay if they fail */
	apply_counters();
	apply_mixer();
	apply_mameplus();
	
	/* return true only if we can apply the port data */
	result = apply_ports(input_ports_default, input_ports);
	cleanup_file();
	return result;
}


void config_save(const struct InputPort *input_ports_default, const struct InputPort *input_ports)
{
	/* clear current file data to zero */
	memset(&curfile, 0, sizeof(curfile));

	/* build the various components in memory */
	if (!build_ports(input_ports_default, input_ports))
		goto error;
	if (!build_counters())
		goto error;
	if (!build_mixer())
		goto error;
	if (!build_mameplus())
		goto error;

	/* open the config file */
	curfile.file = mame_fopen(Machine->gamedrv->name, 0, FILETYPE_CONFIG, 1);
	if (!curfile.file)
		return;
	curfile.filetype = FILE_TYPE_GAME;

	/* save the XML */
	config_save_xml();
	mame_fclose(curfile.file);

error:
	cleanup_file();
}



/*************************************
 *
 *  Default control data save/load
 *
 *************************************/

int config_load_default(const struct InputPortDefinition *input_ports_backup, struct InputPortDefinition *input_ports)
{
	int result;

	/* clear current file data to zero */
	memset(&curfile, 0, sizeof(curfile));

	/* open the config file */
	curfile.file = mame_fopen("default", 0, FILETYPE_CONFIG, 0);
	if (!curfile.file)
		return 0;
	curfile.filetype = FILE_TYPE_DEFAULT;

	/* load the XML */
	result = config_load_xml();
	mame_fclose(curfile.file);
	if (!result)
		return 0;

	/* return true only if we can apply the port data */
	result = apply_default_ports(input_ports_backup, input_ports);
	cleanup_file();
	return result;
}


void config_save_default(const struct InputPortDefinition *input_ports_backup, const struct InputPortDefinition *input_ports)
{
	/* clear current file data to zero */
	memset(&curfile, 0, sizeof(curfile));

	/* build the various components in memory */
	if (!build_default_ports(input_ports_backup, input_ports))
		return;

	/* open the config file */
	curfile.file = mame_fopen("default", 0, FILETYPE_CONFIG, 1);
	if (!curfile.file)
		goto error;
	curfile.filetype = FILE_TYPE_DEFAULT;

	/* save the XML */
	config_save_xml();
	mame_fclose(curfile.file);

error:
	cleanup_file();
}

#ifdef SHOW_UIKEY
int config_load_default_ui(const char *lpszCfgDir, const struct InputPortDefinition *input_ports_backup, struct InputPortDefinition *input_ports)
{
	char szCfgFileName[256];
	int result;
	
	/* clear current file data to zero */
	memset(&curfile, 0, sizeof(curfile));

	/* setup cfg filename */	
	if((lpszCfgDir == NULL) || (lpszCfgDir[0] == 0)) 
		sprintf(szCfgFileName, "%s", "default" );
	else
		sprintf(szCfgFileName, "%s\\%s", lpszCfgDir, "default" );
		
	/* open the config file */
	curfile.file = mame_fopen(szCfgFileName, 0, FILETYPE_CONFIG, 0);
	
	if (!curfile.file)
		return 0;
	curfile.filetype = FILE_TYPE_DEFAULT;
	
	/* load the XML */
	result = config_load_xml();
	mame_fclose(curfile.file);
	if (!result)
		return 0;

	/* return true only if we can apply the port data */
	result = apply_default_ports(input_ports_backup, input_ports);
	cleanup_file();
	return result;
}

void config_save_default_ui(const char *lpszCfgDir, const struct InputPortDefinition *input_ports_backup, const struct InputPortDefinition *input_ports)
{
	char szCfgFileName[256];
	
	/* clear current file data to zero */
	memset(&curfile, 0, sizeof(curfile));

	/* setup cfg filename */	
	if((lpszCfgDir == NULL) || (lpszCfgDir[0] == 0)) 
		sprintf(szCfgFileName, "%s", "default" );
	else
		sprintf(szCfgFileName, "%s\\%s", lpszCfgDir, "default" );
	
	/* build the various components in memory */
	if (!build_default_ports(input_ports_backup, input_ports))
		return;
	
	/* open the config file */
	curfile.file = mame_fopen(szCfgFileName, 0, FILETYPE_CONFIG, 1);
	if (!curfile.file)
		goto error;
	curfile.filetype = FILE_TYPE_DEFAULT;
	
	/* save the XML */
	config_save_xml();
	mame_fclose(curfile.file);

error:
	cleanup_file();
}
#endif /*SHOW_UIKEY*/

/*************************************
 *
 *  Controller-specific data load
 *
 *************************************/

int config_load_controller(const char *name, struct InputPortDefinition *input_ports)
{
	int result;

	/* clear current file data to zero */
	memset(&curfile, 0, sizeof(curfile));

	/* open the config file */
	curfile.file = mame_fopen(NULL, name, FILETYPE_CTRLR, 0);
	if (!curfile.file)
		return 0;
	curfile.filetype = FILE_TYPE_CONTROLLER;

	/* load the XML */
	result = config_load_xml();
	mame_fclose(curfile.file);
	if (!result)
		return 0;

	/* return true only if we can apply the port data */
	result = apply_default_ports(NULL, input_ports);
	cleanup_file();
	return result;
}



/*************************************
 *
 *  Allocated data cleanup
 *
 *************************************/

static void cleanup_file(void)
{
	struct config_port *port, *next;

	/* free any allocated port data */
	for (port = curfile.data.port; port != NULL; port = next)
	{
		next = port->next;
		free(port);
	}
}



/*************************************
 *
 *  Helper to streamline defaults
 *
 *************************************/

INLINE input_code_t get_default_code(int type)
{
	switch (type)
	{
		case IPT_DIPSWITCH_NAME:
		case IPT_CATEGORY_NAME:
			return CODE_NONE;

		default:
			if (curfile.filetype != FILE_TYPE_GAME)
				return CODE_NONE;
			else
				return CODE_DEFAULT;
	}
	return CODE_NONE;
}



/*************************************
 *
 *  XML loading callbacks
 *
 *************************************/

static void config_element_start(void *data, const XML_Char *name, const XML_Char **attributes)
{
	int attr;

	/* increase the depth and stash the tag */
	if (strlen(curfile.parse.tag) + strlen(name) + 1 < sizeof(curfile.parse.tag))
	{
		strcat(curfile.parse.tag, "/");
		strcat(curfile.parse.tag, name);
	}
	curfile.parse.datalen = 0;

	/* look for top-level mameconfig tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig"))
	{
		for (attr = 0; attributes[attr]; attr += 2)
		{
			if (!stricmp(attributes[attr], "version"))
				sscanf(attributes[attr + 1], "%d", &curfile.data.version);
		}
	}

	/* look for second-level system tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig/system"))
	{
		for (attr = 0; attributes[attr]; attr += 2)
			if (!stricmp(attributes[attr], "name"))
			{
				switch (curfile.filetype)
				{
					case FILE_TYPE_GAME:
						curfile.data.ignore_game = (strcmp(attributes[attr + 1], Machine->gamedrv->name) != 0);
						break;

					case FILE_TYPE_DEFAULT:
						curfile.data.ignore_game = (strcmp(attributes[attr + 1], "default") != 0);
						break;

					case FILE_TYPE_CONTROLLER:
					{
						const char *srcfile = strrchr(Machine->gamedrv->source_file, '/');
						if (!srcfile) srcfile = strrchr(Machine->gamedrv->source_file, '\\');
						if (!srcfile) srcfile = strrchr(Machine->gamedrv->source_file, ':');
						if (!srcfile) srcfile = Machine->gamedrv->source_file;
						else srcfile++;

						curfile.data.ignore_game =
							(strcmp(attributes[attr + 1], "default") != 0 &&
							 strcmp(attributes[attr + 1], Machine->gamedrv->name) != 0 &&
							 strcmp(attributes[attr + 1], srcfile) != 0 &&
							 (Machine->gamedrv->clone_of == NULL || strcmp(attributes[attr + 1], Machine->gamedrv->clone_of->name) != 0) &&
							 (Machine->gamedrv->clone_of == NULL || Machine->gamedrv->clone_of->clone_of == NULL || strcmp(attributes[attr + 1], Machine->gamedrv->clone_of->clone_of->name) != 0));

						if (DEBUG_CONFIG && !curfile.data.ignore_game)
							printf("Entry: %s -- processing\n", attributes[attr + 1]);
						break;
					}
				}
				curfile.data.loaded_count += !curfile.data.ignore_game;
			}
	}

	/* if we don't match the current game, punt now */
	if (curfile.data.ignore_game)
		return;

	/* look for second-level port tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig/system/input/port"))
	{
		struct config_port *port = malloc(sizeof(*port));
		curfile.parse.seq_index = -1;
		if (port != NULL)
		{
			int seqnum;
			memset(port, 0, sizeof(*port));

			port->next = curfile.data.port;
			curfile.data.port = port;
			for (attr = 0; attributes[attr]; attr += 2)
			{
				if (!stricmp(attributes[attr], "type"))
					curfile.data.port->type = token_to_port_type(attributes[attr + 1], &curfile.data.port->player);
				else if (!stricmp(attributes[attr], "mask"))
					sscanf(attributes[attr + 1], "%d", &curfile.data.port->mask);
				else if (!stricmp(attributes[attr], "defvalue"))
					sscanf(attributes[attr + 1], "%d", &curfile.data.port->defvalue);
				else if (!stricmp(attributes[attr], "value"))
					sscanf(attributes[attr + 1], "%d", &curfile.data.port->value);
				else if (!stricmp(attributes[attr], "keydelta"))
					sscanf(attributes[attr + 1], "%d", &curfile.data.port->keydelta);
				else if (!stricmp(attributes[attr], "centerdelta"))
					sscanf(attributes[attr + 1], "%d", &curfile.data.port->centerdelta);
				else if (!stricmp(attributes[attr], "sensitivity"))
					sscanf(attributes[attr + 1], "%d", &curfile.data.port->sensitivity);
				else if (!stricmp(attributes[attr], "reverse"))
				{
					if (!stricmp(attributes[attr + 1], "yes"))
						curfile.data.port->reverse = 1;
					else
						curfile.data.port->reverse = 0;
				}
				else if (!stricmp(attributes[attr], "autofire"))
				{
					if (!stricmp(attributes[attr + 1], "on"))
						curfile.data.port->autofire = 1;
					else if (!stricmp(attributes[attr + 1], "toggle"))
						curfile.data.port->autofire = 2;
				}
#ifdef USE_CUSTOM_BUTTON
				else if (!stricmp(attributes[attr], "custom"))
					sscanf(attributes[attr + 1], "%d", &curfile.data.port->custom);
#endif /* USE_CUSTOM_BUTTON */
			}

			for (seqnum = 0; seqnum < 3; seqnum++)
			{
				seq_set_1(&port->defseq[seqnum], get_default_code(curfile.data.port->type));
				seq_set_1(&port->newseq[seqnum], get_default_code(curfile.data.port->type));
			}
		}
	}

	/* look for second-level remap tag (we never write it, so it's only value for controller files) */
	if (curfile.filetype == FILE_TYPE_CONTROLLER && !stricmp(curfile.parse.tag, "/mameconfig/system/input/remap"))
	{
		input_code_t origcode = CODE_NONE, newcode = CODE_NONE;
		for (attr = 0; attributes[attr]; attr += 2)
		{
			if (!stricmp(attributes[attr], "origcode"))
				origcode = token_to_code(attributes[attr + 1]);
			else if (!stricmp(attributes[attr], "newcode"))
				newcode = token_to_code(attributes[attr + 1]);
		}
		if (origcode != CODE_NONE && origcode < __code_max && newcode != CODE_NONE)
			curfile.data.remap[origcode] = newcode;
	}

	/* look for third-level defseq or newseq tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig/system/input/port/defseq") || !stricmp(curfile.parse.tag, "/mameconfig/system/input/port/newseq"))
	{
		curfile.parse.seq_index = -1;
		for (attr = 0; attributes[attr]; attr += 2)
			if (!stricmp(attributes[attr], "type"))
			{
				int typenum;
				for (typenum = 0; typenum < 3; typenum++)
					if (!stricmp(attributes[attr + 1], seqtypestrings[typenum]))
					{
						curfile.parse.seq_index = typenum;
						break;
					}
			}
	}

	/* look for second-level coins tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig/system/counters/coins"))
	{
		int indx = -1, count;
		for (attr = 0; attributes[attr]; attr += 2)
		{
			if (!stricmp(attributes[attr], "index"))
				sscanf(attributes[attr + 1], "%d", &indx);
			else if (!stricmp(attributes[attr], "number") && indx >= 0 && indx < COIN_COUNTERS && sscanf(attributes[attr + 1], "%d", &count) == 1)
				curfile.data.counters.coins[indx] = count;
		}
	}

	/* look for second-level tickets tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig/system/counters/tickets"))
	{
		int count;
		for (attr = 0; attributes[attr]; attr += 2)
			if (!stricmp(attributes[attr], "number") && sscanf(attributes[attr + 1], "%d", &count) == 1)
				curfile.data.counters.tickets = count;
	}

	/* look for second-level default tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig/system/mixer/channel"))
	{
		int indx = -1;
		float value;
		for (attr = 0; attributes[attr]; attr += 2)
		{
			if (!stricmp(attributes[attr], "index"))
				sscanf(attributes[attr + 1], "%d", &indx);
			else if (!stricmp(attributes[attr], "defvol") && indx >= 0 && indx < MAX_MIXER_CHANNELS && sscanf(attributes[attr + 1], "%f", &value) == 1)
				curfile.data.mixer.deflevel[indx] = value;
			else if (!stricmp(attributes[attr], "newvol") && indx >= 0 && indx < MAX_MIXER_CHANNELS && sscanf(attributes[attr + 1], "%f", &value) == 1)
				curfile.data.mixer.newlevel[indx] = value;
		}
	}

#ifdef USE_VOLUME_AUTO_ADJUST
	/* look for second-level volume tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig/system/mixer/volume"))
	{
		INT32 volume_multiplier_final = DEFAULT_VOLUME_MULTIPLIER;
		INT32 volume_multiplier_final_max = DEFAULT_VOLUME_MULTIPLIER_MAX;
		INT32 volume_multiplier_mixer = DEFAULT_VOLUME_MULTIPLIER;
		INT32 volume_multiplier_mixer_max = DEFAULT_VOLUME_MULTIPLIER_MAX;

		for (attr = 0; attributes[attr]; attr += 2)
			if (!stricmp(attributes[attr], "multiplier_final") && sscanf(attributes[attr + 1], "%d", &volume_multiplier_final) == 1)
				curfile.data.mixer.volume_multiplier_final = volume_multiplier_final;
			else if (!stricmp(attributes[attr], "multiplier_final_max") && sscanf(attributes[attr + 1], "%d", &volume_multiplier_final_max) == 1)
				curfile.data.mixer.volume_multiplier_final_max = volume_multiplier_final_max;
			else if (!stricmp(attributes[attr], "multiplier_mixer") && sscanf(attributes[attr + 1], "%d", &volume_multiplier_mixer) == 1)
				curfile.data.mixer.volume_multiplier_mixer = volume_multiplier_mixer;
			else if (!stricmp(attributes[attr], "multiplier_mixer_max") && sscanf(attributes[attr + 1], "%d", &volume_multiplier_mixer_max) == 1)
				curfile.data.mixer.volume_multiplier_mixer_max = volume_multiplier_mixer_max;
	}
#endif /* USE_VOLUME_AUTO_ADJUST */

	/* look for second-level autofire tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig/system/mameplus/autofire"))
	{
		int delay;

		for (attr = 0; attributes[attr]; attr += 2)
			if (!stricmp(attributes[attr], "delay") && sscanf(attributes[attr + 1], "%d", &delay) == 1)
				curfile.data.mameplus.autofire_delay = delay;
	}

	/* look for second-level clock tag */
	if (!stricmp(curfile.parse.tag, "/mameconfig/system/mameplus/clock"))
	{
		int indx = -1, value;
		for (attr = 0; attributes[attr]; attr += 2)
		{
			if (!stricmp(attributes[attr], "index"))
				sscanf(attributes[attr + 1], "%d", &indx);
			else if (!stricmp(attributes[attr], "value") && indx >= 0 && indx < MAX_CPU && sscanf(attributes[attr + 1], "%d", &value) == 1)
				curfile.data.mameplus.cpu_clocks[indx] = value;
		}
	}
}


static void config_element_end(void *data, const XML_Char *name)
{
	XML_Char *p;

	/* only process if we match the current game */
	if (!curfile.data.ignore_game)
	{
		/* set the defseq sequence */
		if (!stricmp(curfile.parse.tag, "/mameconfig/system/input/port/defseq") && curfile.parse.seq_index != -1)
		{
			if (xml_string_to_seq(curfile.parse.data, &curfile.data.port->defseq[curfile.parse.seq_index]) == 0)
				seq_set_1(&curfile.data.port->defseq[curfile.parse.seq_index], get_default_code(curfile.data.port->type));
			seq_copy(&curfile.data.port->newseq[curfile.parse.seq_index], &curfile.data.port->defseq[curfile.parse.seq_index]);
		}

		/* set the newseq sequence */
		if (!stricmp(curfile.parse.tag, "/mameconfig/system/input/port/newseq") && curfile.parse.seq_index != -1)
			if (xml_string_to_seq(curfile.parse.data, &curfile.data.port->newseq[curfile.parse.seq_index]) == 0)
				seq_set_1(&curfile.data.port->newseq[curfile.parse.seq_index], get_default_code(curfile.data.port->type));
	}

	/* remove the last tag */
	p = strrchr(curfile.parse.tag, '/');
	if (p) *p = 0;
}


static void config_data(void *data, const XML_Char *s, int len)
{
	/* accumulate data and NULL-terminate */
	if (curfile.parse.datalen + len > sizeof(curfile.parse.data) - 1)
		len = sizeof(curfile.parse.data) - 1 - curfile.parse.datalen;
	memcpy(&curfile.parse.data[curfile.parse.datalen], s, len);
	curfile.parse.datalen += len;
	curfile.parse.data[curfile.parse.datalen] = 0;
}



/*************************************
 *
 *  XML file load
 *
 *************************************/

static int config_load_xml(void)
{
	struct config_port *port, *next;
	XML_Parser parser;
	int done, mixernum, remapnum;
	int first = 1;

	/* initialize the defaults */
	for (mixernum = 0; mixernum < MAX_MIXER_CHANNELS; mixernum++)
	{
		curfile.data.mixer.deflevel[mixernum] = 1.0;
		curfile.data.mixer.newlevel[mixernum] = 1.0;
	}
	for (remapnum = 0; remapnum < __code_max; remapnum++)
		curfile.data.remap[remapnum] = remapnum;

#ifdef USE_VOLUME_AUTO_ADJUST
	curfile.data.mixer.volume_multiplier_final = DEFAULT_VOLUME_MULTIPLIER;
	curfile.data.mixer.volume_multiplier_final_max = DEFAULT_VOLUME_MULTIPLIER_MAX;
	curfile.data.mixer.volume_multiplier_mixer = DEFAULT_VOLUME_MULTIPLIER;
	curfile.data.mixer.volume_multiplier_mixer_max = DEFAULT_VOLUME_MULTIPLIER_MAX;
#endif /* USE_VOLUME_AUTO_ADJUST */
	
	/* reset our parse data */
	curfile.parse.tag[0] = 0;
	curfile.parse.seq_index = -1;

	/* create the XML parser */
	parser = XML_ParserCreate(NULL);
	if (!parser)
		goto error;

	/* configure the parser */
	XML_SetElementHandler(parser, config_element_start, config_element_end);
	XML_SetCharacterDataHandler(parser, config_data);

	/* loop through the file and parse it */
	do
	{
		char tempbuf[TEMP_BUFFER_SIZE];

		/* read as much as we can */
		int bytes = mame_fread(curfile.file, tempbuf, sizeof(tempbuf));
		done = mame_feof(curfile.file);

		/* if this is the first read, make sure we aren't sucking up an old binary file */
		if (first && !memcmp(tempbuf, "MAMECFG", 7))
			goto error;
		first = 0;

		/* parse the data */
		if (XML_Parse(parser, tempbuf, bytes, done) == XML_STATUS_ERROR)
			goto error;

	} while (!done);

	/* error if this isn't a valid game match */
	if (curfile.data.loaded_count == 0)
		goto error;

	/* reverse the port list */
	port = curfile.data.port;
	curfile.data.port = NULL;
	for ( ; port != NULL; port = next)
	{
		next = port->next;
		port->next = curfile.data.port;
		curfile.data.port = port;
	}

	/* free the parser */
	XML_ParserFree(parser);
	return 1;

error:
	if (parser)
		XML_ParserFree(parser);
	return 0;
}



/*************************************
 *
 *  XML file save
 *
 *************************************/

static void config_print_seq(int is_default, int type, int porttype, const input_seq_t *seq)
{
	char seqbuffer[256];

	/* skip if empty */
	if (is_default && seq_get_1(seq) == get_default_code(porttype))
		return;

	/* write the outer tag */
	mame_fprintf(curfile.file, "\t\t\t\t<%s type=\"%s\">", is_default ? "defseq" : "newseq", seqtypestrings[type]);
	if (seq_get_1(seq) == CODE_NONE)
		strcpy(seqbuffer, "NONE");
	else
		xml_seq_to_string(seq, seqbuffer, sizeof(seqbuffer));
	mame_fprintf(curfile.file, "%s", seqbuffer);
	mame_fprintf(curfile.file, "</%s>\n", is_default ? "defseq" : "newseq");
}


static int config_save_xml(void)
{
	struct config_port *port;
	int i, doit;

	/* start with the version */
	mame_fprintf(curfile.file, "<mameconfig version=\"%d\">\n", CONFIG_VERSION);

	/* print out the system */
	mame_fprintf(curfile.file, "\t<system name=\"%s\">\n\n", (curfile.filetype == FILE_TYPE_DEFAULT) ? "default" : Machine->gamedrv->name);

	/* first write the input section */
	mame_fprintf(curfile.file, "\t\t<input>\n");
	for (port = curfile.data.port; port != NULL; port = port->next)
	{
		int is_analog = port_type_is_analog(port->type);

		/* write the port information and attributes */
		mame_fprintf(curfile.file, "\t\t\t<port type=\"%s\"", port_type_to_token(port->type, port->player));
		if (curfile.filetype != FILE_TYPE_DEFAULT)
		{
			mame_fprintf(curfile.file, " mask=\"%d\" defvalue=\"%d\" value=\"%d\"", port->mask, port->defvalue & port->mask, port->value & port->mask);
			if (is_analog)
				mame_fprintf(curfile.file, " keydelta=\"%d\" centerdelta=\"%d\" sensitivity=\"%d\" reverse=\"%s\"", port->keydelta, port->centerdelta, port->sensitivity, port->reverse ? "yes" : "no");
			else
			{
				if (port->autofire == 1)
					mame_fprintf(curfile.file, " autofire=\"on\"");
				else if (port->autofire == 2)
					mame_fprintf(curfile.file, " autofire=\"toggle\"");
#ifdef USE_CUSTOM_BUTTON
				if (port->type >= IPT_CUSTOM1 && port->type < IPT_CUSTOM1 + MAX_CUSTOM_BUTTONS)

					mame_fprintf(curfile.file, " custom=\"%d\"", port->custom);
#endif /* USE_CUSTOM_BUTTON */

			}
		}
		mame_fprintf(curfile.file, ">\n");

		/* write the default and current sequences */
		if (!is_analog)
		{
			config_print_seq(1, 0, port->type, &port->defseq[0]);
			if (seq_cmp(&port->defseq[0], &port->newseq[0]) != 0)
				config_print_seq(0, 0, port->type, &port->newseq[0]);
		}
		else
		{
			config_print_seq(1, 0, port->type, &port->defseq[0]);
			config_print_seq(1, 1, port->type, &port->defseq[1]);
			config_print_seq(1, 2, port->type, &port->defseq[2]);
			if (seq_cmp(&port->defseq[0], &port->newseq[0]) != 0)
				config_print_seq(0, 0, port->type, &port->newseq[0]);
			if (seq_cmp(&port->defseq[1], &port->newseq[1]) != 0)
				config_print_seq(0, 1, port->type, &port->newseq[1]);
			if (seq_cmp(&port->defseq[2], &port->newseq[2]) != 0)
				config_print_seq(0, 2, port->type, &port->newseq[2]);
		}

		/* close the tag */
		mame_fprintf(curfile.file, "\t\t\t</port>\n");
	}
	mame_fprintf(curfile.file, "\t\t</input>\n\n");

	/* if we are just writing default ports, leave it at that */
	if (curfile.filetype != FILE_TYPE_DEFAULT)
	{
		/* next write the counters section */
		doit = (curfile.data.counters.tickets != 0);
		for (i = 0; i < COIN_COUNTERS; i++)
			if (curfile.data.counters.coins[i] != 0)
				doit = 1;
		if (doit)
		{
			mame_fprintf(curfile.file, "\t\t<counters>\n");
			for (i = 0; i < COIN_COUNTERS; i++)
				if (curfile.data.counters.coins[i] != 0)
					mame_fprintf(curfile.file, "\t\t\t<coins index=\"%d\" number=\"%d\" />\n", i, curfile.data.counters.coins[i]);
			if (curfile.data.counters.tickets != 0)
				mame_fprintf(curfile.file, "\t\t\t<tickets number=\"%d\" />\n", curfile.data.counters.tickets);
			mame_fprintf(curfile.file, "\t\t</counters>\n\n");
		}

		/* finally, write the mixer section */
		doit = 0;
		for (i = 0; i < MAX_MIXER_CHANNELS; i++)
			if (curfile.data.mixer.newlevel[i] != curfile.data.mixer.deflevel[i])
				doit = 1;

#ifdef USE_VOLUME_AUTO_ADJUST
		if (curfile.data.mixer.volume_multiplier_final != DEFAULT_VOLUME_MULTIPLIER)
			doit = 1;
		if (curfile.data.mixer.volume_multiplier_final_max != DEFAULT_VOLUME_MULTIPLIER_MAX)
			doit = 1;
		if (curfile.data.mixer.volume_multiplier_mixer != DEFAULT_VOLUME_MULTIPLIER)
			doit = 1;
		if (curfile.data.mixer.volume_multiplier_mixer_max != DEFAULT_VOLUME_MULTIPLIER_MAX)
			doit = 1;
#endif /* USE_VOLUME_AUTO_ADJUST */

		if (doit)
		{
			mame_fprintf(curfile.file, "\t\t<mixer>\n");
			for (i = 0; i < MAX_MIXER_CHANNELS; i++)
				if (curfile.data.mixer.newlevel[i] != curfile.data.mixer.deflevel[i])
					mame_fprintf(curfile.file, "\t\t\t<channel index=\"%d\" defvol=\"%f\" newvol=\"%f\" />\n", i, curfile.data.mixer.deflevel[i], curfile.data.mixer.newlevel[i]);
#ifdef USE_VOLUME_AUTO_ADJUST
			if (curfile.data.mixer.volume_multiplier_final != DEFAULT_VOLUME_MULTIPLIER)
				mame_fprintf(curfile.file, "\t\t\t<volume multiplier_final=\"%d\" />\n", curfile.data.mixer.volume_multiplier_final);
			if (curfile.data.mixer.volume_multiplier_final_max != DEFAULT_VOLUME_MULTIPLIER_MAX)
				mame_fprintf(curfile.file, "\t\t\t<volume multiplier_final_max=\"%d\" />\n", curfile.data.mixer.volume_multiplier_final_max);
			if (curfile.data.mixer.volume_multiplier_mixer != DEFAULT_VOLUME_MULTIPLIER)
				mame_fprintf(curfile.file, "\t\t\t<volume multiplier_mixer=\"%d\" />\n", curfile.data.mixer.volume_multiplier_mixer);
			if (curfile.data.mixer.volume_multiplier_mixer_max != DEFAULT_VOLUME_MULTIPLIER_MAX)
				mame_fprintf(curfile.file, "\t\t\t<volume multiplier_mixer_max=\"%d\" />\n", curfile.data.mixer.volume_multiplier_mixer_max);
#endif /* USE_VOLUME_AUTO_ADJUST */
			mame_fprintf(curfile.file, "\t\t</mixer>\n\n");
		}

		/* MAME Plus! extension */
		doit = 0;
		if (curfile.data.mameplus.autofire_delay)
			doit = 1;
		for (i = 0; i < MAX_CPU; i++)
			if (curfile.data.mameplus.cpu_clocks[i])
				doit = 1;

		if (doit)
		{
			mame_fprintf(curfile.file, "\t\t<mameplus>\n");
			if (curfile.data.mameplus.autofire_delay)
				mame_fprintf(curfile.file, "\t\t\t<autofire delay=\"%d\" />\n", curfile.data.mameplus.autofire_delay);
			for (i = 0; i < MAX_CPU; i++)
				if (curfile.data.mameplus.cpu_clocks[i])
					mame_fprintf(curfile.file, "\t\t\t<clock index=\"%d\" value=\"%d\" />\n", i, curfile.data.mameplus.cpu_clocks[i]);
			mame_fprintf(curfile.file, "\t\t</mameplus>\n\n");
		}
	}

#ifdef MESS
	mess_config_save_xml(curfile.filetype, curfile.file);
#endif

	mame_fprintf(curfile.file, "\t</system>\n");
	mame_fprintf(curfile.file, "</mameconfig>\n");
	return 1;
}



/*************************************
 *
 *  Identifies which port types are
 *  saved/loaded
 *
 *************************************/

static int save_this_port_type(int type)
{
	switch (type)
	{
		case IPT_UNUSED:
		case IPT_END:
		case IPT_PORT:
		case IPT_DIPSWITCH_SETTING:
		case IPT_CONFIG_SETTING:
		case IPT_CATEGORY_SETTING:
		case IPT_VBLANK:
		case IPT_UNKNOWN:
			return 0;
	}
	return 1;
}



/*************************************
 *
 *  Apply game-specific read port data
 *
 *************************************/

static int apply_ports(const struct InputPort *input_ports_default, struct InputPort *input_ports)
{
	struct config_port *port;
	const struct InputPort *cin;
	struct InputPort *in;

	/* compare the saved default settings against the current ones in the driver */
	for (port = curfile.data.port, cin = input_ports_default; cin->type != IPT_END; cin++)
		if (save_this_port_type(cin->type))
		{
			/* if we don't have any more ports, we're corrupt */
			if (port == NULL)
				return 0;

			/* if we don't match, return a corrupt error */
			if ((port->defvalue & port->mask) != (cin->default_value & cin->mask) ||
				port->mask != cin->mask || port->type != cin->type || port->player != cin->player)
				return 0;

			/* if the default sequence(s) don't match, return a corrupt error */
			if (!port_type_is_analog(port->type))
			{
				if (seq_cmp(&port->defseq[0], &cin->seq) != 0)
					return 0;
			}
			else
			{
				if (seq_cmp(&port->defseq[0], &cin->seq) != 0 ||
					seq_cmp(&port->defseq[1], &cin->analog.decseq) != 0 ||
					seq_cmp(&port->defseq[2], &cin->analog.incseq) != 0)
					return 0;
			}

			/* advance to the next port */
			port = port->next;
		}

	/* apply the current settings */
	for (port = curfile.data.port, in = input_ports; in->type != IPT_END; in++)
		if (save_this_port_type(in->type))
		{
			/* apply the current values */
			in->default_value = port->value;
			in->analog.delta = port->keydelta;
			in->analog.centerdelta = port->centerdelta;
			in->analog.sensitivity = port->sensitivity;
			in->analog.reverse = port->reverse;
			in->autofire_setting = port->autofire;

#ifdef USE_CUSTOM_BUTTON
			if (port->type >= IPT_CUSTOM1 && port->type < IPT_CUSTOM1 + MAX_CUSTOM_BUTTONS)
				custom_button[port->player][port->type - IPT_CUSTOM1] = port->custom;
#endif /* USE_CUSTOM_BUTTON */
			/* copy the sequence(s) */
			seq_copy(&in->seq, &port->newseq[0]);
			if (port_type_is_analog(port->type))
			{
				seq_copy(&in->analog.decseq, &port->newseq[1]);
				seq_copy(&in->analog.incseq, &port->newseq[2]);
			}

			/* advance to the next port */
			port = port->next;
		}

	return 1;
}



/*************************************
 *
 *  Apply default port data
 *
 *************************************/

static int apply_default_ports(const struct InputPortDefinition *input_ports_default_backup, struct InputPortDefinition *input_ports_default)
{
	struct config_port *port;
	int portnum, remapnum, seqnum;

	/* apply any remapping first */
	for (remapnum = 0; remapnum < __code_max; remapnum++)
		if (curfile.data.remap[remapnum] != remapnum)
			for (portnum = 0; input_ports_default[portnum].type != IPT_END; portnum++)
			{
				for (seqnum = 0; seqnum < SEQ_MAX; seqnum++)
					if (input_ports_default[portnum].defaultseq.code[seqnum] == remapnum)
						input_ports_default[portnum].defaultseq.code[seqnum] = curfile.data.remap[remapnum];

				if (port_type_is_analog(input_ports_default[portnum].type))
				{
					for (seqnum = 0; seqnum < SEQ_MAX; seqnum++)
						if (input_ports_default[portnum].defaultdecseq.code[seqnum] == remapnum)
							input_ports_default[portnum].defaultdecseq.code[seqnum] = curfile.data.remap[remapnum];
					for (seqnum = 0; seqnum < SEQ_MAX; seqnum++)
						if (input_ports_default[portnum].defaultincseq.code[seqnum] == remapnum)
							input_ports_default[portnum].defaultincseq.code[seqnum] = curfile.data.remap[remapnum];
				}
			}

	/* loop over the ports */
	for (port = curfile.data.port; port != NULL; port = port->next)
	{
		/* find a matching entry in the defaults */
		for (portnum = 0; input_ports_default[portnum].type != IPT_END; portnum++)
			if (input_ports_default[portnum].type == port->type && input_ports_default[portnum].player == port->player)
			{
				/* load stored settings only if the default hasn't changed or if the backup array is NULL */

				/* non-analog case */
				if (!port_type_is_analog(port->type))
				{
					if (input_ports_default_backup == NULL || seq_cmp(&input_ports_default_backup[portnum].defaultseq, &port->defseq[0]) == 0)
						seq_copy(&input_ports_default[portnum].defaultseq, &port->newseq[0]);
				}

				/* analog case */
				else
				{
					if (input_ports_default_backup == NULL ||
						(seq_cmp(&input_ports_default_backup[portnum].defaultseq, &port->defseq[0]) == 0 &&
						 seq_cmp(&input_ports_default_backup[portnum].defaultdecseq, &port->defseq[1]) == 0 &&
						 seq_cmp(&input_ports_default_backup[portnum].defaultincseq, &port->defseq[2]) == 0))
					{
						seq_copy(&input_ports_default[portnum].defaultseq, &port->newseq[0]);
						seq_copy(&input_ports_default[portnum].defaultdecseq, &port->newseq[1]);
						seq_copy(&input_ports_default[portnum].defaultincseq, &port->newseq[2]);
					}
				}
			}
	}

	return 1;
}



/*************************************
 *
 *  Apply counter/mixer data
 *
 *************************************/

static int apply_counters(void)
{
	int counternum;

	/* copy data from the saved config and reset the lastcount and lockout flags */
	for (counternum = 0; counternum < COIN_COUNTERS; counternum++)
		coin_count[counternum] = curfile.data.counters.coins[counternum];
	dispensed_tickets = curfile.data.counters.tickets;

	return 1;
}


static int apply_mixer(void)
{
	int num_vals = sound_get_user_gain_count();
	int mixernum;

	/* set the mixer gain on all channels */
	for (mixernum = 0; mixernum < num_vals; mixernum++)
		if (curfile.data.mixer.deflevel[mixernum] == sound_get_default_gain(mixernum))
			sound_set_user_gain(mixernum, curfile.data.mixer.newlevel[mixernum]);

#ifdef USE_VOLUME_AUTO_ADJUST
	sound_set_volume_multiplier_final(curfile.data.mixer.volume_multiplier_final);
	sound_set_volume_multiplier_final_max(curfile.data.mixer.volume_multiplier_final_max);
	sound_set_volume_multiplier_mixer(curfile.data.mixer.volume_multiplier_mixer);
	sound_set_volume_multiplier_mixer_max(curfile.data.mixer.volume_multiplier_mixer_max);
#endif /* USE_VOLUME_AUTO_ADJUST */

	return 1;
}


/*************************************
 *
 *	Apply MAME Plus! extension data
 *
 *************************************/

static int apply_mameplus(void)
{
	int i;

	options.autofiredelay = curfile.data.mameplus.autofire_delay;
	if (options.autofiredelay < 1)
		options.autofiredelay = 1;
	if (options.autofiredelay > 99)
		options.autofiredelay = 99;

	/* load overclock setting if cheat is enabled */
	if (options.cheat)
	{
		for (i = 0; i < cpu_gettotalcpu(); i++)
			cpu_clocks[i] = curfile.data.mameplus.cpu_clocks[i];
	}

	return 1;
}



/*************************************
 *
 *  Build game-specific port data
 *
 *************************************/

static int build_ports(const struct InputPort *input_ports_default, const struct InputPort *input_ports)
{
	struct config_port *porthead = NULL, *port, *next;
	int portnum;

	/* loop through the ports and allocate/fill in data structures along the way */
	for (portnum = 0; input_ports_default[portnum].type != IPT_END; portnum++)
		if (save_this_port_type(input_ports_default[portnum].type))
		{
			/* allocate memory for a new port */
			port = malloc(sizeof(*port));
			if (!port)
				goto error;
			memset(port, 0, sizeof(*port));

			/* add this port to our temporary list */
			port->next = porthead;
			porthead = port;

			/* fill in the data */
			port->type        = input_ports_default[portnum].type;
			port->player      = input_ports_default[portnum].player;
			port->mask        = input_ports_default[portnum].mask;
			port->defvalue    = input_ports_default[portnum].default_value;
			port->value       = input_ports[portnum].default_value;
			port->keydelta    = input_ports[portnum].analog.delta;
			port->centerdelta = input_ports[portnum].analog.centerdelta;
			port->sensitivity = input_ports[portnum].analog.sensitivity;
			port->reverse     = input_ports[portnum].analog.reverse;
			port->autofire    = input_ports[portnum].autofire_setting;
#ifdef USE_CUSTOM_BUTTON
			if (port->type >= IPT_CUSTOM1 && port->type < IPT_CUSTOM1 + MAX_CUSTOM_BUTTONS)
				port->custom = custom_button[port->player][port->type - IPT_CUSTOM1];
			else
				port->custom = 0;
#endif /* USE_CUSTOM_BUTTON */

			/* copy the sequences */
			seq_copy(&port->defseq[0], &input_ports_default[portnum].seq);
			seq_copy(&port->newseq[0], &input_ports[portnum].seq);
			if (port_type_is_analog(port->type))
			{
				seq_copy(&port->defseq[1], &input_ports_default[portnum].analog.decseq);
				seq_copy(&port->newseq[1], &input_ports[portnum].analog.decseq);
				seq_copy(&port->defseq[2], &input_ports_default[portnum].analog.incseq);
				seq_copy(&port->newseq[2], &input_ports[portnum].analog.incseq);
			}
		}

	/* now reverse the list and connect it to the config file */
	curfile.data.port = NULL;
	for (port = porthead; port != NULL; port = next)
	{
		next = port->next;
		port->next = curfile.data.port;
		curfile.data.port = port;
	}
	return 1;

error:
	/* free what we have and error */
	cleanup_file();
	return 0;
}



/*************************************
 *
 *  Build default port data
 *
 *************************************/

static int build_default_ports(const struct InputPortDefinition *input_ports_default_backup, const struct InputPortDefinition *input_ports_default)
{
	struct config_port *porthead = NULL, *port, *next;
	int portnum;

	/* loop through the ports and allocate/fill in data structures along the way */
	for (portnum = 0; input_ports_default[portnum].type != IPT_END; portnum++)
		if (save_this_port_type(input_ports_default[portnum].type))
		{
			/* allocate memory for a new port */
			port = malloc(sizeof(*port));
			if (!port)
				goto error;
			memset(port, 0, sizeof(*port));

			/* add this port to our temporary list */
			port->next = porthead;
			porthead = port;

			/* fill in the data */
			port->type        = input_ports_default[portnum].type;
			port->player      = input_ports_default[portnum].player;

			/* copy the sequences */
			seq_copy(&port->defseq[0], &input_ports_default_backup[portnum].defaultseq);
			seq_copy(&port->newseq[0], &input_ports_default[portnum].defaultseq);
			if (port_type_is_analog(port->type))
			{
				seq_copy(&port->defseq[1], &input_ports_default_backup[portnum].defaultdecseq);
				seq_copy(&port->newseq[1], &input_ports_default[portnum].defaultdecseq);
				seq_copy(&port->defseq[2], &input_ports_default_backup[portnum].defaultincseq);
				seq_copy(&port->newseq[2], &input_ports_default[portnum].defaultincseq);
			}
		}

	/* now reverse the list and connect it to the config file */
	curfile.data.port = NULL;
	for (port = porthead; port != NULL; port = next)
	{
		next = port->next;
		port->next = curfile.data.port;
		curfile.data.port = port;
	}
	return 1;

error:
	/* free what we have and error */
	cleanup_file();
	return 0;
}



/*************************************
 *
 *  Build counter/mixer data
 *
 *************************************/

static int build_counters(void)
{
	int counternum;

	/* copy out the coin/ticket counters for this machine */
	for (counternum = 0; counternum < COIN_COUNTERS; counternum++)
		curfile.data.counters.coins[counternum] = coin_count[counternum];
	curfile.data.counters.tickets = dispensed_tickets;
	return 1;
}


static int build_mixer(void)
{
	int num_vals = sound_get_user_gain_count();
	int mixernum;

	/* copy out the mixing levels */
	for (mixernum = 0; mixernum < num_vals; mixernum++)
	{
		if (mixernum < num_vals)
		{
			curfile.data.mixer.deflevel[mixernum] = sound_get_default_gain(mixernum);
			curfile.data.mixer.newlevel[mixernum] = sound_get_user_gain(mixernum);
		}
		else
			curfile.data.mixer.deflevel[mixernum] = curfile.data.mixer.newlevel[mixernum] = 0.0;
	}

#ifdef USE_VOLUME_AUTO_ADJUST
	curfile.data.mixer.volume_multiplier_final = sound_get_volume_multiplier_final();
	curfile.data.mixer.volume_multiplier_final_max = sound_get_volume_multiplier_final_max();
	curfile.data.mixer.volume_multiplier_mixer = sound_get_volume_multiplier_mixer();
	curfile.data.mixer.volume_multiplier_mixer_max = sound_get_volume_multiplier_mixer_max();
#endif /* USE_VOLUME_AUTO_ADJUST */

	return 1;
}


/*************************************
 *
 *	Build MAME Plus! extension data
 *
 *************************************/

static int build_mameplus(void)
{
	int i;

	memset(&curfile.data.mameplus, 0, sizeof curfile.data.mameplus);

	if (options.autofiredelay > 1 && options.autofiredelay <= 99) 
		curfile.data.mameplus.autofire_delay = options.autofiredelay;

	/* store overclock setting if cheat is enabled */
	if (options.cheat)
	{
		for (i = 0; i < cpu_gettotalcpu(); i++)
		{
			INT16 clock = cpunum_get_clockscale(i) * 100.0 + 0.5;

			if (clock == 100)
				clock = 0;

			curfile.data.mameplus.cpu_clocks[i] = clock;
		}
	}

	return 1;
}
