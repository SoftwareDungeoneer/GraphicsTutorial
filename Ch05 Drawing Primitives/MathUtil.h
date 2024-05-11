#ifndef GRAPHICS_TUTORIAL_MATH_UTIL_H
#define GRAPHICS_TUTORIAL_MATH_UTIL_H
#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

union Float_t
{
	Float_t(float _f) :f(_f) {}

	int32_t i;
	uint32_t u;
	float f;
};

template <typename T> 
inline T sgn(T x)
{
	constexpr T zed{ 0 };
	if (x > zed) return 1;
	if (x < zed) return -1;
	return zed;
}

#undef max
#undef min

inline bool flt_cmp_rel(float a, float b, float maxRelDiff = std::numeric_limits<float>::epsilon())
{
	float diff = fabs(a - b);
	a = fabs(a);
	b = fabs(b);
	auto largest = std::max(a, b);
	return (diff <= (largest * maxRelDiff));
}

inline bool flt_cmp_abs(float a, float b, float maxDiff = std::numeric_limits<float>::epsilon())
{
	return fabs(a - b) <= maxDiff;
}

inline unsigned flt_dist_ulp(float a, float b)
{
	if (a == b)
		return 0;
	Float_t uA(std::max(a, b)), uB(std::min(a, b));	
	return uA.u - uB.u;
}

#endif // GRAPHICS_TUTORIAL_MATH_UTIL_H
