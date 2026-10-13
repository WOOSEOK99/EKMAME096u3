/*****************************************************************************

  ui_font.c

  Original version by NJ (NMAME)

  Korean & Command data version by AL.ceio www.mbox32.com
*****************************************************************************/

#include <windows.h>
#include "driver.h"
#include "artwork.h"
#include "ui_pal.h"
#include "ui_lang.h"

// ** Font Code Address Process Part **
// Game Command Font Total Color Layout & Total Font data
#define COLOR_BUTTONS	(92)
#define MAX_FONT_DATA	(17631)

static UINT16 _uifonttable[65536];
static UINT32 colortable[MAX_COLORTABLE];
static int need_font_warning;
static unsigned char *fontdata_db;

/****************************************************************************/

/* in artwork.c */
static int gamescale;

/****************************************************************************/

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
	ui_markdirty - mark a raw rectangle dirty
-------------------------------------------------*/

INLINE void ui_markdirty(const struct rectangle *rect)
{
	extern UINT8 ui_dirty;

	artwork_mark_ui_dirty(rect->min_x, rect->min_y, rect->max_x, rect->max_y);
	ui_dirty = 5;
}



static unsigned char* load_fontdata(void)
{
	FILE *fd;
	int fontsize = ui_lang_info[options.langcode].numchars * 24;
	unsigned char *fontdata;

	need_font_warning = 0;

	if (!fontsize)
		return NULL;

	if ((fd = fopen(_("font\\fontname.fnt"), "rb")) == NULL)
	{
		need_font_warning = 1;
		return NULL;
	}

	fontdata = malloc(fontsize);
	memset(fontdata,0,fontsize);
	fread(fontdata, fontsize, 1, fd);
	return fontdata;
}


int uifont_need_font_warning(void)
{
	return need_font_warning;
}

