/***************************************************************************

                              -= Tetris Plus 2 =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  TMP68HC000P-12

Video Chips  :  SS91022-03 9428XX001
                GS91022-04 9721PD008
                SS91022-05 9347EX002
                GS91022-05 048 9726HX002

Sound Chips  :  Yamaha YMZ280B-F

Other        :  XILINX XC5210 PQ240C X68710M AKJ9544
                XC7336 PC44ACK9633 A63458A
                NVRAM


To Do:

-   There is a 3rd unimplemented layer capable of rotation (not used by
    the game, can be tested in service mode).
-   Priority RAM is not taken into account.

Notes:

-   The Japan set doesn't seem to have (or use) NVRAM. I can't enter
    a test mode or use the service coin either !?

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "sound/ymz280b.h"


UINT16 tetrisp2_systemregs[0x10];
UINT16 rocknms_sub_systemregs[0x10];

UINT16 rockn_protectdata;
UINT16 rockn_adpcmbank;
UINT16 rockn_soundvolume;

static void *rockn_timer_l4;
static void *rockn_timer_sub_l4;

/* Variables defined in vidhrdw: */

extern data16_t *tetrisp2_vram_bg, *tetrisp2_scroll_bg;
extern data16_t *tetrisp2_vram_fg, *tetrisp2_scroll_fg;
extern data16_t *tetrisp2_vram_rot, *tetrisp2_rotregs;

extern data16_t *tetrisp2_priority;

extern data16_t *rocknms_sub_vram_bg, *rocknms_sub_scroll_bg;
extern data16_t *rocknms_sub_vram_fg, *rocknms_sub_scroll_fg;
extern data16_t *rocknms_sub_vram_rot, *rocknms_sub_rotregs;

extern data16_t *rocknms_sub_priority;

/* Functions defined in vidhrdw: */

WRITE16_HANDLER( tetrisp2_palette_w );
WRITE16_HANDLER( rocknms_sub_palette_w );
READ16_HANDLER( tetrisp2_priority_r );
WRITE16_HANDLER( tetrisp2_priority_w );
WRITE16_HANDLER( rockn_priority_w );
READ16_HANDLER( rocknms_sub_priority_r );
WRITE16_HANDLER( rocknms_sub_priority_w );

WRITE16_HANDLER( tetrisp2_vram_bg_w );
WRITE16_HANDLER( tetrisp2_vram_fg_w );
WRITE16_HANDLER( tetrisp2_vram_rot_w );

WRITE16_HANDLER( rocknms_sub_vram_bg_w );
WRITE16_HANDLER( rocknms_sub_vram_fg_w );
WRITE16_HANDLER( rocknms_sub_vram_rot_w );

VIDEO_START( tetrisp2 );
VIDEO_UPDATE( tetrisp2 );
VIDEO_START( rockntread );
VIDEO_UPDATE( rockntread );
VIDEO_START( rocknms );
VIDEO_UPDATE( rocknms );


/***************************************************************************


                                    Sound


***************************************************************************/

static WRITE16_HANDLER( tetrisp2_systemregs_w )
{
	if (ACCESSING_LSB)
	{
		tetrisp2_systemregs[offset] = data;
	}
}

#define ROCKN_TIMER_BASE 500000

static WRITE16_HANDLER( rockn_systemregs_w )
{
	if (ACCESSING_LSB)
	{
		tetrisp2_systemregs[offset] = data;
		if (offset == 0x0c)
		{
			double timer = TIME_IN_NSEC(ROCKN_TIMER_BASE) * (4096 - data);
			timer_adjust(rockn_timer_l4, timer, 0, timer);
		}
	}
}

static WRITE16_HANDLER( rocknms_sub_systemregs_w )
{
	if (ACCESSING_LSB)
	{
		rocknms_sub_systemregs[offset] = data;
		if (offset == 0x0c)
		{
			double timer = TIME_IN_NSEC(ROCKN_TIMER_BASE) * (4096 - data);
			timer_adjust(rockn_timer_sub_l4, timer, 0, timer);
		}
	}
}

/***************************************************************************


                                    Sound


***************************************************************************/

static READ16_HANDLER( tetrisp2_sound_r )
{
	return YMZ280B_status_0_r(offset);
}

static WRITE16_HANDLER( tetrisp2_sound_w )
{
	if (ACCESSING_LSB)
	{
		if (offset)	YMZ280B_data_0_w     (offset, data & 0xff);
		else		YMZ280B_register_0_w (offset, data & 0xff);
	}
}

static READ16_HANDLER( rockn_adpcmbank_r )
{
	return ((rockn_adpcmbank & 0xf0ff) | (rockn_protectdata << 8));
}

static WRITE16_HANDLER( rockn_adpcmbank_w )
{
	UINT8 *SNDROM = memory_region(REGION_SOUND1);
	int bank;

	rockn_adpcmbank = data;
	bank = ((data & 0x001f) >> 2);

	if (bank > 7)
	{
		usrintf_showmessage("!!!!! ADPCM BANK OVER:%01X (%04X) !!!!!", bank, data);
		bank = 0;
	}

	memcpy(&SNDROM[0x0400000], &SNDROM[0x1000000 + (0x0c00000 * bank)], 0x0c00000);
}


static WRITE16_HANDLER( rockn2_adpcmbank_w )
{
	UINT8 *SNDROM = memory_region(REGION_SOUND1);
	int bank;

	char banktable[9][3]=
	{
		{  0,  1,  2 },		// bank $00
		{  3,  4,  5 },		// bank $04
		{  6,  7,  8 },		// bank $08
		{  9, 10, 11 },		// bank $0c
		{ 12, 13, 14 },		// bank $10
		{ 15, 16, 17 },		// bank $14
		{ 18, 19, 20 },		// bank $18
		{  0,  0,  0 },		// bank $1c
		{  0,  5, 14 },		// bank $20
	};

	rockn_adpcmbank = data;
	bank = ((data & 0x003f) >> 2);

	if (bank > 8)
	{
		usrintf_showmessage("!!!!! ADPCM BANK OVER:%01X (%04X) !!!!!", bank, data);
		bank = 0;
	}

	memcpy(&SNDROM[0x0400000], &SNDROM[0x1000000 + (0x0400000 * banktable[bank][0] )], 0x0400000);
	memcpy(&SNDROM[0x0800000], &SNDROM[0x1000000 + (0x0400000 * banktable[bank][1] )], 0x0400000);
	memcpy(&SNDROM[0x0c00000], &SNDROM[0x1000000 + (0x0400000 * banktable[bank][2] )], 0x0400000);
}

static READ16_HANDLER( rockn_soundvolume_r )
{
	return 0xffff;
}

static WRITE16_HANDLER( rockn_soundvolume_w )
{
	rockn_soundvolume = data;
}

/***************************************************************************


                                Protection


***************************************************************************/

static READ16_HANDLER( tetrisp2_ip_1_word_r )
{
	return	( readinputportbytag("IN1") &  0xfcff ) |
			(           rand() & ~0xfcff ) |
			(      1 << (8 + (rand()&1)) );
}


/***************************************************************************


                                    NVRAM


***************************************************************************/

static data16_t *tetrisp2_nvram;
static size_t tetrisp2_nvram_size;

NVRAM_HANDLER( tetrisp2 )
{
	if (read_or_write)
		mame_fwrite(file,tetrisp2_nvram,tetrisp2_nvram_size);
	else
	{
		if (file)
			mame_fread(file,tetrisp2_nvram,tetrisp2_nvram_size);
		else
		{
			/* fill in the default values */
			memset(tetrisp2_nvram,0,tetrisp2_nvram_size);
		}
	}
}


/* The game only ever writes even bytes and reads odd bytes */
READ16_HANDLER( tetrisp2_nvram_r )
{
	return	( (tetrisp2_nvram[offset] >> 8) & 0x00ff ) |
			( (tetrisp2_nvram[offset] << 8) & 0xff00 ) ;
}

READ16_HANDLER( rockn_nvram_r )
{
	return	tetrisp2_nvram[offset];
}
WRITE16_HANDLER( tetrisp2_nvram_w )
{
	COMBINE_DATA(&tetrisp2_nvram[offset]);
}



/***************************************************************************





***************************************************************************/
static UINT16 rocknms_main2sub;
static UINT16 rocknms_sub2main;

READ16_HANDLER( rocknms_main2sub_r )
{
	return rocknms_main2sub;
}

WRITE16_HANDLER( rocknms_main2sub_w )
{
	if (ACCESSING_LSB)
		rocknms_main2sub = (data ^ 0xffff);
}

READ16_HANDLER( rocknms_port_0_r )
{
	return ((readinputport(0) & 0xfffc ) | (rocknms_sub2main & 0x0003));
}

WRITE16_HANDLER( rocknms_sub2main_w )
{
	if (ACCESSING_LSB)
		rocknms_sub2main = (data ^ 0xffff);
}


WRITE16_HANDLER( tetrisp2_coincounter_w )
{
	coin_counter_w( 0, (data & 0x0001));
}


/***************************************************************************


                                Memory Map


***************************************************************************/

