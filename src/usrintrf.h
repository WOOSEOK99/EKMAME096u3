/*********************************************************************

  usrintrf.h

  Functions used to handle MAME's crude user interface.

*********************************************************************/

#ifndef USRINTRF_H
#define USRINTRF_H

struct MEMCARDinterface
{
	int (*create)(int);
	int (*load)(int);
	void (*save)(void);
	void (*eject)(void);
};
extern struct MEMCARDinterface memcard_intf;

extern int	mcd_action;
extern int	mcd_number;
extern int	memcard_status;
extern int	memcard_number;
extern int	memcard_manager;

#define init_memcard() memset(&memcard_intf, 0, sizeof(struct MEMCARDinterface))

struct DisplayText
{
	const char *text;	/* 0 marks the end of the array */
	int color;	/* see #defines below */
	int x;
	int y;
};

#define UI_COLOR_NORMAL 0	/* white on black text */
#define UI_COLOR_INVERSE 1	/* black on white text */

#ifdef UI_TITLE_MENU
#define UI_COLOR_BOLD  2	/* white bold text */
#define UI_COLOR_BLACK 3	/* black bold text */
#endif /* UI_TITLE_MENU */

#ifdef CMD_LIST
#define SCR_PAGE_MASK  0xffffff
#define SCR_NEXT_PAGE  0x1000000
#define SCR_PREV_PAGE  0x10000000
#endif /* CMD_LIST */

#define SEL_BITS 12		/* main menu selection mask */
#define SEL_BITS2 4		/* submenu selection masks */
#define SEL_MASK ((1<<SEL_BITS)-1)
#define SEL_MASK2 ((1<<SEL_BITS2)-1)

extern UINT8 ui_dirty;

void pick_uifont_colors(void);
void displaytext(struct mame_bitmap *bitmap,const struct DisplayText *dt);

void ui_drawchar(struct mame_bitmap *dest, int ch, int color, int sx, int sy);
void ui_text(struct mame_bitmap *bitmap,const char *buf,int x,int y);
void ui_drawbox(struct mame_bitmap *bitmap,int leftx,int topy,int width,int height);

#ifdef UI_TITLE_MENU
int ui_displaymessagewindow(struct mame_bitmap *bitmap,const char *text,const char *title);
#else
int ui_displaymessagewindow(struct mame_bitmap *bitmap,const char *text);
#endif /* UI_TITLE_MENU */

int ui_displaymessagewindow_under(struct mame_bitmap *bitmap,const char *text);

#ifdef UI_TITLE_MENU
void ui_displaymenu(struct mame_bitmap *bitmap,const char **items,const char **subitems,char *flag,const char *title,int selected,int arrowize_subitem);
#else
void ui_displaymenu(struct mame_bitmap *bitmap,const char **items,const char **subitems,char *flag,int selected,int arrowize_subitem);
#endif /* UI_TITLE_MENU */

void ui_display_fps(struct mame_bitmap *bitmap);
int show_font_warning(struct mame_bitmap *bitmap);
int showcopyright(struct mame_bitmap *bitmap);
int showgamewarnings(struct mame_bitmap *bitmap);
int showgameinfo(struct mame_bitmap *bitmap);
void set_ui_visarea (int xmin, int ymin, int xmax, int ymax);


void init_user_interface(void);
int handle_user_interface(struct mame_bitmap *bitmap);

void ui_show_fps_temp(double seconds);
void ui_show_fps_set(int show);
int ui_show_fps_get(void);

void ui_show_profiler_set(int show);
int ui_show_profiler_get(void);

int onscrd_active(void);
int setup_active(void);

#if defined(__sgi) && ! defined(MESS)
int is_game_paused(void);
#endif

void switch_ui_orientation(struct mame_bitmap *bitmap);
void switch_debugger_orientation(struct mame_bitmap *bitmap);
void switch_true_orientation(struct mame_bitmap *bitmap);

#ifdef __GNUC__
void CLIB_DECL usrintf_showmessage(const char *text,...)
      __attribute__ ((format (printf, 1, 2)));

void CLIB_DECL usrintf_showmessage_secs(int seconds, const char *text,...)
      __attribute__ ((format (printf, 2, 3)));
#else
void CLIB_DECL usrintf_showmessage(const char *text,...);
void CLIB_DECL usrintf_showmessage_secs(int seconds, const char *text,...);
#endif

void ui_auto_pause(void);
#endif
