/***************************************************************************

	inptport_kaillera.c

	Input port handling for kaillera.

	by DarkCoder
	
	---------------------------------------------------------------------
	2005. 2.16	Apply newly added mahjong input ports. (0.92u2000)
	
	2005. 1.	Update initialize function and fix some bug.	
	
	2004. 7.	Totally re-written to sync updated mame input system.
				analog input system may not work now. (0.84u1)
***************************************************************************/

#include "windows/netApi.h"
#include "ui/kcommand.h"
#include "ui/chat.h"
#include "cheat.h"
#include "ui_text.h"

int kBitsPlayValues[10][32][6];
int kUpdateDip;
unsigned short kDipSwitch[MAX_INPUT_PORTS];
static UINT32  kDefValues[MAX_INPUT_PORTS];	/* DARKCODER (2004.7.30): modified unsigned short -> UINT32 ( = input_port_value[] ) */

int kFirstMsg;
extern int kPlay;
extern int kPlayers;
extern int kPlayerNB;
extern void addClock(cycles_t a);
extern void *record_k;
extern void *playback_k;
extern cycles_t (*cycle_counter)(void);
extern unsigned int fgetsavecrc(const char *gamename, const char *filename);

/* Control Swap Variables */
int p1, p2, p3, p4, p5, p6, p7, p8;
int ana_p1, ana_p2, ana_p3, ana_p4;
int p1_pos = 1;
int control_counter = 0;
/* General Global Variables */
int control_type = 0;
int endemu = 0;
int special_key = 0;
int netplay_pause = -1;

static UINT32 playback_value[MAX_INPUT_PORTS];	/* DARKCODER (2004.7.30): modified unsigned short -> UINT32 for read_port_value() */

/* For Multiplayer Hacks */
int p1_1c, p1_2c, p1_3c, p1_4c, p2_1c, p2_2c, p2_3c, p2_4c;
int player1_c, player2_c, player3_c, player4_c;
int player1_sub, player2_sub, player3_sub, player4_sub;
int p1_1st, p1_2nd, p1_3rd, p1_4th, p2_1st, p2_2nd, p2_3rd, p2_4th;
int init = -1;
int kGameIndex = -1;		// inptport.c

#define X_AXIS			0
#define Y_AXIS			1
#define Z_AXIS			2
#define PEDAL_AXIS		3

enum {
	K_UI_NONE = 0,
	K_UI_RESET_MACHINE,
	K_UI_LOAD_STATE,
	K_UI_SAVE_STATE,
	K_UI_APPLY_DIP,
	K_UI_SHIFT_DOWN,
	K_UI_SHIFT_UP,
	K_UI_SWAP_CONTROL,
	K_UI_AUTO_SAVE,
	K_UI_LOAD_AUTOSAVE,
	K_UI_PAUSE,
	K_UI_MASK  = 0x7fffffff,
	K_UI_UIBIT = 0x80000000,
};

/* ---------------------------------------------- */
int show_chat_log = 0;	// CHAT_LOG

UINT32 _kInputQueue[10];

typedef struct _strcKINPUTLIST {
	UINT32	type;
	int 	control_type;	// 0:digial 1:analog 2:other(mahjong)
	int 	player;			// player (-1 for port->player)
	int 	pos;
	int		axis;			// analog axis(0:X 1:Y 2:other)
	int		abs;			// analog absolute axis (0:absolute 1:relative)
} KINPUTLIST;