static ADDRESS_MAP_START( tetrisp2_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA16_ROM				)	// ROM
	AM_RANGE(0x100000, 0x103fff) AM_READ(MRA16_RAM				)	// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_READ(MRA16_RAM				)	// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_READ(MRA16_RAM				)	// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_READ(tetrisp2_priority_r	)	// Priority
	AM_RANGE(0x300000, 0x31ffff) AM_READ(MRA16_RAM				)	// Palette
	AM_RANGE(0x400000, 0x403fff) AM_READ(MRA16_RAM				)	// Foreground
	AM_RANGE(0x404000, 0x407fff) AM_READ(MRA16_RAM				)	// Background
	AM_RANGE(0x408000, 0x409fff) AM_READ(MRA16_RAM				)	// ???
	AM_RANGE(0x500000, 0x50ffff) AM_READ(MRA16_RAM				)	// Line
	AM_RANGE(0x600000, 0x60ffff) AM_READ(MRA16_RAM				)	// Rotation
	AM_RANGE(0x650000, 0x651fff) AM_READ(MRA16_RAM				)	// Rotation (mirror)
	AM_RANGE(0x800002, 0x800003) AM_READ(tetrisp2_sound_r		)	// Sound
	AM_RANGE(0x900000, 0x903fff) AM_READ(tetrisp2_nvram_r	)	// NVRAM
	AM_RANGE(0x904000, 0x907fff) AM_READ(tetrisp2_nvram_r	)	// NVRAM (mirror)
	AM_RANGE(0xbe0000, 0xbe0001) AM_READ(MRA16_NOP				)	// INT-level1 dummy read
	AM_RANGE(0xbe0002, 0xbe0003) AM_READ(input_port_0_word_r	)	// Inputs
	AM_RANGE(0xbe0004, 0xbe0005) AM_READ(tetrisp2_ip_1_word_r	)	// Inputs & protection
	AM_RANGE(0xbe0008, 0xbe0009) AM_READ(input_port_2_word_r	)	// Inputs
	AM_RANGE(0xbe000a, 0xbe000b) AM_READ(watchdog_reset16_r	)	// Watchdog
ADDRESS_MAP_END

static ADDRESS_MAP_START( tetrisp2_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA16_ROM									)	// ROM
	AM_RANGE(0x100000, 0x103fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size	)	// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_WRITE(MWA16_RAM									)	// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_WRITE(MWA16_RAM									)	// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_WRITE(tetrisp2_priority_w) AM_BASE(&tetrisp2_priority	)	// Priority
	AM_RANGE(0x300000, 0x31ffff) AM_WRITE(tetrisp2_palette_w) AM_BASE(&paletteram16			)	// Palette
	AM_RANGE(0x400000, 0x403fff) AM_WRITE(tetrisp2_vram_fg_w) AM_BASE(&tetrisp2_vram_fg		)	// Foreground
	AM_RANGE(0x404000, 0x407fff) AM_WRITE(tetrisp2_vram_bg_w) AM_BASE(&tetrisp2_vram_bg		)	// Background
	AM_RANGE(0x408000, 0x409fff) AM_WRITE(MWA16_RAM									)	// ???
	AM_RANGE(0x500000, 0x50ffff) AM_WRITE(MWA16_RAM									)	// Line
	AM_RANGE(0x600000, 0x60ffff) AM_WRITE(tetrisp2_vram_rot_w) AM_BASE(&tetrisp2_vram_rot	)	// Rotation
	AM_RANGE(0x650000, 0x651fff) AM_WRITE(tetrisp2_vram_rot_w						)	// Rotation (mirror)
	AM_RANGE(0x800000, 0x800003) AM_WRITE(tetrisp2_sound_w							)	// Sound
	AM_RANGE(0x900000, 0x903fff) AM_WRITE(tetrisp2_nvram_w) AM_BASE(&tetrisp2_nvram) AM_SIZE(&tetrisp2_nvram_size	)	// NVRAM
	AM_RANGE(0x904000, 0x907fff) AM_WRITE(tetrisp2_nvram_w							)	// NVRAM (mirror)
	AM_RANGE(0xb00000, 0xb00001) AM_WRITE(tetrisp2_coincounter_w					)	// Coin Counter
	AM_RANGE(0xb20000, 0xb20001) AM_WRITE(MWA16_NOP									)	// ???
	AM_RANGE(0xb40000, 0xb4000b) AM_WRITE(MWA16_RAM) AM_BASE(&tetrisp2_scroll_fg			)	// Foreground Scrolling
	AM_RANGE(0xb40010, 0xb4001b) AM_WRITE(MWA16_RAM) AM_BASE(&tetrisp2_scroll_bg			)	// Background Scrolling
	AM_RANGE(0xb4003e, 0xb4003f) AM_WRITE(MWA16_NOP									)	// scr_size
	AM_RANGE(0xb60000, 0xb6002f) AM_WRITE(MWA16_RAM) AM_BASE(&tetrisp2_rotregs				)	// Rotation Registers
	AM_RANGE(0xba0000, 0xba001f) AM_WRITE(tetrisp2_systemregs_w						)	// system param
	AM_RANGE(0xba001a, 0xba001b) AM_WRITE(MWA16_NOP									)	// Lev 4 irq ack
	AM_RANGE(0xba001e, 0xba001f) AM_WRITE(MWA16_NOP									)	// Lev 2 irq ack
ADDRESS_MAP_END

static ADDRESS_MAP_START( rockn1_readmem , ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA16_ROM				)	// ROM
	AM_RANGE(0x100000, 0x103fff) AM_READ(MRA16_RAM				)	// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_READ(MRA16_RAM				)	// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_READ(MRA16_RAM				)	// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_READ(tetrisp2_priority_r	)	// Priority
	AM_RANGE(0x300000, 0x31ffff) AM_READ(MRA16_RAM				)	// Palette
	AM_RANGE(0x400000, 0x403fff) AM_READ(MRA16_RAM				)	// Foreground
	AM_RANGE(0x404000, 0x407fff) AM_READ(MRA16_RAM				)	// Background
	AM_RANGE(0x408000, 0x409fff) AM_READ(MRA16_RAM				)	// ???
	AM_RANGE(0x500000, 0x50ffff) AM_READ(MRA16_RAM				)	// Line
	AM_RANGE(0x600000, 0x60ffff) AM_READ(MRA16_RAM				)	// Rotation
	AM_RANGE(0x900000, 0x903fff) AM_READ(rockn_nvram_r		)	// NVRAM
	AM_RANGE(0xa30000, 0xa30001) AM_READ(rockn_soundvolume_r	)	// Sound Volume
	AM_RANGE(0xa40002, 0xa40003) AM_READ(tetrisp2_sound_r		)	// Sound
	AM_RANGE(0xa44000, 0xa44001) AM_READ(rockn_adpcmbank_r		)	// Sound Bank
	AM_RANGE(0xbe0000, 0xbe0001) AM_READ(MRA16_NOP				)	// INT-level1 dummy read
	AM_RANGE(0xbe0002, 0xbe0003) AM_READ(input_port_0_word_r	)	// Inputs
	AM_RANGE(0xbe0004, 0xbe0005) AM_READ(input_port_1_word_r	)	// Inputs
	AM_RANGE(0xbe0008, 0xbe0009) AM_READ(input_port_2_word_r	)	// Inputs
	AM_RANGE(0xbe000a, 0xbe000b) AM_READ(watchdog_reset16_r	)	// Watchdog
ADDRESS_MAP_END

static ADDRESS_MAP_START( rockn1_writemem , ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA16_ROM									)	// ROM
	AM_RANGE(0x100000, 0x103fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size	)	// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_WRITE(MWA16_RAM									)	// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_WRITE(MWA16_RAM									)	// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_WRITE(rockn_priority_w) AM_BASE(&tetrisp2_priority	)	// Priority
	AM_RANGE(0x300000, 0x31ffff) AM_WRITE(tetrisp2_palette_w) AM_BASE(&paletteram16			)	// Palette
	AM_RANGE(0x400000, 0x403fff) AM_WRITE(tetrisp2_vram_fg_w) AM_BASE(&tetrisp2_vram_fg		)	// Foreground
	AM_RANGE(0x404000, 0x407fff) AM_WRITE(tetrisp2_vram_bg_w) AM_BASE(&tetrisp2_vram_bg		)	// Background
	AM_RANGE(0x408000, 0x409fff) AM_WRITE(MWA16_RAM									)	// ???
	AM_RANGE(0x500000, 0x50ffff) AM_WRITE(MWA16_RAM									)	// Line
	AM_RANGE(0x600000, 0x60ffff) AM_WRITE(tetrisp2_vram_rot_w) AM_BASE(&tetrisp2_vram_rot	)	// Rotation
	AM_RANGE(0x900000, 0x903fff) AM_WRITE(tetrisp2_nvram_w) AM_BASE(&tetrisp2_nvram) AM_SIZE(&tetrisp2_nvram_size	)	// NVRAM
	AM_RANGE(0xa30000, 0xa30001) AM_WRITE(rockn_soundvolume_w						)	// Sound Volume
	AM_RANGE(0xa40000, 0xa40003) AM_WRITE(tetrisp2_sound_w							)	// Sound
	AM_RANGE(0xa44000, 0xa44001) AM_WRITE(rockn_adpcmbank_w							)	// Sound Bank
	AM_RANGE(0xa48000, 0xa48001) AM_WRITE(MWA16_NOP									)	// YMZ280 Reset
	AM_RANGE(0xb00000, 0xb00001) AM_WRITE(tetrisp2_coincounter_w					)	// Coin Counter
	AM_RANGE(0xb20000, 0xb20001) AM_WRITE(MWA16_NOP									)	// ???
	AM_RANGE(0xb40000, 0xb4000b) AM_WRITE(MWA16_RAM) AM_BASE(&tetrisp2_scroll_fg			)	// Foreground Scrolling
	AM_RANGE(0xb40010, 0xb4001b) AM_WRITE(MWA16_RAM) AM_BASE(&tetrisp2_scroll_bg			)	// Background Scrolling
	AM_RANGE(0xb4003e, 0xb4003f) AM_WRITE(MWA16_NOP									)	// scr_size
	AM_RANGE(0xb60000, 0xb6002f) AM_WRITE(MWA16_RAM) AM_BASE(&tetrisp2_rotregs				)	// Rotation Registers
	AM_RANGE(0xba0000, 0xba001f) AM_WRITE(rockn_systemregs_w						)	// system param
	AM_RANGE(0xba001a, 0xba001b) AM_WRITE(MWA16_NOP									)	// Lev 4 irq ack
	AM_RANGE(0xba001e, 0xba001f) AM_WRITE(MWA16_NOP									)	// Lev 2 irq ack
