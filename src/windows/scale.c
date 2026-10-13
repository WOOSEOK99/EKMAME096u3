//============================================================
//
//	scale.c - scaling effects framework code
//
//============================================================
 

// MAME headers
#include "driver.h"
#include "rc.h"

// scale2x
 #include "scale/scale2x.h"
#include "scale/scale3x.h"
#include "scale/hlq.h"

// defines
#define SCALE_EFFECT_NONE		 	 0
#define SCALE_EFFECT_SCALE2X		 1
#define SCALE_EFFECT_SCALE2X3		 2
#define SCALE_EFFECT_SCALE2X4		 3
#define SCALE_EFFECT_SCALE3X		 4
#define SCALE_EFFECT_SUPERSCALE		 5
#define SCALE_EFFECT_SUPERSCALE75	 6
#define SCALE_EFFECT_2XSAI		 	 7
#define SCALE_EFFECT_SUPER2XSAI		 8
#define SCALE_EFFECT_EAGLE		 	 9
#define SCALE_EFFECT_SUPEREAGLE		10
#define SCALE_EFFECT_LQ2X			11
#define SCALE_EFFECT_LQ2X3			12
#define SCALE_EFFECT_LQ2X4			13
#define SCALE_EFFECT_LQ3X			14
#define SCALE_EFFECT_HQ2X			15
#define SCALE_EFFECT_HQ2X3			16
#define SCALE_EFFECT_HQ2X4			17
#define SCALE_EFFECT_HQ3X			18
#ifdef SUPPORT_4X_EFFECT
#define SCALE_EFFECT_HQ4X			19
#define SCALE_EFFECT_LQ4X			20
#endif /* SUPPORT_4X_EFFECT */


//============================================================
//	GLOBAL VARIABLES
//============================================================

#ifdef SCALE_EFFECTS_FIX
struct { int effect; int xsize; int ysize; int blit; const char *name; } scale_effect;
#else
struct { int effect; int xsize; int ysize; const char *name; } scale_effect;
#endif /* SCALE_EFFECTS_FIX */



//============================================================
//	IMPORTS
//============================================================

extern UINT32 asmblit_cpuid_features(void);



//============================================================
//	LOCAL VARIABLES
//============================================================

static int use_mmx;

static UINT8 *scale_buffer;

static int previous_depth;
static int previous_pitch;
static int previous_height;

int scale_prefer_32;
#ifdef SCALE_EFFECTS_FIX
int scale_prefer_16;
#endif /* SCALE_EFFECTS_FIX */

static int   win_video_memory;
//============================================================
//	prototypes
//============================================================

// functions from scale2x
static int scale_perform_scale2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_scale2x_line_16)(UINT16 *dst0,UINT16 *dst1, const UINT16 *src0, const UINT16 *src1, const UINT16 *src2, unsigned count);
static void (*scale_scale2x_line_32)(UINT32 *dst0, UINT32 *dst1, const UINT32 *src0, const UINT32 *src1, const UINT32 *src2, unsigned count);

static int scale_perform_scale2x3(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_scale2x3_line_16)(UINT16 *dst0, UINT16 *dst1, UINT16 *dst2, const UINT16 *src0, const UINT16 *src1, const UINT16 *src2, unsigned count);
static void (*scale_scale2x3_line_32)(UINT32 *dst0, UINT32 *dst1, UINT32 *dst2, const UINT32 *src0, const UINT32 *src1, const UINT32 *src2, unsigned count);

static int scale_perform_scale2x4(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_scale2x4_line_16)(UINT16 *dst0, UINT16 *dst1, UINT16 *dst2, UINT16 *dst3, const UINT16 *src0, const UINT16 *src1, const UINT16 *src2, unsigned count);
static void (*scale_scale2x4_line_32)(UINT32 *dst0, UINT32 *dst1, UINT32 *dst2, UINT32 *dst3, const UINT32 *src0, const UINT32 *src1, const UINT32 *src2, unsigned count);

static int scale_perform_scale3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);