static UINT16 uifont_buildtable(UINT16 code)
{
	UINT8 c1, c2;
	c1 = (code >> 8) & 0xff;
	c2 = code & 0xff;
	
	switch (options.langcode)
	{
		case UI_LANG_JA_JP:
			if (code >= 0x8140 && code <= 0x9ffc)			//Shift-JIS (Japanese) c1: (0x81-0x9F, 0xE0-0xFC) c2: (0x40-0xFC)
				code = ((c1 - 0x81) << 8) + (c2 - 0x40);
			else if (code >= 0xe040 && code <= 0xfcfc)
				code = ((c1 - 0xe0 + (0xa0 - 0x81)) << 8) + (c2 - 0x40);
			else
				return 0x60;
			return (code - ((code >> 8) & 0xff) * 0x40);
			break;

		case UI_LANG_ZH_CN:
			if(0x81 <= c1 && c1 <= 0xFE)			//GBK c1: (0x81-0xFE)  c2: (0x40-0x7E,0x80-0xFE), Kana code a4a1 - a6fe
			{
				if( 0x40 <= c2 && c2 <= 0x7E )
					code = (c1 - 0x81) * 0xBE + (c2 - 0x40);
				else if( 0x80 <= c2 && c2 <= 0xFE )
					code = (c1 - 0x81) * 0xBE + (c2 - 0x80 + 0x3F);
			}
			else
				return 0x60;
			break;

		case UI_LANG_ZH_TW:
			if(0xA1 <= c1 && c1 <= 0xF9)			//BIG5 (Traditional Chinese) c1: (0xA1-0xF9) c2: (0x40-0x7E, 0xA1-0xFE)
			{
				if( 0x40 <= c2 && c2 <= 0x7E )
					code = (c1 - 0xA1) * 0x9D + (c2 - 0x40);
				else if( 0xA1 <= c2 && c2 <= 0xFE )
					code = (c1 - 0xA1) * 0x9D + (c2 - 0xA1 + 0x3F);
			}
			else
				return 0x60;
			break;

		case UI_LANG_KO_KR:
	// ** Font Code Address Process Part **
	// KSC5601 Code Font Area: c1: (0xa1-0xfd)  c2: (0xa1-0xfe)
	if ((0xa1 <= c1 && c1 <= 0xfd) && (c2 >= 0xa1))
	{
		if(0xa1 <= c1 && c1 <= 0xac)		// Special Characters Area
			code = (c1 - 0xa1 + 4) * 94 + (c2 - 0xa1);
		else if(0xad <= c1 && c1 <= 0xaf)	// Game Command Font Area: 3Blocks / 282 fonts
			code = (c1 - 0xad) * 94 + (c2 - 0xa1);
		else if(0xb0 <= c1 && c1 <= 0xc8)	// Korean Characters Area: 2,350 fonts
			code = (c1 - 0xa1 + 1) * 94 + (c2 - 0xa1);
		else if(c1 == 0xc9)					// Game Command Font Area: 1Block / 94 fonts
			code = (c1 - 0xc9 + 3) * 94 + (c2 - 0xa1);
		else if(0xca <= c1 && c1 <= 0xfe )	// Chinese Characters Area: 4,888 fonts
			code = (c1 - 0xa1) * 94 + (c2 - 0xa1);
	}

	// KSX1001 Code Expanded Font Area A : c1: (0x81-0xa0)  c2: (0x41-0x5a,0x61-0x7a,0x8a-0xfe)
	else if (0x81 <= c1 && c1 <= 0xa0)
	{
		if(0x41 <= c2 && c2 <= 0x5a)
			code = (c1 - 0x81) * 178 + (c2 - 0x41) + 8742;
		else if( 0x61 <= c2 && c2 <= 0x7a )
			code = (c1 - 0x81) * 178 + (c2 - 0x61 + 0x1a) + 8742;
		else if( 0x81 <= c2 && c2 <= 0xfe )
			code = (c1 - 0x81) * 178 + (c2 - 0x81 + 0x34) + 8742;
	}

	// KSX1001 Code Expanded Font Area B : c1: (0xa1-0xc6)  c2: (0x41-0x5a,0x61-0x7a,0x81-0xa0)
	else if ((0xa1 <= c1 && c1 <= 0xc6) && (c2 <= 0xa0))
	{
		if( 0x41 <= c2 && c2 <= 0x5a )
			code = (c1 - 0xa1) * 84 + (c2 - 0x41) + 14438;
		else if( 0x61 <= c2 && c2 <= 0x7a )
			code = (c1 - 0xa1) * 84 + (c2 - 0x61 + 0x1a) + 14438;
		else if( 0x81 <= c2 && c2 <= 0xa0 )
			code = (c1 - 0xa1) * 84 + (c2 - 0x81 + 0x34) + 14438;
	}
			else
				return 0x60;
			break;

		default:
			// ** Font Code Address Process Part **
      // Game Command Font Area: c1: (0x90-0x92)  c2: (0x81-0xfe)
        if ((0x90 <= c1 && c1 <= 0x92) && (0x81 <= c2 && c2 <= 0xfe))
                code = (c1 - 0x90) * 126 + (c2 - 0x81);	
			else
					return 0x60;
	}

	return(code);
}