ADDRESS_MAP_END


static ADDRESS_MAP_START( rockn2_readmem , ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x0fffff) AM_READ( MRA16_ROM				)	// ROM
	AM_RANGE( 0x100000, 0x103fff) AM_READ( MRA16_RAM				)	// Object RAM
	AM_RANGE( 0x104000, 0x107fff) AM_READ( MRA16_RAM				)	// Spare Object RAM
	AM_RANGE( 0x108000, 0x10ffff) AM_READ( MRA16_RAM				)	// Work RAM
	AM_RANGE( 0x200000, 0x23ffff) AM_READ( tetrisp2_priority_r	)	// Priority
	AM_RANGE( 0x300000, 0x31ffff) AM_READ( MRA16_RAM				)	// Palette
	AM_RANGE( 0x500000, 0x50ffff) AM_READ( MRA16_RAM				)	// Line
	AM_RANGE( 0x600000, 0x60ffff) AM_READ( MRA16_RAM				)	// Rotation
	AM_RANGE( 0x800000, 0x803fff) AM_READ( MRA16_RAM				)	// Foreground
	AM_RANGE( 0x804000, 0x807fff) AM_READ( MRA16_RAM				)	// Background
	AM_RANGE( 0x808000, 0x809fff) AM_READ( MRA16_RAM				)	// ???
	AM_RANGE( 0x900000, 0x903fff) AM_READ( rockn_nvram_r		    )	// NVRAM
	AM_RANGE( 0xa30000, 0xa30001) AM_READ( rockn_soundvolume_r	)	// Sound Volume
	AM_RANGE( 0xa40002, 0xa40003) AM_READ( tetrisp2_sound_r		)	// Sound
	AM_RANGE( 0xa44000, 0xa44001) AM_READ( rockn_adpcmbank_r		)	// Sound Bank
	AM_RANGE( 0xbe0000, 0xbe0001) AM_READ( MRA16_NOP				)	// INT-level1 dummy read
	AM_RANGE( 0xbe0002, 0xbe0003) AM_READ( input_port_0_word_r	)	// Inputs
	AM_RANGE( 0xbe0004, 0xbe0005) AM_READ( input_port_1_word_r	)	// Inputs
	AM_RANGE( 0xbe0008, 0xbe0009) AM_READ( input_port_2_word_r	)	// Inputs
	AM_RANGE( 0xbe000a, 0xbe000b) AM_READ( watchdog_reset16_r	)	// Watchdog
ADDRESS_MAP_END

static ADDRESS_MAP_START( rockn2_writemem , ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x0fffff) AM_WRITE( MWA16_ROM					)	// ROM
	AM_RANGE( 0x100000, 0x103fff) AM_WRITE( MWA16_RAM) AM_BASE( &spriteram16) AM_SIZE( &spriteram_size	)	// Object RAM
	AM_RANGE( 0x104000, 0x107fff) AM_WRITE( MWA16_RAM						)	// Spare Object RAM
	AM_RANGE( 0x108000, 0x10ffff) AM_WRITE( MWA16_RAM						)	// Work RAM
	AM_RANGE( 0x200000, 0x23ffff) AM_WRITE( rockn_priority_w) AM_BASE( &tetrisp2_priority	    )	// Priority
	AM_RANGE( 0x300000, 0x31ffff) AM_WRITE( tetrisp2_palette_w) AM_BASE( &paletteram16		)	// Palette
	AM_RANGE( 0x500000, 0x50ffff) AM_WRITE( MWA16_RAM									)	// Line
	AM_RANGE( 0x600000, 0x60ffff) AM_WRITE( tetrisp2_vram_rot_w) AM_BASE( &tetrisp2_vram_rot	)	// Rotation
	AM_RANGE( 0x800000, 0x803fff) AM_WRITE( tetrisp2_vram_fg_w) AM_BASE( &tetrisp2_vram_fg		)	// Foreground
	AM_RANGE( 0x804000, 0x807fff) AM_WRITE( tetrisp2_vram_bg_w) AM_BASE( &tetrisp2_vram_bg		)	// Background
	AM_RANGE( 0x808000, 0x809fff) AM_WRITE( MWA16_RAM						)	// ???
	AM_RANGE( 0x900000, 0x903fff) AM_WRITE( tetrisp2_nvram_w) AM_BASE( &tetrisp2_nvram) AM_SIZE( &tetrisp2_nvram_size	)	// NVRAM
	AM_RANGE( 0xa30000, 0xa30001) AM_WRITE( rockn_soundvolume_w					)	// Sound Volume
	AM_RANGE( 0xa40000, 0xa40003) AM_WRITE( tetrisp2_sound_w					)	// Sound
	AM_RANGE( 0xa44000, 0xa44001) AM_WRITE( rockn2_adpcmbank_w					    )	// Sound Bank
	AM_RANGE( 0xa48000, 0xa48001) AM_WRITE( MWA16_NOP						)	// YMZ280 Reset
	AM_RANGE( 0xb00000, 0xb00001) AM_WRITE( tetrisp2_coincounter_w					)	// Coin Counter
	AM_RANGE( 0xb20000, 0xb20001) AM_WRITE( MWA16_NOP						)	// ???
	AM_RANGE( 0xb40000, 0xb4000b) AM_WRITE( MWA16_RAM) AM_BASE( &tetrisp2_scroll_fg			)	// Foreground Scrolling
	AM_RANGE( 0xb40010, 0xb4001b) AM_WRITE( MWA16_RAM) AM_BASE( &tetrisp2_scroll_bg			)	// Background Scrolling
	AM_RANGE( 0xb4003e, 0xb4003f) AM_WRITE( MWA16_NOP						)	// scr_size
	AM_RANGE( 0xb60000, 0xb6002f) AM_WRITE( MWA16_RAM) AM_BASE( &tetrisp2_rotregs				)	// Rotation Registers
	AM_RANGE( 0xba0000, 0xba001f) AM_WRITE( rockn_systemregs_w					)	// system param
	AM_RANGE( 0xba001a, 0xba001b) AM_WRITE( MWA16_NOP						)	// Lev 4 irq ack
	AM_RANGE( 0xba001e, 0xba001f) AM_WRITE( MWA16_NOP						)	// Lev 2 irq ack
ADDRESS_MAP_END


static ADDRESS_MAP_START( rocknms_main_readmem , ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x0fffff) AM_READ(  MRA16_ROM				)	// ROM
	AM_RANGE( 0x100000, 0x103fff) AM_READ(  MRA16_RAM				)	// Object RAM
	AM_RANGE( 0x104000, 0x107fff) AM_READ(  MRA16_RAM				)	// Spare Object RAM
	AM_RANGE( 0x108000, 0x10ffff) AM_READ(  MRA16_RAM				)	// Work RAM
	AM_RANGE( 0x200000, 0x23ffff) AM_READ(  MRA16_RAM				)	// Priority
	AM_RANGE( 0x300000, 0x31ffff) AM_READ(  MRA16_RAM				)	// Palette
//	AM_RANGE( 0x500000, 0x50ffff) AM_READ(  MRA16_RAM				)	// Line
	AM_RANGE( 0x600000, 0x60ffff) AM_READ(  MRA16_RAM				)	// Rotation
	AM_RANGE( 0x800000, 0x803fff) AM_READ(  MRA16_RAM				)	// Foreground
	AM_RANGE( 0x804000, 0x807fff) AM_READ(  MRA16_RAM				)	// Background
