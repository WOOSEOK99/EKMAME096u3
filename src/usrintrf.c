/*********************************************************************

    usrintrf.c

    Functions used to handle MAME's user interface.

*********************************************************************/

#include "driver.h"
#include "info.h"
#include "vidhrdw/vector.h"
#include "datafile.h"
#include "artwork.h"
#include <windows.h>
#include <stdarg.h>
#include <math.h>
#include "ui_text.h"
#include "state.h"
#include "ui_pal.h"
#include "ui_lang.h"
#include "osd_so.h"

#ifdef MESS
#include "mess.h"
#include "mesintrf.h"
#include "inputx.h"
#endif

#ifdef KAILLERA
#include "kailleraclient.h"
#include "ui/kcommand.h"
#include "ui/chat.h"

extern int kPlay;
extern int kPlayers;
extern int kPlayerNB;
extern int p1_pos;
extern int control_counter;
extern int init;
extern int control_type;
extern int netplay_pause;
extern int endemu;
extern int nEndChat;
extern void *record;
extern void *record_k;
extern UINT8 win_trying_to_quit;
extern void write_port_value(mame_file *f, UINT32 value);	// inptport.c
// TODO (DARKCODER) (0.84u5): 아래의 write_port_values함수는 inptport.c에 구현되어 있는
// write_port_value()함수와 동일하다. 그냥 inptport.c의 함수를 그냥쓰면 되는데
// 이상하게 함수구현에 static선언을 삭제하였는데도 링크시에 이 함수를 찾을 수 없다는 
// 메세지가 뜬다. 따라서 응급처방으로 동일하게 write_port_values()함수로 작성하여서 
// 이 함수를 쓰도록 하였다. 추후에 문제점의 원인을 찾게 되면 그냥 inptport.c에
// 구현된 함수를 쓰도록 해야 한다.
void write_port_values(mame_file *f, UINT32 value)
{
	//write_port_value(f, value);
#ifdef LSB_FIRST
	value = (value >> 24) | ((value >> 8) & 0xff00) | ((value & 0xff00) << 8) | (value << 24);
#endif
	mame_fwrite(f, &value, sizeof(value));
}
#endif /* KAILLERA */


/***************************************************************************

    Externals

***************************************************************************/

#ifdef AUTO_PAUSE_PLAYBACK
extern void *playback;
#endif /* AUTO_PAUSE_PLAYBACK */

/* Variables for stat menu */
extern char build_version[];

/* MARTINEZ.F 990207 Memory Card */

struct MEMCARDinterface memcard_intf;
int	mcd_action;
int	mcd_number;
int	memcard_status;
int	memcard_number;
int	memcard_manager;

#if defined(__sgi) && !defined(MESS)
static int game_paused = 0; /* not zero if the game is paused */
#endif
/* MARTINEZ.F 990207 Memory Card End */

#ifdef CMD_LIST
static int command_lastselected;
static char *commandlist_buf = NULL;
static int command_scroll = 0;
static int command_load = 1;
static int command_sc = 0;
static int command_sel = 1;
#endif /* CMD_LIST */

static int cheat_sc = 0;
static int cheat_sel = 1;
static int cheat_fe = 0;

#ifdef INP_CAPTION
static int next_caption_frame, caption_timer;
#endif /* INP_CAPTION */

static int lhw, rhw, uaw, daw, law, raw;
static int scroll_reset;
static int displaymessagewindow_under;

#ifdef CMD_LIST
#ifdef UI_TITLE_MENU
static int display_scroll_message (struct mame_bitmap *bitmap, int *scroll, int width, int height, char *buf, const char *title);
#else
static int display_scroll_message (struct mame_bitmap *bitmap, int *scroll, int width, int height, char *buf);
#endif /* UI_TITLE_MENU */
#else /* CMD_LIST */
static void display_scroll_message (struct mame_bitmap *bitmap, int *scroll, int width, int height, char *buf);
#endif /* CMD_LIST */

#define ROWMARGIN 2
#define ROWHEIGHT       (uirotcharheight + ROWMARGIN)
#define MENUROWHEIGHT   (uirotcharheight + uirotcharheight / 6)

static int wordwrap_text_buffer(char *buffer, int maxwidth);
static int count_lines_in_buffer (char *buffer);

#ifdef UI_COLOR_DISPLAY
#define DRAWBOX_MARGIN 3
#endif /* UI_COLOR_DISPLAY */

#ifdef TRANS_UI
static int draw_transparent_box = 1;
#endif /* TRANS_UI */

#ifdef UI_TITLE_MENU
#define DRAW_TITLEBAR(x, y, len, w) \
	int max_margin = 0;                                                                                  \
	ui_plotbox(bitmap, x, y + ROWHEIGHT - 1, (len) * uirotcharwidth - w, 1, SYSTEM_COLOR_FRAMEMEDIUM);   \
	ui_plotbox(bitmap, x, y + ROWHEIGHT,     (len) * uirotcharwidth - w, 1, SYSTEM_COLOR_FRAMEDARK);     \
	for (i = 0; i < GRADATION_LEVEL; i++)                                                                \
	{                                                                                                    \
		if (i < GRADATION_LEVEL - 1)                                                                     \
			max_margin = (int)(((len) * uirotcharwidth - w) / GRADATION_LEVEL) + 1;                      \
		else                                                                                             \
			max_margin = (len) * uirotcharwidth - w - (int)(i*((len)*uirotcharwidth-w)/GRADATION_LEVEL); \
																										 \
		ui_plotbox(bitmap, x + (int)(i * ((len) * uirotcharwidth - w) / GRADATION_LEVEL), y - ROWMARGIN, \
				   max_margin, ROWHEIGHT + 1, GRADATION_COLOR + i);                                      \
	}
#endif /* UI_TITLE_MENU */

#ifdef INPUT_LOG
extern int show_input;
extern UINT8 inputlog_buf[COMMAND_LOG_BUFSIZE];
extern int inputlog_counter;
#endif /* INPUT_LOG */

#ifdef FRAMES_COUNTER
int total_frame_counter = 0;
int Total_FT_Position;
#endif /*FRAMES_COUNTER*/

#ifdef LOG_WAV
int wave_loging = 0;
int wave_seconds = 0;
int wave_minutes = 0;
#endif /*LOG_WAV*/

/***************************************************************************

    Local variables

***************************************************************************/

/* raw coordinates, relative to the real scrbitmap */
static struct rectangle uirawbounds;
static int uirawcharwidth, uirawcharheight;

/* rotated coordinates, easier to deal with */
static struct rectangle uirotbounds;
static int uirotwidth, uirotheight;
int uirotcharwidth, uirotcharheight;

static int setup_selected;
static int osd_selected;
static int jukebox_selected;
static int single_step;
static int auto_pause;

static int showfps;
static int showfpstemp;

static int show_profiler;

UINT8 ui_dirty;



#if 0
#pragma mark UTILITIES
#endif

/*-------------------------------------------------
    ui_markdirty - mark a raw rectangle dirty
-------------------------------------------------*/

INLINE void ui_markdirty(const struct rectangle *rect)
{
	artwork_mark_ui_dirty(rect->min_x, rect->min_y, rect->max_x, rect->max_y);
	ui_dirty = 5;
}



/*-------------------------------------------------
    ui_raw2rot_rect - convert a rect from raw
    coordinates to rotated coordinates
-------------------------------------------------*/

static void ui_raw2rot_rect(struct rectangle *rect)
{
	int temp, w, h;

	/* get the effective screen size, including artwork */
	artwork_get_screensize(&w, &h);

	/* apply X flip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_X)
	{
		temp = w - rect->min_x - 1;
		rect->min_x = w - rect->max_x - 1;
		rect->max_x = temp;
	}

	/* apply Y flip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_Y)
	{
		temp = h - rect->min_y - 1;
		rect->min_y = h - rect->max_y - 1;
		rect->max_y = temp;
	}

	/* apply X/Y swap first */
	if (Machine->ui_orientation & ORIENTATION_SWAP_XY)
	{
		temp = rect->min_x; rect->min_x = rect->min_y; rect->min_y = temp;
		temp = rect->max_x; rect->max_x = rect->max_y; rect->max_y = temp;
	}
}



/*-------------------------------------------------
    ui_rot2raw_rect - convert a rect from rotated
    coordinates to raw coordinates
-------------------------------------------------*/

static void ui_rot2raw_rect(struct rectangle *rect)
{
	int temp, w, h;

	/* get the effective screen size, including artwork */
	artwork_get_screensize(&w, &h);

	/* apply X/Y swap first */
	if (Machine->ui_orientation & ORIENTATION_SWAP_XY)
	{
		temp = rect->min_x; rect->min_x = rect->min_y; rect->min_y = temp;
		temp = rect->max_x; rect->max_x = rect->max_y; rect->max_y = temp;
	}

	/* apply X flip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_X)
	{
		temp = w - rect->min_x - 1;
		rect->min_x = w - rect->max_x - 1;
		rect->max_x = temp;
	}

	/* apply Y flip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_Y)
	{
		temp = h - rect->min_y - 1;
		rect->min_y = h - rect->max_y - 1;
		rect->max_y = temp;
	}
}



/*-------------------------------------------------
    set_ui_visarea - called by the OSD code to
    set the UI's domain
-------------------------------------------------*/

void set_ui_visarea(int xmin, int ymin, int xmax, int ymax)
{
	/* fill in the rect */
	uirawbounds.min_x = xmin;
	uirawbounds.min_y = ymin;
	uirawbounds.max_x = xmax;
	uirawbounds.max_y = ymax;

	/* orient it */
	uirotbounds = uirawbounds;
	ui_raw2rot_rect(&uirotbounds);

	/* make some easier-to-access globals */
	uirotwidth = uirotbounds.max_x - uirotbounds.min_x + 1;
	uirotheight = uirotbounds.max_y - uirotbounds.min_y + 1;

	/* remove me */
	Machine->uiwidth = uirotbounds.max_x - uirotbounds.min_x + 1;
	Machine->uiheight = uirotbounds.max_y - uirotbounds.min_y + 1;
	Machine->uixmin = uirotbounds.min_x;
	Machine->uiymin = uirotbounds.min_y;

	/* rebuild the font */
	uifont_buildfont();

	if (Machine->ui_orientation & ORIENTATION_SWAP_XY)
	{
		uirotcharwidth = uirawcharheight = Machine->uifontwidth;
		uirotcharheight = uirawcharwidth = Machine->uifontheight;
	}
	else
	{
		uirotcharwidth = uirawcharwidth = Machine->uifontwidth;
		uirotcharheight = uirawcharheight = Machine->uifontheight;
	}
}



/*-------------------------------------------------
    erase_screen - erase the screen
-------------------------------------------------*/

static void erase_screen(struct mame_bitmap *bitmap)
{
	fillbitmap(bitmap, get_black_pen(), NULL);
	schedule_full_refresh();
}


#ifdef UI_TITLE_MENU
/*-------------------------------------------------
	ui_plotbox - draw a color box
-------------------------------------------------*/

void ui_plotbox(struct mame_bitmap *bitmap, int leftx, int topy, int width, int height, int color)
{
	struct rectangle bounds;

	/* make a rect and clip it */
	bounds.min_x = uirotbounds.min_x + leftx;
	bounds.min_y = uirotbounds.min_y + topy;
	bounds.max_x = bounds.min_x + width - 1;
	bounds.max_y = bounds.min_y + height - 1;
	sect_rect(&bounds, &uirotbounds);

	ui_rot2raw_rect(&bounds);

	fillbitmap(bitmap, Machine->uifont->colortable[color], &bounds);
}
#endif /* UI_TITLE_MENU */
#ifdef INPUT_LOG
/*-------------------------------------------------
	ui_transbox - draw a trans box
-------------------------------------------------*/

static void ui_transbox(struct mame_bitmap *bitmap, int leftx, int topy, int width, int height)
{
	struct rectangle bounds;

	/* make a rect and clip it */
	bounds.min_x = uirotbounds.min_x + leftx;
	bounds.min_y = uirotbounds.min_y + topy;
	bounds.max_x = bounds.min_x + width - 1;
	bounds.max_y = bounds.min_y + height - 1;
	sect_rect(&bounds, &uirotbounds);

	ui_rot2raw_rect(&bounds);
	
	fillbitmap_ex(bitmap, MAKE_RGB(16,32,32), &bounds);
}
#endif /* INPUT_LOG */

#if 0
#pragma mark -
#pragma mark FONTS & TEXT
#endif

#if 0
/*-------------------------------------------------
    builduifont - build the user interface fonts
-------------------------------------------------*/
#define MAX_UIFONT_SIZE 8 /* max(width,height) */
struct GfxElement *builduifont(void)
{
	struct GfxLayout layout = uifontlayout;
	struct GfxElement *font;
	int temp, i;

	/* free any existing fonts */
	if (Machine->uifont)
		freegfx(Machine->uifont);
	if (Machine->uirotfont)
		freegfx(Machine->uirotfont);

	/* first decode a straight on version for games */
	Machine->uifont = font = decodegfx(uifontdata, &layout);
	Machine->uifontwidth = layout.width;
	Machine->uifontheight = layout.height;

	/* pixel double horizontally */
	if (uirotwidth >= 420)
	{
		for (i = 0; i < layout.width; i++)
			layout.xoffset[i*2+0] = layout.xoffset[i*2+1] = uifontlayout.xoffset[i];
		layout.width *= 2;
	}

	/* pixel double vertically */
	if (uirotheight >= 420)
	{
		for (i = 0; i < layout.height; i++)
			layout.yoffset[i*2+0] = layout.yoffset[i*2+1] = uifontlayout.yoffset[i];
		layout.height *= 2;
	}

	/* apply swappage */
	if (Machine->ui_orientation & ORIENTATION_SWAP_XY)
	{
		for (i=0; i<2*MAX_UIFONT_SIZE; i++)
			temp = layout.xoffset[i], layout.xoffset[i] = layout.yoffset[i], layout.yoffset[i] = temp;

		temp = layout.width;
		layout.width = layout.height;
		layout.height = temp;
	}

	/* apply xflip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_X)
	{
		for (i = 0; i < layout.width/2; i++)
			temp = layout.xoffset[i], layout.xoffset[i] = layout.xoffset[layout.width - 1 - i], layout.xoffset[layout.width - 1 - i] = temp;
	}

	/* apply yflip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_Y)
	{
		for (i = 0; i < layout.height/2; i++)
			temp = layout.yoffset[i], layout.yoffset[i] = layout.yoffset[layout.height - 1 - i], layout.yoffset[layout.height - 1 - i] = temp;
	}

	/* decode rotated font */
	Machine->uirotfont = decodegfx(uifontdata, &layout);

	/* set the raw and rotated character width/height */
	uirawcharwidth = layout.width;
	uirawcharheight = layout.height;
	uirotcharwidth = (Machine->ui_orientation & ORIENTATION_SWAP_XY) ? layout.height : layout.width;
	uirotcharheight = (Machine->ui_orientation & ORIENTATION_SWAP_XY) ? layout.width : layout.height;

	/* set up the bogus colortable */
	if (font)
	{
		static pen_t colortable[2*2];

		/* colortable will be set at run time */
		font->colortable = colortable;
		font->total_colors = 2;
		Machine->uirotfont->colortable = colortable;
		Machine->uirotfont->total_colors = 2;
	}

	return font;
}



/*-------------------------------------------------
    ui_drawchar - draw a rotated character
-------------------------------------------------*/

void ui_drawchar(struct mame_bitmap *dest, int ch, int color, int sx, int sy)
{
	struct rectangle bounds;

#ifdef MESS
	extern int skip_this_frame;
	skip_this_frame = 0;
#endif /* MESS */

	/* construct a rectangle in rotated coordinates, then transform it */
	bounds.min_x = sx + uirotbounds.min_x;
	bounds.min_y = sy + uirotbounds.min_y;
	bounds.max_x = bounds.min_x + uirotcharwidth - 1;
	bounds.max_y = bounds.min_y + uirotcharheight - 1;
	ui_rot2raw_rect(&bounds);

	/* now render */
	drawgfx(dest, Machine->uirotfont, ch, color, 0, 0, bounds.min_x, bounds.min_y, &uirawbounds, TRANSPARENCY_NONE, 0);

	/* mark dirty */
	ui_markdirty(&bounds);
}
#endif



/*-------------------------------------------------
    ui_text_ex - draw a string to the screen
-------------------------------------------------*/

static void ui_text_ex(struct mame_bitmap *bitmap, const char *buf_begin, const char *buf_end, int x, int y, int color)
{
	struct rectangle bounds;
	int len;
	char buf[256];

	memset(buf, 0, 256);

	len = (buf_end - buf_begin) + 1;
	if (*buf_end == ' ')
		len--;

	memcpy(buf, buf_begin, len);

	bounds.min_x = uirotbounds.min_x + x;
	bounds.min_y = uirotbounds.min_y + x;
	bounds.max_x = bounds.min_x;
	bounds.max_y = bounds.min_y;
	ui_rot2raw_rect(&bounds);

	uifont_drawfont(bitmap, buf, x, y, color);
}



/*-------------------------------------------------
	ui_text - draw a string to the screen
-------------------------------------------------*/

#ifdef UI_COLOR_DISPLAY
void ui_text(struct mame_bitmap *bitmap, const char *buf, int x, int y)
{
	struct rectangle bounds;
	unsigned char *c = (unsigned char *)buf;
	int len = 0;

	while (*c)
	{
		int increment;
		unsigned short code;

		increment = uifont_decodechar((unsigned char *)c, &code);
		len += (code < 256) ? 1 : 2;
		c += increment;
	}

	bounds.min_x = uirotbounds.min_x + x;
	bounds.min_y = uirotbounds.min_y + y;
	bounds.max_x = bounds.min_x + uirotcharwidth * len;
	bounds.max_y = bounds.min_y + uirotcharheight;
	ui_rot2raw_rect(&bounds);

#ifdef TRANS_UI
	fillbitmap_ts(bitmap, Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND], &bounds);
#else /* TRANS_UI */
	fillbitmap(bitmap, Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND], &bounds);
#endif /* TRANS_UI */

	ui_text_ex(bitmap, buf, buf + strlen(buf), x, y, UI_COLOR_NORMAL);
}

#else /* UI_COLOR_DISPLAY */
void ui_text(struct mame_bitmap *bitmap,const char *buf,int x,int y)
{
	ui_text_ex(bitmap, buf, buf + strlen(buf), x, y, UI_COLOR_NORMAL);
}
#endif /* UI_COLOR_DISPLAY */

void ui_text_nobk(struct mame_bitmap *bitmap, const char *buf, int x, int y)
{
	ui_text_ex(bitmap, buf, buf + strlen(buf), x, y, UI_COLOR_NORMAL);
}

#ifdef MAME_AVI
void ui_text2(struct mame_bitmap *bitmap,const char *buf,int x,int y, int color)
{
	ui_text_ex(bitmap, buf, buf + strlen(buf), x, y, color);
}
#endif

/*-------------------------------------------------
    displaytext - display a series of text lines
-------------------------------------------------*/

void displaytext(struct mame_bitmap *bitmap, const struct DisplayText *dt)
{
#if 1
   /* loop until we run out of descriptors */
   for ( ; dt->text; dt++)
   {
      ui_text_ex(bitmap, dt->text, dt->text + strlen(dt->text), dt->x, dt->y, dt->color);
   }
#else
	/* loop until we run out of descriptors */
	for ( ; dt->text; dt++)
#ifdef TRANS_UI
		ui_text_nobk(bitmap, dt->text, dt->x, dt->y);
#else /* TRANS_UI */
#ifdef UI_COLOR_DISPLAY
		ui_text_nobk(bitmap, dt->text, dt->x, dt->y);
#else /* UI_COLOR_DISPLAY */
		ui_text(bitmap, dt->text, dt->x, dt->y);
#endif /* UI_COLOR_DISPLAY */
#endif /* TRANS_UI */
#endif
}



/*-------------------------------------------------
    multiline_extract - extract one line from a
    multiline buffer; return the number of
    characters in the line; pbegin points to the
    start of the next line
-------------------------------------------------*/

static unsigned multiline_extract(const char **pbegin, const char *end, unsigned maxchars)
{
	const char *begin = *pbegin;
	unsigned numchars = 0;

	/* loop until we hit the end or max out */
	while (begin != end && numchars < maxchars)
	{
		/* if we hit an EOL, strip it and return the current count */
		if (*begin == '\n')
		{
			*pbegin = begin + 1; /* strip final space */
			return numchars;
		}

		/* if we hit a space, word wrap */
		else if (*begin == ' ')
		{
			/* find the end of this word */
			const char* word_end = begin + 1;
			while (word_end != end && *word_end != ' ' && *word_end != '\n')
				++word_end;

			/* if that pushes us past the max, truncate here */
			if (numchars + word_end - begin > maxchars)
			{
				/* if we have at least one character, strip the space */
				if (numchars)
				{
					*pbegin = begin + 1;
					return numchars;
				}

				/* otherwise, take as much as we can */
				else
				{
					*pbegin = begin + maxchars;
					return maxchars;
				}
			}

			/* advance to the end of this word */
			numchars += word_end - begin;
			begin = word_end;
		}

		/* for all other chars, just increment */
		else
		{
			++numchars;
			++begin;
		}
	}

	/* make sure we always make forward progress */
	if (begin != end && (*begin == '\n' || *begin == ' '))
		++begin;
	*pbegin = begin;
	return numchars;
}



/*-------------------------------------------------
    multiline_size - compute the output size of a
    multiline string
-------------------------------------------------*/

static void multiline_size(int *dx, int *dy, const char *begin, const char *end, unsigned maxchars)
{
	unsigned rows = 0;
	unsigned cols = 0;

	/* extract lines until the end, counting rows and tracking the max columns */
	while (begin != end)
	{
		unsigned len;
		len = multiline_extract(&begin, end, maxchars);
		if (len > cols)
			cols = len;
		++rows;
	}

	/* return the final result scaled by the char size */
	*dx = cols * uirotcharwidth;
	*dy = (rows - 1) * ROWHEIGHT + uirotcharheight;
}



/*-------------------------------------------------
    multilinebox_size - compute the output size of
    a multiline string with box
-------------------------------------------------*/

static void multilinebox_size(int *dx, int *dy, const char *begin, const char *end, unsigned maxchars)
{
	/* standard computation, plus an extra char width and height */
	multiline_size(dx, dy, begin, end, maxchars);
	*dx += uirotcharwidth;
	*dy += uirotcharheight;
}



/*-------------------------------------------------
    ui_multitext_ex - display a multiline string
-------------------------------------------------*/

static void ui_multitext_ex(struct mame_bitmap *bitmap, const char *begin, const char *end, unsigned maxchars, int x, int y, int color)
{
	/* extract lines until the end */
	while (begin != end)
	{
		const char *line_begin = begin;
		unsigned len = multiline_extract(&begin, end, maxchars);
		ui_text_ex(bitmap, line_begin, line_begin + len, x, y, color);
		y += ROWHEIGHT;
	}
}



/*-------------------------------------------------
    ui_multitextbox_ex - display a multiline
    string with box
-------------------------------------------------*/

static void ui_multitextbox_ex(struct mame_bitmap *bitmap, const char *begin, const char *end, unsigned maxchars, int x, int y, int dx, int dy, int color)
{
	/* draw the box first */
	ui_drawbox(bitmap, x, y, dx, dy);

	/* indent by half a character */
	x += uirotcharwidth/2;
	y += uirotcharheight/2;

	/* draw the text */
	ui_multitext_ex(bitmap, begin, end, maxchars, x, y, color);
}



/*-------------------------------------------------
    ui_drawbox - draw a black box with white border
-------------------------------------------------*/

void ui_drawbox(struct mame_bitmap *bitmap, int leftx, int topy, int width, int height)
{
	struct rectangle bounds, tbounds;

#ifdef UI_COLOR_DISPLAY
#define DRAWHORIZ(loffs, roffs, y, c) \
	tbounds.min_x = bounds.min_x + loffs; \
	tbounds.max_x = bounds.max_x - roffs; \
	tbounds.min_y = tbounds.max_y = y; \
	ui_rot2raw_rect(&tbounds); \
	fillbitmap(bitmap, c, &tbounds)

#define DRAWVERT(toffs, boffs, x, c) \
	tbounds.min_y = bounds.min_y + toffs; \
	tbounds.max_y = bounds.max_y - boffs; \
	tbounds.min_x = tbounds.max_x = x; \
	ui_rot2raw_rect(&tbounds); \
	fillbitmap(bitmap, c, &tbounds)

	leftx  -= DRAWBOX_MARGIN;
	topy   -= DRAWBOX_MARGIN;
	width  += DRAWBOX_MARGIN * 2;
	height += DRAWBOX_MARGIN * 2;

	/* make a rect and clip it */
	bounds.min_x = uirotbounds.min_x + leftx;
	bounds.min_y = uirotbounds.min_y + topy;
	bounds.max_x = bounds.min_x + width - 1;
	bounds.max_y = bounds.min_y + height - 1;
	sect_rect(&bounds, &uirotbounds);

	/* top edge */
	DRAWHORIZ(0, 0, bounds.min_y,   Machine->uifont->colortable[SYSTEM_COLOR_FRAMELIGHT]);
	DRAWHORIZ(1, 1, bounds.min_y+1, Machine->uifont->colortable[SYSTEM_COLOR_FRAMEMEDIUM]);
	DRAWHORIZ(2, 2, bounds.min_y+2, Machine->uifont->colortable[SYSTEM_COLOR_FRAMEDARK]);

	/* bottom edge */
	DRAWHORIZ(3, 2, bounds.max_y-2, Machine->uifont->colortable[SYSTEM_COLOR_FRAMELIGHT]);
	DRAWHORIZ(1, 1, bounds.max_y-1, Machine->uifont->colortable[SYSTEM_COLOR_FRAMEMEDIUM]);
	DRAWHORIZ(0, 0, bounds.max_y,   Machine->uifont->colortable[SYSTEM_COLOR_FRAMEDARK]);

	/* left edge */
	DRAWVERT(1, 1, bounds.min_x,   Machine->uifont->colortable[SYSTEM_COLOR_FRAMELIGHT]);
	DRAWVERT(2, 2, bounds.min_x+1, Machine->uifont->colortable[SYSTEM_COLOR_FRAMEMEDIUM]);
	DRAWVERT(3, 2, bounds.min_x+2, Machine->uifont->colortable[SYSTEM_COLOR_FRAMEDARK]);

	/* right edge */
	DRAWVERT(3, 3, bounds.max_x-2, Machine->uifont->colortable[SYSTEM_COLOR_FRAMELIGHT]);
	DRAWVERT(2, 2, bounds.max_x-1, Machine->uifont->colortable[SYSTEM_COLOR_FRAMEMEDIUM]);
	DRAWVERT(1, 1, bounds.max_x,   Machine->uifont->colortable[SYSTEM_COLOR_FRAMEDARK]);

	/* fill in the middle with black */
	tbounds = bounds;
	tbounds.min_x += DRAWBOX_MARGIN;
	tbounds.min_y += DRAWBOX_MARGIN;
	tbounds.max_x -= DRAWBOX_MARGIN;
	tbounds.max_y -= DRAWBOX_MARGIN;
	ui_rot2raw_rect(&tbounds);
#ifdef TRANS_UI
#ifdef MENU_TRANS
   fillbitmap_ex(bitmap, options.menu_color, &tbounds);
#else
	if (draw_transparent_box)
		fillbitmap_ts(bitmap, Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND], &tbounds);
	else
