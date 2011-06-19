/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// EERIEApp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//
// Code:	Cyril Meynier
//			S�bastien Scieux	(Zbuffer)
//			Didier Pedreno		(ScreenSaver Problem Fix)
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "core/Application.h"

#include <cstdio>

#include "core/Config.h"
#include "core/Resource.h"
#include "core/GameTime.h"

#include "game/Player.h"

#include "gui/Menu.h"
#include "gui/Interface.h"
#include "gui/MenuWidgets.h"

#include "graphics/Frame.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/GraphicsUtility.h"
#include "graphics/data/Mesh.h"
#include "graphics/font/Font.h"

#include "io/FilePath.h"
#include "io/PakManager.h"
#include "io/Logger.h"

using std::max;

//-----------------------------------------------------------------------------
extern long USE_OLD_MOUSE_SYSTEM;
extern long FINAL_RELEASE;
extern bool bGLOBAL_DINPUT_GAME;

extern long FINAL_COMMERCIAL_DEMO;
extern long FINAL_COMMERCIAL_GAME;

//-----------------------------------------------------------------------------
long _EERIEMouseXdep, _EERIEMouseYdep, EERIEMouseXdep, EERIEMouseYdep, EERIEMouseX, EERIEMouseY, EERIEWheel = 0;
long EERIEMouseButton = 0;
long LastEERIEMouseButton = 0;
long EERIEMouseGrab = 0;
long MouseDragX, MouseDragY;

CD3DApplication		* g_pD3DApp = NULL;
EERIE_CAMERA		* Kam;
LPDIRECT3DDEVICE7	GDevice;
HWND				MSGhwnd = NULL;
float				FPS;
LightMode ModeLight = 0;
ViewModeFlags ViewMode = 0;

static RECT srRect;
static int iCurrZBias;

//-----------------------------------------------------------------------------
// Internal function prototypes and variables
//-----------------------------------------------------------------------------
enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOSOFTWARE };

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//*************************************************************************************
// CD3DApplication()
// Constructor
//*************************************************************************************
CD3DApplication::CD3DApplication()
{
	long i;
	d_dlgframe = 0;
	m_pFramework   = NULL;
	m_hWnd         = NULL;
	m_pDD          = NULL;
	m_pD3D         = NULL;

	m_pddsRenderTarget     = NULL;
	m_pddsRenderTargetLeft = NULL;

	m_bActive         = false;
	m_bReady          = false;
	m_bSingleStep     = false;

	m_strWindowTitle  = "EERIE Application";
	m_bAppUseZBuffer  = false;
	m_bAppUseStereo   = false;
	m_bShowStats      = false;
	m_fnConfirmDevice = NULL;
	WndSizeX = 640;
	WndSizeY = 480;
	Fullscreen = 0;
	CreationFlags = 0;
	owner = 0L;
	CreationMenu = 0;
#ifdef BUILD_EDITOR
	ToolBar = NULL;
#endif
	strcpy(StatusText, "");
	memset(&Project, 0, sizeof(PROJECT));
	Project.TextureSize = 0;
	Project.ambient = 0;
	Project.vsync = true;		
	Project.interpolatemouse = 1;

	//Keyboard Init;
	for (i = 0; i < 255; i++)	kbd.inkey[i] = 0;

	kbd.nbkeydown = 0;
	kbd._CAPS = 0;

	zbuffer_max = 1.f;
	zbuffer_max_div = 1.f;
}


extern long FINAL_COMMERCIAL_DEMO;
extern long FINAL_COMMERCIAL_GAME;

//*************************************************************************************
// Create()
//*************************************************************************************
bool CD3DApplication::Create(HINSTANCE hInst) {
	
	HRESULT hr;

	// Enumerate available D3D devices. The callback is used so the app can
	// confirm/reject each enumerated device depending on its capabilities.
	if (FAILED(hr = D3DEnum_EnumerateDevices(m_fnConfirmDevice)))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		return false;
	}

	// Select a device. Ask for a hardware device that renders in a window.
	if (FAILED(hr = D3DEnum_SelectDefaultDevice(&m_pDeviceInfo)))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		return false;
	}

	// Initialize the app's custom scene stuff
	if (FAILED(hr = OneTimeSceneInit()))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		return false;
	}

	// Create a new CD3DFramework class. This class does all of our D3D
	// initialization and manages the common D3D objects.
	if (NULL == (m_pFramework = new CD3DFramework7()))
	{
		DisplayFrameworkError(E_OUTOFMEMORY, MSGERR_APPMUSTEXIT);
		return false;
	}

	//	DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