//	AM_RANGE( 0x808000, 0x809fff) AM_READ(  MRA16_RAM				)	// ???
	AM_RANGE( 0x900000, 0x903fff) AM_READ(  rockn_nvram_r	    	)	// NVRAM
	AM_RANGE( 0xa30000, 0xa30001) AM_READ(  rockn_soundvolume_r	)	// Sound Volume
	AM_RANGE( 0xa40002, 0xa40003) AM_READ(  tetrisp2_sound_r		)	// Sound
	AM_RANGE( 0xa44000, 0xa44001) AM_READ(  rockn_adpcmbank_r		)	// Sound Bank
	AM_RANGE( 0xbe0000, 0xbe0001) AM_READ(  MRA16_NOP				)	// INT-level1 dummy read
	AM_RANGE( 0xbe0002, 0xbe0003) AM_READ(  rocknms_port_0_r	    )	// Inputs & MAIN <- SUB Communication
	AM_RANGE( 0xbe0004, 0xbe0005) AM_READ(  input_port_1_word_r	)	// Inputs
	AM_RANGE( 0xbe0008, 0xbe0009) AM_READ(  input_port_2_word_r	)	// Inputs
	AM_RANGE( 0xbe000a, 0xbe000b) AM_READ(  watchdog_reset16_r	)	// Watchdog
ADDRESS_MAP_END

static ADDRESS_MAP_START( rocknms_main_writemem , ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x0fffff) AM_WRITE( MWA16_ROM						)	// ROM
	AM_RANGE( 0x100000, 0x103fff) AM_WRITE( MWA16_RAM) AM_BASE( &spriteram16) AM_SIZE( &spriteram_size	)	// Object RAM
	AM_RANGE( 0x104000, 0x107fff) AM_WRITE( MWA16_RAM							)	// Spare Object RAM
	AM_RANGE( 0x108000, 0x10ffff) AM_WRITE( MWA16_RAM							)	// Work RAM
	AM_RANGE( 0x200000, 0x23ffff) AM_WRITE( rockn_priority_w) AM_BASE( &tetrisp2_priority	    )	// Priority
	AM_RANGE( 0x300000, 0x31ffff) AM_WRITE( tetrisp2_palette_w) AM_BASE( &paletteram16			)	// Palette
//	AM_RANGE( 0x500000, 0x50ffff) AM_WRITE( MWA16_RAM							)	// Line
	AM_RANGE( 0x600000, 0x60ffff) AM_WRITE( tetrisp2_vram_rot_w) AM_BASE( &tetrisp2_vram_rot	)	// Rotation
	AM_RANGE( 0x800000, 0x803fff) AM_WRITE( tetrisp2_vram_fg_w) AM_BASE( &tetrisp2_vram_fg		)	// Foreground
	AM_RANGE( 0x804000, 0x807fff) AM_WRITE( tetrisp2_vram_bg_w) AM_BASE( &tetrisp2_vram_bg		)	// Background
//	AM_RANGE( 0x808000, 0x809fff) AM_WRITE( MWA16_RAM							)	// ???
	AM_RANGE( 0x900000, 0x903fff) AM_WRITE( tetrisp2_nvram_w) AM_BASE( &tetrisp2_nvram) AM_SIZE( &tetrisp2_nvram_size	)	// NVRAM
	AM_RANGE( 0xa30000, 0xa30001) AM_WRITE( rockn_soundvolume_w						)	// Sound Volume
	AM_RANGE( 0xa40000, 0xa40003) AM_WRITE( tetrisp2_sound_w							)	// Sound
	AM_RANGE( 0xa44000, 0xa44001) AM_WRITE( rockn_adpcmbank_w							)	// Sound Bank
	AM_RANGE( 0xa48000, 0xa48001) AM_WRITE( MWA16_NOP				)	// YMZ280 Reset
	AM_RANGE( 0xa00000, 0xa00001) AM_WRITE( rocknms_main2sub_w	                    )	// MAIN -> SUB Communication
	AM_RANGE( 0xb00000, 0xb00001) AM_WRITE( tetrisp2_coincounter_w					)	// Coin Counter
	AM_RANGE( 0xb20000, 0xb20001) AM_WRITE( MWA16_NOP						)	// ???
	AM_RANGE( 0xb40000, 0xb4000b) AM_WRITE( MWA16_RAM) AM_BASE( &tetrisp2_scroll_fg			)	// Foreground Scrolling
	AM_RANGE( 0xb40010, 0xb4001b) AM_WRITE( MWA16_RAM) AM_BASE( &tetrisp2_scroll_bg			)	// Background Scrolling
	AM_RANGE( 0xb4003e, 0xb4003f) AM_WRITE( MWA16_NOP						)	// scr_size
	AM_RANGE( 0xb60000, 0xb6002f) AM_WRITE( MWA16_RAM) AM_BASE( &tetrisp2_rotregs				)	// Rotation Registers
	AM_RANGE( 0xba0000, 0xba001f) AM_WRITE( rockn_systemregs_w						)	// system param
	AM_RANGE( 0xba001a, 0xba001b) AM_WRITE( MWA16_NOP							)	// Lev 4 irq ack
	AM_RANGE( 0xba001e, 0xba001f) AM_WRITE( MWA16_NOP							)	// Lev 2 irq ack
ADDRESS_MAP_END


static ADDRESS_MAP_START( rocknms_sub_readmem , ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x0fffff) AM_READ( MRA16_ROM					)	// ROM
	AM_RANGE( 0x100000, 0x103fff) AM_READ( MRA16_RAM					)	// Object RAM
	AM_RANGE( 0x104000, 0x107fff) AM_READ( MRA16_RAM					)	// Spare Object RAM
	AM_RANGE( 0x108000, 0x10ffff) AM_READ( MRA16_RAM					)	// Work RAM
	AM_RANGE( 0x200000, 0x23ffff) AM_READ( rocknms_sub_priority_r	)	// Priority
	AM_RANGE( 0x300000, 0x31ffff) AM_READ( MRA16_RAM					)	// Palette
//	AM_RANGE( 0x500000, 0x50ffff) AM_READ( MRA16_RAM					)	// Line
	AM_RANGE( 0x600000, 0x60ffff) AM_READ( MRA16_RAM					)	// Rotation
	AM_RANGE( 0x800000, 0x803fff) AM_READ( MRA16_RAM					)	// Foreground
	AM_RANGE( 0x804000, 0x807fff) AM_READ( MRA16_RAM					)	// Background
//	AM_RANGE( 0x808000, 0x809fff) AM_READ( MRA16_RAM					)	// ???
	AM_RANGE( 0x900000, 0x907fff) AM_READ( MRA16_RAM					)	// NVRAM
//	AM_RANGE( 0xbe0000, 0xbe0001) AM_READ( MRA16_NOP					)	// INT-level1 dummy read
	AM_RANGE( 0xbe0002, 0xbe0003) AM_READ( rocknms_main2sub_r		)	// MAIN -> SUB Communication
	AM_RANGE( 0xbe000a, 0xbe000b) AM_READ( watchdog_reset16_r		)	// Watchdog
ADDRESS_MAP_END

static ADDRESS_MAP_START( rocknms_sub_writemem , ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x0fffff) AM_WRITE( MWA16_ROM							)	// ROM
	AM_RANGE( 0x100000, 0x103fff) AM_WRITE( MWA16_RAM) AM_BASE( &spriteram16_2) AM_SIZE(  &spriteram_2_size	)	// Object RAM
	AM_RANGE( 0x104000, 0x107fff) AM_WRITE( MWA16_RAM							)	// Spare Object RAM
	AM_RANGE( 0x108000, 0x10ffff) AM_WRITE( MWA16_RAM							)	// Work RAM
	AM_RANGE( 0x200000, 0x23ffff) AM_WRITE( rocknms_sub_priority_w) AM_BASE( &rocknms_sub_priority	)	// Priority
	AM_RANGE( 0x300000, 0x31ffff) AM_WRITE( rocknms_sub_palette_w) AM_BASE( &paletteram16_2		)	// Palette
//	AM_RANGE( 0x500000, 0x50ffff) AM_WRITE( MWA16_RAM						)	// Line
	AM_RANGE( 0x600000, 0x60ffff) AM_WRITE( rocknms_sub_vram_rot_w) AM_BASE( &rocknms_sub_vram_rot	)	// Rotation
	AM_RANGE( 0x800000, 0x803fff) AM_WRITE( rocknms_sub_vram_fg_w) AM_BASE( &rocknms_sub_vram_fg	)	// Foreground
	AM_RANGE( 0x804000, 0x807fff) AM_WRITE( rocknms_sub_vram_bg_w) AM_BASE( &rocknms_sub_vram_bg	)	// Background
//	AM_RANGE( 0x808000, 0x809fff) AM_WRITE( MWA16_RAM						)	// ???
	AM_RANGE( 0x900000, 0x907fff) AM_WRITE( MWA16_RAM						)	// NVRAM
	AM_RANGE( 0xb00000, 0xb00001) AM_WRITE( rocknms_sub2main_w							)	// MAIN <- SUB Communication
	AM_RANGE( 0xb20000, 0xb20001) AM_WRITE( MWA16_NOP								)	// ???
	AM_RANGE( 0xb40000, 0xb4000b) AM_WRITE( MWA16_RAM) AM_BASE( &rocknms_sub_scroll_fg				)	// Foreground Scrolling
	AM_RANGE( 0xb40010, 0xb4001b) AM_WRITE( MWA16_RAM) AM_BASE( &rocknms_sub_scroll_bg				)	// Background Scrolling
	AM_RANGE( 0xb4003e, 0xb4003f) AM_WRITE( MWA16_NOP								)	// scr_size
	AM_RANGE( 0xb60000, 0xb6002f) AM_WRITE( MWA16_RAM) AM_BASE( &rocknms_sub_rotregs				)	// Rotation Registers
	AM_RANGE( 0xba0000, 0xba001f) AM_WRITE( rocknms_sub_systemregs_w						)	// system param
	AM_RANGE( 0xba001a, 0xba001b) AM_WRITE( MWA16_NOP								)	// Lev 4 irq ack
	AM_RANGE( 0xba001e, 0xba001f) AM_WRITE( MWA16_NOP								)	// Lev 2 irq ack
	AM_RANGE( 0xbe0002, 0xbe0003) AM_WRITE( rocknms_sub2main_w							)	// MAIN <- SUB Communication (mirror)