int uifont_buildfont(void)
{

	static UINT8 font_data6x8[] =
	{
  #include "font/asc6x8.dat"
	};
	
	// support Polish and Hungarian
	static UINT8 cp1250_fontdata6x8[] =
	{
  #include "font/cp1250_default6x8.dat"
	};

	static UINT8 font_data12x8[] =
	{
  #include "font/cmd12x8.dat"
  #include "font/end12x8.dat"
	};
	
	static unsigned char fontdata6x12[] =
	{
  #include "font/default6x12.dat"
	};
	
	static unsigned char font_data6x12[] =
	{
		#include "font/asc6x12.dat"
	};
	
	static unsigned char font_data12x12[] =
	{
		#include "font/cmd12x12.dat"
		#include "font/ksc12x12.dat"
		#include "font/kse12x12.dat"
		#include "font/end12x12.dat"
	};
	
	static const struct GfxLayout fontlayout6x8 =
	{
		6,8,
		256,
		1,
		{ 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
		8*8
	};

	static const struct GfxLayout fontlayout12x8 =
	{
		12,8,
		COLOR_BUTTONS,
		1,
		{ 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
		{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
		8*16
	};

	static struct GfxLayout fontlayout6x12 =
	{
		6,12,	/* 6*12 characters */
		256,    /* 256 characters */
		1,	/* 1 bit per pixel */
		{ 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },	/* straightforward layout */
		{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
		8*12	/* every char takes 8 consecutive bytes */
	};
	static struct GfxLayout fontlayout12x12 =
	{
		12,12,
		0,	// total number set later
		1,
		{ 0 },
		{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },
		{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,8*16,9*16,10*16,11*16 },
		16*12
	};

	struct GfxLayout layout;
	struct GfxLayout layout1;
	struct GfxLayout layout2;
	unsigned char *fontdata_sb;
	static int fontlangcode = -1;
	static int use_8linefont = 1;
	int temp, i;

	#define MAX_UIFONT_SIZE 12			/* max(width,height) */
	#define UIFONT_LAYOUT_x(type,i)		type.xoffset[i]
	#define UIFONT_LAYOUT_y(type,i)		type.yoffset[i]
	#define UIFONT_LAYOUT1(offset,i)	UIFONT_LAYOUT_##offset(fontlayout6x12,  i)
	#define UIFONT_LAYOUT2(offset,i)	UIFONT_LAYOUT_##offset(fontlayout12x12, i)

	uifont_freefont();

if (options.langcode == UI_LANG_KO_KR){
	fontdata_sb = font_data6x12;
	fontdata_db = font_data12x12;

	layout = fontlayout6x12;
	layout1 = fontlayout6x12;
	layout2 = fontlayout12x12;
	layout2.total = MAX_FONT_DATA;
 }
else {
	if (fontlangcode != options.langcode)
	{
		if (!use_8linefont && fontdata_db)
			free(fontdata_db);

		if ((fontdata_db = load_fontdata()) == NULL)
			use_8linefont = 1;
		else
			use_8linefont = 0;

		fontlangcode = options.langcode;
	}
 
	if (use_8linefont)
	{
		switch (options.langcode)
		{
		case UI_LANG_PL_PL:
		case UI_LANG_HU_HU:
			fontdata_sb = cp1250_fontdata6x8;
			break;
		default:
			fontdata_sb = font_data6x8;
			break;
		}

		fontdata_db = font_data12x8;

		layout = fontlayout6x8;
		layout1 = fontlayout6x8;
		layout2 = fontlayout12x8;
	}
	else
	{
		fontdata_sb = fontdata6x12;

		layout = fontlayout6x12;
		layout1 = fontlayout6x12;
		layout2 = fontlayout12x12;
		layout2.total = ui_lang_info[options.langcode].numchars;
	}
}

gamescale = options.artwork_res;
if (gamescale == 2)
{
	if (Machine->uiwidth >= 420)
	{
		if (Machine->ui_orientation & ORIENTATION_SWAP_XY)
		{
			/* pixel double vertically */
			for (i = 0; i < layout.height; i++)
				layout.yoffset[i*2+0] = layout.yoffset[i*2+1] = UIFONT_LAYOUT1(x,i);
			layout.height *= 2;
		}
		else
		{
			/* pixel double horizontally */
			for (i = 0; i < layout.width; i++)
				layout.xoffset[i*2+0] = layout.xoffset[i*2+1] = UIFONT_LAYOUT1(y,i);
			layout.width *= 2;
		}
	}

	if (Machine->uiheight >= 420)
	{
		if (Machine->ui_orientation & ORIENTATION_SWAP_XY)
		{
			/* pixel double horizontally */
			for (i = 0; i < layout.width; i++)
				layout.xoffset[i*2+0] = layout.xoffset[i*2+1] = UIFONT_LAYOUT1(x,i);
			layout.width *= 2;
		}
		else
		{
			/* pixel double vertically */
			for (i = 0; i < layout.height; i++)
				layout.yoffset[i*2+0] = layout.yoffset[i*2+1] = UIFONT_LAYOUT1(y,i);
			layout.height *= 2;
		}
	}
}

	/* decode a straight on version for games */
	Machine->uifont = decodegfx(fontdata_sb, &layout);
	if (!Machine->uifont)
		return 0;

	Machine->uifontwidth = layout.width;
	Machine->uifontheight = layout.height;

	Machine->uifont->colortable = colortable;
	Machine->uifont->total_colors = MAX_COLORTABLE;

	/* apply swappage */
	if (Machine->ui_orientation & ORIENTATION_SWAP_XY)
	{
		for (i=0; i<2*MAX_UIFONT_SIZE; i++)
			temp = layout1.xoffset[i], layout1.xoffset[i] = layout1.yoffset[i], layout1.yoffset[i] = temp;

		temp = layout1.width;
		layout1.width = layout1.height;
		layout1.height = temp;

		for (i=0; i<2*MAX_UIFONT_SIZE; i++)
			temp = layout2.xoffset[i], layout2.xoffset[i] = layout2.yoffset[i], layout2.yoffset[i] = temp;

		temp = layout2.width;
		layout2.width = layout2.height;
		layout2.height = temp;
	}

	/* apply xflip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_X)
	{
		for (i = 0; i < layout1.width/2; i++)
			temp = layout1.xoffset[i], layout1.xoffset[i] = layout1.xoffset[layout1.width - 1 - i], layout1.xoffset[layout1.width - 1 - i] = temp;

		for (i = 0; i < layout2.width/2; i++)
			temp = layout2.xoffset[i], layout2.xoffset[i] = layout2.xoffset[layout2.width - 1 - i], layout2.xoffset[layout2.width - 1 - i] = temp;
	}

	/* apply yflip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_Y)
	{
		for (i = 0; i < layout1.height/2; i++)
			temp = layout1.yoffset[i], layout1.yoffset[i] = layout1.yoffset[layout1.height - 1 - i], layout1.yoffset[layout1.height - 1 - i] = temp;

		for (i = 0; i < layout2.height/2; i++)
			temp = layout2.yoffset[i], layout2.yoffset[i] = layout2.yoffset[layout2.height - 1 - i], layout2.yoffset[layout2.height - 1 - i] = temp;
	}

gamescale = options.artwork_res;
if (gamescale == 2)
{
	/* pixel double horizontally */
	if (Machine->uiwidth >= 420)
	{
		for (i = 0; i < layout1.width; i++)
			layout1.xoffset[i*2+0] = layout1.xoffset[i*2+1] = UIFONT_LAYOUT1(x,i);
		layout1.width *= 2;

		for (i = 0; i < layout2.width; i++)
			layout2.xoffset[i*2+0] = layout2.xoffset[i*2+1] = UIFONT_LAYOUT2(x,i);
		layout2.width *= 2;
	}

	/* pixel double vertically */
	if (Machine->uiheight >= 420)
	{
		for (i = 0; i < layout1.height; i++)
			layout1.yoffset[i*2+0] = layout1.yoffset[i*2+1] = UIFONT_LAYOUT1(y,i);
		layout1.height *= 2;

		for (i = 0; i < layout2.height; i++)
			layout2.yoffset[i*2+0] = layout2.yoffset[i*2+1] = UIFONT_LAYOUT2(y,i);
		layout2.height *= 2;
	}
}

	Machine->uirotfont = decodegfx(fontdata_sb, &layout1);
	Machine->uirotfont2 = decodegfx(fontdata_db, &layout2);
	if (!Machine->uirotfont || !Machine->uirotfont2)
		return 0;

	Machine->uirotfont->colortable = colortable;
	Machine->uirotfont->total_colors  = MAX_COLORTABLE;

	Machine->uirotfont2->colortable = colortable;
	Machine->uirotfont2->total_colors = MAX_COLORTABLE;

	for (i = 0x4081; i < 0xfffe; i++)
		_uifonttable[i] = uifont_buildtable(i) + 256;

	return 1;
}


void uifont_freefont(void)
{
	if (Machine->uifont)
	{
		freegfx(Machine->uifont);
		Machine->uifont = NULL;
	}
	if (Machine->uirotfont)
	{
		freegfx(Machine->uirotfont);
		Machine->uirotfont = NULL;
	}
	if (Machine->uirotfont2)
	{
		freegfx(Machine->uirotfont2);
		Machine->uirotfont2 = NULL;
	}
}


int uifont_decodechar(unsigned char *s, unsigned short *code)
{
	unsigned char c1 = *(unsigned char *)s;
	unsigned char c2 = *(unsigned char *)(s + 1);
	int isDBC;
#ifdef CMD_PLUS
	#define NULL_SPACE1	'\b'
	// Remove to Null Space Character
	if (c1 == NULL_SPACE1)
		return 3;
#endif /* CMD_PLUS */
	switch (options.langcode)
	{
	case UI_LANG_JA_JP:
		isDBC = (((c1 >= 0x81 && c1 <= 0x9f) || (c1 >= 0xe0 && c1 <= 0xfc)) && (c2 >= 0x40 && c2 <= 0xfc));
		break;
	case UI_LANG_ZH_CN:
		isDBC = ((c1 >= 0x81 && c1 <= 0xfe) && ((c2 >= 0x40 && c2 <= 0x7e) || (c2 >= 0x80 && c2 <= 0xfe)));
		break;
	case UI_LANG_ZH_TW:
		isDBC = ((c1 >= 0xa1 && c1 <= 0xf9) && ((c2 >= 0x40 && c2 <= 0x7e) || (c2 >= 0xa1 && c2 <= 0xfe)));
		break;
	case UI_LANG_KO_KR:
		isDBC = ((c1 >= 0x81 && c1 <= 0xfe) && ((c2 >= 0x41 && c2 <= 0x5a) || (c2 >= 0x61 && c2 <= 0x7a) || (c2 >= 0x81 && c2 <= 0xfe)));
		break;
	default:
        // ** Font Code Address Process Part **
        // Game Command Font Area: c1: (0x90-0x92)  c2: (0x81-0xfe)
        isDBC = ((0x90 <= c1 && c1 <= 0x92) && (0x81 <= c2 && c2 <= 0xfe));		
	}

	if (isDBC)
	{
		*code = _uifonttable[(c1 << 8) | c2];

		if (*code == 0)
			*code = _uifonttable[0x817f];

		return 2;
	}

	*code = c1;
	return 1;
}

int uifont_drawfont(struct mame_bitmap *bitmap, const char *s, int sx,
		    int sy, int color)
{
	struct rectangle bounds;
	int x, y, wrapped, next_sy, increment, uifontwidth;
#ifdef UI_COLOR_DISPLAY
	int white = 0;
#ifdef UI_TITLE_MENU 
	int white2 = 0;
#endif /* UI_TITLE_MENU */ 

#ifdef CMD_PLUS  
  #include "cmd_plus/cmd_table.c"
  int white3 = 0;
	int btn_line = LINE_FONT;
	int btn_logo = FIRST_LOGO_FONT;
	int btn_deco = DECO_FONT;
#endif /* CMD_PLUS */

	int btn_offset, _code;
#endif /* UI_COLOR_DISPLAY */
	unsigned short code;
	unsigned char *c;
	struct GfxElement *uifont;

	c       = (unsigned char *)s;
	x       = sx;
	y       = sy;
	code    = 0;
	next_sy = 0;

#ifdef UI_COLOR_DISPLAY
	if (color == UI_COLOR_INVERSE)
	{
#ifdef CMD_PLUS
		white3 = Machine->uifont->colortable[FONT_COLOR_NORMAL];
#else
		white = Machine->uifont->colortable[FONT_COLOR_NORMAL];
#endif /* CMD_PLUS */
		Machine->uifont->colortable[FONT_COLOR_NORMAL] = Machine->uifont->colortable[FONT_COLOR_SPECIAL];
	}
#endif /* UI_COLOR_DISPLAY */

	while (*c)
	{
		wrapped = 0;

		increment = uifont_decodechar(c, &code);
#ifdef CMD_PLUS
		if (increment == 3)
		{
			c++;
			continue;
		}
#endif /* CMD_PLUS */
		if (*c == '\n')
		{
			x = sx;
			y += Machine->uifontheight + 1;
			next_sy += Machine->uifontheight;
			c++;
			continue;
		}
		else
		{
			if (code > 0x00ff && Machine->uirotfont2 != NULL)
			{
				uifont      = Machine->uirotfont2;
				uifontwidth = Machine->uifontwidth * 2;
			}
			else
			{
				uifont      = Machine->uirotfont;
				uifontwidth = Machine->uifontwidth;
			}
		}

		if (y >= Machine->uiheight)
			return next_sy;

		if (!wrapped)
		{
#ifdef UI_COLOR_DISPLAY
			pen_t col = 0;
#endif /* UI_COLOR_DISPLAY */

			if (code > 0x00ff)
			{
				code -= 256;

#ifdef UI_COLOR_DISPLAY


#ifdef CMD_PLUS
if(options.langcode == UI_LANG_KO_KR){
				if (code > 0 && code < COLOR_BUTTONS)
					col = color_table[code];
				else if (code >= btn_line && code <= btn_line + 68)
					col = BUTTON_COLOR_BLUE;
				else if (code >= (btn_logo) && code <= (btn_logo + 7))
					col = FIRST_LOGO_COLOR;
				else if (code >= (btn_logo + 8) && code <= (btn_logo + 15))
					col = SECOND_LOGO_COLOR;
				else if (code >= btn_deco && code <= btn_deco + 2)
					col = DECO_COLOR;
				else if (code >= btn_deco + 3 && code <= btn_deco + 5)
					col = DECO_ZZIRY;
				}
#endif /* CMD_PLUS */

				btn_offset = 0;
				_code = code;
				
				switch (options.langcode)
				{
				case UI_LANG_JA_JP:
					btn_offset = 895; //897-2
					break;
				case UI_LANG_ZH_CN:
					btn_offset = 7916;
					break;
				case UI_LANG_ZH_TW:
					btn_offset = 407;
					break;
				}
				
				_code -= btn_offset;

				if (_code > 0 && _code < COLOR_BUTTONS)
					col = color_table[_code];

				if (col)
				{
					white = Machine->uifont->colortable[FONT_COLOR_NORMAL];
					Machine->uifont->colortable[FONT_COLOR_NORMAL] = Machine->uifont->colortable[col];
				}
#endif /* UI_COLOR_DISPLAY */
			}


#ifdef UI_TITLE_MENU
			if ((color == UI_COLOR_BOLD || color == UI_COLOR_BLACK) && col == 0)
			{
				white2 = Machine->uifont->colortable[FONT_COLOR_NORMAL];
				if (color == UI_COLOR_BOLD)
					Machine->uifont->colortable[FONT_COLOR_NORMAL] = Machine->uifont->colortable[FONT_COLOR_LIGHT];
				else
					Machine->uifont->colortable[FONT_COLOR_NORMAL] = Machine->uifont->colortable[FONT_COLOR_DARK];

				bounds.min_x = x + Machine->uixmin + 1;
				bounds.min_y = y + Machine->uiymin;
				bounds.max_x = x + Machine->uixmin + uifontwidth - 1;
				bounds.max_y = y + Machine->uiymin + Machine->uifontheight - 1;
				ui_rot2raw_rect(&bounds);

				drawgfx(bitmap,
					uifont,
					code,
					UI_COLOR_NORMAL,
					0,
					0,
					bounds.min_x,
					bounds.min_y,
					0,
					TRANSPARENCY_PEN,
					0);

				Machine->uifont->colortable[FONT_COLOR_NORMAL] = white2;
			}

			if (color == UI_COLOR_BLACK && col == 0)
			{
				white2 = Machine->uifont->colortable[FONT_COLOR_NORMAL];
				Machine->uifont->colortable[FONT_COLOR_NORMAL] = Machine->uifont->colortable[FONT_COLOR_BLANK];
			}
#endif /* UI_TITLE_MENU */


			bounds.min_x = x + Machine->uixmin;
			bounds.min_y = y + Machine->uiymin;
			bounds.max_x = x + Machine->uixmin + uifontwidth - 1;
			bounds.max_y = y + Machine->uiymin + Machine->uifontheight - 1;
			ui_rot2raw_rect(&bounds);

			drawgfx(bitmap,
			        uifont,
			        code,
#ifdef UI_COLOR_DISPLAY
			        UI_COLOR_NORMAL,
#else /* UI_COLOR_DISPLAY */
				color,
#endif /* UI_COLOR_DISPLAY */
			        0,
			        0,
			        bounds.min_x,
			        bounds.min_y,
			        0,
#ifdef UI_COLOR_DISPLAY
			        TRANSPARENCY_PEN,
#else /* UI_COLOR_DISPLAY */
#ifdef TRANS_UI
			        TRANSPARENCY_PEN,
#else /* TRANS_UI */
			        TRANSPARENCY_NONE,
#endif /* TRANS_UI */
#endif /* UI_COLOR_DISPLAY */
			        0);

#ifdef UI_TITLE_MENU
			if (color == UI_COLOR_BLACK && col == 0)
			{
				Machine->uifont->colortable[FONT_COLOR_NORMAL] = white2;
			}
#endif /* UI_TITLE_MENU */

			ui_markdirty(&bounds);

			x += uifontwidth;

			while (increment)
			{
				c++;
				increment--;
			}
			
#ifdef UI_COLOR_DISPLAY
			if (col)
				Machine->uifont->colortable[FONT_COLOR_NORMAL] = white;
#endif /* UI_COLOR_DISPLAY */
		}
	}

#ifdef UI_COLOR_DISPLAY
	if (color == UI_COLOR_INVERSE)
#ifdef CMD_PLUS
		Machine->uifont->colortable[FONT_COLOR_NORMAL] = white3;
#else
		Machine->uifont->colortable[FONT_COLOR_NORMAL] = white;
#endif /* CMD_PLUS */
#endif /* UI_COLOR_DISPLAY */

	return next_sy;
}

#ifdef UI_COLOR_DISPLAY
#ifdef CMD_PLUS
void convert_command_move(char *buf)
{
	unsigned char *c = (unsigned char *)buf;

	int i = 0;
  int c1 = 0;
  int c2 = 0;
  int s1 = 0;
  
  // ** Font Code Address Process Part **
	#include "cmd_plus/cmd_plus.c"
	
	switch (options.langcode)
	{
		
		case UI_LANG_ZH_CN:
			c1 = 0xaa;
			break;

		case UI_LANG_ZH_TW:
			c1 = 0xa3;
			break;

		case UI_LANG_JA_JP:
			c1 = 0x85;
			break;
			
		case UI_LANG_KO_KR:	
			c1 = 0xad;	// Game Command Font 1st Byte Address
	    c2 = 0xa1;	// Game Command Font 2nd Byte Address
	    s1 = 94;	// Game Command Font Block
		  break;
		  
		default:
			c1 = 0x90;	// Game Command Font 1st Byte Address
	    c2 = 0x81;	// Game Command Font 2nd Byte Address
	    s1 = 126;	// Game Command Font Block
		  break;
		  
			
   if(options.langcode == UI_LANG_ZH_CN || UI_LANG_ZH_TW || UI_LANG_JA_JP){
     #include "cmd_plus/decode_charb.c"
			return;
   }
  }
switch (options.langcode)
 {
		case UI_LANG_KO_KR: 
		default :
		while (*c)
			{
				unsigned short code;
 
				if (uifont_decodechar(c, &code) == 2)
				{
					c+=2;
				}
				else
				{
   		 #include "cmd_plus/cmd_move.c"
		
					else if (*c == '\\' && *(c+1) == 'n')
					{
						*c     = ' ';
						*(c+1) = '\n';
						c+=2;
					}
					else
						c++;
				}
			}
			break;
	
		 case UI_LANG_ZH_CN:
		 case UI_LANG_ZH_TW:
		 case UI_LANG_JA_JP:
	 		while (*c)
				{
					unsigned short code;

						if (uifont_decodechar(c, &code) == 2)
						{
							c+=2;
						}
						else				
						{
 					      #include "cmd_plus/decode_charn.c"

						else if (*c == '\\' && *(c+1) == 'n')
						{
							*c     = ' ';
							*(c+1) = '\n';
							c+=2;
						}
						else
							c++;
						}
	
				}
				break;
	}			
} 
#endif /* CMD_PLUS */
#endif /* UI_COLOR_DISPLAY */


