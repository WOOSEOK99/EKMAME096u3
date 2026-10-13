/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2001, 2002, 2003, 2004 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * This file contains a C and MMX implementation of the Scale2x effect.
 *
 * You can find an high level description of the effect at :
 *
 * http://scale2x.sourceforge.net/
 *
 * Alternatively at the previous license terms, you are allowed to use this
 * code in your program with these conditions:
 * - the program is not used in commercial activities.
 * - the whole source code of the program is released with the binary.
 * - derivative works of the program are allowed.
 */

#include "scale2x.h"

#include <assert.h>

/***************************************************************************/
/* Scale2x C implementation */

#if 0
INLINE void scale2x_8_def_border(scale2x_uint8* restrict dst, const scale2x_uint8* restrict src0, const scale2x_uint8* restrict src1, const scale2x_uint8* restrict src2, unsigned count)
{
	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[1] = src0[0];
	else
		dst[1] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 2;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src0[0] != src2[0] && src1[-1] != src1[1]) {
			dst[0] = src1[-1] == src0[0] ? src0[0] : src1[0];
			dst[1] = src1[1] == src0[0] ? src0[0] : src1[0];
		} else {
			dst[0] = src1[0];
			dst[1] = src1[0];
		}

		++src0;
		++src1;
		++src2;
		dst += 2;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
}
#endif

INLINE void scale2x_16_def_border(scale2x_uint16* restrict dst, const scale2x_uint16* restrict src0, const scale2x_uint16* restrict src1, const scale2x_uint16* restrict src2, unsigned count)
{
	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[1] = src0[0];
	else
		dst[1] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 2;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src0[0] != src2[0] && src1[-1] != src1[1]) {
			dst[0] = src1[-1] == src0[0] ? src0[0] : src1[0];
			dst[1] = src1[1] == src0[0] ? src0[0] : src1[0];
		} else {
			dst[0] = src1[0];
			dst[1] = src1[0];
		}

		++src0;
		++src1;
		++src2;
		dst += 2;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
}

INLINE void scale2x_32_def_border(scale2x_uint32* restrict dst, const scale2x_uint32* restrict src0, const scale2x_uint32* restrict src1, const scale2x_uint32* restrict src2, unsigned count)
{
	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[1] = src0[0];
	else
		dst[1] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 2;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src0[0] != src2[0] && src1[-1] != src1[1]) {
			dst[0] = src1[-1] == src0[0] ? src0[0] : src1[0];
			dst[1] = src1[1] == src0[0] ? src0[0] : src1[0];
		} else {
			dst[0] = src1[0];
			dst[1] = src1[0];
		}

		++src0;
		++src1;
		++src2;
		dst += 2;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
}

#if 0
INLINE void scale2x_8_def_center(scale2x_uint8* restrict dst, const scale2x_uint8* restrict src0, const scale2x_uint8* restrict src1, const scale2x_uint8* restrict src2, unsigned count)
{
	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	if (src0[0] != src2[0]) {
		dst[1] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
	} else {
		dst[1] = src1[0];
	}
	++src0;
	++src1;
	++src2;
	dst += 2;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src0[0] != src2[0] && src1[-1] != src1[1]) {
			dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
			dst[1] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
		} else {
			dst[0] = src1[0];
			dst[1] = src1[0];
		}

		++src0;
		++src1;
		++src2;
		dst += 2;
		--count;
	}

	/* last pixel */
	if (src0[0] != src2[0]) {
		dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
	} else {
		dst[0] = src1[0];
	}
	dst[1] = src1[0];
}
#endif

INLINE void scale2x_16_def_center(scale2x_uint16* restrict dst, const scale2x_uint16* restrict src0, const scale2x_uint16* restrict src1, const scale2x_uint16* restrict src2, unsigned count)
{
	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	if (src0[0] != src2[0]) {
		dst[1] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
	} else {
		dst[1] = src1[0];
	}
	++src0;
	++src1;
	++src2;
	dst += 2;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src0[0] != src2[0] && src1[-1] != src1[1]) {
			dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
			dst[1] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
		} else {
			dst[0] = src1[0];
			dst[1] = src1[0];
		}

		++src0;
		++src1;
		++src2;
		dst += 2;
		--count;
	}

	/* last pixel */
	if (src0[0] != src2[0]) {
		dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
	} else {
		dst[0] = src1[0];
	}
	dst[1] = src1[0];
}