ADDRESS_MAP_END


/***************************************************************************


                                Input Ports


***************************************************************************/

/***************************************************************************
                            Tetris Plus 2 (World)
***************************************************************************/

#define TETPLUS2_COMMON\
	PORT_START_TAG("IN0") /*$be0002.w*/ \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)\
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)\
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)\
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)\
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)\
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)\
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)\
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)\
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)\
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)\
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)\
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)\
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)\
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)\
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_START_TAG("IN1") /*$be0004.w*/\
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )\
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )\
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_START1   )\
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_START2   )\
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )\
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )\
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_COIN1    )\
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_COIN2    )\
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_SPECIAL  )	/* ?*/\
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_SPECIAL  )	/* ?*/\
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN  )\
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN  )\
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN  )\
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN  )\
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN  )\
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN  )


INPUT_PORTS_START( tetrisp2 )
TETPLUS2_COMMON

	PORT_START_TAG("IN2") //$be0008.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Vs Mode Rounds" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Language ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x1000, 0x1000, "F.B.I Logo" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END


/***************************************************************************
                            Tetris Plus 2 (Japan)
***************************************************************************/


INPUT_PORTS_START( teplus2j )
TETPLUS2_COMMON

/*
    The code for checking the "service mode" and "free play" DSWs
    is (deliberately?) bugged in this set
*/
	PORT_START_TAG("IN2")	// $be0008.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 1-6" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 1-7" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Vs Mode Rounds" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_UNUSED  ) // Language dip
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown 2-4" )	// F.B.I. Logo (in the USA set?)
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END


/***************************************************************************
                            Rock'n Tread (Japan)
***************************************************************************/


INPUT_PORTS_START( rockn1 )
	PORT_START_TAG("IN0")	//$be0002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")	//$be0004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_COIN2    )
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START_TAG("IN2")	//$be0008.w
	PORT_DIPNAME( 0x0001, 0x0001, "DIPSW 1-1") // All these used to be marked 'Cheat', can't think why.
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIPSW 1-2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIPSW 1-3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIPSW 1-4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIPSW 1-5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIPSW 1-6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIPSW 1-7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIPSW 1-8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME(0x0100, 0x0100, "DIPSW 2-1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(0x0200, 0x0200, "DIPSW 2-2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0400, 0x0400, "DIPSW 2-3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0800, 0x0800, "DIPSW 2-4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x1000, 0x1000, "DIPSW 2-5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x2000, 0x2000, "DIPSW 2-6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x4000, 0x4000, "DIPSW 2-7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x8000, 0x8000, "DIPSW 2-8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


INPUT_PORTS_START( rocknms )
	PORT_START_TAG("IN0")	//$be0002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_SPECIAL )		// MAIN -> SUB Communication
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_SPECIAL )		// MAIN -> SUB Communication
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1       ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2       ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1       ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2       ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)

	PORT_START_TAG("IN1")	//$be0004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_COIN2    )

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START_TAG("IN2")	//$be0008.w
	PORT_DIPNAME( 0x0001, 0x0001, "DIPSW 1-1") // All these used to be marked 'Cheat', can't think why.
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIPSW 1-2") 
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIPSW 1-3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIPSW 1-4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIPSW 1-5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIPSW 1-6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIPSW 1-7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIPSW 1-8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME(0x0100, 0x0100, "DIPSW 2-1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(0x0200, 0x0200, "DIPSW 2-2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0400, 0x0400, "DIPSW 2-3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0800, 0x0800, "DIPSW 2-4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x1000, 0x1000, "DIPSW 2-5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x2000, 0x2000, "DIPSW 2-6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x4000, 0x4000, "DIPSW 2-7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x8000, 0x8000, "DIPSW 2-8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************


                            Graphics Layouts


***************************************************************************/


/* 8x8x8 tiles */
static struct GfxLayout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

/* 16x16x8 tiles */
static struct GfxLayout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};

static struct GfxDecodeInfo tetrisp2_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &layout_8x8x8,   0x0000, 0x10 }, // [0] Sprites
	{ REGION_GFX2, 0, &layout_16x16x8, 0x1000, 0x10 }, // [1] Background
	{ REGION_GFX3, 0, &layout_16x16x8, 0x2000, 0x10 }, // [2] Rotation
	{ REGION_GFX4, 0, &layout_8x8x8,   0x6000, 0x10 }, // [3] Foreground
	{ -1 }
};

static struct GfxDecodeInfo rocknms_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &layout_8x8x8,   0x0000, 0x10 }, // [0] Sprites
	{ REGION_GFX2, 0, &layout_16x16x8, 0x1000, 0x10 }, // [1] Background
	{ REGION_GFX3, 0, &layout_16x16x8, 0x2000, 0x10 }, // [2] Rotation
	{ REGION_GFX4, 0, &layout_8x8x8,   0x6000, 0x10 }, // [3] Foreground
	{ REGION_GFX5, 0, &layout_8x8x8,   0x8000, 0x10 }, // [0] Sprites
	{ REGION_GFX6, 0, &layout_16x16x8, 0x9000, 0x10 }, // [1] Background
	{ REGION_GFX7, 0, &layout_16x16x8, 0xa000, 0x10 }, // [2] Rotation
	{ REGION_GFX8, 0, &layout_8x8x8,   0xe000, 0x10 }, // [3] Foreground
	{ -1 }
};


/***************************************************************************


                                Machine Drivers


***************************************************************************/

static struct YMZ280Binterface ymz280b_intf =
{
	REGION_SOUND1,
	0	// irq
};

void rockn_timer_level4_callback(int param)
{
	cpunum_set_input_line(0, 4, HOLD_LINE);
}

void rockn_timer_sub_level4_callback(int param)
{
	cpunum_set_input_line(1, 4, HOLD_LINE);
}

void rockn_timer_level1_callback(int param)
{
	cpunum_set_input_line(0, 1, HOLD_LINE);
}

void rockn_timer_sub_level1_callback(int param)
{
	cpunum_set_input_line(1, 1, HOLD_LINE);
}

DRIVER_INIT( rockn_timer )
{
	timer_pulse(TIME_IN_MSEC(32), 0, rockn_timer_level1_callback);
	rockn_timer_l4 = timer_alloc(rockn_timer_level4_callback);
}

DRIVER_INIT( rockn1 )
{
	init_rockn_timer();
	rockn_protectdata = 1;
}

DRIVER_INIT( rockn2 )
{
	init_rockn_timer();
	rockn_protectdata = 2;
}

DRIVER_INIT( rocknms )
{
	init_rockn_timer();

	timer_pulse(TIME_IN_MSEC(32), 0, rockn_timer_sub_level1_callback);
	rockn_timer_sub_l4 = timer_alloc(rockn_timer_sub_level4_callback);

	rockn_protectdata = 3;
}

DRIVER_INIT( rockn3 )
{
	init_rockn_timer();
	rockn_protectdata = 4;
}

static MACHINE_DRIVER_START( tetrisp2 )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(tetrisp2_readmem,tetrisp2_writemem)
	MDRV_CPU_VBLANK_INT(irq2_line_hold,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_NVRAM_HANDLER(tetrisp2)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)
	MDRV_GFXDECODE(tetrisp2_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(tetrisp2)
	MDRV_VIDEO_UPDATE(rockntread)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YMZ280B, 16934400)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( rockn1 )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(rockn1_readmem,rockn1_writemem)
	MDRV_CPU_VBLANK_INT(irq2_line_hold,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_NVRAM_HANDLER(tetrisp2)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)
	MDRV_GFXDECODE(tetrisp2_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(rockntread)
	MDRV_VIDEO_UPDATE(rockntread)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YMZ280B, 16934400)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( rockn2 )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(rockn2_readmem,rockn2_writemem)
	MDRV_CPU_VBLANK_INT(irq2_line_hold,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_NVRAM_HANDLER(tetrisp2)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)
	MDRV_GFXDECODE(tetrisp2_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(rockntread)
	MDRV_VIDEO_UPDATE(rockntread)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YMZ280B, 16934400)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( rocknms )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(rocknms_main_readmem,rocknms_main_writemem)
	MDRV_CPU_VBLANK_INT(irq2_line_hold,1)

	MDRV_CPU_ADD(M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(rocknms_sub_readmem,rocknms_sub_writemem)
	MDRV_CPU_VBLANK_INT(irq2_line_hold,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_NVRAM_HANDLER(tetrisp2)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_NEEDS_6BITS_PER_GUN | VIDEO_DUAL_MONITOR | VIDEO_RGB_DIRECT)
	MDRV_ASPECT_RATIO(4, 7)
	MDRV_SCREEN_SIZE(0x140, 0xe0+0x140)
	MDRV_VISIBLE_AREA(0, 0x140-1, 0, 0xe0+0x140-1)
	MDRV_GFXDECODE(rocknms_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x10000)

	MDRV_VIDEO_START(rocknms)
	MDRV_VIDEO_UPDATE(rocknms)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YMZ280B, 16934400)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/


