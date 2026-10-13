/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

***************************************************************************/
 
/***************************************************************************

  Properties.c

    Properties Popup and Misc UI support routines.
    
    Created 8/29/98 by Mike Haaland (mhaaland@hypertech.com)

***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define NONAMELESSUNION 1
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#undef NONAMELESSUNION
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "MAME32.h"	// include this first
#include "driver.h"
#include "info.h"
#include "rc.h"
#include "audit.h"
#include "audit32.h"
#include "bitmask.h"
#include "options.h"
#include "file.h"
#include "resource.h"
#include "DIJoystick.h"     /* For DIJoystick avalibility. */
#include "m32util.h"
#include "directdraw.h"
#include "properties.h"
#include "treeview.h"
#include "translate.h"

#include "screenshot.h"
#include "DataMap.h"
#include "help.h"
#include "resource.hm"
#include "datafile.h"

typedef HANDLE HTHEME;

#ifdef ROM_PATCH
#define PATCH_MAX 8
#ifdef UNICODE
#define TTM_SETTITLE            TTM_SETTITLEW
#else
#define TTM_SETTITLE            TTM_SETTITLEA
#endif
#endif /* ROM_PATCH */

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#ifdef MESS
// done like this until I figure out a better idea
#include "ui/resourcems.h"
#include "ui/propertiesms.h"
#endif

// missing win32 api defines
#ifndef TBCD_TICS
#define TBCD_TICS 1
#endif
#ifndef TBCD_THUMB
#define TBCD_THUMB 2
#endif
#ifndef TBCD_CHANNEL
#define TBCD_CHANNEL 3
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#define FOLDER_OPTIONS	-2
#define GLOBAL_OPTIONS	-1

#define IS_GLOBAL	(g_nGame == GLOBAL_OPTIONS)
#define IS_FOLDER	(g_pFolder)
#define IS_GAME		(g_nGame > -1)

#if defined(__GNUC__)
/* fix warning: cast does not match function type */
#undef  PropSheet_GetTabControl
#define PropSheet_GetTabControl(d) (HWND)(LRESULT)(int)SendMessage((d),PSM_GETTABCONTROL,0,0)
#endif /* defined(__GNUC__) */

#include "windows/windbg.h"

/***************************************************************
 * Imported function prototypes
 ***************************************************************/

/**************************************************************
 * Local function prototypes
 **************************************************************/

static int CALLBACK PropSheetCallbackProc(HWND hDlg, UINT Msg, LPARAM lParam);
static void SetStereoEnabled(HWND hWnd, int nIndex);
static void SetYM3812Enabled(HWND hWnd, int nIndex);
static void SetSamplesEnabled(HWND hWnd, int nIndex, BOOL bSoundEnabled);
static void InitializeOptions(HWND hDlg);
static void InitializeMisc(HWND hDlg);
static void OptOnHScroll(HWND hWnd, HWND hwndCtl, UINT code, int pos);
static void BeamSelectionChange(HWND hwnd);
static void FlickerSelectionChange(HWND hwnd);
static void GammaSelectionChange(HWND hwnd);
static void BrightCorrectSelectionChange(HWND hwnd);
static void PauseBrightSelectionChange(HWND hwnd);
static void BrightnessSelectionChange(HWND hwnd);
static void IntensitySelectionChange(HWND hwnd);
static void A2DSelectionChange(HWND hwnd);
static void ResDepthSelectionChange(HWND hWnd, HWND hWndCtrl);
static void RefreshSelectionChange(HWND hWnd, HWND hWndCtrl);
static void VolumeSelectionChange(HWND hwnd);
static void AudioLatencySelectionChange(HWND hwnd);
static void D3DScanlinesSelectionChange(HWND hwnd);
static void D3DFeedbackSelectionChange(HWND hwnd);
static void ZoomSelectionChange(HWND hwnd);
#ifdef USE_VIRTUAL_3D_VIEW
static void D3DRotationChange(HWND hwnd);
static void D3DZoomxChange(HWND hwnd);
static void D3DZoomyChange(HWND hwnd);
#endif /* USE_VIRTUAL_3D_VIEW */
static void UpdateDisplayModeUI(HWND hwnd, DWORD dwDepth, DWORD dwRefresh);
static void InitializeDisplayModeUI(HWND hwnd);
static void InitializeSoundUI(HWND hwnd);
static void InitializeSkippingUI(HWND hwnd);
static void InitializeRotateUI(HWND hwnd);
static void InitializeResDepthUI(HWND hwnd);
static void InitializeRefreshUI(HWND hwnd);
static void InitializeDefaultInputUI(HWND hWnd);
static void InitializeEffectUI(HWND hWnd);
static void InitializeArtresUI(HWND hWnd);
static void InitializeD3DFilterUI(HWND hwnd);
static void InitializeD3DEffectUI(HWND hwnd);
static void InitializeD3DPrescaleUI(HWND hwnd);
static void InitializeDefaultBIOSUI(HWND hwnd);
static void InitializeBIOSUI(HWND hwnd);
#ifdef ROM_PATCH
static void InitializeIPSUI(HWND hwnd);
#endif /* ROM_PATCH */
static void InitializeLEDModeUI(HWND hwnd);
static void InitializeCleanStretchUI(HWND hwnd);
static void InitializeM68kCoreUI(HWND hwnd);
#ifdef MENU_TRANS
static void GameOptionsPaint( HWND hDlg, WPARAM wParam, LPARAM lParam );
static void PickGameMenuColor(HWND hWnd, COLORREF *lpcrColor );
static void InitializePlusUI(HWND hwnd);
static void MenuTransSelectionChange(HWND hwnd);
#endif
#ifdef USE_SCALE_EFFECTS
static void InitializeScaleEffectUI(HWND hwnd);
#endif /* USE_SCALE_EFFECTS */
#ifdef JOYSTICK_ID
static void InitializeJoyidUI(HWND hWnd);
#endif /* JOYSTICK_ID */
#ifdef TRANS_UI
static void TransparencySelectionChange(HWND hwnd);
#endif /* TRANS_UI */
#ifdef USE_PSXPLUGIN
static void InitializePSXPlugInUI(HWND hwnd);
static BOOL IsPSXGPUConfigureAvail(HWND hwnd);
static void RunPSXGPUConfigure(HWND hwnd);
#endif /* USE_PSXPLUGIN */	

static void PropToOptions(HWND hWnd, options_type *o);
static void OptionsToProp(HWND hWnd, options_type *o);
static void SetPropEnabledControls(HWND hWnd);

static void BuildDataMap(void);
static void ResetDataMap(void);

static BOOL IsControlOptionValue(HWND hDlg,HWND hwnd_ctrl, options_type *opts);

static void UpdateBackgroundBrush(HWND hwndTab);
HBRUSH hBkBrush;
BOOL bThemeActive;

/**************************************************************
 * Local private variables
 **************************************************************/

static options_type  origGameOpts;
static BOOL orig_uses_defaults;
static options_type* pGameOpts = NULL;
static const struct SystemBios *g_biosinfo = NULL;
static int  default_bios_index[MAX_SYSTEM_BIOS];

static int  g_nGame            = 0;
static const char *g_pFolder   = NULL;
static int  g_nPropertyMode    = 0;
static BOOL g_bInternalSet     = FALSE;
static BOOL g_bUseDefaults     = FALSE;
static BOOL g_bReset           = FALSE;
static int  g_nSampleRateIndex = 0;
static int  g_nVolumeIndex     = 0;
static int  g_nGammaIndex      = 0;
static int  g_nBrightCorrectIndex = 0;
static int  g_nPauseBrightIndex = 0;
static int  g_nBeamIndex       = 0;
static int  g_nFlickerIndex    = 0;
static int  g_nIntensityIndex  = 0;
static int  g_nRotateIndex     = 0;
static int  g_nInputIndex      = 0;
static int  g_nBrightnessIndex = 0;
static int  g_nEffectIndex     = 0;
static int  g_nLedmodeIndex    = 0;
static int  g_nCleanStretchIndex = 0;
static int  g_nD3DEffectIndex  = 0;
static int  g_nD3DPrescaleIndex  = 0;
static int  g_nBiosIndex       = 0;
#ifdef ROM_PATCH
static int  g_nIpsIndex[PATCH_MAX];
static HWND hTooltipWnd = NULL;
#endif /* ROM_PATCH */
static int  g_nA2DIndex        = 0;

#ifdef USE_SCALE_EFFECTS
static int  g_nScaleEffectIndex= 0;
#endif /* USE_SCALE_EFFECTS */

#ifdef USE_VIRTUAL_3D_VIEW
static int g_d3d_virtual3d_enable = 0;
static int g_d3d_nZoomxIndex      = 0;
static int g_d3d_nZoomyIndex      = 0;
#endif /* USE_VIRTUAL_3D_VIEW */
#ifdef MENU_TRANS
static COLORREF	g_crMenuColor	= RGB(0,0,0);
#endif /*MENU_TRANS*/
#ifdef TRANS_UI
static int  g_nUITransparencyIndex  = 0;
#endif /* TRANS_UI */

static HICON g_hIcon = NULL;

/* Property sheets */

#ifdef TREE_SHEET
#define	SHEET_TREE_WIDTH		180
static int			g_nFirstInitPropertySheet = 0;
static RECT 		_rcTabCtrl;
static RECT 		_rcTabCaption;
static RECT 		_rcSheetSnap;
static HBITMAP 		_hSheetBitmap = NULL;
static int			_bUseScreenShot = FALSE;
static int			_nCaptionHeight;
static HWND			_hSheetTreeCtrl = NULL;
static HINSTANCE	_hSheetInstance	= 0;
static WNDPROC		_pfnOldSheetProc = NULL;
#endif /*TREE_SHEET (END)*/

#define HIGHLIGHT_COLOR RGB(0,196,0)
HBRUSH highlight_brush = NULL;
HBRUSH background_brush = NULL;
#define VECTOR_COLOR RGB( 190, 0, 0) //DARK RED
#define FOLDER_COLOR RGB( 0, 128, 0 ) // DARK GREEN
#define PARENT_COLOR RGB( 190, 128, 192 ) // PURPLE
#define GAME_COLOR RGB( 0, 128, 192 ) // CYAN

BOOL PropSheetFilter_Vector(void)
{
	if (IS_GLOBAL)
	{
#if 1
		int i;

		for (i = 0; drivers[i]; i++)
			if (DriverIsVector(i))
				return 1;
#endif

		return 0;
	}

	if (IS_FOLDER)
	{
		if (g_nPropertyMode == SOURCE_VECTOR)
			return 1;

		return FolderHasVector(g_pFolder);
	}

	return DriverIsVector(g_nGame);
}

BOOL PropSheetFilter_BIOS(void)
{
	if (IS_GLOBAL)
		return (GetSystemBiosInfo(0) != NULL);

	return 0;
}

#ifdef ROM_PATCH
BOOL PropSheetFilter_IPS(void)
{
	if (IS_GAME)
		return HasPatch(drivers[g_nGame]->name, "*");

	return 0;
}
#endif /* ROM_PATCH */


/* Help IDs */
static DWORD dwHelpIDs[] =
{
	
	IDC_A2D,                HIDC_A2D,
	IDC_ANTIALIAS,          HIDC_ANTIALIAS,
	IDC_ARTRES,             HIDC_ARTRES,
	IDC_ARTWORK,            HIDC_ARTWORK,
	IDC_ARTWORK_CROP,       HIDC_ARTWORK_CROP,
	IDC_ASPECTRATIOD,       HIDC_ASPECTRATIOD,
	IDC_ASPECTRATION,       HIDC_ASPECTRATION,
	IDC_AUTOFRAMESKIP,      HIDC_AUTOFRAMESKIP,
	IDC_BACKDROPS,          HIDC_BACKDROPS,
	IDC_BEAM,               HIDC_BEAM,
	IDC_BEZELS,             HIDC_BEZELS,
	IDC_BRIGHTNESS,         HIDC_BRIGHTNESS,
	IDC_BRIGHTCORRECT,      HIDC_BRIGHTCORRECT,
	IDC_BROADCAST,          HIDC_BROADCAST,
	IDC_RANDOM_BG,          HIDC_RANDOM_BG,
	IDC_CHEAT,              HIDC_CHEAT,
	IDC_DDRAW,              HIDC_DDRAW,
	IDC_DEFAULT_INPUT,      HIDC_DEFAULT_INPUT,
	IDC_EFFECT,             HIDC_EFFECT,
	IDC_FILTER_CLONES,      HIDC_FILTER_CLONES,
	IDC_FILTER_EDIT,        HIDC_FILTER_EDIT,
	IDC_FILTER_NONWORKING,  HIDC_FILTER_NONWORKING,
	IDC_FILTER_ORIGINALS,   HIDC_FILTER_ORIGINALS,
	IDC_FILTER_RASTER,      HIDC_FILTER_RASTER,
	IDC_FILTER_UNAVAILABLE, HIDC_FILTER_UNAVAILABLE,
	IDC_FILTER_VECTOR,      HIDC_FILTER_VECTOR,
	IDC_FILTER_WORKING,     HIDC_FILTER_WORKING,
	IDC_FLICKER,            HIDC_FLICKER,
	IDC_FLIPX,              HIDC_FLIPX,
	IDC_FLIPY,              HIDC_FLIPY,
	IDC_FRAMESKIP,          HIDC_FRAMESKIP,
	IDC_GAMMA,              HIDC_GAMMA,
	IDC_HISTORY,            HIDC_HISTORY,
	IDC_HWSTRETCH,          HIDC_HWSTRETCH,
	IDC_INTENSITY,          HIDC_INTENSITY,
	IDC_JOYSTICK,           HIDC_JOYSTICK,
	IDC_KEEPASPECT,         HIDC_KEEPASPECT,
#if 0
	IDC_LANGUAGECHECK,      HIDC_LANGUAGECHECK,
	IDC_LANGUAGEEDIT,       HIDC_LANGUAGEEDIT,
#endif
	IDC_LEDS,               HIDC_LEDS,
	IDC_LOG,                HIDC_LOG,
	IDC_SLEEP,              HIDC_SLEEP,
	IDC_MATCHREFRESH,       HIDC_MATCHREFRESH,
	IDC_MAXIMIZE,           HIDC_MAXIMIZE,
	IDC_OVERLAYS,           HIDC_OVERLAYS,
	IDC_PROP_RESET,         HIDC_PROP_RESET,
	IDC_REFRESH,            HIDC_REFRESH,
	IDC_RESDEPTH,           HIDC_RESDEPTH,
	IDC_RESET_DEFAULT,      HIDC_RESET_DEFAULT,
	IDC_RESET_FILTERS,      HIDC_RESET_FILTERS,
	IDC_RESET_GAMES,        HIDC_RESET_GAMES,
	IDC_RESET_UI,           HIDC_RESET_UI,
	IDC_ROTATE,             HIDC_ROTATE,
	IDC_SAMPLERATE,         HIDC_SAMPLERATE,
	IDC_SAMPLES,            HIDC_SAMPLES,
	IDC_SCANLINES,          HIDC_SCANLINES,
	IDC_SIZES,              HIDC_SIZES,
	IDC_START_GAME_CHECK,   HIDC_START_GAME_CHECK,
	IDC_SWITCHBPP,          HIDC_SWITCHBPP,
	IDC_SWITCHRES,          HIDC_SWITCHRES,
	IDC_SYNCREFRESH,        HIDC_SYNCREFRESH,
	IDC_THROTTLE,           HIDC_THROTTLE,
	IDC_TRANSLUCENCY,       HIDC_TRANSLUCENCY,
	IDC_TRIPLE_BUFFER,      HIDC_TRIPLE_BUFFER,
	IDC_USE_DEFAULT,        HIDC_USE_DEFAULT,
	IDC_USE_FILTER,         HIDC_USE_FILTER,
#ifdef USE_VOLUME_AUTO_ADJUST
	IDC_VOLUME_ADJUST,      HIDC_VOLUME_ADJUST,
#endif /* USE_VOLUME_AUTO_ADJUST */
	IDC_USE_MOUSE,          HIDC_USE_MOUSE,
#ifdef USE_JOY_MOUSE_MOVE // Support Stick-type Pointing Device by miko2u@hotmail.com
	IDC_USE_STICKPOINT,     HIDC_USE_MOUSE,
#endif /* USE_JOY_MOUSE_MOVE */
	IDC_USE_SOUND,          HIDC_USE_SOUND,
	IDC_VOLUME,             HIDC_VOLUME,
	IDC_WAITVSYNC,          HIDC_WAITVSYNC,
	IDC_WINDOWED,           HIDC_WINDOWED,
	IDC_PAUSEBRIGHT,        HIDC_PAUSEBRIGHT,
	IDC_LIGHTGUN,           HIDC_LIGHTGUN,
	IDC_DUAL_LIGHTGUN,      HIDC_DUAL_LIGHTGUN,
	IDC_RELOAD,             HIDC_RELOAD,
	IDC_STEADYKEY,          HIDC_STEADYKEY,
	IDC_OLD_TIMING,         HIDC_OLD_TIMING,
	IDC_JOY_GUI,            HIDC_JOY_GUI,
	IDC_RANDOM_BG,          HIDC_RANDOM_BG,
	IDC_SKIP_DISCLAIMER,    HIDC_SKIP_DISCLAIMER,
	IDC_SKIP_GAME_INFO,     HIDC_SKIP_GAME_INFO,
	IDC_HIGH_PRIORITY,      HIDC_HIGH_PRIORITY,
	IDC_D3D,                HIDC_D3D,
	IDC_D3D_FILTER,         HIDC_D3D_FILTER,
	IDC_D3D_EFFECT,         HIDC_D3D_EFFECT,
	IDC_D3D_TEXTURE_MANAGEMENT, HIDC_D3D_TEXTURE_MANAGEMENT,
	IDC_AUDIO_LATENCY,      HIDC_AUDIO_LATENCY,
	IDC_D3D_PRESCALE,       HIDC_D3D_PRESCALE,
	IDC_D3D_SCANLINES_ENABLE, HIDC_D3D_SCANLINES,
	IDC_D3D_SCANLINES,      HIDC_D3D_SCANLINES,
	IDC_D3D_FEEDBACK_ENABLE, HIDC_D3D_FEEDBACK,
	IDC_D3D_FEEDBACK,       HIDC_D3D_FEEDBACK,
	IDC_ZOOM,               HIDC_ZOOM,
	IDC_BIOS,               HIDC_BIOS,
	IDC_CLEAN_STRETCH,      HIDC_CLEAN_STRETCH,
	IDC_D3D_ROTATE_EFFECTS, HIDC_D3D_ROTATE_EFFECTS,
	IDC_STRETCH_SCREENSHOT_LARGER, HIDC_STRETCH_SCREENSHOT_LARGER,
	IDC_LEDMODE,            HIDC_LEDMODE,
	IDC_BIOS1,              HIDC_BIOS1,
	IDC_BIOS2,              HIDC_BIOS2,
	IDC_BIOS3,              HIDC_BIOS3,
	IDC_BIOS4,              HIDC_BIOS4,
	IDC_BIOS5,              HIDC_BIOS5,
	IDC_BIOS6,              HIDC_BIOS6,
	IDC_BIOS7,              HIDC_BIOS7,
	IDC_BIOS8,              HIDC_BIOS8,
#ifdef ROM_PATCH
	IDC_IPS1,               HIDC_IPS1,
	IDC_IPS2,               HIDC_IPS2,
	IDC_IPS3,               HIDC_IPS3,
	IDC_IPS4,               HIDC_IPS4,
	IDC_IPS5,               HIDC_IPS5,
	IDC_IPS6,               HIDC_IPS6,
	IDC_IPS7,               HIDC_IPS7,
	IDC_IPS8,               HIDC_IPS8,
#endif /* ROM_PATCH */
	0,                      0
};

static const char *d3d_prescale_name[] =
{
	"None",
	"Auto",
	"Full",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8"
};

#define NUMD3DPRESCALE (sizeof(d3d_prescale_name) / sizeof(d3d_prescale_name[0]))

static const char *clean_stretch_name[] =
{
	"None",
	"Full",
	"Auto",
	"Horizontal",
	"Vertical"
};

#define NUMCLEANSTRETCH (sizeof(clean_stretch_name) / sizeof(clean_stretch_name[0]))

static struct ComboBoxEffect
{
	const char*	m_pText;
	const char* m_pData;
} g_ComboBoxEffect[] = 
{
	{ "None",                           "none"    },
	{ "25% scanlines",                  "scan25"  },
	{ "50% scanlines",                  "scan50"  },
	{ "75% scanlines",                  "scan75"  },
	{ "75% vertical scanlines",         "scan75v" },
	{ "RGB triad of 16 pixels",         "rgb16"   },
	{ "RGB triad of 6 pixels",          "rgb6"    },
	{ "RGB triad of 4 pixels",          "rgb4"    },
	{ "RGB triad of 4 vertical pixels", "rgb4v"   },
	{ "RGB triad of 3 pixels",          "rgb3"    },
	{ "RGB tiny",                       "rgbtiny" },
	{ "RGB sharp",                      "sharp"   }
};

#define NUMEFFECTS (sizeof(g_ComboBoxEffect) / sizeof(g_ComboBoxEffect[0]))

static const char * d3d_effects_long_name[] =
{
	"None",
	"Auto",
	"Aperture grille",
	"Dot medium mask",
	"Dot medium bright",
	"RGB minimum mask",
	"RGB medium mask",
	"RGB maximum bright",
	"RGB micro",
	"RGB tiny",
// This is same as -prescale, see in windows/wind3dfx.c
//	"RGB sharp",
	"RGB 3 pixel triad",
	"RGB 4 pixel triad",
// Completely same as "RGB 4 pixel triad"
//	"RGB 4 vertical pixel triad",
	"RGB 6 pixel triad",
	"RGB 16 pixel triad",
	"Scanlines 25%",
	"Scanlines 50%",
	"Scanlines 75%"
// Completely same as "Scanlines 75%"
//	"Scanlines vertical 75%"
};

static const char * d3d_effects_short_name[] =
{
	"none",
	"auto",
	"aperturegrille",
	"dotmedmask",
	"dotmedbright",
	"rgbminmask",
	"rgbmedmask",
	"rgbmaxbright",
	"rgbmicro",
	"rgbtiny",
//	"sharp",
	"rgb3",
	"rgb4",
//	"rgb4v",
	"rgb6",
	"rgb16",
	"scan25",
	"scan50",
	"scan75"
//	"scan75v"
};

#define NUMD3DEFFECTS (sizeof(d3d_effects_short_name) / sizeof(d3d_effects_short_name[0]))

static const char * d3d_filter_long_name[] =
{
	"None",
	"Bilinear",
	"Cubic (flat kernel)",
	"Cubic (gaussian kernel)",
	"Anisotropic"
};

#define NUMD3DFILTER (sizeof(d3d_filter_long_name) / sizeof(d3d_filter_long_name[0]))

#ifdef USE_SCALE_EFFECTS
static const char * scale_effects_long_name[] =
{
	"None",
	"Andrea's Scale2x",
	"Andrea's Scale2x3",
	"Andrea's Scale2x4",
	"Andrea's Scale3x",
	"ElSemi's SuperScale",
	"SuperScale 75% SL",
	"Kreed's 2xSaI",
	"Kreed's Super 2xSaI",
	"Dirk Stevens' Eagle",
	"Kreed's Super Eagle",
	"Maxim Stepin's HQ2x",
	"Maxim Stepin's HQ2x3",
	"Maxim Stepin's HQ2x4",
	"Maxim Stepin's HQ3x",
#ifdef SUPPORT_4X_EFFECT
	"Maxim Stepin's HQ4x",
#endif /* SUPPORT_4X_EFFECT */
	"Andrea's LQ2x",
	"Andrea's LQ2x3",
	"Andrea's LQ2x4",
	"Andrea's LQ3x",
#ifdef SUPPORT_4X_EFFECT
	"Andrea's LQ4x",
#endif /* SUPPORT_4X_EFFECT */
	NULL
};

static const char * scale_effects_short_name[] =
{
	"none",
	"scale2x",
	"scale2x3",
	"scale2x4",
	"scale3x",
	"superscale",
	"superscale75",
	"2xsai",
	"super2xsai",
	"eagle",
	"supereagle",
	"hq2x",
	"hq2x3",
	"hq2x4",
	"hq3x",
#ifdef SUPPORT_4X_EFFECT
	"hq4x",
#endif /* SUPPORT_4X_EFFECT */
	"lq2x",
	"lq2x3",
	"lq2x4",
	"lq3x",
#ifdef SUPPORT_4X_EFFECT
	"lq4x",
#endif /* SUPPORT_4X_EFFECT */
	NULL
};

#define NUMSCALEEFFECTS ((sizeof(scale_effects_short_name) / sizeof(scale_effects_short_name[0])) - 1)
#endif /* USE_SCALE_EFFECTS */

static struct ComboBoxLedmode
{
	const char*	m_pText;
	const char*	m_pData;
} g_ComboBoxLedmode[] = 
{
	{ "PS/2 Keyboard",                  "ps/2"    },
	{ "USB Keyboard",                   "usb"     },
};

#define NUMLEDMODES (sizeof(g_ComboBoxLedmode) / sizeof(g_ComboBoxLedmode[0]))

/***************************************************************
 * Public functions
 ***************************************************************/

