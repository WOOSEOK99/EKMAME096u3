#ifndef __LQ4X_H
#define __LQ4X_H

#include "interp.h"

/***************************************************************************/
/* LQ4x C implementation */

#define PIXEL_LQ_FILTER

#define PIXEL_BITS 16
#include "hlq4x.inc"

#define PIXEL_BITS 32
#include "hlq4x.inc"

void lq4x_interp_set(unsigned bits_per_pixel)
{
}

#endif