/***************************************************************************

                                Tetris Plus 2

(C) Jaleco 1996

TP-97222
96019 EB-00-20117-0
MDK332V-0

BRIEF HARDWARE OVERVIEW

Toshiba TMP68HC000P-12
Yamaha YMZ280B-F
OSC: 12.000MHz, 48.000MHz, 16.9344MHz

Listing of custom chips. (Some on scan are hard to read).

IC38    JALECO SS91022-03 9428XX001
IC31    JALECO SS91022-05 9347EX002
IC32    JALECO GS91022-05    048  9726HX002
IC30    JALECO GS91022-04 9721PD008
IC39    XILINX XC5210 PQ240C X68710M AKJ9544
IC49    XILINX XC7336 PC44ACK9633 A63458A

***************************************************************************/

ROM_START( tetrisp2 )

	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "t2p_04.rom", 0x000000, 0x080000, CRC(e67f9c51) SHA1(d8b2937699d648267b163c7c3f591426877f3701) )
	ROM_LOAD16_BYTE( "t2p_01.rom", 0x000001, 0x080000, CRC(5020a4ed) SHA1(9c0f02fe3700761771ac026a2e375144e86e5eb7) )

	ROM_REGION( 0x800000, REGION_GFX1, ROMREGION_DISPOSE )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "96019-01.9", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "96019-02.8", 0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )
	/* If t2p_m01&2 from this board were correctly read, since they
       hold the same data of the above but with swapped halves, it
       means they had to invert the top bit of the "page select"
       register in the sprite's hardware on this board! */

	ROM_REGION( 0x800000, REGION_GFX2, ROMREGION_DISPOSE )	/* 16x16x8 (Background) */
	ROM_LOAD( "96019-06.13", 0x000000, 0x400000, CRC(16f7093c) SHA1(2be77c6a692c5d762f5553ae24e8c415ab194cc6) )
	ROM_LOAD( "96019-04.6",  0x400000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "96019-04.6",  0x000000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x080000, REGION_GFX4, ROMREGION_DISPOSE )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "tetp2-10.bin", 0x000000, 0x080000, CRC(34dd1bad) SHA1(9bdf1dde11f82839676400de5dd7acb06ea8cdb2) )	// 11111xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, REGION_SOUND1, ROMREGION_SOUNDONLY )	/* Samples */
	ROM_LOAD( "96019-07.7", 0x000000, 0x400000, CRC(a8a61954) SHA1(86c3db10b348ba1f44ff696877b8b20845fa53de) )

ROM_END


/***************************************************************************

                            Tetris Plus 2 (Japan)

(c)1997 Jaleco / The Tetris Company

TP-97222
96019 EB-00-20117-0

CPU:    68000-12
Sound:  YMZ280B-F
OSC:    12.000MHz
        48.0000MHz
        16.9344MHz

Custom: SS91022-03
        GS91022-04
        GS91022-05
        SS91022-05

***************************************************************************/

ROM_START( teplus2j )

	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "tet2-4v2.2", 0x000000, 0x080000, CRC(5bfa32c8) SHA1(55fb2872695fcfbad13f5c0723302e72da69e44a) )	// v2.2
	ROM_LOAD16_BYTE( "tet2-1v2.2", 0x000001, 0x080000, CRC(919116d0) SHA1(3e1c0fd4c9175b2900a4717fbb9e8b591c5f534d) )

	ROM_REGION( 0x800000, REGION_GFX1, ROMREGION_DISPOSE )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "96019-01.9", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "96019-02.8", 0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )

	ROM_REGION( 0x400000, REGION_GFX2, ROMREGION_DISPOSE )	/* 16x16x8 (Background) */
	ROM_LOAD( "96019-06.13", 0x000000, 0x400000, CRC(16f7093c) SHA1(2be77c6a692c5d762f5553ae24e8c415ab194cc6) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "96019-04.6",  0x000000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x080000, REGION_GFX4, ROMREGION_DISPOSE )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "tetp2-10.bin", 0x000000, 0x080000, CRC(34dd1bad) SHA1(9bdf1dde11f82839676400de5dd7acb06ea8cdb2) )	// 11111xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, REGION_SOUND1, ROMREGION_SOUNDONLY )	/* Samples */
	ROM_LOAD( "96019-07.7", 0x000000, 0x400000, CRC(a8a61954) SHA1(86c3db10b348ba1f44ff696877b8b20845fa53de) )

ROM_END


/***************************************************************************

							Rock'n Tread 1 (Japan)
							Rock'n Tread 2 (Japan)
							Rock'n MegaSession (Japan)
							Rock'n 3 (Japan)
							Rock'n 4 (Japan)

(c)1997 Jaleco

CPU:	68000-12
Sound:	YMZ280B-F
OSC:	12.000MHz
		48.0000MHz
		16.9344MHz

Custom:	SS91022-03
		GS91022-04
		GS91022-05
		SS91022-05

***************************************************************************/

ROM_START( rockn )

	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "98344_1.bin", 0x000001, 0x80000, CRC(4cf79e58) SHA1(f50e596d43c9ab2072ae0476169eee2a8512fd8d) )
	ROM_LOAD16_BYTE( "98344_4.bin", 0x000000, 0x80000, CRC(caa33f79) SHA1(8ccff67091dac5ad871cae6cdb31e1fc37c1a4c2) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "98344_8.bin", 0x000002, 0x200000, CRC(fa3f6f9c) SHA1(586dcc690a1a4aa7c97932ad496382def6a074a4) )
	ROM_LOAD32_WORD( "98344_9.bin", 0x000000, 0x200000, CRC(3d12a688) SHA1(356b2ea81d960838b604c5a17cc77e79fb0e40ce) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "98344_13.bin", 0x000000, 0x200000, CRC(261b99a0) SHA1(7b3c768ae9d7429e2559fe32c1a4ff220d727e7e) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "98344_6.bin", 0x000000, 0x100000, CRC(5551717f) SHA1(64943a9a68ad4074f3f5128d7796e4f03baa14d5) )

	ROM_REGION( 0x080000, REGION_GFX4, ROMREGION_DISPOSE )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "98344_10.bin", 0x000000, 0x080000, CRC(918663a8) SHA1(aedacb741c986ef8159385cfef866cb7e3ef6cb6) )

//	ROM_REGION( 0x8000000, REGION_SOUND1, 0 )	/* Samples */
//	ROM_LOAD( "audio", 0x0000000, 0x8000000, NO_DUMP ) // not dumped (number of roms / type unknown)

	ROM_REGION( 0x8000000, REGION_SOUND1, 0 )	/* Samples */
	ROM_LOAD( "audio", 0x0000000, 0x8000000, NO_DUMP ) // not dumped (number of roms / type unknown)
ROM_END

ROM_START( rockn1 )

	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* 68000 Code */
	ROM_LOAD16_WORD( "live.bin", 0x000000, 0x100000, CRC(ad90f2a3) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE )	/* 8x8x8 (Sprites) */
	ROM_LOAD16_WORD( "spr.img", 0x000000, 0x400000, CRC(eef37ad6) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "scr.bin", 0x000000, 0x200000, CRC(261b99a0) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rot.bin", 0x000000, 0x100000, CRC(5551717f) )

	ROM_REGION( 0x080000, REGION_GFX4, ROMREGION_DISPOSE )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "asc.bin", 0x000000, 0x080000, CRC(918663a8) )

	ROM_REGION( 0x7000000, REGION_SOUND1, 0 )	/* Samples */
	ROM_LOAD( "ajrt00ma.shp", 0x0000000, 0x0400000, CRC(c354f753) ) // COMMON AREA
	ROM_FILL(                 0x0400000, 0x0c00000, 0xffffffff ) // BANK AREA
	ROM_LOAD( "ajrt01ma.shp", 0x1000000, 0x0400000, CRC(5b42999e))  // bank 0
	ROM_LOAD( "ajrt02ma.shp", 0x1400000, 0x0400000, CRC(8306f302))  // bank 0
	ROM_LOAD( "ajrt03ma.shp", 0x1800000, 0x0400000, CRC(3fda842c))  // bank 0
	ROM_LOAD( "ajrt04ma.shp", 0x1c00000, 0x0400000, CRC(86d4f289))  // bank 1
	ROM_LOAD( "ajrt05ma.shp", 0x2000000, 0x0400000, CRC(f8dbf47d))  // bank 1
	ROM_LOAD( "ajrt06ma.shp", 0x2400000, 0x0400000, CRC(525aff97))  // bank 1
	ROM_LOAD( "ajrt07ma.shp", 0x2800000, 0x0400000, CRC(5bd8bb95))  // bank 2
	ROM_LOAD( "ajrt08ma.shp", 0x2c00000, 0x0400000, CRC(304c1643))  // bank 2
	ROM_LOAD( "ajrt09ma.shp", 0x3000000, 0x0400000, CRC(78c22c56))  // bank 2
	ROM_LOAD( "ajrt10ma.shp", 0x3400000, 0x0400000, CRC(d5e8d8a5))  // bank 3
	ROM_LOAD( "ajrt13ma.shp", 0x3800000, 0x0400000, CRC(569ef4dd))  // bank 3
	ROM_LOAD( "ajrt14ma.shp", 0x3c00000, 0x0400000, CRC(aae8d59c))  // bank 3
	ROM_LOAD( "ajrt15ma.shp", 0x4000000, 0x0400000, CRC(9ec1459b))  // bank 4
	ROM_LOAD( "ajrt16ma.shp", 0x4400000, 0x0400000, CRC(b26f9a81))  // bank 4

