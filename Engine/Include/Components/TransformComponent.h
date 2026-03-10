#pragma once

struct Transform
{
	Transform() = default;

	Transform(float x, float y, float z)
		: x(x), y(y), z(z)
	{

	}

	float x = 0;
	float y = 0;
	float z = 0;
};