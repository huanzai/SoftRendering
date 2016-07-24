#pragma once

// 设备渲染

#include "Config.h"

class Transform;
struct Vertex;
struct Vector;
struct Color;

class Device
{
public:
	Device();

	void init(int w, int h, uint32* fb, Transform* ts);
	void clear();
	void close();

	void drawPoint(const Vector& p, const Color& color);
	void drawLine(const Vector& p1, const Vector& p2);
	void drawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3);

private:
	Transform* transform;
	uint32 ** framebuffer;
	float * zbuffer; 
	uint32 background; // 背景颜色
	uint32 foreground; // 线框颜色
	int width;
	int height; 
};