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

#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib, "gdiplus.lib")

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <cassert>
#include <vector>

using namespace std;

// TODO: reference additional headers your program requires here
#include "RCMath.h"
#include "World.h"
