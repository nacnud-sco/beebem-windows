/****************************************************************************/
/*                               Beebem                                     */
/*                               ------                                     */
/* This program may be distributed freely within the following restrictions:*/
/*                                                                          */
/* 1) You may not charge for this program or for any part of it.            */
/* 2) This copyright message must be distributed with all copies.           */
/* 3) This program must be distributed complete with source code.  Binary   */
/*    only distribution is not permitted.                                   */
/* 4) The author offers no warrenties, or guarentees etc. - you use it at   */
/*    your own risk.  If it messes something up or destroys your computer   */
/*    thats YOUR problem.                                                   */
/* 5) You may use small sections of code from this program in your own      */
/*    applications - but you must acknowledge its use.  If you plan to use  */
/*    large sections then please ask the author.                            */
/*                                                                          */
/* If you do not agree with any of the above then please do not use this    */
/* program.                                                                 */
/****************************************************************************/
/* Mike Wyatt and NRM's port to win32 - 7/6/97 */

#include <stdio.h>
#include <windows.h>
#include "main.h"
#include "beebwin.h"
#include "port.h"
#include "6502core.h"
#include "disc8271.h"
#include "sysvia.h"
#include "uservia.h"
#include "video.h"
#include "beebsound.h"
#include "beebmem.h"
#include "beebemrc.h"
#include "atodconv.h"
#include "beebstate.h"
#include "userkybd.h"

static const char *WindowTitle = "BBC Emulator";
static const char *AboutText = "BeebEm\nBBC Micro Emulator\n"
								"Version 0.8, 1 Sep 1997\n";

/* Configuration file strings */
static const char *CFG_FILE_NAME = "BeebEm.ini";

static const char *CFG_VIEW_SECTION = "View";
static const char *CFG_VIEW_WIN_SIZE = "WinSize";
static const char *CFG_VIEW_WIN_XPOS = "WinXPos";
static const char *CFG_VIEW_WIN_YPOS = "WinYPos";
static const char *CFG_VIEW_SHOW_FPS = "ShowFSP";

static const char *CFG_SOUND_SECTION = "Sound";
static const char *CFG_SOUND_SAMPLE_RATE = "SampleRate";
static const char *CFG_SOUND_VOLUME = "Volume";
static const char *CFG_SOUND_ENABLED = "SoundEnabled";

static const char *CFG_OPTIONS_SECTION = "Options";
static const char *CFG_OPTIONS_STICKS = "Sticks";
static const char *CFG_OPTIONS_KEY_MAPPING = "KeyMapping";
static const char *CFG_OPTIONS_USER_KEY_MAP = "UserKeyMap";

static const char *CFG_SPEED_SECTION = "Speed";
static const char *CFG_SPEED_TIMING = "Timing";

/* Prototypes */
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int TranslateKey(int, int *, int *);

// Row,Col
static int transTable1[256][2]={
	0,0,	0,0,	0,0,	0,0,   // 0
	0,0,	0,0,	0,0,	0,0,   // 4
	5,9,	6,0,	0,0,	0,0,   // 8 [BS][TAB]..
	0,0,	4,9,	0,0,	0,0,   // 12 .RET..
	0,0,	0,1,	0,0,	-2,-2,	 // 16 .CTRL.BREAK
	4,0,	0,0,	0,0,	0,0,   // 20 CAPS...
	0,0,	0,0,	0,0,	7,0,   // 24 ...ESC
	0,0,	0,0,	0,0,	0,0,   // 28
	6,2,	0,0,	0,0,	6,9,   // 32 SPACE..[End]
	0,0,	1,9,	3,9,	7,9,   // 36 .[Left][Up][Right]
	2,9,	0,0,	0,0,	0,0,   // 40 [Down]...
	0,0,	0,0,	5,9,	0,0,   // 44 ..[DEL].
	2,7,	3,0,	3,1,	1,1,   // 48 0123   
	1,2,	1,3,	3,4,	2,4,   // 52 4567
	1,5,	2,6,	0,0,	0,0,   // 56 89
	0,0,	0,0,	0,0,	0,0,   // 60
	0,0,	4,1,	6,4,	5,2,   // 64.. ABC
	3,2,	2,2,	4,3,	5,3,   // 68  DEFG
	5,4,	2,5,	4,5,	4,6,   // 72  HIJK
	5,6,	6,5,	5,5,	3,6,   // 76  LMNO
	3,7,	1,0,	3,3,	5,1,   // 80  PQRS
	2,3,	3,5,	6,3,	2,1,   // 84  TUVW
	4,2,	4,4,	6,1,	0,0,   // 88  XYZ
	0,0,	6,2,	0,0,	0,0,   // 92  . SPACE ..
	0,0,	0,0,	0,0,	0,0,   // 96
	0,0,	0,0,	0,0,	0,0,   // 100
	0,0,	0,0,	0,0,	0,0,   // 104
	0,0,	0,0,	0,0,	0,0,   // 108
	7,1,	7,2,	7,3,	1,4,   // 112 F1 F2 F3 F4
	7,4,	7,5,	1,6,	7,6,   // 116 F5 F6 F7 F8
	7,7,	2,0,	2,0,	-2,-2,	 // 120 F9 F10 F11 F12
	0,0,	0,0,	0,0,	0,0,   // 124 
	0,0,	0,0,	0,0,	0,0,   // 128
	0,0,	0,0,	0,0,	0,0,   // 132
	0,0,	0,0,	0,0,	0,0,   // 136
	0,0,	0,0,	0,0,	0,0,   // 140
	0,0,	0,0,	0,0,	0,0,   // 144
	0,0,	0,0,	0,0,	0,0,   // 148
	0,0,	0,0,	0,0,	0,0,   // 152
	0,0,	0,0,	0,0,	0,0,   // 156
	0,0,	0,0,	0,0,	0,0,   // 160
	0,0,	0,0,	0,0,	0,0,   // 164
	0,0,	0,0,	0,0,	0,0,   // 168
	0,0,	0,0,	0,0,	0,0,   // 172
	0,0,	0,0,	0,0,	0,0,   // 176
	0,0,	0,0,	0,0,	0,0,   // 180
	0,0,	0,0,	5,7,	1,8,   // 184  ..;=
	6,6,	1,7,	6,7,	6,8,   // 188  ,-./
	4,8,	0,0,	0,0,	0,0,   // 192  @ ...
	0,0,	0,0,	0,0,	0,0,   // 196
	0,0,	0,0,	0,0,	0,0,   // 200
	0,0,	0,0,	0,0,	0,0,   // 204
	0,0,	0,0,	0,0,	0,0,   // 208
	0,0,	0,0,	0,0,	0,0,   // 212
	0,0,	0,0,	0,0,	3,8,   // 216 ...[
	7,8,	5,8,	2,8,	4,7,   // 220 \]#`
	0,0,	0,0,	0,0,	0,0,   // 224
	0,0,	0,0,	0,0,	0,0,   // 228
	0,0,	0,0,	0,0,	0,0,   // 232
	0,0,	0,0,	0,0,	0,0, 
	0,0,	0,0,	0,0,	0,0, 
	0,0,	0,0,	0,0,	0,0, 
	0,0,	0,0,	0,0,	0,0, 
	0,0,	0,0,	0,0,	0,0 
};
	
