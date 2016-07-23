#include "Screen.h"
#include <windows.h>
#include <tchar.h>

int ScreenKeys[512]; // 记录键盘是否按下
int Exit = 0;

static LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:Exit = 1; break;
	case WM_KEYDOWN:ScreenKeys[wParam & 511] = 1; break;
	case WM_KEYUP:ScreenKeys[wParam & 511] = 0; break;
	default:return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

int Screen::init(int width, int height, LPCTSTR title)
{
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)WndProc, 0, 0, 0, NULL, NULL, NULL, NULL, _T("SCREEN") };
	// -height 表示 top-down
	BITMAPINFO bi = { {sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB, width * height * 4, 0, 0, 0, 0} };
	RECT rect = { 0, 0, width, height };

	this->close();

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	wndHandle = CreateWindow(_T("SCREEN"), title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (wndHandle == NULL) return -2;

	HDC hDC = GetDC(wndHandle);
	wndDc = CreateCompatibleDC(hDC);
	ReleaseDC(wndHandle, hDC);

	wndHb = CreateDIBSection(wndDc, &bi, DIB_RGB_COLORS, &wndFramebuffer, 0, 0);
	if (wndHb == NULL) return -3;

	wndOb = (HBITMAP)SelectObject(wndDc, wndHb);
	this->width = width;
	this->height = height;

	AdjustWindowRect(&rect, GetWindowLong(wndHandle, GWL_STYLE), 0);
	int wx = rect.right - rect.left;
	int wy = rect.bottom - rect.top;
	int sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	int sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(wndHandle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(wndHandle);

	ShowWindow(wndHandle, SW_NORMAL);
	dispatch();

	memset(wndFramebuffer, 0, width * height * 4);
	memset(ScreenKeys, 0, sizeof(int) * 512);

	return 0;
}

void Screen::update()
{
	HDC hDC = GetDC(wndHandle);
	BitBlt(hDC, 0, 0, width, height, wndDc, 0, 0, SRCCOPY);
	ReleaseDC(wndHandle, hDC);

	dispatch();
}

void Screen::dispatch()
{
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void Screen::close()
{
	if (wndDc) {
		if (wndOb) {
			SelectObject(wndDc, wndOb);
			wndOb = NULL;
		}
		DeleteDC(wndDc);
		wndDc = NULL;
	}

	if (wndHb) {
		DeleteObject(wndHb);
		wndHb = NULL;
	}
	
	if (wndHandle) {
		CloseWindow(wndHandle);
		wndHandle = NULL;
	}
}

int Screen::isKeyPressed(int key)
{
	return ScreenKeys[key & 511];
}

int Screen::isExit()
{
	return Exit;
}

LPVOID Screen::getFrameBuffer() 
{
	return wndFramebuffer;
}