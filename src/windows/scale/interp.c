#include "interp.h"

#ifdef INTERP_MMX

__M64_CONST CONST_ZERO = _MM_PI16(0, 0, 0, 0);
__M64_CONST CONST_3333 = _MM_PI16(3, 3, 3, 3);
__M64_CONST CONST_4444 = _MM_PI16(4, 4, 4, 4);
__M64_CONST CONST_5555 = _MM_PI16(5, 5, 5, 5);
__M64_CONST CONST_6666 = _MM_PI16(6, 6, 6, 6);
__M64_CONST CONST_7777 = _MM_PI16(7, 7, 7, 7);
__M64_CONST CONST_8888 = _MM_PI16(8, 8, 8, 8);
__M64_CONST CONST_9999 = _MM_PI16(9, 9, 9, 9);
__M64_CONST CONST_AAAA = _MM_PI16(10, 10, 10, 10);
__M64_CONST CONST_BBBB = _MM_PI16(11, 11, 11, 11);
__M64_CONST CONST_CCCC = _MM_PI16(12, 12, 12, 12);
__M64_CONST CONST_DDDD = _MM_PI16(13, 13, 13, 13);
__M64_CONST CONST_EEEE = _MM_PI16(14, 14, 14, 14);
__M64_CONST CONST_FFFF = _MM_PI16(15, 15, 15, 15);

__M64_CONST CONST_YUV_THRESHOLD = _MM_PI16(0, 0, 0x30, 0x0706);

UINT32 interp_RGB2YUV[65536];

void interp_init(void)
{
	static int interp_inited = 0;
	
	int i, j, k, r, g, b, y, u, v;

	if (interp_inited) return;
	interp_inited = 1;

	for (i=0; i<32; i++) {
		for (j=0; j<64; j++) {
			for (k=0; k<32; k++) {
				r = i << 3;
				g = j << 2;
				b = k << 3;
				y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2*g -b) >> 3);
				interp_RGB2YUV[ (i << 11) + (j << 5) + k ] = (y << 16) + (u << 8) + v;
			}
		}
	}
}

#else

void interp_init(void)
{
}

#endif
