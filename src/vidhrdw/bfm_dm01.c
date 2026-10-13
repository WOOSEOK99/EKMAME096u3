/***************************************************************************

  Bellfruit dotmatrix driver, (under heavy construction !!!)

  09-08-2004: Re-Animator

Any fixes for this driver should be forwarded to MAGENET
(www.magesrc.sf.net)

Standard dm01 memorymap
           
   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-----------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k)
-----------+---+-----------------+-----------------------------------------
2000       |R/W| D D D D D D D D | control register
-----------+---+-----------------+-----------------------------------------
2800       |R/W| D D D D D D D D | mux
-----------+---+-----------------+-----------------------------------------
3000       |R/W| D D D D D D D D | communication with mainboard
-----------+---+-----------------+-----------------------------------------
3800       |R/W| D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
4000-FFFF  | W | D D D D D D D D | ROM (48k)
-----------+---+-----------------+-----------------------------------------

  TODO: - find out clockspeed of CPU


***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "machine/lamps.h"
#include "machine/steppers.h" // stepper motor
#include "dbgfonts/m0507fnt.c"
#include "genfmvid.h"

extern void Scorpion2_SetSwitchState(int strobe, int data, int state);
extern int  Scorpion2_GetSwitchState(int strobe, int data);


// local prototypes ///////////////////////////////////////////////////////



// local vars /////////////////////////////////////////////////////////////

#define DM_BYTESPERROW 9
#define DM_MAXLINES    21

static int   control;
static int   xcounter;
static int   busy;
static int   data_avail;

static UINT8 scanline[DM_BYTESPERROW];

static UINT8 dm_data[DM_BYTESPERROW*DM_MAXLINES];


static UINT8 comdata;

//static int   read_index;
//static int   write_index;

static struct mame_bitmap *dm_bitmap;

static struct GfxElement *font;	// pointer to font gfx



///////////////////////////////////////////////////////////////////////////

static int read_data(void)
{
  int data = comdata;

  logerror("rd:   %02X\n", data);

  data_avail = 0;

  busy = 0;

  return data;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( control_r )
{
  return 0;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( control_w )
{
  int changed = control ^ data;

  control = data;

  if ( changed & 2 )
  {	// reset horizontal counter
    if ( !(data & 2) )
    {
	  //int offset = 0;

	  xcounter = 0;
	}
  }

  if ( changed & 8 )
  { // bit 3 changed = BUSY line
	if ( data & 8 )
	{	  
	//  busy = 0;

	  //Switch_On(0x05);
	  Scorpion2_SetSwitchState(5,0, 1);
	}
	else
	{
	//  busy = 1;
	  //Switch_Off(0x05);
	  Scorpion2_SetSwitchState(5,0, 0);
	}
  }

  busy = 0;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux_r )
{
  return 0;
}


///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux_w )
{
  static UINT8 line[DM_BYTESPERROW*8];

  int x = xcounter;


  //logerror("xcnt = %d\n", x);

  if ( x < DM_BYTESPERROW )
  {
	scanline[xcounter++] = data;
  }

  if ( x == 8 )
  {
    int off = ((0xFF^data) & 0x7C) >> 2;	// 7C = 000001111100

	//logerror("  line complete %d\n", off);

	scanline[x] &= 0x80;
	if ( (off >= 0)  && (off < DM_MAXLINES) )
	{
	  int p;

	  memcpy( dm_data+(off*DM_BYTESPERROW), scanline, DM_BYTESPERROW);

	  x = 0;
	  p = 0;

	  while ( x < DM_BYTESPERROW )
	  {
		UINT8 d = scanline[x];

		line[p++] = d & 0x80 ? x : x+1;
		line[p++] = d & 0x40 ? x : x+1;
		line[p++] = d & 0x20 ? x : x+1;
		line[p++] = d & 0x10 ? x : x+1;
		line[p++] = d & 0x08 ? x : x+1;
		line[p++] = d & 0x04 ? x : x+1;
		line[p++] = d & 0x02 ? x : x+1;
		line[p++] = d & 0x01 ? x : x+1;
		x++;
	  }

	  draw_scanline8(dm_bitmap, 0, off, DM_BYTESPERROW*8, line, NULL, -1);
//void draw_scanline8(struct mame_bitmap *bitmap,int x,int y,int length,const UINT8 *src,pen_t *pens,int transparent_pen);

	  // update bitmap
	}
  }

}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( comm_r )
{
  int result = 0;

  if ( data_avail )
  {
	result = read_data();
	/*
	if ( data_avail() )
	{
	  cpu_set_irq_line(1, M6809_IRQ_LINE, HOLD_LINE );	// trigger IRQ
	}
	*/
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( comm_w )
{

}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( unknown_r )
{
  return 0;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( unknown_w )
{
}

///////////////////////////////////////////////////////////////////////////

ADDRESS_MAP_START( bfm_dm01_memmap, ADDRESS_SPACE_PROGRAM, 8 )

  AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_RAM)	// 8k RAM
  AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_RAM)	// 8k RAM
  AM_RANGE(0x2000, 0x2000) AM_READ(control_r)	// control reg
  AM_RANGE(0x2000, 0x2000) AM_WRITE(control_w)	// control reg
  AM_RANGE(0x2800, 0x2800) AM_READ(mux_r)		// mux
  AM_RANGE(0x2800, 0x2800) AM_WRITE(mux_w)		// mux
  AM_RANGE(0x3000, 0x3000) AM_READ(comm_r)		// 
  AM_RANGE(0x3000, 0x3000) AM_WRITE(comm_w)		// 
  AM_RANGE(0x3800, 0x3800) AM_READ(unknown_r)	// ???
  AM_RANGE(0x3800, 0x3800) AM_WRITE(unknown_w)	// ???
  AM_RANGE(0x4000, 0xFfff) AM_READ(MRA8_ROM)	// 48k  ROM