#endif /*MENU_TRANS*/	
#endif /* TRANS_UI */
#ifndef MENU_TRANS
	fillbitmap(bitmap, Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND], &tbounds);
#endif /*MENU_TRANS*/

#else /* UI_COLOR_DISPLAY */
	pen_t black, white;

	/* make a rect and clip it */
	bounds.min_x = uirotbounds.min_x + leftx;
	bounds.min_y = uirotbounds.min_y + topy;
	bounds.max_x = bounds.min_x + width - 1;
	bounds.max_y = bounds.min_y + height - 1;
	sect_rect(&bounds, &uirotbounds);

	/* pick colors from the colortable */
	black = Machine->uirotfont->colortable[0];
#ifdef UI_COLOR_DISPLAY
	white  = Machine->uirotfont->colortable[FONT_COLOR_WHITE];
#else
	white  = Machine->uirotfont->colortable[1];
#endif /* UI_COLOR_DISPLAY */

#ifdef UI_COLOR_DISPLAY
	light  = Machine->uirotfont->colortable[OSDBAR_COLOR_FRAMELIGHT];
	medium = Machine->uirotfont->colortable[OSDBAR_COLOR_FRAMEMEDIUM];
	dark   = Machine->uirotfont->colortable[OSDBAR_COLOR_FRAMEDARK];

	/* ----------- outer border ---------- */
	/* top edge */
	tbounds = bounds; 
	tbounds.min_x++;tbounds.max_x--; 
	tbounds.max_y = tbounds.min_y; 
	ui_rot2raw_rect(&tbounds); 
	fillbitmap(bitmap, light, &tbounds);

	/* bottom edge */
	tbounds = bounds; 
	tbounds.min_x++;tbounds.max_x--; 
	tbounds.min_y = tbounds.max_y; 
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, dark, &tbounds);

	/* left edge */
	tbounds = bounds;
	tbounds.min_y++;tbounds.max_y--; 
	tbounds.max_x = tbounds.min_x;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, light, &tbounds);

	/* right edge */
	tbounds = bounds;
	tbounds.min_y++;tbounds.max_y--; 
	tbounds.min_x = tbounds.max_x;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, dark, &tbounds);

	/* ----------- inner border ---------- */
	bounds.min_x++;
	bounds.min_y++;
	bounds.max_x--;
	bounds.max_y--;
	
	/* top edge */
	tbounds = bounds; 
	tbounds.max_y = tbounds.min_y; 
	ui_rot2raw_rect(&tbounds); 
	fillbitmap(bitmap, medium, &tbounds);

	/* bottom edge */
	tbounds = bounds; 
	tbounds.min_y = tbounds.max_y; 
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, medium, &tbounds);

	/* left edge */
	tbounds = bounds;
	tbounds.max_x = tbounds.min_x;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, medium, &tbounds);

	/* right edge */
	tbounds = bounds;
	tbounds.min_x = tbounds.max_x;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, medium, &tbounds);
	/* ------------------------------------- */
#else
	/* top edge */
	tbounds = bounds;
	tbounds.max_y = tbounds.min_y;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);

	/* bottom edge */
	tbounds = bounds;
	tbounds.min_y = tbounds.max_y;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);

	/* left edge */
	tbounds = bounds;
	tbounds.max_x = tbounds.min_x;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);

	/* right edge */
	tbounds = bounds;
	tbounds.min_x = tbounds.max_x;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);
#endif /* UI_COLOR_DISPLAY */

	/* fill in the middle with black */
	tbounds = bounds;
	tbounds.min_x++;
	tbounds.min_y++;
	tbounds.max_x--;
	tbounds.max_y--;
	ui_rot2raw_rect(&tbounds);
#ifdef TRANS_UI
#ifdef MENU_TRANS
  fillbitmap_ex(bitmap, options.menu_color, &tbounds);
#else
	fillbitmap_ts(bitmap, black, &tbounds);
#endif /*MENU_TRANS*/	
#else /* TRANS_UI */
	fillbitmap(bitmap, black, &tbounds);
#endif /* TRANS_UI */
#endif /* UI_COLOR_DISPLAY */

	/* mark things dirty */
	ui_rot2raw_rect(&bounds);
	ui_markdirty(&bounds);
}



/*-------------------------------------------------
    drawbar - draw a thermometer bar
-------------------------------------------------*/

static void drawbar(struct mame_bitmap *bitmap, int leftx, int topy, int width, int height, int percentage, int default_percentage)
{
	struct rectangle bounds, tbounds;
	UINT32 black, white;

	/* make a rect and orient/clip it */
	bounds.min_x = uirotbounds.min_x + leftx;
	bounds.min_y = uirotbounds.min_y + topy;
	bounds.max_x = bounds.min_x + width - 1;
	bounds.max_y = bounds.min_y + height - 1;
	sect_rect(&bounds, &uirotbounds);

	/* pick colors from the colortable */
	black = Machine->uirotfont->colortable[0];
	white = Machine->uirotfont->colortable[1];

	/* make a rect and orient/clip it */
	bounds.min_x = uirotbounds.min_x + leftx;
	bounds.min_y = uirotbounds.min_y + topy;
	bounds.max_x = bounds.min_x + width - 1;
	bounds.max_y = bounds.min_y + height - 1;
	sect_rect(&bounds, &uirotbounds);

	/* pick colors from the colortable */
	black = Machine->uirotfont->colortable[0];
	white = Machine->uirotfont->colortable[1];

#ifdef UI_COLOR_DISPLAY
	/* draw the top line of the bar */
	tbounds = bounds;
	tbounds.min_y += height / 8;
	tbounds.max_y = tbounds.min_y;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);

	/* draw the bottom line of the bar */
	tbounds = bounds;
	tbounds.max_y -= height / 8;
	tbounds.min_y = tbounds.max_y;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);

	/* top of the bar */
	DRAWHORIZ(0, (width - 1) * (100 - percentage) / 100 - 1,
	          bounds.min_y + height/8 + 1, Machine->uifont->colortable[OSDBAR_COLOR_FRAMEDARK]);

	/* left of the bar */
	DRAWVERT(height/8 + 2, height/8 - 1,
	         bounds.min_x, Machine->uifont->colortable[OSDBAR_COLOR_FRAMEDARK]);

	/* right of the bar */
	DRAWVERT(height/8 + 2, height/8 -1,
	         bounds.max_x - (width - 1) * (100 - percentage) / 100 - 1, Machine->uifont->colortable[OSDBAR_COLOR_FRAMELIGHT]);

	/* body of the bar */
	tbounds = bounds;
	tbounds.min_x = bounds.min_x + 1;
	tbounds.min_y = bounds.min_y + height/8 + 2;
	tbounds.max_x = bounds.max_x - (width - 1) * (100 - percentage) / 100 - 2;
	tbounds.max_y = bounds.max_y - height/8 -1;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, Machine->uifont->colortable[OSDBAR_COLOR_FRAMEMEDIUM], &tbounds);

	/* draw the top default percentage marker */
	DRAWVERT(0, 0, bounds.min_x + (width - 1) * default_percentage / 100, Machine->uifont->colortable[OSDBAR_COLOR_DEFAULTBAR]);

#else /* UI_COLOR_DISPLAY */
	/* draw the top default percentage marker */
	tbounds = bounds;
	tbounds.min_x += (width - 1) * default_percentage / 100;
	tbounds.max_x = tbounds.min_x;
	tbounds.max_y = tbounds.min_y + height / 8;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);

	/* draw the bottom default percentage marker */
	tbounds = bounds;
	tbounds.min_x += (width - 1) * default_percentage / 100;
	tbounds.max_x = tbounds.min_x;
	tbounds.min_y = tbounds.max_y - height / 8;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);

	/* draw the top line of the bar */
	tbounds = bounds;
	tbounds.min_y += height / 8;
	tbounds.max_y = tbounds.min_y;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);

	/* draw the bottom line of the bar */
	tbounds = bounds;
	tbounds.max_y -= height / 8;
	tbounds.min_y = tbounds.max_y;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);

	/* fill in the percentage */
	tbounds = bounds;
	tbounds.max_x = tbounds.min_x + (width - 1) * percentage / 100;
	tbounds.min_y += height / 8;
	tbounds.max_y -= height / 8;
	ui_rot2raw_rect(&tbounds);
	fillbitmap(bitmap, white, &tbounds);
#endif /* UI_COLOR_DISPLAY */

	/* mark things dirty */
	ui_rot2raw_rect(&bounds);
	ui_markdirty(&bounds);
}


#ifdef UI_TITLE_MENU
void ui_displaymenu(struct mame_bitmap *bitmap,const char **items,const char **subitems,char *flag,const char *title,int selected,int arrowize_subitem)
#else
void ui_displaymenu(struct mame_bitmap *bitmap,const char **items,const char **subitems,char *flag,int selected,int arrowize_subitem)
#endif /* UI_TITLE_MENU */
{
	struct DisplayText dt[256];
	int curr_dt, curr_y;
#ifdef UI_COLOR_DISPLAY
	struct rectangle bounds;
#else /* UI_COLOR_DISPLAY */
	const char *lefthilight = ui_getstring (UI_lefthilight);
	const char *righthilight = ui_getstring (UI_righthilight);
#endif /* UI_COLOR_DISPLAY */
	const char *uparrow = ui_getstring (UI_uparrow);
	const char *downarrow = ui_getstring (UI_downarrow);
	const char *leftarrow = ui_getstring (UI_leftarrow);
	const char *rightarrow = ui_getstring (UI_rightarrow);
	int i,count,len,maxlen,highlen;
	int leftoffs,topoffs,visible,topitem;
	int selected_long;


	i = 0;
	maxlen = 0;
#ifdef UI_COLOR_DISPLAY
	highlen = (uirotwidth - 2 * DRAWBOX_MARGIN) / uirotcharwidth;
#else /* UI_COLOR_DISPLAY */
	highlen = uirotwidth / uirotcharwidth;
#endif /* UI_COLOR_DISPLAY */
	while (items[i])
	{
		len = (raw+law+1) + strlen(items[i]);
		if (subitems && subitems[i])
			len += (lhw+rhw) + strlen(subitems[i]);
		if (len > maxlen)
		{
			if (len <= highlen)
			maxlen = len;
			else
				maxlen = highlen;
		}
		i++;
	}
	count = i;

#ifdef UI_TITLE_MENU
	if (title)
		visible = (uirotheight / MENUROWHEIGHT) - 2;
	else
#endif /* UI_TITLE_MENU */
	visible = (uirotheight / MENUROWHEIGHT) - 1;
	topitem = 0;
	if (visible > count) visible = count;
	else
	{
		topitem = selected - visible / 2;
		if (topitem < 0) topitem = 0;
		if (topitem > count - visible) topitem = count - visible;
	}

	leftoffs = (uirotwidth - maxlen * uirotcharwidth) / 2;

#ifdef UI_TITLE_MENU
	if (title)
	{
		topoffs = (uirotheight - (visible + 1) * MENUROWHEIGHT) / 2;

		/* black background */
		ui_drawbox(bitmap,leftoffs,topoffs - DRAWBOX_MARGIN,maxlen * uirotcharwidth,(visible + 1) * MENUROWHEIGHT + DRAWBOX_MARGIN*2);
	}
	else
	{
#endif /* UI_TITLE_MENU */
	topoffs = (uirotheight - visible * MENUROWHEIGHT) / 2;

	/* black background */
	ui_drawbox(bitmap,leftoffs,topoffs - ROWMARGIN,maxlen * uirotcharwidth,visible * MENUROWHEIGHT + ROWMARGIN*2);
#ifdef UI_TITLE_MENU
	}

	if (title)
	{
		DRAW_TITLEBAR(leftoffs, topoffs - 1, maxlen, 0);

		dt[0].text = title;
		dt[0].color = UI_COLOR_BOLD;
		dt[0].x = (Machine->uiwidth - Machine->uifontwidth * strlen(dt[0].text)) / 2;
		dt[0].y = topoffs - 1;

		dt[1].text = 0;	/* terminate array */

		displaytext(bitmap,dt);

		topoffs += MENUROWHEIGHT + ROWMARGIN;
	}
#endif /* UI_TITLE_MENU */

	selected_long = 0;
	curr_dt = 0;
	for (i = 0;i < visible;i++)
	{
		int item = i + topitem;

		curr_y = topoffs + i * MENUROWHEIGHT + (uirotcharheight / 8);

		if (i == 0 && item > 0)
		{
			dt[curr_dt].text = uparrow;
			dt[curr_dt].color = UI_COLOR_NORMAL;
			dt[curr_dt].x = (uirotwidth - uirotcharwidth * uaw) / 2;
			dt[curr_dt].y = curr_y;
			curr_dt++;
		}
		else if (i == visible - 1 && item < count - 1)
		{
			dt[curr_dt].text = downarrow;
			dt[curr_dt].color = UI_COLOR_NORMAL;
			dt[curr_dt].x = (uirotwidth - uirotcharwidth * daw) / 2;
			dt[curr_dt].y = curr_y;
			curr_dt++;
		}
		else
		{
			if (subitems && subitems[item])
			{
				int sublen;
				len = strlen(items[item]);
				dt[curr_dt].text = items[item];
				dt[curr_dt].color = UI_COLOR_NORMAL;
				dt[curr_dt].x = leftoffs + uirotcharwidth/2 + uirotcharwidth * raw;
				dt[curr_dt].y = curr_y;
				curr_dt++;
				sublen = strlen(subitems[item]);
				if (sublen > maxlen-(raw+lhw+rhw+2)-len)
				{
					dt[curr_dt].text = "...";
					sublen = strlen(dt[curr_dt].text);
					if (item == selected)
						selected_long = 1;
				} else {
					dt[curr_dt].text = subitems[item];
				}
				/* If this item is flagged, draw it in inverse print */
				dt[curr_dt].color = (flag && flag[item]) ? UI_COLOR_INVERSE : UI_COLOR_NORMAL;
				dt[curr_dt].x = leftoffs + uirotcharwidth * (maxlen-rhw-sublen) - uirotcharwidth/2;
				dt[curr_dt].y = curr_y;
				curr_dt++;
			}
			else
			{
				dt[curr_dt].text = items[item];
				dt[curr_dt].color = UI_COLOR_NORMAL;
				dt[curr_dt].x = (uirotwidth - uirotcharwidth * strlen(items[item])) / 2;
				dt[curr_dt].y = curr_y;
				curr_dt++;
			}
		}
	}

	i = selected - topitem;

	curr_y = topoffs + i * MENUROWHEIGHT + (uirotcharheight / 8);

	if (subitems && subitems[selected] && arrowize_subitem)
	{
		if (arrowize_subitem & 1)
		{
			int sublen;

			len = strlen(items[selected]);

			dt[curr_dt].text = leftarrow;
			dt[curr_dt].color = UI_COLOR_NORMAL;

			sublen = strlen(subitems[selected]);
			if (sublen > maxlen-(raw+lhw+rhw+2)-len)
				sublen = strlen("...");

			dt[curr_dt].x = leftoffs + uirotcharwidth * (maxlen-(lhw+law)-sublen) - uirotcharwidth/2 - raw;
			dt[curr_dt].y = curr_y;
			curr_dt++;
		}
		if (arrowize_subitem & 2)
		{
			dt[curr_dt].text = rightarrow;
			dt[curr_dt].color = UI_COLOR_NORMAL;
			dt[curr_dt].x = leftoffs + uirotcharwidth * (maxlen-raw) - uirotcharwidth/2;
			dt[curr_dt].y = curr_y;
			curr_dt++;
		}
	}
#ifndef UI_COLOR_DISPLAY
	else
	{
		dt[curr_dt].text = righthilight;
		dt[curr_dt].color = UI_COLOR_NORMAL;
		dt[curr_dt].x = leftoffs + uirotcharwidth * (maxlen-rhw) - uirotcharwidth/2;
		dt[curr_dt].y = curr_y;
		curr_dt++;
	}
	dt[curr_dt].text = lefthilight;
	dt[curr_dt].color = UI_COLOR_NORMAL;
	dt[curr_dt].x = leftoffs + uirotcharwidth/2;
	dt[curr_dt].y = curr_y;
	curr_dt++;
#endif /* !UI_COLOR_DISPLAY */

	dt[curr_dt].text = 0;	/* terminate array */

#ifdef UI_COLOR_DISPLAY
	bounds.min_x = uirotbounds.min_x + leftoffs;
	bounds.min_y = uirotbounds.min_y + curr_y - 1;
	bounds.max_x = bounds.min_x + maxlen * uirotcharwidth - 1;
	bounds.max_y = bounds.min_y + ROWHEIGHT - 1;
#ifdef UI_TITLE_MENU
	bounds.max_y -= 1;
#endif /* UI_TITLE_MENU */

	ui_rot2raw_rect(&bounds);
	fillbitmap(bitmap, Machine->uifont->colortable[CURSOR_COLOR], &bounds);
#endif /* UI_COLOR_DISPLAY */

	displaytext(bitmap,dt);

	if (selected_long)
	{
		int long_dx;
		int long_dy;
		int long_x;
		int long_y;
		unsigned long_max;

#ifdef UI_COLOR_DISPLAY
		long_max = (uirotwidth / uirotcharwidth) - DRAWBOX_MARGIN * 2;
#else /* UI_COLOR_DISPLAY */
		long_max = (uirotwidth / uirotcharwidth) - 2;
#endif /* UI_COLOR_DISPLAY */
		multilinebox_size(&long_dx,&long_dy,subitems[selected],subitems[selected] + strlen(subitems[selected]), long_max);

		long_x = uirotwidth - long_dx;
		long_y = topoffs + (i+1) * MENUROWHEIGHT;
#ifdef UI_COLOR_DISPLAY
		long_x -= DRAWBOX_MARGIN;
#endif /* UI_COLOR_DISPLAY */

		/* if too low display up */
		if (long_y + long_dy > uirotheight)
			long_y = topoffs + i * MENUROWHEIGHT - long_dy;

		ui_multitextbox_ex(bitmap,subitems[selected],subitems[selected] + strlen(subitems[selected]), long_max, long_x,long_y,long_dx,long_dy, UI_COLOR_NORMAL);
	}
}


#ifdef UI_TITLE_MENU
int ui_displaymessagewindow(struct mame_bitmap *bitmap,const char *text, const char *title)
#else
int ui_displaymessagewindow(struct mame_bitmap *bitmap,const char *text)
#endif /* UI_TITLE_MENU */
{
	struct DisplayText dt[256];
	int curr_dt;
	char *c,*c2;
	int i,maxlen,lines;
	char textcopy[2048];
	int leftoffs,topoffs;
	int maxcols,maxrows;

#ifdef UI_COLOR_DISPLAY
	maxcols = ((uirotwidth - 2 * DRAWBOX_MARGIN) / uirotcharwidth) - 1;
	maxrows = ((uirotheight - 2 * DRAWBOX_MARGIN) / ROWHEIGHT) - 1;
#else /* UI_COLOR_DISPLAY */
	maxcols = (uirotwidth / uirotcharwidth) - 1;
	maxrows = (uirotheight / ROWHEIGHT) - 1;
#endif /* UI_COLOR_DISPLAY */

	if (maxcols < 0)
		maxcols = 0;
	if (maxrows < 0)
		maxrows = 0;

	strcpy(textcopy, text);
	maxlen = wordwrap_text_buffer(textcopy, maxcols);
	lines  = count_lines_in_buffer(textcopy) + 1;

#ifdef UI_TITLE_MENU
	if (title)
	{
		int len = strlen(title) + 2;
		if (len > maxlen && len <= Machine->uiwidth / Machine->uifontwidth)
			maxlen = len;
	}
#endif /* UI_TITLE_MENU */

	if (lines > maxrows)
	{
		static int scroll = 0;

		strcat(textcopy, "\n");
		if (scroll_reset)
		{
			scroll = 0;
			scroll_reset = 0;
		}

#ifdef UI_TITLE_MENU
		display_scroll_message(bitmap, &scroll, maxlen, maxrows, textcopy, title);
#else
		display_scroll_message(bitmap, &scroll, maxlen, maxrows, textcopy);
#endif /* UI_TITLE_MENU */

		if ((scroll > 0) && input_ui_pressed_repeat(IPT_UI_UP,4))
		{
			if (scroll == 2) scroll = 0;	/* 1 would be the same as 0, but with arrow on top */
			else scroll--;
			return -1;
		}
		else if (input_ui_pressed_repeat(IPT_UI_DOWN,4))
			{
			if (scroll == 0) scroll = 2;	/* 1 would be the same as 0, but with arrow on top */
			else scroll++;
			return -1;
		}
				else
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			scroll = 0;
			return 1;
			}
		else
		if (input_ui_pressed(IPT_UI_CANCEL))
		{
			scroll = 0;
			return 2;
		}
		else
		if (input_ui_pressed(IPT_UI_CONFIGURE))
		{
			scroll = 0;
			return 3;
		}

		return 0;
	}

	maxlen += 1;

	leftoffs = (uirotwidth - uirotcharwidth * maxlen) / 2;
	if (leftoffs < 0) leftoffs = 0;

#ifdef UI_TITLE_MENU
	if (title)
	{
		topoffs = (uirotheight - ((lines + 1) * ROWHEIGHT + ROWMARGIN)) / 2;

		if (topoffs < DRAWBOX_MARGIN)
			topoffs = DRAWBOX_MARGIN;

		/* black background */
		ui_drawbox(bitmap,leftoffs,topoffs - DRAWBOX_MARGIN,maxlen * uirotcharwidth,(lines + 1) * ROWHEIGHT + DRAWBOX_MARGIN*2);
	}
	else
	{
#endif /* UI_TITLE_MENU */
	if (!displaymessagewindow_under)
		topoffs = (uirotheight - (lines * ROWHEIGHT + ROWMARGIN)) / 2;
	else
		topoffs = uirotheight - ((lines + 1) * ROWHEIGHT + ROWMARGIN);
	if (topoffs < 0) topoffs = 0;

	/* black background */
	ui_drawbox(bitmap,leftoffs,topoffs,maxlen * uirotcharwidth,lines * ROWHEIGHT + ROWMARGIN);

#ifdef UI_TITLE_MENU
	}

	if (title)
	{
		DRAW_TITLEBAR(leftoffs, topoffs - 1, maxlen, 0);

		dt[0].text = title;
		dt[0].color = UI_COLOR_BOLD;
		dt[0].x = (Machine->uiwidth - Machine->uifontwidth * strlen(dt[0].text)) / 2;
		dt[0].y = topoffs - 1;

		dt[1].text = 0;	/* terminate array */

		displaytext(bitmap,dt);

		topoffs += ROWHEIGHT;
	}
#endif /* UI_TITLE_MENU */

	curr_dt = 0;
	c = textcopy;
	i = 0;
	while (*c)
	{
		c2 = c;
		while (*c && *c != '\n')
			c++;

		if (*c == '\n')
		{
			*c = '\0';
			c++;
		}

		if (*c2 == '\t')    /* center text */
		{
			c2++;
			dt[curr_dt].x = (uirotwidth - uirotcharwidth * (c - c2)) / 2;
		}
		else
			dt[curr_dt].x = leftoffs + uirotcharwidth/2;

		dt[curr_dt].text = c2;
		dt[curr_dt].color = UI_COLOR_NORMAL;
		dt[curr_dt].y = topoffs + i * ROWHEIGHT + ROWMARGIN;
		curr_dt++;

		i++;
	}

	dt[curr_dt].text = 0;	/* terminate array */

	displaytext(bitmap,dt);

	return 0;
}



int ui_displaymessagewindow_under(struct mame_bitmap *bitmap,const char *text)
{
	int result;

	displaymessagewindow_under = 1;
#ifdef UI_TITLE_MENU
	result = ui_displaymessagewindow(bitmap, text, 0);
#else	
	result = ui_displaymessagewindow(bitmap, text);
#endif /* UI_TITLE_MENU */
	displaymessagewindow_under = 0;

	return result;
}