#ifdef BUILD_EDITOR
	if (ToolBar != NULL)
	{
		if (this->ToolBar->Type == EERIE_TOOLBAR_TOP)
		{
			this->m_pFramework->Ystart = 26;
			this->m_pFramework->Xstart = 0;
		}
		else
		{
			this->m_pFramework->Ystart = 0;
			this->m_pFramework->Xstart = 98;
		}
	}
	else
#endif
	{
		this->m_pFramework->Ystart = 0;
		this->m_pFramework->Xstart = 0;
	}


	// Register the window class
	WNDCLASS wndClass = { CS_DBLCLKS, WndProc, 0, 0, hInst,
	                      LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAIN)),
	                      LoadCursor(NULL, IDC_ARROW),
	                      (HBRUSH)GetStockObject(BLACK_BRUSH),
	                      NULL, "D3D Window"
	                    };
	RegisterClass(&wndClass);

	// Create the render window
	DWORD flags = WS_OVERLAPPEDWINDOW;

	if (CreationFlags & WCF_NORESIZE)
	{
		flags |= (WS_DLGFRAME | WS_MINIMIZEBOX);
		flags &= ~WS_CAPTION;
		flags &= ~WS_MAXIMIZEBOX;
		flags &= ~WS_THICKFRAME;
	}
	else if (CreationFlags & WCF_CHILD)
	{
        flags &= ~WS_OVERLAPPEDWINDOW;
		flags |= WS_CHILD;
	}

    long menu;
	if( Fullscreen )
	{
		flags &= ~WS_CAPTION;
		flags &= ~WS_MAXIMIZEBOX;
		flags &= ~WS_THICKFRAME;
		flags &= ~WS_SYSMENU;
		flags &= ~WS_OVERLAPPEDWINDOW;
		menu = 0;
	}
	else
	{
		menu = CreationMenu;
	}

	// We want the client rectangle (3d display) to be a certain size, adjust the window size accordingly
	RECT rcWnd = { 0, 0, WndSizeX, WndSizeY };
	AdjustWindowRectEx(&rcWnd, flags, menu != 0, 0);

	// Bound the window size to fit the desktop
	HWND hWndDesktop = GetDesktopWindow();
	RECT rcDesktop;
	GetWindowRect(hWndDesktop, &rcDesktop);

	LONG wndWidth = rcWnd.right - rcWnd.left;
	LONG wndHeight = rcWnd.bottom - rcWnd.top;
	LONG maxWidth = rcDesktop.right - rcDesktop.left;
	LONG maxHeight = rcDesktop.bottom - rcDesktop.top;
	
	wndWidth = std::min(wndWidth, maxWidth);
	wndHeight = std::min(wndHeight, maxHeight);

	MSGhwnd = m_hWnd = CreateWindow("D3D Window", m_strWindowTitle.c_str(),
		                            flags,
									CW_USEDEFAULT, CW_USEDEFAULT, wndWidth, wndHeight, owner,
		                            LoadMenu(hInst, MAKEINTRESOURCE(menu)),
		                            hInst, 0L);

	if (!m_hWnd) return  false;

	UpdateWindow(m_hWnd);
	ShowWindow(m_hWnd, SW_SHOW);
	

	// Nuky - this isnt used for the game, and I can set WIN32_LEAN_AND_MEAN with it commented
	//// � supprimer au final
	//if (CreationFlags & WCF_ACCEPTFILES)
	//	DragAcceptFiles(m_hWnd, true);

	// Initialize the 3D environment for the app
	if (FAILED(hr = Initialize3DEnvironment()))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		Cleanup3DEnvironment();
		return false;
	}

	// Setup the app so it can support single-stepping
	m_dwBaseTime = dwARX_TIME_Get();

	// The app is ready to go
	m_bReady = true;

#ifdef BUILD_EDITOR
	if (ToolBar != NULL)
	{
		ToolBar->hWnd = CreateToolBar(m_hWnd, hInst);
	}
#endif

	this->m_pFramework->ShowFrame();
	GetZBufferMax();
	return true;
}

#ifdef BUILD_EDITOR
HWND CD3DApplication::CreateToolBar(HWND hWndParent, HINSTANCE hInst) {
	
	HWND hWndToolbar;
	long flags;


	flags = WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_WRAPABLE;

	if (this->ToolBar->Type == EERIE_TOOLBAR_TOP) flags |= CCS_TOP;
	else flags |= CCS_LEFT;

	hWndToolbar = CreateToolbarEx(hWndParent,
	                              flags,
	                              this->ToolBar->CreationToolBar, this->ToolBar->ToolBarNb, //nb buttons in bmp
	                              (HINSTANCE)hInst, this->ToolBar->Bitmap,
	                              (LPCTBBUTTON)this->ToolBar->Buttons, this->ToolBar->ToolBarNb,
	                              16, 15, 16, 15, sizeof(TBBUTTON));

	SetWindowPos(hWndToolbar, HWND_TOP, 0, 90, 2, 500, SWP_SHOWWINDOW);
	ShowWindow(hWndToolbar, SW_SHOW);
	UpdateWindow(hWndToolbar);


	return hWndToolbar;
}
#endif

