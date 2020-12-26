// hddblink.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "hddblink.h"

#define MAX_LOADSTRING 100
#define	WM_USER_SHELLICON WM_USER + 1
#define	POLL_INTERVAL 300

#pragma comment(lib, "pdh.lib")

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

NOTIFYICONDATA nidApp;
HWND ghWnd;
HICON gIcons[5];
HQUERY Query = NULL;
HCOUNTER gCounter[2];

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void SetIcon(int icon);
void PollHdd();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_HDDBLINK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HDDBLINK));

	// TODO: Place code here.
	nidApp.cbSize = sizeof(NOTIFYICONDATA);
	nidApp.hWnd = (HWND)ghWnd;
	nidApp.uID = IDI_SYSTRAY;
	nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nidApp.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	nidApp.uCallbackMessage = WM_USER_SHELLICON;
	LoadString(hInstance, IDS_APPTOOLTIP, nidApp.szTip, MAX_LOADSTRING);

	int i = 0;
	gIcons[i++] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	gIcons[i++] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
	gIcons[i++] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
	gIcons[i++] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON4));
	gIcons[i++] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON5));

	Shell_NotifyIcon(NIM_ADD, &nidApp);

	if (ERROR_SUCCESS != PdhOpenQuery(NULL, NULL, &Query))
	{
		MessageBox(NULL, L"PdhOpenQuery failed", L"", MB_ICONERROR | MB_OK);
	}
	if (ERROR_SUCCESS != PdhAddCounter(Query, L"\\PhysicalDisk(*)\\% Disk Read Time", 0, &gCounter[0]))
	{
		MessageBox(NULL, L"PdhAddCounter (read) failed", L"", MB_ICONERROR | MB_OK);
	}
	if (ERROR_SUCCESS != PdhAddCounter(Query, L"\\PhysicalDisk(*)\\% Disk Write Time", 0, &gCounter[1]))
	{
		MessageBox(NULL, L"PdhAddCounter (write) failed", L"", MB_ICONERROR | MB_OK);
	}


	SetTimer(ghWnd, IDT_TIMER1, POLL_INTERVAL, NULL);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Cleanup
	KillTimer(ghWnd, IDT_TIMER1);
	Shell_NotifyIcon(NIM_DELETE, &nidApp);
	if (Query)
	{
		PdhCloseQuery(Query);
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HDDBLINK));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_HDDBLINK);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	ghWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!ghWnd)
	{
		return FALSE;
	}

	ShowWindow(ghWnd, nCmdShow);
	UpdateWindow(ghWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_TIMER:
		PollHdd();
		break;
	case WM_USER_SHELLICON:
		// systray msg callback 
		switch (LOWORD(lParam))
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if (IDCANCEL == MessageBox(NULL, L"hddblink\n\n [Cancel] - exit program", L"about", MB_OKCANCEL))
			{
				DestroyWindow(ghWnd);
			}
		}
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void SetIcon(int icon)
{
	static int prev = -1;
	if (prev != icon)
	{
		prev = icon;
		nidApp.uFlags = NIF_ICON;
		nidApp.hIcon = gIcons[icon];
		Shell_NotifyIcon(NIM_MODIFY, &nidApp);
	}
}

void PollHdd()
{
	//static unsigned long f = 1;
	//if (f % 2)
	//	SetIcon(1);
	//else
	//	SetIcon(2);
	//f++;

	DWORD dwtype;
	PDH_RAW_COUNTER cvalue[2];
	static LONGLONG prevv[2] = {};
	LONGLONG res[2];

	if (ERROR_SUCCESS != PdhCollectQueryData(Query))
	{
		goto poll_error;
	}
	if (ERROR_SUCCESS != PdhGetRawCounterValue(gCounter[0], &dwtype, &cvalue[0]))
	{
		goto poll_error;
	}
	if (ERROR_SUCCESS != PdhGetRawCounterValue(gCounter[1], &dwtype, &cvalue[1]))
	{
		goto poll_error;
	}
	res[0] = prevv[0] - cvalue[0].FirstValue;
	res[1] = prevv[1] - cvalue[1].FirstValue;
	prevv[0] = cvalue[0].FirstValue;
	prevv[1] = cvalue[1].FirstValue;
	if (!res[0] && !res[1])
	{
		SetIcon(0);
	}
	else if (res[0] && !res[1])
	{
		SetIcon(1);
	}
	else if (!res[0] && res[1])
	{
		SetIcon(2);
	}
	else
	{
		SetIcon(3);
	}

	return;

poll_error:
	KillTimer(ghWnd, IDT_TIMER1);
	SetIcon(4);
}