static void showcharset(struct mame_bitmap *bitmap)
{
#ifdef UI_COLOR_DISPLAY
#define ui_text	ui_text_nobk
#endif /* UI_COLOR_DISPLAY */

	int i;
	char buf[80];
	int mode,bank,color,firstdrawn;
	int palpage;
	int changed;
	int total_colors = 0;
	pen_t *colortable = NULL;
	int cpx=0,cpy,skip_chars=0,skip_tmap=0;
	int tilemap_xpos = 0;
	int tilemap_ypos = 0;

	mode = 0;
	bank = 0;
	color = 0;
	firstdrawn = 0;
	palpage = 0;

	changed = 1;

	do
	{
		static const struct rectangle fullrect = { 0, 10000, 0, 10000 };

		/* mark the whole thing dirty */
		ui_markdirty(&fullrect);

		switch (mode)
		{
			case 0: /* palette or clut */
			{
				if (bank == 0)	/* palette */
				{
					total_colors = Machine->drv->total_colors;
					colortable = Machine->pens;
					strcpy(buf,"PALETTE");
				}
				else if (bank == 1)	/* clut */
				{
					total_colors = Machine->drv->color_table_len;
					colortable = Machine->remapped_colortable;
					strcpy(buf,"CLUT");
				}
				else
				{
					buf[0] = 0;
					total_colors = 0;
					colortable = 0;
				}

				/*if (changed) -- temporary */
				{
					erase_screen(bitmap);

					if (total_colors)
					{
						int sx,sy,colors;
						int column_heading_max;
						struct bounds;

						colors = total_colors - 256 * palpage;
						if (colors > 256) colors = 256;

						/* min(colors, 16) */
						if (colors < 16)
							column_heading_max = colors;
						else
							column_heading_max = 16;

						for (i = 0;i < column_heading_max;i++)
						{
							char bf[40];

							sx = 3*uirotcharwidth + (uirotcharwidth*4/3)*(i % 16);
							sprintf(bf,"%X",i);
							ui_text(bitmap,bf,sx,2*uirotcharheight);
							if (16*i < colors)
							{
								sy = 3*uirotcharheight + (uirotcharheight)*(i % 16);
								sprintf(bf,"%3X",i+16*palpage);
								ui_text(bitmap,bf,0,sy);
							}
						}

						for (i = 0;i < colors;i++)
						{
							struct rectangle bounds;
							bounds.min_x = uirotbounds.min_x + 3*uirotcharwidth + (uirotcharwidth*4/3)*(i % 16);
							bounds.min_y = uirotbounds.min_y + 2*uirotcharheight + (uirotcharheight)*(i / 16) + uirotcharheight;
							bounds.max_x = bounds.min_x + uirotcharwidth*4/3 - 1;
							bounds.max_y = bounds.min_y + uirotcharheight - 1;
							ui_rot2raw_rect(&bounds);
							fillbitmap(bitmap, colortable[i + 256*palpage], &bounds);
						}
					}
					else
						ui_text(bitmap,"N/A",3*uirotcharwidth,2*uirotcharheight);

					ui_text(bitmap,buf,0,0);
					changed = 0;
				}

				break;
			}
			case 1: /* characters */
			{
				int crotwidth = (Machine->ui_orientation & ORIENTATION_SWAP_XY) ? Machine->gfx[bank]->height : Machine->gfx[bank]->width;
				int crotheight = (Machine->ui_orientation & ORIENTATION_SWAP_XY) ? Machine->gfx[bank]->width : Machine->gfx[bank]->height;
				cpx = uirotwidth / crotwidth;
				if (cpx == 0) cpx = 1;
				cpy = (uirotheight - uirotcharheight) / crotheight;
				if (cpy == 0) cpy = 1;
				skip_chars = cpx * cpy;
				/*if (changed) -- temporary */
				{
					int flipx,flipy;
					int lastdrawn=0;

					erase_screen(bitmap);

					/* validity check after char bank change */
					if (firstdrawn >= Machine->gfx[bank]->total_elements)
					{
						firstdrawn = Machine->gfx[bank]->total_elements - skip_chars;
						if (firstdrawn < 0) firstdrawn = 0;
					}

					flipx = 0;
					flipy = 0;

					for (i = 0; i+firstdrawn < Machine->gfx[bank]->total_elements && i<cpx*cpy; i++)
					{
						struct rectangle bounds;
						bounds.min_x = (i % cpx) * crotwidth + uirotbounds.min_x;
						bounds.min_y = uirotcharheight + (i / cpx) * crotheight + uirotbounds.min_y;
						bounds.max_x = bounds.min_x + crotwidth - 1;
						bounds.max_y = bounds.min_y + crotheight - 1;
						ui_rot2raw_rect(&bounds);

						drawgfx(bitmap,Machine->gfx[bank],
								i+firstdrawn,color,  /*sprite num, color*/
								flipx,flipy,bounds.min_x,bounds.min_y,
								0,Machine->gfx[bank]->colortable ? TRANSPARENCY_NONE : TRANSPARENCY_NONE_RAW,0);

						lastdrawn = i+firstdrawn;
					}

					sprintf(buf,"GFXSET %d COLOR %2X CODE %X-%X",bank,color,firstdrawn,lastdrawn);
					ui_text(bitmap,buf,0,0);
					changed = 0;
				}

				break;
			}
			case 2: /* Tilemaps */
			{
				/*if (changed) -- temporary */
				{
					UINT32 tilemap_width, tilemap_height;
					tilemap_nb_size (bank, &tilemap_width, &tilemap_height);
					while (tilemap_xpos < 0)
						tilemap_xpos += tilemap_width;
					tilemap_xpos %= tilemap_width;

					while (tilemap_ypos < 0)
						tilemap_ypos += tilemap_height;
					tilemap_ypos %= tilemap_height;

					erase_screen(bitmap);
					tilemap_nb_draw (bitmap, bank, tilemap_xpos, tilemap_ypos);
					sprintf(buf, "TILEMAP %d (%dx%d)  X:%d  Y:%d", bank, tilemap_width, tilemap_height, tilemap_xpos, tilemap_ypos);
					ui_text(bitmap,buf,0,0);
					changed = 0;
					skip_tmap = 0;
				}
				break;
			}
		}

		update_video_and_audio();

		if (code_pressed(KEYCODE_LCONTROL) || code_pressed(KEYCODE_RCONTROL))
		{
			skip_chars = cpx;
			skip_tmap = 8;
		}
#ifdef MAME_AVI
		extern BOOL	bAviRun;
		if ((code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT)) && !bAviRun)
#else
		if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
#endif
		{
			skip_chars = 1;
			skip_tmap = 1;
		}


		if (input_ui_pressed_repeat(IPT_UI_RIGHT,8))
		{
			int next_bank, next_mode;
			int jumped;

			next_mode = mode;
			next_bank = bank+1;
			do {
				jumped = 0;
				switch (next_mode)
				{
					case 0:
						if (next_bank == 2 || Machine->drv->color_table_len == 0)
						{
							jumped = 1;
							next_mode++;
							next_bank = 0;
						}
						break;
					case 1:
						if (next_bank == MAX_GFX_ELEMENTS || !Machine->gfx[next_bank])
						{
							jumped = 1;
							next_mode++;
							next_bank = 0;
						}
						break;
					case 2:
						if (next_bank == tilemap_count())
							next_mode = -1;
						break;
				}
			}	while (jumped);
			if (next_mode != -1 )
			{
				bank = next_bank;
				mode = next_mode;
//              firstdrawn = 0;
				changed = 1;
			}
		}

		if (input_ui_pressed_repeat(IPT_UI_LEFT,8))
		{
			int next_bank, next_mode;

			next_mode = mode;
			next_bank = bank-1;
			while(next_bank < 0 && next_mode >= 0)
			{
				next_mode = next_mode - 1;
				switch (next_mode)
				{
					case 0:
						if (Machine->drv->color_table_len == 0)
							next_bank = 0;
						else
							next_bank = 1;
						break;
					case 1:
						next_bank = MAX_GFX_ELEMENTS-1;
						while (next_bank >= 0 && !Machine->gfx[next_bank])
							next_bank--;
						break;
					case 2:
						next_bank = tilemap_count() - 1;
						break;
				}
			}
			if (next_mode != -1 )
			{
				bank = next_bank;
				mode = next_mode;
//              firstdrawn = 0;
				changed = 1;
			}
		}

		if (code_pressed_memory_repeat(KEYCODE_PGDN,4))
		{
			switch (mode)
			{
				case 0:
				{
					if (256 * (palpage + 1) < total_colors)
					{
						palpage++;
						changed = 1;
					}
					break;
				}
				case 1:
				{
					if (firstdrawn + skip_chars < Machine->gfx[bank]->total_elements)
					{
						firstdrawn += skip_chars;
						changed = 1;
					}
					break;
				}
				case 2:
				{
					if (skip_tmap)
						tilemap_ypos -= skip_tmap;
					else
						tilemap_ypos -= bitmap->height/4;
					changed = 1;
					break;
				}
			}
		}

		if (code_pressed_memory_repeat(KEYCODE_PGUP,4))
		{
			switch (mode)
			{
				case 0:
				{
					if (palpage > 0)
					{
						palpage--;
						changed = 1;
					}
					break;
				}
				case 1:
				{
					firstdrawn -= skip_chars;
					if (firstdrawn < 0) firstdrawn = 0;
					changed = 1;
					break;
				}
				case 2:
				{
					if (skip_tmap)
						tilemap_ypos += skip_tmap;
					else
						tilemap_ypos += bitmap->height/4;
					changed = 1;
					break;
				}
			}
		}

		if (code_pressed_memory_repeat(KEYCODE_D,4))
		{
			switch (mode)
			{
				case 2:
				{
					if (skip_tmap)
						tilemap_xpos -= skip_tmap;
					else
						tilemap_xpos -= bitmap->width/4;
					changed = 1;
					break;
				}
			}
		}

		if (code_pressed_memory_repeat(KEYCODE_G,4))
		{
			switch (mode)
			{
				case 2:
				{
					if (skip_tmap)
						tilemap_xpos += skip_tmap;
					else
						tilemap_xpos += bitmap->width/4;
					changed = 1;
					break;
				}
			}
		}

		if (input_ui_pressed_repeat(IPT_UI_UP,6))
		{
			switch (mode)
			{
				case 1:
				{
					if (color < Machine->gfx[bank]->total_colors - 1)
					{
						color++;
						changed = 1;
					}
					break;
				}
			}
		}

		if (input_ui_pressed_repeat(IPT_UI_DOWN,6))
		{
			switch (mode)
			{
				case 0:
					break;
				case 1:
				{
					if (color > 0)
					{
						color--;
						changed = 1;
					}
				}
			}
		}

		if (input_ui_pressed(IPT_UI_SNAPSHOT))
			save_screen_snapshot(bitmap);

#ifdef MAME_AVI //(NEW_AVI)
		if (input_ui_pressed(IPT_UI_RECORD_AVI))
			toggle_record_avi();
#endif
			
	} while (!input_ui_pressed(IPT_UI_SHOW_GFX) &&
			!input_ui_pressed(IPT_UI_CANCEL));

	schedule_full_refresh();
#ifdef UI_COLOR_DISPLAY
#undef ui_text
#endif /* UI_COLOR_DISPLAY */
}



static int switchmenu(struct mame_bitmap *bitmap, int selected, UINT32 switch_name, UINT32 switch_setting)
{
	#define MAX_DIP_SWITCHES 256
	const char *menu_item[MAX_DIP_SWITCHES+2];	// 2 more for "return to mainmenu" and end-of-list marker
	const char *menu_subitem[MAX_DIP_SWITCHES+2];
	struct InputPort *entry[MAX_DIP_SWITCHES];
	char flag[40];
	int i,sel;
	struct InputPort *in;
	int total;
	int arrowize;


	sel = selected - 1;


	in = Machine->input_ports;

	total = 0;
	while (in->type != IPT_END && total < MAX_DIP_SWITCHES)
	{
		if (in->type == switch_name && input_port_active(in))
		{
			entry[total] = in;
			menu_item[total] = _(input_port_name(in));

			total++;
		}

		in++;
	}

	if (total == 0) return 0;

	menu_item[total] = ui_getstring (UI_returntomain);
	menu_item[total + 1] = 0;	/* terminate array */
	total++;


	for (i = 0;i < total;i++)
	{
		flag[i] = 0; /* TODO: flag the dip if it's not the real default */
		if (i < total - 1)
		{
			in = entry[i] + 1;
			while (in->type == switch_setting &&
					(in->default_value != entry[i]->default_value || !input_port_condition(in)))
				in++;

			if (in->type != switch_setting)
				menu_subitem[i] = ui_getstring (UI_INVALID);
			else menu_subitem[i] = _(input_port_name(in));
		}
		else menu_subitem[i] = 0;	/* no subitem */
	}

	arrowize = 0;
	if (sel < total - 1)
	{
		in = entry[sel] + 1;
		while (in->type == switch_setting &&
				(in->default_value != entry[sel]->default_value || !input_port_condition(in)))
			in++;

		if (in->type != switch_setting)
			/* invalid setting: revert to a valid one */
			arrowize |= 1;
		else
		{
			in--;
			while (in->type == switch_setting && !input_port_condition(in))
				in--;
			if (in->type == switch_setting)
				arrowize |= 1;
		}
	}
	if (sel < total - 1)
	{
		in = entry[sel] + 1;
		while (in->type == switch_setting &&
				(in->default_value != entry[sel]->default_value || !input_port_condition(in)))
			in++;

		if (in->type != switch_setting)
			/* invalid setting: revert to a valid one */
			arrowize |= 2;
		else
		{
			in++;
			while (in->type == switch_setting && !input_port_condition(in))
				in++;
			if (in->type == switch_setting)
				arrowize |= 2;
		}
	}

#ifdef UI_TITLE_MENU
	ui_displaymenu(bitmap,menu_item,menu_subitem,flag,ui_getstring(UI_dipswitches),sel,arrowize);
#else
	ui_displaymenu(bitmap,menu_item,menu_subitem,flag,sel,arrowize);
#endif /* UI_TITLE_MENU */

	if (input_ui_pressed_repeat(IPT_UI_DOWN,8))
		sel = (sel + 1) % total;

	if (input_ui_pressed_repeat(IPT_UI_UP,8))
		sel = (sel + total - 1) % total;

	if (input_ui_pressed_repeat(IPT_UI_RIGHT,8))
	{
		if (sel < total - 1)
		{
			in = entry[sel] + 1;
			while (in->type == switch_setting &&
					(in->default_value != entry[sel]->default_value || !input_port_condition(in)))
				in++;

			if (in->type != switch_setting)
				/* invalid setting: revert to a valid one */
				entry[sel]->default_value = (entry[sel]+1)->default_value & entry[sel]->mask;
			else
			{
				in++;
				while (in->type == switch_setting && !input_port_condition(in))
					in++;
				if (in->type == switch_setting)
					entry[sel]->default_value = in->default_value & entry[sel]->mask;
			}

			/* tell updatescreen() to clean after us (in case the window changes size) */
			schedule_full_refresh();
		}
	}

	if (input_ui_pressed_repeat(IPT_UI_LEFT,8))
	{
		if (sel < total - 1)
		{
			in = entry[sel] + 1;
			while (in->type == switch_setting &&
					(in->default_value != entry[sel]->default_value || !input_port_condition(in)))
				in++;

			if (in->type != switch_setting)
				/* invalid setting: revert to a valid one */
				entry[sel]->default_value = (entry[sel]+1)->default_value & entry[sel]->mask;
			else
			{
				in--;
				while (in->type == switch_setting && !input_port_condition(in))
					in--;
				if (in->type == switch_setting)
					entry[sel]->default_value = in->default_value & entry[sel]->mask;
			}

			/* tell updatescreen() to clean after us (in case the window changes size) */
			schedule_full_refresh();
		}
	}

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		if (sel == total - 1) sel = -1;
	}

	if (input_ui_pressed(IPT_UI_CANCEL))
		sel = -1;

	if (input_ui_pressed(IPT_UI_CONFIGURE))
		sel = -2;

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();
	}

	return sel + 1;
}



static int setdipswitches(struct mame_bitmap *bitmap, int selected)
{
	return switchmenu(bitmap, selected, IPT_DIPSWITCH_NAME, IPT_DIPSWITCH_SETTING);
}



#ifdef MESS
static int setconfiguration(struct mame_bitmap *bitmap, int selected)
{
	return switchmenu(bitmap, selected, IPT_CONFIG_NAME, IPT_CONFIG_SETTING);
}



static int setcategories(struct mame_bitmap *bitmap, int selected)
{
	return switchmenu(bitmap, selected, IPT_CATEGORY_NAME, IPT_CATEGORY_SETTING);
}
#endif /* MESS */



/* This flag is used for record OR sequence of key/joy */
/* when is !=0 the first sequence is record, otherwise the first free */
/* it's used by setdefkeysettings, setdefjoysettings, setkeysettings, setjoysettings */
static int record_first_insert = 1;

#define MAX_PORT_ENTRIES		1000

static char menu_item_buffer[MAX_PORT_ENTRIES][96];
static char menu_subitem_buffer[MAX_PORT_ENTRIES][96];

static int setdefcodesettings(struct mame_bitmap *bitmap,int selected)
{
	static input_seq_t starting_seq;
	static int menugroup = -1;
	const char *menu_item[MAX_PORT_ENTRIES];
	const char *menu_subitem[MAX_PORT_ENTRIES];
	input_seq_t *entry[MAX_PORT_ENTRIES];
	input_seq_t *defentry[MAX_PORT_ENTRIES];
	UINT8 analog[MAX_PORT_ENTRIES];
	char flag[MAX_PORT_ENTRIES];
	int i,sel;
	struct InputPortDefinition *in, *indef;
	int total;
	int visible;
	static int counter = 0;
	static int fast = 4;

	sel = selected - 1;
	visible = (uirotheight / MENUROWHEIGHT) - 3;


	if (Machine->input_ports == 0)
		return 0;

	if (menugroup == -1)
	{
		for (total = 0; total < IPG_TOTAL_GROUPS; total++)
			menu_item[total] = ui_getstring(UI_uigroup + total);
		menu_item[total++] = ui_getstring (UI_returntomain);
		menu_item[total] = 0;

#ifdef UI_TITLE_MENU
		ui_displaymenu(bitmap,menu_item,0,0,ui_getstring(UI_inputgeneral),sel,0);
#else
		ui_displaymenu(bitmap,menu_item,0,0,sel,0);
#endif /* UI_TITLE_MENU */
	}
	else
	{
		in = get_input_port_list();
		indef = get_input_port_list_backup();

		total = 0;

		while (in->type != IPT_END)
		{
			if (in->name != 0 && in->type != IPT_UNKNOWN && in->type != IPT_DIPSWITCH_NAME && in->name[0] != 0 && in->group == menugroup)
			{
				analog[total] = port_type_is_analog(in->type);
				if (!analog[total])
				{
					entry[total] = &in->defaultseq;
					defentry[total] = &indef->defaultseq;
					menu_item[total] = _(in->name);
					total++;
				}
				else
				{
					entry[total] = &in->defaultseq;
					defentry[total] = &indef->defaultseq;
					sprintf(menu_item_buffer[total], _("%s Analog"), _(in->name));
					menu_item[total] = menu_item_buffer[total];
					total++;

					analog[total] = 0;
					entry[total] = &in->defaultdecseq;
					defentry[total] = &indef->defaultdecseq;
					sprintf(menu_item_buffer[total], _("%s Dec"), _(in->name));
					menu_item[total] = menu_item_buffer[total];
					total++;

					analog[total] = 0;
					entry[total] = &in->defaultincseq;
					defentry[total] = &indef->defaultincseq;
					sprintf(menu_item_buffer[total], _("%s Inc"), _(in->name));
					menu_item[total] = menu_item_buffer[total];
					total++;
				}
			}

			in++;
			indef++;
		}

		if (total == 0) return 0;

		menu_item[total] = ui_getstring (UI_returntogroup);
		menu_item[total + 1] = 0;	/* terminate array */
		total++;

		for (i = 0;i < total;i++)
		{
			if (i < total - 1)
			{
				seq_name(entry[i],menu_subitem_buffer[i],sizeof(menu_subitem_buffer[0]));
				menu_subitem[i] = menu_subitem_buffer[i];
			} else
				menu_subitem[i] = 0;	/* no subitem */
			flag[i] = 0;
		}

		if (sel > SEL_MASK)   /* are we waiting for a new key? */
		{
			int ret;

			menu_subitem[sel & SEL_MASK] = "    ";
#ifdef UI_TITLE_MENU
		ui_displaymenu(bitmap,menu_item,menu_subitem,flag,ui_getstring(UI_uigroup + menugroup),sel & SEL_MASK,3);
#else
			ui_displaymenu(bitmap,menu_item,menu_subitem,flag,sel & SEL_MASK,3);
#endif /* UI_TITLE_MENU */
			ret = seq_read_async(entry[sel & SEL_MASK],record_first_insert);

			if (ret >= 0)
			{
				sel &= SEL_MASK;

				if (ret > 0 || seq_get_1(entry[sel]) == CODE_NONE)
				{
					if (seq_get_1(&starting_seq) == CODE_NONE)
						seq_copy(entry[sel], defentry[sel]);
					else
						seq_set_1(entry[sel],CODE_NONE);
					ret = 1;
				}

				/* tell updatescreen() to clean after us (in case the window changes size) */
				schedule_full_refresh();

				record_first_insert = ret != 0;
			}

			return sel + 1;
		}

#ifdef UI_TITLE_MENU
	ui_displaymenu(bitmap,menu_item,menu_subitem,flag,ui_getstring(UI_uigroup + menugroup),sel,0);
#else
		ui_displaymenu(bitmap,menu_item,menu_subitem,flag,sel,0);
#endif /* UI_TITLE_MENU */
	}
	
	if (input_ui_pressed_repeat(IPT_UI_DOWN,fast))
	{
		sel = (sel + 1) % total;
		record_first_insert = 1;
	}

	if (input_ui_pressed_repeat(IPT_UI_UP,fast))
	{
		sel = (sel + total - 1) % total;
		record_first_insert = 1;
	}

	if (input_port_type_pressed(IPT_UI_UP,0) | input_port_type_pressed(IPT_UI_DOWN,0))
	{
		if (++counter == 25)
		{
			fast--;
			if (fast < 2) fast = 2;
			counter = 0;
		}
	}
	else fast = 8;

	if (input_ui_pressed_repeat(IPT_UI_PAN_UP,8))
	{
		sel -= visible;
		if (sel < 0) sel = 0;
		record_first_insert = 1;
	}

	if (input_ui_pressed_repeat(IPT_UI_PAN_DOWN,8))
	{
		sel += visible;
		if (sel >= total) sel = total - 1;
		record_first_insert = 1;
	}

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		if (sel == total - 1)
		{
			if (menugroup == -1)
				sel = -1;
			else
			{
				sel = menugroup;
				menugroup = -1;
			}
		}
		else if (menugroup == -1)
		{
			menugroup = sel;
			sel = 0;
		}
		else
		{
			seq_read_async_start(analog[sel]);
			seq_copy(&starting_seq,entry[sel]);

			sel |= 1 << SEL_BITS;	/* we'll ask for a key */

			/* tell updatescreen() to clean after us (in case the window changes size) */
			schedule_full_refresh();
		}
	}

	if (input_ui_pressed(IPT_UI_CANCEL))
	{
		if (menugroup == -1)
			sel = -1;
		else
		{
			sel = menugroup;
			menugroup = -1;
		}
	}

	if (input_ui_pressed(IPT_UI_CLEAR))
	{
		if (menugroup != -1)
		{
			if (sel < (total - 1))
			{
				if (seq_get_1(entry[sel]) == CODE_NONE)
					seq_copy(entry[sel], defentry[sel]);
				else
					seq_set_1(entry[sel], CODE_NONE);

				schedule_full_refresh();
			}
		}
	}

	if (input_ui_pressed(IPT_UI_CONFIGURE))
		sel = -2;

	if (sel == -1 || sel == -2)
	{
		/* tell updatescreen() to clean after us */
		schedule_full_refresh();

		record_first_insert = 1;
	}

	return sel + 1;
}



