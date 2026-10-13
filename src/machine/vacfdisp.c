///////////////////////////////////////////////////////////////////////////
//																		 //
// vacfdisp.c: Vacuum Fluorescence Display								 //
// display emulation for BFM machines, amongst others					 //
//																		 //
// emulated displays: BFM BD1 vfd display, OKI MSC1937					 //
//																		 //
// 05-03-2004: Re-Animator												 //
//																		 //
// TODO: - BFM sys85 seems to use a slightly different vfd to OKI MSC1937//
//																		 //
//		 - Implement display flashing and brightness					 //
//																		 //
//		 - Add dotmatrix VFD (5x7)										 //
// This file is part of MAGE, and is therefore released under the LGPL.  //
// Permission is given for its release under the MAME license, in the    //
// case of MAME and its derivatives.Any fixes for this driver should be  //
// forwarded to MAGENET (www.magesrc.sf.net\index.php)					 //
//																		 //
///////////////////////////////////////////////////////////////////////////

#include "driver.h"
#include "vacfdisp.h"


static struct
{
  UINT8	  type,				// type of alpha display
							// VFDTYPE_BFMBD1 or VFDTYPE_MSC1937

		  changed;			// flag <>0, if vfd contents is changed

  UINT8	  window_start,		// display window start pos 0-15
		  window_end,		// display window end   pos 0-15
		  window_size,		// window  size 
		  pad;				// unused align byte

  INT8	  pcursor_pos,		// previous cursor pos
		  cursor_pos;		// current cursor pos

  UINT16  brightness;		// display brightness level 0-31 (31=MAX)

  UINT16  user_def,			// user defined character state
		  user_data;		// user defined character data (16 bit)

  UINT8	  scroll_active,	// flag <>0, scrolling active
		  display_mode,		// display scroll   mode, 0/1/2/3
		  display_blanking,	// display blanking mode 0/1/2/3
		  flash_rate,		// flash rate 0-F
		  flash_control;	// flash control 0/1/2/3

  UINT8	  string[18];		// text buffer
  UINT16  segments[16];		// segments

  UINT8	  count,			// bit counter
		  data;				// receive register

} vfds[MAX_VFDS];


// local prototypes ///////////////////////////////////////////////////////

static void ScrollLeft( int id);

// local vars /////////////////////////////////////////////////////////////

// 
// Bellfruit BD1 charset to ASCII conversion table
//
static const char BFM2ASCII[] = 
//0123456789ABCDEF0123456789ABC DEF01 23456789ABCDEF0123456789ABCDEF
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ £\"#$%%'()*+.-./0123456789&%<=>?"\
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ £\"#$%%'()*+.-./0123456789&%<=>?"\
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ £\"#$%%'()*+.-./0123456789&%<=>?";

// 
// OKI MSC1937 charset to ASCII conversion table
//
static const char OKI1937ASCII[]=
//0123456789ABCDEF0123456789ABC DEF01 23456789ABCDEF0123456789ABCDEF
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ £\"#$%%'()*+.-./0123456789&%<=>?"\
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ £\"#$%%'()*+.-./0123456789&%<=>?"\
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ £\"#$%%'()*+.-./0123456789&%<=>?";

static const int poslut1937[]=
{
   1, // (0000)
   2, // (0001)
   3, // (0010)
   4, // (0011)
   5, // (0100)
   6, // (0101)
   7, // (0110)
   8, // (0111)
   9, // (1000)
  10, // (1001)
  11, // (1010)
  12, // (1011)
  13, // (1100)
  14, // (1101)
  15, // (1110)
   0  // (1111)
};

///////////////////////////////////////////////////////////////////////////

void vfd_init(int id, int type )
{
  memset( &vfds[id], 0, sizeof(vfds[0]));

  vfds[id].type = type;

  vfd_reset(id);
}

///////////////////////////////////////////////////////////////////////////

void vfd_reset(int id)
{
  if ( id > 2 || id < 0 ) return;

  vfds[id].window_end  = 15;
  vfds[id].window_size = (vfds[id].window_end - vfds[id].window_start)+1;
  memset(vfds[id].string, ' ', 16);

  vfds[id].brightness = 31;
  vfds[id].count      = 0;

  vfds[id].changed |= 1;
}

///////////////////////////////////////////////////////////////////////////

UINT16 *vfd_get_segments(int id)
{
  return vfds[id].segments;
}

///////////////////////////////////////////////////////////////////////////