#ifdef ROM_PATCH
void InitTooltip (HWND hParent)
{
    // struct specifying control classes to register
    INITCOMMONCONTROLSEX iccex; 
    // struct specifying info about tool in ToolTip control
    TOOLINFO ti;
    LPTSTR lptstr = 0;

    /* INITIALIZE COMMON CONTROLS */
	iccex.dwICC = ICC_BAR_CLASSES;
    iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);

	if(!InitCommonControlsEx(&iccex))
	   return;

    /* CREATE A TOOLTIP WINDOW */
    hTooltipWnd = CreateWindowEx(WS_EX_TOPMOST,
        TOOLTIPS_CLASS,
        NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,		
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hParent,
        NULL,
        NULL,
        NULL
        );

    /* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_IDISHWND | TTF_TRANSPARENT | TTF_TRACK | TTF_ABSOLUTE;
    ti.hwnd = hParent;
    ti.hinst = NULL;
    ti.uId = (UINT)hParent;
    ti.lpszText = lptstr;

	ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;

    /* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
    SendMessage(hTooltipWnd, TTM_SETMAXTIPWIDTH, 0, 4096);
    SendMessage(hTooltipWnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
}

void ShowTooltip (HWND hParent, BOOL bShow)
{
	TOOLINFO    ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_IDISHWND;
	ti.hwnd   = hParent;
	ti.uId    = (UINT)hParent;
	SendMessage(hTooltipWnd,TTM_TRACKACTIVATE,(WPARAM)bShow,(LPARAM)&ti);
}

void SetToolTipPosition(HWND hCtrl)
{
	RECT rt = {0,0,0,0};
	GetWindowRect(hCtrl, &rt);

	// move position
	SendMessage(hTooltipWnd, TTM_TRACKPOSITION, 0,(LPARAM)MAKELPARAM(rt.right + 3, rt.top + 8));
	// make it top most
	SetWindowPos(hTooltipWnd, HWND_TOPMOST ,0,0,0,0, SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOMOVE);
}

void UpdateToolTipText(HWND hParent, LPWSTR sczBuff, const char *sczTipTitle)
{
	TOOLINFO    ti;

	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_IDISHWND;
	ti.hwnd   = hParent;
	ti.uId    = (UINT)hParent;
	ti.lpszText = sczBuff;
	SendMessage(hTooltipWnd, TTM_SETTITLE, 1, (LPARAM)(void *)_Unicode(sczTipTitle));
	SendMessage(hTooltipWnd, TTM_UPDATETIPTEXT, (WPARAM)0, (LPARAM)&ti);
}
#endif /* ROM_PATCH */

typedef HTHEME (WINAPI *OpenThemeProc)(HWND hwnd, LPCWSTR pszClassList);

HMODULE hThemes;
OpenThemeProc fnOpenTheme;
FARPROC fnIsThemed;

void PropertiesInit(void)
{
	hThemes = LoadLibraryA("uxtheme.dll");

	if (hThemes)
	{
		fnIsThemed = GetProcAddress(hThemes,"IsAppThemed");
	}
	bThemeActive = FALSE;
}

DWORD GetHelpIDs(void)
{
	return (DWORD) (LPSTR) dwHelpIDs;
}

static PROPSHEETPAGE *CreatePropSheetPages(HINSTANCE hInst, BOOL bOnlyDefault,
	UINT *pnMaxPropSheets )
{
	PROPSHEETPAGE *pspages;
	int maxPropSheets;
	int possiblePropSheets;
	int i;

	if (g_pFolder)
	{
		i = 2;
	}
	else
	{
		i = 0;
	}
	for (; g_propSheets[i].pfnDlgProc; i++)
		;

	if (g_pFolder)
	{
		possiblePropSheets = i - 1;
	}
	else
	{
		possiblePropSheets = i + 1;
	}

	pspages = malloc(sizeof(PROPSHEETPAGE) * possiblePropSheets);
	if (!pspages)
		return NULL;
	memset(pspages, 0, sizeof(PROPSHEETPAGE) * possiblePropSheets);

	maxPropSheets = 0;
	if (g_pFolder)
	{
		i = 2;
	}
	else
	{
		i = 0;
	}
	for (; g_propSheets[i].pfnDlgProc; i++)
	{
		if (!bOnlyDefault || g_propSheets[i].bOnDefaultPage)
		{
			if (!g_propSheets[i].pfnFilterProc || g_propSheets[i].pfnFilterProc())
			{
				pspages[maxPropSheets].dwSize                     = sizeof(PROPSHEETPAGE);
				pspages[maxPropSheets].dwFlags                    = 0;
				pspages[maxPropSheets].hInstance                  = hInst;
				pspages[maxPropSheets].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(g_propSheets[i].dwDlgID);
				pspages[maxPropSheets].pfnCallback                = NULL;
				pspages[maxPropSheets].lParam                     = 0;
				pspages[maxPropSheets].pfnDlgProc                 = g_propSheets[i].pfnDlgProc;
				maxPropSheets++;
			}
		}
	}
	
	if (pnMaxPropSheets)
		*pnMaxPropSheets = maxPropSheets;

	return pspages;
}

void InitDefaultPropertyPage(HINSTANCE hInst, HWND hWnd)
{
	PROPSHEETHEADER pshead;
	PROPSHEETPAGE   *pspage;

	g_nGame = GLOBAL_OPTIONS;
	g_pFolder = NULL;
	g_nPropertyMode = SOURCE_GLOBAL;
	/* Get default options to populate property sheets */
	pGameOpts = GetDefaultOptions();
	g_bUseDefaults = FALSE;
	/* Stash the result for comparing later */
	FreeGameOptions(&origGameOpts);
	CopyGameOptions(pGameOpts,&origGameOpts);
	orig_uses_defaults = FALSE;
	g_bReset = FALSE;
	BuildDataMap();

	ZeroMemory(&pshead, sizeof(pshead));
	pspage = CreatePropSheetPages(hInst, TRUE, &pshead.nPages);
	if (!pspage)
		return;

	/* Fill in the property sheet header */
	pshead.hwndParent                 = hWnd;
	pshead.dwSize                     = sizeof(PROPSHEETHEADER);
	pshead.dwFlags                    = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_PROPTITLE | PSH_USECALLBACK;
	pshead.pfnCallback                = PropSheetCallbackProc;
	pshead.hInstance                  = hInst;
	pshead.pszCaption                 = _Unicode(_UI("Default Game"));
	pshead.DUMMYUNIONNAME2.nStartPage = 0;
	pshead.DUMMYUNIONNAME.pszIcon     = MAKEINTRESOURCE(IDI_MAME32_ICON);
	pshead.DUMMYUNIONNAME3.ppsp       = pspage;

#ifdef TREE_SHEET
	g_nFirstInitPropertySheet = 1;
	_hSheetInstance	= hInst;
#endif /*TREE_SHEET*/

	/* Create the Property sheet and display it */
#ifdef ROM_PATCH
	hTooltipWnd = NULL;
#endif /* ROM_PATCH */
	if (PropertySheet(&pshead) == -1)
	{
		char temp[100];
		DWORD dwError = GetLastError();

		sprintf(temp, _UI("Propery Sheet Error %d %X"), (int)dwError, (int)dwError);
		MessageBox(0, _Unicode(temp), _Unicode(_UI("Error")), IDOK);
	}

	free(pspage);
}

void InitPropertyPage(HINSTANCE hInst, HWND hWnd, int game_num, HICON hIcon, const char *folder)
{
	InitPropertyPageToPage(hInst, hWnd, game_num, hIcon, PROPERTIES_PAGE, folder);
}

void InitPropertyPageToPage(HINSTANCE hInst, HWND hWnd, int game_num, HICON hIcon, int start_page, const char *folder)
{
	PROPSHEETHEADER pshead;
	PROPSHEETPAGE   *pspage;

	if (highlight_brush == NULL)
		highlight_brush = CreateSolidBrush(HIGHLIGHT_COLOR);

	if (background_brush == NULL)
		background_brush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));

	g_hIcon = CopyIcon(hIcon);
	InitGameAudit(game_num);

	if (folder)
	{
		g_nGame = FOLDER_OPTIONS;
		g_pFolder = folder;
	}
	else
	{
		g_nGame = game_num;
		g_pFolder = NULL;
	}

	if (IS_GAME)
	{
		pGameOpts = GetGameOptions(game_num);
		g_bUseDefaults = GetGameUsesDefaults(game_num);
		/* Stash the result for comparing later */
		FreeGameOptions(&origGameOpts);
		CopyGameOptions(pGameOpts,&origGameOpts);
		g_nPropertyMode = SOURCE_GAME;
	}
	else
	{
		pGameOpts = GetFolderOptions(folder);
		g_bUseDefaults = GetFolderUsesDefaults(folder);
		if (!strcmp(folder, "Vector"))
			g_nPropertyMode = SOURCE_VECTOR;
		else
			g_nPropertyMode = SOURCE_FOLDER;
		/* Stash the result for comparing later */
		FreeGameOptions(&origGameOpts);
		CopyGameOptions(pGameOpts,&origGameOpts);
	}
	orig_uses_defaults = g_bUseDefaults;
	g_bReset = FALSE;
	BuildDataMap();

	ZeroMemory(&pshead, sizeof(PROPSHEETHEADER));

	pspage = CreatePropSheetPages(hInst, FALSE, &pshead.nPages);
	if (!pspage)
		return;

	/* Fill in the property sheet header */
	pshead.hwndParent                 = hWnd;
	pshead.dwSize                     = sizeof(PROPSHEETHEADER);
	pshead.dwFlags                    = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_PROPTITLE | PSH_USECALLBACK;
	pshead.pfnCallback                = PropSheetCallbackProc;
	pshead.hInstance                  = hInst;
	if (!g_pFolder)
	{
		pshead.pszCaption             = _Unicode(UseLangList()
		                                  ? _LST(drivers[g_nGame]->description)
		                                  : ModifyThe(drivers[g_nGame]->description));
	}
	else
	{
		pshead.pszCaption             = _Unicode(_UI(g_pFolder));
	}
	pshead.DUMMYUNIONNAME2.nStartPage = start_page;
	pshead.DUMMYUNIONNAME.pszIcon     = MAKEINTRESOURCE(IDI_MAME32_ICON);
	pshead.DUMMYUNIONNAME3.ppsp       = pspage;

#ifdef TREE_SHEET
	g_nFirstInitPropertySheet = 1;
	_hSheetInstance = hInst;
#endif /*TREE_SHEET*/

	/* Create the Property sheet and display it */
#ifdef ROM_PATCH
	hTooltipWnd = NULL;
#endif /* ROM_PATCH */
	if (PropertySheet(&pshead) == -1)
	{
		char temp[100];
		DWORD dwError = GetLastError();
		sprintf(temp, _UI("Propery Sheet Error %d %X"), (int)dwError, (int)dwError);
		MessageBox(0, _Unicode(temp), _Unicode(_UI("Error")), IDOK);
	}

	free(pspage);
}

/*********************************************************************
 * Local Functions
 *********************************************************************/

/* Build CPU info string */
static LPWSTR GameInfoCPU(UINT nIndex)
{
	int i;
	char buf[1024];
	struct InternalMachineDriver drv;
	expand_machine_driver(drivers[nIndex]->drv, &drv);

	ZeroMemory(buf, sizeof(buf));
	i = 0;
	while (i < MAX_CPU && drv.cpu[i].cpu_type)
	{
		if (drv.cpu[i].cpu_clock >= 1000000)
			sprintf(&buf[strlen(buf)], "%s %d.%06d MHz",
					cputype_name(drv.cpu[i].cpu_type),
					drv.cpu[i].cpu_clock / 1000000,
					drv.cpu[i].cpu_clock % 1000000);
		else
			sprintf(&buf[strlen(buf)], "%s %d.%03d kHz",
					cputype_name(drv.cpu[i].cpu_type),
					drv.cpu[i].cpu_clock / 1000,
					drv.cpu[i].cpu_clock % 1000);

		strcat(buf, "\n");

		i++;
	}

	return _Unicode(buf);
}

/* Build Sound system info string */
static LPWSTR GameInfoSound(UINT nIndex)
{
	int soundnum;
	static char buf[1024];
	struct InternalMachineDriver drv;
	expand_machine_driver(drivers[nIndex]->drv,&drv);

	buf[0] = 0;

	for (soundnum = 0; soundnum < MAX_SOUND; soundnum++)
		if (drv.sound[soundnum].sound_type != 0)
		{
			sprintf(&buf[strlen(buf)], "%s", sndtype_name(drv.sound[soundnum].sound_type));

			if (drv.sound[soundnum].clock)
			{
				if (drv.sound[soundnum].clock >= 1000000)
					sprintf(&buf[strlen(buf)], " %d.%06d MHz",
							drv.sound[soundnum].clock / 1000000,
							drv.sound[soundnum].clock % 1000000);
				else
					sprintf(&buf[strlen(buf)], " %d.%03d kHz",
							drv.sound[soundnum].clock / 1000,
							drv.sound[soundnum].clock % 1000);
			}
			strcat(buf,"\n");
	}
	return _Unicode(buf);
}

/* Build Display info string */
static LPWSTR GameInfoScreen(UINT nIndex)
{
	char buf[1024];
	struct InternalMachineDriver drv;
	expand_machine_driver(drivers[nIndex]->drv, &drv);

	if (drv.video_attributes & VIDEO_TYPE_VECTOR)
		strcpy(buf, _UI("Vector Game"));
	else
	{
		if (drivers[nIndex]->flags & ORIENTATION_SWAP_XY)
			sprintf(buf,_UI("%d x %d (V) %f Hz"),
					drv.default_visible_area.max_y - drv.default_visible_area.min_y + 1,
					drv.default_visible_area.max_x - drv.default_visible_area.min_x + 1,
					drv.frames_per_second);
		else
			sprintf(buf,_UI("%d x %d (H) %f Hz"),
					drv.default_visible_area.max_x - drv.default_visible_area.min_x + 1,
					drv.default_visible_area.max_y - drv.default_visible_area.min_y + 1,
					drv.frames_per_second);
	}
	return _Unicode(buf);
}

/* Build color information string */
static LPWSTR GameInfoColors(UINT nIndex)
{
	char buf[1024];
	struct InternalMachineDriver drv;
	expand_machine_driver(drivers[nIndex]->drv, &drv);

	ZeroMemory(buf, sizeof(buf));
	sprintf(buf, _UI("%d colors "), drv.total_colors);

	return _Unicode(buf);
}

/* Build game status string */
LPWSTR GameInfoStatus(int driver_index, BOOL bRomStatus)
{
	char buffer[1024];
	int audit_result = GetRomAuditResults(driver_index);
	memset(buffer,0,sizeof(char)*1024);

	if (bRomStatus && IsAuditResultKnown(audit_result) == FALSE)
	{
		strcpy(buffer, _UI("Unknown"));
	}

	else if (!bRomStatus || IsAuditResultYes(audit_result))
	{
		if (DriverIsBroken(driver_index))
			strcpy(buffer, _UI("Not working"));
		else
			strcpy(buffer, _UI("Working"));

		//the Flags are checked in the order of "noticability"
		//1) visible deficiencies
		//2) audible deficiencies
		//3) other deficiencies
		if (drivers[driver_index]->flags & GAME_UNEMULATED_PROTECTION)
		{
			if (*buffer != '\0')
				strcat(buffer, "\r\n");
			strcat(buffer, _UI("Game protection isn't fully emulated"));
		}
#ifdef GAME_UNSIMULATED_HARDWARE
		if (drivers[driver_index]->flags & GAME_UNSIMULATED_HARDWARE)
		{
			if (*buffer != '\0')
				strcat(buffer, "\r\n");
			strcat(buffer, _UI("Unsimulated Hardware"));
		}
#endif /* GAME_UNSIMULATED_HARDWARE */
		if (drivers[driver_index]->flags & GAME_WRONG_COLORS)
		{
			if (*buffer != '\0')
				strcat(buffer, "\r\n");
			strcat(buffer, _UI("Colors are completely wrong"));
		}
		if (drivers[driver_index]->flags & GAME_IMPERFECT_COLORS)
		{
			if (*buffer != '\0')
				strcat(buffer, "\r\n");
			strcat(buffer, _UI("Colors aren't 100% accurate"));
		}
		if (drivers[driver_index]->flags & GAME_IMPERFECT_GRAPHICS)
		{
			if (*buffer != '\0')
				strcat(buffer, "\r\n");
			strcat(buffer, _UI("Video emulation isn't 100% accurate"));
		}
		if (drivers[driver_index]->flags & GAME_NO_SOUND)
		{
			if (*buffer != '\0')
				strcat(buffer, "\r\n");
			strcat(buffer, _UI("Game lacks sound"));
		}
		if (drivers[driver_index]->flags & GAME_IMPERFECT_SOUND)
		{
			if (*buffer != '\0')
				strcat(buffer, "\r\n");
			strcat(buffer, _UI("Sound emulation isn't 100% accurate"));
		}
		if (drivers[driver_index]->flags & GAME_NO_COCKTAIL)
		{
			if (*buffer != '\0')
				strcat(buffer, "\r\n");
			strcat(buffer, _UI("Screen flipping is not supported"));
		}
	}
	else
	{
#ifdef MESS
		strcpy(buffer, _UI("BIOS missing"));
#else
		strcpy(buffer, _UI("ROMs missing"));
#endif
	}

	return _Unicode(buffer);
}

/* Build game manufacturer string */
static LPWSTR GameInfoManufactured(UINT nIndex)
{
	char buffer[1024];

	snprintf(buffer,sizeof(buffer),"%s %s",drivers[nIndex]->year,(UseLangList()?_MANUFACT(drivers[nIndex]->manufacturer):drivers[nIndex]->manufacturer));
	return _Unicode(buffer);
}

/* Build Game title string */
LPWSTR GameInfoTitle(UINT nIndex)
{
	char buf[1024];

	if (nIndex == GLOBAL_OPTIONS)
		strcpy(buf, _UI("Global game options\nDefault options used by all games"));
	else if (nIndex == FOLDER_OPTIONS)
		strcpy(buf, _UI("Global folder options\nDefault options used by all games in the folder"));
	else
		UseLangList()?
			sprintf(buf, "%s [%s]", _LST(drivers[nIndex]->description), drivers[nIndex]->name):
			sprintf(buf, "%s [%s]", ModifyThe(drivers[nIndex]->description), drivers[nIndex]->name);
	return _Unicode(buf);
}

/* Build game clone infromation string */
static LPWSTR GameInfoCloneOf(UINT nIndex)
{
	char buf[1024];

	buf[0] = '\0';

	if (DriverIsClone(nIndex))
	{
		UseLangList()?
		sprintf(buf, "%s [%s]",
				ConvertAmpersandString(_LST(drivers[nIndex]->clone_of->description)),
				drivers[nIndex]->clone_of->name):
		sprintf(buf, "%s [%s]",
				ConvertAmpersandString(ModifyThe(drivers[nIndex]->clone_of->description)),
				drivers[nIndex]->clone_of->name);
	}

	return _Unicode(buf);
}

static LPWSTR GameInfoSource(UINT nIndex)
{
	return _Unicode(GetDriverFilename(nIndex));
}

#ifdef TREE_SHEET

static  BOOL m_bPageTreeSelChangedActive = FALSE;
static  void 		MovePropertySheetChildWindows(HWND hWnd, int nDx, int nDy);
static  HTREEITEM 	GetSheetPageTreeItem(int nPage);
static  int 		GetSheetPageTreeCurSelText(char *lpszText, int iBufSize);

static void UpdateSheetCaption(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC			hDC;
	HRGN		hRgn;
	HBRUSH		hBrush;
	RECT		rect, rc;
	int			i, iWidth, iR, iG, iB, iSR, iSG, iSB, iER, iEG, iEB;
	char		szText[256];
	
	// Gradation 시작색과 끝색
	iSR = 0;	iER = 128;
	iSG = 0;	iEG = 0;
	iSB = 128;	iEB = 128;

	memcpy( &rect, &_rcTabCaption, sizeof(RECT) );
	
	iWidth = rect.right - rect.left;
	if(!iWidth) return;

	// 페인트 시작	
	BeginPaint (hWnd, &ps);
	hDC = ps.hdc;
	
	// 페인트 영역 설정 및 그라데이션 막대 드로잉
	hRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
	SelectClipRgn( hDC, hRgn );

	rc.left   = rect.left;
	rc.top    = rect.top;
	rc.right  = rect.left+1;
	rc.bottom = rect.bottom;
	
	for(i=0; i<iWidth; i++)
	{
		iR = iSR + ((iER - iSR) * i) / iWidth;
		iG = iSG + ((iEG - iSG) * i) / iWidth;
		iB = iSB + ((iEB - iSB) * i) / iWidth;
		
		hBrush = CreateSolidBrush( RGB(iR,iG,iB) );
		
		FillRect( hDC, &rc, hBrush );
		DeleteObject( hBrush );

		rc.left  ++;
		rc.right ++;
	}

	// 현재 선택된 페이지의 타이틀(=tree의 아이템이름과 동일)을 획득
	i = GetSheetPageTreeCurSelText(szText, 256);
	if(i > 0) {
		HFONT hFontCaption, hOldFont;
												
		hFontCaption = CreateFontW(	14, 0,		// Height, Width,
									0, 0, 		// nEscapement, nOrientation, 
									FW_BOLD,	// nWeight: FW_BOLD, 
									0, 0, 0, 	// bItalic, bUnderline, cStrikeOut,
									ANSI_CHARSET,		//nCharSet, 
									OUT_DEFAULT_PRECIS,	//nOutPrecision, 
									CLIP_DEFAULT_PRECIS,//nClipPrecision, ,
									DEFAULT_QUALITY,	// nQuality
									FF_MODERN,			// nPitchAndFamily, 
									_Unicode("Tahoma"));

		if(hFontCaption) {
			hOldFont = (HFONT)SelectObject( hDC, hFontCaption );
			
			SetTextColor( hDC, RGB(255,255,255) );
			SetBkMode(hDC, TRANSPARENT);
			
			memcpy( &rc, &rect, sizeof(RECT) );
			rc.left += 5; // 앞쪽 여백
			
			// 타이틀 출력 
			DrawTextW( hDC, _Unicode(szText), strlen(szText), &rc, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
			
			SelectObject(hDC, hOldFont);
			DeleteObject(hFontCaption);
		}
	}
	
	// 설정된 페인트영역 해제
	SelectClipRgn( hDC, NULL );
	DeleteObject( hRgn );

	// ------------------------------------------------------------
	memcpy( &rect, &_rcSheetSnap, sizeof(RECT) );
	
	// 페인트 영역 설정 (snapshot 이미지 영역)
	hRgn = CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom );
	SelectClipRgn( hDC, hRgn );

	if(_hSheetBitmap != NULL) 
	{
		HDC hMemDC;
		HBITMAP hOldBitmap;
		int iWidth, iHeight, iSnapWidth, iSnapHeight, iDrawWidth, iDrawHeight;
		
		if(_bUseScreenShot == TRUE) {
			iSnapWidth  = GetScreenShotWidth();
			iSnapHeight = GetScreenShotHeight();
		}
		else {
			BITMAP bmpInfo;

			GetObject(_hSheetBitmap,sizeof(BITMAP), &bmpInfo);
			iSnapWidth  = bmpInfo.bmWidth;
			iSnapHeight = bmpInfo.bmHeight;
		}
		
		iWidth  = rect.right - rect.left;
		iHeight = rect.bottom - rect.top;
		
		if(iWidth && iHeight) {
			
			int iXOffs, iYOffs;
			double dXRatio, dYRatio;
			
			dXRatio = (double)iWidth  / (double)iSnapWidth;
			dYRatio = (double)iHeight / (double)iSnapHeight;
			
			if(dXRatio > dYRatio) {
				iDrawWidth  = (int)((iSnapWidth  * dYRatio) + 0.5);
				iDrawHeight = (int)((iSnapHeight * dYRatio) + 0.5);
			}
			else {
				iDrawWidth  = (int)((iSnapWidth  * dXRatio) + 0.5);
				iDrawHeight = (int)((iSnapHeight * dXRatio) + 0.5);
			}
			
			iXOffs = (iWidth  - iDrawWidth)  / 2;
			iYOffs = (iHeight - iDrawHeight) / 2;
			
			hMemDC = CreateCompatibleDC(hDC);
	
			hOldBitmap = SelectObject(hMemDC, _hSheetBitmap);
	
			SetStretchBltMode( hDC,STRETCH_HALFTONE );
			StretchBlt( hDC, rect.left+iXOffs, rect.top+iYOffs,
						iDrawWidth, iDrawHeight,
					    hMemDC,0,0,
					    iSnapWidth, iSnapHeight, SRCCOPY);
					    
			SelectObject(hMemDC, hOldBitmap);
			DeleteDC(hMemDC);
		}
	}
	else {
		hBrush = CreateSolidBrush( RGB(220,220,220) );
		FillRect( hDC, &rect, hBrush );
		DeleteObject( hBrush );
	}
	
	// 설정된 페인트영역 해제
	SelectClipRgn( hDC, NULL );
	DeleteObject( hRgn );
	
	// 페인트 끝
	EndPaint (hWnd, &ps); 
		
	return;
}