KINPUTLIST _kInpData[] = {	
	// type				ctrl_type	player		pos	aixs	abs
	/* digital input ports */
	{ IPT_BUTTON1,				0,		-1,		0,	-1,		-1		},
	{ IPT_BUTTON2,				0,		-1,		1,	-1,		-1		},
	{ IPT_BUTTON3,				0,		-1,		2,	-1,		-1		},
	{ IPT_BUTTON4,				0,		-1,		3,	-1,		-1		},
	{ IPT_BUTTON5,				0,		-1,		4,	-1,		-1		},
	{ IPT_BUTTON6,				0,		-1,		5,	-1,		-1		},
	{ IPT_BUTTON7,				0,		-1,		6,	-1,		-1		},

	{ IPT_JOYSTICK_UP,			0,		-1,		7,	-1,		-1		},
	{ IPT_JOYSTICKRIGHT_UP,		0,		-1,		7,	-1,		-1		},
	{ IPT_JOYSTICK_DOWN,		0,		-1,		8,	-1,		-1		},
	{ IPT_JOYSTICKRIGHT_DOWN,	0,		-1,		8,	-1,		-1		},
	{ IPT_JOYSTICK_LEFT,		0,		-1,		9,	-1,		-1		},
	{ IPT_JOYSTICKRIGHT_LEFT,	0,		-1,		9,	-1,		-1		},
	{ IPT_JOYSTICK_RIGHT,		0,		-1,		10,	-1,		-1		},
	{ IPT_JOYSTICKRIGHT_RIGHT,	0,		-1,		10,	-1,		-1		},
				
	{ IPT_COIN1,				0,		 0,		11,	-1,		-1		},
	{ IPT_START1,				0,		 0,		12,	-1,		-1		},
	{ IPT_COIN2,				0,		 1,		11,	-1,		-1		},
	{ IPT_START2,				0,		 1,		12,	-1,		-1		},
	{ IPT_COIN3,				0,		 2,		11,	-1,		-1		},
	{ IPT_START3,				0,		 2,		12,	-1,		-1		},
	{ IPT_COIN4,				0,		 3,		11,	-1,		-1		},
	{ IPT_START4,				0,		 3,		12,	-1,		-1		},

	{ IPT_SERVICE,				0,		 0,		13,	-1,		-1		},
	{ IPT_SERVICE1,				0,		 0,		14,	-1,		-1		},
	{ IPT_SERVICE3,				0,		 0,		14,	-1,		-1		},

	// type				ctrl_type	player		pos	aixs	abs
	/* analog input ports */
	// 아래의 x축키는 pos 5,6을 차지함
	{ IPT_PADDLE,				1,		-1,		5,	 0,		0		},
	{ IPT_AD_STICK_X,			1,		-1,		5,	 0,		0		},
	{ IPT_LIGHTGUN_X,			1,		-1,		5,	 0,		0		},
	{ IPT_DIAL,					1,		-1,		5,	 0,		1		},
	{ IPT_TRACKBALL_X,			1,		-1,		5,	 0,		1		},

	// 아래의 y축키는 pos 3,4를 차지함
	{ IPT_PADDLE_V,				1,		-1,		3,	 1,		0		},
	{ IPT_AD_STICK_Y,			1,		-1,		3,	 1,		0		},
	{ IPT_LIGHTGUN_Y,			1,		-1,		3,	 1,		0		},
	{ IPT_DIAL_V,				1,		-1,		3,	 1,		1		},
	{ IPT_TRACKBALL_Y,			1,		-1,		3,	 1,		1		},
				
	{ IPT_PEDAL,				1,		-1,		4,	 2,		0		},
	{ IPT_PEDAL2,				1,		-1,		4,	 2,		0		},
				
	// type				ctrl_type	player		pos	aixs	abs
	/* other(mahjong/card) input ports */
	{ IPT_MAHJONG_A,			2,		-1,		0,	-1,		-1		},
	{ IPT_MAHJONG_B,			2,		-1,		1,	-1,		-1		},
	{ IPT_MAHJONG_C,			2,		-1,		2,	-1,		-1		},
	{ IPT_MAHJONG_D,			2,		-1,		3,	-1,		-1		},
	{ IPT_MAHJONG_E,			2,		-1,		4,	-1,		-1		},
	{ IPT_MAHJONG_F,			2,		-1,		5,	-1,		-1		},
	{ IPT_MAHJONG_G,			2,		-1,		6,	-1,		-1		},
	{ IPT_MAHJONG_H,			2,		-1,		7,	-1,		-1		},
	{ IPT_MAHJONG_I,			2,		-1,		8,	-1,		-1		},
	{ IPT_MAHJONG_J,			2,		-1,		9,	-1,		-1		},
	{ IPT_MAHJONG_K,			2,		-1,		10,	-1,		-1		},
	{ IPT_MAHJONG_L,			2,		-1,		11,	-1,		-1		},
	{ IPT_MAHJONG_M,			2,		-1,		12,	-1,		-1		},
	{ IPT_MAHJONG_N,			2,		-1,		13,	-1,		-1		},
	{ IPT_MAHJONG_O,			2,		-1,		14,	-1,		-1		},	/* added 0.92u2000 */
	{ IPT_MAHJONG_P,			2,		-1,		15,	-1,		-1		},	/* added 0.92u2000 */
	{ IPT_MAHJONG_Q,			2,		-1,		16,	-1,		-1		},	/* added 0.92u2000 */
	{ IPT_MAHJONG_KAN,			2,		-1,		17,	-1,		-1		},
	{ IPT_MAHJONG_PON,			2,		-1,		18,	-1,		-1		},
	{ IPT_MAHJONG_CHI,			2,		-1,		19,	-1,		-1		},
	{ IPT_MAHJONG_REACH,		2,		-1,		20,	-1,		-1		},
	{ IPT_MAHJONG_RON,			2,		-1,		21,	-1,		-1		},
	{ IPT_MAHJONG_BET,			2,		-1,		22,	-1,		-1		},
	{ IPT_MAHJONG_LAST_CHANCE,	2,		-1,		23,	-1,		-1		},
	{ IPT_MAHJONG_SCORE,		2,		-1,		24,	-1,		-1		},
	{ IPT_MAHJONG_DOUBLE_UP,	2,		-1,		25,	-1,		-1		},
	{ IPT_MAHJONG_FLIP_FLOP,	2,		-1,		26,	-1,		-1		},
	{ IPT_MAHJONG_BIG,			2,		-1,		27,	-1,		-1		},	/* added 0.92u2000 */
	{ IPT_MAHJONG_SMALL,		2,		-1,		28,	-1,		-1		},	/* added 0.92u2000 */

	/* End */
	{ 0,						0,		0,		0,	0,		0		}
	
};

static void write_port_value(mame_file *f, UINT32 value);

/* 
---------------------------------------------------- 
 Functions
---------------------------------------------------- 
*/

static int GetKInputDataIndex(UINT32 type)
{
	int i = 0;
	
	while(_kInpData[i].type && (type != _kInpData[i].type)) i++;
	
	if(_kInpData[i].type == 0) return -1; /* not found */
	
	return i;
}

/*************************************
 *
 *	Initialize kaillera input system
 *
 *************************************/
 
