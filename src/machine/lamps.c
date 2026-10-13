///////////////////////////////////////////////////////////////////////////
//																		 //
// lamps.c: module to keep track of lamp brightness						 //
//																		 //
//	05-03-2004: Re-Animator												 //
//																		 //
// TODO: add a lamppanel interface for getting on/off info quickly		 //
//		 without having to go trough a list of lamps 					 //
//																		 //
//		 think of a mechanism to check if lamps / lamp panels need		 //
//       to be refreshed												 //
//																		 //
// This file is part of MAGE, and is therefore released under the LGPL.  //
// Permission is given for its release under the MAME license, in the    //
// case of MAME and its derivatives.Any fixes for this driver should be  //
// forwarded to MAGENET (www.magesrc.sf.net/index.php					 //
///////////////////////////////////////////////////////////////////////////

#include "driver.h"
#include "lamps.h"

// local vars /////////////////////////////////////////////////////////////

static UINT8 lamps[MAX_LAMPS];
static int   number_lamps;


///////////////////////////////////////////////////////////////////////////

void Lamps_init(int number)
{
  if ( number > MAX_LAMPS ) number = MAX_LAMPS;

  number_lamps = number;
}

///////////////////////////////////////////////////////////////////////////

int Lamps_GetNumberLamps(void)
{
  return number_lamps;
}

///////////////////////////////////////////////////////////////////////////

void Lamps_SetBrightness( int start_id, int count, UINT8 *bright )
{
  int id = start_id;

  while ( id < number_lamps && count-- )
  {
	lamps[id++] = *bright++;
  }
}

///////////////////////////////////////////////////////////////////////////

int  Lamps_GetBrightness(int lamp)
{
  return ( lamp < number_lamps )? lamps[lamp] : 0;
}
