#ifndef PIXMESH_UTIL_H
#define PIXMESH_UTIL_H
#pragma once

#include <Windows.h>
#include <intrin.h>

#define PIXMESH_ENABLE_ASSERT
#define PIXMESH_ENABLE_LOG

#include "PixMeshLog.h"

namespace PixMesh
{
	inline void DebugPrint(char* buffer) { OutputDebugStringA(buffer); }
	inline void Break() { DebugBreak(); }
	inline void RaiseError(const char* condition, const char* filename, const unsigned line, const char* fmt, ...)
	{
		pmLog("[ASSERT] %s failed in %s at line %d:\n", condition, filename, line);
		va_list args;
		va_start(args, fmt);
		pmLogL(fmt, args);
		//Break();
		va_end(args);
	}
	
	inline void SystemError(DWORD error, const char* filename, const unsigned line, const char* fmt, ...)
	{
		pmLog("[ERROR] in %s at line %d:\n", filename, line);
		va_list args;
		va_start(args, fmt);
		pmLogL(fmt, args);
		va_end(args);
		DWORD fmtFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		DWORD lang = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
		TCHAR buffer[4096] { 0 };
		FormatMessage(fmtFlags, NULL, error, lang, buffer, 4096, NULL);
		pmLog("\t%S", buffer);
		//Break();
	}
}

#ifdef PIXMESH_ENABLE_ASSERT
#define phAssert( cond )           do { if (!(cond))              { PixMesh::RaiseError(  #cond, __FILE__, __LINE__, "" );          __debugbreak(); }} while (false)
#define phAssertMsg( cond, ... )   do { if (!(cond))              { PixMesh::RaiseError(  #cond, __FILE__, __LINE__, __VA_ARGS__ ); __debugbreak(); }} while (false)
#define phAssertCom( result, ... ) do { if (!(SUCCEEDED(result))) { PixMesh::SystemError(result, __FILE__, __LINE__, __VA_ARGS__ ); __debugbreak(); }} while (false)
#else
#define pmAssert(...)
#define pmAssertMsg(...)
#endif // PIXMESH_ASSERT_ENABLE

#define pmError( ... ) do { PixMesh::RaiseError( "Error", __FILE__, __LINE__, __VA_ARGS__ ); __debugbreak(); } while (false)
#define pmVerify( cond ) do { if (!(cond)) PixMesh::RaiseError( #cond, __FILE__, __LINE__, "" ); __debugbreak(); } while (false)
#define pmVerifyMsg( cond, ... ) do { if (!(cond)) PixMesh::RaiseError( #cond, __FILE__, __LINE__, __VA_ARGS__ ); __debugbreak(); } while (false)

#define pmUnused( x ) x

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define EMIT(x) message(__FILE__"("STRINGIFY(__LINE__)"): " x)
#define STATIC_TODO(desc) EMIT("warning: todo: " desc)
#define STATIC_WARNING(desc) EMIT("warning: " desc)
#define STATIC_ERROR(desc) EMIT("error: " desc)

#define SAFE_RELEASE(x) if ((x)) { x->Release(); x = 0; }

#define CHECK_FLAG(value, flag) ((value) & (flag))
#define SET_FLAG(value, flag) ((value) |= (flag))
#define UNSET_FLAG(value, flag) ((value) &= ~(flag))
#define MODIFY_FLAG_FROM_BOOL(value, flag, c) (value) = ((c) ? ((value) | (flag)) : ((value) & ~(flag)))

#endif // PIXMESH_UTIL_H
