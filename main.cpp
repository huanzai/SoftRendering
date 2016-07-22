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
#include "Config.h"
#include <fcntl.h>
#include <io.h>
#include <tchar.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

Screen* screen_ptr = NULL;

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	InitConsoleWindow();

	screen_ptr = new Screen();
	int ret = screen_ptr->init(WINDOW_WIDTH, WINDOW_HEIGHT, _T("SoftRendering"));
	if (ret < 0) {
		printf("screen init failed(%d)!\n", ret);
		exit(ret);
	}

	while (!screen_ptr->isExit()){
		screen_ptr->dispatch();

		uint32* fb = (uint32*)(screen_ptr->getFrameBuffer());
		for (int y = 0; y < WINDOW_WIDTH * WINDOW_HEIGHT; y++) {
			fb[y] = 0xc0c0c0;
			if (y < WINDOW_WIDTH * WINDOW_HEIGHT / 2) fb[y] = 0x506070;
		}

		screen_ptr->update();
		printf("=======test\n");
		Sleep(1);
	}

	return 0;
}