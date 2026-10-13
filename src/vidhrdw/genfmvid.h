/*Any fixes for the games contained in this driver should be forwarded to MAGENET
(www.magesrc.sf.net/index.php)*/

#ifndef INC_GENERIC_FRUITY_VIDEO
#define INC_GENERIC_FRUITY_VIDEO

VIDEO_START(  generic_fruitmachine );
VIDEO_UPDATE( generic_fruitmachine );
PALETTE_INIT( generic_fruitmachine );

void gfx_printf(struct mame_bitmap *bmp, struct GfxElement *font, int x, int y, const char *fmt, ... );

#endif
