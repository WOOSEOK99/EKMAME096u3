#ifndef INC_BFMDM01
#define INC_BFMDM01

ADDRESS_MAP_EXTERN( bfm_dm01_memmap );

INTERRUPT_GEN( bfm_dm01_vbl );

VIDEO_START(  bfm_dm01 );
VIDEO_UPDATE( bfm_dm01 );
PALETTE_INIT( bfm_dm01 );

void BFM_dm01_reset(void);

void BFM_dm01_writedata(UINT8 data);

int BFM_dm01_busy(void);


struct mame_bitmap *BFM_dm01_getBitmap(void);


#endif