char  *vfd_get_string( int id)
{
  return vfds[id].string;
}
///////////////////////////////////////////////////////////////////////////

void vfd_shift_data(int id, int data)
{
  if ( id > 2 || id < 0 ) return;

  vfds[id].data <<= 1;

  if ( !data ) vfds[id].data |= 1;

  if ( ++vfds[id].count >= 8 )
  {

	if ( vfd_newdata(id, vfds[id].data) )
	{
	  vfds[id].changed |= 1;
	}
	//logerror("vfd %3d -> %02X \"%s\"\n", id, vfds[id].data, vfds[id].string);

	vfds[id].count = 0;
	vfds[id].data  = 0;
  }
}

///////////////////////////////////////////////////////////////////////////

int vfd_newdata(int id, int data)
{
  int move   = 0, 
	  change = 0;
	  
  switch ( vfds[id].type )
  {
  case VFDTYPE_BFMBD1:

		if ( vfds[id].user_def )
		{
		  vfds[id].user_def--;

		  vfds[id].user_data <<= 8;
		  vfds[id].user_data |= data;

		  if ( vfds[id].user_def )
		  {
			return 0;
		  }
		  data = '@';
		}

		switch ( data & 0xF0 )
		{
		case 0x80:	// 0x80 - 0x8F Set display blanking

					vfds[id].display_blanking = data & 0x0F;					
					change = 1;
					break;

		case 0x90:	// 0x90 - 0x9F Set cursor pos
					vfds[id].cursor_pos = data & 0x0F;

					vfds[id].scroll_active = 0;
					if ( vfds[id].display_mode == 2 )
					{
					  if ( vfds[id].cursor_pos >= vfds[id].window_end) vfds[id].scroll_active = 1;
					}
					break;

		case 0xA0:	// 0xA0 - 0xAF Set display mode

					vfds[id].display_mode = data &0x03;
					break;

		case 0xB0:	// 0xB0 - 0xBF Clear display area
  
					switch ( data & 0x03 )
					{
					case 0x00:	// clr nothing
								break;
								
					case 0x01:	// clr inside window

								if ( vfds[id].window_size > 0 )
								{
								  memset( vfds[id].string+vfds[id].window_start, ' ',   vfds[id].window_size );
								}
								break;

					case 0x02:	// clr outside window

								if ( vfds[id].window_size > 0 )
								{
								  if ( vfds[id].window_start > 0 )
								  {
									memset( vfds[id].string, ' ', vfds[id].window_start);
								  }

								  if (vfds[id].window_end < 15 )
								  {
									memset( vfds[id].string+vfds[id].window_end, ' ', 15-vfds[id].window_end);
								  }							
								}
					case 0x03:	// clr entire display

								memset(vfds[id].string, ' ' , 16);
								break;
					}
					change = 1;
					break;

		case 0xC0:	// 0xC0 - 0xCF Set flash rate

					vfds[id].flash_rate = data & 0x0F;
					break;

		case 0xD0:	// 0xD0 - 0xDF Set Flash control

					vfds[id].flash_control = data & 0x03;
					break;

		case 0xE0:	// 0xE0 - 0xEF Set window start pos

					vfds[id].window_start = data &0x0F;
					vfds[id].window_size  = (vfds[id].window_end - vfds[id].window_start)+1;
					break;

		case 0xF0:	// 0xF0 - 0xFF Set window end pos

					vfds[id].window_end   = data &0x0F;
					vfds[id].window_size  = (vfds[id].window_end - vfds[id].window_start)+1;

					vfds[id].scroll_active = 0;
					if ( vfds[id].display_mode == 2 )
					{
					  if ( vfds[id].cursor_pos >= vfds[id].window_end)
					  {
						vfds[id].scroll_active = 1;
						vfds[id].cursor_pos    = vfds[id].window_end;
					  }
					}
					break;

		default:	// normal character

					switch ( data )
					{
					case 0x25:	// flash
								move++;
								break;

					case 0x26:	// undefined
								break;

					case 0x2C:
					case 0x2E:	// decimal point

								break;

					case 0x3B:	// dummy char
								move++;
								break;

					case 0x3A:	vfds[id].user_def = 2;
								break;

					default:	move = 1;
								change = 1;

					}

					if ( move )
					{
					  int mode = vfds[id].display_mode;

					  vfds[id].pcursor_pos = vfds[id].cursor_pos;

					  if ( vfds[id].window_size <= 0 || (vfds[id].window_size > 16))
					  { // no window selected default to rotate mode
						if ( mode == 2 )      mode = 0;
						else if ( mode == 3 ) mode = 1;
						//mode &= -2;
					  }


					  switch ( mode )
					  {
					  case 0: // rotate left

							  vfds[id].cursor_pos &= 0x0F;

							  if ( change )
							  {
								vfds[id].string[vfds[id].cursor_pos] = BFM2ASCII[data];
							  }
							  vfds[id].cursor_pos++;
							  if ( vfds[id].cursor_pos >= 16 ) vfds[id].cursor_pos = 0;
							  break;


					  case 1: // Rotate right

							  vfds[id].cursor_pos &= 0x0F;

							  if ( change )
							  {
								vfds[id].string[vfds[id].cursor_pos] = BFM2ASCII[data];
							  }
							  vfds[id].cursor_pos--;
							  if ( vfds[id].cursor_pos < 0  ) vfds[id].cursor_pos = 15;
							  break;

					  case 2: // Scroll left

							  if ( vfds[id].cursor_pos < vfds[id].window_end )
							  {
								vfds[id].scroll_active = 0;
								if ( change )
								{
								  vfds[id].string[vfds[id].cursor_pos]   = BFM2ASCII[data];
								}
								if ( move ) vfds[id].cursor_pos++;
							  }
							  else
							  {
								if ( move ) 
								{
								  if   ( vfds[id].scroll_active ) ScrollLeft(id);
								  else                            vfds[id].scroll_active = 1; 
								}

								if ( change )
								{
								  vfds[id].string[vfds[id].window_end]   = BFM2ASCII[data];
								}
								else
								{
								  vfds[id].string[vfds[id].window_end]   = ' ';
								}
							  }						
							  break;

					  case 3: // Scroll right

							  if ( vfds[id].cursor_pos > vfds[id].window_start ) 	
							  {
								if ( change )
								{
								  vfds[id].string[vfds[id].cursor_pos]   = BFM2ASCII[data];
								}
								vfds[id].cursor_pos--;
							  }
							  else
							  {
								int i = vfds[id].window_end;

								while ( i > vfds[id].window_start )
								{
								  vfds[id].string[ i ] = vfds[id].string[ i-1 ];
								  i--;
								}

								if ( change )
								{
								  vfds[id].string[vfds[id].window_start]   = BFM2ASCII[data];
								}
							  }
							  break;
					  }

					}
					break;


		}

		break;

  case VFDTYPE_MSC1937:

		if ( data & 0x80 )
		{ // Control data received
		  if ( (data & 0xF0) == 0xA0 ) // 1010 xxxx
		  { // 1010 xxxx Buffer Pointer control
			vfds[id].cursor_pos = poslut1937[data & 0x0F];
		  }
		  else if ( (data & 0xF0) == 0xC0 ) // 1100 xxxx
		  { // 1100 xxxx Set number of digits
			data &= 0x07;

			if ( data == 0 ) vfds[id].window_size = 16;
			else             vfds[id].window_size = data+8;
			vfds[id].window_start = 0;
			vfds[id].window_end   = vfds[id].window_size-1;
		  }
		  else if ( (data & 0xE0) == 0xE0 ) // 111x xxxx
		  { // 111x xxxx Set duty cycle ( brightness )
			vfds[id].brightness = (data & 0xF)*2;
			change = 1;
		  }
		  else if ( (data & 0xE0) == 0x80 ) // 100x ---
		  { // 100x xxxx Test mode
		  }
		}
		else
		{ // Display data
		  data &= 0x3F;
		  change = 1;

		  switch ( data )
		  {
		  case 0x2C: // ;
		  case 0x2E: //

		 			 //vfd->segments[vfd->pcursor_pos] |= (1<<12);
					 break;
		  default :
					 vfds[id].pcursor_pos = vfds[id].cursor_pos;
					 vfds[id].string[ vfds[id].cursor_pos ] = OKI1937ASCII[data];

					 vfds[id].cursor_pos++;
					 if (  vfds[id].cursor_pos > vfds[id].window_end )
					 {
					   vfds[id].cursor_pos = 0;
					 }
		  }
		}
		break;
  }

  return change;
}

///////////////////////////////////////////////////////////////////////////

static void ScrollLeft(int id)
{
  int i = vfds[id].window_start;

  while ( i < vfds[id].window_end )
  {
	vfds[id].string[ i ] = vfds[id].string[ i+1 ];
	i++;
  }
}