static LRESULT CALLBACK NewSheetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = FALSE;
	
	switch (Msg)
	{
	case WM_PAINT:
		UpdateSheetCaption(hWnd);
		bHandled = TRUE;
		break;
		
	case WM_NOTIFY:
		switch (((NMHDR *) lParam)->code)
		{
		case TVN_SELCHANGINGA:
		case TVN_SELCHANGINGW:
			if((m_bPageTreeSelChangedActive == FALSE) && (g_nFirstInitPropertySheet==0)) {
				int 		nPage;
				TVITEM 		item;
				NMTREEVIEW	*pTvn = (NMTREEVIEW	*)lParam;
				
				m_bPageTreeSelChangedActive = TRUE;
				item.hItem = pTvn->itemNew.hItem;
				item.mask  = TVIF_PARAM;
				SendMessage(_hSheetTreeCtrl, TVM_GETITEM, 0, (LPARAM)&item);
				
				nPage = (int)item.lParam;
				if(nPage >= 0) {
					SendMessage(hWnd, PSM_SETCURSEL, nPage, 0);
				}
			
				m_bPageTreeSelChangedActive = FALSE;
				bHandled = TRUE;
			}
			break;
		case TVN_SELCHANGEDA:
		case TVN_SELCHANGEDW:
			//UpdateSheetCaption(hWnd);
			InvalidateRect( hWnd, &_rcTabCaption, FALSE );
			bHandled = TRUE;
			break;
		}
		break;
		
	case WM_DESTROY:
		if(_hSheetTreeCtrl != NULL) {
			DestroyWindow(_hSheetTreeCtrl);
			_hSheetTreeCtrl = NULL;
		}
		
		if(	_hSheetBitmap != NULL ) {
			if( _bUseScreenShot == FALSE ) DeleteObject(_hSheetBitmap);
			_hSheetBitmap = NULL;
		}
		_bUseScreenShot = FALSE;

		if(_pfnOldSheetProc) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)_pfnOldSheetProc );
		break;
	}
	
	if((bHandled==FALSE) && _pfnOldSheetProc) 
	 return CallWindowProc( _pfnOldSheetProc, hWnd, Msg, wParam, lParam );
	 
	return 0;
}

static void AdjustChildWindows(HWND hWnd)
{
	char szClass[128];
	DWORD dwStyle;
	
	// groupbox 컨트롤에 붙은 WS_TABSTOP 속성을 삭제
	// - brcc32로 컴파일한 res파일을 windres로 coff포맷으로 변경할 때
	// 강제로 WS_TABSTOP속성이 붙게 된다. 이로 인해 컨트롤의 포커스 이동시에
	// 컨트롤 위에 groupbox의 라인이 오버랩되는 문제가 발생한다. (by DarkCoder)
	GetClassNameW(hWnd, _Unicode(szClass), 128);
	if (!strcmp(szClass, "Button")) {
		// 스타일 값 획득
		dwStyle = GetWindowLong(hWnd, GWL_STYLE);
		// groupbox이고 WS_TABSTOP속성이 지정되어 있는지 검사
		if(((dwStyle & BS_GROUPBOX) == BS_GROUPBOX) && (dwStyle & WS_TABSTOP)) {
			SetWindowLong(hWnd, GWL_STYLE, (dwStyle & ~WS_TABSTOP));
		}
	}
}

static void AdjustPropertySheetChildWindows(HWND hWnd)
{
	HWND hChild = GetWindow(hWnd, GW_CHILD);
	while (hChild)
	{
		//AdjustChildWindows(hChild);
		hChild = GetNextWindow( hChild, GW_HWNDNEXT );		
	}
}

/*
 MovePropertySheetChildWindows
	: 해당 프로퍼티시트내의 모든 차일드윈도를 검색하여 위치를 이동시킴
*/			
static void MovePropertySheetChildWindows(HWND hWnd, int nDx, int nDy)
{
	HWND hChild = GetWindow(hWnd, GW_CHILD);
	RECT rcChild;
	char szText[256];
	
	while (hChild)
	{
		GetWindowTextW( hChild, _Unicode(szText), 256 );
		
		GetWindowRect(hChild, &rcChild);
		OffsetRect( &rcChild, nDx, nDy);

		// ScreenToClient - RECT버전 (MFC방식)
		ScreenToClient(hWnd, (LPPOINT)&rcChild);
		ScreenToClient(hWnd, ((LPPOINT)&rcChild)+1);
		
		AdjustChildWindows(hChild);
		
		MoveWindow( hChild, rcChild.left, rcChild.top,
					(rcChild.right - rcChild.left), (rcChild.bottom - rcChild.top), TRUE );

		hChild = GetNextWindow( hChild, GW_HWNDNEXT );		
	}
}


/*
 GetSheetPageTreeItem
	: 프로퍼티시트내의 tree컨트롤에서 nPage에 해당하는 아이템의 핸들을 획득
*/			
static HTREEITEM GetSheetPageTreeItem(int nPage)
{
	HTREEITEM	hItem;
	TVITEM 		item;
	int			nTreePage;
	
	if(_hSheetTreeCtrl == NULL) return NULL;
	
	hItem = (HTREEITEM)(int)SendMessage(_hSheetTreeCtrl, TVM_GETNEXTITEM, TVGN_ROOT, (LPARAM)NULL );
	while(hItem) {
		item.hItem = hItem;
		item.mask = TVIF_PARAM;
		SendMessage(_hSheetTreeCtrl, TVM_GETITEM, 0, (LPARAM)&item);
		
		nTreePage = (int)item.lParam;
		
		if(nTreePage == nPage) return hItem;
		
		hItem = (HTREEITEM)(int)SendMessage(_hSheetTreeCtrl, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem );
	}
	
	return NULL;
}

static int GetSheetPageTreeCurSelText(char *lpszText, int iBufSize)
{
	HTREEITEM	hItem;
	TVITEM 		item;

	lpszText[0] = 0;	
	if(_hSheetTreeCtrl == NULL) return -1;

	// Tree에서 현재 선택된 아이템 알아내기(Retrieve the currently selected item of the tree)
	hItem = (HTREEITEM)(int)SendMessage(_hSheetTreeCtrl, TVM_GETNEXTITEM, TVGN_CARET, 0);
	
	if(hItem == NULL) return -1;
	
	item.hItem 		= hItem;
	item.mask  		= TVIF_TEXT;
//	item.pszText 	= (LPTSTR)lpszText;
	item.pszText 	= NULL;
	item.cchTextMax = iBufSize;

	SendMessage(_hSheetTreeCtrl, TVM_GETITEM, 0, (LPARAM)&item);
	
	return strlen(lpszText);
}

/*
 ModifyPropertySheetForTreeSheet
	: 해당 프로퍼티시트의 형태를 변경시킴
*/			
void ModifyPropertySheetForTreeSheet(HWND hPageDlg)
{
	HWND  		hWnd, hTabWnd;
	DWORD 		tabStyle;
	int	 		i, nPage, nPageCount;
	RECT  		rectSheet, rectTree;
	HTREEITEM	hItem;

	if(	!g_nFirstInitPropertySheet ) {
		AdjustPropertySheetChildWindows(hPageDlg);
		return;
	}
	
	hWnd = GetParent(hPageDlg);
	if(!hWnd) return;
	
	{
		// Tree컨트롤로부터의 Notify메세지를 얻기위해 기본 프로시저를 후크함.
		LONG_PTR prevProc;
		
		prevProc = GetWindowLongPtr(hWnd, GWLP_WNDPROC);
		_pfnOldSheetProc = (WNDPROC)prevProc;

		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) NewSheetWndProc);
	}
	
	hTabWnd = PropSheet_GetTabControl(hWnd);
	
	if(!hTabWnd) return;
	
	// SingleLine TabControl : 탭컨트롤을 단일라인으로 나오도록 함
	tabStyle = (GetWindowLong(hTabWnd,GWL_STYLE) & ~TCS_MULTILINE);
	SetWindowLong(hTabWnd, GWL_STYLE,tabStyle | TCS_SINGLELINE);

	// Hite it : 탭컨트롤 숨김
	ShowWindow(hTabWnd, SW_HIDE);
	EnableWindow(hTabWnd, FALSE);

	// 탭컨트롤의 영역정보. 추후 페이지타이틀 출력을 위해 사용
	GetWindowRect( hTabWnd, &_rcTabCtrl );
	// ScreenToClient - RECT버전 (MFC방식)
	ScreenToClient(hTabWnd, (LPPOINT)&_rcTabCtrl);
	ScreenToClient(hTabWnd, ((LPPOINT)&_rcTabCtrl)+1);
	
	// 프로퍼티시트의 크기를 변경
	GetWindowRect( hWnd, &rectSheet );
	rectSheet.right += (SHEET_TREE_WIDTH+5);
	SetWindowPos( hWnd, HWND_TOP, -1, -1, (rectSheet.right-rectSheet.left), (rectSheet.bottom-rectSheet.top), SWP_NOZORDER|SWP_NOMOVE);
	CenterWindow( hWnd );

	// 변경된 크기에 맞게 차일드윈도를 이동(재포지션)시킴
	MovePropertySheetChildWindows( hWnd, SHEET_TREE_WIDTH+5, 0);
	
	if(_hSheetTreeCtrl != NULL) {
		DestroyWindow(_hSheetTreeCtrl);
		_hSheetTreeCtrl = NULL;
	}

	// 페이지의 타이틀(캡션)출력을 위한 타이틀높이 계산 (calculate caption height)
	{
		TCITEM item;
		HWND hTempTab;

		hTempTab = CreateWindowExA( 0, "SysTabControl32", NULL,
							WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,
							rectTree.left, rectTree.top,
							(rectTree.right - rectTree.left), (rectTree.bottom - rectTree.top),
							hWnd, (HMENU)0x1234, _hSheetInstance, NULL );
		
		// "" 아이템 추가						
		item.mask 	 = TCIF_TEXT;
		item.iImage  = 0;
		item.lParam  = 0;
		item.pszText = (LPTSTR)"";
		SendMessage(hTempTab, TCM_INSERTITEM, 0, (LPARAM) &item);

		// GetItemRect		
		SendMessage(hTempTab, TCM_GETITEMRECT, 0, (LPARAM)&_rcTabCaption );
		_nCaptionHeight = (_rcTabCaption.bottom - _rcTabCaption.top );
		
		_rcTabCaption.left   = _rcTabCtrl.left + SHEET_TREE_WIDTH + 5;
		_rcTabCaption.top    = 4;
		_rcTabCaption.right  = _rcTabCaption.left + (_rcTabCtrl.right - _rcTabCtrl.left);
		_rcTabCaption.bottom = _rcTabCaption.top + _nCaptionHeight;
		
		DestroyWindow(hTempTab);
	}

	// 스냅샷 로드 관련 부분
	// SHEET_TREE_WIDTH에 대응하는 4:3비율의 높이 계산
	i = (int)((SHEET_TREE_WIDTH * 3) / 4 + 0.5);
	
	_rcSheetSnap.left   = _rcTabCtrl.left + 4;
	_rcSheetSnap.top    = (_rcTabCtrl.bottom - i);
	_rcSheetSnap.right  = _rcTabCtrl.left + SHEET_TREE_WIDTH;
	_rcSheetSnap.bottom = _rcTabCtrl.bottom;
	
	if ((g_nGame == GLOBAL_OPTIONS) ||(g_nGame == FOLDER_OPTIONS))
	{
		// "기본 게임 옵션" / "폴더 게임 옵션" 모드인 경우
		_hSheetBitmap   = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_ABOUT));
		_bUseScreenShot = FALSE;
	}
	else {
		if(!ScreenShotLoaded()) LoadScreenShot(g_nGame, TAB_SCREENSHOT );
		
		if(ScreenShotLoaded()) {
			_hSheetBitmap   = GetScreenShotHandle();
			_bUseScreenShot = TRUE;
		}
		else {
			// 해당 게임의 스크린샷을 찾을 수 없는 경우
			_hSheetBitmap   = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_MISSING_SNAPSHOT));
			_bUseScreenShot = FALSE;
		}
	}
	
	
	// TREE컨트롤의 정보 빌드
	rectTree.left   = _rcTabCtrl.left + 4;
	rectTree.top    = _rcTabCtrl.top  + 5;
	rectTree.right  = _rcTabCtrl.left + SHEET_TREE_WIDTH;
	rectTree.bottom = (_rcTabCtrl.bottom - i) - 5;
	
		
	// TREE컨트롤 생성
	_hSheetTreeCtrl = CreateWindowExA( WS_EX_CLIENTEDGE|WS_EX_NOPARENTNOTIFY,
							"SysTreeView32", "PageTree",
							WS_TABSTOP|WS_CHILD|WS_VISIBLE|TVS_SHOWSELALWAYS|TVS_TRACKSELECT|TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS,
							rectTree.left, rectTree.top,
							(rectTree.right - rectTree.left), (rectTree.bottom - rectTree.top),
							hWnd, (HMENU)0x7EEE, _hSheetInstance, NULL );
							
	if( _hSheetTreeCtrl == NULL ) {
		char temp[100];
		DWORD dwError = GetLastError();
		sprintf(temp, "ProperySheet TreeCtrl Creation Error %d %X", (int)dwError, (int)dwError);
		MessageBoxA(hWnd, temp, "Error", IDOK);
	}

	// 생성된 TREE에 내용 채우기 -----------------------------------------
	SendMessage(_hSheetTreeCtrl, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);	// 형식적인 아이템삭제
	
	nPageCount = SendMessage(hTabWnd, TCM_GETITEMCOUNT, 0, 0L);

	// 탭컨트롤의 내용을 TREE컨트롤에 채우기
	for (nPage = 0; nPage < nPageCount; ++nPage)
	{
		char 			szText[256];
		TCITEM			ti;
		TVINSERTSTRUCT  tvis;
		LPTVITEM		lpTvItem;
		
		// Get title and image of the page
		memset( &ti, 0, sizeof(TCITEM) );
		ti.mask       = TCIF_TEXT|TCIF_IMAGE;
		ti.cchTextMax = 256;
		ti.pszText    = (LPTSTR)szText;

		SendMessage(hTabWnd, TCM_GETITEM, nPage, (LPARAM)&ti);		

#if (_WIN32_IE >= 0x0400)
		lpTvItem = &tvis.DUMMYUNIONNAME.item;
#else
		lpTvItem = &tvis.item;		
#endif
		// Create an item in the tree for the page
		tvis.hParent 	  	= TVI_ROOT;
		tvis.hInsertAfter 	= TVI_LAST;
		lpTvItem->mask 	  	= TVIF_TEXT;
		lpTvItem->pszText 	= (LPTSTR)szText;
		lpTvItem->iImage  	= 0;
		lpTvItem->iSelectedImage = 0;
		lpTvItem->state		= 0;
		lpTvItem->stateMask = 0;
		lpTvItem->lParam 	= (LPARAM)NULL;
		
		// insert Item
		hItem = (HTREEITEM)(int)SendMessage(_hSheetTreeCtrl, TVM_INSERTITEM, 0, (LPARAM)&tvis);
		
		if (hItem)
		{
			TVITEM item;
			
			item.hItem 		= hItem;
			item.mask 		= TVIF_PARAM;
			item.pszText 	= NULL;
			item.iImage 	= 0;
			item.iSelectedImage = 0;
			item.state 		= 0;
			item.stateMask 	= 0;
			item.lParam 	= nPage;
			
			SendMessage(_hSheetTreeCtrl, TVM_SETITEM, 0, (LPARAM)&item);
		}
	}
	

	// 현재의 tab컨트롤의 선택사항을 tree컨트롤과 동기화
	nPage = SendMessage(hTabWnd, TCM_GETCURSEL, 0, 0);
	if(nPage != -1) {
		hItem = GetSheetPageTreeItem( nPage );
		if(hItem) SendMessage(_hSheetTreeCtrl, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
	}
	
	g_nFirstInitPropertySheet = 0;
}
#endif /* TREE_SHEET */

static int CALLBACK PropSheetCallbackProc(HWND hDlg, UINT Msg, LPARAM lParam)
{
	switch (Msg)
	{
	case PSCB_INITIALIZED:
		TranslateDialog(hDlg, lParam, FALSE);
		break;
	}
	return 0;
}

/* Handle the information property page */
INT_PTR CALLBACK GamePropertiesDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HWND hWnd;
	switch (Msg)
	{
	case WM_INITDIALOG:
#ifdef TREE_SHEET
		ModifyPropertySheetForTreeSheet(hDlg);
#endif /*TREE_SHEET*/

		if (g_hIcon)
			SendDlgItemMessage(hDlg, IDC_GAME_ICON, STM_SETICON, (WPARAM) g_hIcon, 0);

		TranslateDialog(hDlg, lParam, TRUE);
#if defined(USE_SINGLELINE_TABCONTROL)
		{
			HWND hWnd = PropSheet_GetTabControl(GetParent(hDlg));
			DWORD tabStyle = (GetWindowLong(hWnd,GWL_STYLE) & ~TCS_MULTILINE);
			SetWindowLong(hWnd,GWL_STYLE,tabStyle | TCS_SINGLELINE);
		}
#endif

		Static_SetText(GetDlgItem(hDlg, IDC_PROP_TITLE),         GameInfoTitle(g_nGame));

		if (IS_GAME)
		{
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_MANUFACTURED),  GameInfoManufactured(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_STATUS),        GameInfoStatus(g_nGame, FALSE));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_CPU),           GameInfoCPU(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_SOUND),         GameInfoSound(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_SCREEN),        GameInfoScreen(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_COLORS),        GameInfoColors(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_CLONEOF),       GameInfoCloneOf(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_SOURCE),        GameInfoSource(g_nGame));

			if (DriverIsClone(g_nGame))
			{
				ShowWindow(GetDlgItem(hDlg, IDC_PROP_CLONEOF_TEXT), SW_SHOW);
			}
			else
			{
				ShowWindow(GetDlgItem(hDlg, IDC_PROP_CLONEOF_TEXT), SW_HIDE);
			}
		}
		hWnd = PropSheet_GetTabControl(GetParent(hDlg));
		UpdateBackgroundBrush(hWnd);
		ShowWindow(hDlg, SW_SHOW);
		return 1;

	}
	return 0;
}

/* Handle all options property pages */
INT_PTR CALLBACK GameOptionsProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	switch (Msg)
	{
	case WM_INITDIALOG:
#ifdef TREE_SHEET
		ModifyPropertySheetForTreeSheet(hDlg);
#endif /*TREE_SHEET*/ 
		TranslateDialog(hDlg, lParam, TRUE);

		/* Fill in the Game info at the top of the sheet */
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_TITLE), GameInfoTitle(g_nGame));
		InitializeOptions(hDlg);
		InitializeMisc(hDlg);

		PopulateControls(hDlg);
		OptionsToProp(hDlg, pGameOpts);
		SetPropEnabledControls(hDlg);
		if (IS_GLOBAL)
			ShowWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), SW_HIDE);
		else
			EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);
		ShowWindow(hDlg, SW_SHOW);

		return 1;

	case WM_HSCROLL:
		/* slider changed */
		HANDLE_WM_HSCROLL(hDlg, wParam, lParam, OptOnHScroll);
		g_bUseDefaults = FALSE;
		g_bReset = TRUE;
		EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), TRUE);
		PropSheet_Changed(GetParent(hDlg), hDlg);

		// make sure everything's copied over, to determine what's changed
		PropToOptions(hDlg,pGameOpts);
		ReadControls(hDlg);
		// redraw it, it might be a new color now
		InvalidateRect((HWND)lParam,NULL,TRUE);

		break;

#ifdef MENU_TRANS
	case WM_PAINT:
		GameOptionsPaint( hDlg, wParam, lParam );
		break;
