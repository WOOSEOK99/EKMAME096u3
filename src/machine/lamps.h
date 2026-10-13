// This file is part of MAGE, and is therefore released under the LGPL.  //
// Permission is given for its release under the MAME license, in the    //
// case of MAME and its derivatives.Any fixes for this driver should be  //
// forwarded to MAGENET (www.magesrc.sf.net/index.php)					 //

#ifndef INC_LAMPS
#define INC_LAMPS

#define MAX_LAMPS 512


void Lamps_init(int number);

int  Lamps_GetNumberLamps(void);

void Lamps_SetBrightness(int start_id, int count, UINT8 *bright );

int  Lamps_GetBrightness(int id);

#endif

