#ifndef GRAPHICS_TUTORIAL_VEC2D_H
#define GRAPHICS_TUTORIAL_VEC2D_H
#pragma once

#include <cmath>

class Vec2 {
public:
	Vec2& Scale(float factor) {
		x *= factor;
		y *= factor;
		return *this;
	}

	Vec2& Add(const Vec2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	Vec2& Sub(const Vec2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	float Dot(const Vec2& rhs) const
	{
		return (x * rhs.x) + (y * rhs.y);
	}

	float MagnitudeSquared() const
	{
		return x * x + y * y;
	}

	float Magnitude() const
	{
		return sqrt(MagnitudeSquared());
	}

	inline Vec2& operator+(const Vec2& rhs) { Add(rhs); }
	inline Vec2& operator-(const Vec2& rhs) { Sub(rhs); }
	inline Vec2& operator*(float rhs) { Scale(rhs); }

	float x, y;

	static const Vec2 X_AXIS;
	static const Vec2 Y_AXIS;
};

inline Vec2 Scale(float lhs, const Vec2& rhs)
{
	return { rhs.x * lhs, rhs.y * lhs };
}

inline Vec2 Add(const Vec2& lhs, const Vec2& rhs)
{
	return { lhs.x + rhs.x, lhs.y + rhs.y };
}

inline Vec2 Sub(const Vec2& lhs, const Vec2& rhs)
{
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}

inline float Dot(const Vec2& lhs, const Vec2& rhs)
{
	return (lhs.x * rhs.x) + (lhs.y * rhs.y);
}

inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs) { return Add(lhs, rhs); }
inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs) { return Sub(lhs, rhs); }
inline Vec2 operator*(float lhs, const Vec2& rhs) { return Scale(lhs, rhs); }
inline Vec2 operator*(const Vec2& lhs, float rhs) { return Scale(rhs, lhs); }
inline float operator*(const Vec2& lhs, const Vec2& rhs) { return Dot(lhs, rhs); }

#endif // GRAPHICS_TUTORIAL_VEC2D_H