ROM_END


ROM_START( rockn2 )

	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* 68000 Code */
	ROM_LOAD16_WORD( "live.bin", 0x000000, 0x100000, CRC(369750fd) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE )	/* 8x8x8 (Sprites) */
	ROM_LOAD16_WORD( "spr.img", 0x000000, 0x3e0000, CRC(13de1054) )

	ROM_REGION( 0x1a7400, REGION_GFX2, ROMREGION_DISPOSE )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "scr.bin", 0x000000, 0x1a7400, CRC(9f35b2cf) )

	ROM_REGION( 0x130e00, REGION_GFX3, ROMREGION_DISPOSE )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rot.bin", 0x000000, 0x130e00, CRC(4d9989ff) )

	ROM_REGION( 0x019c00, REGION_GFX4, ROMREGION_DISPOSE )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "asc.bin", 0x000000, 0x019c00, CRC(6e724e73) )

	ROM_REGION( 0x7000000, REGION_SOUND1, 0 )	/* Samples */
	ROM_LOAD( "snd25.bin", 0x0000000, 0x0400000, CRC(4e9611a3) ) // COMMON AREA
	ROM_FILL(              0x0400000, 0x0c00000, 0xffffffff ) 		  // BANK AREA
	ROM_LOAD( "snd01.bin", 0x1000000, 0x0400000, CRC(ec600f13))  // bank 0
	ROM_LOAD( "snd02.bin", 0x1400000, 0x0400000, CRC(8306f302))  // bank 0
	ROM_LOAD( "snd03.bin", 0x1800000, 0x0400000, CRC(3fda842c) ) // bank 0
	ROM_LOAD( "snd04.bin", 0x1c00000, 0x0400000, CRC(86d4f289))  // bank 1
	ROM_LOAD( "snd05.bin", 0x2000000, 0x0400000, CRC(f8dbf47d))  // bank 1
	ROM_LOAD( "snd06.bin", 0x2400000, 0x0400000, CRC(06f7bd63))  // bank 1
	ROM_LOAD( "snd07.bin", 0x2800000, 0x0400000, CRC(22f042f6) ) // bank 2
	ROM_LOAD( "snd08.bin", 0x2c00000, 0x0400000, CRC(dd294d8e))  // bank 2
	ROM_LOAD( "snd09.bin", 0x3000000, 0x0400000, CRC(8fedee6e) ) // bank 2
	ROM_LOAD( "snd10.bin", 0x3400000, 0x0400000, CRC(01292f11) ) // bank 3
	ROM_LOAD( "snd11.bin", 0x3800000, 0x0400000, CRC(20dc76ba) ) // bank 3
	ROM_LOAD( "snd12.bin", 0x3c00000, 0x0400000, CRC(11fff0bc) ) // bank 3
	ROM_LOAD( "snd13.bin", 0x4000000, 0x0400000, CRC(2367dd18) ) // bank 4
	ROM_LOAD( "snd14.bin", 0x4400000, 0x0400000, CRC(75ced8c0) ) // bank 4
	ROM_LOAD( "snd15.bin", 0x4800000, 0x0400000, CRC(aeaca380) ) // bank 4
	ROM_LOAD( "snd16.bin", 0x4c00000, 0x0400000, CRC(21d50e32) ) // bank 5
	ROM_LOAD( "snd17.bin", 0x5000000, 0x0400000, CRC(de785a2a) ) // bank 5
	ROM_LOAD( "snd18.bin", 0x5400000, 0x0400000, CRC(18cabb1e) ) // bank 5
	ROM_LOAD( "snd19.bin", 0x5800000, 0x0400000, CRC(33c89e53) ) // bank 6
	ROM_LOAD( "snd20.bin", 0x5c00000, 0x0400000, CRC(89c1b088) ) // bank 6
	ROM_LOAD( "snd21.bin", 0x6000000, 0x0400000, CRC(13db74bd) ) // bank 6

ROM_END


ROM_START( rockn3 )

	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* 68000 Code */
	ROM_LOAD16_WORD( "live.bin", 0x000000, 0x100000, CRC(31895ef5) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE )	/* 8x8x8 (Sprites) */
	ROM_LOAD16_WORD( "spr.img", 0x000000, 0x400000, CRC(3fa0a3fa) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "scr.bin", 0x000000, 0x200000, CRC(e01bf471) )

	ROM_REGION( 0x200000, REGION_GFX3, ROMREGION_DISPOSE )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rot.bin", 0x000000, 0x200000, CRC(4e146de5) )

	ROM_REGION( 0x080000, REGION_GFX4, ROMREGION_DISPOSE )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "asc.bin", 0x000000, 0x080000, CRC(8100039e) )

	ROM_REGION( 0x7000000, REGION_SOUND1, 0 )	/* Samples */
	ROM_LOAD( "rt3rd001.bin", 0x0000000, 0x0400000, CRC(e2f69042) ) // COMMON AREA
	ROM_FILL(                 0x0400000, 0x0c00000, 0xffffffff ) 		 // BANK AREA
	ROM_LOAD( "rt3rd002.bin", 0x1000000, 0x0400000, CRC(b328b18f) ) // bank 0
	ROM_LOAD( "rt3rd003.bin", 0x1400000, 0x0400000, CRC(f46438e3) ) // bank 0
	ROM_LOAD( "rt3rd004.bin", 0x1800000, 0x0400000, CRC(b979e887) ) // bank 0
	ROM_LOAD( "rt3rd005.bin", 0x1c00000, 0x0400000, CRC(0bb2c212) ) // bank 1
	ROM_LOAD( "rt3rd006.bin", 0x2000000, 0x0400000, CRC(3116e437) ) // bank 1
	ROM_LOAD( "rt3rd007.bin", 0x2400000, 0x0400000, CRC(26b37ef6) ) // bank 1
	ROM_LOAD( "rt3rd008.bin", 0x2800000, 0x0400000, CRC(1dd3f4e3) ) // bank 2
	ROM_LOAD( "rt3rd009.bin", 0x2c00000, 0x0400000, CRC(a1b03d67) ) // bank 2
	ROM_LOAD( "rt3rd010.bin", 0x3000000, 0x0400000, CRC(35107aac) ) // bank 2
	ROM_LOAD( "rt3rd011.bin", 0x3400000, 0x0400000, CRC(059ec592))  // bank 3
	ROM_LOAD( "rt3rd012.bin", 0x3800000, 0x0400000, CRC(84d4badb) ) // bank 3
	ROM_LOAD( "rt3rd013.bin", 0x3c00000, 0x0400000, CRC(4527a9b7) ) // bank 3
	ROM_LOAD( "rt3rd014.bin", 0x4000000, 0x0400000, CRC(bfa4b7ce) ) // bank 4
	ROM_LOAD( "rt3rd015.bin", 0x4400000, 0x0400000, CRC(a2ccd2ce) ) // bank 4
	ROM_LOAD( "rt3rd016.bin", 0x4800000, 0x0400000, CRC(95baf678) ) // bank 4
	ROM_LOAD( "rt3rd017.bin", 0x4c00000, 0x0400000, CRC(5883c84b) ) // bank 5
	ROM_LOAD( "rt3rd018.bin", 0x5000000, 0x0400000, CRC(f92098ce) ) // bank 5
	ROM_LOAD( "rt3rd019.bin", 0x5400000, 0x0400000, CRC(dbb2c228) ) // bank 5
	ROM_LOAD( "rt3rd020.bin", 0x5800000, 0x0400000, CRC(9efdae1c) ) // bank 6
	ROM_LOAD( "rt3rd021.bin", 0x5c00000, 0x0400000, CRC(5f301b83) ) // bank 6

ROM_END