// functions from superscale.asm
void superscale_line(UINT16 *src0, UINT16 *src1, UINT16 *src2, UINT16 *dst, UINT32 width, UINT64 *mask);
void superscale_line_75(UINT16 *src0, UINT16 *src1, UINT16 *src2, UINT16 *dst, UINT32 width, UINT64 *mask);

static void (*superscale_line_func)(UINT16 *src0, UINT16 *src1, UINT16 *src2, UINT16 *dst, UINT32 width, UINT64 *mask);

static int scale_perform_superscale(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);

// functions from eagle
void _eagle_mmx16(unsigned long *lb, unsigned long *lb2, short width,	unsigned long *screen_address1, unsigned long *screen_address2);

static int scale_perform_eagle(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);

// functions from 2xsaimmx.asm
void  _2xSaISuperEagleLine(UINT8 *srcPtr, UINT8 *deltaPtr, UINT32 srcPitch, UINT32 width, UINT8 *dstPtr, UINT32 dstPitch, UINT16 dstBlah);
void  _2xSaILine(UINT8* srcPtr, UINT8* deltaPtr, UINT32 srcPitch, UINT32 width, UINT8* dstPtr, UINT32 dstPitch, UINT16 dstBlah);
void  _2xSaISuper2xSaILine(UINT8 *srcPtr, UINT8 *deltaPtr, UINT32 srcPitch, UINT32 width, UINT8 *dstPtr, UINT32 dstPitch, UINT16 dstBlah);
void  Init_2xSaIMMX(UINT32 BitFormat);

