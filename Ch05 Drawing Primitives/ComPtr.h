#ifndef COM_PTR_H
#define COM_PTR_H
#pragma once

#include <typeinfo>
#include <type_traits>
#include <utility>


//#define ENABLE_COM_PTR_LOGGING
//#define COM_PTR_LOG_IF_T ID3D11SamplerState

#ifdef ENABLE_COM_PTR_LOGGING
#include <cstdio>
#include <Windows.h>
namespace
{
	inline void DebugPrint(char* buffer) { OutputDebugStringA(buffer); }

	inline void Log(const char* fmt, ...)
	{
		va_list args, argcopy;
		va_start(args, fmt);
		va_copy(argcopy, args);
		size_t len = (size_t)_vscprintf(fmt, argcopy) + 1;
		char* buffer = new char[len];
		vsnprintf_s(buffer, len, len - 1, fmt, args);

		DebugPrint(buffer);
		delete[] buffer;

		va_end(argcopy);
		va_end(args);
	}
}
#ifndef COM_PTR_LOG_IF_T
#define LOG(...) Log(__VA_ARGS__)
#else // COM_PTR_LOG_IF_T
#define LOG(...) Log(__VA_ARGS__)
#endif // COM_PTR_LOG_IF_T
#else
#define LOG(...)
#endif // ENABLE_COM_PTR_LOGGING

#define TYPENAME(x) typeid(x).name()

template <class T>
class ComPtr
{
public:
	ComPtr() :pointer{ 0 } {
		LOG("[ComPtr] <%s> Default Init\n", TYPENAME(T));
	}
	explicit ComPtr(T* inptr) : pointer{ inptr } {
		LOG("[ComPtr] <%s> ComPtr(T* = 0x%p), AddRef called\n",
			TYPENAME(T), inptr
		);
		AddRef();
	}
	//explicit ComPtr(T& inobj): pointer{&inobj} { 
	//	LOG("[ComPtr] <%s> ComPtr(T& = 0x%p) - NO REF\n",
	//		TYPENAME(T), pointer
	//	);
	//	/* NO ADDREF */
	//}
	ComPtr(const ComPtr& rhs) : pointer{ rhs.pointer } {
		LOG("[ComPtr] <%s> ComPtr(const ComPtr& = 0x%p), AddRef called\n",
			TYPENAME(T)
		);
		AddRef();
	}
	ComPtr(ComPtr&& rhs) noexcept : pointer{ 0 } {
		LOG("[ComPtr] <%s> ComPtr(ComPtr&&), swaping\n", TYPENAME(T));
		swap(rhs);
	}
	~ComPtr() {
		LOG("[ComPtr] <%s> Destructor, releasing 0x%p\n",
			TYPENAME(T), pointer
		);
		Release();
	}

	template <
		typename U,
		typename = std::enable_if_t<std::is_convertible<T*, U*>::value>
	>
	operator ComPtr<U>()
	{
		LOG("[ComPtr] <%s> Conversion to base <%s> ptr\n",
			TYPENAME(T), T_STRING(U)
		);
		return ComPtr<U>(pointer);
	}

	template <typename U>
	ComPtr<U> QueryInterface()
	{
		ComPtr<U> pu;
		if (pointer)
			pointer->QueryInterface(__uuidof(U), (void**)&*pu);
		LOG("[ComPtr] <%s> QueryInterface<%s>, returning interface 0x%p\n",
			TYPENAME(T), TYPENAME(U), pu.pointer
		);
		return pu;
	}

	ComPtr& operator=(const ComPtr& rhs) {
		// string literal concat around newline
		LOG("[ComPtr] <%s> Copy assign, releasing 0x%p, new value = 0x%p"
			" - AddRef\n", TYPENAME(T), pointer, rhs.pointer
		);
		Release();
		pointer = rhs.pointer;
		AddRef();
		return *this;
	}
	ComPtr& operator=(ComPtr&& rhs) noexcept {
		LOG("[ComPtr] <%s> Move assign, swapping\n", TYPENAME(T));
		swap(rhs);
		return *this;
	}
	ComPtr& operator=(T* inptr) {
		LOG("[ComPtr] <%s> operator=(T* = 0x%p), Releasing 0x%p\n",
			TYPENAME(T), inptr, pointer
		);
		Release();
		pointer = inptr;
		AddRef();
		return *this;
	}
	ComPtr& operator=(T& inobj) {
		// string literal concat around newline
		LOG("[ComPtr] <%s> operator=(T& = 0x%p), Releasing 0x%p,"
			" NO ADDREF\n", TYPENAME(T), &inobj, pointer
		);
		Release();
		pointer = &inobj;
		/* NO ADDREF */
		return *this;
	}

	void AddRef() {
		LOG("[ComPtr] <%s> AddRef, 0x%p\n", TYPENAME(T), pointer);
		if (pointer) pointer->AddRef();
	}
	void Release() {
		LOG("[ComPtr] <%s> Release, 0x%p\n", TYPENAME(T), pointer);
		if (pointer)
			pointer->Release();
		pointer = 0;
	}

	void swap(ComPtr<T>& rhs)
	{
		using std::swap;
		swap(pointer, rhs.pointer);
	}

	//void** ForFactory() { return (void**)&pointer; }

	T*& operator*() { return pointer; }
	T* operator->() { return pointer; }
	const T*& operator*() const { return pointer; }
	const T* operator->() const { return pointer; }

	//operator T**() { return &pointer; }
	//operator void**() { return (void**)&pointer; }

	operator bool() const { return !!pointer; }

	bool operator==(ComPtr& rhs) { return pointer == rhs.pointer; }

private:
#ifdef COM_PTR_LOG_IF_T
	template <class... Args> void Log(...) {}

	template <typename = std::enable_if<
		std::is_same_v<typename T, COM_PTR_LOG_IF_T>
	>::type, class... Args>
	void Log(Args... args) { qnLog((args)...); }
#endif // COM_PTR_LOG_IF_T
	T* pointer;
};

template <class T>
void swap(ComPtr<T>& lhs, ComPtr<T>& rhs)
{
	lhs.swap(rhs);
}

#endif // COM_PTR_H

