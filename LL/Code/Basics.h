#pragma once

#include "GameCore.h"
#include "GraphicsCommon.h"
#include "ColorBuffer.h"
#include "Display.h"

// Typedef common primitive data types
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int32_t     b32;

typedef float       f32;
typedef double      f64;


// Helper macros
#define KILOBYTE(X)         (X * 1024UL)
#define MEGABYTE(X)         (X * KILOBYTE(1024UL))
#define GIGABYTE(X)         (X * MEGABYTE(1024UL))
#define ARRAY_COUNT(Arr)    (sizeof(Arr)/sizeof(Arr[0]))
#define OFFSET_OF(T, M)     offsetof(T, M)

// MiniEngine specific forward decls
namespace GameCore { extern HWND g_hWnd; }
namespace Graphics
{ 
    extern IDXGISwapChain1 *s_SwapChain1;
    extern uint32_t g_NativeWidth;
    extern uint32_t g_NativeHeight;
}