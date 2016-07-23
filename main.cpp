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
#include "Transform.h"
#include "Config.h"
#include <fcntl.h>
#include <io.h>
#include <tchar.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

Screen* screen = NULL;
Device* device = NULL;
Transform* transform = NULL;

void InitConsoleWindow()
{
	int nCrt;
	FILE* fp;
	if (AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
	}
}

void DrawLine()
{
	Vertex v1 = { {0.f, 0.f, 0.f, 1.f} };
	Vertex v2 = { {799.f, 599.f, 0.f, 1.f} };
	device->drawLine(v1, v2);
}

void DrawLine2()
{
	Vertex v1 = { {799.f, 0.f, 0.f, 1.f} };
	Vertex v2 = { {0.f, 599.f, 0.f, 1.f} };
	device->drawLine(v1, v2);
}

void DrawTriangle(const Vector& p1, const Vector& p2, const Vector& p3)
{
	Vector c1, c2, c3, h1, h2, h3;

	transform->apply(c1, p1);
	transform->apply(c2, p2);
	transform->apply(c3, p3);

	if (transform->checkCvv(c1)) return;
	if (transform->checkCvv(c2)) return;
	if (transform->checkCvv(c3)) return;

	transform->homogenize(h1, c1);
	transform->homogenize(h2, c2);
	transform->homogenize(h3, c3);

	device->drawLine({ h1 }, { h2 });
	device->drawLine({ h1 }, { h3 });
	device->drawLine({ h2 }, { h3 });
}

void DrawPlane(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4)
{
	const Vector& p1 = v1.pos; const Vector& p2 = v2.pos; const Vector& p3 = v3.pos; const Vector& p4 = v4.pos;
	DrawTriangle(p1, p2, p3);
	DrawTriangle(p3, p4, p1);
}

void DrawBox(float theta)
{
	Matrix m;
	MatrixSetRotate(m, 0.f, 1.f, 0.f, theta);
	transform->setWorld(m);
	transform->update();

	Vertex vs[8] = {
		{{-1.f,  1.f, -1.f, 1.f}},
		{{-1.f,  1.f,  1.f, 1.f}},
		{{ 1.f,  1.f,  1.f, 1.f}},
		{{ 1.f,  1.f, -1.f, 1.f}},

		{{-1.f, -1.f, -1.f, 1.f}},
		{{-1.f, -1.f,  1.f, 1.f}},
		{{ 1.f, -1.f,  1.f, 1.f}},
		{{ 1.f, -1.f, -1.f, 1.f}},
	};

	DrawPlane(vs[0], vs[1], vs[2], vs[3]);
	DrawPlane(vs[7], vs[4], vs[0], vs[3]);
	DrawPlane(vs[2], vs[6], vs[7], vs[3]);
	DrawPlane(vs[5], vs[1], vs[0], vs[4]);
	DrawPlane(vs[2], vs[1], vs[5], vs[6]);
	DrawPlane(vs[6], vs[7], vs[4], vs[5]);
}

void SetCamera(float x, float y, float z)
{
	Vector eye = { x, y, z, 1.f }, at = { 0.f, 0.f, 0.f, 1.f }, up = {0.f, 1.f, 0.f, 1.f};
	Matrix m;
	MatrixSetLookAt(m, eye, at, up);
	transform->setView(m);
	transform->update();
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

	transform = new Transform();
	transform->init(WINDOW_WIDTH, WINDOW_HEIGHT);

	float theta = 1.f;
	float dist = 3.f;

	while (!screen->isExit()){
		device->clear();
		screen->dispatch();
		SetCamera(0.f, 0.f, dist);

		if (screen->isKeyPressed(VK_UP))
			dist -= 0.01f;
		if (screen->isKeyPressed(VK_DOWN))
			dist += 0.01f;

		if (screen->isKeyPressed(VK_LEFT)) 
			theta -= 0.01f;
		if (screen->isKeyPressed(VK_RIGHT)) 
			theta += 0.01f;

		printf("===============>>> %.2f  %.2f\n", dist, theta);

		//DrawLine();
		DrawBox(theta);

		screen->dispatch();
		screen->update();
		Sleep(1);
	}

	return 0;
}