/*********************************************************************

  ui_font.h

  日本語文字列を処理する補助マクロ及び関数を提供します。

*********************************************************************/

#ifndef UI_FONT_H
#define UI_FONT_H

// USE uifont_decodechar instead of those macros.
//#define iszen1(c)  ((c) >= 0x81 && (c) <= 0xfe)
//#define iszen2(c)  ((c) >= 0x40 && (c) <= 0xfe)

int uifont_buildfont(void);
int uifont_need_font_warning(void);
void uifont_freefont(void);
int uifont_decodechar(unsigned char *s, unsigned short *code);
int uifont_drawfont(struct mame_bitmap *bitmap, const char *s,
                    int sx, int sy, int color);

#ifdef UI_COLOR_DISPLAY
void convert_command_move(char *buf);
#endif /* UI_COLOR_DISPLAY */

#endif