/* This table is better for games:
    A & S are mapped to Caps Lock and Ctrl
	F1-F9 are mapped to f0-f8 */
static int transTable2[256][2]={
	0,0,	0,0,	0,0,	0,0,   // 0
	0,0,	0,0,	0,0,	0,0,   // 4
	5,9,	6,0,	0,0,	0,0,   // 8 [BS][TAB]..
	0,0,	4,9,	0,0,	0,0,   // 12 .RET..
	0,0,	0,1,	0,0,	-2,-2,	 // 16 .CTRL.BREAK
	4,0,	0,0,	0,0,	0,0,   // 20 CAPS...
	0,0,	0,0,	0,0,	7,0,   // 24 ...ESC
	0,0,	0,0,	0,0,	0,0,   // 28
	6,2,	0,0,	0,0,	6,9,   // 32 SPACE..[End]
	0,0,	1,9,	3,9,	7,9,   // 36 .[Left][Up][Right]
	2,9,	0,0,	0,0,	0,0,   // 40 [Down]...
	0,0,	0,0,	5,9,	0,0,   // 44 ..[DEL].
	2,7,	3,0,	3,1,	1,1,   // 48 0123   
	1,2,	1,3,	3,4,	2,4,   // 52 4567
	1,5,	2,6,	0,0,	0,0,   // 56 89
	0,0,	0,0,	0,0,	0,0,   // 60
	0,0,	4,0,	6,4,	5,2,   // 64  . CAPS B C
	3,2,	2,2,	4,3,	5,3,   // 68  DEFG
	5,4,	2,5,	4,5,	4,6,   // 72  HIJK
	5,6,	6,5,	5,5,	3,6,   // 76  LMNO
	3,7,	1,0,	3,3,	0,1,   // 80  PQR CTRL
	2,3,	3,5,	6,3,	2,1,   // 84  TUVW
	4,2,	4,4,	6,1,	6,2,   // 88  XYZ SPACE
	6,2,	6,2,	0,0,	0,0,   // 92  SPACE SPACE ..
	0,0,	0,0,	0,0,	0,0,   // 96
	0,0,	0,0,	0,0,	0,0,   // 100
	0,0,	0,0,	0,0,	0,0,   // 104
	0,0,	0,0,	0,0,	0,0,   // 108
	2,0,	7,1,	7,2,	7,3,   // 112 F1 F2 F3 F4      - f0 f1 f2 f3
	1,4,	7,4,	7,5,	1,6,   // 116 F5 F6 F7 F8      - f4 f5 f6 f7
	7,6,	2,0,	7,7,	-2,-2,	 // 120 F9 F10 F11 F12 - f8 .  f9 BREAK
	0,0,	0,0,	0,0,	0,0,   // 124 
	0,0,	0,0,	0,0,	0,0,   // 128
	0,0,	0,0,	0,0,	0,0,   // 132
	0,0,	0,0,	0,0,	0,0,   // 136
	0,0,	0,0,	0,0,	0,0,   // 140
	0,0,	0,0,	0,0,	0,0,   // 144
	0,0,	0,0,	0,0,	0,0,   // 148
	0,0,	0,0,	0,0,	0,0,   // 152
	0,0,	0,0,	0,0,	0,0,   // 156
	0,0,	0,0,	0,0,	0,0,   // 160
	0,0,	0,0,	0,0,	0,0,   // 164
	0,0,	0,0,	0,0,	0,0,   // 168
	0,0,	0,0,	0,0,	0,0,   // 172
	0,0,	0,0,	0,0,	0,0,   // 176
	0,0,	0,0,	0,0,	0,0,   // 180
	0,0,	0,0,	5,7,	1,8,   // 184  ..;=
	6,6,	1,7,	6,7,	6,8,   // 188  ,-./
	4,8,	0,0,	0,0,	0,0,   // 192  @ ...
	0,0,	0,0,	0,0,	0,0,   // 196
	0,0,	0,0,	0,0,	0,0,   // 200
	0,0,	0,0,	0,0,	0,0,   // 204
	0,0,	0,0,	0,0,	0,0,   // 208
	0,0,	0,0,	0,0,	0,0,   // 212
	0,0,	0,0,	0,0,	3,8,   // 216 ...[
	7,8,	5,8,	2,8,	4,7,   // 220 \]#`
	0,0,	0,0,	0,0,	0,0,   // 224
	0,0,	0,0,	0,0,	0,0,   // 228
	0,0,	0,0,	0,0,	0,0,   // 232
	0,0,	0,0,	0,0,	0,0, 
	0,0,	0,0,	0,0,	0,0, 
	0,0,	0,0,	0,0,	0,0, 
	0,0,	0,0,	0,0,	0,0, 
	0,0,	0,0,	0,0,	0,0 
};

/* Currently selected translation table */
static int (*transTable)[2] = transTable1;
				    