ROM_START( rockn4 )

	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* 68000 Code */
	ROM_LOAD16_WORD( "live.bin", 0x000000, 0x100000, CRC(f2c9ff1c) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE )	/* 8x8x8 (Sprites) */
	ROM_LOAD16_WORD( "spr.img", 0x000000, 0x400000, CRC(5ca2228e) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "scr.bin", 0x000000, 0x200000, CRC(ead41e79) )

	ROM_REGION( 0x200000, REGION_GFX3, ROMREGION_DISPOSE )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rot.bin", 0x000000, 0x200000, CRC(eb16fc67) )

	ROM_REGION( 0x100000, REGION_GFX4, ROMREGION_DISPOSE )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "asc.bin", 0x000000, 0x100000, CRC(37d50259) )

	ROM_REGION( 0x7000000, REGION_SOUND1, 0 )	/* Samples */
	ROM_LOAD( "snd25.bin", 0x0000000, 0x0400000, CRC(918ea8eb) ) // COMMON AREA
	ROM_FILL(              0x0400000, 0x0c00000, 0xffffffff ) 		  // BANK AREA
	ROM_LOAD( "snd01.bin", 0x1000000, 0x0400000, CRC(c548e51e) ) // bank 0
	ROM_LOAD( "snd02.bin", 0x1400000, 0x0400000, CRC(ffda0253) ) // bank 0
	ROM_LOAD( "snd03.bin", 0x1800000, 0x0400000, CRC(1f813af5) ) // bank 0
	ROM_LOAD( "snd04.bin", 0x1c00000, 0x0400000, CRC(035c4ff3))  // bank 1
	ROM_LOAD( "snd05.bin", 0x2000000, 0x0400000, CRC(0f01f7b0) ) // bank 1
	ROM_LOAD( "snd06.bin", 0x2400000, 0x0400000, CRC(31574b1c))  // bank 1
	ROM_LOAD( "snd07.bin", 0x2800000, 0x0400000, CRC(388e2c91))  // bank 2
	ROM_LOAD( "snd08.bin", 0x2c00000, 0x0400000, CRC(6e7e3f23))  // bank 2
	ROM_LOAD( "snd09.bin", 0x3000000, 0x0400000, CRC(39fa512f) ) // bank 2

ROM_END


ROM_START( rocknms )

	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* 68000 Code */
	ROM_LOAD16_WORD( "live_m.bin", 0x000000, 0x100000, CRC(857483da) )

	ROM_REGION( 0x100000, REGION_CPU2, 0 )		/* 68000 Code */
	ROM_LOAD16_WORD( "live_s.bin", 0x000000, 0x100000, CRC(630dcb2e) )

	ROM_REGION( 0x0800000, REGION_GFX1, ROMREGION_DISPOSE )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "spr_m.im1", 0x000000, 0x400000, CRC(1caad02a) )
	ROM_LOAD32_WORD( "spr_m.im2", 0x000002, 0x400000, CRC(520152dc) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "scr_m.bin", 0x000000, 0x200000, CRC(1ca30e3f) )

	ROM_REGION( 0x200000, REGION_GFX3, ROMREGION_DISPOSE )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rot_m.bin", 0x000000, 0x200000, CRC(1f29b622) )

	ROM_REGION( 0x080000, REGION_GFX4, ROMREGION_DISPOSE )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "asc_m.bin", 0x000000, 0x080000, CRC(a4717579) )

	ROM_REGION( 0x0680000, REGION_GFX5, ROMREGION_DISPOSE )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "spr_s.im1", 0x000000, 0x340000, CRC(eea4699c) )
	ROM_LOAD32_WORD( "spr_s.im2", 0x000002, 0x340000, CRC(2c02a1ec) )

	ROM_REGION( 0x0ac900, REGION_GFX6, ROMREGION_DISPOSE )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "scr_s.bin", 0x000000, 0x0ac900, CRC(a00dfdaa) )

	ROM_REGION( 0x148300, REGION_GFX7, ROMREGION_DISPOSE )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rot_s.bin", 0x000000, 0x148300, CRC(c76eff18) )

	ROM_REGION( 0x02c700, REGION_GFX8, ROMREGION_DISPOSE )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "asc_s.bin", 0x000000, 0x02c700, CRC(26a8bcaa) )

	ROM_REGION( 0x7000000, REGION_SOUND1, 0 )	/* Samples */
	ROM_LOAD( "rtdx001.bin", 0x0000000, 0x0400000, CRC(8bafae71) ) // COMMON AREA
	ROM_FILL(                0x0400000, 0x0c00000, 0xffffffff ) 		// BANK AREA
	ROM_LOAD( "rtdx002.bin", 0x1000000, 0x0400000, CRC(eec0589b))  // bank 0
	ROM_LOAD( "rtdx003.bin", 0x1400000, 0x0400000, CRC(564aa972) ) // bank 0
	ROM_LOAD( "rtdx004.bin", 0x1800000, 0x0400000, CRC(940302d0) ) // bank 0
	ROM_LOAD( "rtdx005.bin", 0x1c00000, 0x0400000, CRC(766db7f8) ) // bank 1
	ROM_LOAD( "rtdx006.bin", 0x2000000, 0x0400000, CRC(3a3002f9) ) // bank 1
	ROM_LOAD( "rtdx007.bin", 0x2400000, 0x0400000, CRC(06b04df9) ) // bank 1
	ROM_LOAD( "rtdx008.bin", 0x2800000, 0x0400000, CRC(da74305e) ) // bank 2
	ROM_LOAD( "rtdx009.bin", 0x2c00000, 0x0400000, CRC(b5a0aa48) ) // bank 2
	ROM_LOAD( "rtdx010.bin", 0x3000000, 0x0400000, CRC(0fd4a088) ) // bank 2
	ROM_LOAD( "rtdx011.bin", 0x3400000, 0x0400000, CRC(33c89e53) ) // bank 3
	ROM_LOAD( "rtdx012.bin", 0x3800000, 0x0400000, CRC(f9256a3f) ) // bank 3
	ROM_LOAD( "rtdx013.bin", 0x3c00000, 0x0400000, CRC(b0a09f3e) ) // bank 3
	ROM_LOAD( "rtdx014.bin", 0x4000000, 0x0400000, CRC(d5cee673) ) // bank 4
	ROM_LOAD( "rtdx015.bin", 0x4400000, 0x0400000, CRC(b394aa8a) ) // bank 4
	ROM_LOAD( "rtdx016.bin", 0x4800000, 0x0400000, CRC(6c791501) ) // bank 4
	ROM_LOAD( "rtdx017.bin", 0x4c00000, 0x0400000, CRC(fe80159e) ) // bank 5
	ROM_LOAD( "rtdx018.bin", 0x5000000, 0x0400000, CRC(142c1159) ) // bank 5
	ROM_LOAD( "rtdx019.bin", 0x5400000, 0x0400000, CRC(cc595d85) ) // bank 5
	ROM_LOAD( "rtdx020.bin", 0x5800000, 0x0400000, CRC(82b085a3) ) // bank 6
	ROM_LOAD( "rtdx021.bin", 0x5c00000, 0x0400000, CRC(dd5e9680) ) // bank 6

ROM_END


/***************************************************************************/

#define ROCKNMS_MONITOR 0

#if (ROCKNMS_MONITOR == 1)
static MACHINE_DRIVER_START( rocknmt1 )
	MDRV_IMPORT_FROM(rocknms)
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_ASPECT_RATIO(4, 3)
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)
	MDRV_GFXDECODE(tetrisp2_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x8000)
	MDRV_VIDEO_UPDATE(rockntread)
MACHINE_DRIVER_END
#endif

#if (ROCKNMS_MONITOR == 2)
VIDEO_UPDATE( rocknmt2 );
static MACHINE_DRIVER_START( rocknmt2 )
	MDRV_IMPORT_FROM(rocknms)
	MDRV_ASPECT_RATIO(7, 4)
	MDRV_SCREEN_SIZE(0x140+0xe0, 0x140)
	MDRV_VISIBLE_AREA(0, 0x140+0xe0-1, 0, 0x140-1)
	MDRV_VIDEO_UPDATE(rocknmt2)
MACHINE_DRIVER_END
#endif


/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1997, tetrisp2, 0,        tetrisp2, tetrisp2, 0,       ROT0,   "Jaleco / The Tetris Company", "Tetris Plus 2 (World?)" )
GAME( 1997, teplus2j, tetrisp2, tetrisp2, teplus2j, 0,       ROT0,   "Jaleco / The Tetris Company", "Tetris Plus 2 (Japan)" )
GAMEX(1999, rockn,    0,        rockn1,   rockn1,   rockn1,  ROT270, "Jaleco", "Rock'n Tread (Japan)", GAME_NO_SOUND )
GAME( 1999, rockn1,   0,        rockn1,   rockn1,   rockn1,  ROT270, "Jaleco", "Rock'n Tread 1 (Japan)" )
GAME( 1999, rockn2,   0,        rockn2,   rockn1,   rockn2,  ROT270, "Jaleco", "Rock'n Tread 2 (Japan)" )

#if (ROCKNMS_MONITOR == 0)
GAMEX(1999, rocknms,  0,        rocknms,  rocknms,  rocknms, ROT0,   "Jaleco", "Rock'n MegaSession (Japan)", GAME_IMPERFECT_GRAPHICS )
#endif
#if (ROCKNMS_MONITOR == 1)
GAMEX(1999, rocknms,  0,        rocknmt1, rocknms,  rocknms, ROT270, "Jaleco", "Rock'n MegaSession (Japan)", GAME_IMPERFECT_GRAPHICS )
#endif
#if (ROCKNMS_MONITOR == 2)
GAMEX(1999, rocknms,  0,        rocknmt2, rocknms,  rocknms, ROT0,   "Jaleco", "Rock'n MegaSession (Japan)", GAME_IMPERFECT_GRAPHICS )
#endif

GAME( 1999, rockn3,   0,        rockn2,   rockn1,   rockn3,  ROT270, "Jaleco", "Rock'n 3 (Japan)" )
GAME( 1999, rockn4,   0,        rockn2,   rockn1,   rockn3,  ROT270, "Jaleco (PCCWJ)", "Rock'n 4 (Japan prototype version)" )