#endif

	case WM_COMMAND:
		{
			/* Below, 'changed' is used to signify the 'Apply'
			 * button should be enabled.
			 */
			WORD wID         = GET_WM_COMMAND_ID(wParam, lParam);
			HWND hWndCtrl    = GET_WM_COMMAND_HWND(wParam, lParam);
			WORD wNotifyCode = GET_WM_COMMAND_CMD(wParam, lParam);
			BOOL changed     = FALSE;
#ifdef ROM_PATCH
			int nCount;
			char patchName[256];
			LPWSTR desc;
#endif /* ROM_PATCH */

			switch (wID)
			{
			case IDC_SIZES:
			case IDC_FRAMESKIP:
			case IDC_EFFECT:
			case IDC_DEFAULT_INPUT:
			case IDC_ROTATE:
			case IDC_SAMPLERATE:
			case IDC_ARTRES:
			case IDC_CLEAN_STRETCH:
			case IDC_M68K_CORE:
#ifdef USE_SCALE_EFFECTS
			case IDC_SCALEEFFECT:
#endif /* USE_SCALE_EFFECTS */
			case IDC_LEDMODE:
#ifdef JOYSTICK_ID
			case IDC_JOYID1:
			case IDC_JOYID2:
			case IDC_JOYID3:
			case IDC_JOYID4:
			case IDC_JOYID5:
			case IDC_JOYID6:
			case IDC_JOYID7:
			case IDC_JOYID8:
#endif /* JOYSTICK_ID */
				if (wNotifyCode == CBN_SELCHANGE)
				{
					changed = TRUE;
				}
				break;

			case IDC_WINDOWED:
				changed = ReadControl(hDlg, wID);
				break;

			case IDC_RESDEPTH:
				if (wNotifyCode == LBN_SELCHANGE)
				{
					ResDepthSelectionChange(hDlg, hWndCtrl);
					changed = TRUE;
				}
				break;

			case IDC_REFRESH:
				if (wNotifyCode == LBN_SELCHANGE)
				{
					RefreshSelectionChange(hDlg, hWndCtrl);
					changed = TRUE;
				}
				break;

			case IDC_ASPECTRATION:
			case IDC_ASPECTRATIOD:
#ifdef USE_VIRTUAL_3D_VIEW
			case IDC_D3D_KEYSTONERATIOU:
			case IDC_D3D_KEYSTONERATIOD:
#endif /* USE_VIRTUAL_3D_VIEW */
				if (wNotifyCode == EN_CHANGE)
				{
					if (g_bInternalSet == FALSE)
						changed = TRUE;
				}
				break;

#ifdef USE_VIDEO_MEMORY
			case IDC_USE_VIDEOMEMORY:
#endif /* USE_VIDEO_MEMORY */

			case IDC_TRIPLE_BUFFER:
				changed = ReadControl(hDlg, wID);
				break;

			case IDC_D3D_FILTER :
			case IDC_D3D_EFFECT :
			case IDC_D3D_PRESCALE :
#ifdef USE_VIRTUAL_3D_VIEW
			case IDC_D3D_ROTATION:
			case IDC_D3D_ZOOMX:
			case IDC_D3D_ZOOMY:
#endif /* USE_VIRTUAL_3D_VIEW */
				if (wNotifyCode == CBN_SELCHANGE)
					changed = TRUE;
				break;

			case IDC_BIOS :
				if (wNotifyCode == CBN_SELCHANGE)
					changed = TRUE;
				break;

			case IDC_BIOS1 :
			case IDC_BIOS2 :
			case IDC_BIOS3 :
			case IDC_BIOS4 :
			case IDC_BIOS5 :
			case IDC_BIOS6 :
			case IDC_BIOS7 :
			case IDC_BIOS8 :
				if (wNotifyCode == CBN_SELCHANGE)
					changed = TRUE;
				break;
				
#ifdef USE_PSXPLUGIN
			//case IDC_COMBO_GPUPLUGIN :
			//	if (wNotifyCode == CBN_SELCHANGE || wNotifyCode == CBN_EDITCHANGE) 
			//		changed = TRUE;
			//	break;
				
			// PSX_GPU_EXTRA
			case IDC_COMBO_PSXGPU_SCREENSIZE :
				if (wNotifyCode == CBN_SELCHANGE || wNotifyCode == CBN_EDITCHANGE) 
					changed = TRUE;
				break;

			//case IDC_BTN_GPUCFG:
			//	if (wNotifyCode == BN_CLICKED) {
			//		RunPSXGPUConfigure(hDlg);
			//	}
				break;
#endif

#ifdef ROM_PATCH
			case IDC_IPS1 :
			case IDC_IPS2 :
			case IDC_IPS3 :
			case IDC_IPS4 :
			case IDC_IPS5 :
			case IDC_IPS6 :
			case IDC_IPS7 :
			case IDC_IPS8 :
				if (wNotifyCode == CBN_SETFOCUS)
				{
					SetToolTipPosition(hWndCtrl);
					
					nCount = ComboBox_GetCurSel(hWndCtrl);
					if (nCount > 0)
					{
						ComboBox_GetLBTextA(hWndCtrl, nCount, patchName);
						desc = GetPatchDesc(drivers[g_nGame]->name, patchName);
						if (desc)
						{
							UpdateToolTipText(hDlg,desc,patchName);
							free(desc);
							ShowTooltip(hDlg, TRUE);
						}
					}
				}
				else if (wNotifyCode == CBN_KILLFOCUS)
				{
					ShowTooltip(hDlg, FALSE);
				}
				else if (wNotifyCode == CBN_SELCHANGE)
				{
					nCount = ComboBox_GetCurSel(hWndCtrl);
					if (nCount > 0)
					{
						ComboBox_GetLBTextA(hWndCtrl, nCount, patchName);
						desc = GetPatchDesc(drivers[g_nGame]->name, patchName);
						if (desc)
						{
							UpdateToolTipText(hDlg,desc,patchName);
							free(desc);
							ShowTooltip(hDlg, TRUE);
						}
						else
							ShowTooltip(hDlg, FALSE);
					}
					else
						ShowTooltip(hDlg, FALSE);
					changed = TRUE;
				}
				break;
#endif /* ROM_PATCH */
			case IDC_PROP_RESET:
				if (wNotifyCode != BN_CLICKED)
					break;

				FreeGameOptions(pGameOpts);
				CopyGameOptions(&origGameOpts,pGameOpts);
				if (IS_GAME)
					SetGameUsesDefaults(g_nGame,orig_uses_defaults);
				else if (IS_FOLDER)
					SetFolderUsesDefaults(g_pFolder,orig_uses_defaults);

				BuildDataMap();
				PopulateControls(hDlg);
				OptionsToProp(hDlg, pGameOpts);
				SetPropEnabledControls(hDlg);
				g_bReset = FALSE;
				PropSheet_UnChanged(GetParent(hDlg), hDlg);
				g_bUseDefaults = orig_uses_defaults;
				EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
				break;

			case IDC_USE_DEFAULT:
				if (!IS_GLOBAL)
				{
					if (IS_GAME)
					{
						SetGameUsesDefaults(g_nGame,TRUE);

						pGameOpts = GetGameOptions(g_nGame);
						g_bUseDefaults = GetGameUsesDefaults(g_nGame);
					}
					else
					{
						SetFolderUsesDefaults(g_pFolder,TRUE);

						pGameOpts = GetFolderOptions(g_pFolder);
						g_bUseDefaults = GetFolderUsesDefaults(g_pFolder);
					}

					BuildDataMap();
					PopulateControls(hDlg);
					OptionsToProp(hDlg, pGameOpts);
					SetPropEnabledControls(hDlg);
					if (orig_uses_defaults != g_bUseDefaults)
					{
						PropSheet_Changed(GetParent(hDlg), hDlg);
						g_bReset = TRUE;
					}
					else
					{
						if (IS_GAME)
						{
							PropSheet_UnChanged(GetParent(hDlg), hDlg);
							g_bReset = FALSE;
						}
						else
						{
							if (strcmp(origGameOpts.bios, pGameOpts->bios))
							{
								PropSheet_Changed(GetParent(hDlg), hDlg);
								g_bReset = TRUE;
							}
							else
							{
								PropSheet_UnChanged(GetParent(hDlg), hDlg);
								g_bReset = FALSE;
							}
						}
					}
					EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
				}

				break;
#ifdef MENU_TRANS
			case IDC_BTN_MENUBGCOLOR:
			{
				if (wNotifyCode != BN_CLICKED)
					break;

				g_crMenuColor = pGameOpts->menu_color;
				PickGameMenuColor( hDlg, &g_crMenuColor );
				pGameOpts->menu_color = g_crMenuColor;
				
				InvalidateRect(hDlg,NULL,FALSE);
			}
			break;
#endif
			default:
#ifdef MESS
				if (MessPropertiesCommand(g_nGame, hDlg, wNotifyCode, wID, &changed))
					break;
#endif

				if (wNotifyCode == BN_CLICKED)
				{
					switch (wID)
					{
					case IDC_SCANLINES:
						if (Button_GetCheck(GetDlgItem(hDlg, IDC_SCANLINES)))
						{
							Button_SetCheck(GetDlgItem(hDlg, IDC_VSCANLINES), FALSE);
						}
						break;

					case IDC_VSCANLINES:
						if (Button_GetCheck(GetDlgItem(hDlg, IDC_VSCANLINES)))
						{
							Button_SetCheck(GetDlgItem(hDlg, IDC_SCANLINES), FALSE);
						}
						break;
					}
					changed = TRUE;
				}
			}

			if (changed == TRUE)
			{
				// enable the apply button
				if (IS_GAME)
				{
					SetGameUsesDefaults(g_nGame,FALSE);
					g_bUseDefaults = FALSE;
				}
				else if (IS_FOLDER)
				{
					if (wID != IDC_BIOS)
					{
						SetFolderUsesDefaults(g_pFolder,FALSE);
						g_bUseDefaults = FALSE;
					}
				}
				PropSheet_Changed(GetParent(hDlg), hDlg);
				g_bReset = TRUE;
				EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
			}
			SetPropEnabledControls(hDlg);

			// make sure everything's copied over, to determine what's changed
			PropToOptions(hDlg, pGameOpts);
			ReadControls(hDlg);

			// redraw it, it might be a new color now
			if (GetDlgItem(hDlg,wID))
				InvalidateRect(GetDlgItem(hDlg,wID),NULL,FALSE);
		}
		break;

	case WM_NOTIFY:
		switch (((NMHDR *) lParam)->code)
		{
		case PSN_SETACTIVE:
			/* Initialize the controls. */
			PopulateControls(hDlg);
			OptionsToProp(hDlg, pGameOpts);
			SetPropEnabledControls(hDlg);
			EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
			break;

		case PSN_APPLY:
			/* Save and apply the options here */
			PropToOptions(hDlg, pGameOpts);
			ReadControls(hDlg);
			if (IS_GLOBAL)
				pGameOpts = GetDefaultOptions();
			else if (IS_FOLDER)
			{
				SetFolderUsesDefaults(g_pFolder,g_bUseDefaults);
				orig_uses_defaults = g_bUseDefaults;
				pGameOpts = GetFolderOptions(g_pFolder);
			}
			else
			{
				SetGameUsesDefaults(g_nGame,g_bUseDefaults);
				orig_uses_defaults = g_bUseDefaults;
				pGameOpts = GetGameOptions(g_nGame);
			}

			FreeGameOptions(&origGameOpts);
			CopyGameOptions(pGameOpts,&origGameOpts);

			BuildDataMap();
			PopulateControls(hDlg);
			OptionsToProp(hDlg, pGameOpts);
			SetPropEnabledControls(hDlg);
			EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
			g_bReset = FALSE;
			PropSheet_UnChanged(GetParent(hDlg), hDlg);
			SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
			return PSNRET_NOERROR;

		case PSN_KILLACTIVE:
			/* Save Changes to the options here. */
			PropToOptions(hDlg, pGameOpts);
			ReadControls(hDlg);
			ResetDataMap();
			if (IS_GAME)
				SetGameUsesDefaults(g_nGame,g_bUseDefaults);
			else if (IS_FOLDER)
				SetFolderUsesDefaults(g_pFolder,g_bUseDefaults);
			SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
			return 1;  

		case PSN_RESET:
			// Reset to the original values. Disregard changes
			FreeGameOptions(pGameOpts);
			CopyGameOptions(&origGameOpts,pGameOpts);
			if (IS_GAME)
				SetGameUsesDefaults(g_nGame,orig_uses_defaults);
			else if (IS_FOLDER)
				SetFolderUsesDefaults(g_pFolder,orig_uses_defaults);
			SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
			break;

		case PSN_HELP:
			// User wants help for this property page
			break;
		}
		break;

	case WM_CTLCOLORSTATIC :
	case WM_CTLCOLOREDIT :
		//Set the Coloring of the elements
		if( GetControlID(hDlg,(HWND)lParam) < 0)
			break;

		if (IsControlOptionValue(hDlg,(HWND)lParam, GetDefaultOptions() ) )
		{
			//Normal Black case
			SetTextColor((HDC)wParam,COLOR_WINDOWTEXT);
		}
		else if (((IS_GAME && DriverIsVector(g_nGame)) || (IS_FOLDER && FolderHasVector(g_pFolder))) && IsControlOptionValue(hDlg,(HWND)lParam, GetVectorOptions() ) )
		{
			SetTextColor((HDC)wParam,VECTOR_COLOR);
		}
		else if (IS_GAME && IsControlOptionValue(hDlg,(HWND)lParam, GetSourceOptions(g_nGame) ) )
		{
			SetTextColor((HDC)wParam,FOLDER_COLOR);
		}
		else if (IS_GAME && IsControlOptionValue(hDlg,(HWND)lParam, GetParentOptions(g_nGame) ) )
		{
			SetTextColor((HDC)wParam,PARENT_COLOR);
		}
		else if (IS_GAME && IsControlOptionValue(hDlg,(HWND)lParam, &origGameOpts) )
		{
			SetTextColor((HDC)wParam,GAME_COLOR);
		}
		else
		{
			switch ( g_nPropertyMode )
			{
				case SOURCE_GAME:
					SetTextColor((HDC)wParam,GAME_COLOR);
					break;
				case SOURCE_FOLDER:
					SetTextColor((HDC)wParam,FOLDER_COLOR);
					break;
				case SOURCE_VECTOR:
					SetTextColor((HDC)wParam,VECTOR_COLOR);
					break;
				default:
				case SOURCE_GLOBAL:
					SetTextColor((HDC)wParam,COLOR_WINDOWTEXT);
					break;
			}
		}
		if( Msg == WM_CTLCOLORSTATIC )
		{

		//	SetBkColor((HDC)wParam,GetSysColor(COLOR_3DFACE) );
			if( hThemes )
			{
				if( fnIsThemed && fnIsThemed() )
				{
					HWND hWnd = PropSheet_GetTabControl(GetParent(hDlg));
					// Set the background mode to transparent
					SetBkMode((HDC)wParam, TRANSPARENT);

					// Get the controls window dimensions
					GetWindowRect((HWND)lParam, &rc);

					// Map the coordinates to coordinates with the upper left corner of dialog control as base
					MapWindowPoints(NULL, hWnd, (LPPOINT)(&rc), 2);

					// Adjust the position of the brush for this control (else we see the top left of the brush as background)
					SetBrushOrgEx((HDC)wParam, -rc.left, -rc.top, NULL);

					// Return the brush
					return (INT_PTR)(hBkBrush);
				}
				else
				{
					SetBkColor((HDC) wParam,GetSysColor(COLOR_3DFACE) );
				}
			}
			else
				SetBkColor((HDC) wParam,GetSysColor(COLOR_3DFACE) );
		}
		else
			SetBkColor((HDC)wParam,RGB(255,255,255) );
		UnrealizeObject(background_brush);
		return (DWORD)background_brush;

	case WM_HELP:
		/* User clicked the ? from the upper right on a control */
		HelpFunction(((LPHELPINFO)lParam)->hItemHandle, MAME32CONTEXTHELP, HH_TP_HELP_WM_HELP, GetHelpIDs());
		break;

	case WM_CONTEXTMENU: 
		HelpFunction((HWND)wParam, MAME32CONTEXTHELP, HH_TP_HELP_CONTEXTMENU, GetHelpIDs());
		break; 

	}
	EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);

	return 0;
}

/* Read controls that are not handled in the DataMap */
static void PropToOptions(HWND hWnd, options_type *o)
{
	HWND hCtrl;
	HWND hCtrl2;
	int  nIndex;

	if (IS_GAME)
		SetGameUsesDefaults(g_nGame,g_bUseDefaults);
	else if (IS_FOLDER)
		SetFolderUsesDefaults(g_pFolder,g_bUseDefaults);

	/* resolution size */
	hCtrl = GetDlgItem(hWnd, IDC_SIZES);
	if (hCtrl)
	{
		char buffer[200];

		/* Screen size control */
		nIndex = ComboBox_GetCurSel(hCtrl);
		
		if (nIndex == 0)
			sprintf(buffer, "%dx%d", 0, 0); // auto
		else
		{
			int w, h;

			ComboBox_GetTextA(hCtrl, buffer, sizeof(buffer)-1);
			if (sscanf(buffer, "%d x %d", &w, &h) == 2)
			{
				sprintf(buffer, "%dx%d", w, h);
			}
			else
			{
				sprintf(buffer, "%dx%d", 0, 0); // auto
			}
		}   

		/* resolution depth */
		hCtrl = GetDlgItem(hWnd, IDC_RESDEPTH);
		if (hCtrl)
		{
			int nResDepth = 0;

			nIndex = ComboBox_GetCurSel(hCtrl);
			if (nIndex != CB_ERR)
				nResDepth = ComboBox_GetItemData(hCtrl, nIndex);

			switch (nResDepth)
			{
			default:
			case 0:  strcat(buffer, "x0"); break;
			case 16: strcat(buffer, "x16"); break;
			case 24: strcat(buffer, "x24"); break;
			case 32: strcat(buffer, "x32"); break;
			}
		}
		if (strcmp(buffer,"0x0x0") == 0)
			sprintf(buffer,"auto");
		FreeIfAllocated(&o->resolution);
		o->resolution = strdup(buffer);
	}

	/* refresh */
	hCtrl = GetDlgItem(hWnd, IDC_REFRESH);
	if (hCtrl)
	{
		nIndex = ComboBox_GetCurSel(hCtrl);
		
		pGameOpts->gfx_refresh = ComboBox_GetItemData(hCtrl, nIndex);
	}

	/* aspect ratio */
	hCtrl  = GetDlgItem(hWnd, IDC_ASPECTRATION);
	hCtrl2 = GetDlgItem(hWnd, IDC_ASPECTRATIOD);
	if (hCtrl && hCtrl2)
	{
		int n = 0;
		int d = 0;
		char buffer[200];

		Edit_GetTextA(hCtrl,buffer,sizeof(buffer));
		sscanf(buffer,"%d",&n);

		Edit_GetTextA(hCtrl2,buffer,sizeof(buffer));
		sscanf(buffer,"%d",&d);

		if (n == 0 || d == 0)
		{
			n = 4;
			d = 3;
		}

		snprintf(buffer,sizeof(buffer),"%d:%d",n,d);
		FreeIfAllocated(&o->aspect);
		o->aspect = strdup(buffer);
	}
#ifdef USE_VIRTUAL_3D_VIEW
	/* keystone ratio */
	hCtrl  = GetDlgItem(hWnd, IDC_D3D_KEYSTONERATIOU);
	hCtrl2 = GetDlgItem(hWnd, IDC_D3D_KEYSTONERATIOD);
	if (hCtrl && hCtrl2)
	{
		int u = 0;
		int d = 0;
		char buffer[200];

		Edit_GetTextA(hCtrl,buffer,sizeof(buffer));
		sscanf(buffer,"%d",&u);

		Edit_GetTextA(hCtrl2,buffer,sizeof(buffer));
		sscanf(buffer,"%d",&d);

		if (u == 0 || d == 0)
		{
			u = 1;
			d = 2;
		}

		snprintf(buffer,sizeof(buffer),"%d:%d",u,d);
		FreeIfAllocated(&o->d3d_keystone);
		o->d3d_keystone = strdup(buffer);
	}
#endif /* USE_VIRTUAL_3D_VIEW */
#ifdef MESS
	MessPropToOptions(g_nGame, hWnd, o);
#endif

#ifdef USE_PSXPLUGIN
	/* External Plugin's Name */
	//hCtrl = GetDlgItem(hWnd, IDC_COMBO_GPUPLUGIN);
	//if (hCtrl)
	//{
	//	char buffer[200];
	//	
	//	ComboBox_GetTextA(hCtrl, buffer, sizeof(buffer)-1);
	//	FreeIfAllocated(&o->szGpuPlugIn);
	//	o->szGpuPlugIn = strdup(buffer);
	//}

	// PSX_GPU_EXTRA
	hCtrl = GetDlgItem(hWnd, IDC_COMBO_PSXGPU_SCREENSIZE);
	if (hCtrl)
	{
		nIndex = ComboBox_GetCurSel(hCtrl);
		switch(nIndex) {
		default:
		case 0:	o->nPsxGpuScreenSize = GPUSCR_640x480;	break;
		case 1:	o->nPsxGpuScreenSize = GPUSCR_800x600;	break;
		case 2:	o->nPsxGpuScreenSize = GPUSCR_1024x768; break;
		case 3:	o->nPsxGpuScreenSize = GPUSCR_1152x864; break;
		}
	}
#endif

}

/* Populate controls that are not handled in the DataMap */
static void OptionsToProp(HWND hWnd, options_type* o)
{
	HWND hCtrl;
	HWND hCtrl2;
	char buf[100];
	int  h = 0;
	int  w = 0;
	int  n = 0;
	int  d = 0;
#ifdef USE_VIRTUAL_3D_VIEW
	int  u = 0;
#endif /* USE_VIRTUAL_3D_VIEW */

	g_bInternalSet = TRUE;

	/* video */

	/* get desired resolution */
	if (!stricmp(o->resolution, "auto"))
	{
		w = h = 0;
	}
	else
	if (sscanf(o->resolution, "%dx%dx%d", &w, &h, &d) < 2)
	{
		w = h = d = 0;
	}

	/* Setup sizes list based on depth. */
	UpdateDisplayModeUI(hWnd, d, o->gfx_refresh);

	/* Screen size drop down list */
	hCtrl = GetDlgItem(hWnd, IDC_SIZES);
	if (hCtrl)
	{
		if (w == 0 && h == 0)
		{
			/* default to auto */
			ComboBox_SetCurSel(hCtrl, 0);
		}
		else
		{
			/* Select the mode in the list. */
			int nSelection = 0;
			int nCount = 0;

			/* Get the number of items in the control */
			nCount = ComboBox_GetCount(hCtrl);

			while (0 < nCount--)
			{
				int nWidth, nHeight;

				/* Get the screen size */
				ComboBox_GetLBTextA(hCtrl, nCount, buf);

				if (sscanf(buf, "%d x %d", &nWidth, &nHeight) == 2)
				{
					/* If we match, set nSelection to the right value */
					if (w == nWidth
					&&  h == nHeight)
					{
						nSelection = nCount;
						break;
					}
				}
			}
			ComboBox_SetCurSel(hCtrl, nSelection);
		}
	}

	/* Screen depth drop down list */
	hCtrl = GetDlgItem(hWnd, IDC_RESDEPTH);
	if (hCtrl)
	{
		if (d == 0)
		{
			/* default to auto */
			ComboBox_SetCurSel(hCtrl, 0);
		}
		else
		{
			/* Select the mode in the list. */
			int nSelection = 0;
			int nCount = 0;

			/* Get the number of items in the control */
			nCount = ComboBox_GetCount(hCtrl);

			while (0 < nCount--)
			{
				int nDepth;

				/* Get the screen depth */
				nDepth = ComboBox_GetItemData(hCtrl, nCount);

				/* If we match, set nSelection to the right value */
				if (d == nDepth)
				{
					nSelection = nCount;
					break;
				}
			}
			ComboBox_SetCurSel(hCtrl, nSelection);
		}
	}

	/* Screen refresh list */
	hCtrl = GetDlgItem(hWnd, IDC_REFRESH);
	if (hCtrl)
	{
		if (o->gfx_refresh == 0)
		{
			/* default to auto */
			ComboBox_SetCurSel(hCtrl, 0);
		}
		else
		{
			/* Select the mode in the list. */
			int nSelection = 0;
			int nCount = 0;

			/* Get the number of items in the control */
			nCount = ComboBox_GetCount(hCtrl);

			while (0 < nCount--)
			{
				int nRefresh;

				/* Get the screen depth */
				nRefresh = ComboBox_GetItemData(hCtrl, nCount);

				/* If we match, set nSelection to the right value */
				if (o->gfx_refresh == nRefresh)
				{
					nSelection = nCount;
					break;
				}
			}
			ComboBox_SetCurSel(hCtrl, nSelection);
		}
	}

	
	hCtrl = GetDlgItem(hWnd, IDC_BRIGHTNESSDISP);
	if (hCtrl)
	{
		snprintf(buf,sizeof(buf), "%03.2f", o->gfx_brightness);
		Static_SetTextA(hCtrl, buf);
	}
	
	/* aspect ratio */
	hCtrl  = GetDlgItem(hWnd, IDC_ASPECTRATION);
	hCtrl2 = GetDlgItem(hWnd, IDC_ASPECTRATIOD);
	if (hCtrl && hCtrl2)
	{
		n = 0;
		d = 0;

		if (sscanf(o->aspect, "%d:%d", &n, &d) == 2 && n != 0 && d != 0)
		{
			sprintf(buf, "%d", n);
			Edit_SetTextA(hCtrl, buf);
			sprintf(buf, "%d", d);
			Edit_SetTextA(hCtrl2, buf);
		}
		else
		{
			Edit_SetTextA(hCtrl,  "4");
			Edit_SetTextA(hCtrl2, "3");
		}
	}

	ZoomSelectionChange(hWnd);

	/* core video */
	hCtrl = GetDlgItem(hWnd, IDC_GAMMADISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->f_gamma_correct);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_BRIGHTCORRECTDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->f_bright_correct);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_PAUSEBRIGHTDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->f_pause_bright);
		Static_SetTextA(hCtrl, buf);
	}

	/* Input */
	hCtrl = GetDlgItem(hWnd, IDC_A2DDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->f_a2d);
		Static_SetTextA(hCtrl, buf);
	}

#ifdef TRANS_UI
	hCtrl = GetDlgItem(hWnd, IDC_TRANSPARENCYDISP);
	if (hCtrl)
	{
		sprintf(buf, "%d", o->ui_transparency);
		Static_SetTextA(hCtrl, buf);
	}
#endif /* TRANS_UI */

	/* vector */
	hCtrl = GetDlgItem(hWnd, IDC_BEAMDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->f_beam);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_FLICKERDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->f_flicker);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_INTENSITYDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->f_intensity);
		Static_SetTextA(hCtrl, buf);
	}

	/* sound */
	hCtrl = GetDlgItem(hWnd, IDC_VOLUMEDISP);
	if (hCtrl)
	{
		sprintf(buf, "%ddB", o->attenuation);
		Static_SetTextA(hCtrl, buf);
	}
	AudioLatencySelectionChange(hWnd);

	// d3d
	D3DScanlinesSelectionChange(hWnd);
	D3DFeedbackSelectionChange(hWnd);

#ifdef USE_VIRTUAL_3D_VIEW
	/* d3d rotation */
	hCtrl = GetDlgItem(hWnd, IDC_D3D_ROTATION_DISP);
	if (hCtrl)
	{
		sprintf(buf, "%3d", o->d3d_rotation);
		Static_SetTextA(hCtrl, buf);
	}
 
	/* d3d zoom x */
	hCtrl = GetDlgItem(hWnd, IDC_D3D_ZOOMX_DISP);
	if (hCtrl)
	{
		snprintf(buf,sizeof(buf), "%02.01f", o->d3d_zoomx);
		Static_SetTextA(hCtrl, buf);
	}

	/* d3d zoom y */
	hCtrl = GetDlgItem(hWnd, IDC_D3D_ZOOMY_DISP);
	if (hCtrl)
	{
		snprintf(buf,sizeof(buf), "%02.01f", o->d3d_zoomy);
		Static_SetTextA(hCtrl, buf);
	}

	/* d3d keystone ratio */
	hCtrl  = GetDlgItem(hWnd, IDC_D3D_KEYSTONERATIOU);
	hCtrl2 = GetDlgItem(hWnd, IDC_D3D_KEYSTONERATIOD);
	if (hCtrl && hCtrl2)
	{
		u = 0;
		d = 0;

		if (sscanf(o->d3d_keystone, "%d:%d", &u, &d) == 2 && u != 0 && d != 0)
		{
			sprintf(buf, "%d", u);
			Edit_SetTextA(hCtrl, buf);
			sprintf(buf, "%d", d);
			Edit_SetTextA(hCtrl2, buf);
		}
		else
		{
			Edit_SetTextA(hCtrl,  "1");
			Edit_SetTextA(hCtrl2, "2");
		}
	}
#endif /* USE_VIRTUAL_3D_VIEW */

	g_bInternalSet = FALSE;

	g_nInputIndex = 0;
	hCtrl = GetDlgItem(hWnd, IDC_DEFAULT_INPUT);	
	if (hCtrl)
	{
		int nCount;

		/* Get the number of items in the control */
		nCount = ComboBox_GetCount(hCtrl);

		while (0 < nCount--)
		{
			ComboBox_GetLBTextA(hCtrl, nCount, buf);

			if (stricmp (buf,o->ctrlr) == 0)
			{
				g_nInputIndex = nCount;
			}
		}

		ComboBox_SetCurSel(hCtrl, g_nInputIndex);
	}

#ifdef MESS
	MessOptionsToProp(g_nGame, hWnd, o);
#endif

#ifdef MENU_TRANS
	/* PLUS! */
	g_crMenuColor = o->menu_color;
	MenuTransSelectionChange(hWnd);
#endif

#ifdef USE_PSXPLUGIN
	/* External GPU Plugin's Name */
	//hCtrl = GetDlgItem(hWnd, IDC_COMBO_GPUPLUGIN);
	//if (hCtrl)
	//{
	//	char buffer[256];
	//	int nCount, nSel = -1;
	//	
	//	/* Get the number of items in the control */
	//	nCount = ComboBox_GetCount(hCtrl);
    //    
	//	while (0 < nCount--)
	//	{
	//		ComboBox_GetLBText(hCtrl, nCount, buffer);
	//		
	//		if (stricmp (buffer, o->szGpuPlugIn) == 0)
	//		{
	//			nSel = nCount;
	//		}
	//	}
	//	
	//	if( nSel >= 0)
	//		ComboBox_SetCurSel(hCtrl, nSel);
	//	else
	//		ComboBox_SetTextA(hCtrl, o->szGpuPlugIn);
	//}

	// PSX_GPU_EXTRA
	hCtrl = GetDlgItem(hWnd, IDC_COMBO_PSXGPU_SCREENSIZE);
	if (hCtrl)
	{
		int nSel = -1;
		switch(o->nPsxGpuScreenSize) {
		default:
		case GPUSCR_640x480:	nSel = 0; break;
		case GPUSCR_800x600:	nSel = 1; break;
		case GPUSCR_1024x768:	nSel = 2; break;
		case GPUSCR_1152x864:	nSel = 3; break;
		}
		ComboBox_SetCurSel(hCtrl, nSel);
	}
#endif

}

