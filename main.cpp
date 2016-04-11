#include <Windows.h>
#include <WinUser.h>
#include <windef.h>
#include <WinBase.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// User specified events
	switch (uMsg)
	{
	case WM_CREATE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

class App
{
public:
	HRESULT Init(HINSTANCE hInstance);
	void Quit();

private:
	HWND mMainWindow;
};

HRESULT App::Init(HINSTANCE hInstance)
{
	// Create Window Class
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.lpszClassName = L"D3DWND";

	RECT rc = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	// Register Class and Create new Window
	RegisterClass(&wc);

	mMainWindow = CreateWindow(
		L"D3DWND", L"SoftRendering", WS_OVERLAPPEDWINDOW, 0, 0,
		rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, 0);

	if (mMainWindow == NULL)
	{
		return E_FAIL;
	}

	ShowWindow(mMainWindow, SW_SHOW);
	UpdateWindow(mMainWindow);

	return S_OK;
}

void App::Quit()
{
	DestroyWindow(mMainWindow);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	App app;
	HRESULT ret = app.Init(hInstance);
	if (FAILED(ret))
	{
		exit(1);
	}

	MSG msg;
	memset(&msg, 0, sizeof(msg));

	// Keep track of the time 
	DWORD startTime = GetTickCount();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			DWORD t = GetTickCount();
			float deltaTime = (t - startTime) * 0.001f;

			// TODO Rendering

			startTime = t;
		}

	}

	return 0;
}