/*
 현재 게임의 입력시스템을 검사하고 input_port_value[] 데이터 갱신을 위한
 포트번호와 정보를 kBitsPlayValues[]버퍼 및 기타 변수에 기록한다.

 kBitsPlayValues[]는 나중에 네트웍을 통해 모든 플레이어의 입력상태를
 알아냈을 때 적절한 input_port_value[] 갱신위치를 찾기 위해 포트번호와
 기타 정보를 저장하기위해 사용한다. 
 
 입력시스템을 검사한 결과 입력시스템의 종류에 따라 control_type변수값이
 다음과 같이 세팅된다.
 - 단순 키보드 입력시스템인 경우에는 0,
 - analog 디바이스가 사용되는 경우에는 1,
 - 기타 특수 입력시스템(버튼이 기본시스템보다 많은 경우)에는 2
 
 _kInputQueue버퍼를 이용해서 입력상태를 네트웍을 통해 전송한다.
 */
 
static void inputport_kaillera_init(void)
{
	struct InputPort *port;
	int portnum;
	int i;
	UINT32 mask, default_value;
	
	if (!kPlay) return;
	
	endemu = 0;
	special_key = 0;
	init = -1;
	netplay_pause = -1;
	
	control_type = 0;
	
	memset(&kBitsPlayValues, -1, sizeof(kBitsPlayValues));

	port = Machine->input_ports_default;
	port++;

	portnum = 0;

	/* 모든 입력포트에 대한 루프 */
	while ((port->type != IPT_END) && (portnum < MAX_INPUT_PORTS))
	{
		while ((port->type != IPT_END) && (port->type != IPT_PORT))
		{
			int idx, player, pos;
			
			idx = GetKInputDataIndex(port->type);
			if(idx >= 0) 
			{
				player = port->player;
				mask   = port->mask;
				default_value = port->default_value;
				
				pos = _kInpData[idx].pos;
				if(_kInpData[idx].player != -1) player = _kInpData[idx].player;
				if(_kInpData[idx].control_type) control_type = _kInpData[idx].control_type;
				
				
				switch(_kInpData[idx].control_type) {
				case 0: 
					/* digital input */
					if (kBitsPlayValues[player][pos][0] == -1) {
						kBitsPlayValues[player][pos][0] = portnum;
						kBitsPlayValues[player][pos][1] = mask;
						kBitsPlayValues[player][pos][2] = default_value;
					}
					else {
						kBitsPlayValues[player][pos][4] = portnum;
						kBitsPlayValues[player][pos][5] = mask;
					}
					break;
				case 1:
					/* analog input */
					switch(_kInpData[idx].axis) {
					case 0: /* x-axis */
						kBitsPlayValues[player][pos  ][0]=portnum;
						kBitsPlayValues[player][pos  ][1]=mask;
						kBitsPlayValues[player][pos  ][2]=default_value;
						kBitsPlayValues[player][pos  ][3]=2048;

						kBitsPlayValues[player][pos+1][0]=portnum;
						kBitsPlayValues[player][pos+1][1]=mask;
						kBitsPlayValues[player][pos+1][2]=default_value;
						kBitsPlayValues[player][pos+1][3]=4096;
						break;

					case 1: /* y-axis */
						kBitsPlayValues[player][pos  ][0]=portnum;
						kBitsPlayValues[player][pos  ][1]=mask;
						kBitsPlayValues[player][pos  ][2]=default_value;
						kBitsPlayValues[player][pos  ][3]=512;

						kBitsPlayValues[player][pos+1][0]=portnum;
						kBitsPlayValues[player][pos+1][1]=mask;
						kBitsPlayValues[player][pos+1][2]=default_value;
						kBitsPlayValues[player][pos+1][3]=1024;
						break;
					
					case 2: /* other */
						kBitsPlayValues[player][pos][0]=portnum;    
						kBitsPlayValues[player][pos][1]=mask;                      
						kBitsPlayValues[player][pos][2]=default_value;                                        
						kBitsPlayValues[player][pos][3]=1;
						break;
					}
					break;
					
				case 2: 
					/* other(mahjong) input */
					kBitsPlayValues[player][pos][0]=portnum;
					kBitsPlayValues[player][pos][1]=mask;
					kBitsPlayValues[player][pos][2]=default_value;
					break;
				}
			} /* end if */
			
			port++;
		}
		portnum++;
		if(port->type == IPT_PORT) port++;
	}
	
	kFirstMsg		= 1;
	
	kUpdateDip      = 0;
	kdipcounter     = 0;
	kgotdip         = 0;
	kcrccounter     = 0;
	kgotcrc         = 0;
	ksavecounter    = 0;
	
	p1_pos          = 1;
	
	p1		= 0;
	p2		= 1;
	p3		= 2;
	p4		= 3;
	p5		= 4;
	p6		= 5;
	p7		= 6;
	p8		= 7;
	
	ana_p1		= 0;
	ana_p2		= 1;
	ana_p3		= 2;
	ana_p4		= 3;
	
	control_counter   = 0;
	kaillera_schedule = KS_NONE;
	
	for (i = 0; i < 10; i++) {
		_kInputQueue[i] = 0;
	}	
	
}


/*************************************
 *
 *	Send and receive other player's input
 *
 *************************************/
 