/* Adjust controls - tune them to the currently selected game */
static void SetPropEnabledControls(HWND hWnd)
{
	HWND hCtrl;
#ifdef USE_VIRTUAL_3D_VIEW
	HWND hCtrl2;
	int  Ctrl3 = 0;
#endif /* USE_VIRTUAL_3D_VIEW */
	int  nIndex;
	int  sound;
	BOOL ddraw = FALSE;
	BOOL d3d = FALSE;
	BOOL hws = FALSE;
	BOOL useart = FALSE;
	int joystick_attached = 9;
	int in_window = 0;
#ifdef JOYSTICK_ID
	int  i;
#endif /* JOYSTICK_ID */

#ifdef MESS
	MessSetPropEnabledControls(hWnd, pGameOpts);
#endif

	nIndex = g_nGame;

	// check windows mode is enabled
	hCtrl = GetDlgItem(hWnd, IDC_WINDOWED);
	if (hCtrl)
		in_window = Button_GetCheck(hCtrl);
	else
		in_window = pGameOpts->window_mode;

	// check ddraw is enabled
	hCtrl = GetDlgItem(hWnd, IDC_DDRAW);
	if (hCtrl)
		ddraw = Button_GetCheck(hCtrl);
	else
		ddraw = pGameOpts->use_ddraw;

	// check hw stretch is enabled
	hCtrl = GetDlgItem(hWnd, IDC_HWSTRETCH);
	if (hCtrl)
		hws = DirectDraw_HasHWStretch() && Button_GetCheck(hCtrl);
	else
		hws = DirectDraw_HasHWStretch() && pGameOpts->ddraw_stretch;

	// check d3d is enabled
	hCtrl = GetDlgItem(hWnd, IDC_D3D);
	if (hCtrl)
		d3d = Button_GetCheck(hCtrl);
	else
		d3d = pGameOpts->use_d3d;

	EnableWindow(GetDlgItem(hWnd, IDC_MAXIMIZE), in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_RESDEPTH), !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_RESDEPTHTEXT), !in_window);

	EnableWindow(GetDlgItem(hWnd, IDC_WAITVSYNC),       ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_TRIPLE_BUFFER),   ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_HWSTRETCH),       ddraw && DirectDraw_HasHWStretch());
	EnableWindow(GetDlgItem(hWnd, IDC_SWITCHRES),       ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_SWITCHBPP),       ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_MATCHREFRESH),    ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_SYNCREFRESH),     ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_REFRESH),         !in_window && ((ddraw && DirectDraw_HasRefresh()) || d3d));
	EnableWindow(GetDlgItem(hWnd, IDC_REFRESHTEXT),     !in_window && ((ddraw && DirectDraw_HasRefresh()) || d3d));
	EnableWindow(GetDlgItem(hWnd, IDC_BRIGHTNESS),      ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_BRIGHTNESSTEXT),  ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_BRIGHTNESSDISP),  ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_ASPECTRATIOTEXT), (ddraw && hws) || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_ASPECTRATION),    (ddraw && hws) || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_ASPECTRATIOD),    (ddraw && hws) || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_ZOOM),            (ddraw && hws) || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_ZOOMTEXT),        (ddraw && hws) || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_ZOOMDIST),        (ddraw && hws) || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_SCANLINES),       !g_nEffectIndex && (!ddraw || !(hws)) && !d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_CLEAN_STRETCH),   (ddraw && hws) || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_CLEAN_STRETCHTEXT), (ddraw && hws) || d3d);

#ifdef USE_SCALE_EFFECTS
	EnableWindow(GetDlgItem(hWnd, IDC_SCALEEFFECTTEXT), ddraw || d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_SCALEEFFECT),     ddraw || d3d);
#ifdef USE_VIDEO_MEMORY
	{
		BOOL enable = ddraw && (pGameOpts->scale_effect > SCALE_EFFECT_MEMORY);
		EnableWindow(GetDlgItem(hWnd, IDC_USE_VIDEOMEMORY), enable );
	}
#endif /*USE_VIDEO_MEMORY*/	
#endif /* USE_SCALE_EFFECTS */

	// d3d
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_FILTER),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_FILTERTEXT),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_TEXTURE_MANAGEMENT),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_EFFECT),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_EFFECTTEXT),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_PRESCALE),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_PRESCALETEXT),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_ROTATE_EFFECTS),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_EFFECTOVERRIDES),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_SCANLINES_ENABLE),d3d);
	EnableWindow(GetDlgItem(hWnd,IDC_D3D_FEEDBACK_ENABLE),d3d);

	hCtrl = GetDlgItem(hWnd,IDC_D3D_SCANLINES_ENABLE);
	if (hCtrl)
	{
		BOOL enable = d3d && Button_GetCheck(hCtrl);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_SCANLINES),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_SCANLINES_DISP),enable);
	}
	
	hCtrl = GetDlgItem(hWnd,IDC_D3D_FEEDBACK_ENABLE);
	if (hCtrl)
	{
		BOOL enable = d3d && Button_GetCheck(hCtrl);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_FEEDBACK),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_FEEDBACK_DISP),enable);
	}
	
#ifdef USE_VIRTUAL_3D_VIEW
	hCtrl2 = GetDlgItem(hWnd,IDC_D3D_VIRTUAL_3DVIEW);
	if (hCtrl2)
	{
		BOOL enable = Button_GetCheck(hCtrl2);
		pGameOpts->use_d3d = 1;
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_ROTATION),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_ROTATION_DISP),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_ZOOMX),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_ZOOMX_DISP),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_ZOOMY),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_ZOOMY_DISP),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_KEYSTONERATIOU),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_KEYSTONERATIOD),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_KEYSTONERATIOTEXT),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_ZOOMXTEXT),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_ZOOMYTEXT),enable);
		EnableWindow(GetDlgItem(hWnd,IDC_D3D_ROTATIONTEXT),enable);
		
		Ctrl3 = pGameOpts->d3d_virtual3d;
		if (Ctrl3 == 0)
		{
			pGameOpts->use_d3d = 0;
		}
	}
#endif /* USE_VIRTUAL_3D_VIEW */
#ifdef TRANS_UI
	hCtrl = GetDlgItem(hWnd, IDC_TRANSUI);
	useart = Button_GetCheck(hCtrl);

	EnableWindow(GetDlgItem(hWnd, IDC_TRANSPARENCY),    useart);
	EnableWindow(GetDlgItem(hWnd, IDC_TRANSPARENCYDISP),useart);
#endif /* TRANS_UI */

	/* Artwork options */
	hCtrl = GetDlgItem(hWnd, IDC_ARTWORK);

	useart = Button_GetCheck(hCtrl);

	EnableWindow(GetDlgItem(hWnd, IDC_ARTWORK_CROP),	useart);
	EnableWindow(GetDlgItem(hWnd, IDC_BACKDROPS),		useart);
	EnableWindow(GetDlgItem(hWnd, IDC_BEZELS),			useart);
	EnableWindow(GetDlgItem(hWnd, IDC_OVERLAYS),		useart);
	EnableWindow(GetDlgItem(hWnd, IDC_ARTRES),			useart);
	EnableWindow(GetDlgItem(hWnd, IDC_ARTRESTEXT),		useart);
	EnableWindow(GetDlgItem(hWnd, IDC_ARTMISCTEXT),		useart);

	/* Joystick options */
	joystick_attached = DIJoystick.Available();

	Button_Enable(GetDlgItem(hWnd,IDC_JOYSTICK),		joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_A2DTEXT),			joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_A2DDISP),			joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_A2D),				joystick_attached);

#ifdef JOYSTICK_ID
	if (Button_GetCheck(GetDlgItem(hWnd, IDC_JOYSTICK)) && DIJoystick.Available())
	{
		EnableWindow(GetDlgItem(hWnd, IDC_JOYIDTEXT),  TRUE);

		for (i = 0; i < 8; i++)
		{
			if (i < DIJoystick_GetNumPhysicalJoysticks())
			{
				EnableWindow(GetDlgItem(hWnd, IDC_JOYID1 + i),     TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_JOYID1TEXT + i), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hWnd, IDC_JOYID1 + i),     FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_JOYID1TEXT + i), FALSE);
			}
		}
	}
	else
	{
		EnableWindow(GetDlgItem(hWnd, IDC_JOYIDTEXT),  FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID1),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID2),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID3),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID4),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID5),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID6),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID7),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID8),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID1TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID2TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID3TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID4TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID5TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID6TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID7TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID8TEXT), FALSE);
	}
#endif /* JOYSTICK_ID */

	/* Trackball / Mouse options */
	if ((nIndex == GLOBAL_OPTIONS || nIndex == FOLDER_OPTIONS) || DriverUsesTrackball(nIndex) || DriverUsesLightGun(nIndex))
		Button_Enable(GetDlgItem(hWnd,IDC_USE_MOUSE),TRUE);
	else
		Button_Enable(GetDlgItem(hWnd,IDC_USE_MOUSE),FALSE);

	if (!in_window && ((nIndex == GLOBAL_OPTIONS || nIndex == FOLDER_OPTIONS) || DriverUsesLightGun(nIndex)))
	{
		BOOL enable = Button_GetCheck(GetDlgItem(hWnd,IDC_LIGHTGUN));

		Button_Enable(GetDlgItem(hWnd,IDC_LIGHTGUN), TRUE);
		Button_Enable(GetDlgItem(hWnd,IDC_DUAL_LIGHTGUN), enable);
		Button_Enable(GetDlgItem(hWnd,IDC_RELOAD), enable);
	}
	else
	{
		Button_Enable(GetDlgItem(hWnd,IDC_LIGHTGUN), FALSE);
		Button_Enable(GetDlgItem(hWnd,IDC_DUAL_LIGHTGUN), FALSE);
		Button_Enable(GetDlgItem(hWnd,IDC_RELOAD), FALSE);
	}

	/* Keyboard LED */
	if (Button_GetCheck(GetDlgItem(hWnd, IDC_LEDS)))
		EnableWindow(GetDlgItem(hWnd, IDC_LEDMODE),  TRUE);
	else
		EnableWindow(GetDlgItem(hWnd, IDC_LEDMODE),  FALSE);


	/* Sound options */
	hCtrl = GetDlgItem(hWnd, IDC_USE_SOUND);
	if (hCtrl)
	{
		sound = Button_GetCheck(hCtrl);
		ComboBox_Enable(GetDlgItem(hWnd, IDC_SAMPLERATE), sound);

		EnableWindow(GetDlgItem(hWnd,IDC_VOLUME),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_RATETEXT),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_USE_FILTER),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_VOLUMEDISP),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_VOLUMETEXT),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_AUDIO_LATENCY),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_AUDIO_LATENCY_DISP),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_AUDIO_LATENCY_TEXT),sound);
#ifdef USE_VOLUME_AUTO_ADJUST
		EnableWindow(GetDlgItem(hWnd,IDC_VOLUME_ADJUST),sound);
#endif /* USE_VOLUME_AUTO_ADJUST */
		SetSamplesEnabled(hWnd, nIndex, sound);
		SetStereoEnabled(hWnd, nIndex);
		SetYM3812Enabled(hWnd, nIndex);
	}

	if (Button_GetCheck(GetDlgItem(hWnd, IDC_AUTOFRAMESKIP)))
		EnableWindow(GetDlgItem(hWnd, IDC_FRAMESKIP), FALSE);
	else
		EnableWindow(GetDlgItem(hWnd, IDC_FRAMESKIP), TRUE);


	// misc
	//if (nIndex == GLOBAL_OPTIONS || DriverHasOptionalBIOS(nIndex))
	if (g_biosinfo)
	{
		ShowWindow(GetDlgItem(hWnd,IDC_BIOSTEXT), SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,IDC_BIOS), SW_SHOW);
	}
	else
	{
		ShowWindow(GetDlgItem(hWnd,IDC_BIOSTEXT), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_BIOS), SW_HIDE);
	}

//ks hcmame s switch m68k core
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020)
	if (nIndex != GLOBAL_OPTIONS && nIndex != FOLDER_OPTIONS)
	{
		BOOL has_m68k = DriverHasM68K(nIndex);

		ShowWindow(GetDlgItem(hWnd, IDC_M68K_CORE), has_m68k ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, IDC_M68K_CORETEXT), has_m68k ? SW_SHOW : SW_HIDE);
	}
#else
	ShowWindow(GetDlgItem(hWnd, IDC_M68K_CORE), SW_HIDE);
	ShowWindow(GetDlgItem(hWnd, IDC_M68K_CORETEXT), SW_HIDE);
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020) */
//ks hcmame e switch m68k core

	// BIOS
	if (nIndex == GLOBAL_OPTIONS)
	{
		for (i = 0; i < MAX_SYSTEM_BIOS; i++)
		{
			const struct GameDriver *drv = GetSystemBiosInfo(i);

			if (drv)
			{
				Static_SetText(GetDlgItem(hWnd, IDC_BIOSTEXT1 + i), _Unicode(drv->description));

				ShowWindow(GetDlgItem(hWnd,IDC_BIOSTEXT1 + i), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd,IDC_BIOS1 + i), SW_SHOW);
			}
			else
			{
				ShowWindow(GetDlgItem(hWnd,IDC_BIOSTEXT1 + i), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd,IDC_BIOS1 + i), SW_HIDE);
			}
		}
	}
	
#ifdef USE_PSXPLUGIN
	{
		//BOOL bGpuEnabled = FALSE;
		
		//if( g_nGame < 0 || ((g_nGame>=0) && DriverUsesPSXCpu(g_nGame))) {
		//	EnableWindow(GetDlgItem(hWnd, IDC_USE_GPUPLUGIN),	TRUE );
        //  
		//	hCtrl = GetDlgItem(hWnd, IDC_USE_GPUPLUGIN);
		//	if( Button_GetCheck(hCtrl) ) bGpuEnabled = TRUE;
		//}
		//else {
			// disable all psx plugins properties
		//	bGpuEnabled = FALSE;
		//	EnableWindow(GetDlgItem(hWnd, IDC_USE_GPUPLUGIN),	FALSE );
		//}
		
		//EnableWindow(GetDlgItem(hWnd, IDC_MAKE_GPUGAMEWINDOW),	bGpuEnabled );
		//EnableWindow(GetDlgItem(hWnd, IDC_COMBO_GPUPLUGIN),		bGpuEnabled );

		// PSX_GPU_EXTRA
		if( g_nGame < 0 || ((g_nGame>=0) && DriverUsesPSXCpu(g_nGame))) 
		{
			EnableWindow(GetDlgItem(hWnd, IDC_COMBO_PSXGPU_SCREENSIZE),	TRUE );
			EnableWindow(GetDlgItem(hWnd, IDC_PSXGPU_FULLSCREEN),		TRUE );
			EnableWindow(GetDlgItem(hWnd, IDC_PSXGPU_SHOWFPS),			TRUE );
			EnableWindow(GetDlgItem(hWnd, IDC_PSXGPU_SCANLINE),			TRUE );
			EnableWindow(GetDlgItem(hWnd, IDC_PSXGPU_BLENDING),			TRUE );
		}
	}
#endif 
}

/**************************************************************
 * Control Helper functions for data exchange
 **************************************************************/

static void AssignSampleRate(HWND hWnd)
{
	switch (g_nSampleRateIndex)
	{
		case 0:  pGameOpts->samplerate = 11025; break;
		case 1:  pGameOpts->samplerate = 22050; break;
		case 2:  pGameOpts->samplerate = 24000; break;
		case 3:	 pGameOpts->samplerate = 44100; break;
		case 4:  pGameOpts->samplerate = 48000; break;
		default: pGameOpts->samplerate = 44100; break;
	}
}

static void AssignVolume(HWND hWnd)
{
	pGameOpts->attenuation = g_nVolumeIndex - 32;
}

static void AssignBrightCorrect(HWND hWnd)
{
	/* "1.0", 0.5, 2.0 */
	pGameOpts->f_bright_correct = g_nBrightCorrectIndex / 20.0 + 0.5;
	
}

static void AssignPauseBright(HWND hWnd)
{
	/* "0.65", 0.5, 2.0 */
	pGameOpts->f_pause_bright = g_nPauseBrightIndex / 20.0 + 0.5;
	
}

static void AssignGamma(HWND hWnd)
{
	pGameOpts->f_gamma_correct = g_nGammaIndex / 20.0 + 0.5;
}

static void AssignBrightness(HWND hWnd)
{
	pGameOpts->gfx_brightness = g_nBrightnessIndex / 20.0 + 0.1;
}

static void AssignBeam(HWND hWnd)
{
	pGameOpts->f_beam = g_nBeamIndex / 20.0 + 1.0;
}

static void AssignFlicker(HWND hWnd)
{
	pGameOpts->f_flicker = g_nFlickerIndex;
}

static void AssignIntensity(HWND hWnd)
{
	pGameOpts->f_intensity = g_nIntensityIndex / 20.0 + 0.5;
}

static void AssignA2D(HWND hWnd)
{
	pGameOpts->f_a2d = g_nA2DIndex / 20.0;
}

static void AssignRotate(HWND hWnd)
{
	pGameOpts->ror = 0;
	pGameOpts->rol = 0;
	pGameOpts->norotate = 0;
	pGameOpts->auto_ror = 0;
	pGameOpts->auto_rol = 0;

	switch (g_nRotateIndex)
	{
		case 1:  pGameOpts->ror = 1; break;
		case 2:  pGameOpts->rol = 1; break;
	case 3 : pGameOpts->norotate = 1; break;
	case 4 : pGameOpts->auto_ror = 1; break;
	case 5 : pGameOpts->auto_rol = 1; break;
		default: break;
	}
}

static void AssignInput(HWND hWnd)
{
	int new_length;

	FreeIfAllocated(&pGameOpts->ctrlr);

	new_length = ComboBox_GetLBTextLen(hWnd,g_nInputIndex);
	if (new_length == CB_ERR)
	{
		dprintf("error getting text len");
		return;
	}
	pGameOpts->ctrlr = (char *)malloc(new_length + 1);
	ComboBox_GetLBTextA (hWnd, g_nInputIndex, pGameOpts->ctrlr);
	if (!strcmp(pGameOpts->ctrlr, "Standard"))
	{
		FreeIfAllocated(&pGameOpts->ctrlr);
		pGameOpts->ctrlr = strdup("");
	}
}

static void AssignLedmode(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nLedmodeIndex);

	FreeIfAllocated(&pGameOpts->ledmode);
	if (ptr != NULL)
		pGameOpts->ledmode = strdup(ptr);
}

static void AssignCleanStretch(HWND hWnd)
{
	char buf[128];

	FreeIfAllocated(&pGameOpts->clean_stretch);
	strcpy(buf, clean_stretch_name[g_nCleanStretchIndex]);
	if (*buf >= 'A')
		*buf -= 'A' - 'a';
	pGameOpts->clean_stretch = strdup(buf);
}

static void AssignEffect(HWND hWnd)
{
	const char* ptr= (const char*)ComboBox_GetItemData(hWnd, g_nEffectIndex);

	FreeIfAllocated(&pGameOpts->effect);
	if (ptr != NULL)
		pGameOpts->effect = strdup(ptr);
}

static void AssignD3DEffect(HWND hWnd)
{
	FreeIfAllocated(&pGameOpts->d3d_effect);
	pGameOpts->d3d_effect = strdup(d3d_effects_short_name[g_nD3DEffectIndex]);
}

static void AssignD3DPrescale(HWND hWnd)
{
	char buf[128];

	FreeIfAllocated(&pGameOpts->d3d_prescale);
	strcpy(buf, d3d_prescale_name[g_nD3DPrescaleIndex]);
	if (*buf >= 'A')
		*buf -= 'A' - 'a';
	pGameOpts->d3d_prescale = strdup(buf);
}

static void AssignBios(HWND hWnd)
{
	FreeIfAllocated(&pGameOpts->bios);

	if (g_biosinfo && g_nBiosIndex)
		pGameOpts->bios = strdup(g_biosinfo[g_nBiosIndex]._name);
	else
		pGameOpts->bios = strdup(BIOS_DEFAULT);
}

#define AssignDefaultBios(i) \
static void AssignDefaultBios##i(HWND hWnd) \
{ \
	const struct GameDriver *drv = GetSystemBiosInfo(i); \
 \
	if (drv) \
	{ \
		if (default_bios_index[i]) \
			SetDefaultBios(i, drv->bios[default_bios_index[i]]._name); \
		else \
			SetDefaultBios(i, BIOS_DEFAULT); \
	} \
}

AssignDefaultBios(0)
AssignDefaultBios(1)
AssignDefaultBios(2)
AssignDefaultBios(3)
AssignDefaultBios(4)
AssignDefaultBios(5)
AssignDefaultBios(6)
AssignDefaultBios(7)

#ifdef ROM_PATCH
static void AssignIPS(HWND hWnd)
{
	int n;

	FreeIfAllocated(&pGameOpts->patchname);

	for (n = 0; n < PATCH_MAX; n++)
		if (g_nIpsIndex[n])
		{
			char patchName[64];

			if (GetPatchName(patchName, drivers[g_nGame]->name, g_nIpsIndex[n] - 1))
			{
				if (pGameOpts->patchname)
				{
					char *p;
					int len = strlen(pGameOpts->patchname);

					len += strlen(patchName) + 2;
					p = malloc(len);
					sprintf(p, "%s,%s", pGameOpts->patchname, patchName);
					FreeIfAllocated(&pGameOpts->patchname);
					pGameOpts->patchname = p;
				}
				else
				{
					pGameOpts->patchname = strdup(patchName);
				}
			}
		}
}
#endif /* ROM_PATCH */

#ifdef USE_SCALE_EFFECTS
static void AssignScaleEffect(HWND hWnd)
{
	FreeIfAllocated(&pGameOpts->scale_effect);
	pGameOpts->scale_effect = strdup(scale_effects_short_name[g_nScaleEffectIndex]);
}
#endif /* USE_SCALE_EFFECTS */

#ifdef USE_VIRTUAL_3D_VIEW
static void AssignD3DZoomx(HWND hWnd)
{
	if (g_d3d_virtual3d_enable)
	{
		pGameOpts->d3d_zoomx = g_d3d_nZoomxIndex / 10.0 - 10.0;
	}
	else
	{
		pGameOpts->d3d_zoomx = 1.3;
	}
}

static void AssignD3DZoomy(HWND hWnd)
{
	if (g_d3d_virtual3d_enable)
	{
		pGameOpts->d3d_zoomy = g_d3d_nZoomyIndex / 10.0 - 10.0;
	}
	else
	{
		pGameOpts->d3d_zoomy = 1.0;
	}
}
#endif /* USE_VIRTUAL_3D_VIEW */
#ifdef TRANS_UI
static void AssignUI_TRANSPARENCY(HWND hWnd)
{
	pGameOpts->ui_transparency = g_nUITransparencyIndex;
}
#endif /* TRANS_UI */

/************************************************************
 * DataMap initializers
 ************************************************************/

/* Initialize local helper variables */
static void ResetDataMap(void)
{
	int i;

	// add the 0.001 to make sure it truncates properly to the integer
	// (we don't want 35.99999999 to be cut down to 35 because of floating point error)
	g_nGammaIndex           = (int)((pGameOpts->f_gamma_correct  - 0.5) * 20.0 + 0.001);
	g_nBrightnessIndex      = (int)((pGameOpts->gfx_brightness   - 0.1) * 20.0 + 0.001);
	g_nBrightCorrectIndex   = (int)((pGameOpts->f_bright_correct - 0.5) * 20.0 + 0.001);
	g_nPauseBrightIndex     = (int)((pGameOpts->f_pause_bright   - 0.5) * 20.0 + 0.001);
	g_nBeamIndex            = (int)((pGameOpts->f_beam           - 1.0) * 20.0 + 0.001);
	g_nFlickerIndex         = (int)(pGameOpts->f_flicker);
	g_nIntensityIndex       = (int)((pGameOpts->f_intensity      - 0.5) * 20.0 + 0.001);
	g_nA2DIndex             = (int)(pGameOpts->f_a2d                    * 20.0 + 0.001);
#ifdef TRANS_UI
	g_nUITransparencyIndex = (int)(pGameOpts->ui_transparency);
#endif /* TRANS_UI */

#ifdef USE_VIRTUAL_3D_VIEW
	g_d3d_virtual3d_enable = pGameOpts->d3d_virtual3d;
	if (!g_d3d_virtual3d_enable)
	{
		int u = 1;
		int d = 2;
		char buffer[200];

		snprintf(buffer,sizeof(buffer),"%d:%d",u,d);
		FreeIfAllocated(&pGameOpts->d3d_keystone);
		pGameOpts->d3d_keystone = strdup(buffer);
		pGameOpts->d3d_rotation = 0;
	}
	g_d3d_nZoomxIndex = (int)((pGameOpts->d3d_zoomx + 10.0) * 10.0 + 0.001);
	g_d3d_nZoomyIndex = (int)((pGameOpts->d3d_zoomy + 10.0) * 10.0 + 0.001);
#endif /* USE_VIRTUAL_3D_VIEW */

#ifdef USE_VIDEO_MEMORY
	if (pGameOpts->scale_effect <= SCALE_EFFECT_MEMORY)
		pGameOpts->use_videomemory = 0;
#endif /* USE_VIDEO_MEMORY */

	// if no controller type was specified or it was standard
	if (pGameOpts->ctrlr == NULL || stricmp(pGameOpts->ctrlr,"Standard") == 0)
	{
		FreeIfAllocated(&pGameOpts->ctrlr);
		pGameOpts->ctrlr = strdup("");
	}

	g_nRotateIndex = 0;
	if (pGameOpts->ror == TRUE && pGameOpts->rol == FALSE)
		g_nRotateIndex = 1;
	if (pGameOpts->ror == FALSE && pGameOpts->rol == TRUE)
		g_nRotateIndex = 2;
	if (pGameOpts->norotate)
		g_nRotateIndex = 3;
	if (pGameOpts->auto_ror)
		g_nRotateIndex = 4;
	if (pGameOpts->auto_rol)
		g_nRotateIndex = 5;

	g_nVolumeIndex = pGameOpts->attenuation + 32;
	switch (pGameOpts->samplerate)
	{
		case 11025:  g_nSampleRateIndex = 0; break;
		case 22050:  g_nSampleRateIndex = 1; break;
		case 24000:  g_nSampleRateIndex = 2; break;
		default:
		case 44100:  g_nSampleRateIndex = 3; break;
		case 48000:  g_nSampleRateIndex = 4; break;
	}

	g_nCleanStretchIndex = 0;
	for (i = 0; i < NUMCLEANSTRETCH; i++)
	{
		if (!stricmp(pGameOpts->clean_stretch, clean_stretch_name[i]))
			g_nCleanStretchIndex = i;
	}

	g_nEffectIndex = 0;
	for (i = 0; i < NUMEFFECTS; i++)
	{
		if (!stricmp(pGameOpts->effect, g_ComboBoxEffect[i].m_pData))
			g_nEffectIndex = i;
	}

	g_nD3DEffectIndex = 0;
	for (i = 0; i < NUMD3DEFFECTS; i++)
	{
		if (!stricmp(pGameOpts->d3d_effect, d3d_effects_short_name[i]))
			g_nD3DEffectIndex = i;
	}

	g_nD3DPrescaleIndex = 0;
	for (i = 0; i < NUMD3DPRESCALE; i++)
	{
		if (!stricmp(pGameOpts->d3d_prescale, d3d_prescale_name[i]))
			g_nD3DPrescaleIndex = i;
	}

	g_nLedmodeIndex = 0;
	for (i = 0; i < NUMLEDMODES; i++)
	{
		if (!stricmp(pGameOpts->ledmode, g_ComboBoxLedmode[i].m_pData))
			g_nLedmodeIndex = i;
	}

	g_biosinfo = NULL;
	if (IS_GAME)
		g_biosinfo = drivers[g_nGame]->bios;
	else if (IS_FOLDER)
	{
		for (i = 0; drivers[i]; i++)
			if (!strcmp(GetDriverFilename(i), g_pFolder) && drivers[i]->bios)
			{
				g_biosinfo = drivers[i]->bios;
				break;
			}
	}

	g_nBiosIndex = 0;
	if (g_biosinfo)
	{
		options.bios = pGameOpts->bios;
		g_nBiosIndex = determine_bios_rom(g_biosinfo);
	}

	if (IS_GLOBAL)
	{
		for (i = 0; i < MAX_SYSTEM_BIOS; i++)
		{
			const struct GameDriver *drv = GetSystemBiosInfo(i);

			if (drv)
			{
				options.bios = strdup(GetDefaultBios(i));
				default_bios_index[i] = determine_bios_rom(drv->bios);
				free(options.bios);
				options.bios = NULL;
			}
		}
	}

#ifdef ROM_PATCH
	for (i = 0; i < PATCH_MAX; i++)
		g_nIpsIndex[i] = 0;

	if (IS_GAME && pGameOpts->patchname)
	{
		char *s = strdup(pGameOpts->patchname);
		int patch_count = HasPatch(drivers[g_nGame]->name, "*");
		char *patch_list = malloc(64 * patch_count);
		int n = 0;
		char *p, *q;

		for (i = 0; i < patch_count; i++)
			if (!GetPatchName(patch_list + i * 64, drivers[g_nGame]->name, i))
				patch_list[i * 64] = '\0';

		for (p = s; *p; p = q)
		{

			for (q = p; *q; q++)
				if (*q == ',')
				{
					*q++ = '\0';
					break;
				}

			for (i = 0; i < patch_count; i++)
				if (stricmp(p, patch_list + i * 64) == 0)
				{
					g_nIpsIndex[n++] = i + 1;
				}
		}

		free(patch_list);
		free(s);
	}
#endif /* ROM_PATCH */

#ifdef USE_SCALE_EFFECTS
	g_nScaleEffectIndex = 0;
	for (i = 0; i < NUMSCALEEFFECTS; i++)
	{
		if (!stricmp(pGameOpts->scale_effect, scale_effects_short_name[i]))
			g_nScaleEffectIndex = i;
	}
#endif /* USE_SCALE_EFFECTS */

}

