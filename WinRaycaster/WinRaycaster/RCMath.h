#pragma once

#define _USE_MATH_DEFINES
#define CIRCLE 0
//#define RECT 1
#define PLAYER_RADIUS 50.f
#define MIN_FACING_DP 0.70710678118f // cos(45 * PI / 180)

#define TAU		(6.283185307179586476925286766559f)
#define PI		(TAU / 2.0f)

#define Ang2Rad	(TAU / 360.0f)


struct Rotator {
	float radians;
	float notches;

	static constexpr float notchestoRadians = TAU / 1024;
	static constexpr float radiansToNotches = 1024 / TAU;

	Rotator(const int inNotches);

	struct Vec2 ToVector();
};

struct Vec2 {
	float x, y;

	Vec2();
	Vec2(const Vec2& other);
	Vec2(const float inX, const float inY);

	float SizeSquared() const;
	float Size() const;
	void Normalize();
	float Dot(Vec2& other) const;
	float Cross(const Vec2& other) const;
	bool IsRightOf(const Vec2& B) const;
	bool IsLeftOf(const Vec2& B) const;
	bool IsWithin(const Vec2& leftBound, const Vec2& rightBound) const;
	void Rotate90Degrees();

	Vec2 operator - (const Vec2& other) const;
	void operator -= (const Vec2& other);
	Vec2 operator + (const Vec2& other) const;
	void operator += (const Vec2& other);
	Vec2 operator * (const float scalar) const;
	void operator *= (const float scalar);
};