/****************************************************************************/
BeebWin::BeebWin()
{   
	char CfgName[256];
	char CfgValue[256];
	char DefValue[256];
	int row, col;

	sprintf(DefValue, "%d", IDM_320X256);
	GetPrivateProfileString(CFG_VIEW_SECTION, CFG_VIEW_WIN_SIZE, DefValue,
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	m_MenuIdWinSize = atoi(CfgValue);
	TranslateWindowSize();

	GetPrivateProfileString(CFG_VIEW_SECTION, CFG_VIEW_SHOW_FPS, "1",
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	m_ShowSpeedAndFPS = atoi(CfgValue);

	sprintf(DefValue, "%d", IDM_REALTIME);
	GetPrivateProfileString(CFG_SPEED_SECTION, CFG_SPEED_TIMING, DefValue,
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	m_MenuIdTiming = atoi(CfgValue);
	TranslateTiming();

	sprintf(DefValue, "%d", IDM_22050KHZ);
	GetPrivateProfileString(CFG_SOUND_SECTION, CFG_SOUND_SAMPLE_RATE, DefValue,
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	m_MenuIdSampleRate = atoi(CfgValue);
	TranslateSampleRate();

	sprintf(DefValue, "%d", IDM_MEDIUMVOLUME);
	GetPrivateProfileString(CFG_SOUND_SECTION, CFG_SOUND_VOLUME, DefValue,
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	m_MenuIdVolume = atoi(CfgValue);
	TranslateVolume();

	GetPrivateProfileString(CFG_SOUND_SECTION, CFG_SOUND_ENABLED, "0",
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	SoundEnabled = atoi(CfgValue);
	if (SoundEnabled)
		SoundInit();

	GetPrivateProfileString(CFG_OPTIONS_SECTION, CFG_OPTIONS_STICKS, "0",
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	m_MenuIdSticks = atoi(CfgValue);

	m_HideCursor = FALSE;

	sprintf(DefValue, "%d", IDM_KEYBOARDMAPPING1);
	GetPrivateProfileString(CFG_OPTIONS_SECTION, CFG_OPTIONS_KEY_MAPPING, DefValue,
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	m_MenuIdKeyMapping = atoi(CfgValue);
	TranslateKeyMapping();

	for (int key=0; key<256; ++key)
	{
		sprintf(CfgName, "%s%d", CFG_OPTIONS_USER_KEY_MAP, key);
		GetPrivateProfileString(CFG_OPTIONS_SECTION, CfgName, "-1 -1",
				CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
		sscanf(CfgValue, "%d %d", &row, &col);
		if (row != -1 && col != -1)
		{
			UserKeymap[key][0] = row;
			UserKeymap[key][1] = col;
		}
	}

	m_DiscTypeSelection = 1;
	IgnoreIllegalInstructions = 1;

	m_WriteProtectDisc[0] = !IsDiscWritable(0);
	m_WriteProtectDisc[1] = !IsDiscWritable(1);

	m_hBitmap = m_hOldObj = m_hDCBitmap = NULL;
	m_ScreenRefreshCount = 0;
	m_RelativeSpeed = 1;
	m_FramesPerSecond = 50;
	strcpy(m_szTitle, WindowTitle);

	for(int i=0;i<8;i++)
		cols[i] = i;

	CreateBitmap();
	InitClass();
	CreateBeebWindow(); 
	InitMenu();

	/* Joysticks can only be initialised after the window is created (needs hwnd) */
	if (m_MenuIdSticks == IDM_JOYSTICK)
		InitJoystick();

	m_hDC = GetDC(m_hWnd);

	/* Get the applications path - used for disc and state operations */
	char app_path[_MAX_PATH];
	char app_drive[_MAX_DRIVE];
	char app_dir[_MAX_DIR];
	GetModuleFileName(NULL, app_path, _MAX_PATH);
	_splitpath(app_path, app_drive, app_dir, NULL, NULL);
	_makepath(m_AppPath, app_drive, app_dir, NULL, NULL);
}

/****************************************************************************/
BeebWin::~BeebWin()
{   
	if (SoundEnabled)
		SoundReset();
	
	ReleaseDC(m_hWnd, m_hDC);

	if (m_hOldObj != NULL)
		SelectObject(m_hDCBitmap, m_hOldObj);
	if (m_hBitmap != NULL)
		DeleteObject(m_hBitmap);
	if (m_hDCBitmap != NULL)
		DeleteDC(m_hDCBitmap);
}

/****************************************************************************/
void BeebWin::CreateBitmap()
{
	m_hDCBitmap = CreateCompatibleDC(NULL);

	m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bmi.bmiHeader.biWidth = 640;
	m_bmi.bmiHeader.biHeight = -256;
	m_bmi.bmiHeader.biPlanes = 1;
	m_bmi.bmiHeader.biBitCount = 8;     
	m_bmi.bmiHeader.biCompression = BI_RGB;
	m_bmi.bmiHeader.biSizeImage = 640*256;
	m_bmi.bmiHeader.biClrUsed = 8;
	m_bmi.bmiHeader.biClrImportant = 8;

#ifdef USE_PALETTE
	__int16 *pInts = (__int16 *)&m_bmi.bmiColors[0];
    
	for(int i=0; i<8; i++)
		pInts[i] = i;
    
	m_hBitmap = CreateDIBSection(m_hDCBitmap, (BITMAPINFO *)&m_bmi, DIB_PAL_COLORS,
							(void**)&m_screen, NULL,0);
#else
	for (int i = 0; i < 8; ++i)
	{
		m_bmi.bmiColors[i].rgbRed = (i & 1) * 255;
		m_bmi.bmiColors[i].rgbGreen = ((i & 2) >> 1) * 255;
		m_bmi.bmiColors[i].rgbBlue = ((i & 4) >> 1) * 255;
		m_bmi.bmiColors[i].rgbReserved = 0;
	}

	m_hBitmap = CreateDIBSection(m_hDCBitmap, (BITMAPINFO *)&m_bmi, DIB_RGB_COLORS,
							(void**)&m_screen, NULL,0);
#endif

	m_hOldObj = SelectObject(m_hDCBitmap, m_hBitmap);
	if(m_hOldObj == NULL)
		MessageBox(NULL,"Cannot select the screen bitmap\n"
					"Try running in a 256 colour mode",WindowTitle,MB_OK|MB_ICONERROR);
}

/****************************************************************************/
BOOL BeebWin::InitClass(void)
{
	WNDCLASS  wc;

	// Fill in window class structure with parameters that describe the
	// main window.

	wc.style		 = CS_HREDRAW | CS_VREDRAW;// Class style(s).
	wc.lpfnWndProc	 = (WNDPROC)WndProc;	   // Window Procedure
	wc.cbClsExtra	 = 0;					   // No per-class extra data.
	wc.cbWndExtra	 = 0;					   // No per-window extra data.
	wc.hInstance	 = hInst;				   // Owner of this class
	wc.hIcon		 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BEEBEM));
	wc.hCursor		 = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);// Default color
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU); // Menu from .RC
	wc.lpszClassName = "BEEBWIN"; //szAppName;				// Name to register as

	// Register the window class and return success/failure code.
	return (RegisterClass(&wc));
}

/****************************************************************************/
void BeebWin::CreateBeebWindow(void)
{
	int x,y;
	char CfgValue[256];

	GetPrivateProfileString(CFG_VIEW_SECTION, CFG_VIEW_WIN_XPOS, "-1",
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	x=atoi(CfgValue);
	GetPrivateProfileString(CFG_VIEW_SECTION, CFG_VIEW_WIN_YPOS, "-1",
			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
	y=atoi(CfgValue);
	if (x == -1 || y == -1)
	{
		x = CW_USEDEFAULT;
		y = 0;
	}

	m_hWnd = CreateWindow(
				"BEEBWIN",				// See RegisterClass() call.
				m_szTitle, 		// Text for window title bar.
				WS_OVERLAPPED|				  
				WS_CAPTION|
				WS_SYSMENU|
				WS_MINIMIZEBOX, // Window style.			    
				x, y,
				m_XWinSize + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
				m_YWinSize + GetSystemMetrics(SM_CYFIXEDFRAME) * 2
					+ GetSystemMetrics(SM_CYMENUSIZE)
					+ GetSystemMetrics(SM_CYCAPTION)
					+ 1,
				NULL,					// Overlapped windows have no parent.
				NULL,				 // Use the window class menu.
				hInst,			 // This instance owns this window.
				NULL				 // We don't use any data in our WM_CREATE
		); 

	ShowWindow(m_hWnd, SW_SHOW); // Show the window
	UpdateWindow(m_hWnd);		  // Sends WM_PAINT message
}

/****************************************************************************/
void BeebWin::InitMenu(void)
{
	HMENU hMenu = GetMenu(m_hWnd);

	CheckMenuItem(hMenu, IDM_SPEEDANDFPS, m_ShowSpeedAndFPS ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, m_MenuIdWinSize, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_SOUNDONOFF, SoundEnabled ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, m_MenuIdSampleRate, MF_CHECKED);
	CheckMenuItem(hMenu, m_MenuIdVolume, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_ALLOWALLROMWRITES, WritableRoms ? MF_CHECKED : MF_UNCHECKED); 
	CheckMenuItem(hMenu, m_MenuIdTiming, MF_CHECKED);
	if (m_MenuIdSticks != 0)
		CheckMenuItem(hMenu, m_MenuIdSticks, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_HIDECURSOR, m_HideCursor ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_IGNOREILLEGALOPS,
					IgnoreIllegalInstructions ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, m_MenuIdKeyMapping, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_WPDISC0, m_WriteProtectDisc[0] ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WPDISC1, m_WriteProtectDisc[1] ? MF_CHECKED : MF_UNCHECKED);

	/* Initialise the ROM Menu. */
	SetRomMenu();
}

/****************************************************************************/
void BeebWin::SetRomMenu(void)
{
	HMENU hMenu = GetMenu(m_hWnd);

	// Set the ROM Titles in the ROM/RAM menu.
	CHAR Title[19];
	
	int	 i;

	for( i=0; i<16; i++ )
	{
		Title[0] = '&';
		_itoa( i, &Title[1], 16 );
		Title[2] = ' ';
		
		// Get the Rom Title.
		ReadRomTitle( i, &Title[3], sizeof( Title )-4);
	
		if ( Title[3]== '\0' )
			strcpy( &Title[3], "Empty" );

		ModifyMenu( hMenu,	// handle of menu 
					IDM_ALLOWWRITES_ROM0 + i,
					MF_BYCOMMAND,	// menu item to modify
				//	MF_STRING,	// menu item flags 
					IDM_ALLOWWRITES_ROM0 + i,	// menu item identifier or pop-up menu handle
					Title		// menu item content 
					);
	}

	/* LRW Now uncheck the Roms which are NOT writable, that have already been loaded. */
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM0, RomWritable[0] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM1, RomWritable[1] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM2, RomWritable[2] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM3, RomWritable[3] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM4, RomWritable[4] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM5, RomWritable[5] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM6, RomWritable[6] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM7, RomWritable[7] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM8, RomWritable[8] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM9, RomWritable[9] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROMA, RomWritable[10] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROMB, RomWritable[11] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROMC, RomWritable[12] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROMD, RomWritable[13] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROME, RomWritable[14] ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROMF, RomWritable[15] ? MF_CHECKED : MF_UNCHECKED );
}

/****************************************************************************/
void BeebWin::GetRomMenu(void)
{
	HMENU hMenu = GetMenu(m_hWnd);

	/* LRW Now uncheck the Roms as NOT writable, that have already been loaded. */
	RomWritable[0] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM0, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[1] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM1, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[2] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM2, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[3] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM3, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[4] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM4, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[5] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM5, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[6] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM6, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[7] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM7, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[8] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM8, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[9] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM9, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[10] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROMA, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[11] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROMB, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[12] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROMC, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[13] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROMD, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[14] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROME, MF_BYCOMMAND ) & MF_CHECKED );
	RomWritable[15] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROMF, MF_BYCOMMAND ) & MF_CHECKED );
}

/****************************************************************************/
void BeebWin::GreyRomMenu(BOOL SetToGrey)
{
	HMENU hMenu = GetMenu(m_hWnd);

	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROM1, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROM2, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROM3, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROM4, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROM5, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROM6, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROM7, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROM8, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROM9, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROMA, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROMB, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROMC, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROMD, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROME, SetToGrey ? MF_GRAYED : MF_ENABLED );
	EnableMenuItem(hMenu, IDM_ALLOWWRITES_ROMF, SetToGrey ? MF_GRAYED : MF_ENABLED );
}

/****************************************************************************/
void BeebWin::InitJoystick(void)
{
	MMRESULT mmresult;

	/* Get joystick updates 10 times a second */
	mmresult = joySetCapture(m_hWnd, JOYSTICKID1, 100, FALSE);
	if (mmresult == JOYERR_NOERROR)
		mmresult = joyGetDevCaps(JOYSTICKID1, &m_JoystickCaps, sizeof(JOYCAPS));

	if (mmresult == JOYERR_NOERROR)
	{
		AtoDInit();
	}
	else if (mmresult == JOYERR_UNPLUGGED)
	{
		MessageBox(m_hWnd, "Joystick is not plugged in",
					WindowTitle, MB_OK|MB_ICONERROR);
	}
	else
	{
		MessageBox(m_hWnd, "Failed to initialise the joystick",
					WindowTitle, MB_OK|MB_ICONERROR);
	}
}

/****************************************************************************/
void BeebWin::ScaleJoystick(unsigned int x, unsigned int y)
{
	/* Scale and reverse the readings */
	JoystickX = (int)((double)(m_JoystickCaps.wXmax - x) * 65536.0 /
						(double)(m_JoystickCaps.wXmax - m_JoystickCaps.wXmin));
	JoystickY = (int)((double)(m_JoystickCaps.wYmax - y) * 65536.0 /
						(double)(m_JoystickCaps.wYmax - m_JoystickCaps.wYmin));
}

/****************************************************************************/
void BeebWin::ResetJoystick(void)
{
	joyReleaseCapture(JOYSTICKID1);
	AtoDReset();
}

/****************************************************************************/
void BeebWin::SetMousestickButton(int button)
{
	if (m_MenuIdSticks == IDM_MOUSESTICK)
		JoystickButton = button;
}

/****************************************************************************/
void BeebWin::ScaleMousestick(unsigned int x, unsigned int y)
{
	if (m_MenuIdSticks == IDM_MOUSESTICK)
	{
		JoystickX = (m_XWinSize - x - 1) * 65536 / m_XWinSize;
		JoystickY = (m_YWinSize - y - 1) * 65536 / m_YWinSize;
	}

	if (m_HideCursor)
		SetCursor(NULL);
}

/****************************************************************************/
LRESULT CALLBACK WndProc(
				HWND hWnd,		   // window handle
				UINT message,	   // type of message
				WPARAM uParam,	   // additional information
				LPARAM lParam)	   // additional information
{
	int wmId, wmEvent;
	HDC hdc;
	int row, col;

	switch (message)
	{
		case WM_COMMAND:  // message: command from application menu
			wmId	= LOWORD(uParam);
			wmEvent = HIWORD(uParam);
			if (mainWin)
				mainWin->HandleCommand(wmId);
			break;						  

		case WM_PALETTECHANGED:
			if(!mainWin)
				break;
			if ((HWND)uParam == hWnd)
				break;

			// fall through to WM_QUERYNEWPALETTE
		case WM_QUERYNEWPALETTE:
			if(!mainWin)
				break;

			hdc = GetDC(hWnd);
			mainWin->RealizePalette(hdc);					    
			ReleaseDC(hWnd,hdc);    
			return TRUE;							    
			break;

		case WM_PAINT:
			if(mainWin != NULL) {
				PAINTSTRUCT ps;
				HDC 		hDC;

				hDC = BeginPaint(hWnd, &ps);
				mainWin->RealizePalette(hDC);
				mainWin->updateLines(hDC, 0, 256);
				EndPaint(hWnd, &ps);
			}
			break;

		case WM_KEYDOWN:
			if(TranslateKey(uParam, &row, &col)>=0)
				BeebKeyDown(row, col);
			break;

		case WM_KEYUP:
			if(TranslateKey(uParam,&row, &col)>=0)
				BeebKeyUp(row, col);
			else if(row==-2)
			{ // Must do a reset!
				Init6502core();
				Disc8271_reset();
			}
			break;					  

		case WM_KILLFOCUS:
			BeebReleaseAllKeys();
			break;					  

		case MM_JOY1MOVE:
			if (mainWin)
				mainWin->ScaleJoystick(LOWORD(lParam), HIWORD(lParam));
			break;

		case MM_JOY1BUTTONDOWN:
		case MM_JOY1BUTTONUP:
			JoystickButton = ((UINT)uParam & (JOY_BUTTON1 | JOY_BUTTON2)) ? 1 : 0;
			break; 

		case WM_MOUSEMOVE:
			if (mainWin)
				mainWin->ScaleMousestick(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			if (mainWin)
				mainWin->SetMousestickButton(((UINT)uParam & MK_LBUTTON) ? TRUE : FALSE);
			break;

		case WM_DESTROY:  // message: window being destroyed
			PostQuitMessage(0);
			break;

		default:		  // Passes it on if unproccessed
			return (DefWindowProc(hWnd, message, uParam, lParam));
		}
	return (0);
}

/****************************************************************************/
int TranslateKey(int vkey, int *row, int *col)
{
	row[0] = transTable[vkey][0];
	col[0] = transTable[vkey][1];   

	return(row[0]);
}

/****************************************************************************/
int BeebWin::StartOfFrame(void)
{
	int FrameNum = 1;

	if (UpdateTiming())
		FrameNum = 0;

	return FrameNum;
}

/****************************************************************************/
void BeebWin::updateLines(HDC hDC, int starty, int nlines)
{
	++m_ScreenRefreshCount;

	if (m_MenuIdWinSize == IDM_640X256)
	{
		BitBlt(hDC, 0, starty, 640, nlines, m_hDCBitmap, 0, starty, SRCCOPY);
	}
	else
	{
		int win_starty = starty * m_YWinSize / 256;
		int win_nlines = nlines * m_YWinSize / 256;
		StretchBlt(m_hDC, 0, win_starty, m_XWinSize, win_nlines,
				m_hDCBitmap, 0, starty, 640, nlines, SRCCOPY);
	}
}

/****************************************************************************/
BOOL BeebWin::UpdateTiming(void)
{
	static unsigned long LastTotalCycles = 0;
	static unsigned long LastTickCount = 0;
	static unsigned long LastSleepCount = 0;
	static unsigned long LastFPSCount = 0;
	static double FrameUpdateTotal = 0.0;
	static double FrameUpdateIncrement = 1.0;
	double AdjustedRelativeSpeed;
	unsigned long Cycles;
	unsigned long Ticks, TickCount;
	BOOL UpdateScreen = TRUE;

	if (LastTickCount == 0)
	{
		LastSleepCount = LastTickCount = GetTickCount();
		LastTotalCycles = TotalCycles;
	}
	else
	{
		/* Only update timings every second */
		TickCount = GetTickCount();
		if (TickCount >= LastTickCount + 1000)
		{
			Ticks = TickCount - LastTickCount;

			/* Limit the number ticks otherwise timing gets messed up after a
				long pause due to menu commands. */
			if (Ticks > 1500)
				Ticks = 1500;

			if ((unsigned long)TotalCycles < LastTotalCycles)
			{
				/* Wrap around in cycle count */
				Cycles = TotalCycles + (CycleCountTMax - LastTotalCycles);
			}
			else
			{	
				Cycles = TotalCycles - LastTotalCycles;
			}

			/* Ticks are in ms, Cycles are in 0.5 us (beeb runs at 2MHz) */
			m_RelativeSpeed = (Cycles / 2000.0) / Ticks;
			m_FramesPerSecond += (m_ScreenRefreshCount * 1000.0) / Ticks;
			m_FramesPerSecond /= 2.0;
			m_ScreenRefreshCount = 0;

			AdjustedRelativeSpeed = m_RelativeSpeed + 1.0 - m_RealTimeTarget;
			FrameUpdateIncrement *= AdjustedRelativeSpeed * AdjustedRelativeSpeed;
			if (FrameUpdateIncrement < 0.05)
				FrameUpdateIncrement = 0.05;

			DisplayTiming();

			LastTotalCycles = TotalCycles;
			LastTickCount = TickCount;
		}

		if (m_FPSTarget == 0)
		{
			/* One of the real time speeds required */
			if (FrameUpdateIncrement > 1.0)
			{
				/* Sleep for a bit */
				if (TickCount >= LastSleepCount + 200)
				{
					if (FrameUpdateIncrement > 201.0)
						Sleep(200);
					else
						Sleep((long)(FrameUpdateIncrement - 1.0));
					LastSleepCount = TickCount;
				}
				UpdateScreen = TRUE;
			}
			else
			{
				FrameUpdateTotal += FrameUpdateIncrement;
				if (FrameUpdateTotal >= 1.0)
				{
					UpdateScreen = TRUE;
					FrameUpdateTotal -= 1.0;
				}
				else
				{
					UpdateScreen = FALSE;
				}
			}
		}
		else
		{
			/* Fast as possible with a certain frame rate */
			if (TickCount >= LastFPSCount + (1000 / m_FPSTarget))
			{
				UpdateScreen = TRUE;
				LastFPSCount = TickCount;
			}
			else
			{
				UpdateScreen = FALSE;
			}
		}
	}

	return UpdateScreen;
}

/****************************************************************************/
void BeebWin::DisplayTiming(void)
{
	if (m_ShowSpeedAndFPS)
	{
		sprintf(m_szTitle, "%s  Speed: %2.2f  fps: %2.2f",
				WindowTitle, m_RelativeSpeed, m_FramesPerSecond);
		SetWindowText(m_hWnd, m_szTitle);
	}
}

/****************************************************************************/
void BeebWin::TranslateWindowSize(void)
{
	switch (m_MenuIdWinSize)
	{
	case IDM_160X128:
		m_XWinSize = 160;
		m_YWinSize = 128;
		break;

	case IDM_240X192:
		m_XWinSize = 240;
		m_YWinSize = 192;
		break;

	case IDM_640X256:
		m_XWinSize = 640;
		m_YWinSize = 256;
		break;

	default:
	case IDM_320X256:
		m_XWinSize = 320;
		m_YWinSize = 256;
		break;

	case IDM_640X512:
		m_XWinSize = 640;
		m_YWinSize = 512;
		break;

	case IDM_800X600:
		m_XWinSize = 800;
		m_YWinSize = 600;
		break;

	case IDM_1024X768:
		m_XWinSize = 1024;
		m_YWinSize = 768;
		break;

	case IDM_1024X512:
		m_XWinSize = 1024;
		m_YWinSize = 512;
		break;
	}
}

/****************************************************************************/
void BeebWin::TranslateSampleRate(void)
{
	switch (m_MenuIdSampleRate)
	{
	case IDM_44100KHZ:
		SoundSampleRate = 44100;
		break;

	default:
	case IDM_22050KHZ:
		SoundSampleRate = 22050;
		break;

	case IDM_11025KHZ:
		SoundSampleRate = 11025;
		break;
	}
}

/****************************************************************************/
void BeebWin::TranslateVolume(void)
{
	switch (m_MenuIdVolume)
	{
	case IDM_FULLVOLUME:
		SoundVolume = 1;
		break;

	case IDM_HIGHVOLUME:
		SoundVolume = 2;
		break;

	default:
	case IDM_MEDIUMVOLUME:
		SoundVolume = 3;
		break;

	case IDM_LOWVOLUME:
		SoundVolume = 4;
		break;
	}
}

/****************************************************************************/
void BeebWin::TranslateTiming(void)
{
	m_FPSTarget = 0;
	m_RealTimeTarget = 1.0;

	switch (m_MenuIdTiming)
	{
	default:
	case IDM_REALTIME:
		m_RealTimeTarget = 1.0;
		break;

	case IDM_3QSPEED:
		m_RealTimeTarget = 0.75;
		break;

	case IDM_HALFSPEED:
		m_RealTimeTarget = 0.5;
		break;

	case IDM_50FPS:
		m_FPSTarget = 50;
		break;

	case IDM_25FPS:
		m_FPSTarget = 25;
		break;

	case IDM_10FPS:
		m_FPSTarget = 10;
		break;

	case IDM_5FPS:
		m_FPSTarget = 5;
		break;

	case IDM_1FPS:
		m_FPSTarget = 1;
		break;
	}
}

/****************************************************************************/
void BeebWin::TranslateKeyMapping(void)
{
	switch (m_MenuIdKeyMapping )
	{
	default:
	case IDM_KEYBOARDMAPPING1:
		transTable = transTable1;
		break;

	case IDM_KEYBOARDMAPPING2:
		transTable = transTable2;
		break;

	case IDM_USERKYBDMAPPING:
		transTable = UserKeymap;
		break;
	}
}

/****************************************************************************/
void BeebWin::ReadDisc(int Drive)
{
	char StartPath[_MAX_PATH];
	char FileName[256];
	OPENFILENAME ofn;

	strcpy(StartPath, m_AppPath);
	strcat(StartPath, "discims");
	FileName[0] = '\0';

	/* Hmm, what do I put in all these fields! */
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "Single Sided Disc\0*.*\0Double Sided Disc\0*.*\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = m_DiscTypeSelection;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = sizeof(FileName);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = StartPath;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (GetOpenFileName(&ofn))
	{
		/* Default to same type of disc next time */
		m_DiscTypeSelection = ofn.nFilterIndex;

		if (ofn.nFilterIndex == 1)
		{
			LoadSimpleDiscImage(FileName, Drive, 0, 80);
		}
		else
		{
			LoadSimpleDSDiscImage(FileName, Drive, 80);
		}

		/* Write protect the disc */
		if (!m_WriteProtectDisc[Drive])
			ToggleWriteProtect(Drive);
	}
}

/****************************************************************************/
void BeebWin::NewDiscImage(int Drive)
{
	char StartPath[_MAX_PATH];
	char FileName[256];
	OPENFILENAME ofn;

	strcpy(StartPath, m_AppPath);
	strcat(StartPath, "discims");
	FileName[0] = '\0';

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "Single Sided Disc\0*.*\0Double Sided Disc\0*.*\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = m_DiscTypeSelection;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = sizeof(FileName);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = StartPath;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (GetSaveFileName(&ofn))
	{
		/* Default to same type of disc next time */
		m_DiscTypeSelection = ofn.nFilterIndex;

		/* Add a file extension if the user did not specify one */
		if (strchr(FileName, '.') == NULL)
		{
			if (ofn.nFilterIndex == 1)
				strcat(FileName, ".ssd");
			else
				strcat(FileName, ".dsd");
		}

		if (ofn.nFilterIndex == 1)
		{
			CreateDiscImage(FileName, Drive, 1, 80);
		}
		else
		{
			CreateDiscImage(FileName, Drive, 2, 80);
		}

		/* Allow disc writes */
		if (m_WriteProtectDisc[Drive])
			ToggleWriteProtect(Drive);
	}
}

/****************************************************************************/
void BeebWin::SaveState()
{
	char StartPath[_MAX_PATH];
	char FileName[256];
	OPENFILENAME ofn;

	strcpy(StartPath, m_AppPath);
	strcat(StartPath, "beebstate");
	FileName[0] = '\0';

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "Beeb State File\0*.*\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = sizeof(FileName);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = StartPath;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (GetSaveFileName(&ofn))
	{
		BeebSaveState(FileName);
	}
}

/****************************************************************************/
void BeebWin::RestoreState()
{
	char StartPath[_MAX_PATH];
	char FileName[256];
	OPENFILENAME ofn;

	strcpy(StartPath, m_AppPath);
	strcat(StartPath, "beebstate");
	FileName[0] = '\0';

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "Beeb State File\0*.*\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = sizeof(FileName);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = StartPath;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (GetOpenFileName(&ofn))
	{
		BeebRestoreState(FileName);
	}
}

/****************************************************************************/
void BeebWin::ToggleWriteProtect(int Drive)
{
	HMENU hMenu = GetMenu(m_hWnd);

	if (m_WriteProtectDisc[Drive])
	{
		m_WriteProtectDisc[Drive] = 0;
		DiscWriteEnable(Drive, 1);
	}
	else
	{
		m_WriteProtectDisc[Drive] = 1;
		DiscWriteEnable(Drive, 0);
	}

	if (Drive == 0)
		CheckMenuItem(hMenu, IDM_WPDISC0, m_WriteProtectDisc[0] ? MF_CHECKED : MF_UNCHECKED);
	else
		CheckMenuItem(hMenu, IDM_WPDISC1, m_WriteProtectDisc[1] ? MF_CHECKED : MF_UNCHECKED);
}

/****************************************************************************/
void BeebWin::SavePreferences()
{
	char CfgValue[256];
	char CfgName[256];
	RECT wndrect;

	sprintf(CfgValue, "%d", m_MenuIdWinSize);
	WritePrivateProfileString(CFG_VIEW_SECTION, CFG_VIEW_WIN_SIZE,
			CfgValue, CFG_FILE_NAME);

	GetWindowRect(m_hWnd, &wndrect);
	sprintf(CfgValue, "%d", wndrect.left);
	WritePrivateProfileString(CFG_VIEW_SECTION, CFG_VIEW_WIN_XPOS,
			CfgValue, CFG_FILE_NAME);
	sprintf(CfgValue, "%d", wndrect.top);
	WritePrivateProfileString(CFG_VIEW_SECTION, CFG_VIEW_WIN_YPOS,
			CfgValue, CFG_FILE_NAME);

	sprintf(CfgValue, "%d", m_ShowSpeedAndFPS);
	WritePrivateProfileString(CFG_VIEW_SECTION, CFG_VIEW_SHOW_FPS,
			CfgValue, CFG_FILE_NAME);

	sprintf(CfgValue, "%d", SoundEnabled);
	WritePrivateProfileString(CFG_SOUND_SECTION, CFG_SOUND_ENABLED,
			CfgValue, CFG_FILE_NAME);

	sprintf(CfgValue, "%d", m_MenuIdSampleRate);
	WritePrivateProfileString(CFG_SOUND_SECTION, CFG_SOUND_SAMPLE_RATE,
			CfgValue, CFG_FILE_NAME);

	sprintf(CfgValue, "%d", m_MenuIdVolume);
	WritePrivateProfileString(CFG_SOUND_SECTION, CFG_SOUND_VOLUME,
			CfgValue, CFG_FILE_NAME);

	sprintf(CfgValue, "%d", m_MenuIdTiming);
	WritePrivateProfileString(CFG_SPEED_SECTION, CFG_SPEED_TIMING,
			CfgValue, CFG_FILE_NAME);

	sprintf(CfgValue, "%d", m_MenuIdKeyMapping);
	WritePrivateProfileString(CFG_OPTIONS_SECTION, CFG_OPTIONS_KEY_MAPPING,
			CfgValue, CFG_FILE_NAME);

	sprintf(CfgValue, "%d", m_MenuIdSticks);
	WritePrivateProfileString(CFG_OPTIONS_SECTION, CFG_OPTIONS_STICKS,
			CfgValue, CFG_FILE_NAME);

	for (int key=0; key<256; ++key)
	{
		if (UserKeymap[key][0] != 0 || UserKeymap[key][1] != 0)
		{
			sprintf(CfgName, "%s%d", CFG_OPTIONS_USER_KEY_MAP, key);
			sprintf(CfgValue, "%d %d", UserKeymap[key][0], UserKeymap[key][1]);
			WritePrivateProfileString(CFG_OPTIONS_SECTION, CfgName,
				CfgValue, CFG_FILE_NAME);
		}
	}
}

/****************************************************************************/
void BeebWin::HandleCommand(int MenuId)
{
	HMENU hMenu = GetMenu(m_hWnd);

	switch (MenuId)
	{
	case IDM_LOADSTATE:
		RestoreState();
		break;
	case IDM_SAVESTATE:
		SaveState();
		break;

	case IDM_QUICKLOAD:
	case IDM_QUICKSAVE:
		{
			char FileName[_MAX_PATH];
			strcpy(FileName, m_AppPath);
			strcat(FileName, "beebstate\\quicksave");
			if (MenuId == IDM_QUICKLOAD)
				BeebRestoreState(FileName);
			else
				BeebSaveState(FileName);
		}
		break;

	case IDM_LOADDISC0:
		ReadDisc(0);
		break;
	case IDM_LOADDISC1:
		ReadDisc(1);
		break;

	case IDM_NEWDISC0:
		NewDiscImage(0);
		break;
	case IDM_NEWDISC1:
		NewDiscImage(1);
		break;

	case IDM_WPDISC0:
		ToggleWriteProtect(0);
		break;
	case IDM_WPDISC1:
		ToggleWriteProtect(1);
		break;

	case IDM_160X128:
	case IDM_240X192:
	case IDM_640X256:
	case IDM_320X256:
	case IDM_640X512:
	case IDM_800X600:
	case IDM_1024X768:
	case IDM_1024X512:
		if (MenuId != m_MenuIdWinSize)
		{
			CheckMenuItem(hMenu, m_MenuIdWinSize, MF_UNCHECKED);
			m_MenuIdWinSize = MenuId;
			CheckMenuItem(hMenu, m_MenuIdWinSize, MF_CHECKED);
			TranslateWindowSize();

			SetWindowPos(m_hWnd, HWND_TOP, 0,0,
					m_XWinSize + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
					m_YWinSize + GetSystemMetrics(SM_CYFIXEDFRAME) * 2
						+ GetSystemMetrics(SM_CYMENUSIZE) * (MenuId == IDM_160X128 ? 2:1)
						+ GetSystemMetrics(SM_CYCAPTION)
						+ 1,
					SWP_NOMOVE);
   		}
		break;

	case IDM_SPEEDANDFPS:
		if (m_ShowSpeedAndFPS)
		{
			m_ShowSpeedAndFPS = FALSE;
			CheckMenuItem(hMenu, IDM_SPEEDANDFPS, MF_UNCHECKED);
			SetWindowText(m_hWnd, WindowTitle);
		}
		else
		{
			m_ShowSpeedAndFPS = TRUE;
			CheckMenuItem(hMenu, IDM_SPEEDANDFPS, MF_CHECKED);
		}
		break;
	
	case IDM_SOUNDONOFF:
		if (SoundEnabled)
		{
			CheckMenuItem(hMenu, IDM_SOUNDONOFF, MF_UNCHECKED);
			SoundReset();
		}
		else
		{
			SoundInit();
			if (SoundEnabled)
				CheckMenuItem(hMenu, IDM_SOUNDONOFF, MF_CHECKED);
		}
		break;
	
	case IDM_44100KHZ:
	case IDM_22050KHZ:
	case IDM_11025KHZ:
		if (MenuId != m_MenuIdSampleRate)
		{
			CheckMenuItem(hMenu, m_MenuIdSampleRate, MF_UNCHECKED);
			m_MenuIdSampleRate = MenuId;
			CheckMenuItem(hMenu, m_MenuIdSampleRate, MF_CHECKED);
			TranslateSampleRate();

			if (SoundEnabled)
			{
				SoundReset();
				SoundInit();
			}
		}
		break;
	
	case IDM_FULLVOLUME:
	case IDM_HIGHVOLUME:
	case IDM_MEDIUMVOLUME:
	case IDM_LOWVOLUME:
		if (MenuId != m_MenuIdVolume)
		{
			CheckMenuItem(hMenu, m_MenuIdVolume, MF_UNCHECKED);
			m_MenuIdVolume = MenuId;
			CheckMenuItem(hMenu, m_MenuIdVolume, MF_CHECKED);
			TranslateVolume();
		}
		break;
	
	case IDM_ALLOWALLROMWRITES:	
		if (WritableRoms)
		{
			WritableRoms = FALSE;
			CheckMenuItem(hMenu, IDM_ALLOWALLROMWRITES, MF_UNCHECKED);
			GreyRomMenu( FALSE );	
		}
		else
		{
			WritableRoms = TRUE;
			CheckMenuItem(hMenu, IDM_ALLOWALLROMWRITES, MF_CHECKED);
			GreyRomMenu( TRUE );	
		}
		break;

	/* LRW Added switch individual ROMS Writable ON/OFF */
	case IDM_ALLOWWRITES_ROM0:
	case IDM_ALLOWWRITES_ROM1:
	case IDM_ALLOWWRITES_ROM2:
	case IDM_ALLOWWRITES_ROM3:
	case IDM_ALLOWWRITES_ROM4:
	case IDM_ALLOWWRITES_ROM5:
	case IDM_ALLOWWRITES_ROM6:
	case IDM_ALLOWWRITES_ROM7:
	case IDM_ALLOWWRITES_ROM8:
	case IDM_ALLOWWRITES_ROM9:
	case IDM_ALLOWWRITES_ROMA:
	case IDM_ALLOWWRITES_ROMB:
	case IDM_ALLOWWRITES_ROMC:
	case IDM_ALLOWWRITES_ROMD:
	case IDM_ALLOWWRITES_ROME:
	case IDM_ALLOWWRITES_ROMF:

		CheckMenuItem(hMenu,  MenuId, RomWritable[( MenuId-IDM_ALLOWWRITES_ROM0)] ? MF_UNCHECKED : MF_CHECKED );
		GetRomMenu();	// Update the Rom/Ram state for all the roms.
		break;				

	case IDM_REALTIME:
	case IDM_3QSPEED:
	case IDM_HALFSPEED:
	case IDM_50FPS:
	case IDM_25FPS:
	case IDM_10FPS:
	case IDM_5FPS:
	case IDM_1FPS:
		if (MenuId != m_MenuIdTiming)
		{
			CheckMenuItem(hMenu, m_MenuIdTiming, MF_UNCHECKED);
			m_MenuIdTiming = MenuId;
			CheckMenuItem(hMenu, m_MenuIdTiming, MF_CHECKED);
			TranslateTiming();
		}
		break;

	case IDM_JOYSTICK:
	case IDM_MOUSESTICK:
		/* Disable current selection */
		if (m_MenuIdSticks != 0)
		{
			CheckMenuItem(hMenu, m_MenuIdSticks, MF_UNCHECKED);
			if (m_MenuIdSticks == IDM_JOYSTICK)
			{
				ResetJoystick();
			}
			else /* mousestick */
			{
				AtoDReset();
			}
		}

		if (MenuId == m_MenuIdSticks)
		{
			/* Joysticks switched off completely */
			m_MenuIdSticks = 0;
		}
		else
		{
			/* Initialise new selection */
			m_MenuIdSticks = MenuId;
			if (m_MenuIdSticks == IDM_JOYSTICK)
			{
				InitJoystick();
			}
			else /* mousestick */
			{
				AtoDInit();
			}
			if (JoystickEnabled)
				CheckMenuItem(hMenu, m_MenuIdSticks, MF_CHECKED);
			else
				m_MenuIdSticks = 0;
		}
		break;

	case IDM_HIDECURSOR:
		if (m_HideCursor)
		{
			m_HideCursor = FALSE;
			CheckMenuItem(hMenu, IDM_HIDECURSOR, MF_UNCHECKED);
		}
		else
		{
			m_HideCursor = TRUE;
			CheckMenuItem(hMenu, IDM_HIDECURSOR, MF_CHECKED);
		}
		break;

	case IDM_IGNOREILLEGALOPS:
		if (IgnoreIllegalInstructions)
		{
			IgnoreIllegalInstructions = FALSE;
			CheckMenuItem(hMenu, IDM_IGNOREILLEGALOPS, MF_UNCHECKED);
		}
		else
		{
			IgnoreIllegalInstructions = TRUE;
			CheckMenuItem(hMenu, IDM_IGNOREILLEGALOPS, MF_CHECKED);
		}
		break;

	case IDM_DEFINEKEYMAP:
		UserKeyboardDialog( m_hWnd );
		break;

	case IDM_USERKYBDMAPPING:
	case IDM_KEYBOARDMAPPING1:
	case IDM_KEYBOARDMAPPING2:
		if (MenuId != m_MenuIdKeyMapping)
		{
			CheckMenuItem(hMenu, m_MenuIdKeyMapping, MF_UNCHECKED);
			m_MenuIdKeyMapping = MenuId;
			CheckMenuItem(hMenu, m_MenuIdKeyMapping, MF_CHECKED);
			TranslateKeyMapping();
		}
		break;

	case IDM_ABOUT:
		MessageBox(NULL, AboutText, WindowTitle, MB_OK);
		break;

	case IDM_EXIT:
		PostMessage(m_hWnd, WM_CLOSE, 0, 0L);
		break;

	case IDM_SAVE_PREFS:
		SavePreferences();
		break;
	}
}