/* Build the control mapping by adding all needed information to the DataMap */
static void BuildDataMap(void)
{
	InitDataMap();


	ResetDataMap();

	/* video */
	DataMapAdd(IDC_AUTOFRAMESKIP, DM_BOOL, CT_BUTTON,   &pGameOpts->autoframeskip, DM_BOOL, &pGameOpts->autoframeskip, 0, 0, 0);
	DataMapAdd(IDC_FRAMESKIP,     DM_INT,  CT_COMBOBOX, &pGameOpts->frameskip,     DM_INT,  &pGameOpts->frameskip,     0, 0, 0);
	DataMapAdd(IDC_WAITVSYNC,     DM_BOOL, CT_BUTTON,   &pGameOpts->wait_vsync,    DM_BOOL, &pGameOpts->wait_vsync,    0, 0, 0);
	DataMapAdd(IDC_TRIPLE_BUFFER, DM_BOOL, CT_BUTTON,   &pGameOpts->use_triplebuf, DM_BOOL, &pGameOpts->use_triplebuf, 0, 0, 0);
	DataMapAdd(IDC_WINDOWED,      DM_BOOL, CT_BUTTON,   &pGameOpts->window_mode,   DM_BOOL, &pGameOpts->window_mode,   0, 0, 0);
	DataMapAdd(IDC_DDRAW,         DM_BOOL, CT_BUTTON,   &pGameOpts->use_ddraw,     DM_BOOL, &pGameOpts->use_ddraw,     0, 0, 0);
	DataMapAdd(IDC_HWSTRETCH,     DM_BOOL, CT_BUTTON,   &pGameOpts->ddraw_stretch, DM_BOOL, &pGameOpts->ddraw_stretch, 0, 0, 0);
	DataMapAdd(IDC_REFRESH,       DM_NONE, CT_NONE,     &pGameOpts->gfx_refresh,   DM_INT,  &pGameOpts->gfx_refresh,   0, 0, 0);
	DataMapAdd(IDC_SCANLINES,     DM_BOOL, CT_BUTTON,   &pGameOpts->scanlines,     DM_BOOL, &pGameOpts->scanlines,     0, 0, 0);
	DataMapAdd(IDC_SWITCHRES,     DM_BOOL, CT_BUTTON,   &pGameOpts->switchres,     DM_BOOL, &pGameOpts->switchres,     0, 0, 0);
	DataMapAdd(IDC_SWITCHBPP,     DM_BOOL, CT_BUTTON,   &pGameOpts->switchbpp,     DM_BOOL, &pGameOpts->switchbpp,     0, 0, 0);
	DataMapAdd(IDC_MAXIMIZE,      DM_BOOL, CT_BUTTON,   &pGameOpts->maximize,      DM_BOOL, &pGameOpts->maximize,      0, 0, 0);
	DataMapAdd(IDC_KEEPASPECT,    DM_BOOL, CT_BUTTON,   &pGameOpts->keepaspect,    DM_BOOL, &pGameOpts->keepaspect,    0, 0, 0);
	DataMapAdd(IDC_MATCHREFRESH,  DM_BOOL, CT_BUTTON,   &pGameOpts->matchrefresh,  DM_BOOL, &pGameOpts->matchrefresh,  0, 0, 0);
	DataMapAdd(IDC_SYNCREFRESH,   DM_BOOL, CT_BUTTON,   &pGameOpts->syncrefresh,   DM_BOOL, &pGameOpts->syncrefresh,   0, 0, 0);
	DataMapAdd(IDC_THROTTLE,      DM_BOOL, CT_BUTTON,   &pGameOpts->throttle,      DM_BOOL, &pGameOpts->throttle,      0, 0, 0);
	DataMapAdd(IDC_BRIGHTNESS,    DM_INT,  CT_SLIDER,   &g_nBrightnessIndex,       DM_FLOAT, &pGameOpts->gfx_brightness, 0, 0, AssignBrightness);
	DataMapAdd(IDC_BRIGHTNESSDISP,DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->gfx_brightness, 0, 0, 0);
	/* pGameOpts->frames_to_display */
	DataMapAdd(IDC_EFFECT,        DM_INT,  CT_COMBOBOX, &g_nEffectIndex,           DM_STRING, &pGameOpts->effect,	   0, 0, AssignEffect);
	DataMapAdd(IDC_ASPECTRATIOD,  DM_NONE, CT_NONE, &pGameOpts->aspect,            DM_STRING, &pGameOpts->aspect,      0, 0, 0);
	DataMapAdd(IDC_ASPECTRATION,  DM_NONE, CT_NONE, &pGameOpts->aspect,            DM_STRING, &pGameOpts->aspect,      0, 0, 0);
	DataMapAdd(IDC_SIZES,         DM_NONE, CT_NONE, &pGameOpts->resolution,        DM_STRING, &pGameOpts->resolution,  0, 0, 0);
	DataMapAdd(IDC_RESDEPTH,      DM_NONE, CT_NONE, &pGameOpts->resolution,        DM_STRING, &pGameOpts->resolution,  0, 0, 0);
	DataMapAdd(IDC_CLEAN_STRETCH, DM_INT,  CT_COMBOBOX, &g_nCleanStretchIndex,     DM_STRING, &pGameOpts->clean_stretch, 0, 0, AssignCleanStretch);
	DataMapAdd(IDC_ZOOM,          DM_INT,  CT_SLIDER,   &pGameOpts->zoom,          DM_INT, &pGameOpts->zoom,      0, 0, 0);
	DataMapAdd(IDC_ZOOMDIST,      DM_NONE, CT_NONE,   NULL,          DM_INT, &pGameOpts->zoom,      0, 0, 0);
	DataMapAdd(IDC_HIGH_PRIORITY, DM_BOOL, CT_BUTTON,   &pGameOpts->high_priority, DM_BOOL, &pGameOpts->high_priority, 0, 0, 0);
#ifdef USE_SCALE_EFFECTS
	DataMapAdd(IDC_SCALEEFFECT,   DM_INT,  CT_COMBOBOX, &g_nScaleEffectIndex,      DM_STRING, &pGameOpts->scale_effect,0, 0, AssignScaleEffect);
#ifdef USE_VIDEO_MEMORY
  DataMapAdd(IDC_USE_VIDEOMEMORY, DM_BOOL, CT_BUTTON,   &pGameOpts->use_videomemory, DM_BOOL, &pGameOpts->use_videomemory, 0, 0, 0);
#endif /*USE_VIDEO_MEMORY*/
#endif /* USE_SCALE_EFFECTS */

	// direct3d
	DataMapAdd(IDC_D3D,           DM_BOOL, CT_BUTTON,   &pGameOpts->use_d3d,       DM_BOOL, &pGameOpts->use_d3d,       0, 0, 0);
	DataMapAdd(IDC_D3D_FILTER,    DM_INT,  CT_COMBOBOX, &pGameOpts->d3d_filter,    DM_INT, &pGameOpts->d3d_filter,     0, 0, 0);
	DataMapAdd(IDC_D3D_TEXTURE_MANAGEMENT,DM_BOOL,CT_BUTTON,&pGameOpts->d3d_texture_management,DM_BOOL,&pGameOpts->d3d_texture_management, 0, 0, 0);
	DataMapAdd(IDC_D3D_EFFECT,    DM_INT,  CT_COMBOBOX, &g_nD3DEffectIndex,        DM_STRING, &pGameOpts->d3d_effect,  0, 0, AssignD3DEffect);
	DataMapAdd(IDC_D3D_PRESCALE,  DM_INT,  CT_COMBOBOX, &g_nD3DPrescaleIndex,      DM_STRING, &pGameOpts->d3d_prescale,0, 0, AssignD3DPrescale);
	DataMapAdd(IDC_D3D_ROTATE_EFFECTS,DM_BOOL,CT_BUTTON,&pGameOpts->d3d_rotate_effects,DM_BOOL,&pGameOpts->d3d_rotate_effects, 0, 0, 0);
	DataMapAdd(IDC_D3D_SCANLINES_ENABLE,DM_BOOL, CT_BUTTON, &pGameOpts->d3d_scanlines_enable, DM_BOOL, &pGameOpts->d3d_scanlines_enable, 0, 0, 0);
	DataMapAdd(IDC_D3D_SCANLINES, DM_INT,  CT_SLIDER,   &pGameOpts->d3d_scanlines, DM_INT, &pGameOpts->d3d_scanlines,  0, 0, 0);
	DataMapAdd(IDC_D3D_SCANLINES_DISP, DM_NONE,  CT_NONE,   NULL, DM_INT, &pGameOpts->d3d_scanlines, 0, 0, 0);
	DataMapAdd(IDC_D3D_FEEDBACK_ENABLE,DM_BOOL, CT_BUTTON, &pGameOpts->d3d_feedback_enable, DM_BOOL, &pGameOpts->d3d_feedback_enable, 0, 0, 0);
	DataMapAdd(IDC_D3D_FEEDBACK,  DM_INT,  CT_SLIDER,   &pGameOpts->d3d_feedback,  DM_INT, &pGameOpts->d3d_feedback,   0, 0, 0);
	DataMapAdd(IDC_D3D_FEEDBACK_DISP,  DM_NONE,  CT_NONE,   NULL,  DM_INT, &pGameOpts->d3d_feedback, 0, 0, 0);
#ifdef USE_VIRTUAL_3D_VIEW
	DataMapAdd(IDC_D3D_VIRTUAL_3DVIEW, DM_BOOL, CT_BUTTON, &pGameOpts->d3d_virtual3d, DM_BOOL,   &pGameOpts->d3d_virtual3d, 0, 0, 0);
	DataMapAdd(IDC_D3D_ROTATION,       DM_INT,  CT_SLIDER, &pGameOpts->d3d_rotation,  DM_INT,    &pGameOpts->d3d_rotation, 0, 0, 0);
	DataMapAdd(IDC_D3D_ROTATION_DISP,  DM_NONE, CT_NONE,   NULL,                      DM_INT,    &pGameOpts->d3d_rotation, 0, 0, 0);
	DataMapAdd(IDC_D3D_ZOOMX,          DM_INT,  CT_SLIDER, &g_d3d_nZoomxIndex,        DM_FLOAT,    &pGameOpts->d3d_zoomx, 0, 0, AssignD3DZoomx);
	DataMapAdd(IDC_D3D_ZOOMX_DISP,     DM_NONE, CT_NONE,   NULL,                      DM_FLOAT, &pGameOpts->d3d_zoomx, 0, 0, 0);
	DataMapAdd(IDC_D3D_ZOOMY,          DM_INT,  CT_SLIDER, &g_d3d_nZoomyIndex,        DM_FLOAT, &pGameOpts->d3d_zoomy, 0, 0, AssignD3DZoomy);
	DataMapAdd(IDC_D3D_ZOOMY_DISP,     DM_NONE, CT_NONE,   NULL,                      DM_FLOAT, &pGameOpts->d3d_zoomy, 0, 0, 0);
	DataMapAdd(IDC_D3D_KEYSTONERATIOU, DM_NONE, CT_NONE,   &pGameOpts->d3d_keystone,  DM_STRING, &pGameOpts->d3d_keystone, 0, 0, 0);
	DataMapAdd(IDC_D3D_KEYSTONERATIOD, DM_NONE, CT_NONE,   &pGameOpts->d3d_keystone,  DM_STRING, &pGameOpts->d3d_keystone, 0, 0, 0);
#endif /* USE_VIRTUAL_3D_VIEW */

	/* input */
	DataMapAdd(IDC_DEFAULT_INPUT, DM_INT,  CT_COMBOBOX, &g_nInputIndex,            DM_STRING, &pGameOpts->ctrlr,       0, 0, AssignInput);
	DataMapAdd(IDC_USE_MOUSE,     DM_BOOL, CT_BUTTON,   &pGameOpts->use_mouse,     DM_BOOL,   &pGameOpts->use_mouse,   0, 0, 0);   
#ifdef USE_JOY_MOUSE_MOVE // Support Stick-type Pointing Device by miko2u@hotmail.com
	DataMapAdd(IDC_USE_STICKPOINT,DM_BOOL, CT_BUTTON,   &pGameOpts->use_stickpoint,DM_BOOL,   &pGameOpts->use_stickpoint, 0, 0, 0);
#endif /* USE_JOY_MOUSE_MOVE */
	DataMapAdd(IDC_JOYSTICK,      DM_BOOL, CT_BUTTON,   &pGameOpts->use_joystick,  DM_BOOL,   &pGameOpts->use_joystick,0, 0, 0);
	DataMapAdd(IDC_LEDMODE,       DM_INT,  CT_COMBOBOX, &g_nLedmodeIndex,          DM_STRING, &pGameOpts->ledmode,     0, 0, AssignLedmode);
	DataMapAdd(IDC_A2D,           DM_INT,  CT_SLIDER,   &g_nA2DIndex,              DM_FLOAT,  &pGameOpts->f_a2d,       0, 0, AssignA2D);
	DataMapAdd(IDC_A2DDISP,       DM_NONE, CT_NONE,     NULL,                      DM_FLOAT,  &pGameOpts->f_a2d,       0, 0, 0);
	DataMapAdd(IDC_STEADYKEY,     DM_BOOL, CT_BUTTON,   &pGameOpts->steadykey,     DM_BOOL,   &pGameOpts->steadykey,   0, 0, 0);
	DataMapAdd(IDC_LIGHTGUN,      DM_BOOL, CT_BUTTON,   &pGameOpts->lightgun,      DM_BOOL,   &pGameOpts->lightgun,    0, 0, 0);
	DataMapAdd(IDC_DUAL_LIGHTGUN, DM_BOOL, CT_BUTTON,   &pGameOpts->dual_lightgun, DM_BOOL,   &pGameOpts->dual_lightgun, 0, 0, 0);
	DataMapAdd(IDC_RELOAD,DM_BOOL, CT_BUTTON,  &pGameOpts->offscreen_reload,DM_BOOL, &pGameOpts->offscreen_reload, 0, 0, 0);
#ifdef JOYSTICK_ID
	DataMapAdd(IDC_JOYID1,        DM_INT,  CT_COMBOBOX, &pGameOpts->joyid[0],      DM_INT, &pGameOpts->joyid[0], 0, 0, 0);
	DataMapAdd(IDC_JOYID2,        DM_INT,  CT_COMBOBOX, &pGameOpts->joyid[1],      DM_INT, &pGameOpts->joyid[1], 0, 0, 0);
	DataMapAdd(IDC_JOYID3,        DM_INT,  CT_COMBOBOX, &pGameOpts->joyid[2],      DM_INT, &pGameOpts->joyid[2], 0, 0, 0);
	DataMapAdd(IDC_JOYID4,        DM_INT,  CT_COMBOBOX, &pGameOpts->joyid[3],      DM_INT, &pGameOpts->joyid[3], 0, 0, 0);
	DataMapAdd(IDC_JOYID5,        DM_INT,  CT_COMBOBOX, &pGameOpts->joyid[4],      DM_INT, &pGameOpts->joyid[4], 0, 0, 0);
	DataMapAdd(IDC_JOYID6,        DM_INT,  CT_COMBOBOX, &pGameOpts->joyid[5],      DM_INT, &pGameOpts->joyid[5], 0, 0, 0);
	DataMapAdd(IDC_JOYID7,        DM_INT,  CT_COMBOBOX, &pGameOpts->joyid[6],      DM_INT, &pGameOpts->joyid[6], 0, 0, 0);
	DataMapAdd(IDC_JOYID8,        DM_INT,  CT_COMBOBOX, &pGameOpts->joyid[7],      DM_INT, &pGameOpts->joyid[7], 0, 0, 0);
#endif /* JOYSTICK_ID */

	/* core video */
	DataMapAdd(IDC_BRIGHTCORRECT,    DM_INT,  CT_SLIDER,   &g_nBrightCorrectIndex,    DM_FLOAT, &pGameOpts->f_bright_correct, 0, 0, AssignBrightCorrect);
	DataMapAdd(IDC_BRIGHTCORRECTDISP,DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->f_bright_correct, 0, 0, 0);
	DataMapAdd(IDC_PAUSEBRIGHT,      DM_INT,  CT_SLIDER,   &g_nPauseBrightIndex,      DM_FLOAT, &pGameOpts->f_pause_bright,   0, 0, AssignPauseBright);
	DataMapAdd(IDC_PAUSEBRIGHTDISP,  DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->f_pause_bright,   0, 0, 0);
	DataMapAdd(IDC_ROTATE,           DM_INT,  CT_COMBOBOX, &g_nRotateIndex,           DM_INT,   &pGameOpts->ror,              0, 0, AssignRotate);
	DataMapAdd(IDC_FLIPX,            DM_BOOL, CT_BUTTON,   &pGameOpts->flipx,         DM_BOOL,  &pGameOpts->flipx,            0, 0, 0);
	DataMapAdd(IDC_FLIPY,            DM_BOOL, CT_BUTTON,   &pGameOpts->flipy,         DM_BOOL,  &pGameOpts->flipy,            0, 0, 0);
	/* debugres */
	DataMapAdd(IDC_GAMMA,         DM_INT,  CT_SLIDER,   &g_nGammaIndex,            DM_FLOAT, &pGameOpts->f_gamma_correct,  0, 0, AssignGamma);
	DataMapAdd(IDC_GAMMADISP,     DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->f_gamma_correct,  0, 0, 0);

	/* vector */
	DataMapAdd(IDC_ANTIALIAS,     DM_BOOL, CT_BUTTON,   &pGameOpts->antialias,     DM_BOOL,  &pGameOpts->antialias,        0, 0, 0);
	DataMapAdd(IDC_TRANSLUCENCY,  DM_BOOL, CT_BUTTON,   &pGameOpts->translucency,  DM_BOOL,  &pGameOpts->translucency,     0, 0, 0);
	DataMapAdd(IDC_BEAM,          DM_INT,  CT_SLIDER,   &g_nBeamIndex,             DM_FLOAT, &pGameOpts->f_beam,           0, 0, AssignBeam);
	DataMapAdd(IDC_BEAMDISP,      DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->f_beam,           0, 0, 0);
	DataMapAdd(IDC_FLICKER,       DM_INT,  CT_SLIDER,   &g_nFlickerIndex,          DM_FLOAT, &pGameOpts->f_flicker,        0, 0, AssignFlicker);
	DataMapAdd(IDC_FLICKERDISP,   DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->f_flicker,        0, 0, 0);
	DataMapAdd(IDC_INTENSITY,     DM_INT,  CT_SLIDER,   &g_nIntensityIndex,        DM_FLOAT, &pGameOpts->f_intensity,      0, 0, AssignIntensity);	
	DataMapAdd(IDC_INTENSITYDISP, DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->f_intensity,      0, 0, 0);

	/* sound */
	DataMapAdd(IDC_SAMPLERATE,    DM_INT,  CT_COMBOBOX, &g_nSampleRateIndex,       DM_INT,  &pGameOpts->samplerate,    0, 0, AssignSampleRate);
	DataMapAdd(IDC_SAMPLES,       DM_BOOL, CT_BUTTON,   &pGameOpts->use_samples,   DM_BOOL, &pGameOpts->use_samples,   0, 0, 0);
	DataMapAdd(IDC_USE_FILTER,    DM_BOOL, CT_BUTTON,   &pGameOpts->use_filter,    DM_BOOL, &pGameOpts->use_filter,    0, 0, 0);
	DataMapAdd(IDC_USE_SOUND,     DM_BOOL, CT_BUTTON,   &pGameOpts->enable_sound,  DM_BOOL, &pGameOpts->enable_sound,  0, 0, 0);
	DataMapAdd(IDC_VOLUME,        DM_INT,  CT_SLIDER,   &g_nVolumeIndex,           DM_INT,  &pGameOpts->attenuation,   0, 0, AssignVolume);
	DataMapAdd(IDC_VOLUMEDISP,    DM_NONE, CT_NONE,     NULL,                      DM_INT,  &pGameOpts->attenuation,   0, 0, 0);
#ifdef USE_VOLUME_AUTO_ADJUST
	DataMapAdd(IDC_VOLUME_ADJUST, DM_BOOL, CT_BUTTON,   &pGameOpts->use_volume_adjust, DM_BOOL, &pGameOpts->use_volume_adjust, 0, 0, 0);
#endif /* USE_VOLUME_AUTO_ADJUST */
	DataMapAdd(IDC_AUDIO_LATENCY, DM_INT,  CT_SLIDER,   &pGameOpts->audio_latency, DM_INT,  &pGameOpts->audio_latency, 0, 0, 0);
	DataMapAdd(IDC_AUDIO_LATENCY_DISP, DM_NONE,  CT_NONE,   NULL, DM_INT, &pGameOpts->audio_latency, 0, 0, 0);

	/* misc artwork options */
	DataMapAdd(IDC_ARTWORK,       DM_BOOL, CT_BUTTON,   &pGameOpts->use_artwork,   DM_BOOL, &pGameOpts->use_artwork,   0, 0, 0);
	DataMapAdd(IDC_BACKDROPS,     DM_BOOL, CT_BUTTON,   &pGameOpts->backdrops,     DM_BOOL, &pGameOpts->backdrops,     0, 0, 0);
	DataMapAdd(IDC_OVERLAYS,      DM_BOOL, CT_BUTTON,   &pGameOpts->overlays,      DM_BOOL, &pGameOpts->overlays,      0, 0, 0);
	DataMapAdd(IDC_BEZELS,        DM_BOOL, CT_BUTTON,   &pGameOpts->bezels,        DM_BOOL, &pGameOpts->bezels,        0, 0, 0);
	DataMapAdd(IDC_ARTRES,        DM_INT,  CT_COMBOBOX, &pGameOpts->artres,        DM_INT,  &pGameOpts->artres,        0, 0, 0);
	DataMapAdd(IDC_ARTWORK_CROP,  DM_BOOL, CT_BUTTON,   &pGameOpts->artwork_crop,  DM_BOOL, &pGameOpts->artwork_crop,  0, 0, 0);

	/* misc */
	DataMapAdd(IDC_CHEAT,         DM_BOOL, CT_BUTTON,   &pGameOpts->cheat,         DM_BOOL, &pGameOpts->cheat,         0, 0, 0);
/*	DataMapAdd(IDC_DEBUG,         DM_BOOL, CT_BUTTON,   &pGameOpts->mame_debug,    DM_BOOL, &pGameOpts->mame_debug,    0, 0, 0);*/
	DataMapAdd(IDC_LOG,           DM_BOOL, CT_BUTTON,   &pGameOpts->errorlog,      DM_BOOL, &pGameOpts->errorlog,      0, 0, 0);
	DataMapAdd(IDC_SLEEP,         DM_BOOL, CT_BUTTON,   &pGameOpts->sleep,         DM_BOOL, &pGameOpts->sleep,         0, 0, 0);
	DataMapAdd(IDC_OLD_TIMING,    DM_BOOL, CT_BUTTON,   &pGameOpts->old_timing,    DM_BOOL, &pGameOpts->old_timing,    0, 0, 0);
	DataMapAdd(IDC_LEDS,          DM_BOOL, CT_BUTTON,   &pGameOpts->leds,          DM_BOOL, &pGameOpts->leds,          0, 0, 0);
	DataMapAdd(IDC_BIOS,          DM_INT,  CT_COMBOBOX, &g_nBiosIndex,             DM_STRING, &pGameOpts->bios,        0, 0, AssignBios);
	DataMapAdd(IDC_SKIP_DISCLAIMER, DM_BOOL,CT_BUTTON,  &pGameOpts->skip_disclaimer, DM_BOOL, &pGameOpts->skip_disclaimer, 0, 0, 0);
	DataMapAdd(IDC_SKIP_GAME_INFO,  DM_BOOL,CT_BUTTON,  &pGameOpts->skip_gameinfo, DM_BOOL, &pGameOpts->skip_gameinfo, 0, 0, 0);
	DataMapAdd(IDC_CONFIRM_QUIT,  DM_BOOL, CT_BUTTON,   &pGameOpts->confirm_quit,  DM_BOOL, &pGameOpts->confirm_quit,  0, 0, 0);
	DataMapAdd(IDC_SKIP_VALIDITY_CHECKS,  DM_BOOL, CT_BUTTON,   &pGameOpts->skip_validitychecks,  DM_BOOL, &pGameOpts->skip_validitychecks,  0, 0, 0);
	DataMapAdd(IDC_CRC_ONLY,  DM_BOOL, CT_BUTTON,   &pGameOpts->crc_only,  DM_BOOL, &pGameOpts->crc_only,  0, 0, 0);
#ifdef TRANS_UI
	DataMapAdd(IDC_TRANSUI,       DM_BOOL, CT_BUTTON,   &pGameOpts->use_transui,   DM_BOOL, &pGameOpts->use_transui,   0, 0, 0);
	DataMapAdd(IDC_TRANSPARENCY,  DM_INT,  CT_SLIDER,   &g_nUITransparencyIndex,   DM_INT,  &pGameOpts->ui_transparency, 0, 0, AssignUI_TRANSPARENCY);
	DataMapAdd(IDC_TRANSPARENCYDISP, DM_NONE,  CT_NONE,   NULL, DM_INT,  &pGameOpts->ui_transparency, 0, 0, 0);
#endif /* TRANS_UI */
//ks hcmame s switch m68k core
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020)
	DataMapAdd(IDC_M68K_CORE,     DM_INT,  CT_COMBOBOX, &pGameOpts->m68k_core,     DM_INT,  &pGameOpts->m68k_core,     0, 0, 0);
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020) */
//ks hcmame e switch m68k core

