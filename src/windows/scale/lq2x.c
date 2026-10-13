#ifndef __LQ2X_H
#define __LQ2X_H

#include "interp.h"

/***************************************************************************/
/* LQ2x C implementation */

#define PIXEL_LQ_FILTER

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

void lq2x_interp_set(unsigned bits_per_pixel)
{
}

#endif
