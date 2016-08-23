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
#include "Vertex.h"
#include "Light.h"
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

Screen* screen = NULL;
Device* device = NULL;
Transform* transform = NULL;
int *textures[3] = { 0,0,0 };

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
	Vector v1 = {0.f, 0.f, 0.f, 1.f};
	Vector v2 = {799.f, 599.f, 0.f, 1.f};
	device->drawLine(v1, v2);
}

void DrawLine2()
{
	Vector v1 = {799.f, 0.f, 0.f, 1.f};
	Vector v2 = {0.f, 599.f, 0.f, 1.f};
	device->drawLine(v1, v2);
}

int GetTriangleInterp(const Vector& v1, const Vector& v2, const Vector& v3, const Vector& p, float *u, float *v)
{
	Vector ca, ba, pa;
	VectorSub(ca, v3, v1);
	VectorSub(ba, v2, v1);
	VectorSub(pa, p, v1);

	float dot00 = VectorDotProduct(ca, ca);
	float dot01 = VectorDotProduct(ca, ba);
	float dot02 = VectorDotProduct(ca, pa);
	float dot11 = VectorDotProduct(ba, ba);
	float dot12 = VectorDotProduct(ba, pa);

	float inverDeno = 1/(dot00 * dot11 - dot01 * dot01);

	*u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
	if (*u < 0 || *u > 1) return 0; // out of triangle

	*v = (dot00 * dot12 - dot01 * dot02) * inverDeno;
	if (*v < 0 || *v > 1) return 0; // out of triangle

	return *u + *v <= 1;
}

void DrawColorTriangle()
{
	Vertex v1 = { {350.f, 250.f, 0.f, 1.f}, { 1.f, 0.f, 0.f } };
	Vertex v2 = { {450.f, 250.f, 0.f, 1.f}, { 0.f, 1.f, 0.f } };
	Vertex v3 = { {400.f, 350.f, 0.f, 1.f}, { 0.f, 0.f, 1.f } };

	Vector p1, p2, p3, min, max;
	p1 = v1.pos; p2 = v2.pos; p3 = v3.pos;
	min = p1; max = p1;

	if (p2.x < min.x) min.x = p2.x;
	if (p3.x < min.x) min.x = p3.x;
	if (p2.y < min.y) min.y = p2.y;
	if (p3.y < min.y) min.y = p3.y;
	
	if (p2.x > max.x) max.x = p2.x;
	if (p3.x > max.x) max.x = p3.x;
	if (p2.y > max.y) max.y = p2.y;
	if (p3.y > max.y) max.y = p3.y;

	for (int j = min.y; j < max.y; j++) {
		for (int i = min.x; i < max.x; i++) {
			Vector p = {i,j,0.f,1.f};
			float u, v;
			if (GetTriangleInterp(p1, p2, p3, p, &u, &v)) {
				float r = v1.color.r * u + v2.color.r * v + v3.color.r * (1 - u - v);
				float g = v1.color.g * u + v2.color.g * v + v3.color.g * (1 - u - v);
				float b = v1.color.b * u + v2.color.b * v + v3.color.b * (1 - u - v);

				device->drawPoint(p, { r,g,b }, { 0.f, 0.f }, { 0.f, 0.f, 0.f });
			}
		}
	}
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

void FixNormal(Vertex& v1, Vertex& v2, Vertex& v3);

void DrawPlane(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4)
{
	// 必须重新制定一下纹理，不然就乱掉了
	Vertex t1 = v1, t2 = v2, t3 = v3, t4 = v4;
	t1.tex.u = 0.f; t2.tex.u = 1.f; t3.tex.u = 1.f; t4.tex.u = 0.f;
	t1.tex.v = 0.f; t2.tex.v = 0.f; t3.tex.v = 1.f; t4.tex.v = 1.f;

	device->drawTriangle(t1, t2, t3);
	device->drawTriangle(t3, t4, t1);
}

int* CreateTexture()
{
	int *tex = (int*)malloc(100 * 100 * sizeof(int));

	int color[10][10];

	int c = 0x11eeee;
	for (int i = 0; i < 10; i++) {
		int c1 = c;
		for (int j = 0; j < 10; j++) {
			color[i][j] = c1;
			c1 = c1 == 0xffffff ? 0x11eeee : 0xffffff;
		}
		c = c == 0xffffff ? 0x11eeee : 0xffffff;
	}

	// 100 x 100
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			tex[i * 100 + j] = color[i / 10][j / 10];
		}
	}

	return tex;
}

