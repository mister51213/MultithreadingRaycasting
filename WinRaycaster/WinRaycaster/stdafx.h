// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//#define ULONG_PTR ULONG

// Windows Header Files:
#include <windows.h>
#pragma comment(lib, "Winmm.lib")

// GDI+ for debug
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")


// Standard headers
#include <malloc.h>
#include <memory.h>
#include <process.h>
#include <stdlib.h>
#include <tchar.h>

#include <cassert>
#include <vector>

// Direct3D
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

#pragma comment(lib, "dxgi.lib") 
#pragma comment(lib, "d3d11.lib")


using namespace std;

typedef unsigned __int64	uint64;
typedef signed __int64		int64;
typedef unsigned __int32	uint32;
typedef signed __int32		int32;


// Application headers
#include "RayMath.h"
#include "Config.h"
#include "Pixel.h"
#include "Texture.h"
#include "Thread.h"
#include "QuadTree.h"
#include "Camera.h"
#include "Map.h"
#include "DXHelper.h"
#include "Game.h"