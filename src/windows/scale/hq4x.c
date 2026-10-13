#ifndef __HQ4X_H
#define __HQ4X_H

#include "interp.h"

/***************************************************************************/
/* HQ4x C implementation */

#define PIXEL_BITS 16
#include "hlq4x.inc"

#define PIXEL_BITS 32
#include "hlq4x.inc"

void hq4x_interp_set(unsigned bits_per_pixel)
{
	interp_init();
}

#endif
