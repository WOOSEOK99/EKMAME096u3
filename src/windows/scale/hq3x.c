#ifndef __HQ3X_H
#define __HQ3X_H

#include "interp.h"

/***************************************************************************/
/* HQ3x C implementation */

#define PIXEL_BITS 16
#include "hlq3x.inc"

#define PIXEL_BITS 32
#include "hlq3x.inc"

void hq3x_interp_set(unsigned bits_per_pixel)
{
	interp_init();
}

#endif