static int setcodesettings(struct mame_bitmap *bitmap,int selected)
{
	static input_code_t starting_code = CODE_NONE;
	const char *menu_item[MAX_PORT_ENTRIES];
	const char *menu_subitem[MAX_PORT_ENTRIES];
	input_seq_t *seq[MAX_PORT_ENTRIES];
	UINT8 flag[MAX_PORT_ENTRIES];
	UINT8 analog[MAX_PORT_ENTRIES];
	int i,sel;
	struct InputPort *in;
	int total;
	int visible;
	static int counter = 0;
	static int fast = 4;


	sel = selected - 1;
	visible = (uirotheight / MENUROWHEIGHT) - 3;


	if (Machine->input_ports == 0)
		return 0;

	in = Machine->input_ports;

	total = 0;
	while (in->type != IPT_END)
	{
		if (input_port_name(in) != NULL &&
#ifdef MESS
			(in->category == 0 || input_category_active(in->category)) &&
#endif /* MESS */
			((in->type == IPT_OTHER && in->name != IP_NAME_DEFAULT) || port_type_to_group(in->type, in->player) != IPG_INVALID))
		{
			analog[total] = port_type_is_analog(in->type);
			if (!analog[total])
			{
				seq[total] = &in->seq;
				menu_item[total] = _(input_port_name(in));
				seq_name(input_port_seq(in, SEQ_TYPE_STANDARD), menu_subitem_buffer[total], sizeof(menu_subitem_buffer[0]));
				menu_subitem[total] = menu_subitem_buffer[total];
				total++;
			}
			else
			{
				seq[total] = &in->seq;
				sprintf(menu_item_buffer[total], _("%s Analog"), _(input_port_name(in)));
				menu_item[total] = menu_item_buffer[total];
				seq_name(input_port_seq(in, SEQ_TYPE_STANDARD), menu_subitem_buffer[total], sizeof(menu_subitem_buffer[0]));
				menu_subitem[total] = menu_subitem_buffer[total];
				total++;

				analog[total] = 0;
				seq[total] = &in->analog.decseq;
				sprintf(menu_item_buffer[total], _("%s Dec"), _(input_port_name(in)));
				menu_item[total] = menu_item_buffer[total];
				seq_name(input_port_seq(in, SEQ_TYPE_DECREMENT), menu_subitem_buffer[total], sizeof(menu_subitem_buffer[0]));
				menu_subitem[total] = menu_subitem_buffer[total];
				total++;

				analog[total] = 0;
				seq[total] = &in->analog.incseq;
				sprintf(menu_item_buffer[total], _("%s Inc"), _(input_port_name(in)));
				menu_item[total] = menu_item_buffer[total];
				seq_name(input_port_seq(in, SEQ_TYPE_INCREMENT), menu_subitem_buffer[total], sizeof(menu_subitem_buffer[0]));
				menu_subitem[total] = menu_subitem_buffer[total];
				total++;
			}
		}

		in++;
	}

	for (i = 0; i < total; i++)
	{
		/* If the key isn't the default, flag it */
		if (seq_get_1(seq[i]) != CODE_DEFAULT)
			flag[i] = 1;
		else
			flag[i] = 0;
	}


	if (total == 0) return 0;

	menu_item[total] = ui_getstring (UI_returntomain);
	menu_subitem[total] = 0;	/* no subitem */
	menu_item[total + 1] = 0;	/* terminate array */
	total++;

	if (sel > SEL_MASK)   /* are we waiting for a new key? */
	{
		int ret;

		menu_subitem[sel & SEL_MASK] = "    ";
#ifdef UI_TITLE_MENU
		ui_displaymenu(bitmap,menu_item,menu_subitem,flag,ui_getstring(UI_inputspecific),sel & SEL_MASK,3);
#else
		ui_displaymenu(bitmap,menu_item,menu_subitem, (char *) flag,sel & SEL_MASK,3);
#endif /* UI_TITLE_MENU */
		ret = seq_read_async(seq[sel & SEL_MASK],record_first_insert);

		if (ret >= 0)
		{
			sel &= SEL_MASK;

			if (ret > 0 || seq_get_1(seq[sel]) == CODE_NONE)
			{
				seq_set_1(seq[sel], (starting_code == CODE_DEFAULT) ? CODE_NONE : CODE_DEFAULT);
				ret = 1;
			}

			/* tell updatescreen() to clean after us (in case the window changes size) */
			schedule_full_refresh();

			record_first_insert = ret != 0;
		}

		return sel + 1;
	}

#ifdef UI_TITLE_MENU
	ui_displaymenu(bitmap,menu_item,menu_subitem, (char *) flag,ui_getstring(UI_inputspecific),sel,0);
#else
	ui_displaymenu(bitmap,menu_item,menu_subitem, (char *) flag,sel,0);
#endif /* UI_TITLE_MENU */
	if (input_ui_pressed_repeat(IPT_UI_DOWN,fast))
	{
		sel = (sel + 1) % total;
		record_first_insert = 1;
	}

	if (input_ui_pressed_repeat(IPT_UI_UP,fast))
	{
		sel = (sel + total - 1) % total;
		record_first_insert = 1;
	}

	if (input_port_type_pressed(IPT_UI_UP,0) | input_port_type_pressed(IPT_UI_DOWN,0))
	{
		if (++counter == 25)
		{
			fast--;
			if (fast < 2) fast = 2;
			counter = 0;
		}
	}
	else fast = 8;

	if (input_ui_pressed_repeat(IPT_UI_PAN_UP,8))
	{
		sel -= visible;
		if (sel < 0) sel = 0;
		record_first_insert = 1;
	}

	if (input_ui_pressed_repeat(IPT_UI_PAN_DOWN,8))
	{
		sel += visible;
		if (sel >= total) sel = total - 1;
		record_first_insert = 1;
	}

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		if (sel == total - 1) sel = -1;
		else
		{
			starting_code = seq_get_1(seq[sel]);
			seq_read_async_start(analog[sel]);

			sel |= 1 << SEL_BITS;	/* we'll ask for a key */

			/* tell updatescreen() to clean after us (in case the window changes size) */
			schedule_full_refresh();
		}
	}

	if (input_ui_pressed(IPT_UI_CANCEL))
		sel = -1;

	if (input_ui_pressed(IPT_UI_CLEAR))
	{
		if (sel < (total - 1))
		{
			if (seq_get_1(seq[sel]) == CODE_NONE)
				seq_set_1(seq[sel], CODE_DEFAULT);
			else
				seq_set_1(seq[sel], CODE_NONE);

			schedule_full_refresh();
		}
	}

	if (input_ui_pressed(IPT_UI_CONFIGURE))
		sel = -2;

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();

		record_first_insert = 1;
	}

	return sel + 1;
}


#ifdef USE_CUSTOM_BUTTON
static int setcustombutton(struct mame_bitmap *bitmap, int selected)
{
	const char *menu_item[MAX_PLAYERS * MAX_CUSTOM_BUTTONS + 2];
	const char *menu_subitem[MAX_PLAYERS * MAX_CUSTOM_BUTTONS + 2];
	UINT16 *custom_item[MAX_PLAYERS * MAX_CUSTOM_BUTTONS];
	char buf[MAX_PLAYERS * MAX_CUSTOM_BUTTONS][MAX_NORMAL_BUTTONS * 4];
	int is_neogeo = !stricmp(Machine->gamedrv->source_file+12, "neogeo.c");
	int buttons = 0;
	struct InputPort *in;
	int i, sel;
	int total;

	sel = selected - 1;

	if (Machine->input_ports == 0)
		return 0;

	memset(custom_item, 0, sizeof custom_item);
	total = 0;
	for (in = Machine->input_ports; in->type != IPT_END; in++)
	{
		int b, p;

		p = in->player;
		b = in->type;

		if (b >= IPT_BUTTON1 && b < IPT_BUTTON1 + MAX_NORMAL_BUTTONS)
		{
			b -= IPT_BUTTON1;
			if (b >= buttons)
				buttons = b + 1;
			continue;
		}

		if (b >= IPT_CUSTOM1 && b < IPT_CUSTOM1 + custom_buttons)
		{
			int n = 1;
			char colorbutton = is_neogeo ? 'A' : 'a';
			char *s = buf[total];

			b -= IPT_CUSTOM1;
			custom_item[total] = &custom_button[p][b];

			for (i = 0; i < MAX_NORMAL_BUTTONS; i++, n <<= 1)
				if (*custom_item[total] & n)
				{
					if (s != buf[total])
					{
						*s++ = '_';
						*s++ = '+';
					}

					*s++ = '_';
					*s++ = colorbutton + i;
				}

			*s = '\0';
			convert_command_move(buf[total]);

			menu_item[total] = _(input_port_name(in));
			menu_subitem[total] = buf[total];

			total++;
		}
	}

	menu_item[total] = ui_getstring(UI_returntomain); menu_subitem[total++] = 0;
	menu_item[total] = 0;	/* terminate array */

#ifdef UI_TITLE_MENU
	ui_displaymenu(bitmap,menu_item,menu_subitem,0,ui_getstring(UI_custombuttons),sel,0);
#else
	ui_displaymenu(bitmap, menu_item,menu_subitem,0,sel,0);
#endif /* UI_TITLE_MENU */

	if (input_ui_pressed_repeat(IPT_UI_DOWN,4))
		sel = (sel + 1) % total;

	if (input_ui_pressed_repeat(IPT_UI_UP,4))
		sel = (sel + total - 1) % total;

	if (sel != total - 1)
		for (i = 0; i < buttons; i++)
		{
			int keycode = KEYCODE_1 + i;

			if (i == 9)
				keycode = KEYCODE_0;

			if (code_pressed_memory(keycode))
				*custom_item[sel] ^= 1 << i;
		}

	if (input_ui_pressed(IPT_UI_SELECT))
		if (sel == total - 1)
			sel = -1;

	if (input_ui_pressed(IPT_UI_CANCEL))
		sel = -1;

	if (input_ui_pressed(IPT_UI_CONFIGURE))
		sel = -2;

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();
	}

	return sel + 1;
}
#endif /* USE_CUSTOM_BUTTON */


static int setautofiresettings(struct mame_bitmap *bitmap,int selected)
{
	const char *menu_item[400];
	const char *menu_subitem[400];
	struct InputPort *entry[400];
	struct InputPort *in;
	char delay[4];
	int sel;
	int total;
	int autofire_delay;
	int value = 0;
	int changed = 0;

	sel = selected - 1;

	if (sel < 0) sel = 0;

	total = 0;

	if (Machine->input_ports)
	{
		in = Machine->input_ports;

		while (in->type != IPT_END)
		{
			int type = in->type;

			if (input_port_name(in)	&& (
			    (type >= IPT_BUTTON1 && type < IPT_BUTTON1 + MAX_NORMAL_BUTTONS)
#ifdef USE_CUSTOM_BUTTON
			    || (type >= IPT_CUSTOM1 && type < IPT_CUSTOM1 + MAX_CUSTOM_BUTTONS)
#endif /* USE_CUSTOM_BUTTON */
			   ))
			{
				int v;

				entry[total] = in;

				v = in->autofire_setting;
				menu_item[total] = _(input_port_name(in));
				menu_subitem[total] = ui_getstring(UI_autofireoff + v);
				if (total == sel)
					value = v;
				total++;
			}
			in++;
		}
	}

	if (total == 0)
		return 0;

	sprintf(delay, "%d", options.autofiredelay);
	autofire_delay = total;

	menu_item[total] = ui_getstring(UI_autofiredelay);
	menu_subitem[total++] = delay;
	menu_item[total] = ui_getstring (UI_returntomain);
	menu_subitem[total++] = 0;
	menu_item[total] = 0;

#ifdef UI_TITLE_MENU
	ui_displaymenu(bitmap,menu_item,menu_subitem,0,ui_getstring(UI_autofire),sel,0);
#else
	ui_displaymenu(bitmap,menu_item,menu_subitem,0,sel,0);
#endif /* UI_TITLE_MENU */

	if (sel == autofire_delay)
	{
		if (input_ui_pressed_repeat(IPT_UI_RIGHT,8))
		{
			options.autofiredelay++;
			if (options.autofiredelay > 99)
				options.autofiredelay = 99;
			changed = 1;
		}
		if (input_ui_pressed_repeat(IPT_UI_LEFT,8))
		{
			options.autofiredelay--;
			if (options.autofiredelay < 1)
				options.autofiredelay = 1;
			changed = 1;
		}
	}
	else if (sel < autofire_delay)
	{
		if (input_ui_pressed_repeat(IPT_UI_RIGHT,8))
		{
			if (++value > 2) value = 0;
			changed = 1;
		}
		if (input_ui_pressed_repeat(IPT_UI_LEFT,8))
		{
			if (--value < 0) value = 2;
			changed = 1;
		}
		entry[sel]->autofire_setting = value;
	}

	if (input_ui_pressed_repeat(IPT_UI_DOWN,8))
		sel = (sel + 1) % total;

	if (input_ui_pressed_repeat(IPT_UI_UP,8))
		sel = (sel + total - 1) % total;

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		if (sel == total - 1) sel = -1;
		else sel = 0;
	}

	if (input_ui_pressed(IPT_UI_CANCEL))
		sel = -1;

	if (input_ui_pressed(IPT_UI_CONFIGURE))
		sel = -2;

	if (sel == -1 || sel == -2 || changed)
		schedule_full_refresh();

	return sel + 1;
}

static int calibratejoysticks(struct mame_bitmap *bitmap,int selected)
{
	const char *msg;
	static char buf[2048];
	int sel;
	static int calibration_started = 0;

	sel = selected - 1;

	if (calibration_started == 0)
	{
		osd_joystick_start_calibration();
		calibration_started = 1;
		strcpy (buf, "");
	}

	if (sel > SEL_MASK) /* Waiting for the user to acknowledge joystick movement */
	{
		if (input_ui_pressed(IPT_UI_CANCEL))
		{
			calibration_started = 0;
			sel = -1;
		}
		else if (input_ui_pressed(IPT_UI_SELECT))
		{
			osd_joystick_calibrate();
			sel &= SEL_MASK;
		}

#ifdef UI_TITLE_MENU
		ui_displaymessagewindow(bitmap,buf,0);
#else
		ui_displaymessagewindow(bitmap,buf);
#endif /* UI_TITLE_MENU */		
	}
	else
	{
		msg = osd_joystick_calibrate_next();
		schedule_full_refresh();
		if (msg == 0)
		{
			calibration_started = 0;
			osd_joystick_end_calibration();
			sel = -1;
		}
		else
		{
			strcpy (buf, msg);
#ifdef UI_TITLE_MENU
			ui_displaymessagewindow(bitmap,buf,0);
#else
			ui_displaymessagewindow(bitmap,buf);
#endif /* UI_TITLE_MENU */
			sel |= 1 << SEL_BITS;
		}
	}

	if (input_ui_pressed(IPT_UI_CONFIGURE))
		sel = -2;

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();
	}

	return sel + 1;
}

#define MAX_ANALOG_ENTRIES 80

static int settraksettings(struct mame_bitmap *bitmap,int selected)
{
	const char *menu_item[MAX_ANALOG_ENTRIES];
	const char *menu_subitem[MAX_ANALOG_ENTRIES];
	struct InputPort *entry[MAX_ANALOG_ENTRIES];
	int i,sel;
	struct InputPort *in;
	int total,total2;
	int arrowize;


	sel = selected - 1;


	if (Machine->input_ports == 0)
		return 0;

	in = Machine->input_ports;

	/* Count the total number of analog controls */
	total = 0;
	while (in->type != IPT_END)
	{
		if (port_type_is_analog(in->type))
		{
			entry[total] = in;
			total++;
		}
		in++;
	}

	if (total == 0) return 0;

	/* Each analog control has 4 entries - key & joy delta, center delta, reverse, sensitivity */

#define ENTRIES 4

	total2 = total * ENTRIES;

	menu_item[total2] = ui_getstring (UI_returntomain);
	menu_item[total2 + 1] = 0;	/* terminate array */
	total2++;

	arrowize = 0;
	for (i = 0;i < total2;i++)
	{
		if (i < total2 - 1)
		{
			char label[MAX_ANALOG_ENTRIES][40];
			char setting[MAX_ANALOG_ENTRIES][40];
			int sensitivity,delta,centerdelta;
			int reverse;

			strcpy (label[i], _(input_port_name(entry[i/ENTRIES])));
			sensitivity = entry[i/ENTRIES]->analog.sensitivity;
			delta = entry[i/ENTRIES]->analog.delta;
			centerdelta = entry[i/ENTRIES]->analog.centerdelta;
			reverse = entry[i/ENTRIES]->analog.reverse;

			strcat (label[i], " ");
			switch (i%ENTRIES)
			{
				case 0:
					strcat (label[i], ui_getstring (UI_keyjoyspeed));
					sprintf(setting[i],"%d",delta);
					if (i == sel) arrowize = 3;
					break;
				case 1:
					strcat (label[i], ui_getstring (UI_centerspeed));
					sprintf(setting[i],"%d",centerdelta);
					if (i == sel) arrowize = 3;
					break;
				case 2:
					strcat (label[i], ui_getstring (UI_reverse));
					if (reverse)
						strcpy(setting[i],ui_getstring (UI_on));
					else
						strcpy(setting[i],ui_getstring (UI_off));
					if (i == sel) arrowize = 3;
					break;
				case 3:
					strcat (label[i], ui_getstring (UI_sensitivity));
					sprintf(setting[i],"%3d%%",sensitivity);
					if (i == sel) arrowize = 3;
					break;
			}

			menu_item[i] = label[i];
			menu_subitem[i] = setting[i];

			in++;
		}
		else menu_subitem[i] = 0;	/* no subitem */
	}

#ifdef UI_TITLE_MENU
	ui_displaymenu(bitmap,menu_item,menu_subitem,0,ui_getstring(UI_analogcontrols),sel,arrowize);
#else
	ui_displaymenu(bitmap,menu_item,menu_subitem,0,sel,arrowize);
#endif /* UI_TITLE_MENU */

	if (input_ui_pressed_repeat(IPT_UI_DOWN,8))
		sel = (sel + 1) % total2;

	if (input_ui_pressed_repeat(IPT_UI_UP,8))
		sel = (sel + total2 - 1) % total2;

	if (input_ui_pressed_repeat(IPT_UI_LEFT,8))
	{
		if(sel != total2 - 1)
		{
			if ((sel % ENTRIES) == 0)
			{
				/* keyboard/joystick delta */
				int val = entry[sel/ENTRIES]->analog.delta;

				val --;
				if (val < 1) val = 1;
				entry[sel/ENTRIES]->analog.delta = val;
			}
			else if ((sel % ENTRIES) == 1)
			{
				/* center delta */
				int val = entry[sel/ENTRIES]->analog.centerdelta;

				val --;
				if (val < 0) val = 0;
				entry[sel/ENTRIES]->analog.centerdelta = val;
			}
			else if ((sel % ENTRIES) == 2)
			{
				/* reverse */
				entry[sel/ENTRIES]->analog.reverse ^= 1;
			}
			else if ((sel % ENTRIES) == 3)
			{
				/* sensitivity */
				int val = entry[sel/ENTRIES]->analog.sensitivity;

				val --;
				if (val < 1) val = 1;
				entry[sel/ENTRIES]->analog.sensitivity = val;
			}
		}
	}

	if (input_ui_pressed_repeat(IPT_UI_RIGHT,8))
	{
		if(sel != total2 - 1)
		{
			if ((sel % ENTRIES) == 0)
			{
				/* keyboard/joystick delta */
				int val = entry[sel/ENTRIES]->analog.delta;

				val ++;
				if (val > 255) val = 255;
				entry[sel/ENTRIES]->analog.delta = val;
			}
			else if ((sel % ENTRIES) == 1)
			{
				/* center delta */
				int val = entry[sel/ENTRIES]->analog.centerdelta;

				val ++;
				if (val > 255) val = 255;
				entry[sel/ENTRIES]->analog.centerdelta = val;
			}
			else if ((sel % ENTRIES) == 2)
			{
				/* reverse */
				entry[sel/ENTRIES]->analog.reverse ^= 1;
			}
			else if ((sel % ENTRIES) == 3)
			{
				/* sensitivity */
				int val = entry[sel/ENTRIES]->analog.sensitivity;

				val ++;
				if (val > 255) val = 255;
				entry[sel/ENTRIES]->analog.sensitivity = val;
			}
		}
	}

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		if (sel == total2 - 1) sel = -1;
	}

	if (input_ui_pressed(IPT_UI_CANCEL))
		sel = -1;

	if (input_ui_pressed(IPT_UI_CONFIGURE))
		sel = -2;

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();
	}

	return sel + 1;
}

#ifndef MESS
static int mame_stats(struct mame_bitmap *bitmap,int selected)
{
	char temp[10];
	char buf[2048];
	int sel, i;


	sel = selected - 1;

	buf[0] = 0;

	if (dispensed_tickets)
	{
		strcat(buf, ui_getstring (UI_tickets));
		strcat(buf, ": ");
		sprintf(temp, "%d\n\n", dispensed_tickets);
		strcat(buf, temp);
	}

	for (i=0; i<COIN_COUNTERS; i++)
	{
		strcat(buf, ui_getstring (UI_coin));
		sprintf(temp, " %c: ", i+'A');
		strcat(buf, temp);
		if (!coin_count[i])
			strcat (buf, ui_getstring (UI_NA));
		else
		{
			sprintf (temp, "%d", coin_count[i]);
			strcat (buf, temp);
		}
		if (coinlockedout[i])
		{
			strcat(buf, " ");
			strcat(buf, ui_getstring (UI_locked));
			strcat(buf, "\n");
		}
		else
		{
			strcat(buf, "\n");
		}
	}

	{
		/* menu system, use the normal menu keys */
		strcat(buf,"\n\t");
		strcat(buf,ui_getstring (UI_lefthilight));
		strcat(buf," ");
		strcat(buf,ui_getstring (UI_returntoprior));
		strcat(buf," ");
		strcat(buf,ui_getstring (UI_righthilight));

#ifdef UI_TITLE_MENU
		ui_displaymessagewindow(bitmap,buf,ui_getstring(UI_bookkeeping));
#else
		ui_displaymessagewindow(bitmap,buf);
#endif /* UI_TITLE_MENU */

		if (input_ui_pressed(IPT_UI_SELECT))
			sel = -1;

		if (input_ui_pressed(IPT_UI_CANCEL))
			sel = -1;

		if (input_ui_pressed(IPT_UI_CONFIGURE))
			sel = -2;
	}

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();
	}

	return sel + 1;
}
#endif

int show_font_warning(struct mame_bitmap *bitmap)
{
	/* DO NOT tlanslate this message */
	static const char *font_warning_string =
		"Local font file is not installed. "
		"You must install CJK font into font directory first.\n\n"
		"Please download from:\n"
		"http://mameplus.yeah.net/\n\n"
		"Press ESC to exit, type OK to continue.";
#ifdef UI_COLOR_DISPLAY
	int back = Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND];
#endif /* UI_COLOR_DISPLAY */
	int done;

	setup_selected = -1;////
	done = 0;

	do
	{
#ifdef UI_COLOR_DISPLAY
		Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND] = Machine->uifont->colortable[FONT_COLOR_BLANK];
#endif /* UI_COLOR_DISPLAY */
		erase_screen(bitmap);
#ifdef TRANS_UI
		draw_transparent_box = 0;
		ui_drawbox(bitmap,0,0,uirotwidth,uirotheight); // ?
		draw_transparent_box = 1;
#else /* TRANS_UI */
		ui_drawbox(bitmap,0,0,uirotwidth,uirotheight);
#endif /* TRANS_UI */
#ifdef UI_COLOR_DISPLAY
		Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND] = back;
#endif /* UI_COLOR_DISPLAY */

#ifdef UI_TITLE_MENU
		ui_displaymessagewindow(bitmap,font_warning_string,0);
#else
		ui_displaymessagewindow(bitmap,font_warning_string);
#endif /* UI_TITLE_MENU */

		update_video_and_audio();
		if (input_ui_pressed(IPT_UI_CANCEL))
		{
			setup_selected = 0;////
			return 1;
		}
		if (code_pressed_memory(KEYCODE_O) ||
				input_ui_pressed(IPT_UI_LEFT))
			done = 1;
		if (done == 1 && (code_pressed_memory(KEYCODE_K) ||
				input_ui_pressed(IPT_UI_RIGHT)))
			done = 2;
	} while (done < 2);

	setup_selected = 0;////
	erase_screen(bitmap);
	update_video_and_audio();

	return 0;
}

int showcopyright(struct mame_bitmap *bitmap)
{
#ifdef UI_COLOR_DISPLAY
	int back = Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND];
#endif /* UI_COLOR_DISPLAY */
	int done;
	char buf[1000];
	char buf2[256];

	strcpy (buf, ui_getstring(UI_copyright1));
	strcat (buf, "\n\n");
	sprintf(buf2, ui_getstring(UI_copyright2), (options.use_lang_list ? _LST(Machine->gamedrv->description) : Machine->gamedrv->description));
	strcat (buf, buf2);
	strcat (buf, "\n\n");
	strcat (buf, ui_getstring(UI_copyright3));

	setup_selected = -1;////
	done = 0;

	do
	{
#ifdef UI_COLOR_DISPLAY
		Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND] = Machine->uifont->colortable[FONT_COLOR_BLANK];
#endif /* UI_COLOR_DISPLAY */
		erase_screen(bitmap);
#ifdef TRANS_UI
		draw_transparent_box = 0;
		ui_drawbox(bitmap,0,0,uirotwidth,uirotheight);
		draw_transparent_box = 1;
#else /* TRANS_UI */
		ui_drawbox(bitmap,0,0,uirotwidth,uirotheight);
#endif /* TRANS_UI */
#ifdef UI_COLOR_DISPLAY
		Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND] = back;
#endif /* UI_COLOR_DISPLAY */

#ifdef UI_TITLE_MENU
		ui_displaymessagewindow(bitmap,buf,ui_getstring(UI_copyright));
#else
		ui_displaymessagewindow(bitmap,buf);
#endif /* UI_TITLE_MENU */

		update_video_and_audio();
		if (input_ui_pressed(IPT_UI_CANCEL))
		{
			setup_selected = 0;////
			return 1;
		}
		if (code_pressed_memory(KEYCODE_O) ||
				input_ui_pressed(IPT_UI_LEFT))
			done = 1;
		if (done == 1 && (code_pressed_memory(KEYCODE_K) ||
				input_ui_pressed(IPT_UI_RIGHT)))
			done = 2;
	} while (done < 2);

	setup_selected = 0;////
	erase_screen(bitmap);
	update_video_and_audio();

	return 0;
}

static int displaygameinfo(struct mame_bitmap *bitmap,int selected)
{
	int res;
	int i;
	char buf[4096];
	char buf2[256];
	int sel;


	sel = selected - 1;


	sprintf(buf,"%s\n%s %s\n\n%s:\n",
		options.use_lang_list ? _LST(Machine->gamedrv->description) : Machine->gamedrv->description,
		Machine->gamedrv->year,
		options.use_lang_list ? _MANUFACT(Machine->gamedrv->manufacturer) : Machine->gamedrv->manufacturer,
		ui_getstring (UI_cpu));
	i = 0;
	while (i < MAX_CPU && Machine->drv->cpu[i].cpu_type)
	{
		int type,clock,count;

		type = Machine->drv->cpu[i].cpu_type;
		clock = Machine->drv->cpu[i].cpu_clock;
		count = 1;

		i++;

		while (i < MAX_CPU
				&& Machine->drv->cpu[i].cpu_type == type
				&& Machine->drv->cpu[i].cpu_clock == clock)
		{
			count++;
			i++;
		}

		if (count > 1)
			sprintf(&buf[strlen(buf)],"%dx",count);

		sprintf(&buf[strlen(buf)],"%s",cputype_name(type));

		if (clock >= 1000000)
			sprintf(&buf[strlen(buf)]," %d.%06d MHz",
					clock / 1000000,
					clock % 1000000);
		else
			sprintf(&buf[strlen(buf)]," %d.%03d kHz",
					clock / 1000,
					clock % 1000);

		strcat(buf,"\n");
	}

	sprintf (buf2, "\n%s", ui_getstring (UI_sound));
	strcat (buf, buf2);
	strcat(buf,":\n");

	i = 0;
	while (i < MAX_SOUND && Machine->drv->sound[i].sound_type)
	{
		const char *name;
		int type,clock,count;

		name = sndnum_name(i);
		type = Machine->drv->sound[i].sound_type;
		clock = sndnum_clock(i);
		count = 1;

		i++;

		while (i < MAX_SOUND
				&& Machine->drv->sound[i].sound_type == type
				&& sndnum_clock(i) == clock)
		{
			count++;
			i++;
		}

		if (count > 1)
			sprintf(&buf[strlen(buf)],"%dx",count);

		sprintf(&buf[strlen(buf)],"%s",name);

		if (clock)
		{
			if (clock >= 1000000)
				sprintf(&buf[strlen(buf)]," %d.%06d MHz",
						clock / 1000000,
						clock % 1000000);
			else
				sprintf(&buf[strlen(buf)]," %d.%03d kHz",
						clock / 1000,
						clock % 1000);
		}

		strcat(buf,"\n");
	}

	if (Machine->drv->video_attributes & VIDEO_TYPE_VECTOR)
		sprintf(&buf[strlen(buf)],"\n%s\n", ui_getstring (UI_vectorgame));
	else
	{
		sprintf(&buf[strlen(buf)],"\n%s:\n", ui_getstring (UI_screenres));
		sprintf(&buf[strlen(buf)],"%d x %d (%s) %f Hz\n",
				Machine->visible_area.max_x - Machine->visible_area.min_x + 1,
				Machine->visible_area.max_y - Machine->visible_area.min_y + 1,
				(Machine->gamedrv->flags & ORIENTATION_SWAP_XY) ? "V" : "H",
				Machine->refresh_rate);
		sprintf(&buf[strlen(buf)],_("Palettesize: %d\n"),Machine->drv->total_colors);
	}


	if (sel == -1)
	{
#ifdef UI_COLOR_DISPLAY
		int back = Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND];

		Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND] = Machine->uifont->colortable[FONT_COLOR_BLANK];