int SendAndRecvKailleraInput(void)
{
	struct InputPort *port;
	int portnum;
	int recvLen = 0;
	
	if (!kPlay) return 0;

	if(kUpdateDip) 
	{
		UINT16	mask;

		port = Machine->input_ports_default;
		port++;
		
		portnum = 0;
		while ((port->type != IPT_END) && (portnum < MAX_INPUT_PORTS))
		{
			mask = 0;
			while ((port->type != IPT_END) && (port->type != IPT_PORT))
			{
				if (port->type == IPT_DIPSWITCH_NAME)
				{
					mask |= port->mask;
					port->default_value =
							(port->default_value & ~port->mask) | (kDipSwitch[portnum] & port->mask);
				}
				port++;
			}
			if(mask) portnum++;
			
			if (port->type == IPT_PORT) port++;
		}
		kUpdateDip = 0;
	}
	
	if (kFirstMsg) 
	{
		kFirstMsg = 0;
	}
	else
	{
		LONGLONG st;
		int i, kPortNum, kPortMask;
		//int kAnalogFlag, iDelta, is_negative;
		unsigned long bit;
		unsigned long ui_key;
		
		ui_key = K_UI_NONE;
		
		/************************/
		switch(kaillera_schedule)
		{
		case KS_RESET:
			ui_key = K_UI_RESET_MACHINE;
			break;
		case KS_SAVE:
			if (kcrccounter == 0)
				ui_key = K_UI_AUTO_SAVE;
			break;
		case KS_CRC:
			{
				UINT32 crc = fgetsavecrc(Machine->gamedrv->name, "~");
				char	buf[256];
				sprintf(buf, "%08X", crc);
				SendKailleraCommand(KC_savecrc, buf);
			}
			break;
		case KS_DIP:
			ui_key = K_UI_APPLY_DIP;
			break;
		default:
			if(kPlayerNB == 1)	/* P1에게만 허용한다 */
			{
				if(input_ui_pressed(IPT_UI_RESET_MACHINE)) 		ui_key = K_UI_RESET_MACHINE;
				else if(input_ui_pressed(IPT_UI_P1_SHIFT_DOWN)) ui_key = K_UI_SHIFT_DOWN;
				else if(input_ui_pressed(IPT_UI_P1_SHIFT_UP)) 	ui_key = K_UI_SHIFT_UP;
				else if(input_ui_pressed(IPT_UI_SWAP_CONTROL)) 	ui_key = K_UI_SWAP_CONTROL;
				else if(input_ui_pressed(IPT_UI_LOAD_STATE) && kcrccounter == 0) 	ui_key = K_UI_LOAD_STATE;
				else if(input_ui_pressed(IPT_UI_LOAD_AUTOSAVE) && kcrccounter == 0) ui_key = K_UI_LOAD_AUTOSAVE;
				else if(input_ui_pressed(IPT_UI_SAVE_STATE) && kcrccounter == 0) 	ui_key = K_UI_SAVE_STATE;
				else if(input_ui_pressed(IPT_UI_PAUSE) && !KailleraChatIsActive()) 	ui_key = K_UI_PAUSE;
			}
		}

		if (kaillera_schedule != KS_CLOSE) kaillera_schedule = KS_NONE;

		if(ui_key == K_UI_NONE) {
			switch(control_type) {

			case 0:	
				// 일반 키보드 입력 ---------------------------------------------
				// 16비트 버퍼인 val[]에 버튼의상태(8버튼), 방향키(4방향), 각종
				// 코인버튼 및 서비스버튼의 상태를 16비트에 걸쳐서 세팅한다.
				// --------------------------------------------------------------
				_kInputQueue[0] = 0;

				bit = 1;
				
				//kBitsPlayValues[player][pos][0]=portnum;
				//kBitsPlayValues[player][pos][1]=port->mask;
				//kBitsPlayValues[player][pos][2]=port->default_value;
				//kBitsPlayValues[3][11][4] = portnum;
				//kBitsPlayValues[3][11][5] = port->mask;
				
				for (i = 0; i < 15; i++)
				{
					// player = 0 : P1의 입력결과만 버퍼링
					kPortNum  = kBitsPlayValues[0][i][0];
					kPortMask = kBitsPlayValues[0][i][1];
					
					if (kPortNum != -1) {
						// 키가 눌러지는 순간 input_port_value의 해당 비트가 1 -> 0로 변하므로
						// ~(not)연산 & 마스킹하여 키가 눌러졌는지 검사한다.
						// ex) Coin1 : 000000ff -> 0000007f
						if (~input_port_value[kPortNum] & kPortMask) {
							_kInputQueue[0] |= bit;
						}
					}
					
					bit <<= 1;
				}
				break;
				
			case 1:	
				/*
				// Analog 입력 --------------------------------------------------
				// 32비트 버퍼인 val2[]에 상위 16비트에는 각종 버튼의 상태를,
				// 하위 16비트에는 8비트씩 나누어 x축 delta와 y축 delta값을
				// 세팅한다.
				// --------------------------------------------------------------		
				
				val2[0] = 0;
				bit = 1;
				
				// 버튼의 상태(3버튼)
				for (i = 0; i < 3; i++)
				{
					// player = 0 : P1의 입력결과만 버퍼링
					kPortNum  = kBitsPlayValues[0][i][0];
					kPortMask = kBitsPlayValues[0][i][1];
					if (kPortNum != -1) {
						if (~input_port_value[kPortNum] & kPortMask) val2[0] |= bit;
					}
					bit <<= 1;
				}

				// analog 입력의 상태(4방향)
				// PADDLE/DIAL/TRACKBALL/AD_STICK/LIGHTGUN/PEDAL
				for (i = 3; i < 7; i++)
				{
					// player = 0 : P1의 입력결과만 버퍼링
					kPortNum    = kBitsPlayValues[0][i][0];
					kAnalogFlag = kBitsPlayValues[0][i][3];
					if (kPortNum != -1) {
						if (input_port_value[kPortNum] & kAnalogFlag) val2[0] |= bit;
					}
					bit <<= 1;
				}
				
				// JOYSTICK 및 코인버튼/서비스버튼의 상태
				for (i = 7; i < 15; i++)
				{
					kPortNum  = kBitsPlayValues[0][i][0];
					kPortMask = kBitsPlayValues[0][i][1];
					if (kPortNum != -1) {
						if (~input_port_value[kPortNum] & kPortMask) val2[0] |= bit;
					}
					bit <<= 1;
				}

				// append mouse positions to the end of val2
				// x축 Delta값 8비트 (데이터 7비트 + 부호 1비트)
				val2[0] = val2[0] << 7;
				iDelta = mouse_delta_axis[0][0];
				is_negative = 0;
				if( iDelta < 0 ) { is_negative = 1; iDelta = -Delta; }
				
				if (iDelta > 0 && iDelta < 128) {	val2[0]  += iDelta;	} 
				else if (iDelta >= 128) 		{	val2[0]  += 127;	} 
				val2[0] <<= 1;
				val2[0]  |= is_negative;
				
				// y축 Delta값 8비트 (데이터 7비트 + 부호 1비트)
				val2[0] = val2[0] << 7;
				iDelta = mouse_delta_axis[0][1];
				is_negative = 0;
				if( iDelta < 0 ) { is_negative = 1; iDelta = -Delta; }
				
				if (iDelta > 0 && iDelta < 128) {	val2[0]  += iDelta;	} 
				else if (iDelta >= 128) 		{	val2[0]  += 127;	} 
				val2[0] <<= 1;
				val2[0]  |= is_negative;
				*/
				break; 
			
			case 2:	
				// 게임에 따른 특수키 입력 (위의 _kInpData[] 리스트 참조) -------
				// 32비트 버퍼인 val2[]에 각종 버튼의 상태를 30비트에 걸쳐서 세팅한다.
				// 포커게임과 같은 별도의 추가적인 버튼들이 있는 경우와 같이
				// 특수한 상황이 여기에 해당하는 듯 함.
				// --------------------------------------------------------------		
				_kInputQueue[0] = 0;
				
				bit = 1;
				
				for (i = 0; i < 30; i++)
				{
					// player = 0 : P1의 입력결과만 버퍼링
					kPortNum  = kBitsPlayValues[0][i][0];
					kPortMask = kBitsPlayValues[0][i][1];
					if (kPortNum != -1) {
						//if (input_port_value[kPortNum] & kPortMask) {
						if (~input_port_value[kPortNum] & kPortMask) {
							_kInputQueue[0] |= bit;
						}
					}
					bit <<= 1;
				}
				break;
			}
		} 
		else 
		{
			// ui_key != K_UI_NONE 인 경우
			_kInputQueue[0] = ui_key | K_UI_UIBIT;
		}

		st=cycle_counter();
		recvLen=net_ModifyPlayValues( _kInputQueue, sizeof(UINT32) );
		addClock( cycle_counter() - st );
	}

	return recvLen;
	
}