//*************************************************************************************
// Run()
// Message-processing loop. Idle time is used to render the scene.
//*************************************************************************************
int CD3DApplication::Run()
{
	BeforeRun();
	
	// Load keyboard accelerators
	HACCEL hAccel = NULL;

	// Now we're ready to recieve and process Windows messages.
	BOOL bGotMsg;
	MSG  msg;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (WM_QUIT != msg.message)
	{
		// Use PeekMessage() if the app is active, so we can use idle time to
		// render the scene. Else, use GetMessage() to avoid eating CPU time.
		if (m_bActive)
			bGotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
		else
			bGotMsg = GetMessage(&msg, NULL, 0U, 0U);

		if (bGotMsg)
		{
			// Translate and dispatch the message
			if (0 == TranslateAccelerator(m_hWnd, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// Render a frame during idle time (no messages are waiting)
			if (m_bActive && m_bReady)
			{
				if (FAILED(Render3DEnvironment()))
					DestroyWindow(m_hWnd);
			}
		}
	}

	return msg.wParam;

}

void CD3DApplication::EvictManagedTextures()
{
	if (this->m_pD3D)
	{
		this->m_pD3D->EvictManagedTextures();
	}
}

int CD3DApplication::WinManageMess()
{
	BOOL bGotMsg = true;
	MSG  msg;

	while (bGotMsg)
	{
		if (m_bActive)
			bGotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
		else
			bGotMsg = GetMessage(&msg, NULL, 0U, 0U);

		if (bGotMsg)
		{
			// Translate and dispatch the message
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 1;
}



//*************************************************************************************
// WndProc()
//  Static msg handler which passes messages to the application class.
//*************************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (g_pD3DApp)
		return g_pD3DApp->MsgProc(hWnd, uMsg, wParam, lParam);

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//*************************************************************************************
// Callback for WM_MOUSEMOUVE
//*************************************************************************************
void CD3DApplication::EERIEMouseUpdate(short x, short y)
{
	if ((m_pFramework) &&
	        (m_pFramework->m_bIsFullscreen) &&
	        (bGLOBAL_DINPUT_GAME)) return;

	// Inactive App: ignore
	if (!this->m_bActive)	return;

	// Updates MouseX & MouseY offsets
	_EERIEMouseXdep += (x - EERIEMouseX);
	_EERIEMouseYdep += (y - EERIEMouseY);

	// Manages MouseGrab (Right-click maintained pressed)

	POINT	pos;
	bool	mod	=	false;

	pos.x = pos.y = 0;

	if (EERIEMouseGrab)
	{
		pos.x = (short)(this->m_pFramework->m_dwRenderWidth >> 1);
		pos.y = (short)(this->m_pFramework->m_dwRenderHeight >> 1);

		if ((x != pos.x) || (y != pos.y)) mod = true;
	}

	if (!((ARXmenu.currentmode == AMCM_NEWQUEST)
	        ||	(player.Interface & INTER_MAP && (Book_Mode != BOOKMODE_MINIMAP))))
		if (mod)
		{
			EERIEMouseX = (long)pos.x;
			EERIEMouseY = (long)pos.y;
			ClientToScreen(this->m_hWnd, &pos);
			SetCursorPos(pos.x, pos.y);
			return;
		}

	EERIEMouseX = x;
	EERIEMouseY = y;
}

//*************************************************************************************
//*************************************************************************************
LRESULT CD3DApplication::SwitchFullScreen()
{

	m_bReady = false;
	m_pDeviceInfo->bWindowed = !m_pDeviceInfo->bWindowed;

#ifdef BUILD_EDITOR
	if (this->ToolBar != NULL)
	{
		if (m_pDeviceInfo->bWindowed)  ShowWindow(ToolBar->hWnd, SW_SHOW);
		else ShowWindow(ToolBar->hWnd, SW_HIDE);
	}
#endif

	if (FAILED(Change3DEnvironment()))
	{
		LogError << ("ChangeEnvironement Failed");
		return 0;
	}

	m_bReady = true;
	m_pFramework->m_bHasMoved = true;

	return 0;
}
//*************************************************************************************
// MsgProc()
// Message handling function.
//*************************************************************************************
LRESULT CD3DApplication::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                 LPARAM lParam)
{
	long iii, ij;

	switch (uMsg)
	{
		case WM_KEYDOWN:
			this->kbd.nbkeydown++;
			iii = (lParam >> 16) & 255;
			ij = (lParam >> 24) & 1;

			if (ij)
			{
				iii += 100;
				this->kbd.inkey[iii] = 1;
			}
			else this->kbd.inkey[iii] = 1;;

			this->kbd.lastkey = (short)iii;

			if (iii == 18)
			{
				if (this->kbd._CAPS) this->kbd._CAPS = 0;
				else this->kbd._CAPS = 1;
			}

			break;
		case WM_KEYUP:
			this->kbd.nbkeydown--;
			iii = (lParam >> 16) & 255;
			ij = (lParam >> 24) & 1;

			if (ij)
			{
				iii += 100;
				this->kbd.inkey[iii] = 0;
			}
			else this->kbd.inkey[iii] = 0;;

			break;

		case WM_LBUTTONDBLCLK:

			if (USE_OLD_MOUSE_SYSTEM)
			{
				LastEERIEMouseButton = EERIEMouseButton;
				EERIEMouseButton |= 4;
				EERIEMouseButton &= ~1;
			}

			break;
		case WM_LBUTTONDOWN:

			if (USE_OLD_MOUSE_SYSTEM)
			{
				LastEERIEMouseButton = EERIEMouseButton;
				EERIEMouseButton |= 1;

				if (EERIEMouseButton & 4) EERIEMouseButton &= ~1;
			}

			break;
		case WM_LBUTTONUP:

			if (USE_OLD_MOUSE_SYSTEM)
			{
				LastEERIEMouseButton = EERIEMouseButton;
				EERIEMouseButton &= ~1;
				EERIEMouseButton &= ~4;
			}

			break;
		case WM_RBUTTONDOWN:

			if (USE_OLD_MOUSE_SYSTEM)
			{
				EERIEMouseButton |= 2;
			}

			break;
		case WM_RBUTTONUP:

			if (USE_OLD_MOUSE_SYSTEM)
			{
				EERIEMouseButton &= ~2;
			}

			break;

			//-------------------------------------------
			//warning "macro redefinition" - as we haven't redefined WM_MOUSEWHEEL, WM_MOUSEWHEEL is equal to 0x020A
		case WM_MOUSEWHEEL:
			EERIEWheel = (short) HIWORD(wParam);
			break;
		case WM_MOUSEMOVE:
			EERIEMouseUpdate(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_ERASEBKGND:
		{
			return 1;
		}
		break;
		case WM_PAINT:
			// Handle paint messages when the app is not ready
			if (m_pFramework && !m_bReady)
			{
				if (m_pDeviceInfo->bWindowed)
				{
					m_pFramework->ShowFrame();
				}
				else
				{
					m_pFramework->FlipToGDISurface(true);
				}
			}
			break;
		
		case WM_SIZE:
			// Check to see if we are losing our window...
			if (SIZE_MAXHIDE == wParam || SIZE_MINIMIZED == wParam)
				m_bActive = false;
			else 
				m_bActive = true;
			
			// A new window size will require a new backbuffer
			// size, so the 3D structures must be changed accordingly.
			if (m_bActive && m_bReady && m_pDeviceInfo->bWindowed)
			{

				m_pFramework->m_bHasMoved = true;

#ifdef BUILD_EDITOR
				RECT rec;
				rec.right = LOWORD(lParam);
				rec.bottom = HIWORD(lParam);
				if(ToolBar && ToolBar->hWnd) {
					RECT rectt;
					GetWindowRect(ToolBar->hWnd, &rectt);
					SetWindowPos(ToolBar->hWnd, HWND_TOP, 0, 0, rec.right, rectt.bottom, SWP_NOZORDER | SWP_NOREPOSITION | SWP_NOMOVE | SWP_SHOWWINDOW);
				}
#endif

			}

			break;

		case WM_SETCURSOR:
			// Prevent a cursor in fullscreen mode
			if (m_bActive && m_bReady && !m_pDeviceInfo->bWindowed)
			{
				SetCursor(NULL);
				return 1;
			}
			break;

		case WM_NCHITTEST:
			// Prevent the user from selecting the menu in fullscreen mode
			if (!m_pDeviceInfo->bWindowed)
				return HTCLIENT;
			break;

		case WM_POWERBROADCAST:

			switch (wParam)
			{
				case PBT_APMQUERYSUSPEND:
					// At this point, the app should save any data for open
					// network connections, files, etc.., and prepare to go into
					// a suspended mode.
					return OnQuerySuspend((DWORD)lParam);

				case PBT_APMRESUMESUSPEND:
					// At this point, the app should recover any data, network
					// connections, files, etc.., and resume running from when
					// the app was suspended.
					return OnResumeSuspend((DWORD)lParam);
			}

			break;

		case WM_SYSCOMMAND:

			// Prevent moving/sizing and power loss in fullscreen mode
			switch (wParam)
			{
				case SC_MOVE:
				case SC_SIZE:
				case SC_MAXIMIZE:
					// To Avoid Screensaver Problems
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
					break;
				default:
					break;
			}

			break;

	case WM_COMMAND:
	break;
		case WM_GETMINMAXINFO:
			((MINMAXINFO *)lParam)->ptMinTrackSize.x = 100;
			((MINMAXINFO *)lParam)->ptMinTrackSize.y = 100;
			break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}




//*************************************************************************************
// Initialize3DEnvironment()
// Initializes the sample framework, then calls the app-specific function
// to initialize device specific objects. This code is structured to
// handled any errors that may occur during initialization
//*************************************************************************************
HRESULT CD3DApplication::Initialize3DEnvironment()
{
	HRESULT hr;
	DWORD   dwFrameworkFlags = 0L;
	dwFrameworkFlags |= (!m_pDeviceInfo->bWindowed ? D3DFW_FULLSCREEN : 0L);
	dwFrameworkFlags |= (m_pDeviceInfo->bStereo   ? D3DFW_STEREO     : 0L);
	dwFrameworkFlags |= (m_bAppUseZBuffer         ? D3DFW_ZBUFFER    : 0L);

	if (lpDDGammaControl)
	{
		lpDDGammaControl->Release();
		lpDDGammaControl = NULL;
	}

	// Initialize the D3D framework
	if (SUCCEEDED(hr = m_pFramework->Initialize(m_hWnd,
	                   m_pDeviceInfo->pDriverGUID, m_pDeviceInfo->pDeviceGUID,
	                   &m_pDeviceInfo->ddsdFullscreenMode, dwFrameworkFlags)))
	{
		m_pDD        = m_pFramework->GetDirectDraw();
		m_pD3D       = m_pFramework->GetDirect3D();

		m_pddsRenderTarget     = m_pFramework->GetRenderSurface();
		m_pddsRenderTargetLeft = m_pFramework->GetRenderSurfaceLeft();

		m_ddsdRenderTarget.dwSize = sizeof(m_ddsdRenderTarget);
		m_pddsRenderTarget->GetSurfaceDesc(&m_ddsdRenderTarget);

		if (this->m_pFramework->m_pddsFrontBuffer)
		{
			this->m_pFramework->m_pddsFrontBuffer->QueryInterface(IID_IDirectDrawGammaControl, (void **)&lpDDGammaControl);

			if (lpDDGammaControl)
			{
				lpDDGammaControl->GetGammaRamp(0, &DDGammaOld);
				lpDDGammaControl->GetGammaRamp(0, &DDGammaRamp);
			}
		}

		if (SUCCEEDED(hr = InitDeviceObjects()))
		{
			return S_OK;
		}
		else
		{
			if (lpDDGammaControl)
			{
				lpDDGammaControl->Release();
				lpDDGammaControl = NULL;
			}

			DeleteDeviceObjects();
			m_pFramework->DestroyObjects();
		}
	}

	// If we get here, the first initialization passed failed. If that was with a
	// hardware device, try again using a software rasterizer instead.
	if (m_pDeviceInfo->bHardware)
	{
		// Try again with a software rasterizer
		DisplayFrameworkError(hr, MSGWARN_SWITCHEDTOSOFTWARE);
		D3DEnum_SelectDefaultDevice(&m_pDeviceInfo, D3DENUM_SOFTWAREONLY);
		return Initialize3DEnvironment();
	}
	return hr;
}




//*************************************************************************************
// Change3DEnvironment()
// Handles driver, device, and/or mode changes for the app.
//*************************************************************************************
HRESULT CD3DApplication::Change3DEnvironment()
{
	HRESULT hr;
	static bool  bOldWindowedState = true;
	static DWORD dwSavedStyle;
	static RECT  rcSaved;

	DDGAMMARAMP DDGammaPrevious;

	if (lpDDGammaControl)
	{
		lpDDGammaControl->GetGammaRamp(0, &DDGammaPrevious);
		lpDDGammaControl->SetGammaRamp(0, &DDGammaOld);
		lpDDGammaControl->Release();
		lpDDGammaControl = NULL;
	}

	// Release all scene objects that will be re-created for the new device
	DeleteDeviceObjects();

	// Release framework objects, so a new device can be created
	if (FAILED(hr = m_pFramework->DestroyObjects()))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		SendMessage(m_hWnd, WM_CLOSE, 0, 0);
		return hr;
	}

	// Check if going from fullscreen to windowed mode, or vice versa.
	if (bOldWindowedState != m_pDeviceInfo->bWindowed)
	{
		if (m_pDeviceInfo->bWindowed)
		{
			// Coming from fullscreen mode, so restore window properties
			SetWindowLong(m_hWnd, GWL_STYLE, dwSavedStyle);
			SetWindowPos(m_hWnd, HWND_NOTOPMOST, rcSaved.left, rcSaved.top,
			             (rcSaved.right - rcSaved.left),
			             (rcSaved.bottom - rcSaved.top), SWP_SHOWWINDOW);
		}
		else
		{
			// Going to fullscreen mode, save/set window properties as needed
			dwSavedStyle = GetWindowLong(m_hWnd, GWL_STYLE);
			GetWindowRect(m_hWnd, &rcSaved);

			if (FINAL_COMMERCIAL_DEMO || FINAL_COMMERCIAL_GAME)
			{
				SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			}
			else
			{
				SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE);
			}
		}

		bOldWindowedState = m_pDeviceInfo->bWindowed;
	}

	// Inform the framework class of the driver change. It will internally
	// re-create valid surfaces, a d3ddevice, etc.
	if (FAILED(hr = Initialize3DEnvironment()))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		SendMessage(m_hWnd, WM_CLOSE, 0, 0);
		return hr;
	}

	// Restore gamma ramp...
	if (lpDDGammaControl)
	{
		lpDDGammaControl->SetGammaRamp(0, &DDGammaPrevious);
		lpDDGammaControl->GetGammaRamp(0, &DDGammaRamp);
	}
	
	GetZBufferMax();
	return S_OK;
}

HRESULT CD3DApplication::UpdateGamma()
{
	if (lpDDGammaControl)
	{
		lpDDGammaControl->SetGammaRamp(0, &DDGammaRamp);
		return S_OK;
	}

	return 0;
}

//*************************************************************************************
// Render3DEnvironment()
// Draws the scene.
//*************************************************************************************
HRESULT CD3DApplication::Render3DEnvironment()
{
	HRESULT hr;
	EERIEMouseXdep = _EERIEMouseXdep;
	EERIEMouseYdep = _EERIEMouseYdep;
	_EERIEMouseXdep = 0;
	_EERIEMouseYdep = 0;

	// mode systemshock
	if ((EERIEMouseButton & 1) &&
	        (config.input.autoReadyWeapon == false))
	{
		MouseDragX += EERIEMouseXdep;
		MouseDragY += EERIEMouseYdep;
	}

	// Check the cooperative level before rendering
	if (FAILED(hr = m_pDD->TestCooperativeLevel()))
	{
		printf("TestCooperativeLevel failed\n");
		switch (hr)
		{
			case DDERR_EXCLUSIVEMODEALREADYSET:
			case DDERR_NOEXCLUSIVEMODE:
				// Do nothing because some other app has exclusive mode
				return S_OK;

			case DDERR_WRONGMODE:

				// The display mode changed on us. Resize accordingly
				if (m_pDeviceInfo->bWindowed)
					return Change3DEnvironment();

				break;
		}
		return hr;
	}

	// Get the relative time, in seconds
	if (FAILED(hr = FrameMove()))
		return hr;

	m_bSingleStep = false;

	// Render the scene as normal
	if (FAILED(hr = Render()))
		return hr;

	// Show the frame on the primary surface.
	if (FAILED(hr = m_pFramework->ShowFrame()))
	{
		printf("ShowFrame FAILED: %d %d <- look for this in ddraw.h\n", hr, hr&0xFFFF);
		
		if (DDERR_SURFACELOST != hr)
			return hr;

		printf("RestoreSurfaces\n");
		m_pFramework->RestoreSurfaces();
		RestoreSurfaces();
	}

	return S_OK;
}

//*************************************************************************************
// Cleanup3DEnvironment()
// Cleanup scene objects
//*************************************************************************************
VOID CD3DApplication::Cleanup3DEnvironment()
{
	m_bActive = false;
	m_bReady  = false;

	if (lpDDGammaControl)
	{
//		todo: what is DDSGR_CALIBRATE
//		lpDDGammaControl->SetGammaRamp(DDSGR_CALIBRATE, &DDGammaOld);
		lpDDGammaControl->Release();
		lpDDGammaControl = NULL;
	}

	if (m_pFramework)
	{
		DeleteDeviceObjects();
		SAFE_DELETE(m_pFramework);

		FinalCleanup();
	}

	D3DEnum_FreeResources();
}

//*************************************************************************************
// Pause()
// Called in to toggle the pause state of the app. This function
// brings the GDI surface to the front of the display, so drawing
// output like message boxes and menus may be displayed.
//*************************************************************************************
VOID CD3DApplication::Pause(bool bPause)
{
	static DWORD dwAppPausedCount = 0L;

	dwAppPausedCount += (bPause ? +1 : -1);
	m_bReady          = (dwAppPausedCount ? false : true);

	// Handle the first pause request (of many, nestable pause requests)
	if (bPause && (1 == dwAppPausedCount))
	{
		// Get a surface for the GDI
		if (m_pFramework)
			m_pFramework->FlipToGDISurface(true);

		// Stop the scene from animating
		m_dwStopTime = dwARX_TIME_Get();
	}

	if (0 == dwAppPausedCount)
	{
		// Restart the scene
		m_dwBaseTime += dwARX_TIME_Get() - m_dwStopTime;
	}
}

//*************************************************************************************
// OnQuerySuspend()
// Called when the app receives a PBT_APMQUERYSUSPEND message, meaning
// the computer is about to be suspended. At this point, the app should
// save any data for open network connections, files, etc.., and prepare
// to go into a suspended mode.
//*************************************************************************************
LRESULT CD3DApplication::OnQuerySuspend(DWORD dwFlags) {
	
	(void)dwFlags;
	
	Pause(true);
	return true;
}

//*************************************************************************************
// OnResumeSuspend()
// Called when the app receives a PBT_APMRESUMESUSPEND message, meaning
// the computer has just resumed from a suspended state. At this point,
// the app should recover any data, network connections, files, etc..,
// and resume running from when the app was suspended.
//*************************************************************************************
LRESULT CD3DApplication::OnResumeSuspend(DWORD dwData) {
	
	(void)dwData;
	
	Pause(false);
	return true;
}

//*************************************************************************************
//*************************************************************************************
VOID CalcFPS(bool reset)
{
	static float fFPS      = 0.0f;
	static float fLastTime = 0.0f;
	static DWORD dwFrames  = 0L;

	if (reset)
	{
		dwFrames = 0;
		fLastTime = 0.f;
		FPS = fFPS = 7.f * FPS;
	}
	else
	{
		float tmp;

		// Keep track of the time lapse and frame count
		float fTime = ARX_TIME_Get(false) * 0.001f;   // Get current time in seconds
		++dwFrames;

		tmp = fTime - fLastTime;

		// Update the frame rate once per second
		if (tmp > 1.f)
		{
			FPS = fFPS      = dwFrames / tmp ;
			fLastTime = fTime;
			dwFrames  = 0L;
		}
	}
}

//*************************************************************************************
// OutputText()
// Draws text on the window.
//*************************************************************************************
VOID CD3DApplication::OutputText(DWORD x, DWORD y, const std::string& str)
{
	if (m_pddsRenderTarget)
	{
		hFontInGame->Draw(x, y, str, Color(255, 255, 0));
	}
}

//*************************************************************************************
// DisplayFrameworkError()
// Displays error messages in a message box
//*************************************************************************************
VOID CD3DApplication::DisplayFrameworkError(HRESULT hr, DWORD dwType) {
	switch (hr)	{
		case D3DENUMERR_NODIRECTDRAW:
			LogError << "Unable to create DirectDraw";
			break;
		case D3DENUMERR_NOCOMPATIBLEDEVICES:
			LogError << "Unable to find any compatible Direct3D devices.";
			break;
		case D3DENUMERR_SUGGESTREFRAST:
			LogError << "Unable to find a compatible devices. Try to enable the reference rasterizer using EnableRefRast.reg.";
			break;
		case D3DENUMERR_ENUMERATIONFAILED:
			LogError << "Enumeration failure. Are you missing (32bit) graphics drivers?";
			break;
		case D3DFWERR_INITIALIZATIONFAILED:
			LogError << "Generic initialization error. Enable debug output for detailed information.";
			break;
		case D3DFWERR_NODIRECTDRAW:
			LogError << "No DirectDraw";
			break;
		case D3DFWERR_NODIRECT3D:
			LogError << "No Direct3D";
			break;
		case D3DFWERR_INVALIDMODE:
			LogError << "This Programe requires 16-bits (or higher) display mode to run in a window.";
			break;
		case D3DFWERR_COULDNTSETCOOPLEVEL:
			LogError << "Unable to set Cooperative Level";
			break;
		case D3DFWERR_NO3DDEVICE:
			LogError << "Unable to create Direct3DDevice object.";
			if (MSGWARN_SWITCHEDTOSOFTWARE == dwType)
				LogError << "Your 3D hardware chipset may not support rendering in the current display mode.";
			break;
		case D3DFWERR_NOZBUFFER:
			LogError << "No ZBuffer";
			break;
		case D3DFWERR_INVALIDZBUFFERDEPTH:
			LogError << "Invalid Z-buffer depth. Try switching modes from 16- to 32-bit (or vice versa)";
			break;
		case D3DFWERR_NOVIEWPORT:
			LogError << "No Viewport";
			break;
		case D3DFWERR_NOPRIMARY:
			LogError << "No primary";
			break;
		case D3DFWERR_NOCLIPPER:
			LogError << "No Clipper";
			break;
		case D3DFWERR_BADDISPLAYMODE:
			LogError << "Bad display mode";
			break;
		case D3DFWERR_NOBACKBUFFER:
			LogError << "No backbuffer";
			break;
		case D3DFWERR_NONZEROREFCOUNT:
			LogError << "A DDraw object has a non-zero reference count (meaning it was not properly cleaned up).";
			break;
		case D3DFWERR_NORENDERTARGET:
			LogError << "No render target";
			break;
		case E_OUTOFMEMORY:
			LogError << "Not enough memory!";
			break;
		case DDERR_OUTOFVIDEOMEMORY:
			LogError << "There was insufficient video memory to use the hardware device.";
			break;
		default:
			LogError << "Generic application error. Enable debug output for detailed information.";
	}

	if (MSGERR_APPMUSTEXIT == dwType) {
		LogError <<"This Program will now exit.";
	} else if (MSGWARN_SWITCHEDTOSOFTWARE == dwType){
		LogError <<"Switching to software rasterizer.";
	}
}
 
float CD3DApplication::GetZBufferMax()
{
	Lock();
	Unlock();
	float ret = (float)(((unsigned long)dw_zmask) >> w_zdecal);

	this->zbuffer_max = ret;
	this->zbuffer_max_div = 1.f / this->zbuffer_max;
	return this->zbuffer_max;
}
//*************************************************************************************
//*************************************************************************************
void * CD3DApplication::Lock()
{

	memset((void *)&ddsd, 0, sizeof(ddsd));	
	memset((void *)&ddsd2, 0, sizeof(ddsd2));

	this->ddsd2.dwSize = this->ddsd.dwSize = sizeof(DDSURFACEDESC2);
	this->m_pFramework->m_pddsBackBuffer->GetSurfaceDesc(&ddsd);
	this->m_pFramework->m_pddsZBuffer->GetSurfaceDesc(&ddsd2);

	this->zbuf = (void *)this->ddsd2.lpSurface ;
	this->logical = (void *)this->ddsd.lpSurface ;

	this->zbits = this->ddsd2.ddpfPixelFormat.dwZBufferBitDepth;	
	this->nbbits = this->ddsd.ddpfPixelFormat.dwRGBBitCount;

	dw_zXmodulo = ddsd2.lPitch;
	dw_zmask = ddsd2.ddpfPixelFormat.dwZBitMask;
	w_zdecal = 0;
	unsigned long t = dw_zmask;

	while (!(t & 1))
	{
		w_zdecal++;
		t >>= 1;
	}

	int t2 = 0;
	t = dw_zmask;

	while (t)
	{
		if (t & 0xFF) t2++;

		t >>= 8;
	}

	f_zmul = (float)((1 << (8 * t2)) - 1);	

	return (void *) - 1;
}

//*************************************************************************************
//*************************************************************************************
bool CD3DApplication::Unlock()
{
	if (zbuf)
	{
		m_pFramework->m_pddsZBuffer->Unlock(&srRect);
		zbuf = NULL;
	}

	return true;
}

//******************************************************************************
// MESSAGE BOXES
//******************************************************************************
//*************************************************************************************
//*************************************************************************************
//TODO(lubosz): is this needed in the game? replace
bool OKBox(const std::string& text, const std::string& title)

{
	int i;
	g_pD3DApp->Pause(true);
	i = MessageBox(g_pD3DApp->m_hWnd, text.c_str(), title.c_str(), MB_ICONQUESTION | MB_OKCANCEL);
	g_pD3DApp->Pause(false);

	if (i == IDCANCEL) return false;

	return true;
}

extern void ExitProc();


void SetZBias(int _iZBias)
{
	if (_iZBias < 0)
	{
		_iZBias = 0;
		_iZBias = max(iCurrZBias, -_iZBias);
	}

	if (_iZBias == iCurrZBias) 
		return;

	iCurrZBias = _iZBias;

	GRenderer->SetDepthBias(iCurrZBias);
}