#endif /* UI_COLOR_DISPLAY */
		/* startup info, print MAME version and ask for any key */

		sprintf (buf2, "\n\t%s ", ui_getstring (UI_mame));	/* \t means that the line will be centered */
		strcat(buf, buf2);

		strcat(buf,build_version);
		sprintf (buf2, "\n\t%s", ui_getstring (UI_selectkey));
		strcat(buf,buf2);
#ifdef TRANS_UI
		draw_transparent_box = 0;
		ui_drawbox(bitmap,0,0,uirotwidth,uirotheight);
		draw_transparent_box = 1;
#else /* TRANS_UI */
		ui_drawbox(bitmap,0,0,uirotwidth,uirotheight);
#endif /* TRANS_UI */

#ifdef UI_COLOR_DISPLAY
		Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND] = back;
#endif /* UI_COLOR_DISPLAY */

		sel = 0;

#ifdef UI_TITLE_MENU
		res = ui_displaymessagewindow(bitmap,buf,ui_getstring(UI_gameinfo));
#else
		res = ui_displaymessagewindow(bitmap,buf);
#endif /* UI_TITLE_MENU */

		if (res >= 0)
		{
			if (res == 1 || input_ui_pressed(IPT_UI_SELECT))
				sel = -1;

			if (res == 2 || input_ui_pressed(IPT_UI_CANCEL))
			sel = -1;

			if (res == 3 || input_ui_pressed(IPT_UI_CONFIGURE))
				sel = -2;
		}
	}
	else
	{
		/* menu system, use the normal menu keys */
		strcat(buf,"\n\t");
		strcat(buf,ui_getstring (UI_lefthilight));
		strcat(buf," ");
		strcat(buf,ui_getstring (UI_returntoprior));
		strcat(buf," ");
		strcat(buf,ui_getstring (UI_righthilight));

#ifdef UI_TITLE_MENU
		res = ui_displaymessagewindow(bitmap,buf,ui_getstring(UI_gameinfo));
#else
		res = ui_displaymessagewindow(bitmap,buf);
#endif /* UI_TITLE_MENU */

		if (res >= 0)
		{
			if (res == 1 || input_ui_pressed(IPT_UI_SELECT))
			sel = -1;

			if (res == 2 || input_ui_pressed(IPT_UI_CANCEL))
			sel = -1;

			if (res == 3 || input_ui_pressed(IPT_UI_CONFIGURE))
			sel = -2;
		}
	}

	if (sel == -1 || sel == -2)
	{
		scroll_reset = 1;
		schedule_full_refresh();
	}

	return sel + 1;
}

#if 0
int showgamewarnings(struct mame_bitmap *bitmap)
{
	int i;
	char buf[4096];

	if (Machine->gamedrv->flags &
			(GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS |
#ifdef GAME_UNSIMULATED_HARDWARE
			 GAME_UNSIMULATED_HARDWARE |
#endif /* GAME_UNSIMULATED_HARDWARE */
			 GAME_NO_SOUND | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL))
	{
		int res;
#ifdef UI_COLOR_DISPLAY
		int back = Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND];
#endif /* UI_COLOR_DISPLAY */
		int done;

		strcpy(buf, ui_getstring (UI_knownproblems));
		strcat(buf, "\n\n");

#ifdef MESS
		if (Machine->gamedrv->flags & GAME_COMPUTER)
		{
			strcpy(buf, ui_getstring (UI_comp1));
			strcat(buf, "\n\n");
			strcat(buf, ui_getstring (UI_comp2));
			strcat(buf, "\n");
		}
#endif

		if (Machine->gamedrv->flags & GAME_IMPERFECT_COLORS)
		{
			strcat(buf, ui_getstring (UI_imperfectcolors));
			strcat(buf, "\n");
		}

		if (Machine->gamedrv->flags & GAME_WRONG_COLORS)
		{
			strcat(buf, ui_getstring (UI_wrongcolors));
			strcat(buf, "\n");
		}

		if (Machine->gamedrv->flags & GAME_IMPERFECT_GRAPHICS)
		{
			strcat(buf, ui_getstring (UI_imperfectgraphics));
			strcat(buf, "\n");
		}

		if (Machine->gamedrv->flags & GAME_IMPERFECT_SOUND)
		{
			strcat(buf, ui_getstring (UI_imperfectsound));
			strcat(buf, "\n");
		}

		if (Machine->gamedrv->flags & GAME_NO_SOUND)
		{
			strcat(buf, ui_getstring (UI_nosound));
			strcat(buf, "\n");
		}

		if (Machine->gamedrv->flags & GAME_NO_COCKTAIL)
		{
			strcat(buf, ui_getstring (UI_nococktail));
			strcat(buf, "\n");
		}

#ifdef GAME_UNSIMULATED_HARDWARE
		if (Machine->gamedrv->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_UNSIMULATED_HARDWARE))
#else /* GAME_UNSIMULATED_HARDWARE */
		if (Machine->gamedrv->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION))
#endif /* GAME_UNSIMULATED_HARDWARE */
		{
			const struct GameDriver *maindrv;
			int foundworking;

			if (Machine->gamedrv->flags & GAME_NOT_WORKING)
			{
				strcpy(buf, ui_getstring (UI_brokengame));
				strcat(buf, "\n");
			}
			if (Machine->gamedrv->flags & GAME_UNEMULATED_PROTECTION)
			{
				strcat(buf, ui_getstring (UI_brokenprotection));
				strcat(buf, "\n");
			}

#ifdef GAME_UNSIMULATED_HARDWARE
			if (Machine->gamedrv->flags & GAME_UNSIMULATED_HARDWARE)
			{
				strcat(buf, ui_getstring (UI_unsimulatedhardware));
				strcat(buf, "\n");
			}
#endif /* GAME_UNSIMULATED_HARDWARE */

			if (Machine->gamedrv->clone_of && !(Machine->gamedrv->clone_of->flags & NOT_A_DRIVER))
				maindrv = Machine->gamedrv->clone_of;
			else maindrv = Machine->gamedrv;

			foundworking = 0;
			i = 0;
			while (drivers[i])
			{
				if (drivers[i] == maindrv || drivers[i]->clone_of == maindrv)
				{
#ifdef GAME_UNSIMULATED_HARDWARE
					if ((drivers[i]->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_UNSIMULATED_HARDWARE)) == 0)
#else /* GAME_UNSIMULATED_HARDWARE */
					if ((drivers[i]->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) == 0)
#endif /* GAME_UNSIMULATED_HARDWARE */
					{
						if (foundworking == 0)
						{
							strcat(buf,"\n\n");
							strcat(buf, ui_getstring (UI_workingclones));
							strcat(buf,"\n\n");
						}
						foundworking = 1;

						sprintf(&buf[strlen(buf)],"%s\n",drivers[i]->name);
					}
				}
				i++;
			}
		}

		strcat(buf,"\n\n");
		strcat(buf,ui_getstring (UI_typeok));

		done = 0;
		res = 0;
		do
		{
#ifdef UI_COLOR_DISPLAY
			Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND] = Machine->uifont->colortable[FONT_COLOR_BLANK];
#endif /* UI_COLOR_DISPLAY */
			erase_screen(bitmap);
#ifdef TRANS_UI
			draw_transparent_box = 0;
			ui_drawbox(bitmap,0,0,uirotwidth,uirotheight);
			draw_transparent_box = 1;
#else /* TRANS_UI */
			ui_drawbox(bitmap,0,0,uirotwidth,uirotheight);
#endif /* TRANS_UI */
#ifdef UI_COLOR_DISPLAY
			Machine->uifont->colortable[SYSTEM_COLOR_BACKGROUND] = back;
#endif /* UI_COLOR_DISPLAY */
	
#ifdef UI_TITLE_MENU
			res = ui_displaymessagewindow(bitmap,buf,0);
#else	
			res = ui_displaymessagewindow(bitmap,buf);
#endif /* UI_TITLE_MENU */
			
			update_video_and_audio();
			if (res == 2 || input_ui_pressed(IPT_UI_CANCEL)) // canceled
			{
				return 1;
			}
			else if (res >= 0)
			{
				if (code_pressed_memory(KEYCODE_O)
				||  input_ui_pressed(IPT_UI_LEFT))
				done = 1;
				if (done == 1 && (code_pressed_memory(KEYCODE_K)
				||  input_ui_pressed(IPT_UI_RIGHT)))
				done = 2;
			}
		} while (done < 2);

		scroll_reset = 1;
	}

	erase_screen(bitmap);
	update_video_and_audio();

	return 0;
}
#endif

int showgameinfo(struct mame_bitmap *bitmap)
{
	/* clear the input memory */
	while (code_read_async() != CODE_NONE) {};

	while (displaygameinfo(bitmap,0) == 1)
	{
		update_video_and_audio();
	}

	#ifdef MESS
	while (displayimageinfo(bitmap,0) == 1)
	{
		update_video_and_audio();
	}
	#endif

	/* clear the input memory */
	while (code_read_async() != CODE_NONE) {};

	erase_screen(bitmap);
	/* make sure that the screen is really cleared, in case autoframeskip kicked in */
	update_video_and_audio();
	update_video_and_audio();
	update_video_and_audio();
	update_video_and_audio();

	return 0;
}

/* Word-wraps the text in the specified buffer to fit in maxwidth characters per line.
   The contents of the buffer are modified.
   Known limitations: Words longer than maxwidth cause the function to fail. */
static int wordwrap_text_buffer(char *buffer, int maxwidth)
{
	char *c, *c2;
	char textcopy[256 * 1024 + 256];
	unsigned short code;
	int i, increment, wrapped;
	int len, maxlen;
	char *wrap_c = NULL;
	char *wrap_c2 = NULL;
	int wrap_w;
	int wrap_len = 0;
	int last_is_space = 0;

	if (maxwidth <= 0)
		{
		*buffer = '\0';
		return 0;
		}

	/* copy text, calculate max len, wrap long lines to fit */
	len    = 0;
	maxlen = 0;
	wrapped = i = 0;
	c = buffer;
	c2 = textcopy;
	memset(textcopy, 0, sizeof textcopy);

	while (*c)
		{
		if (wrapped)
			{
			last_is_space = 0;
			wrap_w = 0;
			}
		else
			wrap_w = i;

		wrapped = 0;
		increment = uifont_decodechar((unsigned char *)c, &code);

		if (*c == '\n')
		{
			*c2++ = *c++;
			wrapped = 1;
		}
		else
		{
			i = (code < 256) ? 1 : 2;
			if (i > 1 || wrap_w > 1)
				wrap_c = NULL;

			if (wrap_c == NULL)
			{
				wrap_len = len;
				wrap_c = c;
				wrap_c2 = c2;
			}
			else if (len > 0 && *c == ' ')
			{
				if (!last_is_space)
				{
					wrap_c2 = c2;
					wrap_len = len;
				}
				wrap_c = c;
	}

			last_is_space = (*c == ' ');

			if (len + i > maxwidth)
			{
				if (!wrap_len || !wrap_c)
				{
					wrap_len = len;
					wrap_c = c;
					wrap_c2 = c2;
				}

				len = wrap_len;
				c = wrap_c;
				c2 = wrap_c2;

				while (*c == ' ')
					c++;

				*c2++ = '\n';
				wrapped = 1;
			}
			else
			{
				len += i;

				while (increment)
				{
					*c2++ = *c++;
					increment--;
				}
			}
		}

		if (wrapped)
		{
			if (len > maxlen)
				maxlen = len;

			len = 0;
			wrap_c = NULL;
			wrap_w = 0;
		}
	}

	if (len > maxlen)
		maxlen = len;

	*c2 = '\0';

	strcpy(buffer, textcopy);

	return maxlen;
}

static int count_lines_in_buffer (char *buffer)
{
	int lines = 0;
	char c;

	while ( (c = *buffer++) )
		if (c == '\n') lines++;

	return lines;
}

/* Display lines from buffer, starting with line 'scroll', in a width x height text window */
#ifdef CMD_LIST
#ifdef UI_TITLE_MENU
static int display_scroll_message (struct mame_bitmap *bitmap, int *scroll, int width, int height, char *buf, const char *title)
#else
static int display_scroll_message (struct mame_bitmap *bitmap, int *scroll, int width, int height, char *buf)
#endif /* UI_TITLE_MENU */
#else /* CMD_LIST */
static void display_scroll_message (struct mame_bitmap *bitmap, int *scroll, int width, int height, char *buf)
#endif /* CMD_LIST */
{
	struct DisplayText dt[256];
	int curr_dt = 0;
	const char *uparrow = ui_getstring (UI_uparrow);
	const char *downarrow = ui_getstring (UI_downarrow);
	char textcopy[8192];
	char *copy;
	int leftoffs,topoffs;
	int first = *scroll;
	int buflines,showlines;
	int i;
#ifdef CMD_LIST
	int ret;
#endif /* CMD_LIST */
#ifdef CMD_TITLE
	char is_center[1];
	int title_color;
	int is_bold = UI_COLOR_NORMAL;
#endif /* CMD_TITLE */

	/* draw box */
	leftoffs = (uirotwidth - uirotcharwidth * (width + 1)) / 2;
	if (leftoffs < 0) leftoffs = 0;

#ifdef UI_TITLE_MENU
	if (title)
	{
		topoffs = (uirotheight - ((height + 1) * ROWHEIGHT + 2*DRAWBOX_MARGIN)) / 2;
		ui_drawbox(bitmap,leftoffs,topoffs,(width + 1) * uirotcharwidth,(height + 1) * ROWHEIGHT + DRAWBOX_MARGIN*2);
	}
	else
	{
#endif /* UI_TITLE_MENU */
	topoffs = (uirotheight - (height * ROWHEIGHT + ROWMARGIN)) / 2;
	ui_drawbox(bitmap,leftoffs,topoffs,(width + 1) * uirotcharwidth,height * ROWHEIGHT + ROWMARGIN);
#ifdef UI_TITLE_MENU
	}

	if (title)
	{
		DRAW_TITLEBAR(leftoffs, topoffs + ROWMARGIN, width + 1, 0);

		dt[0].text = title;
		dt[0].color = UI_COLOR_BOLD;
		dt[0].x = (Machine->uiwidth - Machine->uifontwidth * strlen(dt[0].text)) / 2;
		dt[0].y = topoffs+2;

		dt[1].text = 0;	/* terminate array */

		displaytext(bitmap,dt);

		topoffs += ROWHEIGHT + ROWMARGIN*2;
	}
#endif /* UI_TITLE_MENU */

	buflines = count_lines_in_buffer (buf);
	if (first > 0)
	{
		if (buflines <= height)
			first = 0;
		else
		{
			height--;
			if (first > (buflines - height))
				first = buflines - height;
		}
		*scroll = first;
	}

	if (first != 0)
	{
		/* indicate that scrolling upward is possible */
		dt[curr_dt].text = uparrow;
		dt[curr_dt].color = UI_COLOR_NORMAL;
		dt[curr_dt].x = (uirotwidth - uirotcharwidth * strlen(uparrow)) / 2;
		dt[curr_dt].y = topoffs + curr_dt*ROWHEIGHT + ROWMARGIN;
		curr_dt++;
	}

	if ((buflines - first) > height)
		showlines = height - 1;
	else
		showlines = height;

#ifdef CMD_LIST
	ret = buflines;

	if (first)
		ret |= SCR_PREV_PAGE;

	if (showlines == (height - 1))
		ret |= SCR_NEXT_PAGE;
#endif /* CMD_LIST */

	/* skip to first line */
	while (first > 0)
	{
		char c;

		while ( (c = *buf++) )
		{
			if (c == '\n')
			{
				first--;
				break;
			}
		}
	}

	/* copy 'showlines' lines from buffer, starting with line 'first' */
	copy = textcopy;
	for (i = 0; i < showlines; i++)
	{
		char *copystart = copy;

		while (*buf && *buf != '\n')
		{
			*copy = *buf;
			copy++;
			buf++;
		}
		*copy = '\0';
		copy++;
		if (*buf == '\n')
			buf++;

#ifdef CMD_TITLE
		if (*copystart == '<' && (*(copystart+1) == '/' || *(copystart+2) == '/') && *(copystart+3) == '>')
		{
			copystart +=4;

			is_bold = UI_COLOR_BOLD;

			if (*(copystart-3) == '/')
			{
				*is_center = *(copystart-2);
				dt[curr_dt].x = (uirotwidth - uirotcharwidth * (copy - copystart - 1)) / 2;
			}
			else
			{
				*is_center = *(copystart-3);
				dt[curr_dt].x = leftoffs + uirotcharwidth/2;
			}

			switch(*is_center)
			{
				case '1': // TITLE_COLOR_WHITE_1
				case '2': // TITLE_COLOR_WHITE_2
				case '3': // TITLE_COLOR_WHITE_3
				case '4': // TITLE_COLOR_WHITE_4
				case '5': // TITLE_COLOR_WHITE_5
					title_color = (int)(*is_center - '1');
					ui_plotbox(bitmap, leftoffs, topoffs + curr_dt*ROWHEIGHT + 1, (width+1) * uirotcharwidth, ROWHEIGHT - 1, TITLE_COLOR_WHITE_1 + title_color);
					break;

				case '6': // TITLE_COLOR_BLACK_1
				case '7': // TITLE_COLOR_BLACK_2
				case '8': // TITLE_COLOR_BLACK_3
				case '9': // TITLE_COLOR_BLACK_4
					is_bold = UI_COLOR_BLACK;
					title_color = (int)(*is_center - '6');
					ui_plotbox(bitmap, leftoffs, topoffs + curr_dt*ROWHEIGHT + 1, (width+1) * uirotcharwidth, ROWHEIGHT - 1, TITLE_COLOR_BLACK_1 + title_color);
					break;

				case '0': // TITLE_COLOR_BLACK_5
					is_bold = UI_COLOR_BLACK;
					title_color = TITLE_COLOR_BLACK_5;
					ui_plotbox(bitmap, leftoffs, topoffs + curr_dt*ROWHEIGHT + 1, (width+1) * uirotcharwidth, ROWHEIGHT - 1, title_color);
					break;

				case 'c': // Algin Center
				case 'C': // Algin Center
					is_bold = UI_COLOR_NORMAL;
					break;

				case 'b': // Bold Text
				case 'B': // Bold Text
					break;

				default: // NOT WORK
					is_bold = UI_COLOR_NORMAL;
					dt[curr_dt].x = leftoffs + uirotcharwidth/2;
					break;
			}
		}
		else
		if (*copystart == '<' && (*(copystart+1) == '/' || *(copystart+3) == '/') && *(copystart+4) == '>')
		{
			copystart +=5;

			is_bold = UI_COLOR_BOLD;

			if (*(copystart-4) == '/')
			{
				*is_center     = *(copystart-3);
				*(is_center+1) = *(copystart-2);
				dt[curr_dt].x = (uirotwidth - uirotcharwidth * (copy - copystart - 1)) / 2;
			}
			else
			{
				*is_center     = *(copystart-4);
				*(is_center+1) = *(copystart-3);
				dt[curr_dt].x = leftoffs + uirotcharwidth/2;
			}

			switch(*is_center)
			{
				case 'b': // TITLE_COLOR_BLUE, TITLE_COLOR_BLACK
				{
					if (*(is_center+1) == 'l')
						title_color = TITLE_COLOR_BLUE;
					else if (*(is_center+1) == 'k')
						title_color = TITLE_COLOR_BLACK;
					else
						break;
					ui_plotbox(bitmap, leftoffs, topoffs + curr_dt*ROWHEIGHT + 1, (width+1) * uirotcharwidth, ROWHEIGHT - 1, title_color);
					break;
				}
				case 'r': // TITLE_COLOR_RED
				{
					if (*(is_center+1) != 'e')
						break;
					title_color = TITLE_COLOR_RED;
					ui_plotbox(bitmap, leftoffs, topoffs + curr_dt*ROWHEIGHT + 1, (width+1) * uirotcharwidth, ROWHEIGHT - 1, title_color);
					break;
				}
				case 'g': // TITLE_COLOR_GREEN
				{
					if (*(is_center+1) != 'r')
						break;
					title_color = TITLE_COLOR_GREEN;
					ui_plotbox(bitmap, leftoffs, topoffs + curr_dt*ROWHEIGHT + 1, (width+1) * uirotcharwidth, ROWHEIGHT - 1, title_color);
					break;
				}
				case 'y': // TITLE_COLOR_YELLOW
				{
					if (*(is_center+1) != 'e')
						break;
					is_bold = UI_COLOR_BLACK;
					title_color = TITLE_COLOR_YELLOW;
					ui_plotbox(bitmap, leftoffs, topoffs + curr_dt*ROWHEIGHT + 1, (width+1) * uirotcharwidth, ROWHEIGHT - 1, title_color);
					break;
				}
				case 'p': // TITLE_COLOR_PURPLE
				{
					if (*(is_center+1) != 'u')
						break;
					title_color = TITLE_COLOR_PURPLE;
					ui_plotbox(bitmap, leftoffs, topoffs + curr_dt*ROWHEIGHT + 1, (width+1) * uirotcharwidth, ROWHEIGHT - 1, title_color);
					break;
				}
				case 'w': // TITLE_COLOR_WHITE
				{
					if (*(is_center+1) != 'h')
						break;
					is_bold = UI_COLOR_BLACK;
					title_color = TITLE_COLOR_WHITE;
					ui_plotbox(bitmap, leftoffs, topoffs + curr_dt*ROWHEIGHT + 1, (width+1) * uirotcharwidth, ROWHEIGHT - 1, title_color);
					break;
				}

				default: // NOT WORK
					is_bold = UI_COLOR_NORMAL;
					dt[curr_dt].x = leftoffs + uirotcharwidth/2;
					break;
			}
		}
		else
#endif /* CMD_TITLE */

		if (*copystart == '\t') /* center text */
		{
			copystart++;
			dt[curr_dt].x = (uirotwidth - uirotcharwidth * (copy - copystart)) / 2;
		}
		else
			dt[curr_dt].x = leftoffs + uirotcharwidth/2;

		dt[curr_dt].text = copystart;
#ifdef CMD_TITLE
		if (is_bold)
		{
			dt[curr_dt].color = is_bold;
			is_bold = UI_COLOR_NORMAL;
		}
		else
#endif /* CMD_TITLE */	
		dt[curr_dt].color = UI_COLOR_NORMAL;
		dt[curr_dt].y = topoffs + curr_dt*ROWHEIGHT + ROWMARGIN;
		curr_dt++;
	}
 
	if (showlines == (height - 1))
	{
		/* indicate that scrolling downward is possible */
		dt[curr_dt].text = downarrow;
		dt[curr_dt].color = UI_COLOR_NORMAL;
		dt[curr_dt].x = (uirotwidth - uirotcharwidth * strlen(downarrow)) / 2;
		dt[curr_dt].y = topoffs + curr_dt*ROWHEIGHT + ROWMARGIN;
		curr_dt++;
	}

	dt[curr_dt].text = 0;	/* terminate array */

	displaytext(bitmap,dt);

#ifdef CMD_LIST
	return ret;
#endif /* CMD_LIST */
}


#ifdef MASH_DATAFILE
#ifdef UI_TITLE_MENU // ORDER_CHANGE
enum {
	DATAFILE_HISTORY,
	DATAFILE_MAMEINFO,
	DATAFILE_DRIVINFO,
	DATAFILE_STATISTICS
};
#else /* UI_TITLE_MENU */
enum {
	DATAFILE_MAMEINFO,
	DATAFILE_DRIVINFO,
	DATAFILE_HISTORY,
	DATAFILE_STATISTICS
};
#endif /* UI_TITLE_MENU */

