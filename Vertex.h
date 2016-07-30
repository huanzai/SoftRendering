#pragma once

#include "math.h"

struct Color { float r; float g; float b; };
struct Texcoord { float u; float v; };

struct Vertex
{
	Vector pos;
	Color color;
	Texcoord tex;
	Vector normal;
	float rhw; // Í¸ÊÓ½ÃÕý
};