static void (*scale_2xsai_line)(UINT8 *srcPtr, UINT8 *deltaPtr, UINT32 srcPitch, UINT32 width, UINT8 *dstPtr, UINT32 dstPitch, UINT16 dstBlah);
static int scale_perform_2xsai(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void scale_2xsai_flush_buffer(int src_pitch, int height);

// functions from hq2x/hq3x/hq4x/lq2x/lq3x/lq4x
static void (*interp_set)(unsigned bits_per_pixel);
static int scale_perform_hlq2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq2x_16_def)(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq2x_32_def)(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

static int scale_perform_hlq2x3(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq2x3_16_def)(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq2x3_32_def)(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

static int scale_perform_hlq2x4(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq2x4_16_def)(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq2x4_32_def)(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

static int scale_perform_hlq3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq3x_16_def)(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq3x_32_def)(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
#ifdef SUPPORT_4X_EFFECT
static int scale_perform_hlq4x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq4x_16_def)(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq4x_32_def)(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
#endif /* SUPPORT_4X_EFFECT */

//============================================================
//	scale_parse
//============================================================

int scale_decode(struct rc_option *option, const char *arg, int priority)
{
	if (!strcmp(arg, "none"))
	{
		scale_effect.effect = SCALE_EFFECT_NONE;
	}
	// scale2x/scale3x: use Andrea Mazzoleni's scale2x/scale3x
	else if (!strcmp(arg, "scale2x"))
	{
		scale_effect.effect = SCALE_EFFECT_SCALE2X;
	}
	else if (!strcmp(arg, "scale2x3"))
	{
		scale_effect.effect = SCALE_EFFECT_SCALE2X3;
	}
	else if (!strcmp(arg, "scale2x4"))
	{
		scale_effect.effect = SCALE_EFFECT_SCALE2X4;
	}
	else if (!strcmp(arg, "scale3x"))
	{
		scale_effect.effect = SCALE_EFFECT_SCALE3X;
	}
	// eagle: use Dirk Stevens' Eagle FX
	else if (!strcmp(arg, "eagle"))
	{
		scale_effect.effect = SCALE_EFFECT_EAGLE;
#ifdef SCALE_EFFECTS_FIX
		scale_prefer_16 = 1;
#endif /* SCALE_EFFECTS_FIX */		
	}
	// superscale/superscale75: use Elsemi's SuperScale
	else if (!strcmp(arg, "superscale"))
	{
		scale_effect.effect = SCALE_EFFECT_SUPERSCALE;
#ifdef SCALE_EFFECTS_FIX
		scale_prefer_16 = 1;
#endif /* SCALE_EFFECTS_FIX */
	}
	else if (!strcmp(arg, "superscale75"))
	{
		scale_effect.effect = SCALE_EFFECT_SUPERSCALE75;
#ifdef SCALE_EFFECTS_FIX
		scale_prefer_16 = 1;
#endif /* SCALE_EFFECTS_FIX */
	}
	// 2xsai/super2xsai/supereagle: use Derek Liauw Kie Fa's 2xSaI package
	else if (!strcmp(arg, "2xsai"))
	{
		scale_effect.effect = SCALE_EFFECT_2XSAI;
#ifdef SCALE_EFFECTS_FIX
		scale_prefer_16 = 1;
#endif /* SCALE_EFFECTS_FIX */
	}
	else if (!strcmp(arg, "super2xsai"))
	{
		scale_effect.effect = SCALE_EFFECT_SUPER2XSAI;
#ifdef SCALE_EFFECTS_FIX
		scale_prefer_16 = 1;
#endif /* SCALE_EFFECTS_FIX */
	}
	else if (!strcmp(arg, "supereagle"))
	{
		scale_effect.effect = SCALE_EFFECT_SUPEREAGLE;
#ifdef SCALE_EFFECTS_FIX
		scale_prefer_16 = 1;
#endif /* SCALE_EFFECTS_FIX */
	}
	// hq2x/hq3x/hq4x/lq2x/lq3x/lq4x: made by Maxim Stepin
	else if (!strcmp(arg, "hq2x"))
	{
		scale_effect.effect = SCALE_EFFECT_HQ2X;
		scale_prefer_32 = 1;
	}
	else if (!strcmp(arg, "hq2x3"))
	{
		scale_effect.effect = SCALE_EFFECT_HQ2X3;
		scale_prefer_32 = 1;
	}
	else if (!strcmp(arg, "hq2x4"))
	{
		scale_effect.effect = SCALE_EFFECT_HQ2X4;
		scale_prefer_32 = 1;
	}
	else if (!strcmp(arg, "lq2x"))
	{
		scale_effect.effect = SCALE_EFFECT_LQ2X;
		scale_prefer_32 = 1;
	}
	else if (!strcmp(arg, "lq2x3"))
	{
		scale_effect.effect = SCALE_EFFECT_LQ2X3;
		scale_prefer_32 = 1;
	}
	else if (!strcmp(arg, "lq2x4"))
	{
		scale_effect.effect = SCALE_EFFECT_LQ2X4;
		scale_prefer_32 = 1;
	}
	else if (!strcmp(arg, "hq3x"))
	{
		scale_effect.effect = SCALE_EFFECT_HQ3X;
		scale_prefer_32 = 1;
	}
	else if (!strcmp(arg, "lq3x"))
	{
		scale_effect.effect = SCALE_EFFECT_LQ3X;
		scale_prefer_32 = 1;
	}
#ifdef SUPPORT_4X_EFFECT
	else if (!strcmp(arg, "hq4x"))
	{
		scale_effect.effect = SCALE_EFFECT_HQ4X;
		scale_prefer_32 = 1;
	}
	else if (!strcmp(arg, "lq4x"))
	{
		scale_effect.effect = SCALE_EFFECT_LQ4X;
		scale_prefer_32 = 1;
	}
#endif /* SUPPORT_4X_EFFECT */
	else
		return -1;

	option->priority = priority;
	return 0;
}

//============================================================
//	scale_exit
//============================================================

int scale_exit(void)
{
	free(scale_buffer);
	scale_buffer = NULL;
	scale_prefer_32 = 0;
#ifdef SCALE_EFFECTS_FIX
	scale_prefer_16 = 0;
#endif /* SCALE_EFFECTS_FIX */	
	return 0;
}

//============================================================
//	scale_init
//============================================================

int scale_init(void)
{
	static char name[64];

	UINT32 features = asmblit_cpuid_features();
	use_mmx = (features & (1 << 23));

	scale_effect.xsize = scale_effect.ysize = 1;
	sprintf(name, "none");
	scale_effect.name = name;

	previous_depth = previous_pitch = previous_height = 0;
	scale_buffer = NULL;

#ifdef SCALE_EFFECTS_FIX
	if (win_video_memory && scale_prefer_32)
		scale_effect.blit = scale_prefer_32;
	else
		scale_effect.blit = 0;
#endif /* SCALE_EFFECTS_FIX */

	switch (scale_effect.effect)
	{
		case SCALE_EFFECT_NONE:
		{
			break;
		}
		case SCALE_EFFECT_SCALE2X:
		{
			sprintf(name, "Scale2x (%s)", use_mmx ? "mmx optimised" : "non-mmx version");
			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}
		case SCALE_EFFECT_SCALE2X3:
		{
			sprintf(name, "Scale2x3 (%s)", use_mmx ? "mmx optimised" : "non-mmx version");
			scale_effect.xsize = 2;
			scale_effect.ysize = 3;
			break;
		}
		case SCALE_EFFECT_SCALE2X4:
		{
			sprintf(name, "Scale2x4 (%s)", use_mmx ? "mmx optimised" : "non-mmx version");
			scale_effect.xsize = 2;
			scale_effect.ysize = 4;
			break;
		}
		case SCALE_EFFECT_SCALE3X:
		{
			sprintf(name, "Scale3x (non-mmx version)");
			scale_effect.xsize = scale_effect.ysize = 3;
			break;
		}
		case SCALE_EFFECT_EAGLE:
		{
			sprintf(name, "Eagle (mmx optimised)");

			if (!use_mmx)
				return 1;

			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}

		case SCALE_EFFECT_SUPERSCALE:
		case SCALE_EFFECT_SUPERSCALE75:
		{
			if (scale_effect.effect == SCALE_EFFECT_SUPERSCALE)
			{
				sprintf(name, "SuperScale (mmx optimised)");
				superscale_line_func = superscale_line;
			}
			else
			{
				sprintf(name, "SuperScale75%% (mmx optimised)");
				superscale_line_func = superscale_line_75;
			}

			if (!use_mmx)
				return 1;

			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}

		case SCALE_EFFECT_2XSAI:
		case SCALE_EFFECT_SUPER2XSAI:
		case SCALE_EFFECT_SUPEREAGLE:
		{
			if (scale_effect.effect == SCALE_EFFECT_2XSAI)
			{
				sprintf(name, "2xSaI (mmx optimised)");
				scale_2xsai_line = _2xSaILine;
			}
			else if (scale_effect.effect == SCALE_EFFECT_SUPER2XSAI)
			{
				sprintf(name, "Super 2xSaI (mmx optimised)");
				scale_2xsai_line = _2xSaISuper2xSaILine;
			}
			else
			{
				sprintf(name, "Super Eagle (mmx optimised)");
				scale_2xsai_line = _2xSaISuperEagleLine;
			}

			if (!use_mmx)
				return 1;

			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}
		case SCALE_EFFECT_HQ2X:
		{
			sprintf(name, "HQ2x (mmx optimised)");
			scale_effect.xsize = scale_effect.ysize = 2;
			interp_set = hq2x_interp_set;
			scale_hlq2x_16_def = hq2x_16_def;
			scale_hlq2x_32_def = hq2x_32_def;
			if (!use_mmx) return 1;
			break;
		}
		case SCALE_EFFECT_HQ2X3:
		{
			sprintf(name, "HQ2x3 (mmx optimised)");
			scale_effect.xsize = 2;
			scale_effect.ysize = 3;
			interp_set = hq2x_interp_set;
			scale_hlq2x3_16_def = hq2x3_16_def;
			scale_hlq2x3_32_def = hq2x3_32_def;
			if (!use_mmx) return 1;
			break;
		}
		case SCALE_EFFECT_HQ2X4:
		{
			sprintf(name, "HQ2x4 (mmx optimised)");
			scale_effect.xsize = 2;
			scale_effect.ysize = 4;
			interp_set = hq2x_interp_set;
			scale_hlq2x4_16_def = hq2x4_16_def;
			scale_hlq2x4_32_def = hq2x4_32_def;
			if (!use_mmx) return 1;
			break;
		}
		case SCALE_EFFECT_LQ2X:
		{
			sprintf(name, "LQ2x (mmx optimised)");
			scale_effect.xsize = scale_effect.ysize = 2;
			interp_set = lq2x_interp_set;
			scale_hlq2x_16_def = lq2x_16_def;
			scale_hlq2x_32_def = lq2x_32_def;
			if (!use_mmx) return 1;
			break;
		}
		case SCALE_EFFECT_LQ2X3:
		{
			sprintf(name, "LQ2x3 (mmx optimised)");
			scale_effect.xsize = 2;
			scale_effect.ysize = 3;
			interp_set = lq2x_interp_set;
			scale_hlq2x3_16_def = lq2x3_16_def;
			scale_hlq2x3_32_def = lq2x3_32_def;
			if (!use_mmx) return 1;
			break;
		}
		case SCALE_EFFECT_LQ2X4:
		{
			sprintf(name, "LQ2x4 (mmx optimised)");
			scale_effect.xsize = 2;
			scale_effect.ysize = 4;
			interp_set = lq2x_interp_set;
			scale_hlq2x4_16_def = lq2x4_16_def;
			scale_hlq2x4_32_def = lq2x4_32_def;
			if (!use_mmx) return 1;
			break;
		}
		case SCALE_EFFECT_HQ3X:
		{
			sprintf(name, "HQ3x (mmx optimised)");
			scale_effect.xsize = scale_effect.ysize = 3;
			interp_set = hq3x_interp_set;
			scale_hlq3x_16_def = hq3x_16_def;
			scale_hlq3x_32_def = hq3x_32_def;
			if (!use_mmx) return 1;
			break;
		}
		case SCALE_EFFECT_LQ3X:
		{
			sprintf(name, "LQ3x (mmx optimised)");
			scale_effect.xsize = scale_effect.ysize = 3;
			interp_set = lq3x_interp_set;
			scale_hlq3x_16_def = lq3x_16_def;
			scale_hlq3x_32_def = lq3x_32_def;
			if (!use_mmx) return 1;
			break;
		}
#ifdef SUPPORT_4X_EFFECT
		case SCALE_EFFECT_HQ4X:
		{
			sprintf(name, "HQ4x (mmx optimised)");
			scale_effect.xsize = scale_effect.ysize = 4;
			interp_set = hq4x_interp_set;
			scale_hlq4x_16_def = hq4x_16_def;
			scale_hlq4x_32_def = hq4x_32_def;
			if (!use_mmx) return 1;
			break;
		}
		case SCALE_EFFECT_LQ4X:
		{
			sprintf(name, "LQ4x (mmx optimised)");
			scale_effect.xsize = scale_effect.ysize = 4;
			interp_set = lq4x_interp_set;
			scale_hlq4x_16_def = lq4x_16_def;
			scale_hlq4x_32_def = lq4x_32_def;
			if (!use_mmx) return 1;
			break;
		}
#endif /* SUPPORT_4X_EFFECT */
		default:
		{
			return 1;
		}
	}

	return 0;
}



//============================================================
//	scale_check
//============================================================

int scale_check(int depth)
{
	switch (scale_effect.effect)
	{
		case SCALE_EFFECT_NONE:
			return 0;

		case SCALE_EFFECT_SCALE2X:
		case SCALE_EFFECT_SCALE2X3:
		case SCALE_EFFECT_SCALE2X4:
		case SCALE_EFFECT_SCALE3X:
			if (depth == 16 || depth == 32)
				return 0;
			else
				return 1;

		case SCALE_EFFECT_SUPERSCALE:
		case SCALE_EFFECT_SUPERSCALE75:
			if (use_mmx && depth == 16)
				return 0;
			else 
				return 1;

		case SCALE_EFFECT_EAGLE:
			if (use_mmx && depth == 16)
				return 0;
			else
				return 1;

		case SCALE_EFFECT_2XSAI:
		case SCALE_EFFECT_SUPER2XSAI:
		case SCALE_EFFECT_SUPEREAGLE:
			if (use_mmx && (depth == 15 || depth == 16))
				return 0;
			else
				return 1;

		case SCALE_EFFECT_HQ2X:
		case SCALE_EFFECT_HQ2X3:
		case SCALE_EFFECT_HQ2X4:
		case SCALE_EFFECT_LQ2X:
		case SCALE_EFFECT_LQ2X3:
		case SCALE_EFFECT_LQ2X4:
		case SCALE_EFFECT_HQ3X:
		case SCALE_EFFECT_LQ3X:
#ifdef SUPPORT_4X_EFFECT
		case SCALE_EFFECT_HQ4X:
		case SCALE_EFFECT_LQ4X:
#endif /* SUPPORT_4X_EFFECT */
			if (use_mmx && (depth == 16 || depth == 32))
				return 0;
			else
				return 1;

		default:
			return 1;
	}
}



//============================================================
//	scale_perform_scale
//============================================================

int scale_perform_scale(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int update)
{
	switch (scale_effect.effect)
	{
		case SCALE_EFFECT_NONE:
			return 0;
		case SCALE_EFFECT_SCALE2X:
			return scale_perform_scale2x(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_SCALE2X3:
			return scale_perform_scale2x3(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_SCALE2X4:
			return scale_perform_scale2x4(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_SCALE3X:
			return scale_perform_scale3x(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_SUPERSCALE:
		case SCALE_EFFECT_SUPERSCALE75:
			return scale_perform_superscale(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_EAGLE:
			return scale_perform_eagle(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_2XSAI:
		case SCALE_EFFECT_SUPER2XSAI:
		case SCALE_EFFECT_SUPEREAGLE:
			if (update)
				scale_2xsai_flush_buffer(src_pitch, height);
			return scale_perform_2xsai(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_HQ2X:
		case SCALE_EFFECT_LQ2X:
			return scale_perform_hlq2x(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_HQ2X3:
		case SCALE_EFFECT_LQ2X3:
			return scale_perform_hlq2x3(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_HQ2X4:
		case SCALE_EFFECT_LQ2X4:
			return scale_perform_hlq2x4(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_HQ3X:
		case SCALE_EFFECT_LQ3X:
			return scale_perform_hlq3x(src, dst, src_pitch, dst_pitch, width, height, depth);
#ifdef SUPPORT_4X_EFFECT
		case SCALE_EFFECT_HQ4X:
		case SCALE_EFFECT_LQ4X:
			return scale_perform_hlq4x(src, dst, src_pitch, dst_pitch, width, height, depth);
#endif /* SUPPORT_4X_EFFECT */
		default:
			return 1;
	}
}


//============================================================
//	scale_emms
//============================================================

static inline void scale_emms(void)
{
	if (use_mmx)
	{
#ifdef __GNUC__
		__asm__ __volatile__ (
			"emms\n"
		);
#else
		__asm {
			emms;
		}
#endif
	}
}

//============================================================
//	scale_perform_scale2x
//============================================================

static int scale_perform_scale2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	if (depth != 16 && depth != 32)
		return 1;

	if (previous_depth != depth)
	{
		if (use_mmx)
		{
			scale_scale2x_line_16 = scale2x_16_mmx;
			scale_scale2x_line_32 = scale2x_32_mmx;
		}
		else
		{
			scale_scale2x_line_16 = scale2x_16_def;
			scale_scale2x_line_32 = scale2x_32_def;
		}

		previous_depth = depth;
	}

	if (depth == 16)
//		scale_scale2x_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		scale2x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
//		scale_scale2x_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		scale2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 16)
	{
		for (y = 2; y < height; y++)
		{
			dst += 2 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 2; y < height; y++)
		{
			dst += 2 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 2 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 16)
		scale_scale2x_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_scale2x_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}

//============================================================
//	scale_perform_scale2x3
//============================================================

static int scale_perform_scale2x3(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	if (depth != 16 && depth != 32)
		return 1;

	if (previous_depth != depth)
	{
		if (use_mmx)
		{
			scale_scale2x3_line_16 = scale2x3_16_mmx;
			scale_scale2x3_line_32 = scale2x3_32_mmx;
		}
		else
		{
			scale_scale2x3_line_16 = scale2x3_16_def;
			scale_scale2x3_line_32 = scale2x3_32_def;
		}

		previous_depth = depth;
	}

	if (depth == 16)
//		scale_scale2x3_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		scale2x3_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
//		scale_scale2x3_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		scale2x3_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 16)
	{
		for (y = 2; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x3_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 2; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x3_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 3 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 16)
		scale_scale2x3_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_scale2x3_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}

//============================================================
//	scale_perform_scale2x4
//============================================================

static int scale_perform_scale2x4(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	if (depth != 16 && depth != 32)
		return 1;

	if (previous_depth != depth)
	{
		if (use_mmx)
		{
			scale_scale2x4_line_16 = scale2x4_16_mmx;
			scale_scale2x4_line_32 = scale2x4_32_mmx;
		}
		else
		{
			scale_scale2x4_line_16 = scale2x4_16_def;
			scale_scale2x4_line_32 = scale2x4_32_def;
		}

		previous_depth = depth;
	}

	if (depth == 16)
//		scale_scale2x4_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		scale2x4_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
//		scale_scale2x4_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		scale2x4_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 16)
	{
		for (y = 2; y < height; y++)
		{
			dst += 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x4_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 2; y < height; y++)
		{
			dst += 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x4_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 4 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 16)
		scale_scale2x4_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_scale2x4_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}


//============================================================
//	scale_perform_scale3x
//============================================================

static int scale_perform_scale3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	if (depth == 16)
		scale3x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 16)
	{
		for (y = 3; y < height; y++)
		{
			dst	+= 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale3x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 3; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 3 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 16)
		scale3x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	return 0;
}



//============================================================
//	scale_perform_superscale
//============================================================

static int scale_perform_superscale(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	UINT32 srcNextline = src_pitch >> 1;
	UINT16 *dst0=(UINT16 *)dst;
	UINT16 *dst1=(UINT16 *)(dst+dst_pitch);
	UINT16 *src0=(UINT16 *)(src-src_pitch);  //don't worry, there is extra space :)
	UINT16 *src1=(UINT16 *)src;
	UINT16 *src2=(UINT16 *)(src+src_pitch);
	UINT64 mask = 0x7BEF7BEF7BEF7BEF;
	int i;

	for (i = 0; i < height; i++)
	{
		superscale_line(src0, src1, src2, dst0, width, &mask);
		superscale_line_func(src2, src1, src0, dst1, width, &mask);

		src0 = src1;
		src1 = src2;
		src2 += srcNextline;

		dst0 += dst_pitch;
		dst1 += dst_pitch;
	}
	scale_emms();

	return 0;
}



//============================================================
//	scale_perform_eagle
//============================================================

static int scale_perform_eagle(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;

	if (depth != 16 || !use_mmx)
		return 1;

	width *= 2;

	_eagle_mmx16((unsigned long *)src, (unsigned long *)src, width, (unsigned long *)dst, (unsigned long *)dst);
	dst += dst_pitch - 2;
	for (y = 0; y < height; y++, src += src_pitch, dst += 2 * dst_pitch)
		_eagle_mmx16((unsigned long *)src, (unsigned long *)(src + src_pitch), width, (unsigned long *)dst, (unsigned long *)(dst + dst_pitch));

	scale_emms();

	return 0;
}



//============================================================
//	scale_2xsai_flush_buffer
//============================================================

static void scale_2xsai_flush_buffer(int src_pitch, int height)
{
	free(scale_buffer);
#ifdef SCALE_EFFECTS_FIX
	scale_buffer = malloc(src_pitch * (height + 3));
	if (scale_buffer)
		memset(scale_buffer, 0xAA, src_pitch * (height + 3));
#else
	scale_buffer = malloc(src_pitch * (height + 2));
	if (scale_buffer)
		memset(scale_buffer, 0xAA, src_pitch * (height + 2));
#endif /* SCALE_EFFECTS_FIX */
}

//============================================================
//	scale_perform_2xsai
//============================================================

static int scale_perform_2xsai(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *buf;

	if ((depth != 15 && depth != 16) || !use_mmx)
		return 1;

	// see if we need to (re-)initialise
	if (previous_depth != depth || previous_pitch != src_pitch || previous_height != height)
	{
		Init_2xSaIMMX(depth == 15 ? 555 : 565);

		scale_2xsai_flush_buffer(src_pitch, height);

		previous_depth = depth;
		previous_pitch = src_pitch;
		previous_height = height;
	}

	if (scale_buffer == NULL)
		return 1;

	buf = scale_buffer;

	// treat 2xSaI differently from the others to better align the blitters with scanlines
	if (scale_effect.effect == SCALE_EFFECT_2XSAI) {
		for (y = 1; y < height; y++, src += src_pitch, buf += src_pitch, dst += 2 * dst_pitch)
			scale_2xsai_line(src, buf, src_pitch, width - 1, dst, dst_pitch, 0);
		scale_2xsai_line(src, buf, 0, width - 1, dst, dst_pitch, 0);
	}
	else
	{
		scale_2xsai_line(src, buf, src_pitch, width - 1, dst, dst_pitch, 0);
		dst += dst_pitch;
		for (y = 0; y < height; y++, src += src_pitch, buf += src_pitch, dst += 2 * dst_pitch)
			scale_2xsai_line(src, buf, src_pitch, width - 1, dst, dst_pitch, 0);
	}

	return 0;
}



//============================================================
//	scale_perform_hlq2x
//============================================================

static int scale_perform_hlq2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_set(depth);

	if (depth == 15 || depth == 16)
		scale_hlq2x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 15 || depth == 16)
	{
		for (y = 2; y < height; y++)
		{
			dst	+= 2 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq2x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 2; y < height; y++)
		{
			dst += 2 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 2 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 15 || depth == 16)
		scale_hlq2x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}

//============================================================
//	scale_perform_hlq2x3
//============================================================

static int scale_perform_hlq2x3(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_set(depth);

	if (depth == 15 || depth == 16)
		scale_hlq2x3_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq2x3_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 15 || depth == 16)
	{
		for (y = 2; y < height; y++)
		{
			dst	+= 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq2x3_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 2; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq2x3_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 3 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 15 || depth == 16)
		scale_hlq2x3_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq2x3_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}

//============================================================
//	scale_perform_hlq2x4
//============================================================

static int scale_perform_hlq2x4(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_set(depth);

	if (depth == 15 || depth == 16)
		scale_hlq2x4_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq2x4_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 15 || depth == 16)
	{
		for (y = 2; y < height; y++)
		{
			dst	+= 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq2x4_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 2; y < height; y++)
		{
			dst += 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq2x4_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 4 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 15 || depth == 16)
		scale_hlq2x4_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq2x4_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}

//============================================================
//	scale_perform_hlq3x
//============================================================

static int scale_perform_hlq3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_set(depth);

	if (depth == 15 || depth == 16)
		scale_hlq3x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 15 || depth == 16)
	{
		for (y = 3; y < height; y++)
		{
			dst	+= 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq3x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 3; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 3 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 15 || depth == 16)
		scale_hlq3x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}

#ifdef SUPPORT_4X_EFFECT
//============================================================
//	scale_perform_hlq4x
//============================================================

static int scale_perform_hlq4x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_set(depth);

	if (depth == 15 || depth == 16)
		scale_hlq4x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq4x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 15 || depth == 16)
	{
		for (y = 4; y < height; y++)
		{
			dst	+= 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq4x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 4; y < height; y++)
		{
			dst += 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_hlq4x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 4 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 15 || depth == 16)
		scale_hlq4x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq4x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}
#endif /* SUPPORT_4X_EFFECT */