/* Display text entry for current driver from history.dat and mameinfo.dat. */
static int displaydatinfo (struct mame_bitmap *bitmap, int selected, int dattype)
#else /* MASH_DATAFILE */
static int displayhistory (struct mame_bitmap *bitmap, int selected)
#endif /* MASH_DATAFILE */
{
	static int scroll = 0;
	static int counter = 0;
	static int fast = 4;
	static char *buf = 0;
	int maxcols,maxrows;
	int sel;
	int bufsize = 256 * 1024; // 256KB of history.dat buffer, enough for everything

	sel = selected - 1;


	maxcols = (uirotwidth / uirotcharwidth) - 1;
	maxrows = (uirotheight / ROWHEIGHT) - 1;
	maxcols -= 1;
	maxrows -= 2;

	if (!buf)
	{
		/* allocate a buffer for the text */
		buf = malloc (bufsize);

		if (buf)
		{
			/* Disable sound to prevent strange sound*/
			osd_sound_enable(0);

			/* try to load entry */
#ifdef MASH_DATAFILE
			if ((dattype == DATAFILE_MAMEINFO   && (load_driver_mameinfo (Machine->gamedrv, buf, bufsize) == 0))
			||  (dattype == DATAFILE_DRIVINFO   && (load_driver_drivinfo (Machine->gamedrv, buf, bufsize) == 0))
			||  (dattype == DATAFILE_HISTORY    && (load_driver_history  (Machine->gamedrv, buf, bufsize) == 0))
			||  (dattype == DATAFILE_STATISTICS && (load_driver_statistics                 (buf, bufsize) == 0)))
#else /* MASH_DATAFILE */
			if (load_driver_history (Machine->gamedrv, buf, bufsize) == 0)
#endif /* MASH_DATAFILE */
			{
				scroll = 0;
				wordwrap_text_buffer (buf, maxcols);
				strcat(buf,"\n\t");
				strcat(buf,ui_getstring (UI_lefthilight));
				strcat(buf," ");
				strcat(buf,ui_getstring (UI_returntoprior));
				strcat(buf," ");
				strcat(buf,ui_getstring (UI_righthilight));
				strcat(buf,"\n");
			}
			else
			{
				free (buf);
				buf = 0;
			}

			osd_sound_enable(1);

		}
	}

	{
		if (buf)
#ifdef UI_TITLE_MENU
#ifdef MASH_DATAFILE
			display_scroll_message (bitmap, &scroll, maxcols, maxrows, buf, ui_getstring(UI_history + dattype));
#else /* MASH_DATAFILE */
			display_scroll_message (bitmap, &scroll, maxcols, maxrows, buf, ui_getstring(UI_history));
#endif /* MASH_DATAFILE */
#else
			display_scroll_message (bitmap, &scroll, maxcols, maxrows, buf);
#endif /* UI_TITLE_MENU */
		else
		{
			char msg[80];

			strcpy(msg,"\t");

			#ifndef MESS
#ifdef MASH_DATAFILE
			switch (dattype) {
			case DATAFILE_MAMEINFO:
				strcat(msg,ui_getstring(UI_mameinfomissing));
				break;
			case DATAFILE_DRIVINFO:
				strcat(msg,ui_getstring(UI_drivinfomissing));
				break;
			case DATAFILE_HISTORY:
				strcat(msg,ui_getstring(UI_historymissing));
				break;
			case DATAFILE_STATISTICS:
				strcat(msg,ui_getstring(UI_statisticsmissing));
				break;
			}
#else /* MASH_DATAFILE */
			strcat(msg,ui_getstring(UI_historymissing));
#endif /* MASH_DATAFILE */
			#else
			strcat(msg,ui_getstring(UI_historymissing));
			#endif

			strcat(msg,"\n\n\t");
			strcat(msg,ui_getstring (UI_lefthilight));
			strcat(msg," ");
			strcat(msg,ui_getstring (UI_returntoprior));
			strcat(msg," ");
			strcat(msg,ui_getstring (UI_righthilight));
#ifdef UI_TITLE_MENU // TITLE_BAR: MESSAGE
#ifdef MASH_DATAFILE
			ui_displaymessagewindow(bitmap,msg,ui_getstring(UI_history + dattype));
#else /* MASH_DATAFILE */
			ui_displaymessagewindow(bitmap,msg,ui_getstring(UI_history));
#endif /* MASH_DATAFILE */
#else
			ui_displaymessagewindow(bitmap,msg);
#endif /* UI_TITLE_MENU */
		}

		if ((scroll > 0) && input_ui_pressed_repeat(IPT_UI_UP,fast))
		{
			if (scroll == 2) scroll = 0;	/* 1 would be the same as 0, but with arrow on top */
			else scroll--;
		}

		if (input_ui_pressed_repeat(IPT_UI_DOWN,fast))
		{
			if (scroll == 0) scroll = 2;	/* 1 would be the same as 0, but with arrow on top */
			else scroll++;
		}

		if (input_ui_pressed_repeat(IPT_UI_PAN_UP, fast))
		{
			scroll -= maxrows - 2;
			if (scroll < 0) scroll = 0;
		}

		if (input_ui_pressed_repeat(IPT_UI_PAN_DOWN, fast))
		{
			scroll += maxrows - 2;
		}

		if (input_port_type_pressed(IPT_UI_UP,0) | input_port_type_pressed(IPT_UI_DOWN,0))
		{
			if (++counter == 25)
			{
				fast--;
				if (fast < 1) fast = 0;
				counter = 0;
			}
		}
		else fast = 4;

		if (input_ui_pressed(IPT_UI_SELECT))
			sel = -1;

		if (input_ui_pressed(IPT_UI_CANCEL))
			sel = -1;

		if (input_ui_pressed(IPT_UI_CONFIGURE))
			sel = -2;
	}

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();

		/* force buffer to be recreated */
		if (buf)
		{
			free (buf);
			buf = 0;
		}
	}

	return sel + 1;

}


static void flash_all_values_and_buffers(void)
{
	setup_selected    = 0;
	osd_selected      = 0;
	single_step       = 0;

	scroll_reset      = 1;

#ifdef CMD_LIST
	if (commandlist_buf)
	{
		free(commandlist_buf);
		commandlist_buf = NULL;
	}
	command_scroll = 0;
	command_lastselected = 0;

	command_sc = 0;
	command_sel = 1;
	command_load = 1;
#endif /* CMD_LIST */

	cheat_sc = 0;
	cheat_sel = 1;
	cheat_fe = 0;
}

#ifdef CMD_LIST
/* Display text entry for current driver from command.dat. */
static int displaycommand(struct mame_bitmap *bitmap, int selected, int shortcut)
{
	int sel, ret = 0, buflines = 0, visible = 0;
	static int maxcols = 0, maxrows = 0;
#ifdef UI_TITLE_MENU
	static const char *items[64];
#endif /* UI_TITLE_MENU */

	sel = selected - 1;

	if (!commandlist_buf)
	{
		maxcols = (uirotwidth / uirotcharwidth) - 2;
		maxrows = (uirotheight / ROWHEIGHT) - 2;

		/* allocate a buffer for the text */
		if ((commandlist_buf = malloc(65536)))
		{
			memset(commandlist_buf, 0, 65536);

			/* try to load entry */
#ifdef UI_TITLE_MENU
			if (!load_driver_command_ex (Machine->gamedrv, commandlist_buf, 65536, sel, items))
#else
			if (!load_driver_command_ex (Machine->gamedrv, commandlist_buf, 65536, sel))
#endif /* UI_TITLE_MENU */
			{
#ifdef UI_TITLE_MENU // CMD_FIX
				int count  = 0;
#endif /* UI_TITLE_MENU */	
				int height = 1;
				static int presel = -1;
				char *c;

				if( presel != sel )
				{
					command_scroll = 0;
					presel = sel;
				}

				convert_command_move(commandlist_buf);

				strcat(commandlist_buf,"\n\t");
				strcat(commandlist_buf,ui_getstring(UI_lefthilight));
				strcat(commandlist_buf," ");
				strcat(commandlist_buf,ui_getstring(UI_returntoprior));
				strcat(commandlist_buf," ");
				strcat(commandlist_buf,ui_getstring(UI_righthilight));
				strcat(commandlist_buf,"\n");

				c = commandlist_buf;

				while(*c)
#ifdef UI_TITLE_MENU
				{
					if (count < 2)
					{
						commandlist_buf++;
						if (*c++ == '\n')
							count++;
					}
					else
#endif /* UI_TITLE_MENU */	
					if (*c++ == '\n') height++;
#ifdef UI_TITLE_MENU
				}
#endif /* UI_TITLE_MENU */

				if (height <= maxrows)
					maxrows = height - 1;
			}
			else
			{
				free(commandlist_buf);
				commandlist_buf = 0;
			}
		}
	}

	if (commandlist_buf)
	{
#ifdef UI_TITLE_MENU
		ret = display_scroll_message(bitmap, &command_scroll, maxcols, maxrows, commandlist_buf, items[selected - 1]);
#else	
		ret = display_scroll_message(bitmap, &command_scroll, maxcols, maxrows, commandlist_buf);
#endif /* UI_TITLE_MENU */

		buflines = ret & SCR_PAGE_MASK;
		visible  = maxrows - ((ret & SCR_PREV_PAGE) != 0) - ((ret & SCR_NEXT_PAGE) != 0);
	}
	else
	{
		char msg[80];

		strcpy(msg, "\t");
		strcat(msg, ui_getstring(UI_commandmissing));
		strcat(msg, "\n\n\t");
		strcat(msg, ui_getstring(UI_lefthilight));
		strcat(msg, " ");
		if (shortcut)
			strcat(msg, ui_getstring(UI_returntogame));
		else
			strcat(msg, ui_getstring(UI_returntoprior));
		strcat(msg, " ");
		strcat(msg, ui_getstring(UI_righthilight));
#ifdef UI_TITLE_MENU
		ui_displaymessagewindow(bitmap, msg, ui_getstring(UI_command));
#else
		ui_displaymessagewindow(bitmap, msg);
#endif /* UI_TITLE_MENU */
	}

	if ((command_scroll > 0) && input_ui_pressed_repeat(IPT_UI_UP,4))
	{
		if (command_scroll == 2) command_scroll = 0;	/* 1 would be the same as 0, but with arrow on top */
		else command_scroll--;
	}

	if (input_ui_pressed_repeat(IPT_UI_DOWN,4))
	{
		if (command_scroll == 0) command_scroll = 2;	/* 1 would be the same as 0, but with arrow on top */
		else command_scroll++;
	}

	if ((ret & SCR_PREV_PAGE) && input_ui_pressed_repeat(IPT_UI_PAN_UP,4))
	{
		if (command_scroll == maxrows - 1)
			command_scroll = 0;
		else if (command_scroll - visible < 0)
			command_scroll = 0;
		else
			command_scroll -= visible;
	}

	if ((ret & SCR_NEXT_PAGE) && input_ui_pressed_repeat(IPT_UI_PAN_DOWN,4))
	{
		if (command_scroll >= buflines - visible)
			command_scroll = buflines - visible;
		else
			command_scroll += visible;
	}

	if (input_ui_pressed(IPT_UI_SELECT))
		sel = -1;

	if (input_ui_pressed(IPT_UI_CANCEL))
		sel = -1;

	if (input_ui_pressed(IPT_UI_CONFIGURE) && shortcut == 0)
		sel = -2;

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();

		/* force buffer to be recreated */
		if (commandlist_buf)
		{
			free(commandlist_buf);
			commandlist_buf = 0;
		}
	}

	return sel + 1;
}


static int displaycommand_ex(struct mame_bitmap *bitmap, int selected, int shortcut)
{
	static const char *menu_item[64];
	static int total = 0;
	int sel, visible;

	sel = selected - 1;
	visible = (uirotheight / ROWHEIGHT) - 3;

	/* If a submenu has been selected, go there */
	if (command_lastselected)
	{
		command_lastselected = displaycommand(bitmap, (sel + 1), shortcut);

		if (command_lastselected == -1)
		{
			sel = -2;
			command_lastselected = 1;
			schedule_full_refresh();
		}

		return sel + 1;
	}

	/* No submenu active, display the main command index menu */
	if (total == -1)
	{
		char msg[80];

		strcpy(msg, "\t");
		strcat(msg, ui_getstring(UI_commandmissing));
		strcat(msg, "\n\n\t");
		strcat(msg, ui_getstring(UI_lefthilight));
		strcat(msg, " ");
		if (shortcut)
			strcat(msg, ui_getstring(UI_returntogame));
		else
			strcat(msg, ui_getstring(UI_returntoprior));
		strcat(msg, " ");
		strcat(msg, ui_getstring(UI_righthilight));
#ifdef UI_TITLE_MENU
		ui_displaymessagewindow(bitmap, msg, ui_getstring(UI_command));
#else
		ui_displaymessagewindow(bitmap, msg);
#endif /* UI_TITLE_MENU */
	}
	else
		{
		if (total == 0 || command_load)
			{
			total = command_sub_menu(Machine->gamedrv, menu_item);
			command_load = 0;

			if (total > 0)
			{
				if (shortcut)
					menu_item[total++] = ui_getstring(UI_returntogame);
				else
					menu_item[total++] = ui_getstring(UI_returntoprior);
				menu_item[total] = 0;
			}
			else
			{
				char msg[80];

				total = -1;
				strcpy(msg, "\t");
				strcat(msg, ui_getstring(UI_commandmissing));
				strcat(msg, "\n\n\t");
				strcat(msg, ui_getstring(UI_lefthilight));
				strcat(msg, " ");
				if (shortcut)
					strcat(msg, ui_getstring(UI_returntogame));
				else
					strcat(msg, ui_getstring(UI_returntoprior));
				strcat(msg, " ");
				strcat(msg, ui_getstring(UI_righthilight));
#ifdef UI_TITLE_MENU
				ui_displaymessagewindow(bitmap, msg, ui_getstring(UI_command));
#else
				ui_displaymessagewindow(bitmap, msg);
#endif /* UI_TITLE_MENU */
		}
	}
		else
		{
			if (shortcut)
				menu_item[total - 1] = ui_getstring(UI_returntogame);
			else
				menu_item[total - 1] = ui_getstring(UI_returntoprior);

#ifdef UI_TITLE_MENU
			ui_displaymenu(bitmap,menu_item,0,0,ui_getstring(UI_command),sel,0);
#else
			ui_displaymenu(bitmap,menu_item,0,0,sel,0);
#endif /* UI_TITLE_MENU */
		}

		if (total > 0)
		{
			if (input_ui_pressed_repeat(IPT_UI_DOWN,4))
				sel = (sel + 1) % total;

			if (input_ui_pressed_repeat(IPT_UI_UP,4))
				sel = (sel + total - 1) % total;

			if (input_ui_pressed_repeat(IPT_UI_PAN_UP,4))
			{
				sel -= visible;
				if (sel < 0)	sel = 0;
		}

			if (input_ui_pressed_repeat(IPT_UI_PAN_DOWN,4))
		{
				sel += visible;
				if (sel >= total)	sel = total - 1;
			}
		}
		}

		if (input_ui_pressed(IPT_UI_SELECT))
	{
		if (sel == (total - 1) || total == -1) // return to main
		{
			command_lastselected = 0;
			sel = -1;
		}
		else
		{
			command_lastselected = 1;
			schedule_full_refresh();
		}
	}

	/* Cancel pops us up a menu level */
		if (input_ui_pressed(IPT_UI_CANCEL))
			sel = -1;

	/* The UI key takes us all the way back out */
	if (input_ui_pressed(IPT_UI_CONFIGURE) && shortcut == 0)
			sel = -2;

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();
		total = 0;
	}

	return sel + 1;
}
#endif /* CMD_LIST */


#ifdef INP_CAPTION
//============================================================
//	draw_caption
//============================================================

static void draw_caption(struct mame_bitmap *bitmap)
{
	static char next_caption[512], caption_text[512];
	static int next_caption_timer;

	if (options.caption && next_caption_frame < 0)
	{
		char	read_buf[512];
skip_comment:
		if (mame_fgets(read_buf, 511, options.caption) == NULL)
		{
			mame_fclose(options.caption);
			options.caption = NULL;
		}
		else
		{
			char	buf[16] = "";
			int		i, j;

			for (i = 0, j = 0; i < 16; i++)
			{
				if (read_buf[i] == '\t' || read_buf[i] == ' ')
					continue;
				if ((read_buf[i] == '#' || read_buf[i] == '\r' || read_buf[i] == '\n') && j == 0)
					goto skip_comment;
				if (read_buf[i] < '0' || read_buf[i] > '9')
				{
					buf[j++] ='\0';
					break;
				}
				buf[j++] = read_buf[i];
			}

			next_caption_frame = strtol(buf, NULL, 10);
			next_caption_timer = 0;
			if (next_caption_frame == 0)
		{
				next_caption_frame = cpu_getcurrentframe();
				strcpy(next_caption, _("Error: illegal caption file"));
				mame_fclose(options.caption);
				options.caption = NULL;
		}

			for (;;i++)
		{
				if (read_buf[i] == '(')
				{
					for (i++, j = 0;;i++)
					{
						if (read_buf[i] == '\t' || read_buf[i] == ' ')
							continue;
						if (read_buf[i] < '0' || read_buf[i] > '9')
						{
							buf[j++] ='\0';
							break;
						}
						buf[j++] = read_buf[i];
		}

					next_caption_timer = strtol(buf, NULL, 10);

					for (;;i++)
					{
						if (read_buf[i] == '\t' || read_buf[i] == ' ')
							continue;
						if (read_buf[i] == ':')
							break;
	}
				}
				if (read_buf[i] != '\t' && read_buf[i] != ' ' && read_buf[i] != ':')
					break;
			}
			if (next_caption_timer == 0)
	{
				next_caption_timer = 5 * Machine->drv->frames_per_second;	// 5sec.
			}

			strcpy(next_caption, &read_buf[i]);

			for (i = 0; next_caption[i] != '\0'; i++)
		{
				if (next_caption[i] == '\r' || next_caption[i] == '\n')
				{
					next_caption[i] = '\0';
					break;
				}
			}
		}
	}

	if (next_caption_timer && next_caption_frame <= cpu_getcurrentframe())
	{
		caption_timer = next_caption_timer;
		strcpy(caption_text, next_caption);
		next_caption_frame = -1;
		next_caption_timer = 0;
	}

	if (caption_timer)
	{
		ui_displaymessagewindow_under(bitmap, caption_text);
		caption_timer--;
	}
}
#endif /* INP_CAPTION */

int memcard_menu(struct mame_bitmap *bitmap, int selection)
{
	int sel;
	int menutotal = 0;
	const char *menuitem[10];
	char buf[256];
	char buf2[256];

	sel = selection - 1 ;

	sprintf(buf, "%s %03d", ui_getstring (UI_loadcard), mcd_number);
	menuitem[menutotal++] = buf;
	menuitem[menutotal++] = ui_getstring (UI_ejectcard);
	menuitem[menutotal++] = ui_getstring (UI_createcard);
	menuitem[menutotal++] = ui_getstring (UI_returntomain);
	menuitem[menutotal] = 0;

	if (mcd_action!=0)
	{
		strcpy (buf2, "\n");

		switch(mcd_action)
		{
			case 1:
				strcat (buf2, ui_getstring (UI_loadfailed));
				break;
			case 2:
				strcat (buf2, ui_getstring (UI_loadok));
				break;
			case 3:
				strcat (buf2, ui_getstring (UI_cardejected));
				break;
			case 4:
				strcat (buf2, ui_getstring (UI_cardcreated));
				break;
			case 5:
				strcat (buf2, ui_getstring (UI_cardcreatedfailed));
				strcat (buf2, "\n");
				strcat (buf2, ui_getstring (UI_cardcreatedfailed2));
				break;
			default:
				strcat (buf2, ui_getstring (UI_carderror));
				break;
		}

		strcat (buf2, "\n\n");

#ifdef UI_TITLE_MENU
		ui_displaymessagewindow(bitmap,buf2,ui_getstring(UI_memorycard));
#else
		ui_displaymessagewindow(bitmap,buf2);
#endif /* UI_TITLE_MENU */
		if (input_ui_pressed(IPT_UI_SELECT))
			mcd_action = 0;
	}
	else
	{
#ifdef UI_TITLE_MENU
		ui_displaymenu(bitmap,menuitem,0,0,ui_getstring(UI_memorycard),sel,0);
#else
		ui_displaymenu(bitmap,menuitem,0,0,sel,0);
#endif /* UI_TITLE_MENU */

		if (input_ui_pressed_repeat(IPT_UI_RIGHT,8))
			mcd_number = (mcd_number + 1) % 1000;

		if (input_ui_pressed_repeat(IPT_UI_LEFT,8))
			mcd_number = (mcd_number + 999) % 1000;

		if (input_ui_pressed_repeat(IPT_UI_DOWN,8))
			sel = (sel + 1) % menutotal;

		if (input_ui_pressed_repeat(IPT_UI_UP,8))
			sel = (sel + menutotal - 1) % menutotal;

		if (input_ui_pressed(IPT_UI_SELECT))
		{
			switch(sel)
			{
			case 0:
				memcard_intf.eject();
				if (memcard_intf.load(mcd_number))
				{
					memcard_status=1;
					memcard_number=mcd_number;
					mcd_action = 2;
				}
				else
					mcd_action = 1;
				break;
			case 1:
				memcard_intf.eject();
				mcd_action = 3;
				break;
			case 2:
				if (memcard_intf.create(mcd_number))
					mcd_action = 4;
				else
					mcd_action = 5;
				break;
#ifdef MESS
			case 3:
				memcard_manager=1;
				sel=-2;
				machine_reset();
				break;
			case 4:
				sel=-1;
				break;
#else
			case 3:
				sel=-1;
				break;
#endif


			}
		}

		if (input_ui_pressed(IPT_UI_CANCEL))
			sel = -1;

		if (input_ui_pressed(IPT_UI_CONFIGURE))
			sel = -2;

		if (sel == -1 || sel == -2)
		{
			schedule_full_refresh();
		}
	}

	return sel + 1;
}


static int document_menu(struct mame_bitmap *bitmap, int selected)
{
	enum {
#ifndef MESS
#ifdef MASH_DATAFILE
		UI_MAMEINFO, UI_DRIVINFO, UI_STATISTICS,
#endif /* MASH_DATAFILE */
#else
		UI_IMAGEINFO,
#endif
#ifdef CMD_LIST
		UI_COMMAND,
#endif /* CMD_LIST */
		UI_HISTORY, UI_RETURNTOPRIOR
	};

	const char *menuitem[10];
	int menuaction[10];
	int menutotal = 0;
	int sel,res=-1;
	static int menu_lastselected = 0;

	if (selected == -1)
		sel = menu_lastselected;
	else sel = selected - 1;

#ifndef MESS
	menuitem[menutotal] = ui_getstring (UI_history); menuaction[menutotal++] = UI_HISTORY;
#ifdef MASH_DATAFILE
	menuitem[menutotal] = ui_getstring (UI_mameinfo); menuaction[menutotal++] = UI_MAMEINFO;
	menuitem[menutotal] = ui_getstring (UI_drivinfo); menuaction[menutotal++] = UI_DRIVINFO;
	menuitem[menutotal] = ui_getstring (UI_statistics); menuaction[menutotal++] = UI_STATISTICS;
#endif /* MASH_DATAFILE */
#ifdef CMD_LIST
	menuitem[menutotal] = ui_getstring (UI_command); menuaction[menutotal++] = UI_COMMAND;
#endif /* CMD_LIST */
#else
	menuitem[menutotal] = ui_getstring (UI_imageinfo); menuaction[menutotal++] = UI_IMAGEINFO;
	menuitem[menutotal] = ui_getstring (UI_history); menuaction[menutotal++] = UI_HISTORY;
#endif
	menuitem[menutotal] = ui_getstring(UI_returntoprior); menuaction[menutotal++] = UI_RETURNTOPRIOR;
	menuitem[menutotal] = 0; /* terminate array */

	if (sel > SEL_MASK)
	{
		switch (menuaction[sel & SEL_MASK])
		{
			case UI_HISTORY:
#ifdef MASH_DATAFILE
				res = displaydatinfo(bitmap, sel >> SEL_BITS, DATAFILE_HISTORY);
#else /* MASH_DATAFILE */
				res = displayhistory(bitmap, sel >> SEL_BITS);
#endif /* MASH_DATAFILE */
				break;
#ifndef MESS
#ifdef MASH_DATAFILE
			case UI_MAMEINFO:
				res = displaydatinfo(bitmap, sel >> SEL_BITS, DATAFILE_MAMEINFO);
				break;
			case UI_DRIVINFO:
				res = displaydatinfo(bitmap, sel >> SEL_BITS, DATAFILE_DRIVINFO);
				break;
			case UI_STATISTICS:
				res = displaydatinfo(bitmap, sel >> SEL_BITS, DATAFILE_STATISTICS);
				break;
#endif /* MASH_DATAFILE */
#else
			case UI_IMAGEINFO:
				res = displayimageinfo(bitmap, sel >> SEL_BITS);
				break;
#endif
#ifdef CMD_LIST
			case UI_COMMAND:
				res = displaycommand_ex(bitmap, sel >> SEL_BITS, 0);
				break;
#endif /* CMD_LIST */
		}

		if (res == -1)
		{
			menu_lastselected = sel;
			sel = -2;
		}
		else
			sel = (sel & SEL_MASK) | (res << SEL_BITS);

		return sel + 1;
	}

#ifdef UI_TITLE_MENU
	ui_displaymenu(bitmap,menuitem,0,0,ui_getstring(UI_gamedocuments),sel,0);
#else
	ui_displaymenu(bitmap,menuitem,0,0,sel,0);
#endif /* UI_TITLE_MENU */

	if (input_ui_pressed_repeat(IPT_UI_DOWN,8))
		sel = (sel + 1) % menutotal;

	if (input_ui_pressed_repeat(IPT_UI_UP,8))
		sel = (sel + menutotal - 1) % menutotal;

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		switch (menuaction[sel])
		{
			#ifndef MESS
#ifdef MASH_DATAFILE
			case UI_MAMEINFO:
			case UI_DRIVINFO:
			case UI_STATISTICS:
#endif /* MASH_DATAFILE */
			#else
			case UI_IMAGEINFO:
			#endif
			case UI_HISTORY:
#ifdef CMD_LIST
			case UI_COMMAND:
#endif /* CMD_LIST */
				sel |= 1 << SEL_BITS;
				schedule_full_refresh();
				break;

			default:
				if (sel == menutotal - 1)
				{
					menu_lastselected = 0;
					sel = -1;
				}
		}
	}

	if (input_ui_pressed(IPT_UI_CANCEL))
	{
		menu_lastselected = 0;
		sel = -1;
	}

	if (input_ui_pressed(IPT_UI_CONFIGURE))
		sel = -2;

	if (sel == -1 || sel == -2)
	{
		schedule_full_refresh();
	}

	return sel + 1;
}


enum { UI_SWITCH = 0,UI_DEFGROUP,UI_CODE,
#ifdef USE_CUSTOM_BUTTON
		UI_CUSTOM,
#endif /* USE_CUSTOM_BUTTON */
		UI_ANALOG,UI_AUTOFIRE,UI_CALIBRATE,
#ifndef MESS
		UI_STATS,UI_GAMEINFO,
#else
		UI_GAMEINFO, UI_FILEMANAGER,UI_TAPECONTROL,
#endif
		UI_GAMEDOCS,UI_CHEAT,UI_RESET,UI_MEMCARD,
#ifdef MESS
		UI_EXIT, UI_CONFIGURATION, UI_CATEGORIES};
#else
		UI_EXIT };
#endif


#define MAX_SETUPMENU_ITEMS 20
static const char *menu_item[MAX_SETUPMENU_ITEMS];
static int menu_action[MAX_SETUPMENU_ITEMS];
static int menu_total;


static void append_menu(int uistring, int action)
{
	menu_item[menu_total] = ui_getstring(uistring);
	menu_action[menu_total++] = action;
}


static int has_dipswitches(void)
{
	struct InputPort *in;
	int num;

	/* Determine if there are any dip switches */
	num = 0;
	for (in = Machine->input_ports; in->type != IPT_END; in++)
	{
		if (in->type == IPT_DIPSWITCH_NAME && input_port_active(in))
			num++;
	}
	return num > 0;
}


