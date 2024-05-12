#ifndef GRAPHICS_TUTORIAL_MTX2X2_H
#define GRAPHICS_TUTORIAL_MTX2X2_H
#pragma once

#include "Vec2.h"

#include <cassert>
#include <cmath>
#include <limits>

#include "util.h"

class Mtx2x2
{
public:
	Mtx2x2() {}
	Mtx2x2(const Vec2& a, const Vec2& b) :elements{ a.x, a.y, b.x, b.y } {}
	Mtx2x2(
		float _00, float _01,
		float _10, float _11):
	elements{ _00, _10, _01, _11 }
	{
	}

	Mtx2x2& Scale(const float s) {
		elements[0][0] *= s;
		elements[0][1] *= s;
		elements[1][0] *= s;
		elements[1][1] *= s;
		return *this;
	}

	Mtx2x2& Add(const Mtx2x2& rhs) {
		elements[0][0] += rhs.elements[0][0];
		elements[0][1] += rhs.elements[0][1];
		elements[1][0] += rhs.elements[1][0];
		elements[1][1] += rhs.elements[1][1];
		return *this;
	}

	Mtx2x2& Sub(const Mtx2x2& rhs)
	{
		elements[0][0] -= rhs.elements[0][0];
		elements[0][1] -= rhs.elements[0][1];
		elements[1][0] -= rhs.elements[1][0];
		elements[1][1] -= rhs.elements[1][1];
		return *this;
	}

	Mtx2x2& Normalize()
	{
		auto m = sqrt(elements[0][0] * elements[0][0] + elements[0][1] * elements[0][1]);
		elements[0][0] /= m;
		elements[0][1] /= m;
		m = sqrt(elements[1][0] * elements[1][0] + elements[1][1] * elements[1][1]);
		elements[1][0] /= m;
		elements[1][1] /= m;
		return *this;
	}

	Mtx2x2& Transpose() {
		float tmp = elements[0][1];
		elements[0][1] = elements[1][0];
		elements[1][0] = tmp;
		return *this;
	}

	float Det() const {
		return (elements[0][0] * elements[1][1]) - (elements[0][1] * elements[1][0]);
	}

	Mtx2x2 Inverse() const {
		auto det = Det();
		if (flt_cmp_rel(0, det))
			return Mtx2x2::ZERO;

		return Mtx2x2{ 
			elements[1][1], -elements[1][0],
			-elements[0][1], elements[0][0]
		}.Scale(1.f / Det());
	}

	static Mtx2x2 Rotation(float theta);

	Vec2 X() { return { elements[0][0], elements[0][1] }; }
	Vec2 Y() { return { elements[1][0], elements[1][1] }; }

	inline Vec2 operator()(unsigned col) const { 
		assert(col < 2);
		return { 
			elements[col][0],
			elements[col][1]
		}; 
	}

	inline float operator()(unsigned row, unsigned col) const {
		assert(row < 2 && col < 2);
		return elements[col][row];
	}

	inline float& operator()(unsigned row, unsigned col) {
		assert(row < 2 && col < 2);
		return elements[col][row];
	}

	inline Mtx2x2& operator+=(const Mtx2x2& rhs) { return Add(rhs); }
	inline Mtx2x2& operator-=(const Mtx2x2& rhs) { return Sub(rhs); }
	inline Mtx2x2& operator*=(const float f) { return Scale(f); }

	union {
		float elements[2][2];
		float floats[4];
	};

	static const Mtx2x2 Identity;
	static const Mtx2x2 ZERO;
};

inline float Det(const Mtx2x2& m)
{
	return m.Det();
}

inline Mtx2x2 Scale(const float lhs, const Mtx2x2& rhs) { return Mtx2x2{ rhs }.Scale(lhs); }
inline Mtx2x2 Add(const Mtx2x2& lhs, const Mtx2x2& rhs) { return Mtx2x2{ lhs }.Add(rhs); }
inline Mtx2x2 Sub(const Mtx2x2& lhs, const Mtx2x2& rhs) { return Mtx2x2{ lhs }.Sub(rhs); }

inline Mtx2x2 Mul(const Mtx2x2& lhs, const Mtx2x2& rhs)
{
	return {
		lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0),
		lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1),
		lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0),
		lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1)
	};
}

inline Vec2 Mul(const Mtx2x2& lhs, const Vec2& rhs)
{
	return {
		lhs.elements[0][0] * rhs.x + lhs.elements[1][0] * rhs.y,
		lhs.elements[0][1] * rhs.x + lhs.elements[1][1] * rhs.y
	};
}

inline Mtx2x2 Normalize(const Mtx2x2& mtx)
{
	return Mtx2x2{ mtx }.Normalize();
}

inline Mtx2x2 Transpose(const Mtx2x2& mtx)
{
	return {
		mtx.elements[0][0], mtx.elements[0][1],
		mtx.elements[1][0], mtx.elements[1][1]
	};
}

inline Mtx2x2 Mtx2x2::Rotation(float theta)
{
	float c = cos(theta);
	float s = sin(theta);
	return {
		c, -s,
		s,  c
	};
}

inline Mtx2x2 ComponentMultiply(const Mtx2x2& lhs, const Mtx2x2& rhs)
{
	return {
		lhs.elements[0][0] * rhs.elements[0][0],
		lhs.elements[1][0] * rhs.elements[1][0],
		lhs.elements[0][1] * rhs.elements[0][1],
		lhs.elements[1][1] * rhs.elements[1][1]
	};
}

inline Mtx2x2 operator+(const Mtx2x2& lhs, const Mtx2x2& rhs)
{
	return Add(lhs, rhs);
}

inline Mtx2x2 operator-(const Mtx2x2& lhs, const Mtx2x2& rhs)
{
	return Sub(lhs, rhs);
}

inline Mtx2x2 operator*(const float lhs, const Mtx2x2& rhs)
{
	return Scale(lhs, rhs);
}

inline Mtx2x2 operator*(const Mtx2x2& lhs, const float rhs)
{
	return Scale(rhs, lhs);
}

inline Mtx2x2 operator*(const Mtx2x2& lhs, const Mtx2x2& rhs)
{
	return Mul(lhs, rhs);
}

inline Vec2 operator*(const Mtx2x2& lhs, const Vec2& rhs)
{
	return Mul(lhs, rhs);
}

inline bool operator==(const Mtx2x2& lhs, const Mtx2x2& rhs)
{
	bool b{ true };
	for (unsigned n{ 0 }; n < countof(Mtx2x2::floats); ++n)
		b = b && flt_cmp_rel(lhs.floats[n], rhs.floats[n]);
	return b;
}

#endif // GRAPHICS_TUTORIAL_MTX2X2_H