void shift_down(void)
{
	if(p1_pos == 1)
	{
	    change_position(1,0,2,3,4,5);
	    p1_pos = 2;
	    control_counter = 3;
	    usrintf_showmessage("P1 Shift Down: 2*34");
	}
	else if(p1_pos == 2)
	{
	    change_position(2,0,1,3,4,5);
	    p1_pos = 3;
	    control_counter = 6;
	    usrintf_showmessage("P1 Shift Down: 23*4");
	}
	else if(p1_pos == 3)
	{
		change_position(3,0,1,2,4,5);
		p1_pos = 4;
		control_counter = 9;
		usrintf_showmessage("P1 Shift Down: 234*");
	}
	else if(p1_pos == 4)
	{
	    control_counter = 9;
	    usrintf_showmessage("P1 Shift Down: 234*");
	}
}

void shift_up(void)
{
	if(p1_pos == 1)
	{
	    control_counter = 0;
	    usrintf_showmessage("P1 Shift Up: *234");
	}
	else if(p1_pos == 2)
	{
	    change_position(0,1,2,3,4,5);
	    p1_pos = 1;
	    control_counter = 0;
	    usrintf_showmessage("P1 Shift Up: *234");
	}
	else if(p1_pos == 3)
	{
	    change_position(1,0,2,3,4,5);
	    p1_pos = 2;
	    control_counter = 3;
	    usrintf_showmessage("P1 Shift Up: 2*34");
	}
	else if(p1_pos == 4)
	{
	    change_position(2,0,1,3,4,5);
	    p1_pos = 3;
	    control_counter = 6;
	    usrintf_showmessage("P1 Shift Up: 23*4");
	}
}