static int has_analog(void)
{
	struct InputPort *in;
	int num;

	/* Determine if there are any analog controls */
	num = 0;
	for (in = Machine->input_ports; in->type != IPT_END; in++)
	{
		if (port_type_is_analog(in->type) && input_port_active(in))
			num++;
	}
	return num > 0;
}


#ifdef MESS
static int has_configurables(void)
{
	struct InputPort *in;
	int num;

	num = 0;
	for (in = Machine->input_ports; in->type != IPT_END; in++)
	{
		if (in->type == IPT_CONFIG_NAME && input_port_active(in))
			num++;
	}
	return num > 0;
}


static int has_categories(void)
{
	struct InputPort *in;
	int num;

	num = 0;
	for (in = Machine->input_ports; in->type != IPT_END; in++)
	{
		if (in->category > 0 && input_port_active(in))
			num++;
	}
	return num > 0;
}
#endif /* MESS */


static void setup_menu_init(void)
{
	menu_total = 0;

	append_menu(UI_inputgeneral, UI_DEFGROUP);
	append_menu(UI_inputspecific, UI_CODE);
#ifdef USE_CUSTOM_BUTTON
	if (custom_buttons)
		append_menu(UI_custombuttons, UI_CUSTOM);
#endif /* USE_CUSTOM_BUTTON */
	append_menu(UI_autofire, UI_AUTOFIRE);

	if (has_dipswitches())
		append_menu(UI_dipswitches, UI_SWITCH);

#ifdef MESS
	if (has_configurables())
		append_menu(UI_configuration, UI_CONFIGURATION);
	if (has_categories())
		append_menu(UI_categories, UI_CATEGORIES);
#endif /* MESS */


	if (has_analog())
		append_menu(UI_analogcontrols, UI_ANALOG);

	/* Joystick calibration possible? */
	if ((osd_joystick_needs_calibration()) != 0)
	{
		append_menu(UI_calibrate, UI_CALIBRATE);
	}

#ifndef MESS
	append_menu(UI_bookkeeping, UI_STATS);
	append_menu(UI_gameinfo, UI_GAMEINFO);
#endif
#ifdef MESS
	append_menu(UI_filemanager, UI_FILEMANAGER);
#if HAS_WAVE
	if (device_find(Machine->devices, IO_CASSETTE))
		append_menu(UI_tapecontrol, UI_TAPECONTROL);
#endif /* HAS_WAVE */
#endif /* !MESS */
	append_menu(UI_gamedocuments, UI_GAMEDOCS);

	if (options.cheat)
	{
		append_menu(UI_cheat, UI_CHEAT);
	}

	if (	memcard_intf.create != NULL &&
		memcard_intf.load != NULL &&
		memcard_intf.save != NULL &&
		memcard_intf.eject != NULL)
	{
		append_menu(UI_memorycard, UI_MEMCARD);
	}
#ifdef KAILLERA
	if (!kPlay)
	{
#endif /* KAILLERA */

	append_menu(UI_resetgame, UI_RESET);
#ifdef KAILLERA
	}
#endif /* KAILLERA */
	append_menu(UI_returntogame, UI_EXIT);
	menu_item[menu_total] = 0; /* terminate array */
}


static int setup_menu(struct mame_bitmap *bitmap, int selected)
{
	int sel,res=-1;
	static int menu_lastselected = 0;


	if (selected == -1)
		sel = menu_lastselected;
	else sel = selected - 1;

	if (sel > SEL_MASK)
	{
		switch (menu_action[sel & SEL_MASK])
		{
			case UI_SWITCH:
				res = setdipswitches(bitmap, sel >> SEL_BITS);
				break;
			case UI_DEFGROUP:
				res = setdefcodesettings(bitmap, sel >> SEL_BITS);
				break;
			case UI_CODE:
				res = setcodesettings(bitmap, sel >> SEL_BITS);
				break;
			case UI_ANALOG:
				res = settraksettings(bitmap, sel >> SEL_BITS);
				break;
			case UI_AUTOFIRE:
				res = setautofiresettings(bitmap, sel >> SEL_BITS);
				break;
			case UI_CALIBRATE:
				res = calibratejoysticks(bitmap, sel >> SEL_BITS);
				break;
#ifdef USE_CUSTOM_BUTTON
			case UI_CUSTOM:
				res = setcustombutton(bitmap, sel >> SEL_BITS);
				break;
#endif /* USE_CUSTOM_BUTTON */
#ifndef MESS
			case UI_STATS:
				res = mame_stats(bitmap, sel >> SEL_BITS);
				break;
			case UI_GAMEINFO:
				res = displaygameinfo(bitmap, sel >> SEL_BITS);
				break;
#endif
#ifdef MESS
			case UI_IMAGEINFO:
				res = displayimageinfo(bitmap, sel >> SEL_BITS);
				break;
			case UI_FILEMANAGER:
				res = filemanager(bitmap, sel >> SEL_BITS);
				break;
#if HAS_WAVE
			case UI_TAPECONTROL:
				res = tapecontrol(bitmap, sel >> SEL_BITS);
				break;
#endif /* HAS_WAVE */
			case UI_CONFIGURATION:
				res = setconfiguration(bitmap, sel >> SEL_BITS);
				break;
			case UI_CATEGORIES:
				res = setcategories(bitmap, sel >> SEL_BITS);
				break;
#endif /* MESS */
			case UI_GAMEDOCS:
				res = document_menu(bitmap, sel >> SEL_BITS);
				break;
			case UI_CHEAT:
				res = cheat_menu(bitmap, sel >> SEL_BITS);
				break;
			case UI_MEMCARD:
				res = memcard_menu(bitmap, sel >> SEL_BITS);
				break;
		}

		if (res == -1)
		{
			menu_lastselected = sel;
			sel = -1;
		}
		else
			sel = (sel & SEL_MASK) | (res << SEL_BITS);

		return sel + 1;
	}

#ifdef UI_TITLE_MENU
	ui_displaymenu(bitmap,menu_item,0,0,ui_getstring(UI_mainmenu),sel,0);
#else
	ui_displaymenu(bitmap,menu_item,0,0,sel,0);
#endif /* UI_TITLE_MENU */

	if (input_ui_pressed_repeat(IPT_UI_DOWN,8))
		sel = (sel + 1) % menu_total;

	if (input_ui_pressed_repeat(IPT_UI_UP,8))
		sel = (sel + menu_total - 1) % menu_total;

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		switch (menu_action[sel])
		{
			case UI_SWITCH:
			case UI_DEFGROUP:
			case UI_CODE:
			case UI_ANALOG:
			case UI_AUTOFIRE:
			case UI_CALIBRATE:
#ifdef USE_CUSTOM_BUTTON
			case UI_CUSTOM:
#endif /* USE_CUSTOM_BUTTON */
			#ifndef MESS
			case UI_STATS:
			case UI_GAMEINFO:
			#else
			case UI_GAMEINFO:
			case UI_FILEMANAGER:
			case UI_TAPECONTROL:
			case UI_CONFIGURATION:
			case UI_CATEGORIES:
#endif /* !MESS */
			case UI_GAMEDOCS:
			case UI_CHEAT:
			case UI_MEMCARD:
				sel |= 1 << SEL_BITS;
				schedule_full_refresh();
				break;

			case UI_RESET:
				machine_reset();
				break;

			case UI_EXIT:
				menu_lastselected = 0;
				sel = -1;
				break;
		}
	}

	if (input_ui_pressed(IPT_UI_CANCEL) ||
			input_ui_pressed(IPT_UI_CONFIGURE))
	{
		menu_lastselected = sel;
		sel = -1;
	}

	if (sel == -1)
	{
		schedule_full_refresh();
	}

	return sel + 1;
}



/*********************************************************************

  start of On Screen Display handling

*********************************************************************/

static void displayosd(struct mame_bitmap *bitmap,const char *text,int percentage,int default_percentage)
{
	struct DisplayText dt[2];
	int avail;


	avail = (uirotwidth / uirotcharwidth) * 19 / 20;

	ui_drawbox(bitmap,(uirotwidth - uirotcharwidth * avail) / 2,
			(uirotheight - 7*uirotcharheight/2),
			avail * uirotcharwidth,
			3*uirotcharheight);

	avail--;

	drawbar(bitmap,(uirotwidth - uirotcharwidth * avail) / 2,
			(uirotheight - 3*uirotcharheight),
			avail * uirotcharwidth,
			uirotcharheight,
			percentage,default_percentage);

	dt[0].text = text;
	dt[0].color = UI_COLOR_NORMAL;
	dt[0].x = (uirotwidth - uirotcharwidth * strlen(text)) / 2;
	dt[0].y = (uirotheight - 2*uirotcharheight) + 2;
	dt[1].text = 0; /* terminate array */
	displaytext(bitmap,dt);
}

static void onscrd_adjuster(struct mame_bitmap *bitmap,int increment,int arg)
{
	struct InputPort *in = &Machine->input_ports[arg];
	char buf[80];
	int value;

	if (increment)
	{
		value = in->default_value & 0xff;
		value += increment;
		if (value > 100) value = 100;
		if (value < 0) value = 0;
		in->default_value = (in->default_value & ~0xff) | value;
	}
	value = in->default_value & 0xff;

	sprintf(buf,"%s %d%%",_(in->name),value);

	displayosd(bitmap,buf,value,in->default_value >> 8);
}

static void onscrd_volume(struct mame_bitmap *bitmap,int increment,int arg)
{
	char buf[20];
	int attenuation;

	if (increment)
	{
		attenuation = osd_get_mastervolume();
		attenuation += increment;
		if (attenuation > 0) attenuation = 0;
		if (attenuation < -32) attenuation = -32;
		osd_set_mastervolume(attenuation);
	}
	attenuation = osd_get_mastervolume();

	sprintf(buf,"%s %3ddB", ui_getstring (UI_volume), attenuation);
	displayosd(bitmap,buf,100 * (attenuation + 32) / 32,100);
}

static void onscrd_mixervol(struct mame_bitmap *bitmap,int increment,int arg)
{
	static void *driver = 0;
	char buf[40];
	float volume;
	int ch;
	int doallchannels = 0;
	int proportional = 0;


	if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
		doallchannels = 1;
	if (!code_pressed(KEYCODE_LCONTROL) && !code_pressed(KEYCODE_RCONTROL))
		increment *= 5;
	if (code_pressed(KEYCODE_LALT) || code_pressed(KEYCODE_RALT))
		proportional = 1;

	if (increment)
	{
		if (proportional)
		{
			static float old_vol[100];
			int num_vals = sound_get_user_gain_count();
			float ratio = 1.0;
			int overflow = 0;

			if (driver != Machine->drv)
			{
				driver = (void *)Machine->drv;
				for (ch = 0; ch < num_vals; ch++)
					old_vol[ch] = sound_get_user_gain(ch);
			}

			volume = sound_get_user_gain(arg);
			if (old_vol[arg])
				ratio = (volume + increment * 0.02) / old_vol[arg];

			for (ch = 0; ch < num_vals; ch++)
			{
				volume = ratio * old_vol[ch];
				if (volume < 0 || volume > 2.0)
					overflow = 1;
			}

			if (!overflow)
			{
				for (ch = 0; ch < num_vals; ch++)
				{
					volume = ratio * old_vol[ch];
					sound_set_user_gain(ch,volume);
				}
			}
		}
		else
		{
			driver = 0; /* force reset of saved volumes */

			volume = sound_get_user_gain(arg);
			volume += increment * 0.02;
			if (volume > 2.0) volume = 2.0;
			if (volume < 0) volume = 0;

			if (doallchannels)
			{
				int num_vals = sound_get_user_gain_count();
				for (ch = 0;ch < num_vals;ch++)
					sound_set_user_gain(ch,volume);
			}
			else
				sound_set_user_gain(arg,volume);
		}
	}
	volume = sound_get_user_gain(arg);

	if (proportional)
		sprintf(buf,"%s %s %4.2f", ui_getstring (UI_allchannels), ui_getstring (UI_relative), volume);
	else if (doallchannels)
		sprintf(buf,"%s %s %4.2f", ui_getstring (UI_allchannels), ui_getstring (UI_volume), volume);
	else
		sprintf(buf,"%s %s %4.2f",sound_get_user_gain_name(arg), ui_getstring (UI_volume), volume);
	displayosd(bitmap,buf,volume*50,sound_get_default_gain(arg)*50);
}

static void onscrd_brightness(struct mame_bitmap *bitmap,int increment,int arg)
{
	char buf[20];
	double brightness;


	if (increment)
	{
		brightness = palette_get_global_brightness();
		brightness += 0.05 * increment;
		if (brightness < 0.1) brightness = 0.1;
		if (brightness > 1.0) brightness = 1.0;
		palette_set_global_brightness(brightness);
	}
	brightness = palette_get_global_brightness();

	sprintf(buf,"%s %3d%%", ui_getstring (UI_brightness), (int)(brightness * 100));
	displayosd(bitmap,buf,brightness*100,100);
}

static void onscrd_gamma(struct mame_bitmap *bitmap,int increment,int arg)
{
	char buf[20];
	double gamma_correction;

	if (increment)
	{
		gamma_correction = palette_get_global_gamma();

		gamma_correction += 0.05 * increment;
		if (gamma_correction < 0.5) gamma_correction = 0.5;
		if (gamma_correction > 2.0) gamma_correction = 2.0;

		palette_set_global_gamma(gamma_correction);
	}
	gamma_correction = palette_get_global_gamma();

	sprintf(buf,"%s %1.2f", ui_getstring (UI_gamma), gamma_correction);
	displayosd(bitmap,buf,100*(gamma_correction-0.5)/(2.0-0.5),100*(1.0-0.5)/(2.0-0.5));
}

static void onscrd_vector_flicker(struct mame_bitmap *bitmap,int increment,int arg)
{
	char buf[1000];
	float flicker_correction;

	if (!code_pressed(KEYCODE_LCONTROL) && !code_pressed(KEYCODE_RCONTROL))
		increment *= 5;

	if (increment)
	{
		flicker_correction = vector_get_flicker();

		flicker_correction += increment;
		if (flicker_correction < 0.0) flicker_correction = 0.0;
		if (flicker_correction > 100.0) flicker_correction = 100.0;

		vector_set_flicker(flicker_correction);
	}
	flicker_correction = vector_get_flicker();

	sprintf(buf,"%s %1.2f", ui_getstring (UI_vectorflicker), flicker_correction);
	displayosd(bitmap,buf,flicker_correction,0);
}

static void onscrd_vector_intensity(struct mame_bitmap *bitmap,int increment,int arg)
{
	char buf[30];
	float intensity_correction;

	if (increment)
	{
		intensity_correction = vector_get_intensity();

		intensity_correction += 0.05 * increment;
		if (intensity_correction < 0.5) intensity_correction = 0.5;
		if (intensity_correction > 3.0) intensity_correction = 3.0;

		vector_set_intensity(intensity_correction);
	}
	intensity_correction = vector_get_intensity();

	sprintf(buf,"%s %1.2f", ui_getstring (UI_vectorintensity), intensity_correction);
	displayosd(bitmap,buf,100*(intensity_correction-0.5)/(3.0-0.5),100*(1.5-0.5)/(3.0-0.5));
}


static void onscrd_overclock(struct mame_bitmap *bitmap,int increment,int arg)
{
	char buf[30];
	double overclock;
	int cpu, doallcpus = 0, oc;

	if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
		doallcpus = 1;
	if (!code_pressed(KEYCODE_LCONTROL) && !code_pressed(KEYCODE_RCONTROL))
		increment *= 5;
	if( increment )
	{
		overclock = cpunum_get_clockscale(arg);
		overclock += 0.01 * increment;
		if (overclock < 0.01) overclock = 0.01;
/* for NeoGeo s1945p */
		if (overclock > 3.0) overclock = 3.0;
		if( doallcpus )
			for( cpu = 0; cpu < cpu_gettotalcpu(); cpu++ )
				cpunum_set_clockscale(cpu, overclock);
		else
			cpunum_set_clockscale(arg, overclock);
	}

	oc = 100 * cpunum_get_clockscale(arg) + 0.5;

	if( doallcpus )
		sprintf(buf,"%s %s %3d%%", ui_getstring (UI_allcpus), ui_getstring (UI_overclock), oc);
	else
		sprintf(buf,"%s %s%d %3d%%", ui_getstring (UI_overclock), ui_getstring (UI_cpu), arg, oc);
	displayosd(bitmap,buf,oc/3,100/3);
}

static void onscrd_refresh(struct mame_bitmap *bitmap,int increment,int arg)
{
	float delta = Machine->refresh_rate - Machine->drv->frames_per_second;
	char buf[30];

	increment *= 1000;
	if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
		increment /= 10;
	if (code_pressed(KEYCODE_LCONTROL) || code_pressed(KEYCODE_RCONTROL))
		increment /= 100;
	if (code_pressed(KEYCODE_LALT) || code_pressed(KEYCODE_LALT))
		increment /= 1000;
	if (increment)
	{
		float newrate;
		delta += 0.001 * increment;
		if (delta > 10)
			delta = 10;
		if (delta < -10)
			delta = -10;

		newrate = Machine->drv->frames_per_second;
		if (delta != 0)
			newrate = (floor(newrate * 1000) / 1000) + delta;
		set_refresh_rate(newrate);
	}

	sprintf(buf,"%s %.3f", ui_getstring (UI_refresh_rate), Machine->refresh_rate);
	displayosd(bitmap,buf,(10 + delta) * 5,100/2);
}

#define MAX_OSD_ITEMS 50
static void (*onscrd_fnc[MAX_OSD_ITEMS])(struct mame_bitmap *bitmap,int increment,int arg);
static int onscrd_arg[MAX_OSD_ITEMS];
static int onscrd_total_items;

static void onscrd_init(void)
{
	struct InputPort *in;
	int item,ch;

	item = 0;

	if (Machine->sample_rate)
	{
		int num_vals = sound_get_user_gain_count();
		onscrd_fnc[item] = onscrd_volume;
		onscrd_arg[item] = 0;
		item++;

		for (ch = 0;ch < num_vals;ch++)
		{
			onscrd_fnc[item] = onscrd_mixervol;
			onscrd_arg[item] = ch;
			item++;
		}
	}

	for (in = Machine->input_ports; in && in->type != IPT_END; in++)
		if ((in->type & 0xff) == IPT_ADJUSTER)
		{
			onscrd_fnc[item] = onscrd_adjuster;
			onscrd_arg[item] = in - Machine->input_ports;
			item++;
		}

	if (options.cheat)
	{
		for (ch = 0;ch < cpu_gettotalcpu();ch++)
		{
			onscrd_fnc[item] = onscrd_overclock;
			onscrd_arg[item] = ch;
			item++;
		}
		onscrd_fnc[item] = onscrd_refresh;
		onscrd_arg[item] = ch;
		item++;
	}

	onscrd_fnc[item] = onscrd_brightness;
	onscrd_arg[item] = 0;
	item++;

	onscrd_fnc[item] = onscrd_gamma;
	onscrd_arg[item] = 0;
	item++;

	if (Machine->drv->video_attributes & VIDEO_TYPE_VECTOR)
	{
		onscrd_fnc[item] = onscrd_vector_flicker;
		onscrd_arg[item] = 0;
		item++;

		onscrd_fnc[item] = onscrd_vector_intensity;
		onscrd_arg[item] = 0;
		item++;
	}

	onscrd_total_items = item;
}

static int on_screen_display(struct mame_bitmap *bitmap, int selected)
{
	int increment,sel;
	static int lastselected = 0;


	if (selected == -1)
		sel = lastselected;
	else sel = selected - 1;

	increment = 0;
	if (input_ui_pressed_repeat(IPT_UI_LEFT,8))
		increment = -1;
	if (input_ui_pressed_repeat(IPT_UI_RIGHT,8))
		increment = 1;
	if (input_ui_pressed_repeat(IPT_UI_DOWN,8))
		sel = (sel + 1) % onscrd_total_items;
	if (input_ui_pressed_repeat(IPT_UI_UP,8))
		sel = (sel + onscrd_total_items - 1) % onscrd_total_items;

	(*onscrd_fnc[sel])(bitmap,increment,onscrd_arg[sel]);

	lastselected = sel;

	if (input_ui_pressed(IPT_UI_ON_SCREEN_DISPLAY))
	{
		sel = -1;

		schedule_full_refresh();
	}

	return sel + 1;
}

/*********************************************************************

  end of On Screen Display handling

*********************************************************************/


static void displaymessage(struct mame_bitmap *bitmap,const char *text)
{
	struct DisplayText dt[2];
	int avail;


	if (uirotwidth < uirotcharwidth * strlen(text))
	{
#ifdef UI_TITLE_MENU
		ui_displaymessagewindow(bitmap,text,0);
#else
		ui_displaymessagewindow(bitmap,text);
#endif /* UI_TITLE_MENU */ 
		return;
	}

	avail = strlen(text)+2;

	ui_drawbox(bitmap,(uirotwidth - uirotcharwidth * avail) / 2,
			uirotheight - 3*uirotcharheight,
			avail * uirotcharwidth,
			2*uirotcharheight);

	dt[0].text = text;
	dt[0].color = UI_COLOR_NORMAL;
	dt[0].x = (uirotwidth - uirotcharwidth * strlen(text)) / 2;
	dt[0].y = uirotheight - 5*uirotcharheight/2;
	dt[1].text = 0; /* terminate array */
	displaytext(bitmap,dt);
}


static char messagetext[200];
static int messagecounter;

void CLIB_DECL usrintf_showmessage(const char *text,...)
{
	va_list arg;
	va_start(arg,text);
	vsprintf(messagetext,text,arg);
	va_end(arg);
	messagecounter = 2 * Machine->refresh_rate;
}

void CLIB_DECL usrintf_showmessage_secs(int seconds, const char *text,...)
{
	va_list arg;
	va_start(arg,text);
	vsprintf(messagetext,text,arg);
	va_end(arg);
	messagecounter = seconds * Machine->refresh_rate;
}

void do_loadsave(struct mame_bitmap *bitmap, int request_loadsave)
{
	int file = 0;

	mame_pause(1);

	do
	{
		input_code_t code;

		if (request_loadsave == LOADSAVE_SAVE)
			displaymessage(bitmap, _("Select position to save to"));
		else
			displaymessage(bitmap, _("Select position to load from"));

		update_video_and_audio();
		reset_partial_updates();

		if (input_ui_pressed(IPT_UI_CANCEL))
			break;

		code = code_read_async();
		if (code != CODE_NONE)
		{
			if (code >= KEYCODE_A && code <= KEYCODE_Z)
				file = 'a' + (code - KEYCODE_A);
			else if (code >= KEYCODE_0 && code <= KEYCODE_9)
				file = '0' + (code - KEYCODE_0);
			else if (code >= KEYCODE_0_PAD && code <= KEYCODE_9_PAD)
				file = '0' + (code - KEYCODE_0);
		}
	}
	while (!file);

	mame_pause(0);

	if (file > 0)
	{
		if (request_loadsave == LOADSAVE_SAVE)
			usrintf_showmessage(_("Save to position %c"), file);
		else
			usrintf_showmessage(_("Load from position %c"), file);
		cpu_loadsave_schedule(request_loadsave, file);
	}
	else
	{
		if (request_loadsave == LOADSAVE_SAVE)
			usrintf_showmessage(_("Save cancelled"));
		else
			usrintf_showmessage(_("Load cancelled"));
	}
}


void ui_show_fps_temp(double seconds)
{
	if (!showfps)
		showfpstemp = (int)(seconds * Machine->refresh_rate);
}

void ui_show_fps_set(int show)
{
	if (show)
	{
		showfps = 1;
	}
	else
	{
		showfps = 0;
		showfpstemp = 0;
		schedule_full_refresh();
	}
}

int ui_show_fps_get(void)
{
	return showfps || showfpstemp;
}

void ui_show_profiler_set(int show)
{
	if (show)
	{
		show_profiler = 1;
		profiler_start();
	}
	else
	{
		show_profiler = 0;
		profiler_stop();
		schedule_full_refresh();
	}
}

int ui_show_profiler_get(void)
{
	return show_profiler;
}

void ui_display_fps(struct mame_bitmap *bitmap)
{
	const char *text, *end;
	char textbuf[256];
	int done = 0;
	int x, y = 0;
	/* remember which area we cover so that we can
       schedule a full refresh if it gets smaller */
	int len_hash = 0;
	static int old_len_hash = -1;

	/* if we're not currently displaying, skip it */
	if (!showfps && !showfpstemp)
	{
		/* reset covered area vars */
		old_len_hash = -1;
		return;
	}

	/* get the current FPS text */
	text = osd_get_fps_text(mame_get_performance_info());

	/* loop over lines */
	while (!done)
	{
		/* find the end of this line and copy it to the text buf */
		end = strchr(text, '\n');
		if (end)
		{
			memcpy(textbuf, text, end - text);
			textbuf[end - text] = 0;
			text = end + 1;
		}
		else
		{
			strcpy(textbuf, text);
			done = 1;
		}

		/* render */
		x = uirotwidth - strlen(textbuf) * uirotcharwidth;
		ui_text(bitmap, textbuf, x, y);
		y += uirotcharheight;
		len_hash += (y / uirotcharheight) * x;
	}

	if ((old_len_hash != -1) &&
	    (old_len_hash != len_hash))
	{
		schedule_full_refresh();
		ui_markdirty(&uirawbounds);
	}

	old_len_hash = len_hash;

	/* update the temporary FPS display state */
	if (showfpstemp)
	{
		showfpstemp--;
		if (!showfps && showfpstemp == 0)
			schedule_full_refresh();
	}
}


