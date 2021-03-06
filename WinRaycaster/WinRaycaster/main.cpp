#include "stdafx.h"
#include "Resource.h"


HINSTANCE	hInst;							// current instance
HWND		hWnd;

Game game;


#pragma region WINDOWS INTERNALS

#define MAX_LOADSTRING 100

WCHAR szTitle[MAX_LOADSTRING];				// The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];		// the main window class name

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINRAYCASTER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	//wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINRAYCASTER);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

		break;
	}

	return (INT_PTR)FALSE;
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance;

	RECT rect;
	rect.left = rect.top = 0;
	rect.right = SCRN_W;
	rect.bottom = SCRN_H;
	
#ifdef SHOWDEBUG
	//rect.right += DebugScl * 11; rect.bottom = max(rect.bottom, DebugScl * 11);
	rect.right  = max(rect.right,  DebugScl * 16);
	rect.bottom = max(rect.bottom, DebugScl * 16);
#endif

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, true);

	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);

	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_COMMAND: {
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
		break;
	}

	/*case WM_PAINT: {
		// RENDERING CODE
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		//if (game.pCam)
		//	game.Render();

		EndPaint(hWnd, &ps);
		break;
	}*/

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

#pragma endregion


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	XMVerifyCPUSupport();

	/*
	Quad Root(3);	// n=2, 2^2 = 4x4
	Root.Subdivide();
	Root(qTL).Divide()(qBR).Divide();
	
	Quad &q02 = Root.FindCell(Int2(4, 3));
	Quad &q12 = Root.FindCell(Int2(3, 3));
	Quad &q11 = Root.FindCell(Int2(2, 3));
	Quad &q07 = Root.FindCell(Int2(1, 3));
	*/
	/*Quad *pQuad = q.AscendWest(&q, 3, 0);
	int TorB = ((q.Pos.y - pQuad->Pos.y) >> q.Depth) << 1;
	pQuad = pQuad->DescendH(2, TorB);*/

	DebugX   = 0;//SCRN_W;//640;
	DebugY   = 0;
	DebugScl = 40;

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINRAYCASTER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	ULONG_PTR gdiplusToken;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

#ifdef SHOWDEBUG
	game.pGfx = new Graphics(hWnd);
#endif

	//DXHelper DX;
	if (DX.Init(hWnd, SCRN_W, SCRN_H))
		return 1;

	game.InitGame();

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINRAYCASTER));

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// Main message loop:
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			continue;
		}

		static uint64 LastFPS = 0, Frames = 0;
		static uint64 LastTime = 0, ThisTime;
		QueryUnbiasedInterruptTime(&ThisTime);

		if (!LastTime) {
			LastTime = LastFPS = ThisTime;
			continue;
		}

		float DeltaTime = (float)(ThisTime - LastTime) / 10e6f;
		LastTime = ThisTime;

		game.Tick(DeltaTime, hWnd);
		DX.Render(game.pCam, false);

		Frames++;
		QueryUnbiasedInterruptTime(&ThisTime);
		if (ThisTime - LastFPS > 10e6) {
			const double Time = (double)(ThisTime - LastFPS) / 10e6f;

			wchar_t str[128];
			swprintf_s(str, L"WinRaycaster - %.03f FPS", (double)Frames / Time);
			SetWindowText(hWnd, str);

			LastFPS = ThisTime; Frames = 0;
		}
	}

	DX.Release();
	GdiplusShutdown(gdiplusToken);

	return (int)msg.wParam;
}