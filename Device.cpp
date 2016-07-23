#include "Device.h"
#include <Windows.h>

Device::Device() : framebuffer(NULL), zbuffer(NULL), 
	background(0), foreground(0), width(0), height(0)
{
}

void Device::init(int w, int h, uint32* fb)
{
	width = w;
	height = h;

	// fb Îª´°¿ÚÏÔÊ¾»º´æ
	framebuffer = (uint32**)malloc(h * sizeof(uint32*));
	for (int y = 0; y < h; y++) {
		framebuffer[y] = fb + y*w;
	}

	background = 0; // black
	foreground = 0xffffff; // white
}

void Device::close()
{
	if (framebuffer != NULL) {
		free(framebuffer);
	}

	if (zbuffer != NULL) {
		free(zbuffer);
	}
}

void Device::drawPoint(const Vector& p)
{
	int y = (int)p.y;
	int x = (int)p.x;

	if (y >= height) return;
	if (x >= width) return;

	framebuffer[y][x] = foreground;
}

void Device::drawLine(const Vertex& v1, const Vertex& v2)
{
	const Vector& p1 = v1.pos;
	const Vector& p2 = v2.pos;

	if (p1.x == p2.x && p1.y == p2.y) {
		drawPoint(p1);
	}
	else if (p2.x == p1.x) {
		drawPoint(p1);

		int x, y;
		x = p1.x;
		y = p1.y;

		int inc = (p1.y < p2.y) ? 1 : -1;
		while (1) {
			y += inc;
			if (y == p2.y) break;
			Vector p = {x, y, 0.f, 1.f};
			drawPoint(p);
		}

		drawPoint(p2);
	}
	else if (p2.y == p1.y) {
		drawPoint(p1);
		
		int x, y;
		x = p1.x;
		y = p1.y;

		int inc = (p1.x < p2.x) ? 1 : -1;
		while (1) {
			x += inc;
			if (x == p2.x) break;
			Vector p = { x, y, 0.f, 1.f };
			drawPoint(p);
		}

		drawPoint(p2);
	}
	else {
		drawPoint(p1);

		int y,bx;
		y = p1.y;
		bx = p1.x;

		float t = (float)(p2.y - p1.y) / (p2.x - p1.x);
		int inc = (p1.y < p2.y) ? 1 : -1;
		while (1) {
			y += inc;
			if (y == p2.y) break;

			int x = bx + (int)(y / t);
			Vector p = { x, y , 0.f, 1.f };
			drawPoint(p);
		}

		drawPoint(p2);
	}

}