#ifdef MENU_TRANS
	DataMapAdd(IDC_PLUS_MENUTRANSPARENCY,	DM_INT,  CT_SLIDER,   &pGameOpts->menu_trans,       DM_INT, &pGameOpts->menu_trans,			0, 0, 0);
#endif
#ifdef LOCK60FPS
  DataMapAdd(IDC_PLUS_FIX60FPS,			DM_BOOL, CT_BUTTON,   &pGameOpts->bLock60Fps,		DM_BOOL, &pGameOpts->bLock60Fps,		0, 0, 0);
#endif /*LOCK60FPS*/

	/* BIOS */
	if (IS_GLOBAL)
	{
		if (GetSystemBiosInfo(0))
			DataMapAdd(IDC_BIOS1,         DM_INT,  CT_COMBOBOX, &default_bios_index[0],    DM_NONE, NULL,                      0, 0, AssignDefaultBios0);
		if (GetSystemBiosInfo(1))
			DataMapAdd(IDC_BIOS2,         DM_INT,  CT_COMBOBOX, &default_bios_index[1],    DM_NONE, NULL,                      0, 0, AssignDefaultBios1);
		if (GetSystemBiosInfo(2))
			DataMapAdd(IDC_BIOS3,         DM_INT,  CT_COMBOBOX, &default_bios_index[2],    DM_NONE, NULL,                      0, 0, AssignDefaultBios2);
		if (GetSystemBiosInfo(3))
			DataMapAdd(IDC_BIOS4,         DM_INT,  CT_COMBOBOX, &default_bios_index[3],    DM_NONE, NULL,                      0, 0, AssignDefaultBios3);
		if (GetSystemBiosInfo(4))
			DataMapAdd(IDC_BIOS5,         DM_INT,  CT_COMBOBOX, &default_bios_index[4],    DM_NONE, NULL,                      0, 0, AssignDefaultBios4);
		if (GetSystemBiosInfo(5))
			DataMapAdd(IDC_BIOS6,         DM_INT,  CT_COMBOBOX, &default_bios_index[5],    DM_NONE, NULL,                      0, 0, AssignDefaultBios5);
		if (GetSystemBiosInfo(6))
			DataMapAdd(IDC_BIOS7,         DM_INT,  CT_COMBOBOX, &default_bios_index[6],    DM_NONE, NULL,                      0, 0, AssignDefaultBios6);
		if (GetSystemBiosInfo(7))
			DataMapAdd(IDC_BIOS8,         DM_INT,  CT_COMBOBOX, &default_bios_index[7],    DM_NONE, NULL,                      0, 0, AssignDefaultBios7);
	}

#ifdef ROM_PATCH
	if (IS_GAME)
	{
		int n;

		for (n = 0; n < PATCH_MAX; n++)
			DataMapAdd(IDC_IPS1 + n,      DM_INT,  CT_COMBOBOX, &g_nIpsIndex[n],           DM_NONE, NULL,                      0, 0, AssignIPS);
	}
#endif /* ROM_PATCH */

#ifdef MESS
	DataMapAdd(IDC_USE_NEW_UI,    DM_BOOL, CT_BUTTON,   &pGameOpts->mess.use_new_ui,DM_BOOL, &pGameOpts->mess.use_new_ui, 0, 0, 0);
#endif
#ifdef USE_PSXPLUGIN
	//DataMapAdd(IDC_USE_GPUPLUGIN,		DM_BOOL, CT_BUTTON,   &pGameOpts->bUseGpuPlugIn,		DM_BOOL, &pGameOpts->bUseGpuPlugIn,		0, 0, 0);
	//DataMapAdd(IDC_MAKE_GPUGAMEWINDOW,	DM_BOOL, CT_BUTTON,   &pGameOpts->bMakeGpuGameWindow,	DM_BOOL, &pGameOpts->bMakeGpuGameWindow,0, 0, 0);
	
	// PSX_GPU_EXTRA
	DataMapAdd(IDC_PSXGPU_FULLSCREEN,	DM_BOOL, CT_BUTTON,   &pGameOpts->bPsxGpuFullScreen,	DM_BOOL, &pGameOpts->bPsxGpuFullScreen,	0, 0, 0);
	DataMapAdd(IDC_PSXGPU_SHOWFPS,		DM_BOOL, CT_BUTTON,   &pGameOpts->bPsxGpuShowFps,		DM_BOOL, &pGameOpts->bPsxGpuShowFps,	0, 0, 0);
	DataMapAdd(IDC_PSXGPU_SCANLINE,		DM_BOOL, CT_BUTTON,   &pGameOpts->bPsxGpuScanline,		DM_BOOL, &pGameOpts->bPsxGpuScanline,	0, 0, 0);
	DataMapAdd(IDC_PSXGPU_BLENDING,		DM_BOOL, CT_BUTTON,   &pGameOpts->bPsxGpuBlending,		DM_BOOL, &pGameOpts->bPsxGpuBlending,	0, 0, 0);
#endif

}

BOOL IsControlOptionValue(HWND hDlg,HWND hwnd_ctrl, options_type *opts )
{
	int control_id = GetControlID(hDlg,hwnd_ctrl);

	// certain controls we need to handle specially
	switch (control_id)
	{
	case IDC_ASPECTRATION :
	{
		int n1=0, n2=0;

		sscanf(pGameOpts->aspect,"%i",&n1);
		sscanf(opts->aspect,"%i",&n2);

		return n1 == n2;
	}
	case IDC_ASPECTRATIOD :
	{
		int temp, d1=0, d2=0;

		sscanf(pGameOpts->aspect,"%i:%i",&temp,&d1);
		sscanf(opts->aspect,"%i:%i",&temp,&d2);

		return d1 == d2;
	}
#ifdef USE_VIRTUAL_3D_VIEW
	case IDC_D3D_KEYSTONERATIOU :
	{
		int u1=0, u2=0;

		sscanf(pGameOpts->d3d_keystone,"%i",&u1);
		sscanf(opts->d3d_keystone,"%i",&u2);

		return u1 == u2;
	}
	case IDC_D3D_KEYSTONERATIOD :
	{
		int temp, d1=0, d2=0;

		sscanf(pGameOpts->d3d_keystone,"%i:%i",&temp,&d1);
		sscanf(opts->d3d_keystone,"%i:%i",&temp,&d2);

		return d1 == d2;
	}
#endif /* USE_VIRTUAL_3D_VIEW */
	case IDC_SIZES :
	{
		int xx1=0,yy1=0,xx2=0,yy2=0;

		if (strcmp(pGameOpts->resolution,"auto") != 0)
			sscanf(pGameOpts->resolution,"%d x %d",&xx1,&yy1);
		
		if (strcmp(opts->resolution,"auto") != 0)
			sscanf(opts->resolution,"%d x %d",&xx2,&yy2);

		return xx1 == xx2 && yy1 == yy2;
	}
	case IDC_RESDEPTH :
	{
		int temp,d1=0,d2=0;

		if (strcmp(pGameOpts->resolution,"auto") != 0)
			sscanf(pGameOpts->resolution,"%d x %d x %d",&temp,&temp,&d1);
		if (strcmp(opts->resolution,"auto") != 0)
			sscanf(opts->resolution,"%d x %d x %d",&temp,&temp,&d2);

		return d1 == d2;
	}
	case IDC_ROTATE :
	{
		ReadControl(hDlg,control_id);
	
		return pGameOpts->ror == opts->ror &&
			pGameOpts->rol == opts->rol;

	}
	}
	// most options we can compare using data in the data map
	if (IsControlDifferent(hDlg,hwnd_ctrl,pGameOpts,opts))
		return FALSE;

	return TRUE;
}


static void SetStereoEnabled(HWND hWnd, int nIndex)
{
	BOOL enabled = FALSE;
	HWND hCtrl;

	if (nIndex != GLOBAL_OPTIONS && nIndex != FOLDER_OPTIONS)
		enabled = DriverIsStereo(nIndex);

	hCtrl = GetDlgItem(hWnd, IDC_STEREO);
	if (hCtrl)
	{
		if (nIndex == GLOBAL_OPTIONS || nIndex == FOLDER_OPTIONS)
			enabled = TRUE;

		EnableWindow(hCtrl, enabled);
	}
}

static void SetYM3812Enabled(HWND hWnd, int nIndex)
{
	BOOL enabled = FALSE;
	HWND hCtrl;

	if (nIndex != GLOBAL_OPTIONS && nIndex != FOLDER_OPTIONS)
		enabled = DriverUsesYM3812(nIndex);

	hCtrl = GetDlgItem(hWnd, IDC_USE_FM_YM3812);
	if (hCtrl)
	{
		if (nIndex == GLOBAL_OPTIONS || nIndex == FOLDER_OPTIONS)
			enabled = TRUE;

		EnableWindow(hCtrl, enabled);
	}
}

static void SetSamplesEnabled(HWND hWnd, int nIndex, BOOL bSoundEnabled)
{
	BOOL enabled = FALSE;
	HWND hCtrl;

	if (nIndex != GLOBAL_OPTIONS && nIndex != FOLDER_OPTIONS)
		enabled = DriverUsesSamples(nIndex);

	hCtrl = GetDlgItem(hWnd, IDC_SAMPLES);
	if (hCtrl)
	{
		if (nIndex == GLOBAL_OPTIONS || nIndex == FOLDER_OPTIONS)
			enabled = TRUE;

		enabled = enabled && bSoundEnabled;
		EnableWindow(hCtrl, enabled);
	}
}

/* Moved here cause it's called in a few places */
static void InitializeOptions(HWND hDlg)
{
	InitializeResDepthUI(hDlg);
	InitializeRefreshUI(hDlg);
	InitializeDisplayModeUI(hDlg);
	InitializeSoundUI(hDlg);
	InitializeSkippingUI(hDlg);
	InitializeRotateUI(hDlg);
	InitializeDefaultInputUI(hDlg);
	InitializeEffectUI(hDlg);
	InitializeArtresUI(hDlg);
	InitializeD3DFilterUI(hDlg);
	InitializeD3DEffectUI(hDlg);
	InitializeD3DPrescaleUI(hDlg);
	InitializeBIOSUI(hDlg);
	InitializeDefaultBIOSUI(hDlg);
#ifdef ROM_PATCH
	InitializeIPSUI(hDlg);
#endif /* ROM_PATCH */
	InitializeLEDModeUI(hDlg);
	InitializeCleanStretchUI(hDlg);
	InitializeM68kCoreUI(hDlg);
#ifdef USE_SCALE_EFFECTS
	InitializeScaleEffectUI(hDlg);
#endif /* USE_SCALE_EFFECTS */
#ifdef JOYSTICK_ID
	InitializeJoyidUI(hDlg);
#endif /* JOYSTICK_ID */
#ifdef MENU_TRANS
	InitializePlusUI(hDlg);
#endif
#ifdef USE_PSXPLUGIN
	InitializePSXPlugInUI(hDlg);
#endif 
}

/* Moved here because it is called in several places */
static void InitializeMisc(HWND hDlg)
{
	Button_Enable(GetDlgItem(hDlg, IDC_JOYSTICK), DIJoystick.Available());

	SendDlgItemMessage(hDlg, IDC_GAMMA, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 30)); /* [0.50, 2.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_BRIGHTCORRECT, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 30)); /* [0.50, 2.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_PAUSEBRIGHT, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 30)); /* [0.50, 2.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_BRIGHTNESS, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 38)); /* [0.10, 2.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_INTENSITY, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 50)); /* [0.50, 3.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_A2D, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 20)); /* [0.00, 1.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_FLICKER, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 100)); /* [0.0, 100.0] in 1.0 increments */

	SendDlgItemMessage(hDlg, IDC_BEAM, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 300)); /* [1.00, 16.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_VOLUME, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 32)); /* [-32, 0] */
	SendDlgItemMessage(hDlg, IDC_AUDIO_LATENCY, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(1, 4)); // [1, 4]
	SendDlgItemMessage(hDlg, IDC_D3D_SCANLINES, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 100)); // [0, 100]
	SendDlgItemMessage(hDlg, IDC_D3D_FEEDBACK, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 100)); // [0, 100]
	SendDlgItemMessage(hDlg, IDC_ZOOM, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(1, 8)); // [1, 8]
#ifdef USE_VIRTUAL_3D_VIEW
	SendDlgItemMessage(hDlg, IDC_D3D_ROTATION, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 360)); // [0, 360]

	SendDlgItemMessage(hDlg, IDC_D3D_ZOOMX, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 200)); /* [-10.0, 10.0] in .1 increments */

	SendDlgItemMessage(hDlg, IDC_D3D_ZOOMY, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 200)); /* [-10.0, 10.0] in .1 increments */
#endif /* USE_VIRTUAL_3D_VIEW */
#ifdef TRANS_UI
	SendDlgItemMessage(hDlg, IDC_TRANSPARENCY, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 255)); /* [0, 255] in 1.0 increments */
#endif /* TRANS_UI */
#ifdef MENU_TRANS
	SendMessage(GetDlgItem(hDlg, IDC_PLUS_MENUTRANSPARENCY), TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 100)); // [0, 100]
#endif
}

static void OptOnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	if (hwndCtl == GetDlgItem(hwnd, IDC_FLICKER))
	{
		FlickerSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_GAMMA))
	{
		GammaSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_BRIGHTCORRECT))
	{
		BrightCorrectSelectionChange(hwnd);
	}
	if (hwndCtl == GetDlgItem(hwnd, IDC_PAUSEBRIGHT))
	{
		PauseBrightSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_BRIGHTNESS))
	{
		BrightnessSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_BEAM))
	{
		BeamSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_FLICKER))
	{
		FlickerSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_VOLUME))
	{
		VolumeSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_INTENSITY))
	{
		IntensitySelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_A2D))
	{
		A2DSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_AUDIO_LATENCY))
	{
		AudioLatencySelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_D3D_SCANLINES))
	{
		D3DScanlinesSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_D3D_FEEDBACK))
	{
		D3DFeedbackSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_ZOOM))
	{
		ZoomSelectionChange(hwnd);
	}
#ifdef USE_VIRTUAL_3D_VIEW
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_D3D_ROTATION))
	{
		D3DRotationChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_D3D_ZOOMX))
	{
		D3DZoomxChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_D3D_ZOOMY))
	{
		D3DZoomyChange(hwnd);
	}
#endif /* USE_VIRTUAL_3D_VIEW */
#ifdef TRANS_UI
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_TRANSPARENCY))
	{
		TransparencySelectionChange(hwnd);
	}
#endif /* TRANS_UI */
#ifdef MENU_TRANS
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_PLUS_MENUTRANSPARENCY))
	{
		MenuTransSelectionChange(hwnd);
	}
#endif
}

/* Handle changes to the Beam slider */
static void BeamSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dBeam;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_BEAM, TBM_GETPOS, 0, 0);

	dBeam = nValue / 20.0 + 1.0;

	/* Set the static display to the new value */
	snprintf(buf,sizeof(buf), "%03.2f", dBeam);
	Static_SetTextA(GetDlgItem(hwnd, IDC_BEAMDISP), buf);
}

/* Handle changes to the Flicker slider */
static void FlickerSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dFlicker;
	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_FLICKER, TBM_GETPOS, 0, 0);

	dFlicker = nValue;

	/* Set the static display to the new value */
	snprintf(buf,sizeof(buf), "%03.2f", dFlicker);
	Static_SetTextA(GetDlgItem(hwnd, IDC_FLICKERDISP), buf);
}

/* Handle changes to the Gamma slider */
static void GammaSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dGamma;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_GAMMA, TBM_GETPOS, 0, 0);

	dGamma = nValue / 20.0 + 0.5;

	/* Set the static display to the new value */
	snprintf(buf,sizeof(buf), "%03.2f", dGamma);
	Static_SetTextA(GetDlgItem(hwnd, IDC_GAMMADISP), buf);
}

/* Handle changes to the Brightness Correction slider */
static void BrightCorrectSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dValue;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_BRIGHTCORRECT, TBM_GETPOS, 0, 0);

	dValue = nValue / 20.0 + 0.5;

	/* Set the static display to the new value */
	snprintf(buf,sizeof(buf), "%03.2f", dValue);
	Static_SetTextA(GetDlgItem(hwnd, IDC_BRIGHTCORRECTDISP), buf);
}

/* Handle changes to the Pause Brightness slider */
static void PauseBrightSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dValue;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_PAUSEBRIGHT, TBM_GETPOS, 0, 0);

	dValue = nValue / 20.0 + 0.5;

	/* Set the static display to the new value */
	snprintf(buf,sizeof(buf), "%03.2f", dValue);
	Static_SetTextA(GetDlgItem(hwnd, IDC_PAUSEBRIGHTDISP), buf);
}

/* Handle changes to the Brightness slider */
static void BrightnessSelectionChange(HWND hwnd)
{
	char   buf[100];
	int    nValue;
	double dBrightness;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_BRIGHTNESS, TBM_GETPOS, 0, 0);

	dBrightness = nValue / 20.0 + 0.1;

	/* Set the static display to the new value */
	snprintf(buf,sizeof(buf),"%03.2f", dBrightness);
	Static_SetTextA(GetDlgItem(hwnd, IDC_BRIGHTNESSDISP), buf);
}

/* Handle changes to the Intensity slider */
static void IntensitySelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dIntensity;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_INTENSITY, TBM_GETPOS, 0, 0);

	dIntensity = nValue / 20.0 + 0.5;

	/* Set the static display to the new value */
	snprintf(buf,sizeof(buf), "%03.2f", dIntensity);
	Static_SetTextA(GetDlgItem(hwnd, IDC_INTENSITYDISP), buf);
}

/* Handle changes to the A2D slider */
static void A2DSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dA2D;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_A2D, TBM_GETPOS, 0, 0);

	dA2D = nValue / 20.0;

	/* Set the static display to the new value */
	snprintf(buf,sizeof(buf), "%03.2f", dA2D);
	Static_SetTextA(GetDlgItem(hwnd, IDC_A2DDISP), buf);
}

/* Handle changes to the Color Depth drop down */
static void ResDepthSelectionChange(HWND hWnd, HWND hWndCtrl)
{
	int nCurSelection;

	nCurSelection = ComboBox_GetCurSel(hWndCtrl);
	if (nCurSelection != CB_ERR)
	{
		HWND hRefreshCtrl;
		int nResDepth = 0;
		int nRefresh  = 0;

		nResDepth = ComboBox_GetItemData(hWndCtrl, nCurSelection);

		hRefreshCtrl = GetDlgItem(hWnd, IDC_REFRESH);
		if (hRefreshCtrl)
		{
			nCurSelection = ComboBox_GetCurSel(hRefreshCtrl);
			if (nCurSelection != CB_ERR)
				nRefresh = ComboBox_GetItemData(hRefreshCtrl, nCurSelection);
		}

		UpdateDisplayModeUI(hWnd, nResDepth, nRefresh);
	}
}

/* Handle changes to the Refresh drop down */
static void RefreshSelectionChange(HWND hWnd, HWND hWndCtrl)
{
	int nCurSelection;

	nCurSelection = ComboBox_GetCurSel(hWndCtrl);
	if (nCurSelection != CB_ERR)
	{
		HWND hResDepthCtrl;
		int nResDepth = 0;
		int nRefresh  = 0;

		nRefresh = ComboBox_GetItemData(hWndCtrl, nCurSelection);

		hResDepthCtrl = GetDlgItem(hWnd, IDC_RESDEPTH);
		if (hResDepthCtrl)
		{
			nCurSelection = ComboBox_GetCurSel(hResDepthCtrl);
			if (nCurSelection != CB_ERR)
				nResDepth = ComboBox_GetItemData(hResDepthCtrl, nCurSelection);
		}

		UpdateDisplayModeUI(hWnd, nResDepth, nRefresh);
	}
}

/* Handle changes to the Volume slider */
static void VolumeSelectionChange(HWND hwnd)
{
	char buf[100];
	int  nValue;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_VOLUME, TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buf,sizeof(buf), "%ddB", nValue - 32);
	Static_SetTextA(GetDlgItem(hwnd, IDC_VOLUMEDISP), buf);
}

static void AudioLatencySelectionChange(HWND hwnd)
{
	char buffer[100];
	int value;

	// Get the current value of the control
	value = SendMessage(GetDlgItem(hwnd,IDC_AUDIO_LATENCY), TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buffer,sizeof(buffer),"%i/4",value);
	Static_SetTextA(GetDlgItem(hwnd,IDC_AUDIO_LATENCY_DISP),buffer);
}

static void D3DScanlinesSelectionChange(HWND hwnd)
{
	char buffer[100];
	int value;

	// Get the current value of the control
	value = SendMessage(GetDlgItem(hwnd,IDC_D3D_SCANLINES), TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buffer,sizeof(buffer),"%i",value);
	Static_SetTextA(GetDlgItem(hwnd,IDC_D3D_SCANLINES_DISP),buffer);

}

static void D3DFeedbackSelectionChange(HWND hwnd)
{
	char buffer[100];
	int value;

	// Get the current value of the control
	value = SendMessage(GetDlgItem(hwnd,IDC_D3D_FEEDBACK), TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buffer,sizeof(buffer),"%i",value);
	Static_SetTextA(GetDlgItem(hwnd,IDC_D3D_FEEDBACK_DISP),buffer);

}

static void ZoomSelectionChange(HWND hwnd)
{
	char buffer[100];
	int value;

	// Get the current value of the control
	value = SendMessage(GetDlgItem(hwnd,IDC_ZOOM), TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buffer,sizeof(buffer),"%i",value);
	Static_SetTextA(GetDlgItem(hwnd,IDC_ZOOMDIST),buffer);

}
#ifdef USE_VIRTUAL_3D_VIEW
/* Handle changes to the Rotation slider */
static void D3DRotationChange(HWND hwnd)
{
	char buffer[100];
	int  nValue;

	/* Get the current value of the control */
	nValue = SendMessage(GetDlgItem(hwnd,IDC_D3D_ROTATION), TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buffer,sizeof(buffer),"%3d",nValue);
	Static_SetTextA(GetDlgItem(hwnd,IDC_D3D_ROTATION_DISP),buffer);
}

/* Handle changes to the Zoom x slider */
static void D3DZoomxChange(HWND hwnd)
{
	char buffer[100];
	int  nValue;
	double dZoomx;

	/* Get the current value of the control */
	nValue = SendMessage(GetDlgItem(hwnd, IDC_D3D_ZOOMX), TBM_GETPOS, 0, 0);

	dZoomx = nValue / 10.0 - 10.0;

	/* Set the static display to the new value */
	snprintf(buffer,sizeof(buffer),"%02.01f",dZoomx);
	Static_SetTextA(GetDlgItem(hwnd, IDC_D3D_ZOOMX_DISP),buffer);
}

