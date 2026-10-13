void hq2x_interp_set(unsigned bits_per_pixel);
void hq2x_16_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x_32_def(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void hq2x3_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x3_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void hq2x4_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x4_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void lq2x_interp_set(unsigned bits_per_pixel);
void lq2x_16_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x_32_def(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void lq2x3_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x3_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void lq2x4_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x4_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void hq3x_interp_set(unsigned bits_per_pixel);
void hq3x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq3x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void lq3x_interp_set(unsigned bits_per_pixel);
void lq3x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq3x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void hq4x_interp_set(unsigned bits_per_pixel);
void hq4x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq4x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void lq4x_interp_set(unsigned bits_per_pixel);
void lq4x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq4x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