INLINE void scale2x_32_def_center(scale2x_uint32* restrict dst, const scale2x_uint32* restrict src0, const scale2x_uint32* restrict src1, const scale2x_uint32* restrict src2, unsigned count)
{
	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	if (src0[0] != src2[0]) {
		dst[1] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
	} else {
		dst[1] = src1[0];
	}
	++src0;
	++src1;
	++src2;
	dst += 2;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src0[0] != src2[0] && src1[-1] != src1[1]) {
			dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
			dst[1] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
		} else {
			dst[0] = src1[0];
			dst[1] = src1[0];
		}

		++src0;
		++src1;
		++src2;
		dst += 2;
		--count;
	}

	/* last pixel */
	if (src0[0] != src2[0]) {
		dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
	} else {
		dst[0] = src1[0];
	}
	dst[1] = src1[0];
}

/**
 * Scale by a factor of 2 a row of pixels of 8 bits.
 * The function is implemented in C.
 * The pixels over the left and right borders are assumed of the same color of
 * the pixels on the border.
 * Note that the implementation is optimized to write data sequentially to
 * maximize the bandwidth on video memory.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
#if 0
void scale2x_8_def(scale2x_uint8* dst0, scale2x_uint8* dst1, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count)
{
	assert(count >= 2);

	scale2x_8_def_border(dst0, src0, src1, src2, count);
	scale2x_8_def_border(dst1, src2, src1, src0, count);
}
#endif

/**
 * Scale by a factor of 2 a row of pixels of 16 bits.
 * This function operates like scale2x_8_def() but for 16 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
void scale2x_16_def(scale2x_uint16* dst0, scale2x_uint16* dst1, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count)
{
	assert(count >= 2);

	scale2x_16_def_border(dst0, src0, src1, src2, count);
	scale2x_16_def_border(dst1, src2, src1, src0, count);
}

/**
 * Scale by a factor of 2 a row of pixels of 32 bits.
 * This function operates like scale2x_8_def() but for 32 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
void scale2x_32_def(scale2x_uint32* dst0, scale2x_uint32* dst1, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count)
{
	assert(count >= 2);

	scale2x_32_def_border(dst0, src0, src1, src2, count);
	scale2x_32_def_border(dst1, src2, src1, src0, count);
}

/**
 * Scale by a factor of 2x3 a row of pixels of 8 bits.
 * \note Like scale2x_8_def();
 */
#if 0
void scale2x3_8_def(scale2x_uint8* dst0, scale2x_uint8* dst1, scale2x_uint8* dst2, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count)
{
	assert(count >= 2);

	scale2x_8_def_border(dst0, src0, src1, src2, count);
	scale2x_8_def_center(dst1, src0, src1, src2, count);
	scale2x_8_def_border(dst2, src2, src1, src0, count);
}
#endif

/**
 * Scale by a factor of 2x3 a row of pixels of 16 bits.
 * \note Like scale2x_16_def();
 */
void scale2x3_16_def(scale2x_uint16* dst0, scale2x_uint16* dst1, scale2x_uint16* dst2, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count)
{
	assert(count >= 2);

	scale2x_16_def_border(dst0, src0, src1, src2, count);
	scale2x_16_def_center(dst1, src0, src1, src2, count);
	scale2x_16_def_border(dst2, src2, src1, src0, count);
}

/**
 * Scale by a factor of 2x3 a row of pixels of 32 bits.
 * \note Like scale2x_32_def();
 */
void scale2x3_32_def(scale2x_uint32* dst0, scale2x_uint32* dst1, scale2x_uint32* dst2, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count)
{
	assert(count >= 2);

	scale2x_32_def_border(dst0, src0, src1, src2, count);
	scale2x_32_def_center(dst1, src0, src1, src2, count);
	scale2x_32_def_border(dst2, src2, src1, src0, count);
}

/**
 * Scale by a factor of 2x4 a row of pixels of 8 bits.
 * \note Like scale2x_8_def();
 */