void swap_control(void)
{
	switch(control_counter)
	{
	case 0:
      {
		change_position(0,1,3,2,4,5);
		control_counter++;
		p1_pos = 1;
		usrintf_showmessage("Swap Controls: 1243");
		break;
	}
	case 1:
      {
		change_position(0,2,1,3,4,5);
		control_counter++;
		p1_pos = 1;
		usrintf_showmessage("Swap Controls: 1324");
		break;
	}
	case 2:
	{
		change_position(0,3,2,1,4,5);
		control_counter++;
		p1_pos = 1;
		usrintf_showmessage("Swap Controls: 1432");
		break;
	}
	case 3:
	{
		change_position(1,0,3,2,4,5);
		control_counter++;
		p1_pos = 2;
		usrintf_showmessage("Swap Controls: 2143");
		break;
	}
	case 4:
	{
		change_position(2,0,1,3,4,5);
		control_counter++;
		p1_pos = 3;
		usrintf_showmessage("Swap Controls: 2314");
		break;
	}
	case 5:
	{
		change_position(3,0,2,1,4,5);
		control_counter++;
		p1_pos = 4;
		usrintf_showmessage("Swap Controls: 2431");
		break;
	}
	case 6:
	{
		change_position(1,2,0,3,4,5);
		control_counter++;
		p1_pos = 2;
		usrintf_showmessage("Swap Controls: 3124");
		break;
	}
	case 7:
	{
		change_position(2,1,0,3,4,5);
		control_counter++;
		p1_pos = 3;
		usrintf_showmessage("Swap Controls: 3214");
		break;
	}
	case 8:
	{
		change_position(3,2,0,1,4,5);
		control_counter++;
		p1_pos = 4;
		usrintf_showmessage("Swap Controls: 3421");
		break;
	}
	case 9:
	{
		change_position(1,3,2,0,4,5);
		control_counter++;
		p1_pos = 2;
		usrintf_showmessage("Swap Controls: 4132");
		break;
	}
	case 10:
	{
		change_position(2,1,3,0,4,5);
		control_counter++;
		p1_pos = 3;
		usrintf_showmessage("Swap Controls: 4213");
		break;
	}
	case 11:
	{
		change_position(3,2,1,0,4,5);
		control_counter++;
		p1_pos = 4;
		usrintf_showmessage("Swap Controls: 4321");
		break;
	}
	case 12:
	{
		change_position(0,1,2,3,4,5);
		control_counter = 0;
		p1_pos = 1;
		usrintf_showmessage("Swap Controls: 1234");
		break;
	}
	}
}

void change_position(int pos1, int pos2, int pos3, int pos4, int pos5, int pos6)
{
	p1 = pos1;
	p2 = pos2;
	p3 = pos3;
	p4 = pos4;
	p5 = pos5;
	p6 = pos6;

    // analog control is a little bit different
	switch(p1) {
		case 0: ana_p1 = 0; break;
		case 1: ana_p2 = 0; break;
		case 2: ana_p3 = 0; break;
		case 3: ana_p4 = 0; break;
	}

	switch(p2) {
		case 0: ana_p1 = 1; break;
		case 1: ana_p2 = 1; break;
		case 2: ana_p3 = 1; break;
		case 3: ana_p4 = 1; break;
	}

	switch(p3) {
		case 0: ana_p1 = 2; break;
		case 1: ana_p2 = 2; break;
		case 2: ana_p3 = 2; break;
		case 3: ana_p4 = 2; break;
	}

	switch(p4) {
		case 0: ana_p1 = 3; break;
		case 1: ana_p2 = 3; break;
		case 2: ana_p3 = 3; break;
		case 3: ana_p4 = 3; break;
	}
}

void change_position2(int pos1, int pos2, int pos3, int pos4, int pos5, int pos6, int pos7, int pos8)
{
      p1 = pos1;
      p2 = pos2;
      p3 = pos3;
      p4 = pos4;
	  p5 = pos5;
	  p6 = pos6;
	  p7 = pos7;
	  p8 = pos8;
}

/*************************************
 *
 *	Process other player's UI key
 *
 *************************************/

