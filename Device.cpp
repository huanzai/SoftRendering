#include "Device.h"
#include "Vertex.h"
#include "Transform.h"
#include <Windows.h>
#include <math.h>

Device::Device() : transform(NULL), textures(NULL), framebuffer(NULL), zbuffer(NULL), 
	background(0), foreground(0), width(0), height(0)
{
}

void Device::init(int w, int h, uint32* fb, Transform* ts, int** tex)
{
	width = w;
	height = h;

	// fb Îª´°¿ÚÏÔÊ¾»º´æ
	framebuffer = (uint32**)malloc(h * sizeof(uint32*));
	for (int y = 0; y < h; y++) {
		framebuffer[y] = fb + y*w;
	}

	zbuffer = (float*)malloc(w * h * sizeof(float));
	memset(zbuffer, 0, w * h * sizeof(float));

	background = 0xc0c0c0; 
	foreground = 0xffffff; 

	transform = ts;
	textures = tex;
}

void Device::clear()
{	
	float inv_h = (float)1 / height;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int c = (float)0xc0 * inv_h * (height - y);
			framebuffer[y][x] = (c << 16| c << 8 | c);

			zbuffer[y * width + x] = 1.f;
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

void Device::drawPoint(const Vector& p, const Color& color, const Texcoord& tc)
{
	int y = (int)p.y;
	int x = (int)p.x;

	if (zbuffer[y * width + x] < p.z) return;

	if (y >= height) return;
	if (x >= width) return;

	int r = color.r * 255;
	int g = color.g * 255;
	int b = color.b * 255;

	int *tex = textures[0];
	int i = int(tc.u * 100) * 100 + int(tc.v * 100);
	i = i >= 10000 ? 10000 : i;
	int c = tex[i];

	r += (c >> 16);
	g += (c >> 8 & 0xff);
	b += (c & 0xff);
	

	framebuffer[y][x] = (r << 16 | g << 8 | b);
	zbuffer[y * width + x] = p.z;
}

void Device::drawLine(const Vector& p1, const Vector& p2)
{
	float inv = (float)1 / 255;
	Color color = { (foreground >> 16) * inv, (foreground >> 8 & 0xff) * inv, (foreground & 0xff) * inv };
	Texcoord tex = {0.f, 0.f};

	int x1, y1, x2, y2;
	x1 = p1.x;
	y1 = p1.y;
	x2 = p2.x;
	y2 = p2.y;

	float y,x;
	y = p1.y;
	x = p1.x;

	if (x1 == x2 && y1 == y2) {
		drawPoint(p1, color, tex);
	}
	else if (x1 == x2) {
		drawPoint(p1, color, tex);

		int inc = (y1 < y2) ? 1 : -1;
		while (1) {
			y += inc;
			if (int(y) == y2) break;
			Vector p = {x, y, 0.f, 1.f};
			drawPoint(p, color, tex);
		}

		drawPoint(p2, color, tex);
	}
	else if (y1 == y2) {
		drawPoint(p1, color, tex);
		
		int inc = (x1 < x2) ? 1 : -1;
		while (1) {
			x += inc;
			if (int(x) == x2) break;
			Vector p = { x, y, 0.f, 1.f };
			drawPoint(p, color, tex);
		}

		drawPoint(p2, color, tex);
	}
	else {
		drawPoint(p1, color, tex);

		float t = (float)abs(x2 - x1) / abs(y2 - y1);
		int xinc = (p1.x < p2.x) ? 1 : -1;
		int yinc = (p1.y < p2.y) ? 1 : -1;
		while (1) {
			y += yinc;
			if (int(y) == y2) break;
			x += t * xinc;
			drawPoint({x,y,0.f,1.f}, color, tex);
		}

		drawPoint(p2, color, tex);
	}
}

int getTriangleInterp(const Vector& v1, const Vector& v2, const Vector& v3, const Vector& p, float *u, float *v)
{
	float a, b, c;

	c = ((v1.y - v2.y)*p.x + (v2.x - v1.x)*p.y + v1.x*v2.y - v2.x*v1.y) / ((v1.y - v2.y)*v3.x + (v2.x - v1.x)*v3.y + v1.x*v2.y - v2.x*v1.y);
	if (c < 0 || c > 1) return 0;
	b = ((v1.y - v3.y)*p.x + (v3.x - v1.x)*p.y + v1.x*v3.y - v3.x*v1.y) / ((v1.y - v3.y)*v2.x + (v3.x - v1.x)*v2.y + v1.x*v3.y - v3.x*v1.y);
	if (b < 0 || b > 1) return 0;
	a = 1 - b - c;
	if (a < 0 || a > 1) return 0;

	*u = a; *v = b;
	return 1;
}

void VertexRhwInit(Vertex& v)
{
	v.rhw = 1 / v.pos.w;
	v.color.r *= v.rhw;
	v.color.g *= v.rhw;
	v.color.b *= v.rhw;
	v.tex.u *= v.rhw;
	v.tex.v *= v.rhw;
}

void Device::drawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	Vector c1, c2, c3;

	transform->apply(c1, v1.pos);
	transform->apply(c2, v2.pos);
	transform->apply(c3, v3.pos);

	if (transform->checkCvv(c1)) return;
	if (transform->checkCvv(c2)) return;
	if (transform->checkCvv(c3)) return;

	Vector p1, p2, p3, min, max;

	transform->homogenize(p1, c1);
	transform->homogenize(p2, c2);
	transform->homogenize(p3, c3);

	min = p1; max = p1;

	if (p2.x < min.x) min.x = p2.x;
	if (p3.x < min.x) min.x = p3.x;
	if (p2.y < min.y) min.y = p2.y;
	if (p3.y < min.y) min.y = p3.y;

	if (p2.x > max.x) max.x = p2.x;
	if (p3.x > max.x) max.x = p3.x;
	if (p2.y > max.y) max.y = p2.y;
	if (p3.y > max.y) max.y = p3.y;

	Vertex t1 = v1, t2 = v2, t3 = v3;
	t1.pos = p1; t2.pos = p2; t3.pos = p3;
	t1.pos.w = c1.w; t2.pos.w = c2.w; t3.pos.w = c3.w;

	VertexRhwInit(t1);
	VertexRhwInit(t2);
	VertexRhwInit(t3);

	for (int j = min.y; j < max.y; j++) {
		for (int i = min.x; i < max.x; i++) {
			Vector p = { i,j,0.f,1.f };
			float c1, c2, c3;
			if (getTriangleInterp(p1, p2, p3, p, &c1, &c2)) {
				c3 = 1.f - c1 - c2;

				// Rectification for Perspective
				float w = 1 / (c1 * t1.rhw + c2 * t2.rhw + c3 * t3.rhw);

				float r = t1.color.r * c1 + t2.color.r * c2 + t3.color.r * c3;
				float g = t1.color.g * c1 + t2.color.g * c2 + t3.color.g * c3;
				float b = t1.color.b * c1 + t2.color.b * c2 + t3.color.b * c3;

				float u = t1.tex.u * c1 + t2.tex.u * c2 + t3.tex.u * c3;
				float v = t1.tex.v * c1 + t2.tex.v * c2 + t3.tex.v * c3;

				float z = t1.pos.z * t1.rhw * c1 + t2.pos.z * t2.rhw * c2 + t3.pos.z * t3.rhw * c3;
				p.z = w * z;

				drawPoint(p, { w * r, w * g, w * b }, { w * u, w * v});
			}
		}
	}

//	drawLine(p1, p2);
//	drawLine(p1, p3);
//	drawLine(p2, p3);
}