ADDRESS_MAP_END



///////////////////////////////////////////////////////////////////////////

struct mame_bitmap *BFM_dm01_getBitmap(void)
{
  return dm_bitmap;
}

///////////////////////////////////////////////////////////////////////////

void BFM_dm01_writedata(UINT8 data)
{
  comdata = data;
  data_avail = 1;

  busy = 1;

  //pulse IRQ line
  cpunum_set_input_line(1, M6809_IRQ_LINE, HOLD_LINE ); // trigger IRQ


  logerror("wd: %02X\n", data);
}


///////////////////////////////////////////////////////////////////////////

INTERRUPT_GEN( bfm_dm01_vbl )
{
  cpunum_set_input_line(1, INPUT_LINE_NMI, PULSE_LINE );
}


///////////////////////////////////////////////////////////////////////////



PALETTE_INIT( bfm_dm01 )
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


///////////////////////////////////////////////////////////////////////////

VIDEO_START( bfm_dm01 )
{
  font = decodegfx(fontdata,&fontlayout);

  font->colortable   = Machine->debug_remapped_colortable;
  font->total_colors = 16;

  dm_bitmap = bitmap_alloc_depth(DM_BYTESPERROW*8, DM_MAXLINES, 8);


  return 0;
}

//static struct rectangle clip = { 0, 400,  0,  280 };  //minx,maxx, miny,maxy





///////////////////////////////////////////////////////////////////////////

VIDEO_UPDATE( bfm_dm01 )
{
  int x, y,
	  gx, gy,
	  offset, nrlamps;

  char buffer[DM_BYTESPERROW*8+1];

  //copybitmap(bitmap, dm_bitmap, 0,8,0,0, &clip, 0, 0);

  //sprintf(buffer, "%02X %02X %02X %02X", combuf[0], combuf[1], combuf[2], combuf[3]);



  gy = 60;
  y = 0;

  while ( y < DM_MAXLINES )
  {
	char *p = buffer;
	int t;

	x = 0;
	t = 0;
	while ( x < DM_BYTESPERROW )
	{
	  UINT8 d = dm_data[(y*DM_BYTESPERROW)+x];

	  p[t++] =  d & 0x80? '#':' ';
	  p[t++] =  d & 0x40? '#':' ';
	  p[t++] =  d & 0x20? '#':' ';
	  p[t++] =  d & 0x10? '#':' ';
	  p[t++] =  d & 0x08? '#':' ';
	  p[t++] =  d & 0x04? '#':' ';
	  p[t++] =  d & 0x02? '#':' ';
	  p[t++] =  d & 0x01? '#':' ';
	  x++;
	}
	p[t] = 0;

	for ( gx = 0; buffer[gx]; gx++ )
	{
	  drawgfx(bitmap, font, buffer[gx], 15, 0, 0, gx*font->width, gy, 0, TRANSPARENCY_NONE, 0);
	}
	gy += font->height;
	y++;
  }


  nrlamps = Lamps_GetNumberLamps();

  offset  = 0;

  for ( y = 0; y < 16; y++ )
  {
	for ( x = 0; x < 16; x++ )
	{
	  plot_box(bitmap, 300+x*4, y*4, 3, 3, Lamps_GetBrightness(offset)+1 );
	  offset++;
	}
	if ( offset > nrlamps ) break;
  }

  gfx_printf(bitmap, font, 0, 0, "%3d %3d %3d %3d %3d %3d busy=%d (%d)", //extendable if we emulate >6 devices
			  Stepper_get_position(0), Stepper_get_position(1), Stepper_get_position(2),
			  Stepper_get_position(3), Stepper_get_position(4), Stepper_get_position(5), busy,
			  Scorpion2_GetSwitchState(5,0) );

}



int BFM_dm01_busy(void)
{
  return busy;
}




void BFM_dm01_reset(void)
{
  busy     = 0;
  control  = 0;
  xcounter = 0;
  data_avail = 0;

  Scorpion2_SetSwitchState(5,0, 1);
}
