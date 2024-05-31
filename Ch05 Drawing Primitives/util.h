#ifndef GRAPHICS_TUTORIAL_UTIL_H
#define GRAPHICS_TUTORIAL_UTIL_H
#pragma once

#include <Windows.h>

#include <cassert>
#include <type_traits>
#include <memory>
#include <vector>
#include <ostream>

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

inline std::ostream& operator<<(std::ostream& lhs, const RECT& rhs)
{
	lhs << "{ " << rhs.left << ", " << rhs.top
		<< ", " << rhs.right << ", " << rhs.bottom << " }";
	return lhs;
}

inline std::string StringFromWideString(const std::wstring& wstr) noexcept
{
	std::string out;
	int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	if (size <= 0)
		OutputDebugStringA("Wide string to string conversion failure\n");
	assert(size > 0);
	out.resize(size);
	WideCharToMultiByte(
		CP_UTF8, 
		0, 
		wstr.c_str(),
		(int)wstr.size(),
		out.data(),
		(int)out.size(),
		nullptr,
		nullptr
	);
	return out;
}

inline std::wstring WideStringFromString(const std::string& str) noexcept
{
	std::wstring out;
	int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
	if (size <= 0)
		OutputDebugStringA("String to wide string conversion failure\n");
	assert(size > 0);
	out.resize(size);
	MultiByteToWideChar(
		CP_UTF8, 
		0, 
		str.c_str(), 
		(int)str.size(),
		out.data(), 
		(int)out.size()
	);
	return out;
}

#endif // GRAPHICS_TUTORIAL_UTIL_H
