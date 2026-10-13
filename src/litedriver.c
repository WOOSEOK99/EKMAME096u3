/******************************************************************************

  driver.c

  The list of all available drivers. Drivers have to be included here to be
  recognized by the executable.

  To save some typing, we use a hack here. This file is recursively #included
  twice, with different definitions of the DRIVER() macro. The first one
  declares external references to the drivers; the second one builds an array
  storing all the drivers.

******************************************************************************/

#include "driver.h"


#ifndef DRIVER_RECURSIVE

/* The "root" driver, defined so we can have &driver_##NAME in macros. */
const struct GameDriver driver_0 =
{
	__FILE__,
	0,
	"",
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	NOT_A_DRIVER
};

#endif

#ifdef TINY_COMPILE

extern const struct GameDriver TINY_NAME;

const struct GameDriver *drivers[] =
{
	TINY_POINTER,
	0	/* end of array */
};

#else

#ifndef DRIVER_RECURSIVE

#define DRIVER_RECURSIVE

/* step 1: declare all external references */
#define DRIVER(NAME) extern struct GameDriver driver_##NAME;
#include "litedriver.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &driver_##NAME,
const struct GameDriver *drivers[] =
{
#include "litedriver.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

	/* Capcom CPS1 games */
	DRIVER( forgottn )	/*  7/1988 (c) 1988 (US) */
	DRIVER( lostwrld )	/*  7/1988 (c) 1988 (Japan) */
	DRIVER( ghouls )	/* 12/1988 (c) 1988 (World) */
	DRIVER( ghoulsu )	/* 12/1988 (c) 1988 (US) */
	DRIVER( daimakai )	/* 12/1988 (c) 1988 (Japan) */
	DRIVER( strider )	/*  3/1989 (c) 1989 (not explicitly stated but should be US) */
	DRIVER( stridrua )	/*  3/1989 (c) 1989 (not explicitly stated but should be US) */
	DRIVER( striderj )	/*  3/1989 (c) 1989 */
	DRIVER( stridrja )	/*  3/1989 (c) 1989 */
	DRIVER( dynwar )	/*  4/1989 (c) 1989 (World) */
	DRIVER( dynwarj )	/*  4/1989 (c) 1989 (Japan) */
	DRIVER( willow )	/*  6/1989 (c) 1989 (US) */
	DRIVER( willowj )	/*  6/1989 (c) 1989 (Japan) */
	DRIVER( willowje )	/*  6/1989 (c) 1989 (Japan) */
	DRIVER( unsquad )	/*  8/1989 (c) 1989 */
	DRIVER( area88 )	/*  8/1989 (c) 1989 */
	DRIVER( ffight )	/* 12/1989 (c) (World) */
	DRIVER( ffightu )	/* 12/1989 (c) (US)    */
	DRIVER( ffightj )	/* 12/1989 (c) (Japan) */
	DRIVER( ffightj1 )	/* 12/1989 (c) (Japan) */
	DRIVER( 1941 )		/*  2/1990 (c) 1990 (World) */
	DRIVER( 1941j )		/*  2/1990 (c) 1990 (Japan) */
	DRIVER( mercs )		/* 02/03/1990 (c) 1990 (World) */
	DRIVER( mercsu )	/* 02/03/1990 (c) 1990 (US)    */
	DRIVER( mercsua )	/* 08/06/1990 (c) 1990 (US)    */
	DRIVER( mercsj )	/* 02/03/1990 (c) 1990 (Japan) */
	DRIVER( mtwins )	/* 19/06/1990 (c) 1990 (World) */
	DRIVER( chikij )	/* 19/06/1990 (c) 1990 (Japan) */
	DRIVER( msword )	/* 25/07/1990 (c) 1990 (World) */
	DRIVER( mswordr1 )	/* 23/06/1990 (c) 1990 (World) */
	DRIVER( mswordu )	/* 25/07/1990 (c) 1990 (US)    */
	DRIVER( mswordj )	/* 23/06/1990 (c) 1990 (Japan) */
	DRIVER( cawing )	/* 12/10/1990 (c) 1990 (World) */
	DRIVER( cawingu )	/* 12/10/1990 (c) 1990 (US) */
	DRIVER( cawingj )	/* 12/10/1990 (c) 1990 (Japan) */
	DRIVER( nemo )		/* 30/11/1990 (c) 1990 (World) */
	DRIVER( nemoj )		/* 20/11/1990 (c) 1990 (Japan) */
	DRIVER( sf2 )		/* 22/05/1991 (c) 1991 (World) */
	DRIVER( sf2eb )		/* 14/02/1991 (c) 1991 (World) */
	DRIVER( sf2ua )		/* 06/02/1991 (c) 1991 (US)    */
	DRIVER( sf2ub )		/* 14/02/1991 (c) 1991 (US)    */
	DRIVER( sf2ud )		/* 18/03/1991 (c) 1991 (US)    */
	DRIVER( sf2ue )		/* 28/02/1991 (c) 1991 (US)    */
	DRIVER( sf2uf )		/* 11/04/1991 (c) 1991 (US)    */
	DRIVER( sf2ui )		/* 22/05/1991 (c) 1991 (US)    */
	DRIVER( sf2uk )		/* 01/11/1991 (c) 1991 (US)    */
	DRIVER( sf2j )		/* 10/12/1991 (c) 1991 (Japan) */
	DRIVER( sf2ja )		/* 14/02/1991 (c) 1991 (Japan) */
	DRIVER( sf2jc )		/* 06/03/1991 (c) 1991 (Japan) */
	DRIVER( 3wonders )	/* 20/05/1991 (c) 1991 (World) */
	DRIVER( 3wonderu )	/* 20/05/1991 (c) 1991 (US)    */
	DRIVER( wonder3 )	/* 20/05/1991 (c) 1991 (Japan) */
	DRIVER( kod )		/* 11/07/1991 (c) 1991 (World) */
	DRIVER( kodu )		/* 10/09/1991 (c) 1991 (US)    */
	DRIVER( kodj )		/* 05/08/1991 (c) 1991 (Japan) */
	DRIVER( kodb )		/* bootleg */
	DRIVER( captcomm )	/* 14/10/1991 (c) 1991 (World) */
	DRIVER( captcomu )	/* 28/ 9/1991 (c) 1991 (US)    */
	DRIVER( captcomj )	/* 02/12/1991 (c) 1991 (Japan) */
	DRIVER( knights )	/* 27/11/1991 (c) 1991 (World) */
	DRIVER( knightsu )	/* 27/11/1991 (c) 1991 (US)    */
	DRIVER( knightsj )	/* 27/11/1991 (c) 1991 (Japan) */
	DRIVER( sf2ce )		/* 13/03/1992 (c) 1992 (World) */
	DRIVER( sf2ceua )	/* 13/03/1992 (c) 1992 (US)    */
	DRIVER( sf2ceub )	/* 13/05/1992 (c) 1992 (US)    */
	DRIVER( sf2ceuc )	/* 03/08/1992 (c) 1992 (US)    */
	DRIVER( sf2cej )	/* 13/05/1992 (c) 1992 (Japan) */
	DRIVER( sf2rb )		/* hack */
	DRIVER( sf2rb2 )	/* hack */
	DRIVER( sf2red )	/* hack */
	DRIVER( sf2v004 )	/* hack */
	DRIVER( sf2accp2 )	/* hack */
	DRIVER( sf2m1 )		/* hack */
	DRIVER( sf2m2 )		/* hack */
	DRIVER( sf2m3 )		/* hack */
	DRIVER( sf2m4 )		/* hack */
	DRIVER( sf2m5 )		/* hack */
	DRIVER( sf2m6 )		/* hack */
	DRIVER( sf2m7 )		/* hack */
	DRIVER( sf2yyc )	/* hack */
	DRIVER( sf2koryu )	/* hack */
	DRIVER( varth )		/* 12/06/1992 (c) 1992 (World) */
	DRIVER( varthu )	/* 12/06/1992 (c) 1992 (US) */
	DRIVER( varthj )	/* 14/07/1992 (c) 1992 (Japan) */
	DRIVER( cworld2j )	/* 11/06/1992 (QUIZ 5) (c) 1992 (Japan) */
	DRIVER( wof )		/* 02/10/1992 (c) 1992 (World) (CPS1 + QSound) */
	DRIVER( wofa )		/* 05/10/1992 (c) 1992 (Asia)  (CPS1 + QSound) */
	DRIVER( wofu )		/* 31/10/1992 (c) 1992 (US) (CPS1 + QSound) */
	DRIVER( wofj )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
	DRIVER( sf2hf )		/* 09/12/1992 (c) 1992 (World) */
	DRIVER( sf2t )		/* 09/12/1992 (c) 1992 (US)    */
	DRIVER( sf2tj )		/* 09/12/1992 (c) 1992 (Japan) */
	DRIVER( dino )		/* 01/02/1993 (c) 1993 (World) (CPS1 + QSound) */
	DRIVER( dinou )		/* 01/02/1993 (c) 1993 (US)    (CPS1 + QSound) */
	DRIVER( dinoj )		/* 01/02/1993 (c) 1993 (Japan) (CPS1 + QSound) */
	DRIVER( punisher )	/* 22/04/1993 (c) 1993 (World) (CPS1 + QSound) */
	DRIVER( punishru )	/* 22/04/1993 (c) 1993 (US)    (CPS1 + QSound) */
	DRIVER( punishrj )	/* 22/04/1993 (c) 1993 (Japan) (CPS1 + QSound) */
	DRIVER( slammast )	/* 13/07/1993 (c) 1993 (World) (CPS1 + QSound) */
	DRIVER( slammasu )	/* 13/07/1993 (c) 1993 (US)    (CPS1 + QSound) */
	DRIVER( mbomberj )	/* 13/07/1993 (c) 1993 (Japan) (CPS1 + QSound) */
	DRIVER( mbombrd )	/* 06/12/1993 (c) 1993 (World) (CPS1 + QSound) */
	DRIVER( mbombrdj )	/* 06/12/1993 (c) 1993 (Japan) (CPS1 + QSound) */
	DRIVER( pnickj )	/* 08/06/1994 (c) 1994 Compile + Capcom license (Japan) not listed on Capcom's site */
	DRIVER( qad )		/* 01/07/1992 (c) 1992 (US)    */
	DRIVER( qadj )		/* 21/09/1994 (c) 1994 (Japan) */
	DRIVER( qtono2 )	/* 23/01/1995 (c) 1995 (Japan) */
	DRIVER( pang3 )		/* 11/05/1995 (c) 1995 Mitchell (Euro) not listed on Capcom's site */
	DRIVER( pang3j )	/* 11/05/1995 (c) 1995 Mitchell (Japan) not listed on Capcom's site */
	DRIVER( megaman )	/* 06/10/1995 (c) 1995 (Asia)  */
	DRIVER( rockmanj )	/* 22/09/1995 (c) 1995 (Japan) */

	/* Capcom CPS2 games */
	/* list completed by CPS2Shock */
	/* http://cps2shock.retrogames.com */
	DRIVER( ssf2 )		/* 11/09/1993 (c) 1993 (World) */
	DRIVER( ssf2u )		/* 11/09/1993 (c) 1993 (US) */
	DRIVER( ssf2a )		/* 05/10/1993 (c) 1993 (Asia) */
	DRIVER( ssf2ar1 )	/* 14/09/1993 (c) 1993 (Asia) */
	DRIVER( ssf2j )		/* 05/10/1993 (c) 1993 (Japan) */
	DRIVER( ssf2jr1 )	/* 11/09/1993 (c) 1993 (Japan) */
	DRIVER( ssf2jr2 )	/* 10/09/1993 (c) 1993 (Japan) */
	DRIVER( ssf2tb )	/* 11/19/1993 (c) 1993 (World) */
	DRIVER( ssf2tbj )	/* 10/09/1993 (c) 1993 (Japan) */
	DRIVER( ecofghtr )	/* 03/12/1993 (c) 1993 (World) */
	DRIVER( ecofghta )	/* 03/12/1993 (c) 1993 (Asia) */
	DRIVER( uecology ) 	/* 03/12/1993 (c) 1993 (Japan) */
	DRIVER( ddtod )		/* 12/04/1994 (c) 1993 (Euro) */
	DRIVER( ddtodu )	/* 25/01/1994 (c) 1993 (US) */
	DRIVER( ddtodur1 )	/* 13/01/1994 (c) 1993 (US) */
	DRIVER( ddtodj )	/* 25/01/1994 (c) 1993 (Japan) */
	DRIVER( ddtodjr1 )	/* 13/01/1994 (c) 1993 (Japan) */
	DRIVER( ddtoda )	/* 13/01/1994 (c) 1993 (Asia) */
	DRIVER( ddtodh )	/* 25/01/1994 (c) 1993 (Hispanic) */
	DRIVER( ssf2t )		/* 23/02/1994 (c) 1994 (World) */
	DRIVER( ssf2tu )	/* 23/03/1994 (c) 1994 (US) */
	DRIVER( ssf2tur1 )	/* 23/02/1994 (c) 1994 (US) */
	DRIVER( ssf2ta )	/* 23/02/1994 (c) 1994 (Asia) */
	DRIVER( ssf2xj )	/* 23/02/1994 (c) 1994 (Japan) */
	DRIVER( avsp )		/* 20/05/1994 (c) 1994 (Euro) */
	DRIVER( avspu )		/* 20/05/1994 (c) 1994 (US) */
	DRIVER( avspj )		/* 20/05/1994 (c) 1994 (Japan) */
	DRIVER( avspa )		/* 20/05/1994 (c) 1994 (Asia) */
						/*    06/1994? Galum Pa! (not listed on Capcom's site) */
	DRIVER( dstlk )		/* 05/07/1994 (c) 1994 (Euro) */
	DRIVER( dstlku )	/* 18/08/1994 (c) 1994 (US) */
	DRIVER( dstlkur1 )	/* 05/07/1994 (c) 1994 (US) */
	DRIVER( dstlka )	/* 05/07/1994 (c) 1994 (Asia) */
	DRIVER( vampj )		/* 05/07/1994 (c) 1994 (Japan) */
	DRIVER( vampja )	/* 05/07/1994 (c) 1994 (Japan) */
	DRIVER( vampjr1 )	/* 30/06/1994 (c) 1994 (Japan) */
	DRIVER( ringdest )	/* 02/09/1994 (c) 1994 (Euro) */
	DRIVER( smbomb )	/* 31/08/1994 (c) 1994 (Japan) */
	DRIVER( smbombr1 )	/* 08/08/1994 (c) 1994 (Japan) */
	DRIVER( armwar )	/* 24/10/1994 (c) 1994 (Euro) */
	DRIVER( armwarr1 )	/* 11/10/1994 (c) 1994 (Euro) */
	DRIVER( armwaru )	/* 24/10/1994 (c) 1994 (US) */
	DRIVER( pgear )		/* 24/10/1994 (c) 1994 (Japan) */
	DRIVER( pgearr1 )	/* 16/09/1994 (c) 1994 (Japan) */
	DRIVER( armwara )	/* 20/09/1994 (c) 1994 (Asia) */
	DRIVER( xmcota )	/* 05/01/1995 (c) 1994 (Euro) */
	DRIVER( xmcotau )	/* 05/01/1995 (c) 1994 (US) */
	DRIVER( xmcotah )	/* 31/03/1995 (c) 1994 (Hispanic) */
	DRIVER( xmcotaj )	/* 19/12/1994 (c) 1994 (Japan) */
	DRIVER( xmcotaj1 )	/* 17/12/1994 (c) 1994 (Japan) */
	DRIVER( xmcotaa )	/* 17/12/1994 (c) 1994 (Asia) */
	DRIVER( xmcotajr )	/* 08/12/1994 (c) 1994 (Japan Rent) */
	DRIVER( nwarr )		/* 06/04/1995 (c) 1995 (US) */
	DRIVER( nwarrh )	/* 03/04/1995 (c) 1995 (Hispanic) */
	DRIVER( nwarrb )	/* 03/04/1995 (c) 1995 (Brazil) */
	DRIVER( vhuntj )	/* 02/03/1995 (c) 1995 (Japan) */
	DRIVER( vhuntjr1 )	/* 07/03/1995 (c) 1995 (Japan) */
	DRIVER( vhuntjr2 )	/* 02/03/1995 (c) 1995 (Japan) */
	DRIVER( cybots )	/* 24/04/1995 (c) 1995 (Euro) */
	DRIVER( cybotsu )	/* 24/04/1995 (c) 1995 (US) */
	DRIVER( cybotsj )	/* 20/04/1995 (c) 1995 (Japan) */
	DRIVER( sfa )		/* 27/07/1995 (c) 1995 (Euro) */
	DRIVER( sfar1 )		/* 18/07/1995 (c) 1995 (Euro) */
	DRIVER( sfar2 )		/* 27/06/1995 (c) 1995 (Euro) */
	DRIVER( sfar3 )		/* 05/06/1995 (c) 1995 (Euro) */
	DRIVER( sfau )		/* 27/06/1995 (c) 1995 (US) */
	DRIVER( sfzj )		/* 27/07/1995 (c) 1995 (Japan) */
	DRIVER( sfzjr1 )	/* 27/06/1995 (c) 1995 (Japan) */
	DRIVER( sfzjr2 )	/* 05/06/1995 (c) 1995 (Japan) */
	DRIVER( sfzh )		/* 27/06/1995 (c) 1995 (Hispanic) */
	DRIVER( sfzb )		/* 09/11/1995 (c) 1995 (Brazil) */
	DRIVER( sfzbr1 )	/* 27/07/1995 (c) 1995 (Brazil) */
	DRIVER( rckmanj )	/* 22/09/1995 Rockman: The Power Battle (Japan) */
	DRIVER( msh )		/* 24/10/1995 (c) 1995 (Euro) */
	DRIVER( mshu )		/* 24/10/1995 (c) 1995 (US) */
	DRIVER( mshj )		/* 17/11/1995 (c) 1995 (Japan) */
	DRIVER( mshjr1 )	/* 24/10/1995 (c) 1995 (Japan) */
	DRIVER( msha )		/* 24/10/1995 (c) 1995 (Asia) */
	DRIVER( mshh )		/* 17/11/1995 (c) 1996 (Hispanic) */
	DRIVER( mshb )		/* 17/11/1995 (c) 1996 (Brazil) */
	DRIVER( 19xx )		/* 07/12/1995 (c) 1996 (US) */
	DRIVER( 19xxj )		/* 25/12/1995 (c) 1996 (Japan) */
	DRIVER( 19xxjr1 )	/* 07/12/1995 (c) 1996 (Japan) */
	DRIVER( 19xxa )		/* 07/12/1995 (c) 1996 (Asia) */
	DRIVER( 19xxh )		/* 18/12/1995 (c) 1996 (Hispanic) */
	DRIVER( ddsom )		/* 19/06/1996 (c) 1996 (Euro) */
	DRIVER( ddsomr1 )	/* 09/02/1996 (c) 1996 (Euro) */
	DRIVER( ddsomu )	/* 19/06/1996 (c) 1996 (US) */
	DRIVER( ddsomur1 )	/* 09/02/1996 (c) 1996 (US) */
	DRIVER( ddsomjr1 )	/* 06/02/1996 (c) 1996 (Japan) */
	DRIVER( ddsomj )	/* 19/06/1996 (c) 1996 (Japan) */
	DRIVER( ddsoma )	/* 19/06/1996 (c) 1996 (Asia) */
	DRIVER( sfa2 )		/* 06/03/1996 (c) 1996 (US) */
	DRIVER( sfz2j )		/* 27/02/1996 (c) 1996 (Japan) */
	DRIVER( sfz2a )		/* 27/02/1996 (c) 1996 (Asia) */
	DRIVER( sfz2b )		/* 31/05/1996 (c) 1996 (Brazil) */
	DRIVER( sfz2br1 )	/* 04/03/1996 (c) 1996 (Brazil) */
	DRIVER( sfz2h )		/* 04/03/1996 (c) 1996 (Hispanic) */
	DRIVER( sfz2n )		/* 29/02/1996 (c) 1996 (Oceania) */
	DRIVER( spf2t )		/* 20/06/1996 (c) 1996 (US) */
	DRIVER( spf2xj )	/* 31/05/1996 (c) 1996 (Japan) */
	DRIVER( spf2ta )	/* 29/06/1996 (c) 1996 (Asia) */
	DRIVER( qndream )	/* 26/06/1996 (c) 1996 (Japan) */
	DRIVER( megaman2 )	/* 08/07/1996 (c) 1996 (US) */
	DRIVER( megamn2a )	/* 08/07/1996 (c) 1996 (Asia) */
	DRIVER( rckman2j )	/* 08/07/1996 (c) 1996 (Japan) */
	DRIVER( sfz2aj )	/* 05/08/1996 (c) 1996 (Japan) */
	DRIVER( sfz2ah )	/* 13/08/1996 (c) 1996 (Hispanic) */
	DRIVER( sfz2ab )	/* 13/08/1996 (c) 1996 (Brazil) */
	DRIVER( sfz2aa )	/* 26/08/1996 (c) 1996 (Asia) */
	DRIVER( xmvsf )		/* 10/09/1996 (c) 1996 (Euro) */
	DRIVER( xmvsfu )	/* 23/10/1996 (c) 1996 (US) */
	DRIVER( xmvsfur1 )	/* 04/10/1996 (c) 1996 (US) */
	DRIVER( xmvsfj )	/* 04/10/1996 (c) 1996 (Japan) */
	DRIVER( xmvsfjr1 )	/* 10/09/1996 (c) 1996 (Japan) */
	DRIVER( xmvsfjr2 )	/* 09/09/1996 (c) 1996 (Japan) */
	DRIVER( xmvsfa )	/* 23/10/1996 (c) 1996 (Asia) */
	DRIVER( xmvsfh )	/* 04/10/1996 (c) 1996 (Hispanic) */
	DRIVER( xmvsfb )	/* 23/10/1996 (c) 1996 (Brazil) */
	DRIVER( batcir )	/* 19/03/1997 (c) 1997 (Euro) */
	DRIVER( batcirj )	/* 19/03/1997 (c) 1997 (Japan) */
	DRIVER( batcira )	/* 19/03/1997 (c) 1997 (Asia) */
	DRIVER( vsav )		/* 19/05/1997 (c) 1997 (Euro) */
	DRIVER( vsavu )		/* 19/05/1997 (c) 1997 (US) */
	DRIVER( vsavj )		/* 19/05/1997 (c) 1997 (Japan) */
	DRIVER( vsava )		/* 19/05/1997 (c) 1997 (Asia) */
	DRIVER( vsavh )		/* 19/05/1997 (c) 1997 (Hispanic) */
	DRIVER( mshvsf )	/* 27/08/1997 (c) 1997 (US) */
	DRIVER( mshvsfu1 )	/* 25/06/1997 (c) 1997 (US) */
	DRIVER( mshvsfj )	/* 07/07/1997 (c) 1997 (Japan) */
	DRIVER( mshvsfj1 )	/* 02/07/1997 (c) 1997 (Japan) */
	DRIVER( mshvsfj2 )	/* 25/06/1997 (c) 1997 (Japan) */
	DRIVER( mshvsfh )	/* 25/06/1997 (c) 1997 (Hispanic) */
	DRIVER( mshvsfa )	/* 25/06/1997 (c) 1997 (Asia) */
	DRIVER( mshvsfa1 )	/* 20/06/1997 (c) 1997 (Asia) */
	DRIVER( mshvsfb )	/* 27/08/1997 (c) 1997 (Brazil) */
	DRIVER( mshvsfb1 )	/* 25/06/1997 (c) 1997 (Brazil) */
	DRIVER( csclub )	/* 22/07/1997 (c) 1997 (Euro) */
	DRIVER( csclubj )	/* 22/07/1997 (c) 1997 (Japan) */
	DRIVER( cscluba )	/* 22/07/1997 (c) 1997 (Asia) */
	DRIVER( csclubh )	/* 22/07/1997 (c) 1997 (Hispanic) */
	DRIVER( sgemf )		/* 04/09/1997 (c) 1997 (US) */
	DRIVER( pfghtj )	/* 04/09/1997 (c) 1997 (Japan) */
	DRIVER( sgemfa )	/* 04/09/1997 (c) 1997 (Asia) */
	DRIVER( sgemfh )	/* 04/09/1997 (c) 1997 (Hispanic) */
	DRIVER( vhunt2r1 )	/* 13/09/1997 (c) 1997 (Japan) */
	DRIVER( vhunt2 )	/* 29/09/1997 (c) 1997 (Japan) */
	DRIVER( vsav2 )		/* 13/09/1997 (c) 1997 (Japan) */
	DRIVER( mvsc )		/* 23/01/1998 (c) 1998 (US) */
	DRIVER( mvscj )		/* 23/01/1998 (c) 1998 (Japan) */
	DRIVER( mvscjr1 )	/* 12/01/1998 (c) 1998 (Japan) */
	DRIVER( mvsca )		/* 23/01/1998 (c) 1998 (Asia) */
	DRIVER( mvscar1 )	/* 12/01/1998 (c) 1998 (Asia) */
	DRIVER( mvsch )		/* 23/01/1998 (c) 1998 (Hispanic) */
	DRIVER( mvscb )		/* 23/01/1998 (c) 1998 (Brazil) */
	DRIVER( sfa3 )		/* 04/09/1998 (c) 1998 (US) */
	DRIVER( sfa3r1 )	/* 29/06/1998 (c) 1998 (US) */
	DRIVER( sfa3b )		/* 29/06/1998 (c) 1998 (Brazil) */
	DRIVER( sfz3j )		/* 04/09/1998 (c) 1998 (Japan) */
	DRIVER( sfz3jr1 )	/* 27/07/1998 (c) 1998 (Japan) */
	DRIVER( sfz3jr2 )	/* 29/06/1998 (c) 1998 (Japan) */
	DRIVER( sfz3a )		/* 01/07/1998 (c) 1998 (Asia) */
	DRIVER( gigawing )	/* 22/02/1999 (c) 1999 Takumi (US) */
	DRIVER( gwingj )	/* 23/02/1999 (c) 1999 Takumi (Japan) */
						/* 27/05/1999 Jyangokushi: Haoh no Saihai (Japan) */
	DRIVER( dimahoo )	/* 21/01/2000 (c) 2000 Eighting/Raizing (US) */
	DRIVER( gmahou )	/* 21/01/2000 (c) 2000 Eighting/Raizing (Japan) */
	DRIVER( mmatrix )	/* 12/04/2000 (c) 2000 Takumi (US) */
	DRIVER( mmatrixj )	/* 12/04/2000 (c) 2000 Takumi (Japan) */
	DRIVER( 1944 )		/* 20/06/2000 (c) 2000 Eighting/Raizing (US) */
	DRIVER( 1944j )		/* 20/06/2000 (c) 2000 Eighting/Raizing (Japan) */
	DRIVER( mpangj )	/* 11/10/2000 (c) 2000 Mitchell */
	DRIVER( progear )	/* 17/01/2001 (c) 2001 Cave (US) */
	DRIVER( progearj )	/* 17/01/2001 (c) 2001 Cave (Japan) */
	DRIVER( progeara )	/* 17/01/2001 (c) 2001 Cave (Asia) */
	DRIVER( pzloop2j )	/* 05/02/2001 (c) 2001 (Mitchell) */
	DRIVER( choko )		/* 20/08/2001 (c) 2001 (Mitchell) */
	
	/* Neo Geo games */
	/* the four digits number is the game ID stored at address 0x0108 of the program ROM */
	/* info on prototypes taken from http://www.members.tripod.com/fresa/proto/puzzle.htm */
	DRIVER( nam1975 )	/* 0001 (c) 1990 SNK */
	DRIVER( bstars )	/* 0002 (c) 1990 SNK */
	DRIVER( tpgolf )	/* 0003 (c) 1990 SNK */
	DRIVER( mahretsu )	/* 0004 (c) 1990 SNK */
	DRIVER( maglord )	/* 0005 (c) 1990 Alpha Denshi Co. */
	DRIVER( maglordh )	/* 0005 (c) 1990 Alpha Denshi Co. */
	DRIVER( ridhero )	/* 0006 (c) 1990 SNK */
	DRIVER( ridheroh )	/* 0006 (c) 1990 SNK */
	DRIVER( alpham2 )	/* 0007 (c) 1991 SNK */
	/* 0008 Sunshine (prototype) 1990 SNK */
	DRIVER( ncombat )	/* 0009 (c) 1990 Alpha Denshi Co. */
	DRIVER( ncombata )	/* 0009 (c) 1990 Alpha Denshi Co. */
	DRIVER( cyberlip )	/* 0010 (c) 1990 SNK */
	DRIVER( superspy )	/* 0011 (c) 1990 SNK */
	/* 0012 */
	/* 0013 */
	DRIVER( mutnat )	/* 0014 (c) 1992 SNK */
	/* 0015 */
	DRIVER( kotm )		/* 0016 (c) 1991 SNK */
	DRIVER( kotmh )		/* 0016 (c) 1991 SNK */
	DRIVER( sengoku )	/* 0017 (c) 1991 SNK */
	DRIVER( sengokh )	/* 0017 (c) 1991 SNK */
	DRIVER( burningf )	/* 0018 (c) 1991 SNK */
	DRIVER( burningh )	/* 0018 (c) 1991 SNK */
	DRIVER( lbowling )	/* 0019 (c) 1990 SNK */
	DRIVER( gpilots )	/* 0020 (c) 1991 SNK */
	DRIVER( joyjoy )	/* 0021 (c) 1990 SNK */
	DRIVER( bjourney )	/* 0022 (c) 1990 Alpha Denshi Co. */
	DRIVER( quizdais )	/* 0023 (c) 1991 SNK */
	DRIVER( lresort )	/* 0024 (c) 1992 SNK */
	DRIVER( eightman )	/* 0025 (c) 1991 SNK / Pallas */
	/* 0026 Fun Fun Brothers (prototype) 1991 Alpha */
	DRIVER( minasan )	/* 0027 (c) 1990 Monolith Corp. */
	/* 0028 Dunk Star (prototype) Sammy */
	DRIVER( legendos )	/* 0029 (c) 1991 SNK */
	DRIVER( 2020bb )	/* 0030 (c) 1991 SNK / Pallas */
	DRIVER( 2020bba )	/* 0030 (c) 1991 SNK / Pallas */
	DRIVER( 2020bbh )	/* 0030 (c) 1991 SNK / Pallas */
	DRIVER( socbrawl )	/* 0031 (c) 1991 SNK */
	DRIVER( roboarmy )	/* 0032 (c) 1991 SNK */
	DRIVER( roboarma )	/* 0032 (c) 1991 SNK */
	DRIVER( fatfury1 )	/* 0033 (c) 1991 SNK */
	DRIVER( fbfrenzy )	/* 0034 (c) 1992 SNK */
	/* 0035 Mystic Wand (prototype) 1991 Alpha */
	DRIVER( bakatono )	/* 0036 (c) 1991 Monolith Corp. */
	DRIVER( crsword )	/* 0037 (c) 1991 Alpha Denshi Co. */
	DRIVER( trally )	/* 0038 (c) 1991 Alpha Denshi Co. */
	DRIVER( kotm2 )		/* 0039 (c) 1992 SNK */
	DRIVER( sengoku2 )	/* 0040 (c) 1993 SNK */
	DRIVER( bstars2 )	/* 0041 (c) 1992 SNK */
	DRIVER( quizdai2 )	/* 0042 (c) 1992 SNK */
	DRIVER( 3countb )	/* 0043 (c) 1993 SNK */
	DRIVER( aof )		/* 0044 (c) 1992 SNK */
	DRIVER( samsho )	/* 0045 (c) 1993 SNK */
	DRIVER( tophuntr )	/* 0046 (c) 1994 SNK */
	DRIVER( tophunta )	/* 0046 (c) 1994 SNK */
	DRIVER( fatfury2 )	/* 0047 (c) 1992 SNK */
	DRIVER( janshin )	/* 0048 (c) 1994 Aicom */
	DRIVER( androdun )	/* 0049 (c) 1992 Visco */
	DRIVER( ncommand )	/* 0050 (c) 1992 Alpha Denshi Co. */
	DRIVER( viewpoin )	/* 0051 (c) 1992 Sammy */
	DRIVER( ssideki )	/* 0052 (c) 1992 SNK */
	DRIVER( wh1 )		/* 0053 (c) 1992 Alpha Denshi Co. */
	DRIVER( wh1h )		/* 0053 (c) 1992 Alpha Denshi Co. */
	/* 0054 Crossed Swords 2  (CD only? not confirmed, MVS might exist) */
	DRIVER( kof94 )		/* 0055 (c) 1994 SNK */
	DRIVER( aof2 )		/* 0056 (c) 1994 SNK */
	DRIVER( aof2a )		/* 0056 (c) 1994 SNK */
	DRIVER( wh2 )		/* 0057 (c) 1993 ADK */
	DRIVER( fatfursp )	/* 0058 (c) 1993 SNK */
	DRIVER( fatfursa )	/* 0058 (c) 1993 SNK */
	DRIVER( savagere )	/* 0059 (c) 1995 SNK */
	DRIVER( fightfev )	/* 0060 (c) 1994 Viccom */
	DRIVER( fightfva )	/* 0060 (c) 1994 Viccom */
	DRIVER( ssideki2 )	/* 0061 (c) 1994 SNK */
	DRIVER( spinmast )	/* 0062 (c) 1993 Data East Corporation */
	DRIVER( samsho2 )	/* 0063 (c) 1994 SNK */
	DRIVER( wh2j )		/* 0064 (c) 1994 ADK / SNK */
	DRIVER( wjammers )	/* 0065 (c) 1994 Data East Corporation */
	DRIVER( karnovr )	/* 0066 (c) 1994 Data East Corporation */
	DRIVER( gururin )	/* 0067 (c) 1994 Face */
	DRIVER( pspikes2 )	/* 0068 (c) 1994 Video System Co. */
	DRIVER( fatfury3 )	/* 0069 (c) 1995 SNK */
	DRIVER( zupapa )	/* 0070 Zupapa - released in 2001, 1994 prototype probably exists */
	/* 0071 Bang Bang Busters (prototype) 1994 Visco */
	/* 0072 Last Odyssey Pinball Fantasia (prototype) 1995 Monolith */
	DRIVER( panicbom )	/* 0073 (c) 1994 Eighting / Hudson */
	DRIVER( aodk )		/* 0074 (c) 1994 ADK / SNK */
	DRIVER( sonicwi2 )	/* 0075 (c) 1994 Video System Co. */
	DRIVER( zedblade )	/* 0076 (c) 1994 NMK */
	/* 0077 The Warlocks of the Fates (prototype) 1995 Astec */
	DRIVER( galaxyfg )	/* 0078 (c) 1995 Sunsoft */
	DRIVER( strhoop )	/* 0079 (c) 1994 Data East Corporation */
	DRIVER( quizkof )	/* 0080 (c) 1995 Saurus */
	DRIVER( ssideki3 )	/* 0081 (c) 1995 SNK */
	DRIVER( doubledr )	/* 0082 (c) 1995 Technos */
	DRIVER( pbobblen )	/* 0083 (c) 1994 Taito */
	DRIVER( pbobblna )	/* 0083 (c) 1994 Taito */
	DRIVER( kof95 )		/* 0084 (c) 1995 SNK */
	DRIVER( kof95a )	/* 0084 (c) 1995 SNK */
	/* 0085 Shinsetsu Samurai Spirits Bushidoretsuden / Samurai Shodown RPG (CD only) */
	DRIVER( tws96 )		/* 0086 (c) 1996 Tecmo */
	DRIVER( samsho3 )	/* 0087 (c) 1995 SNK */
	DRIVER( samsho3a )	/* 0087 (c) 1995 SNK */
	DRIVER( stakwin )	/* 0088 (c) 1995 Saurus */
	DRIVER( pulstar )	/* 0089 (c) 1995 Aicom */
	DRIVER( whp )		/* 0090 (c) 1995 ADK / SNK */
	/* 0091 */
	DRIVER( kabukikl )	/* 0092 (c) 1995 Hudson */
	DRIVER( neobombe )	/* 0093 (c) 1997 Hudson */
	DRIVER( gowcaizr )	/* 0094 (c) 1995 Technos */
	DRIVER( rbff1 )		/* 0095 (c) 1995 SNK */
	DRIVER( aof3 )		/* 0096 (c) 1996 SNK */
	DRIVER( sonicwi3 )	/* 0097 (c) 1995 Video System Co. */
	/* 0098 Idol Mahjong - final romance 2 (CD only? not confirmed, MVS might exist) */
	/* 0099 Neo Pool Masters */
	DRIVER( turfmast )	/* 0200 (c) 1996 Nazca */
	DRIVER( mslug )		/* 0201 (c) 1996 Nazca */
	DRIVER( puzzledp )	/* 0202 (c) 1995 Taito (Visco license) */
	DRIVER( mosyougi )	/* 0203 (c) 1995 ADK / SNK */
	/* 0204 QP (prototype) */
	/* 0205 Neo-Geo CD Special (CD only) */
	DRIVER( marukodq )	/* 0206 (c) 1995 Takara */
	DRIVER( neomrdo )	/* 0207 (c) 1996 Visco */
	DRIVER( sdodgeb )	/* 0208 (c) 1996 Technos */
	DRIVER( goalx3 )	/* 0209 (c) 1995 Visco */
	/* 0210 Karate Ninja Sho (prototype) 1995 Yumekobo */
	/* 0211 Oshidashi Zintrick (CD only? not confirmed, MVS might exist) 1996 SNK/ADK */
	DRIVER( zintrckb )	/* 0211 hack - this is not a genuine MVS proto, its a bootleg made from the CD version */
	DRIVER( overtop )	/* 0212 (c) 1996 ADK */
	DRIVER( neodrift )	/* 0213 (c) 1996 Visco */
	DRIVER( kof96 )		/* 0214 (c) 1996 SNK */
	DRIVER( kof96h )	/* 0214 (c) 1996 SNK */
	DRIVER( ssideki4 )	/* 0215 (c) 1996 SNK */
	DRIVER( kizuna )	/* 0216 (c) 1996 SNK */
	DRIVER( ninjamas )	/* 0217 (c) 1996 ADK / SNK */
	DRIVER( ragnagrd )	/* 0218 (c) 1996 Saurus */
	DRIVER( pgoal )		/* 0219 (c) 1996 Saurus */
	/* 0220 Choutetsu Brikin'ger - iron clad (MVS existance seems to have been confirmed) */
	DRIVER( magdrop2 )	/* 0221 (c) 1996 Data East Corporation */
	DRIVER( samsho4 )	/* 0222 (c) 1996 SNK */
	DRIVER( rbffspec )	/* 0223 (c) 1996 SNK */
	DRIVER( twinspri )	/* 0224 (c) 1996 ADK */
	DRIVER( wakuwak7 )	/* 0225 (c) 1996 Sunsoft */
	/* 0226 Pair Pair Wars (prototype) 1996 Sunsoft? */
	DRIVER( stakwin2 )	/* 0227 (c) 1996 Saurus */
	DRIVER( ghostlop )	/* 0228 GhostLop (prototype) 1996? Data East */
	/* 0229 King of Fighters '96 CD Collection (CD only) */
	DRIVER( breakers )	/* 0230 (c) 1996 Visco */
	DRIVER( miexchng )	/* 0231 (c) 1997 Face */
	DRIVER( kof97 )		/* 0232 (c) 1997 SNK */
	DRIVER( kof97a )	/* 0232 (c) 1997 SNK */
	DRIVER( kof97pls )	/* 0232 bootleg */
	DRIVER( magdrop3 )	/* 0233 (c) 1997 Data East Corporation */
	DRIVER( lastblad )	/* 0234 (c) 1997 SNK */
	DRIVER( lastblda )	/* 0234 (c) 1997 SNK */
	DRIVER( puzzldpr )	/* 0235 (c) 1997 Taito (Visco license) */
	DRIVER( irrmaze )	/* 0236 (c) 1997 SNK / Saurus */
	DRIVER( popbounc )	/* 0237 (c) 1997 Video System Co. */
	DRIVER( shocktro )	/* 0238 (c) 1997 Saurus */
	DRIVER( shocktra )	/* 0238 (c) 1997 Saurus */
	DRIVER( blazstar )	/* 0239 (c) 1998 Yumekobo */
	DRIVER( rbff2 )		/* 0240 (c) 1998 SNK */
	DRIVER( rbff2a )	/* 0240 (c) 1998 SNK */
	DRIVER( mslug2 )	/* 0241 (c) 1998 SNK */
	DRIVER( kof98 )		/* 0242 (c) 1998 SNK */
	DRIVER( kof98k )	/* 0242 (c) 1998 SNK */
	DRIVER( kof98n )	/* 0242 (c) 1998 SNK */
	DRIVER( lastbld2 )	/* 0243 (c) 1998 SNK */
	DRIVER( neocup98 )	/* 0244 (c) 1998 SNK */
	DRIVER( breakrev )	/* 0245 (c) 1998 Visco */
	DRIVER( shocktr2 )	/* 0246 (c) 1998 Saurus */
	DRIVER( flipshot )	/* 0247 (c) 1998 Visco */
	DRIVER( pbobbl2n )	/* 0248 (c) 1999 Taito (SNK license) */
	DRIVER( ctomaday )	/* 0249 (c) 1999 Visco */
	DRIVER( mslugx )	/* 0250 (c) 1999 SNK */
	DRIVER( kof99 )		/* 0251 (c) 1999 SNK */
	DRIVER( kof99a )	/* 0251 (c) 1999 SNK */
	DRIVER( kof99e )	/* 0251 (c) 1999 SNK */
	DRIVER( kof99n )	/* 0251 (c) 1999 SNK */
	DRIVER( kof99p )	/* 0251 (c) 1999 SNK */
	DRIVER( ganryu )	/* 0252 (c) 1999 Visco */
	DRIVER( garou )		/* 0253 (c) 1999 SNK */
	DRIVER( garouo )	/* 0253 (c) 1999 SNK */
	DRIVER( garoup )	/* 0253 (c) 1999 SNK */
	DRIVER( s1945p )	/* 0254 (c) 1999 Psikyo */
	DRIVER( preisle2 )	/* 0255 (c) 1999 Yumekobo */
	DRIVER( mslug3 )	/* 0256 (c) 2000 SNK */
	DRIVER( mslug3n )	/* 0256 (c) 2000 SNK */
	DRIVER( kof2000 )	/* 0257 (c) 2000 SNK */
	DRIVER( kof2000n )	/* 0257 (c) 2000 SNK */
	/* 0258 SNK vs. Capcom? (prototype) */
	DRIVER( bangbead )	/* 0259 (c) 2000 Visco */
	DRIVER( nitd )		/* 0260 (c) 2000 Eleven / Gavaking */
	DRIVER( sengoku3 )	/* 0261 (c) 2001 SNK */
	DRIVER( kof2001 )	/* 0262 (c) 2001 Eolith / SNK */
	DRIVER( kof2001h )	/* 0262 (c) 2001 Eolith / SNK */
	DRIVER( mslug4 )	/* 0263 (c) 2002 Mega Enterprise */
	DRIVER( ms4plus )	/* 0263 bootleg */
	DRIVER( rotd )		/* 0264 (c) 2002 Evoga */
	DRIVER( kof2002 )	/* 0265 (c) 2002 Eolith / Playmore */
	DRIVER( matrim )	/* 0266 (c) 2002 Atlus */

	/* The BreezaSoft games don't have proper ID codes */
	DRIVER( jockeygp )
	DRIVER( vliner )
	DRIVER( vlinero )
	
// 네오지오 게임 추가분 : 비공식
	DRIVER( samshob )
	DRIVER( samshoeh )
	DRIVER( samsho2eh )	/* 0063 (c) 1994 SNK */
	DRIVER( samsho2b )	/* 0063 (c) 1994 SNK */
	DRIVER( rbff1ud)
	DRIVER( kof94b )
	DRIVER( kof94r )
	DRIVER( aof2b )
	DRIVER( aof3b )
	DRIVER( kof95b) 
	DRIVER( kof95r)
	DRIVER( fatfury3b )	/* 0069 (c) 1995 SNK */
	DRIVER( fatfurspb )	/* 0058 (c) 1993 SNK */
	DRIVER( doubledrb)
	DRIVER( rbff2h )
  DRIVER( samsho3ehc )	
	DRIVER( whpbo2 )	
	DRIVER( kabukikleh )
	DRIVER( samsho4b2 )
	DRIVER( samsho4bh )
	DRIVER( samsho4eh )
	DRIVER( samsho4e2 )
	DRIVER( samsho4c )
	DRIVER( rbffspb)
	DRIVER( kof98nd )
	DRIVER( kof97bh )
	DRIVER( kof97bs )
	DRIVER( lastbldb )
	DRIVER( lastbldeh2 )
	DRIVER( kof98bi )
	DRIVER( kof98ice )
	DRIVER( kof98rh )
	DRIVER( kof98sh )
	DRIVER( lastbld2wh )
	DRIVER( lastbld2eh )
	DRIVER( lastbld2h )
	DRIVER( shocktr2h )
	DRIVER( kof99d )
	DRIVER( kof99rmp )
	DRIVER( kof99rp107 )
	DRIVER( kof99pjcr3 )
	DRIVER( kof99bh )
//	DRIVER( kof99nd )	/* 0251 (c) 1999 SNK */
	DRIVER( garoun )
//	DRIVER( garoupb )
//	DRIVER( garoud )	/* same as garoun 0253 (c) 1999 SNK */
	DRIVER( ganryun )	/* 0252 (c) 1999 Visco */
	DRIVER( s1945pwe )
	DRIVER( s1945pn )	/* 0254 (c) 1999 Psikyo */
//	DRIVER( s1945pd )	/* 0254 (c) 1999 Psikyo */
	DRIVER( mslug3d )      /* 네오지오 롬셋*/
	DRIVER( mslug3_pnc )
	DRIVER( mslug3_ph )
	DRIVER( mslug3_ph1 )
	DRIVER( mslug3_ph2 )
	DRIVER( mslug3_ph3 )
	DRIVER( mslug3eh01 )
	DRIVER( mslug3eh02 )
	DRIVER( mslug3eh03 )
	DRIVER( mslug3nd )	/* 0256 (c) 2000 SNK */
	DRIVER( kof2kb )
	DRIVER( kof2k )	/* 0257 (c) 2000 SNK */
	DRIVER( kof2kevo )
	DRIVER( kof2kev )
	DRIVER( kof2krp )
	DRIVER( kof2kpjc )
	DRIVER( kof2000d )	/* 0257 (c) 2000 SNK */
	DRIVER( nitdn )	/* 0260 (c) 2000 Eleven / Gavaking */
	DRIVER( sengoku3d )	/* 0261 (c) 2001 SNK */
	DRIVER( sengoku3eh )
	DRIVER( sengoku3st )
	DRIVER( sengok3d )	/* 0261 (c) 2001 SNK */
	DRIVER( kof2k1 )
	DRIVER( kof2k1pls )
	DRIVER( kof2k1rpf )
	DRIVER( kof2k1rp )
	DRIVER( kof2k1ru )
	DRIVER( kof2k1ru2 )
	DRIVER( kof2k1al )
	DRIVER( kof2k1cfc )
	DRIVER( kof2k1pjc3 )
	DRIVER( kof2k1k3 )
//	DRIVER( kof2001 )
//	DRIVER( kof2001d )
	
	DRIVER( sonicwi3h )
	DRIVER( neopong )	/* 0xxx (c) 2002 Neodev */
	DRIVER( neonopon )	/* 0xxx (c) 2002 Blastar */
	DRIVER( diggerma )	/* 0xxx (c) 2002 Neodev */
	DRIVER( cthd2k3 )	/* 0261 (c) 2001 Eolith / SNK */
	DRIVER( cthd2k3a )	/* 0261 (c) 2001 Eolith / SNK */
	DRIVER( cthd2003 )	/* 0261 (c) 2001 Eolith / SNK */
	DRIVER( cthd2k3c )	/* 0261 (c) 2001 Eolith / SNK */
	DRIVER( cthd2k3sp )	/* 0261 (c) 2001 Eolith / SNK */
	DRIVER( cthd2003sp )	/* 0261 (c) 2001 Eolith / SNK */
	DRIVER( kof96evo )      // 1996  The King of Fighters '96 Evolution (hack / bootleg)
	DRIVER( kof96eh )      // 1996  The King of Fighters '96 Evolution (hack / bootleg)
	DRIVER( kof96o )
	DRIVER( kof96chj )
	DRIVER( pbobblnb )      // 1994  Puzzle Bobble / Bust-A-Move (Neo-Geo) (set 3)
	DRIVER( fr2cd )         // 1995  Idol-Mahjong Final Romance 2 (CD bootleg) [X]
	DRIVER( jckeygpd )      // 2001  Jockey Grand Prix (decrypted C)
	
	DRIVER( mslug4kh )
	DRIVER( mslug4eh )
//	DRIVER( mslug4)		/* 0261 (c) 2001 Mega Enterprise / Playmore */
	DRIVER( ms4ndplus )
	DRIVER( mslug4eh01 )	/* 0256 (c) 2000 SNK */
	DRIVER( mslug4eh02 )	/* 0256 (c) 2000 SNK */
	DRIVER( mslug4eh03 )	/* 0256 (c) 2000 SNK */
  DRIVER( mslug4nd )
  
 	DRIVER( kof2k2 )
 	DRIVER( kof2k2ru )
	DRIVER( kof2k2pls )
	DRIVER( kof2k2pls2 )
	DRIVER( kof2k2k3 )
	DRIVER( kof2k2b )
	DRIVER( kof2k2bs )
	DRIVER( kof2k2eh )
	DRIVER( kof2k2ice )
	DRIVER( kof2k2gc )
	DRIVER( kof2k2sp )
	DRIVER( kof2k2cfc )
//	DRIVER( kof2002 )
	DRIVER( kof2002d )
	DRIVER( kof2k2nd )
	
	DRIVER( svc )	/* 0261 (c) 2001 Eolith / SNK */
	DRIVER( svcsplus )
	DRIVER( svcndh )
	DRIVER( svcjoy )
	DRIVER( svcice )
	DRIVER( svclspc )
	DRIVER( svcru )
	DRIVER( svcjs )
	DRIVER( svcchaos )
	DRIVER( svchaosa )
	DRIVER( svcboot )
	DRIVER( svcplus )
	DRIVER( svcplusa )
	DRIVER( svcsplusb )
	
	DRIVER( samsho5)
	DRIVER( samsho5d)
	DRIVER( samsho5h)
	DRIVER( samsho5hd)
	DRIVER( samsho5nd)
	DRIVER( samsho5e)
	DRIVER( samsho5e2)
	DRIVER( samsho5sh )
	DRIVER( samsho5a )
	DRIVER( samsho5c )
	DRIVER( samsho5cbh )
	DRIVER( samsh5bl)
	
	DRIVER( samsh5sp )
	DRIVER( samsh5spn )
	DRIVER( samsh5spu)
	DRIVER( samsh5spd)
	DRIVER( samsh5sph)
	DRIVER( samsh5spc )
	
	DRIVER( mslug5d )
	DRIVER( mslug5 )
	DRIVER( ms5plus )       // 2003  Metal Slug 5 Plus (hack / bootleg)

	DRIVER( kof2k3)
	DRIVER( kof2003p)
	DRIVER( kf2k3pls)
	DRIVER( kof2k3upd)
	DRIVER( kof2003 )
	DRIVER( kof2003b )
	DRIVER( kof2003a )
	DRIVER( kof2003h )
	DRIVER( kof2k3up)
	
	DRIVER( kof2k4se )
	DRIVER( kof2k4sed )
	
	DRIVER( kof10th )
	DRIVER( kof10thd )
	
	DRIVER( matrimd  )     /* 0266 (c) 2003 Noise Factory / Playmore Corporation / Atlus Corporation */
	DRIVER( matrimb )
	DRIVER( matrimeh )
	DRIVER( matrimh2 )
//	DRIVER( matrim )
	
//	DRIVER( rotd   )
	DRIVER( rotdn   )
	DRIVER( rotdbh )
	
	DRIVER( pnyaa )	
	DRIVER( pnyaad )	
//	DRIVER( zupapan )	/* 0070 (c) 2001 SNK */	
	DRIVER( mslugeh )
  DRIVER( mslug2eh )
  DRIVER( mslug2eh01 )
  DRIVER( mslugxeh )
  DRIVER( savagedb )
  DRIVER( karnovreh )	/* 0066 (c) 1994 Data East Corporation */
  DRIVER( kizunaeh )
  DRIVER( ragnagrdh)
// 추가 CPS 게임
	DRIVER( ssf2xjh )	/* 23/02/1994 (c) 1994 (Japan) */
	DRIVER( dinoh )
	DRIVER( sfzch )
	DRIVER( sfach )	
  DRIVER( knightsh )      // 1991  Knights of the Round (hack)
  DRIVER( sf2toryu )      // 1992  Street Fighter II' - Champion Edition (Turyu set 1)
  DRIVER( sf2tory2 )      // 1992  Street Fighter II' - Champion Edition (Turyu set 2)	
	DRIVER( hwofa )         // 1992  Sangokushi II (Asia)(Hack)
	DRIVER( pnicku )        // 1994  Pnickies (USA)
	DRIVER( wofjh )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
	DRIVER( wofjb )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
	DRIVER( wofjh2 )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
	DRIVER( wofjh3 )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
	DRIVER( wofjp )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
	DRIVER( wofjr )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
	DRIVER( wofjk )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
	DRIVER( wofjbg )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
  DRIVER( ssf2xjjs )
  DRIVER( mvsck )		/* 23/01/1998 (c) 1998 (US) */

#endif	/* DRIVER_RECURSIVE */

#endif	/* TINY_COMPILE */