/* Handle changes to the Zoom y slider */
static void D3DZoomyChange(HWND hwnd)
{
	char buffer[100];
	int  nValue;
	double dZoomy;

	/* Get the current value of the control */
	nValue = SendMessage(GetDlgItem(hwnd,IDC_D3D_ZOOMY), TBM_GETPOS, 0, 0);

	dZoomy = nValue / 10.0 - 10.0;

	/* Set the static display to the new value */
	snprintf(buffer,sizeof(buffer),"%02.01f",dZoomy);
	Static_SetTextA(GetDlgItem(hwnd,IDC_D3D_ZOOMY_DISP),buffer);
}
#endif /* USE_VIRTUAL_3D_VIEW */


/* Adjust possible choices in the Screen Size drop down */
static void UpdateDisplayModeUI(HWND hwnd, DWORD dwDepth, DWORD dwRefresh)
{
	int                   i;
	char                  buf[100];
	struct tDisplayModes* pDisplayModes;
	int                   nPick;
	int                   nCount = 0;
	int                   nSelection = 0;
	DWORD                 w = 0, h = 0;
	HWND                  hCtrl = GetDlgItem(hwnd, IDC_SIZES);

	if (!hCtrl)
		return;

	/* Find out what is currently selected if anything. */
	nPick = ComboBox_GetCurSel(hCtrl);
	if (nPick != 0 && nPick != CB_ERR)
	{
		ComboBox_GetTextA(GetDlgItem(hwnd, IDC_SIZES), buf, 100);
		if (sscanf(buf, "%lu x %lu", &w, &h) != 2)
		{
			w = 0;
			h = 0;
		}
	}

	/* Remove all items in the list. */
	ComboBox_ResetContent(hCtrl);

	ComboBox_AddStringA(hCtrl, _UI("Auto"));

	pDisplayModes = DirectDraw_GetDisplayModes();

	for (i = 0; i < pDisplayModes->m_nNumModes; i++)
	{
		if ((pDisplayModes->m_Modes[i].m_dwBPP     == dwDepth   || dwDepth   == 0)
		&&  (pDisplayModes->m_Modes[i].m_dwRefresh == dwRefresh || dwRefresh == 0))
		{
			sprintf(buf, "%lu x %lu", pDisplayModes->m_Modes[i].m_dwWidth,
			                          pDisplayModes->m_Modes[i].m_dwHeight);

			if (ComboBox_FindStringA(hCtrl, 0, buf) == CB_ERR)
			{
				ComboBox_AddStringA(hCtrl, buf);
				nCount++;

				if (w == pDisplayModes->m_Modes[i].m_dwWidth
				&&  h == pDisplayModes->m_Modes[i].m_dwHeight)
					nSelection = nCount;
			}
		}
	}

	ComboBox_SetCurSel(hCtrl, nSelection);
}

/* Initialize the Display options to auto mode */
static void InitializeDisplayModeUI(HWND hwnd)
{
	UpdateDisplayModeUI(hwnd, 0, 0);
}

/* Initialize the sound options */
static void InitializeSoundUI(HWND hwnd)
{
	HWND    hCtrl;

	hCtrl = GetDlgItem(hwnd, IDC_SAMPLERATE);
	if (hCtrl)
	{
		ComboBox_AddStringA(hCtrl, "11025");
		ComboBox_AddStringA(hCtrl, "22050");
		ComboBox_AddStringA(hCtrl, "24000");
		ComboBox_AddStringA(hCtrl, "44100");
		ComboBox_AddStringA(hCtrl, "48000");
		ComboBox_SetCurSel(hCtrl, 3);
	}
}

/* Populate the Frame Skipping drop down */
static void InitializeSkippingUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_FRAMESKIP);

	if (hCtrl)
	{
		ComboBox_AddStringA(hCtrl, _UI("Draw every frame"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 1 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 2 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 3 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 4 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 5 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 6 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 7 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 8 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 9 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 10 of 12 frames"));
		ComboBox_AddStringA(hCtrl, _UI("Skip 11 of 12 frames"));
	}
}

/* Populate the Rotate drop down */
static void InitializeRotateUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_ROTATE);

	if (hCtrl)
	{
		ComboBox_AddStringA(hCtrl, _UI("Default"));             // 0
		ComboBox_AddStringA(hCtrl, _UI("Clockwise"));           // 1
		ComboBox_AddStringA(hCtrl, _UI("Anti-clockwise"));      // 2
		ComboBox_AddStringA(hCtrl, _UI("None"));                // 3
		ComboBox_AddStringA(hCtrl, _UI("Auto clockwise"));      // 4
		ComboBox_AddStringA(hCtrl, _UI("Auto anti-clockwise")); // 5
	}
}

/* Populate the resolution depth drop down */
static void InitializeResDepthUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_RESDEPTH);

	if (hCtrl)
	{
		struct tDisplayModes* pDisplayModes;
		int nCount = 0;
		int i;

		/* Remove all items in the list. */
		ComboBox_ResetContent(hCtrl);

		ComboBox_AddStringA(hCtrl, _UI("Auto"));
		ComboBox_SetItemData(hCtrl, nCount++, 0);

		pDisplayModes = DirectDraw_GetDisplayModes();

		for (i = 0; i < pDisplayModes->m_nNumModes; i++)
		{
			if (pDisplayModes->m_Modes[i].m_dwBPP == 16
			||  pDisplayModes->m_Modes[i].m_dwBPP == 24
			||  pDisplayModes->m_Modes[i].m_dwBPP == 32)
			{
				char buf[16];
				LPWSTR p;

				sprintf(buf, "%li bit", pDisplayModes->m_Modes[i].m_dwBPP);
				p = _Unicode(buf);

				if (ComboBox_FindString(hCtrl, 0, p) == CB_ERR)
				{
					ComboBox_InsertString(hCtrl, nCount, p);
					ComboBox_SetItemData(hCtrl, nCount++, pDisplayModes->m_Modes[i].m_dwBPP);
				}
			}
		}
	}
}

/* Populate the refresh drop down */
static void InitializeRefreshUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_REFRESH);

	if (hCtrl)
	{
		struct tDisplayModes* pDisplayModes;
		int nCount = 0;
		int i;

		/* Remove all items in the list. */
		ComboBox_ResetContent(hCtrl);

		ComboBox_AddStringA(hCtrl, _UI("Auto"));
		ComboBox_SetItemData(hCtrl, nCount++, 0);

		pDisplayModes = DirectDraw_GetDisplayModes();

		for (i = 0; i < pDisplayModes->m_nNumModes; i++)
		{
			if (pDisplayModes->m_Modes[i].m_dwRefresh != 0)
			{
				char buf[16];
				LPWSTR p;

				sprintf(buf, "%li Hz", pDisplayModes->m_Modes[i].m_dwRefresh);
				p = _Unicode(buf);

				if (ComboBox_FindString(hCtrl, 0, p) == CB_ERR)
				{
					ComboBox_InsertString(hCtrl, nCount, p);
					ComboBox_SetItemData(hCtrl, nCount++, pDisplayModes->m_Modes[i].m_dwRefresh);
				}
			}
		}
	}
}

/* Populate the Default Input drop down */
static void InitializeDefaultInputUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_DEFAULT_INPUT);

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	char *ext;
	char root[256];
	char path[256];

	if (hCtrl)
	{
		ComboBox_AddStringA(hCtrl, "Standard");

		sprintf (path, "%s\\*.*", GetCtrlrDir());

		hFind = FindFirstFileA(path, &FindFileData);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do 
			{
				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;

				// copy the filename
				strcpy (root,FindFileData.cFileName);

				// find the extension
				ext = strrchr (root,'.');
				if (ext)
				{
					// check if it's a cfg file
					if (strcmp (ext, ".cfg") == 0)
					{
						// and strip off the extension
						*ext = 0;

						// add it as an option
						ComboBox_AddStringA(hCtrl, root);
					}
				}
			}
			while (FindNextFileA (hFind, &FindFileData) != 0);
			
			FindClose (hFind);
		}
	}
}

/* Populate the LED mode drop down */
static void InitializeLEDModeUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_LEDMODE);

	if (hCtrl)
	{
		int i;
		for (i = 0; i < NUMLEDMODES; i++)
		{
			ComboBox_InsertString(hCtrl, i, _Unicode(_UI(g_ComboBoxLedmode[i].m_pText)));
			ComboBox_SetItemData( hCtrl, i, g_ComboBoxLedmode[i].m_pData);
		}
	}
}


static void InitializeEffectUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_EFFECT);

	if (hCtrl)
	{
		int i;
		for (i = 0; i < NUMEFFECTS; i++)
		{
			ComboBox_InsertString(hCtrl, i, _Unicode(_UI(g_ComboBoxEffect[i].m_pText)));
			ComboBox_SetItemData( hCtrl, i, g_ComboBoxEffect[i].m_pData);
		}
	}
}

/* Populate the Art Resolution drop down */
static void InitializeArtresUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_ARTRES);

	if (hCtrl)
	{
		ComboBox_AddStringA(hCtrl, _UI("Auto"));		/* 0 */
		ComboBox_AddStringA(hCtrl, _UI("Standard"));  /* 1 */
		ComboBox_AddStringA(hCtrl, _UI("High"));		/* 2 */
	}
}

static void InitializeD3DFilterUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd,IDC_D3D_FILTER);

	if (hCtrl)
	{
		int i;

		for (i=0;i<NUMD3DFILTER;i++)
			ComboBox_AddStringA(hCtrl,_UI(d3d_filter_long_name[i]));
	}
}

static void InitializeD3DEffectUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd,IDC_D3D_EFFECT);

	if (hCtrl)
	{
		int i;

		for (i=0;i<NUMD3DEFFECTS;i++)
			ComboBox_AddStringA(hCtrl,_UI(d3d_effects_long_name[i]));
	}
}

static void InitializeD3DPrescaleUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd,IDC_D3D_PRESCALE);

	if (hCtrl)
	{
		int i;

		for (i=0;i<NUMD3DPRESCALE;i++)
			ComboBox_AddStringA(hCtrl,_UI(d3d_prescale_name[i]));
	}
}

static void InitializeBIOSUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd,IDC_BIOS);

	if (hCtrl && g_biosinfo)
	{
		int i;

		for (i = 0; !BIOSENTRY_ISEND(&g_biosinfo[i]); i++)
		{
			ComboBox_AddStringA(hCtrl,g_biosinfo[i]._description);
		}
	}
}

static void InitializeDefaultBIOSUI(HWND hwnd)
{
	int n;

	for (n = 0; n < MAX_SYSTEM_BIOS; n++)
	{
		const struct GameDriver *drv = GetSystemBiosInfo(n);
		HWND hCtrl = GetDlgItem(hwnd,IDC_BIOS1 + n);

		if (hCtrl && drv)
		{
			int i;

			for (i = 0; !BIOSENTRY_ISEND(&drv->bios[i]); i++)
			{
				ComboBox_AddStringA(hCtrl,drv->bios[i]._description);
			}
		}
	}
}

#ifdef ROM_PATCH
static void InitializeIPSUI(HWND hwnd)
{
	if (IS_GAME)
	{
		int patch_count = HasPatch(drivers[g_nGame]->name, "*");
		int n;
		int i;
		char patchName[64];

		for (n = 0; n < PATCH_MAX; n++)
		{
			HWND hCtrl = GetDlgItem(hwnd,IDC_IPS1+n);
			if (hCtrl)
				ComboBox_AddStringA(hCtrl, _UI("None"));
		}

		for (i = 0; i < patch_count; i++)
		{
			if (GetPatchName(patchName, drivers[g_nGame]->name, i))
			{
				for (n = 0; n < PATCH_MAX; n++)
				{
					HWND hCtrl = GetDlgItem(hwnd,IDC_IPS1+n);
					if (hCtrl)
						ComboBox_AddStringA(hCtrl, patchName);
				}
			}
		}
	}
	if (hTooltipWnd == NULL)
		InitTooltip(hwnd);	//fixme: create tooltip window only once
}
#endif /* ROM_PATCH */

static void InitializeCleanStretchUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_CLEAN_STRETCH);

	if (hCtrl)
	{
		int i;

		for (i=0;i<NUMCLEANSTRETCH;i++)
			ComboBox_AddStringA(hCtrl,_UI(clean_stretch_name[i]));
	}
}

static void InitializeM68kCoreUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_M68K_CORE);

	if (hCtrl)
	{
		ComboBox_AddStringA(hCtrl, "C");
		ComboBox_AddStringA(hCtrl, "DRC");
		ComboBox_AddStringA(hCtrl, "ASM");
	}
}

#ifdef JOYSTICK_ID
/* Populate the Joystick ID drop down */
static void InitializeJoyidUI(HWND hWnd)
{
	HWND hCtrl;
	int i, j, num;
	char buf[8];

	if (DIJoystick.Available() == FALSE)
		return;

	if (GetJoyGUI() == FALSE)
		DIJoystick.init();

	num = (DIJoystick_GetNumPhysicalJoysticks() < 8) ? DIJoystick_GetNumPhysicalJoysticks() : 8;

	for (i = 0; i < num; i++)
	{
		hCtrl = GetDlgItem(hWnd, IDC_JOYID1 + i);
		if (hCtrl)
		{
			for (j = 0;j < DIJoystick_GetNumPhysicalJoysticks(); j++)
			{
				sprintf(buf, _UI("ID:%d"), j + 1);
				ComboBox_AddStringA(hCtrl, buf);
			}
		}
	}
}
#endif /* JOYSTICK_ID */

#ifdef USE_SCALE_EFFECTS
/* Populate the scale effect drop down */
 static void InitializeScaleEffectUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_SCALEEFFECT);

	if (hCtrl)
	{
		int i;

		for (i=0;i<NUMSCALEEFFECTS;i++)
      ComboBox_AddStringA(hCtrl,_UI(scale_effects_long_name[i]));

	}
}
#endif /* USE_SCALE_EFFECTS */

#ifdef TRANS_UI
static void TransparencySelectionChange(HWND hwnd)
{
	char buf[100];
	int  nValue;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_TRANSPARENCY, TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	sprintf(buf, "%d", nValue);
	Static_SetTextA(GetDlgItem(hwnd, IDC_TRANSPARENCYDISP), buf);
}
#endif /* TRANS_UI */

#ifdef MENU_TRANS
static void GameOptionsPaint( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	HWND hWndCtrl;

	hWndCtrl = GetDlgItem( hWnd, IDC_STATIC_MENUBGCOLOR );
	if( (hWndCtrl == NULL) || !IsWindowVisible(hWndCtrl) ) return;

	RECT rcRect;
	PAINTSTRUCT ps;
	HDC	 memDC;
	BITMAP  bmpInfo;
	HBITMAP	hBmp, hOldBitmap;
	COLORREF crColor, crTemp;

	// 다이얼로그 드로잉 시작
	BeginPaint (hWnd, &ps);

	// 현재 선택된 메뉴바탕색 표시
	//---------------------------------------------------------------
	GetWindowRect( hWndCtrl, &rcRect );
	
	ScreenToClient(hWnd, (LPPOINT)&rcRect);
	ScreenToClient(hWnd, ((LPPOINT)&rcRect)+1);

	// DeflateRect
	rcRect.left	+= 2;
	rcRect.top	+= 2;
	rcRect.right	-= 3;
	rcRect.bottom	-= 3;

	// Change MAME-COLOR to WIN-COLOR
	crColor = pGameOpts->menu_color;
	crColor = RGB(RGB_RED(crColor), RGB_GREEN(crColor), RGB_BLUE(crColor));

	// MFC: FillSolidRect()
	crTemp = SetBkColor(ps.hdc, crColor);
	ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &rcRect, NULL, 0, NULL);
	SetBkColor(ps.hdc, crTemp);

	// 게임내 메뉴의 예상화면 드로잉
	//---------------------------------------------------------------
	hWndCtrl = GetDlgItem( hWnd, IDC_STATIC_MENUPREVIEW );
	GetWindowRect( hWndCtrl, &rcRect );
	
	ScreenToClient(hWnd, (LPPOINT)&rcRect);
	ScreenToClient(hWnd, ((LPPOINT)&rcRect)+1);
	
	hBmp = LoadBitmap( GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_MENUPREVIEW) );
	if( hBmp ) {
		GetObject(hBmp, sizeof(BITMAP), &bmpInfo);

		// 깜박임 방지를 위한 메모리DC 생성
		memDC = CreateCompatibleDC(ps.hdc);
		hOldBitmap = (HBITMAP)SelectObject(memDC, hBmp);

		{
			int x, y, br, bg, bb, sr, sg, sb, opaque;

			opaque = 100 - pGameOpts->menu_trans;
			sr = RGB_RED(pGameOpts->menu_color)	  * opaque;
			sg = RGB_GREEN(pGameOpts->menu_color) * opaque;
			sb = RGB_BLUE(pGameOpts->menu_color)  * opaque;

			for(y=0; y<bmpInfo.bmHeight; y++) {
				for(x=40; x<bmpInfo.bmWidth; x++) {
					crColor = GetPixel(memDC,x, y);

					br = ((GetRValue(crColor) * pGameOpts->menu_trans) + sr) / 100;
					bg = ((GetGValue(crColor) * pGameOpts->menu_trans) + sg) / 100;
					bb = ((GetBValue(crColor) * pGameOpts->menu_trans) + sb) / 100;

					crColor = RGB(br,bg,bb);
					SetPixel(memDC,x,y,crColor);
				}
			}
		}

		// 메모리DC -> 화면DC
		BitBlt( ps.hdc, rcRect.left + 2, rcRect.top + 4,
						rcRect.right-rcRect.left, rcRect.bottom-rcRect.top,
						memDC, 0, 0, SRCCOPY );

		// 메모리DC 삭제
		if( hOldBitmap ) SelectObject(memDC, hOldBitmap);
		DeleteDC(memDC);
		DeleteObject(hBmp);
	}

	// 다이얼로그 드로잉 끝
	EndPaint (hWnd, &ps); 
}

static void PickGameMenuColor(HWND hWnd, COLORREF *lpcrColor)
{
	CHOOSECOLOR cc;
	COLORREF crColor, choice_colors[16];
	int i;

	if( !lpcrColor ) return;

	for (i=0;i<16;i++)
		choice_colors[i] = RGB(0,0,0);

	crColor = *lpcrColor;
	cc.lStructSize  = sizeof(CHOOSECOLOR);
	cc.hwndOwner    = hWnd;
	// Change MAME-COLOR to WIN-COLOR
	cc.rgbResult	= RGB(RGB_RED(crColor), RGB_GREEN(crColor), RGB_BLUE(crColor));
	cc.lpCustColors = choice_colors;
	cc.Flags        = CC_FULLOPEN | CC_ANYCOLOR | CC_RGBINIT | CC_SOLIDCOLOR;
	if (!ChooseColor(&cc))
		return;

	// WIN-COLOR to MAME-COLOR
	*lpcrColor = MAKE_RGB(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult) );
	//InvalidateRect(hWnd,NULL,FALSE);
}

static void InitializePlusUI(HWND hwnd)
{
	// Do nothing. 현재로는 초기화할게 없음..
}

static void MenuTransSelectionChange(HWND hWnd)
{
	char buffer[100];
	int  value;
	RECT rcRect;
	HWND hWndCtrl;

	/* Get the current value of the control */
	value = SendMessage(GetDlgItem(hWnd,IDC_PLUS_MENUTRANSPARENCY), TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buffer,sizeof(buffer),"%i",value);
	Static_SetTextA(GetDlgItem(hWnd,IDC_STATIC_MENUTRANSPARENCY),buffer);

	/* 메뉴 미리보기 이미지 업데이트 */
	hWndCtrl = GetDlgItem(hWnd, IDC_STATIC_MENUPREVIEW);
	if( hWndCtrl ) {
		GetWindowRect( hWndCtrl, &rcRect );
		
		ScreenToClient(hWnd, (LPPOINT)&rcRect);
		ScreenToClient(hWnd, ((LPPOINT)&rcRect)+1);

		InvalidateRect(hWnd, &rcRect, FALSE);
	}

}
#endif /* MENU_TRANSE */

void UpdateBackgroundBrush(HWND hwndTab)
{
	// Check if the application is themed
	if (hThemes)
	{
		if(fnIsThemed)
			bThemeActive = fnIsThemed();
	}
	// Destroy old brush
	if (hBkBrush)
		DeleteObject(hBkBrush);

	hBkBrush = NULL;

	// Only do this if the theme is active
	if (bThemeActive)
	{
		RECT rc;
		HDC hDC, hDCMem;
		HBITMAP hBmp, hBmpOld;
		// Get tab control dimensions
		GetWindowRect( hwndTab, &rc);

		// Get the tab control DC
		hDC = GetDC(hwndTab);

		// Create a compatible DC
		hDCMem = CreateCompatibleDC(hDC);
		hBmp = CreateCompatibleBitmap(hDC, 
		                              rc.right - rc.left, rc.bottom - rc.top);
		hBmpOld = (HBITMAP)(SelectObject(hDCMem, hBmp));

		// Tell the tab control to paint in our DC
		SendMessage(hwndTab, WM_PRINTCLIENT, (WPARAM)(hDCMem), 
		            (LPARAM)(PRF_ERASEBKGND | PRF_CLIENT | PRF_NONCLIENT));

		// Create a pattern brush from the bitmap selected in our DC
		hBkBrush = CreatePatternBrush(hBmp);

		// Restore the bitmap
		SelectObject(hDCMem, hBmpOld);

		// Cleanup
		DeleteObject(hBmp);
		DeleteDC(hDCMem);
		ReleaseDC(hwndTab, hDC);
	}
}

#ifdef USE_PSXPLUGIN
static void InitializePSXPlugInUI(HWND hwnd)
{
	//HWND hCtrl = GetDlgItem(hwnd, IDC_COMBO_GPUPLUGIN);
	//
	//if (hCtrl)
	//{
		// if( GetUserDefaultLangID() == 0x0412 ) {
		//	ComboBox_AddStringA(hCtrl, "d3d_renderer.znc" );
		//	ComboBox_AddStringA(hCtrl, "ogl_renderer.znc" );
		//	ComboBox_AddStringA(hCtrl, "gpuPeteOpenGL2.dll" );
		//}
	//}

	// PSX_GPU_EXTRA
	HWND hCtrl = GetDlgItem(hwnd, IDC_COMBO_PSXGPU_SCREENSIZE);
	if (hCtrl)
	{
		ComboBox_AddStringA(hCtrl, "640x480" );
		ComboBox_AddStringA(hCtrl, "800x600" );
		ComboBox_AddStringA(hCtrl, "1024x768" );
		ComboBox_AddStringA(hCtrl, "1152x864" );
	}
}

typedef long (CALLBACK *PROC_GPUCONFIGURE)(void);

static BOOL IsPSXGPUConfigureAvail(HWND hwnd)
{
	/* Get current selected GPU plugin's name */
	char plugin[200];
	char szFile[256];
	PROC_GPUCONFIGURE	lpfnGPUconfig;
	HMODULE hLib;
	HWND hCtrl = GetDlgItem(hwnd, IDC_COMBO_GPUPLUGIN);
	
	if (!hCtrl) return FALSE;

	ComboBox_GetTextA(hCtrl, plugin, sizeof(plugin)-1);
	
	/* make path to plugin */
	sprintf( szFile, "plugins/%s", plugin );

	/* load library(plugin) */
	hLib = LoadLibrary( _Unicode(szFile) );
	if( hLib == NULL ) return FALSE;
	
	/* get exported configuration function */
	lpfnGPUconfig = (PROC_GPUCONFIGURE)GetProcAddress( hLib, "GPUconfigure" );
	
	FreeLibrary( hLib );

	if( lpfnGPUconfig == NULL ) return FALSE;
	
	return TRUE;
}

static void RunPSXGPUConfigure(HWND hwnd)
{
	/* Get current selected GPU plugin's name */
	char plugin[200];
	char szFile[256];
	long nResult;
	PROC_GPUCONFIGURE	lpfnGPUconfig;
	HMODULE hLib;
	HWND hCtrl = GetDlgItem(hwnd, IDC_COMBO_GPUPLUGIN);
	if (!hCtrl) return;

	ComboBox_GetTextA(hCtrl, plugin, sizeof(plugin)-1);
	
	/* make path to plugin */
	sprintf( szFile, "plugins/%s", plugin );

	/* load library(plugin) */
	hLib = LoadLibrary( _Unicode(szFile) );
	if( hLib == NULL ) {
		char szMsg[200];
		sprintf( szMsg, "Unable to load GPU plugin '%s'", szFile );
		MessageBox( hwnd, _Unicode(szMsg), _Unicode("ERROR"), MB_OK|MB_ICONSTOP );
		return;
	}
	
	/* get exported configuration function */
	lpfnGPUconfig = (PROC_GPUCONFIGURE)GetProcAddress( hLib, "GPUconfigure" );
	if( lpfnGPUconfig == NULL ) {
		char szMsg[200];
		sprintf( szMsg, "This plugin does not contains configuration function" );
		MessageBox( hwnd, _Unicode(szMsg), _Unicode(plugin), MB_OK|MB_ICONSTOP );
		FreeLibrary( hLib );
		return;
	}
	
	nResult = lpfnGPUconfig();
	
	FreeLibrary( hLib );

	if( nResult != 0 ) {
		char szMsg[200];
		sprintf( szMsg, "GPU plugin returned error code:%ld", nResult );
		MessageBox( hwnd, _Unicode(szMsg), _Unicode(plugin), MB_OK|MB_ICONSTOP );
	}
}

#endif 

/* End of source file */