static int confirm_quit(struct mame_bitmap *bitmap)
{
	const char *quit_message =
		"Quit the game?\n"
		"Press Select key/button to quit,\n"
		"Cancel key/button to continue.";

	if (!options.confirm_quit)
		return 1;

	while (1)
	{
#ifdef ADMAME_FIX		
// TRANS_UI_FIX START		
  profiler_mark(PROFILER_VIDEO);
  if (osd_skip_this_frame() == 0)
  {
   /* keep calling vh_screenrefresh() while paused so we can stuff */
   /* debug code in there */
   draw_screen();
  }
  profiler_mark(PROFILER_END);
// TRANS_UI_FIX END	
#endif /*ADMAME_FIX*/	
#ifdef UI_TITLE_MENU	
		ui_displaymessagewindow(bitmap, _(quit_message),0);
#else
		ui_displaymessagewindow(bitmap, _(quit_message));
#endif /* UI_TITLE_MENU */

		update_video_and_audio();
#ifdef ADMAME_FIX
// TRANS_UI_FIX START	 
    reset_partial_updates();
// TRANS_UI_FIX END	  
#endif /*ADMAME_FIX*/ 

		if (!options.confirm_quit)
			return 1;

		if (input_ui_pressed(IPT_UI_CANCEL))
			break;

		if (input_ui_pressed(IPT_UI_SELECT))
			return 1;
	}

	schedule_full_refresh();

	return 0;
}

#ifdef FRAMES_COUNTER
static void showtotalframes(struct mame_bitmap *bitmap)
{
	char str[40];
	char str2[40];
	int width, width2;

	int frame;
	double time;
	unsigned int sec;

	frame = cpu_getcurrentframe();

	time = frame / (double)Machine->drv->frames_per_second;
	sec = time;

	sprintf(str, "[TotalFrame:%8d]", frame);
	sprintf(str2, "[TotalTime: %.2d:%.2d:%.2d]", sec/(60*60), sec/60%60, sec%60);

	width = strlen(str) * Machine->uifontwidth;
	width2 = strlen(str2) * Machine->uifontwidth;

	switch(Total_FT_Position)
	{
		case 0:
			ui_text(bitmap, str, 0, 0);
			ui_text(bitmap, str2, 0, uirotcharheight);
		break;

		case 1:
			ui_text(bitmap, str, 0, Machine->uiheight - (uirotcharheight * 2));
			ui_text(bitmap, str2, 0, Machine->uiheight - uirotcharheight);
		break;

		case 2:
			ui_text(bitmap, str, Machine->uiwidth - width, Machine->uiheight - (uirotcharheight * 2));
			ui_text(bitmap, str2, Machine->uiwidth - width2, Machine->uiheight - uirotcharheight);
		break;

		case 3:
			ui_text(bitmap, str, Machine->uiwidth - width, 0);
			ui_text(bitmap, str2, Machine->uiwidth - width2, uirotcharheight);
		break;

		case 4:
			ui_text(bitmap, str, Machine->uiwidth / 2 - width / 2, 0);
			ui_text(bitmap, str2, Machine->uiwidth / 2 - width2 / 2, Machine->uiheight - uirotcharheight);
		break;
	}

	if (code_pressed(KEYCODE_LSHIFT) && input_ui_pressed(IPT_UI_SHOW_TOTAL_FT))
	{
		Total_FT_Position++;

		if (Total_FT_Position > 4)
			Total_FT_Position = 0;
	}
}
#endif /*FRAMES_COUNTER*/

int handle_user_interface(struct mame_bitmap *bitmap)
{
#ifdef MESS
	extern int mess_pause_for_ui;
#endif

#ifdef USE_BOOST
	extern int boost;			/* in windows/video.c */
	extern int toggle_boost;	/* in windows/video.c */
#endif /* USE_BOOST */
#ifdef KAILLERA
      extern int special_key;
      special_key = 0;

	if (kaillera_schedule == KS_CLOSE)
	{
		if(record)
		{
			write_port_values(record_k,IPT_UI_CANCEL);
		}
		endemu = 0;
		p1_pos = 1;
		control_counter = 0;
		control_type = 0;
		init = -1;
		netplay_pause = -1;
		win_trying_to_quit = 0;
	  	return 1;
	}

	if (kPlay && setup_selected == 0 && osd_selected == 0) KailleraChatUpdate(bitmap);
	if (KailleraChatIsActive())
	{
		if (options.cheat) DoCheat(bitmap);

		if (messagecounter > 0)
		{
			displaymessage(bitmap, messagetext);

			if (--messagecounter == 0)
				schedule_full_refresh();
		}
	}
	else
	{
#endif

	/* if the user pressed F12, save the screen to a file */
	if (input_ui_pressed(IPT_UI_SNAPSHOT))
		save_screen_snapshot(bitmap);

#ifdef MAME_AVI //(NEW_AVI)
	if (input_ui_pressed(IPT_UI_RECORD_AVI))
		toggle_record_avi();
#endif

#ifdef INP_CAPTION
	draw_caption(bitmap);
#endif /* INP_CAPTION */

	/* This call is for the cheat, it must be called once a frame */
	if (options.cheat) DoCheat(bitmap);

	/* if the user pressed ESC, stop the emulation */
	/* but don't quit if the setup menu is on screen */
#ifdef KAILLERA
	if ((setup_selected == 0 && input_ui_pressed(IPT_UI_CANCEL) && nEndChat == 0) || endemu == 1 || win_trying_to_quit) 
	{
		mame_pause(1);

		if (confirm_quit(bitmap))
		{
			flash_all_values_and_buffers();
			endemu = 0;
			if(kPlay) {
				p1_pos = 1;
				control_counter = 0;
				control_type = 0;
				init = -1;
				netplay_pause = -1;
				win_trying_to_quit = 0;
				if(kPlayerNB == 1 && kPlayers > 1) SendKailleraCommand(KC_close, NULL);
			}
			if(record)
			{
				write_port_values(record_k,IPT_UI_CANCEL);
			}

			mame_pause(0);
			return 1;
		}

		mame_pause(0);
	}
#else /* KAILLERA */
	if (setup_selected == 0 && input_ui_pressed(IPT_UI_CANCEL))
	{
		mame_pause(1);

		if (confirm_quit(bitmap))
		{
			flash_all_values_and_buffers();
			return 1;
		}

		mame_pause(0);
	}
#endif
	
	if (setup_selected == 0 && input_ui_pressed(IPT_UI_CONFIGURE))
	{
		setup_selected = -1;
		if (osd_selected != 0)
		{
			osd_selected = 0;	/* disable on screen display */
			schedule_full_refresh();
		}
	}
	
	if (setup_selected != 0) setup_selected = setup_menu(bitmap, setup_selected);

//#ifdef MAME_DEBUG
//	if (!mame_debug)
//#endif
		if (osd_selected == 0 && input_ui_pressed(IPT_UI_ON_SCREEN_DISPLAY))
		{
			osd_selected = -1;
			if (setup_selected != 0)
			{
				setup_selected = 0; /* disable setup menu */
				schedule_full_refresh();
			}
		}
	if (osd_selected != 0) osd_selected = on_screen_display(bitmap, osd_selected);

#ifdef CMD_LIST
	if (setup_selected == 0 && input_ui_pressed(IPT_UI_COMMAND))
	{
		command_sc ^= 1;

		if (osd_selected != 0)
		{
			osd_selected = 0;	/* disable on screen display */
			schedule_full_refresh();
		}
	}

	if (command_sc && setup_selected == 0)
	{
		static int res;
		res = displaycommand_ex(bitmap, command_sel, 1);
		command_sel = res++;

		if (command_sel == 0)
		{
			command_sc = 0;
			command_sel = 1;
		}
	}
#endif /* CMD_LIST */

	if (options.cheat && input_ui_pressed(IPT_UI_CHEAT))
	{
		cheat_sc ^= 1;
		cheat_fe = 1;
	}

	if (cheat_sc && setup_selected == 0)
	{
		static int res;
		res = EnableDisableCheatMenu(bitmap, cheat_sel, cheat_fe, 1);
		cheat_sel = res++;
		cheat_fe = 0;

		if (cheat_sel == 0)
		{
			cheat_sc = 0;
			cheat_sel = 1;
		}
	}

#ifdef KAILLERA
	/* if the user pressed F3, reset the emulation */
	if (!kPlay) {
		if (input_ui_pressed(IPT_UI_RESET_MACHINE))
		{
		if(record) write_port_values(record_k,IPT_UI_RESET_MACHINE);
			special_key = 1;
			
			machine_reset();
		}

		if (input_ui_pressed(IPT_UI_SAVE_STATE))
		{
			if(record) {
			write_port_values(record_k,IPT_UI_SAVE_STATE);
				special_key = 1;
				cpu_loadsave_schedule(LOADSAVE_SAVE, '@');
			}
			else {
				do_loadsave(bitmap, LOADSAVE_SAVE);
			}
		}

		if (input_ui_pressed(IPT_UI_LOAD_STATE))
		{
			if(record) {
			write_port_values(record_k,IPT_UI_LOAD_STATE);
				special_key = 1;
				cpu_loadsave_schedule(LOADSAVE_LOAD, '@');
			}
			else {
				do_loadsave(bitmap, LOADSAVE_LOAD);
			}
		}
	} /* if(!kPlay) */
#else
	/* if the user pressed F3, reset the emulation */
	if (input_ui_pressed(IPT_UI_RESET_MACHINE))
		machine_reset();

	if (input_ui_pressed(IPT_UI_SAVE_STATE))
		do_loadsave(bitmap, LOADSAVE_SAVE);

	if (input_ui_pressed(IPT_UI_LOAD_STATE))
		do_loadsave(bitmap, LOADSAVE_LOAD);
#endif /* KAILLERA */


#ifdef KAILLERA
	if (kPlay && netplay_pause == 1)
	{
		mame_pause(1);

		while (netplay_pause != -1)
		{
			profiler_mark(PROFILER_VIDEO);
			if (osd_skip_this_frame() == 0)
			{
				/* keep calling vh_screenrefresh() while paused so we can stuff */
				/* debug code in there */
				draw_screen();
			}
			profiler_mark(PROFILER_END);

			if (kaillera_schedule == KS_CLOSE)
			{
				if(record)
				{
					write_port_values(record_k,IPT_UI_CANCEL);
				}
				endemu = 0;
				p1_pos = 1;
				control_counter = 0;
				control_type = 0;
				init = -1;
				netplay_pause = -1;
				win_trying_to_quit = 0;
				mame_pause(0);
			  	return 1;
			}

			if (setup_selected == 0 && osd_selected == 0) KailleraChatUpdate(bitmap);
			if (KailleraChatIsActive()) {}
			else {
				if (input_ui_pressed(IPT_UI_SNAPSHOT))
					save_screen_snapshot(bitmap);

#ifdef MAME_AVI //(NEW_AVI)
				if (input_ui_pressed(IPT_UI_RECORD_AVI))
					toggle_record_avi();
#endif

				if ((setup_selected == 0 && input_ui_pressed(IPT_UI_CANCEL) && nEndChat == 0) || win_trying_to_quit) {
					if(record)
					{
						write_port_values(record_k,IPT_UI_CANCEL);
					}
					endemu = 0;
					p1_pos = 1;
					control_counter = 0;
					control_type = 0;
					init = -1;
					netplay_pause = -1;
					win_trying_to_quit = 0;
					if(kPlayerNB == 1 && kPlayers > 1) SendKailleraCommand(KC_close, NULL);
					mame_pause(0);
					return 1;
				}

				if (setup_selected == 0 && input_ui_pressed(IPT_UI_CONFIGURE))
				{
					setup_selected = -1;
					if (osd_selected != 0)
					{
						osd_selected = 0;	/* disable on screen display */
						schedule_full_refresh();
					}
				}
				if (setup_selected != 0) setup_selected = setup_menu(bitmap, setup_selected);

				// [START] 공략
				if (setup_selected == 0 && input_ui_pressed(IPT_UI_COMMAND))
				{
					command_sc ^= 1;

					if (osd_selected != 0)
					{
						osd_selected = 0;	/* disable on screen display */
						schedule_full_refresh();
					}
				}

				if (command_sc && setup_selected == 0)
				{
					static int res;
					res = displaycommand_ex(bitmap, command_sel, 1);
					command_sel = res++;

					if (command_sel == 0)
					{
						command_sc = 0;
						command_sel = 1;
					}
				}
				// [END] 공략

				if (!mame_debug && osd_selected == 0 && input_ui_pressed(IPT_UI_ON_SCREEN_DISPLAY))
				{
					osd_selected = -1;
					if (setup_selected != 0)
					{
						setup_selected = 0; /* disable setup menu */
						schedule_full_refresh();
					}
				}
				if (osd_selected != 0) osd_selected = on_screen_display(bitmap, osd_selected);
	
				/* show popup message if any */
				if (messagecounter > 0)
				{
					displaymessage(bitmap, messagetext);

					if (--messagecounter == 0)
						schedule_full_refresh();
				}
				nEndChat = 0;
			}
//			update_input_ports();
			inputport_vblank_start();
			
			update_video_and_audio();
			reset_partial_updates(); 
		}
		mame_pause(0);
	}

	if (!kPlay)
#endif

#ifndef MESS
	if (auto_pause || single_step || input_ui_pressed(IPT_UI_PAUSE)) /* pause the game */
	{
#else
	if (setup_selected)
		mess_pause_for_ui = 1;

	if (auto_pause || single_step || input_ui_pressed(IPT_UI_PAUSE) || mess_pause_for_ui) /* pause the game */
	{
#endif
/*      osd_selected = 0;      disable on screen display, since we are going   */
							/* to change parameters affected by it */
		auto_pause = 0;

		if (single_step == 0)
			mame_pause(1);

		while (!input_ui_pressed(IPT_UI_PAUSE))
		{
			profiler_mark(PROFILER_VIDEO);
			if (osd_skip_this_frame() == 0)
			{
				/* keep calling vh_screenrefresh() while paused so we can stuff */
				/* debug code in there */
				draw_screen();
			}
			profiler_mark(PROFILER_END);

			if (input_ui_pressed(IPT_UI_SNAPSHOT))
				save_screen_snapshot(bitmap);

#ifdef MAME_AVI //(NEW_AVI)
			if (input_ui_pressed(IPT_UI_RECORD_AVI))
				toggle_record_avi();
#endif

#ifdef INP_CAPTION
			draw_caption(bitmap);
#endif /* INP_CAPTION */
#ifdef CMD_LIST
			if (input_ui_pressed(IPT_UI_COMMAND))
			{
				command_sc ^= 1;
			}

			if (command_sc && setup_selected == 0)
			{
				static int res;
				res = displaycommand_ex(bitmap, command_sel, 1);
				command_sel = res++;

				if (command_sel == 0)
				{
					command_sc = 0;
					command_sel = 1;
				}
			}
#endif /* CMD_LIST */

			if (options.cheat && input_ui_pressed(IPT_UI_CHEAT))
			{
				cheat_sc ^= 1;
				cheat_fe = 1;
			}

			if (cheat_sc && setup_selected == 0)
			{
				static int res;
				res = EnableDisableCheatMenu(bitmap, cheat_sel, cheat_fe, 1);
				cheat_sel = res++;
				cheat_fe = 0;

				if (cheat_sel == 0)
				{
					cheat_sc = 0;
					cheat_sel = 1;
				}
			}

			if (input_ui_pressed(IPT_UI_SAVE_STATE))
				do_loadsave(bitmap, LOADSAVE_SAVE);

			if (input_ui_pressed(IPT_UI_LOAD_STATE))
				do_loadsave(bitmap, LOADSAVE_LOAD);

			/* if the user pressed F4, show the character set */
			if (input_ui_pressed(IPT_UI_SHOW_GFX))
				showcharset(bitmap);

			if (setup_selected == 0 && input_ui_pressed(IPT_UI_CANCEL))
				if (confirm_quit(bitmap))
				{
					flash_all_values_and_buffers();
					return 1;
				}

			if (setup_selected == 0 && input_ui_pressed(IPT_UI_CONFIGURE))
			{
				setup_selected = -1;
				if (osd_selected != 0)
				{
					osd_selected = 0;	/* disable on screen display */
					schedule_full_refresh();
				}
			}
			if (setup_selected != 0) setup_selected = setup_menu(bitmap, setup_selected);

//#ifdef MAME_DEBUG
//			if (!mame_debug)
//#endif
				if (osd_selected == 0 && input_ui_pressed(IPT_UI_ON_SCREEN_DISPLAY))
				{
					osd_selected = -1;
					if (setup_selected != 0)
					{
						setup_selected = 0; /* disable setup menu */
						schedule_full_refresh();
					}
				}
			if (osd_selected != 0) osd_selected = on_screen_display(bitmap, osd_selected);

			if (options.cheat) DisplayWatches(bitmap);

			/* show popup message if any */
			if (messagecounter > 0)
			{
				displaymessage(bitmap, messagetext);

				if (--messagecounter == 0)
					schedule_full_refresh();
			}

			/* show FPS display? */
			if (input_ui_pressed(IPT_UI_SHOW_FPS))
			{
				/* if we're temporarily on, turn it off immediately */
				if (showfpstemp)
				{
					showfpstemp = 0;
					schedule_full_refresh();
				}

				/* otherwise, just toggle; force a refresh if going off */
				else
				{
					showfps ^= 1;
					if (!showfps)
						schedule_full_refresh();
				}
			}

			/* add the FPS counter */
			ui_display_fps(bitmap);

#ifdef FRAMES_COUNTER
			if ((!code_pressed(KEYCODE_LSHIFT) && !code_pressed(KEYCODE_RSHIFT)) && (input_ui_pressed(IPT_UI_SHOW_TOTAL_FT)))
				total_frame_counter = (total_frame_counter == 0) ? 1 : 0;

			if (total_frame_counter)
				showtotalframes(bitmap);
#endif /*FRAMES_COUNTER*/

			update_video_and_audio();
			reset_partial_updates();

#ifdef MESS
			if (!setup_selected && mess_pause_for_ui)
			{
				mess_pause_for_ui = 0;
				break;
			}
#endif /* MESS */
		}

		if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
			single_step = 1;
		else
		{
			single_step = 0;
			mame_pause(0);
		}

		schedule_full_refresh();
	}

#if defined(__sgi) && !defined(MESS)
	game_paused = 0;
#endif
#ifdef INPUT_LOG
	if (input_ui_pressed(IPT_UI_SHOW_INPUT_LOG))
	{
		show_input ^= 1;

		schedule_full_refresh();
		memset(inputlog_buf, 0, COMMAND_LOG_BUFSIZE);
	}

	if (show_input && inputlog_counter)
	{
 		ui_transbox(bitmap, 0, uirotheight - uirotcharheight, uirotwidth, uirotcharheight);

		ui_text(bitmap, inputlog_buf, 0, uirotheight - uirotcharheight);
	
		if (--inputlog_counter == 0) {
			schedule_full_refresh();
			memset(inputlog_buf, 0, COMMAND_LOG_BUFSIZE);
		}
	}

#endif /*INPUT_LOG*/	
	/* show popup message if any */
	if (messagecounter > 0)
	{
		displaymessage(bitmap, messagetext);

		if (--messagecounter == 0)
			schedule_full_refresh();
	}


	if (input_ui_pressed(IPT_UI_SHOW_PROFILER))
	{
		ui_show_profiler_set(!ui_show_profiler_get());
	}

	if (show_profiler) profiler_show(bitmap);

#ifdef KAILLERA
	if(kPlay && kPlayerNB == 1)
	{
		if (input_ui_pressed(IPT_UI_VERSION))
		{
			SendKailleraCommand(KC_version, NULL);
		}
		if (input_ui_pressed(IPT_UI_TOGGLE_AUTOSAVE))
		{
			if(ksavecounter) {
				ksavecounter = 0;
				kailleraChatSend(_("Auto Save disabled"));
			} else {
				ksavecounter = 1;
				kailleraChatSend(_("Auto Save enabled"));
			}
		}
		if (ksavecounter) {
			ksavecounter++;
			if (ksavecounter > AUTOSAVEPERIOD && kcrccounter == 0 && kaillera_schedule == KS_NONE) {
				kaillera_schedule = KS_SAVE;
				ksavecounter = 1;
			}
		}
		if (kdipcounter == 0)
		{
			if (input_ui_pressed(IPT_UI_SEND_DIP)) {
				struct InputPort *in = Machine->input_ports;
				char	buf[256];
				int		port = 0;
				UINT16	mask, value;
				in++;
				while (in->type != IPT_END && port < MAX_INPUT_PORTS)
				{
					mask = 0;
					value = 0;
					while (in->type != IPT_END && in->type != IPT_PORT)
					{
						if (in->type == IPT_DIPSWITCH_NAME)
						{
							mask |= in->mask;
							value |= in->default_value & in->mask;
						}
						in++;
					}
					if(mask){
						sprintf(&buf[port*4], "%04X", value);
						port++;
					}
					if (in->type == IPT_PORT) in++;
				}
				SendKailleraCommand(KC_dip, buf);
				kdipcounter = 3 * 60;
				kgotdip = 0;
			}
		} else {
			kdipcounter--;
			if (kdipcounter <= 0) {
                        usrintf_showmessage("Dipswitch send timeout");
				kgotdip = 0;
			}
		}
		if (kcrccounter != 0) {
			kcrccounter--;
			if (kcrccounter <= 0) {
                        usrintf_showmessage("Desync checking timeout");
				kgotcrc = 0;
			}
		}
	}
	}
	nEndChat = 0;
#endif

	/* show FPS display? */
	if (input_ui_pressed(IPT_UI_SHOW_FPS))
	{
		/* toggle fps */
		ui_show_fps_set(!ui_show_fps_get());
	}


	/* if the user pressed F4, show the character set */
#ifdef KAILLERA
	if (!kPlay) {
#endif
	if (input_ui_pressed(IPT_UI_SHOW_GFX))
	{
		osd_sound_enable(0);

		showcharset(bitmap);

		osd_sound_enable(1);
	}
#ifdef KAILLERA
	}
#endif /* KAILLERA */
	if (input_ui_pressed(IPT_UI_TOGGLE_AUTOFIRE))
	{
		extern int toggle_autofire(void);

		int autofire_enable = toggle_autofire();
		usrintf_showmessage("Autofire %s",(autofire_enable ? "On" : "Off"));
	}

	/* if the user pressed F1 and this is a lightgun game, toggle the crosshair */
	if (input_ui_pressed(IPT_UI_TOGGLE_CROSSHAIR))
	{
		drawgfx_toggle_crosshair();
	}

	/* add the FPS counter */
	ui_display_fps(bitmap);

#ifdef FRAMES_COUNTER
	if ((!code_pressed(KEYCODE_LSHIFT) && !code_pressed(KEYCODE_RSHIFT)) && (input_ui_pressed(IPT_UI_SHOW_TOTAL_FT)))
		total_frame_counter = (total_frame_counter == 0) ? 1 : 0;

	if (total_frame_counter)
		showtotalframes(bitmap);
#endif /*FRAMES_COUNTER*/

#ifdef LOG_WAV
	if (input_ui_pressed(IPT_UI_TOGGLE_LOGWAVE))
	{
		if (wave_loging == 0)
		{
			osd_start_log_wave();
			wave_loging = 1;
			wave_seconds = 0;
			wave_minutes = 0;
		}
		else
		{
			osd_stop_log_wave();
			wave_loging = 0;
		}
	}

	if (wave_loging)
	{
		static int counter = 0;
		char buf[16];
		char buf2[16];
		extern char wavfname[];	/* in windows/sound.c */

		if (++counter == 60){
			counter = 0;
			wave_seconds++;

			if (wave_seconds == 60)
			{
				wave_seconds = 0;
				wave_minutes++;
			}
		}

		if (counter < 40)
			ui_text(bitmap, "REC", 0, 0);

		if (code_pressed(KEYCODE_LSHIFT)){
			sprintf(buf, "[Name: %s.wav]", wavfname);
			ui_text(bitmap, buf, 0, uirotcharheight);
			sprintf(buf2, "[Time: %02d:%02d]", wave_minutes, wave_seconds);
			ui_text(bitmap, buf2, 0, uirotcharheight * 2);
		}
	}		
#endif /*LOG_WAV*/
 
#ifdef USE_BOOST
	if (boost || toggle_boost)
		ui_text(bitmap, "Boost", 0, 0);
#endif /* USE_BOOST */

	mame_pause(setup_selected);
	
	return 0;
}


void init_user_interface(void)
{
	extern int snapno;	/* in common.c */

	snapno = 0; /* reset snapshot counter */
#ifdef KAILLERA
	endemu = 0;
	win_trying_to_quit = 0;
#endif

	/* clear the input memory */
	while (code_read_async() != CODE_NONE) {};

#ifdef INP_CAPTION
	next_caption_frame = -1;
	caption_timer = 0;
#endif /* INP_CAPTION */

	setup_menu_init();
	setup_selected = 0;

	onscrd_init();
	osd_selected = 0;

	jukebox_selected = -1;

#ifdef FRAMES_COUNTER
	total_frame_counter = 0;
	Total_FT_Position = 0;
#endif /*FRAMES_COUNTER*/
#ifdef LOG_WAV
	wave_loging = 0;
	wave_seconds = 0;
	wave_minutes = 0;	
#endif /*LOG_WAV*/		

	single_step = 0;
	auto_pause = 0;
	scroll_reset = 1;

	displaymessagewindow_under = 0;

	lhw = strlen(ui_getstring(UI_lefthilight));
	rhw = strlen(ui_getstring(UI_righthilight));
	uaw = strlen(ui_getstring(UI_uparrow));
	daw = strlen(ui_getstring(UI_downarrow));
	law = strlen(ui_getstring(UI_leftarrow));
	raw = strlen(ui_getstring(UI_rightarrow));

	flash_all_values_and_buffers();

#ifdef AUTO_PAUSE_PLAYBACK
	if (playback && options.auto_pause_playback)
		ui_auto_pause();
#endif /* AUTO_PAUSE_PLAYBACK */
}

int onscrd_active(void)
{
	return osd_selected;
}

int setup_active(void)
{
	return setup_selected;
}

#if defined(__sgi) && !defined(MESS)

/* Return if the game is paused or not */
int is_game_paused(void)
{
	return game_paused;
}

#endif

void ui_auto_pause(void)
{
	auto_pause = 1;
}

#ifdef MAME_AVI
int get_single_step(void) { return single_step; }	// 신규 추가 함수
int usrintrf_message_ok_cancel(struct mame_bitmap *bitmap, void *str)
{
	int ret = FALSE;
	mame_pause(1);

	while (1)
	{
		displaymessage(bitmap, str);

		update_video_and_audio();

		if (input_ui_pressed(IPT_UI_CANCEL))
			break;

		if (input_ui_pressed(IPT_UI_SELECT))
		{
			ret = TRUE;
			break;
		}
	}

	mame_pause(0);

	return ret;
}
#endif /* MAME_AVI */

