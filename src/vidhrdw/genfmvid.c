/*Any fixes for this driver should be forwarded to MAGENET www.magesrc.sf.net */

#include "driver.h"

#include "genfmvid.h"
#include "machine/lamps.h"
#include "machine/vacfdisp.h"
#include "machine/steppers.h"
#include "dbgfonts/m0813fnt.c"// font for displaying alpha display (vfd), doesn't contain pound sign?

static struct GfxElement *font;	// pointer to font gfx

///////////////////////////////////////////////////////////////////////////

void gfx_printf(struct mame_bitmap *bmp, struct GfxElement *fnt, int x, int y, const char *fmt, ... )
{
  va_list marker;
  char buffer[256];

  va_start( marker, fmt );
  vsprintf(buffer,  fmt, marker);
  va_end( marker );

  if ( fnt == NULL ) fnt = font;

  if ( fnt )
  {
	int i;
  
	i = 0;
	while ( buffer[i] && i < 255 )
	{
	  drawgfx(bmp, fnt, buffer[i], 7, 0, 0, x, y, 0, TRANSPARENCY_NONE, 0);
	  x += fnt->width;
	  i++;
	}
  }  
}

// video initialisation ///////////////////////////////////////////////////

VIDEO_START( generic_fruitmachine )
{
  font = decodegfx(fontdata,&fontlayout);

  font->colortable   = Machine->debug_remapped_colortable;
  font->total_colors = 16;

  return 0;
}

// video update ///////////////////////////////////////////////////////////

VIDEO_UPDATE( generic_fruitmachine )
{
  int x, y,offset, nrlamps;
  char *vfd = vfd_get_string(0);

  nrlamps = Lamps_GetNumberLamps();

  offset  = 0;

  for ( y = 0; y < 16; y++ )
  {
	for ( x = 0; x < 16; x++ )
	{
	  plot_box(bitmap, x*8, y*8, 7, 7, Lamps_GetBrightness(offset) );
	  offset++;
	}
	if ( offset > nrlamps ) break;
  }

  y++;

  for ( x = 0; x < 16; x++ )
  {
	drawgfx(bitmap, font, vfd[x], 7, 0, 0, x*font->width, y*font->height,
		    0, TRANSPARENCY_NONE, 0);
  }

  gfx_printf(bitmap, NULL, 10, 240, "%3d %3d %3d %3d %3d %3d", //extendable if we emulate >6 devices
			  Stepper_get_position(0), Stepper_get_position(1), Stepper_get_position(2),
			  Stepper_get_position(3), Stepper_get_position(4), Stepper_get_position(5) );

/* 
  for ( x = 0; x < 6; x++ )
  {
	plot_box(bitmap, x*8, 240, 7, 7, steppers[x].pattern+1 );
	plot_box(bitmap, x*8, 248, 7, 7, (bfm_optic_pattern&(1<<x))?1:2   );
  }
*/

}

// palette initialisation /////////////////////////////////////////////////

PALETTE_INIT( generic_fruitmachine )
{
  palette_set_color( 0,0x00,0x00,0x00);
  palette_set_color( 1,0x00,0x00,0xFF);
  palette_set_color( 2,0x00,0xFF,0x00);
  palette_set_color( 3,0x00,0xFF,0xFF);
  palette_set_color( 4,0xFF,0x00,0x00);
  palette_set_color( 5,0xFF,0x00,0xFF);
  palette_set_color( 6,0xFF,0xFF,0x00);
  palette_set_color( 7,0xFF,0xFF,0xFF);
  palette_set_color( 8,0x80,0x80,0x80);
  palette_set_color( 9,0x00,0x00,0x80);
  palette_set_color(10,0x00,0x80,0x00);
  palette_set_color(11,0x00,0x80,0x80);
  palette_set_color(12,0x80,0x00,0x00);
  palette_set_color(13,0x80,0x00,0x80);
  palette_set_color(14,0x80,0x80,0x00);
  palette_set_color(15,0x80,0x80,0x80);
}