#if 0
void scale2x4_8_def(scale2x_uint8* dst0, scale2x_uint8* dst1, scale2x_uint8* dst2, scale2x_uint8* dst3, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count)
{
	assert(count >= 2);

	scale2x_8_def_border(dst0, src0, src1, src2, count);
	scale2x_8_def_center(dst1, src0, src1, src2, count);
	scale2x_8_def_center(dst2, src0, src1, src2, count);
	scale2x_8_def_border(dst3, src2, src1, src0, count);
}
#endif

/**
 * Scale by a factor of 2x4 a row of pixels of 16 bits.
 * \note Like scale2x_16_def();
 */
void scale2x4_16_def(scale2x_uint16* dst0, scale2x_uint16* dst1, scale2x_uint16* dst2, scale2x_uint16* dst3, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count)
{
	assert(count >= 2);

	scale2x_16_def_border(dst0, src0, src1, src2, count);
	scale2x_16_def_center(dst1, src0, src1, src2, count);
	scale2x_16_def_center(dst2, src0, src1, src2, count);
	scale2x_16_def_border(dst3, src2, src1, src0, count);
}

/**
 * Scale by a factor of 2x4 a row of pixels of 32 bits.
 * \note Like scale2x_32_def();
 */
void scale2x4_32_def(scale2x_uint32* dst0, scale2x_uint32* dst1, scale2x_uint32* dst2, scale2x_uint32* dst3, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count)
{
	assert(count >= 2);

	scale2x_32_def_border(dst0, src0, src1, src2, count);
	scale2x_32_def_center(dst1, src0, src1, src2, count);
	scale2x_32_def_center(dst2, src0, src1, src2, count);
	scale2x_32_def_border(dst3, src2, src1, src0, count);
}

/***************************************************************************/
/* Scale2x MMX implementation */

#ifdef __GNUC__
#include "scale2x_gcc.inc"
#else
#include "scale2x_vc.inc"
#endif

/**
 * Scale by a factor of 2 a row of pixels of 8 bits.
 * This is a very fast MMX implementation.
 * The implementation uses a combination of cmp/and/not operations to
 * completly remove the need of conditional jumps. This trick give the
 * major speed improvement.
 * Also, using the 8 bytes MMX registers more than one pixel are computed
 * at the same time.
 * Before calling this function you must ensure that the currenct CPU supports
 * the MMX instruction set. After calling it you must be sure to call the EMMS
 * instruction before any floating-point operation.
 * The pixels over the left and right borders are assumed of the same color of
 * the pixels on the border.
 * Note that the implementation is optimized to write data sequentially to
 * maximize the bandwidth on video memory.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows. It must
 * be at least 16 and a multiple of 8.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
#if 0
void scale2x_8_mmx(scale2x_uint8* dst0, scale2x_uint8* dst1, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count)
{
	if (count % 8 != 0 || count < 16) {
		scale2x_8_def(dst0, dst1, src0, src1, src2, count);
	} else {
		scale2x_8_mmx_border(dst0, src0, src1, src2, count);
		scale2x_8_mmx_border(dst1, src2, src1, src0, count);
	}
}
#endif

/**
 * Scale by a factor of 2 a row of pixels of 16 bits.
 * This function operates like scale2x_8_mmx() but for 16 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows. It must
 * be at least 8 and a multiple of 4.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
void scale2x_16_mmx(scale2x_uint16* dst0, scale2x_uint16* dst1, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count)
{
	if (count % 4 != 0 || count < 8) {
		scale2x_16_def(dst0, dst1, src0, src1, src2, count);
	} else {
		scale2x_16_mmx_border(dst0, src0, src1, src2, count);
		scale2x_16_mmx_border(dst1, src2, src1, src0, count);
	}
}

/**
 * Scale by a factor of 2 a row of pixels of 32 bits.
 * This function operates like scale2x_8_mmx() but for 32 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows. It must
 * be at least 4 and a multiple of 2.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
void scale2x_32_mmx(scale2x_uint32* dst0, scale2x_uint32* dst1, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count)
{
	if (count % 2 != 0 || count < 4) {
		scale2x_32_def(dst0, dst1, src0, src1, src2, count);
	} else {
		scale2x_32_mmx_border(dst0, src0, src1, src2, count);
		scale2x_32_mmx_border(dst1, src2, src1, src0, count);
	}
}

/**
 * Scale by a factor of 2x3 a row of pixels of 8 bits.
 * \note Like scale2x_8_mmx();
 */
