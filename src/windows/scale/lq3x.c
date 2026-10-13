#ifndef __LQ3X_H
#define __LQ3X_H

#include "interp.h"

/***************************************************************************/
/* LQ3x C implementation */

#define PIXEL_LQ_FILTER

#define PIXEL_BITS 16
#include "hlq3x.inc"

#define PIXEL_BITS 32
#include "hlq3x.inc"

void lq3x_interp_set(unsigned bits_per_pixel)
{
}

#endif