void DrawBox(float theta)
{
	Matrix mm;
	MatrixSetRotate(mm, 0.f, 1.0f, 0.f, theta);

	Matrix tm;
	MatrixSetTranslate(tm, 5.0f, 0.f, 0.f);

	Matrix m;
	MatrixMul(m, mm, tm);

	transform->setWorld(m);
	transform->update();

	Vertex vs[8] = {
		{{-1.f,  1.f, -1.f, 1.f}, {1.0f, 0.0f, 0.0f}, {0.f, 0.f}, {-1.f,  1.f, -1.f, 0.f},  1.f },
		{{-1.f,  1.f,  1.f, 1.f}, {0.0f, 1.0f, 0.0f}, {0.f, 1.f}, {-1.f,  1.f,  1.f, 0.f}, 1.f },
		{{ 1.f,  1.f,  1.f, 1.f}, {0.0f, 0.0f, 1.0f}, {1.f, 1.f}, { 1.f,  1.f,  1.f, 0.f}, 1.f },
		{{ 1.f,  1.f, -1.f, 1.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}, { 1.f,  1.f, -1.f, 0.f}, 1.f },

		{{-1.f, -1.f, -1.f, 1.f}, {0.0f, 0.0f, 1.0f}, {1.f, 0.f}, {-1.f, -1.f, -1.f, 0.f}, 1.f },
		{{-1.f, -1.f,  1.f, 1.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}, {-1.f, -1.f,  1.f, 0.f}, 1.f },
		{{ 1.f, -1.f,  1.f, 1.f}, {1.0f, 0.0f, 0.0f}, {1.f, 1.f}, { 1.f, -1.f,  1.f, 0.f}, 1.f },
		{{ 1.f, -1.f, -1.f, 1.f}, {0.0f, 1.0f, 0.0f}, {1.f, 1.f}, { 1.f, -1.f, -1.f, 0.f}, 1.f },
	};

	DrawPlane(vs[0], vs[1], vs[2], vs[3]);
	DrawPlane(vs[7], vs[4], vs[0], vs[3]);
	DrawPlane(vs[2], vs[6], vs[7], vs[3]);
	DrawPlane(vs[5], vs[1], vs[0], vs[4]);
	DrawPlane(vs[2], vs[1], vs[5], vs[6]);
	DrawPlane(vs[6], vs[5], vs[4], vs[7]);
}

