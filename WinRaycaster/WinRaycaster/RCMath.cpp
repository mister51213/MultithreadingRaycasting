#include "stdafx.h"

/*
// ROTATOR STRUCT
constexpr float Rotator::notchestoRadians;
constexpr float Rotator::radiansToNotches;

Rotator::Rotator(const int inNotches) :
	notches((float)inNotches) {
	radians = notches * notchestoRadians;
}

Vec2 Rotator::ToVector() {
	float xComp = cos(radians);
	float yComp = sin(radians);
	Vec2 result(xComp, yComp);
	return result;
}

// VEC2 STRUCT
Vec2::Vec2() :
	x(0.f),
	y(0.f)
{}

Vec2::Vec2(const Vec2& other) {
	x = other.x;
	y = other.y;
}

Vec2::Vec2(const float inX, const float inY) :
	x(inX),
	y(inY)
{}

float Vec2::SizeSquared() const {
	return x * x + y * y;
}

float Vec2::Size() const {
	return sqrt(SizeSquared());
}

void Vec2::Normalize() {
	const float magnitude = sqrt(x * x + y * y);
	const float coefficient = 1.f / magnitude;
	x *= coefficient;
	y *= coefficient;
}

float Vec2::Dot(Vec2& other) const {
	return x * other.x + y * other.y;
}

// if B is to the left of this vector, result will be positive
// if B is to the right of this vector, result will be negative
float Vec2::Cross(const Vec2& B) const {
	return x * B.y - y * B.x;
}

bool Vec2::IsRightOf(const Vec2& B) const {
	return Cross(B) > 0;
}

bool Vec2::IsLeftOf(const Vec2& B) const {
	return Cross(B) < 0;
}

bool Vec2::IsWithin(const Vec2& leftBound, const Vec2& rightBound) const {
	return Cross(Vec2(leftBound)) >= 0 && Cross(Vec2(rightBound)) <= 0;
}

void Vec2::Rotate90Degrees() {
	float newX = -y;
	y = x;
	x = newX;
}

// VEC2 OPERATORS
Vec2 Vec2::operator - (const Vec2& other) const {
	return Vec2(x - other.x, y - other.y);
}

void Vec2::operator -= (const Vec2& other) {
	x -= other.x;
	y -= other.y;
}

Vec2 Vec2::operator + (const Vec2& other) const {
	return Vec2(x + other.x, y + other.y);
}

void Vec2::operator += (const Vec2& other) {
	x += other.x;
	y += other.y;
}

Vec2 Vec2::operator * (const float scalar) const {
	return Vec2(x * scalar, y * scalar);
}

void Vec2::operator *= (const float scalar) {
	x *= scalar;
	y *= scalar;
}*/