// 네트웍을 통해 전송받은 모든 플레이어의 입력상태 정보중, 특수 동작키에 대한
// 처리를 한다.
void kProcessUIKey( unsigned long ui_key )
{
	switch(ui_key) {
	// Reset
	case K_UI_RESET_MACHINE:
		if(record)
		{
			write_port_value(record_k,	IPT_UI_RESET_MACHINE);
			special_key = 1;
		}
		machine_reset();
		
		p1_pos = 1;
		p1	= 0;
		p2	= 1;
		p3	= 2;
		p4	= 3;
		p5	= 4;
		p6	= 5;
		p7	= 6;
		p8	= 7;
		ana_p1= 0;
		ana_p2= 1;
		ana_p3= 2;
		ana_p4= 3;
		control_counter = 0;
		init = -1;
		usrintf_showmessage("Reset");
		break;
		
	// Load State
	case K_UI_LOAD_STATE:
		if(record)
		{
			write_port_value(record_k, IPT_UI_LOAD_STATE);
			special_key = 1;
		}
		cpu_loadsave_schedule(LOADSAVE_LOAD, '@');
		usrintf_showmessage("State loaded");
		if (ksavecounter) ksavecounter = 1;
		break;
		
	// Save State
	case K_UI_SAVE_STATE:
		if(record)
		{
			write_port_value(record_k, IPT_UI_SAVE_STATE);
			special_key = 1;
		}
		cpu_loadsave_schedule(LOADSAVE_SAVE, '@');
		usrintf_showmessage("State saved");
		break;
		
	// Auto Save state
	case K_UI_AUTO_SAVE:
		cpu_loadsave_schedule(LOADSAVE_SAVE, '~');
		kcrccounter = 3 * 60;
		kgotcrc = 0;
		kaillera_schedule = KS_CRC;
		break;

	// Load auto save		
	case K_UI_LOAD_AUTOSAVE:
		cpu_loadsave_schedule(LOADSAVE_LOAD, '~');
		usrintf_showmessage("Auto Save loaded");
		if (ksavecounter) ksavecounter = 1;
		break;

	// Pause		
	case K_UI_PAUSE:
		netplay_pause = -netplay_pause;
#ifdef M32K_PLUS
		if(netplay_pause == 1) usrintf_showmessage("%s", ui_getstring(UI_net_pause));
		else if(netplay_pause == -1) usrintf_showmessage("%s", ui_getstring(UI_net_resume));
#else
		if(netplay_pause == 1) usrintf_showmessage("Pause");
		else if(netplay_pause == -1) usrintf_showmessage("Unpause");
#endif		
		break;

	// Change DIP switch		
	case K_UI_APPLY_DIP:
		{
			struct InputPort *port;
			int portnum;
			
			port = Machine->input_ports;
			port++;
			portnum = 0;
			while (port->type != IPT_END && portnum < MAX_INPUT_PORTS)
			{
				while (port->type != IPT_END && port->type != IPT_PORT)
				{
					if (port->type == IPT_DIPSWITCH_NAME)
					{
						kDefValues[portnum] = (kDefValues[portnum] & ~port->mask) | (port->default_value & port->mask);
					}
					port++;
				}
				portnum++;
				if (port->type == IPT_PORT) port++;
			}
#ifdef M32K_PLUS
			usrintf_showmessage("%s", ui_getstring(UI_net_dipswitch_changed));
#else
			usrintf_showmessage("Dipswitch changed");
#endif
		}
		break;

	// Player shift down(플레이어번호 뒤로 이동)
	case K_UI_SHIFT_DOWN:
		shift_down();
		break;

	// Player shift up(플레이어번호 앞으로 이동)
	case K_UI_SHIFT_UP:
		shift_up();
		break;

	// Swap player control (플레이어 위치 바꿈)
	case K_UI_SWAP_CONTROL:
		swap_control();
		break;
	}
}


/*************************************
 *
 *	Apply other player's input status
 *
 *************************************/

// Helper Function 1
INLINE int kGetPlayerNum( int iNum )
{
	int kPlayer;
	switch(iNum)
	{
	case 0: kPlayer = p1; break;
	case 1: kPlayer = p2; break;
	case 2: kPlayer = p3; break;
	case 3: kPlayer = p4; break;
	case 4: kPlayer = p5; break;
	case 5: kPlayer = p6; break;
	case 6: kPlayer = p7; break;
	case 7: kPlayer = p8; break;
	default: kPlayer = 0; break;
	}
	return kPlayer;
} 

// Helper Function 2
INLINE void kSetInputPortValue( UINT32 val, UINT32 bit, UINT32 kPortNum, UINT32 kPortMask, UINT32 kPortDefault, UINT32 kPortNum2, UINT32 kPortMask2 )
{
	if (kPortNum != -1)
	{
		if (kPortDefault != IP_ACTIVE_LOW)
		{
			if (!(val & bit)) {
				input_port_value[kPortNum] |= kPortMask;
	            if (kPortNum2 != -1) {
					input_port_value[kPortNum2] |= kPortMask2;
				}
			}
		}
		else
		{
			if (val & bit) {
				input_port_value[kPortNum] &= ~kPortMask;
	            if (kPortNum2 != -1) {
					input_port_value[kPortNum2] &= ~kPortMask2;
				}
			}
		}
	}
}

