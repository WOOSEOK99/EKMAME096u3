#ifndef __HQ2X_H
#define __HQ2X_H

#include "interp.h"

/***************************************************************************/
/* HQ2x C implementation */

#define PIXEL_BITS 16
#include "hlq2x.inc"

#define PIXEL_BITS 16
#include "hlq2x3.inc"

#define PIXEL_BITS 16
#include "hlq2x4.inc"

#define PIXEL_BITS 32
#include "hlq2x.inc"

#define PIXEL_BITS 32
#include "hlq2x3.inc"

#define PIXEL_BITS 32
#include "hlq2x4.inc"

void hq2x_interp_set(unsigned bits_per_pixel)
{
	interp_init();
}

#endif
