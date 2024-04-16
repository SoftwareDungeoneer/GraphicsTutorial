#ifndef GRAPHICS_TUTORIAL_UTIL_H
#define GRAPHICS_TUTORIAL_UTIL_H
#pragma once

#include <Windows.h>

#define countof(arr) (sizeof(arr) / sizeof(arr[0]))

template <typename T>
void ZeroInitialize(T& t) noexcept {
	static_assert(std::is_standard_layout_v<T>, "ZeroInitialize can not be used on non-standard-layout types");
	ZeroMemory(&t, sizeof(T));
}


#endif // GRAPHICS_TUTORIAL_UTIL_H
