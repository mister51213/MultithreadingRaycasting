#pragma once


//#define SHOWDEBUG	// Show trace debug

#define BILERP		// Enable bilinear filtering
#define TRILERP		// Enable trilinear filtering (requires BILERP)
#define DARKEN		// Enable depth-based darkening

#define SCRN_W	1920
#define SCRN_H	1080

//#define SCRN_W	1280
//#define SCRN_H	720

//#define SCRN_W	960
//#define SCRN_H	540

#define MIP_BIAS	(256.f/(float)SCRN_H*1.1f)

#define THREADCOUNT	8
#define FLOATTYPE	float //FixedX

typedef FixedFloat<20, long, long long> FixedX;

//typedef FixedFloat<8,  long, long long> Fixed8;
//typedef FixedFloat<16, long, long long> Fixed16;
//typedef FixedFloat<24, long, long long> Fixed24;

typedef TempVect2D<float>		Vec2;
typedef TempVect2D<int>			Int2;
typedef TempVect2D<FLOATTYPE>	Vect;
