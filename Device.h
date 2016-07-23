#pragma once

// 设备渲染

#include "Config.h"
#include "Vertex.h"

class Device
{
public:
	Device();

	void init(int w, int h, uint32* fb);
	void clear();
	void close();

	void drawPoint(const Vector& p);
	void drawLine(const Vertex& v1, const Vertex& v2);

private:
	uint32 ** framebuffer;
	float ** zbuffer; 
	uint32 background; // 背景颜色
	uint32 foreground; // 线框颜色
	int width;
	int height; 
};