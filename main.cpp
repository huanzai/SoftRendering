#include <Windows.h>
#include <WinUser.h>
#include <windef.h>
#include <WinBase.h>
#include <time.h>
#include <assert.h>
#include <map>
#include <vector>
#include <math.h>

#include "Screen.h"
#include "Device.h"
#include "Config.h"
#include <fcntl.h>
#include <io.h>
#include <tchar.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

Screen* screen = NULL;
Device* device = NULL;

void InitConsoleWindow()
{
	int nCrt;
	FILE* fp;
	AllocConsole();
	nCrt = _open_osfhandle(reinterpret_cast<long>(GetStdHandle(STD_OUTPUT_HANDLE)), _O_TEXT);
	fp = _fdopen(nCrt, "w");
	*stdout = *fp;
	setvbuf(stdout, nullptr, _IONBF, 0);
}

void DrawLine()
{
	Vertex v1 = { {100.f, 399.f, 0.f, 1.f} };
	Vertex v2 = { {259.f, 399.f, 0.f, 1.f} };
	device->drawLine(v1, v2);
}

void DrawLine2()
{
	Vertex v1 = { {150.f, 10.f, 0.f, 1.f} };
	Vertex v2 = { {199.f, 450.f, 0.f, 1.f} };
	device->drawLine(v1, v2);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	InitConsoleWindow();

	screen = new Screen();
	int ret = screen->init(WINDOW_WIDTH, WINDOW_HEIGHT, _T("SoftRendering"));
	if (ret < 0) {
		printf("screen init failed(%d)!\n", ret);
		exit(ret);
	}

	uint32* wfb = (uint32*)(screen->getFrameBuffer());
	device = new Device();
	device->init(WINDOW_WIDTH, WINDOW_HEIGHT, wfb);

	while (!screen->isExit()){
		screen->dispatch();

		DrawLine();
		DrawLine2();

		screen->update();
		printf("=======test\n");
		Sleep(1);
	}

	return 0;
}