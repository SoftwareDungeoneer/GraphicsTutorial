#ifndef GRAPHICS_TUTORIAL_UTIL_H
#define GRAPHICS_TUTORIAL_UTIL_H
#pragma once

#include <Windows.h>

#include <type_traits>
#include <memory>
#include <vector>

#define countof(arr) (sizeof(arr) / sizeof(arr[0]))

template <typename T, unsigned N>
struct __declspec(align(N)) Aligned
{
	T t;
};

template <typename T, unsigned N> constexpr unsigned AlignedSize_v{ sizeof(Aligned<T, N>) };
template <typename T> constexpr unsigned aligned_size_16 = AlignedSize_v<T, 16>;

template <typename T>
void ZeroInitialize(T& t) noexcept {
	static_assert(std::is_standard_layout_v<T>, "ZeroInitialize can not be used on non-standard-layout types");
	ZeroMemory(&t, sizeof(T));
}

std::vector<BYTE> LoadFile(LPCTSTR filename);

struct RECTF {
	float left, top, right, bottom;
};

inline RECTF RectfFromRect(const RECT& r)
{
	return { 
		1.f * r.left,
		1.f * r.top,
		1.f * r.right,
		1.f * r.bottom 
	};
}

#endif // GRAPHICS_TUTORIAL_UTIL_H