#if 0
void scale2x3_8_mmx(scale2x_uint8* dst0, scale2x_uint8* dst1, scale2x_uint8* dst2, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count)
{
	if (count % 8 != 0 || count < 16) {
		scale2x3_8_def(dst0, dst1, dst2, src0, src1, src2, count);
	} else {
		scale2x_8_mmx_border(dst0, src0, src1, src2, count);
		scale2x_8_def_center(dst1, src0, src1, src2, count);
		scale2x_8_mmx_border(dst2, src2, src1, src0, count);
	}
}
#endif

/**
 * Scale by a factor of 2x3 a row of pixels of 16 bits.
 * \note Like scale2x_16_mmx();
 */
void scale2x3_16_mmx(scale2x_uint16* dst0, scale2x_uint16* dst1, scale2x_uint16* dst2, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count)
{
	if (count % 4 != 0 || count < 8) {
		scale2x3_16_def(dst0, dst1, dst2, src0, src1, src2, count);
	} else {
		scale2x_16_mmx_border(dst0, src0, src1, src2, count);
		scale2x_16_def_center(dst1, src0, src1, src2, count);
		scale2x_16_mmx_border(dst2, src2, src1, src0, count);
	}
}

/**
 * Scale by a factor of 2x3 a row of pixels of 32 bits.
 * \note Like scale2x_32_mmx();
 */
void scale2x3_32_mmx(scale2x_uint32* dst0, scale2x_uint32* dst1, scale2x_uint32* dst2, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count)
{
	if (count % 2 != 0 || count < 4) {
		scale2x3_32_def(dst0, dst1, dst2, src0, src1, src2, count);
	} else {
		scale2x_32_mmx_border(dst0, src0, src1, src2, count);
		scale2x_32_def_center(dst1, src0, src1, src2, count);
		scale2x_32_mmx_border(dst2, src2, src1, src0, count);
	}
}

/**
 * Scale by a factor of 2x4 a row of pixels of 8 bits.
 * \note Like scale2x_8_mmx();
 */
#if 0
void scale2x4_8_mmx(scale2x_uint8* dst0, scale2x_uint8* dst1, scale2x_uint8* dst2, scale2x_uint8* dst3, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count)
{
	if (count % 8 != 0 || count < 16) {
		scale2x4_8_def(dst0, dst1, dst2, dst3, src0, src1, src2, count);
	} else {
		scale2x_8_mmx_border(dst0, src0, src1, src2, count);
		scale2x_8_def_center(dst1, src0, src1, src2, count);
		scale2x_8_def_center(dst2, src0, src1, src2, count);
		scale2x_8_mmx_border(dst3, src2, src1, src0, count);
	}
}
#endif

/**
 * Scale by a factor of 2x4 a row of pixels of 16 bits.
 * \note Like scale2x_16_mmx();
 */
void scale2x4_16_mmx(scale2x_uint16* dst0, scale2x_uint16* dst1, scale2x_uint16* dst2, scale2x_uint16* dst3, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count)
{
	if (count % 4 != 0 || count < 8) {
		scale2x4_16_def(dst0, dst1, dst2, dst3, src0, src1, src2, count);
	} else {
		scale2x_16_mmx_border(dst0, src0, src1, src2, count);
		scale2x_16_def_center(dst1, src0, src1, src2, count);
		scale2x_16_def_center(dst2, src0, src1, src2, count);
		scale2x_16_mmx_border(dst3, src2, src1, src0, count);
	}
}

/**
 * Scale by a factor of 2x4 a row of pixels of 32 bits.
 * \note Like scale2x_32_mmx();
 */
void scale2x4_32_mmx(scale2x_uint32* dst0, scale2x_uint32* dst1, scale2x_uint32* dst2, scale2x_uint32* dst3, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count)
{
	if (count % 2 != 0 || count < 4) {
		scale2x4_32_def(dst0, dst1, dst2, dst3, src0, src1, src2, count);
	} else {
		scale2x_32_mmx_border(dst0, src0, src1, src2, count);
		scale2x_32_def_center(dst1, src0, src1, src2, count);
		scale2x_32_def_center(dst2, src0, src1, src2, count);
		scale2x_32_mmx_border(dst3, src2, src1, src0, count);
	}
}