void DrawPlane(float theta)
{
	Matrix m;
	MatrixSetRotate(m, 1.f, 0.0f, 0.f, theta);
	transform->setWorld(m);
	transform->update();

	Vertex vs[4] = { 
		{{-1.f, 0.f, 0.f, 1.f}, {1.f, 0.f, 0.f}, {0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, 1.f }, 
		{{-1.f, 0.f, 10.f, 1.f}, {0.f, 0.5f, 0.f}, {0.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, 1.f }, 
		{{ 1.f, 0.f, 10.f, 1.f}, {0.f, 0.5f, 0.f}, {1.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, 1.f }, 
		{{ 1.f, 0.f, 0.f, 1.f}, {1.f, 0.f, 0.f}, {0.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, 1.f },
	};

	DrawPlane(vs[0], vs[1], vs[2], vs[3]);
}

void DrawTetrahedron(float theta)
{
	Matrix m;
	MatrixSetRotate(m, 0.f, 1.f, 0.f, theta);
	transform->setWorld(m);
	transform->update();

	Vertex vs[4] = {
		{ {  0.f,  0.f,  1.f, 1.f },{ 1.0f, 0.0f, 0.0f } },
		{ { -1.f,  0.f, -1.f, 1.f },{ 0.0f, 1.0f, 0.0f } },
		{ {  1.f,  0.f, -1.f, 1.f },{ 0.0f, 0.0f, 1.0f } },
		{ {  0.f,  2.f,  0.f, 1.f },{ 1.0f, 1.0f, 1.0f } },
	};

	device->drawTriangle(vs[1], vs[3], vs[2]);
	device->drawTriangle(vs[2], vs[3], vs[0]);
	device->drawTriangle(vs[0], vs[3], vs[1]);
	device->drawTriangle(vs[0], vs[1], vs[2]);
}

struct Face {
	int i1, i2, i3;
};

char *ReadFile(const char* file)
{
	FILE *pFile = fopen(file, "r");
	if (!pFile) {
		return NULL;
	}

	char *pBuf;
	fseek(pFile, 0, SEEK_END);
	int len = ftell(pFile);
	pBuf = new char[len + 1];
	rewind(pFile);
	fread(pBuf, 1, len, pFile);
	pBuf[len] = '\0';
	fclose(pFile);
	return pBuf;
}

int LoadMesh(const char *file, Vertex*& pVertexs, int& vsize, Face*& pFaces, int& fsize)
{
	char* pFile;
	pFile = ReadFile(file);
	if (!pFile) {
		return 0;
	}

	char* pSrc;
	pSrc = pFile;

	int i, vc, fc;

	// 计算顶点和面的个数
	i = 0, vc = 0, fc = 0;
	char line[1024];
	memset(line, 0, 1024);
	for (; *pSrc != '\0';) {
		if (*pSrc == '\n') {
			if (line[0] == 'v') {
				++vc;
			}
			else if (line[0] == 'f') {
				++fc;
			}

			i = 0;
			memset(line, 0, 1024);
		}
		else {
			line[i++] = *pSrc;
		}
		++pSrc;
	}
	if (vc == 0 || fc == 0) {
		delete pFile;
		return 0;
	}

	vsize = vc; fsize = fc;
	pVertexs = new Vertex[vc];
	pFaces = new Face[fc];

	pSrc = pFile;

	// 读取数据
	i = 0, vc = 0, fc = 0;
	memset(line, 0, 1024);
	for (; *pSrc != '\0';) {
		if (*pSrc == '\n') {
			if (line[0] == 'v') {
				float x, y, z;
				sscanf(line, "v %f %f %f", &x, &y, &z);

				pVertexs[vc++] = { {x, y, z, 1.f}, {0.f, 0.f, 0.f},{0.f, 1.f}, {0.f, 0.f, 0.f}, 1.f };
			}
			else if (line[0] == 'f') {
				int p1, p2, p3;
				sscanf(line, "f %d %d %d", &p1, &p2, &p3);

				pFaces[fc++] = { p1 - 1, p2 - 1, p3 -1 };
			}

			i = 0;
			memset(line, 0, 1024);
		}
		else {
			line[i++] = *pSrc;
		}
		++pSrc;
	}
	
	delete pFile;

	return 1;
}

void FixNormal(Vertex& v1, Vertex& v2, Vertex& v3)
{
	Vector& p1 = v1.pos;
	Vector& p2 = v2.pos;
	Vector& p3 = v3.pos;

	Vector edge1, edge2, pn;
	VectorSub(edge1, p2, p1);
	VectorSub(edge2, p3, p2);
	VectorCrossProduct(pn, edge1, edge2);
	VectorNormalize(pn);

	pn.w = 0.f;

	v1.normal = pn;
	v2.normal = pn;
	v3.normal = pn;
}

void FixUv(Vertex& v1, Vertex& v2, Vertex& v3)
{
	v1.tex.u = 0.0f; v1.tex.v = 0.0f;
	v2.tex.u = 0.0f; v2.tex.v = 0.0f;
	v3.tex.u = 0.0f; v3.tex.v = 0.0f;
}

void DrawModel(Vertex* pVertexs, int vsize, Face* pFaces, int fsize, float theta)
{
	Matrix m;
	MatrixSetRotate(m, 0.f, 1.0f, 0.f, theta);
	transform->setWorld(m);
	transform->update();

	int i;
	for (i = 0; i < fsize; i++) {
		int i1 = pFaces[i].i1;
		int i2 = pFaces[i].i2;
		int i3 = pFaces[i].i3;

		Vertex v1 = pVertexs[i1];
		Vertex v2 = pVertexs[i2];
		Vertex v3 = pVertexs[i3];

		FixNormal(v1, v2, v3);
		FixUv(v1, v2, v3);
		device->drawTriangle(v1, v2, v3);
	}
}

void SetCamera(float x, float y, float z)
{
	Vector eye = { x, y, z, 1.f }, at = { 0.f, 0.f, 0.f, 1.f }, up = {0.f, 1.f, 0.f, 1.f};
	Matrix m;
	MatrixSetLookAt(m, eye, at, up);
	transform->setView(m);
	transform->update();
}

void TransformLight(Light& light, float theta)
{
	Matrix m;
	MatrixSetRotate(m, 0.f, 1.0f, 0.f, theta);
	transform->setWorld(m);
	transform->update();

	transform->applyMV(light.direction, { -0.3f, 1.f, -0.3f, 0.f });
	VectorNormalize(light.direction);
}

#define VK_J 0x4A
#define VK_K 0x4B

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	InitConsoleWindow();

	srand((unsigned)time(NULL));

	screen = new Screen();
	int ret = screen->init(WINDOW_WIDTH, WINDOW_HEIGHT, _T("SoftRendering"));
	if (ret < 0) {
		printf("screen init failed(%d)!\n", ret);
		exit(ret);
	}

	transform = new Transform();
	transform->init(WINDOW_WIDTH, WINDOW_HEIGHT);

	textures[0] = CreateTexture();

	Light light = { {-1.f, 1.f, -1.f, 0.f}, {0.5f, 0.5f, 0.5f} };

	uint32* wfb = (uint32*)(screen->getFrameBuffer());
	device = new Device();
	device->init(WINDOW_WIDTH, WINDOW_HEIGHT, wfb, transform, textures, &light);
	device->setState(1);

	int vsize, fsize;
	Vertex* pVertexs;
	Face* pFaces;
	LoadMesh("models/cow.obj", pVertexs, vsize, pFaces, fsize);

	float theta = 1.f;
	float dist = 3.f;

	float light_theta = 1.f;
	while (!screen->isExit()){
		device->clear();
		screen->dispatch();
		SetCamera(3.f, 3.f, dist);

		light_theta += 0.03f;
		TransformLight(light, light_theta);

		if (screen->isKeyPressed(VK_UP))
			dist -= 0.05f;
		if (screen->isKeyPressed(VK_DOWN))
			dist += 0.05f;

		if (screen->isKeyPressed(VK_LEFT)) 
			theta += 0.01f;
		if (screen->isKeyPressed(VK_RIGHT)) 
			theta -= 0.01f;

		if (screen->getKeyUpEvent(VK_SPACE)) 
			device->autoChangeState();
			
		if (screen->getKeyUpEvent(VK_J)) // J key
			device->autoChangeInterp();

		if (screen->getKeyUpEvent(VK_K)) // K key
			device->autoChangeCullMode();

		//DrawLine();
		DrawBox(theta);
		DrawModel(pVertexs, vsize, pFaces, fsize, theta);
		//DrawPlane(theta);
		//DrawTetrahedron(theta);
		//DrawColorTriangle();

		screen->dispatch();
		screen->update();
		Sleep(1);
	}

	device->close();
	screen->close();

	for (int i = 0; i < 3; i++) {
		if (textures[i]) {
			delete textures[i];
		}
	}

	delete[] pVertexs;
	delete[] pFaces;

	delete transform;
	delete device;
	delete screen;

	return 0;
}