// 네트웍을 통해 전송받은 모든 플레이어의 입력상태 정보를 input시스템에 적용한다.
void ApplyKailleraInput( int iRecvLen )
{
	unsigned long bit;
	unsigned long ui_key;
	UINT32 kPlayer, kPortNum, kPortNum2, kPortMask, kPortMask2, kPortDefault;
	int	 i, iSize;
	//int iDelta;
	
	if (!kPlay) return;

	if (iRecvLen == 0) {
		memcpy(kDefValues, input_port_value, sizeof(input_port_value));
		return;
	}
	
	// 수신한 데이터의 수(=플레이어의 수)
	iSize = iRecvLen / sizeof(UINT32);
	
	memcpy(input_port_value, kDefValues, sizeof(input_port_value));
	
	switch(control_type) {                                     
	case 0:
		// 일반 키보드 입력 ---------------------------------------------
		// 전달받은 버퍼인 _kInputQueue[]의 상태에 따라 버튼의상태(8버튼), 
		// 방향키(4방향), 각종 코인버튼 및 서비스버튼의 상태를 세팅한다.
		// --------------------------------------------------------------

		// 전달받은 자료는 플레이어1의 자료부터 순서대로 저장되음
		for (i = 0; i < iSize; i++)
		{
			int j;

			// UI키인 경우 
			if((_kInputQueue[i] & K_UI_UIBIT) != 0)
			{
				ui_key = _kInputQueue[i] & K_UI_MASK;
				kProcessUIKey( ui_key );
				continue;
			}
			
			bit = 1;
			for (j = 0; j < 15; j++)
			{
				// 플레이어번호
				kPlayer = kGetPlayerNum(i);

				kPortNum     = kBitsPlayValues[kPlayer][j][0];
				kPortMask    = kBitsPlayValues[kPlayer][j][1];
				kPortDefault = kBitsPlayValues[kPlayer][j][2];
				kPortNum2    = kBitsPlayValues[kPlayer][j][4];
				kPortMask2   = kBitsPlayValues[kPlayer][j][5];
	
				kSetInputPortValue( _kInputQueue[i], bit, kPortNum, kPortMask, kPortDefault, kPortNum2, kPortMask2 );
				bit <<= 1;
			}
		}
		break;
		
	case 1:
		/*
		// Analog 입력 --------------------------------------------------
		// 전달받은 16비트 버퍼인 val[]의 상태에 따라 버튼의상태(8버튼), 
		// 방향키(4방향), 각종 코인버튼 및 서비스버튼의 상태를 세팅한다.
		// 전달받은 32비트 버퍼인 val2[]의 상태에 따라 각종 버튼과 analog
		// 디바이스의 x축 delta와 y축 delta값을 세팅한다.
		// --------------------------------------------------------------		

		// 전달받은 자료는 플레이어1의 자료부터 순서대로 저장되어 있으며
		// 각 자료의 크기(FieldSize)는 4(32비트 4바이트)이다.
		for (i = 0; i < (iRecvLen / 4); i++)
		{
			int k, is_negative, j;
			
			if((val2[i] & K_UI_UIBIT_L) != 0)
			{
				ui_key = val2[i] & K_UI_MASK_L;
				kProcessUIKey( ui_key );
				continue;
			}
			deltax[i] = 0;
			deltay[i] = 0;
			
			// 최하위 8비트(데이터 7비트 + 부호 1비트)에 해당하는 y축 Delta값부터 추출한다.
			// y position
			is_negative = 0;
			bit = 1;
			if (val2[i] & bit) is_negative = 1;
			val2[i] >>= 1;

			deltay[i] = val2[i] & 0x7f;
			if (is_negative) deltay[i] = -deltay[i];
		
			val2[i] = val2[i] >> 7;

			// x축 Delta값 8비트(데이터 7비트 + 부호 1비트) 추출
			// x position
			is_negative = 0;
			bit = 1;
			if (val2[i] & bit) is_negative = 1;
			val2[i] >>= 1;

			deltax[i] = val2[i] & 0x7f;
			if (is_negative) deltax[i] = -deltax[i];

			val2[i] = val2[i] >> 7;
			
			bit = 1;

			// 버튼의 상태(3버튼)
			for (j = 0; j < 3; j++)
			{
				// 플레이어번호
				kPlayer = kGetPlayerNum(i);
			
				kPortNum     = kBitsPlayValues[kPlayer][j][0];
				kPortMask    = kBitsPlayValues[kPlayer][j][1];
				kPortDefault = kBitsPlayValues[kPlayer][j][2];
				kPortNum2    = kBitsPlayValues[kPlayer][j][4];
				kPortMask2   = kBitsPlayValues[kPlayer][j][5];

				kSetInputPortValue( val2[i], bit, kPortNum, kPortMask, kPortDefault, kPortNum2, kPortMask2 );
				bit <<= 1;
			}
			
			// analog 입력의 상태(4방향)
			// PADDLE/DIAL/TRACKBALL/AD_STICK/LIGHTGUN/PEDAL
			for (j = 3; j < 7; j++)
			{
				bit <<= 1;
			}
			
			// JOYSTICK 및 코인버튼/서비스버튼의 상태
			for (j = 7; j < 15; j++)
			{
				// 플레이어번호
				kPlayer = kGetPlayerNum(i);
			
				kPortNum     = kBitsPlayValues[kPlayer][j][0];
				kPortMask    = kBitsPlayValues[kPlayer][j][1];
				kPortDefault = kBitsPlayValues[kPlayer][j][2];
				kPortNum2    = kBitsPlayValues[kPlayer][j][4];
				kPortMask2   = kBitsPlayValues[kPlayer][j][5];

				kSetInputPortValue( val2[i], bit, kPortNum, kPortMask, kPortDefault, kPortNum2, kPortMask2 );
				bit <<= 1;
			}
		}
		*/
		break;
	
	case 2:
		// 게임에 따른 특수키 입력 (위의 _kInpData[] 리스트 참조) -------
		// 전달받은 버퍼인 _kInputQueue[]의 상태에 따라  각종 버튼의 상태를
		// 세팅한다.
		// 포커게임과 같은 별도의 추가적인 버튼들이 있는 경우와 같이
		// 특수한 상황이 여기에 해당하는 듯 함.
		// --------------------------------------------------------------		
		for (i = 0; i < iSize; i++)
		{
			int j;
				
			if((_kInputQueue[i] & K_UI_UIBIT) != 0)
			{
				ui_key = _kInputQueue[i] & K_UI_MASK;
				kProcessUIKey( ui_key );
				continue;
			}

			bit = 1;
			// 각종 버튼의 상태가 30비트에 나뉘어서 세팅되어 있다.
			for (j = 0; j < 30; j++)
			{
				kPlayer = kGetPlayerNum(i);
					
				kPortNum     = kBitsPlayValues[kPlayer][j][0];
				kPortMask    = kBitsPlayValues[kPlayer][j][1];
				kPortDefault = kBitsPlayValues[kPlayer][j][2];
				kPortNum2    = kBitsPlayValues[kPlayer][j][4];
				kPortMask2   = kBitsPlayValues[kPlayer][j][5];
				
				kSetInputPortValue( _kInputQueue[i], bit, kPortNum, kPortMask, kPortDefault, kPortNum2, kPortMask2 );
				bit <<= 1;
			}
		}
		break;
	} /* end of switch */

}
