#include "Math/Transform.hpp"

#define THISFILE "Math/Transform.cpp"

Fmat4 TranslateModel(Fvec2 position)
{
	return TranslateModel(Fvec3(position.x, position.y, 0.0f));
}

Fmat4 TranslateModel(Fvec3 position)
{
	return {
		Fvec4(1, 0, 0, 0),
		Fvec4(0, 1, 0, 0),
		Fvec4(0, 0, 1, 0),
		position & 1
	};
}

Fmat4 RotateModel(float rad, Fvec3 axis)
{
	if (rad == 0)
		return Fmat4(1);
	axis = Normalize(axis);
	float c = std::cos(rad), s = std::sin(rad);
	auto fun1 = [&](int i) -> float { return c + axis[i] * axis[i] * (1 - c); };
	auto fun2 = [&](int i, int j, int k) -> float { return (1 - c) * axis[i] * axis[j] + s * axis[k]; };
	auto fun3 = [&](int i, int j, int k) -> float { return (1 - c) * axis[i] * axis[j] - s * axis[k]; };

	return {
		Fvec4(fun1(0), fun2(0, 1, 2), fun3(0, 2, 1), 0.0f),
		Fvec4(fun3(0, 1, 2), fun1(1), fun2(1, 2, 0), 0.0f),
		Fvec4(fun2(0, 2, 1), fun3(1, 2, 0), fun1(2), 0.0f),
		Fvec4(0, 0, 0, 1)
	};
}

Fmat4 ScaleModel(Fvec2 scale)
{
	return ScaleModel(Fvec3(scale[0], scale[1], 1.0f));
}

Fmat4 ScaleModel(Fvec3 scale)
{
	return {
		Fvec4(scale.x, 0, 0, 0),
		Fvec4(0, scale.y, 0, 0),
		Fvec4(0, 0, scale.z, 0),
		Fvec4(0, 0, 0, 1)
	};
}

Fmat4 LookAtView(Fvec3 position, Fvec3 orientation, Fvec3 up)
{
	Fmat4 res(1.0f);
	Fvec3 f = Normalize(orientation);
	Fvec3 s = Normalize(Cross(f, up));
	Fvec3 u = Cross(s, f);
	res[0][0] = s[0];
	res[1][0] = s[1];
	res[2][0] = s[2];
	res[0][1] = u[0];
	res[1][1] = u[1];
	res[2][1] = u[2];
	res[0][2] = -f[0];
	res[1][2] = -f[1];
	res[2][2] = -f[2];
	res[3][0] = -Dot(s, position);
	res[3][1] = -Dot(u, position);
	res[3][2] = Dot(f, position);
	return res;
}

Fmat4 OrthogonalProjection(float left, float right, float bottom, float top, float near, float far)
{
	float x = right - left,
		y = top - bottom,
		z = far - near;

	return {
		Fvec4(2 / x, 0, 0, 0),
		Fvec4(0, 2 / y, 0, 0),
		Fvec4(0, 0, -2 / z, 0),
		Fvec4(-(right + left) / x, -(top + bottom) / y, -(far + near) / z, 1)
	};
}

Fmat4 OrthogonalProjection(Fvec2 size)
{
	return OrthogonalProjection(
		-size[0] / 2.0f, size[0] / 2.0f, -size[1] / 2.0f, size[1] / 2.0f);
}

Fmat4 PerspectiveProjection(float fovy, float aspect, float near, float far)
{
	Fmat4 res(0.0f);
	float const tanHalfFovy = tan(fovy / 2.0f);
	res[0][0] = 1.0f / (-aspect * tanHalfFovy);
	res[1][1] = 1.0f / (tanHalfFovy);
	res[2][2] = -(far + near) / (far - near);
	res[2][3] = -1.0f;
	res[3][2] = -(2.0f * far * near) / (far - near);
	return res;
}