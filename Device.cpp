#include "Device.h"
#include <Windows.h>
#include <math.h>

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

void Device::clear()
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			framebuffer[y][x] = background;
		}
	}
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

	int x1, y1, x2, y2;
	x1 = p1.x;
	y1 = p1.y;
	x2 = p2.x;
	y2 = p2.y;

	float y,x;
	y = p1.y;
	x = p1.x;

	if (x1 == x2 && y1 == y2) {
		drawPoint(p1);
	}
	else if (x1 == x2) {
		drawPoint(p1);

		int inc = (y1 < y2) ? 1 : -1;
		while (1) {
			y += inc;
			if (int(y) == y2) break;
			Vector p = {x, y, 0.f, 1.f};
			drawPoint(p);
		}

		drawPoint(p2);
	}
	else if (y1 == y2) {
		drawPoint(p1);
		
		int inc = (x1 < x2) ? 1 : -1;
		while (1) {
			x += inc;
			if (int(x) == x2) break;
			Vector p = { x, y, 0.f, 1.f };
			drawPoint(p);
		}

		drawPoint(p2);
	}
	else {
		drawPoint(p1);

		float t = (float)abs(x2 - x1) / abs(y2 - y1);
		int xinc = (p1.x < p2.x) ? 1 : -1;
		int yinc = (p1.y < p2.y) ? 1 : -1;
		while (1) {
			y += yinc;
			if (int(y) == y2) break;
			x += t * xinc;
			drawPoint({x,y,0.f,1.f});
		}

		drawPoint(p2);
	}

}
