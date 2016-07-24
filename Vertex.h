#pragma once

#include "math.h"

struct Color { float r; float g; float b; };

struct Vertex
{
	Vector pos; // u,v color, rhw(น้าปปฏ)
	Color color;
};
