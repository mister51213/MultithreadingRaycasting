#pragma once

#define _USE_MATH_DEFINES
#define CIRCLE 0
//#define RECT 1
#define PLAYER_RADIUS 50.f
#define MIN_FACING_DP 0.70710678118f // cos(45 * PI / 180)

#define TAU		6.283185307179586476925286766559f
#define PI		(TAU / 2.0f)
#define QTAU	(TAU / 4.0f)
#define ETAU	(TAU / 8.0f)

#define Ang2Rad	(TAU / 360.0f)

/*
struct Rotator {
	float radians;
	float notches;

	static constexpr float notchestoRadians = TAU / 1024;
	static constexpr float radiansToNotches = 1024 / TAU;

	Rotator(const int inNotches);

	struct Vec2 ToVector();
};*/



template <int FracBits, typename IntType, typename LongType> class FixedFloat {
protected:
	static FixedFloat Construct(IntType RawValue) {
		FixedFloat fixed;
		fixed.Value = RawValue;
		return fixed;
	}

	static const IntType FracMask = (1 << FracBits) - 1;

	static float FracFlt() {
		return 1.0f / (float)(1 << FracBits);
	}

	static double FracDbl() {
		return 1.0 / (double)(1 << FracBits);
	}

public:
	IntType	Value;

	FixedFloat() {}

	FixedFloat(const FixedFloat &Other) :
		Value(Other.Value) {}

	template <typename OtherType> FixedFloat(const OtherType Value) :
		Value((IntType)(Value * (OtherType)(1 << FracBits))) {}

	template <int OtherFracBits> FixedFloat(const FixedFloat<OtherFracBits, IntType, LongType> &Value) :
		Value((OtherFracBits > FracBits) ? (Value.Value >> (OtherFracBits - FracBits)) : (Value.Value << (FracBits - OtherFracBits))) {}

	inline FixedFloat& operator= (const FixedFloat &Other) {
		Value = Other.Value;
		return *this;
	}

	inline FixedFloat operator+ (const FixedFloat &Other) const {
		return Construct(Value + Other.Value);
	}

	inline FixedFloat operator- (const FixedFloat &Other) const {
		return Construct(Value - Other.Value);
	}

	inline FixedFloat operator* (const FixedFloat &Other) const {
		//return Construct((Value * Other.Value) >> FracBits);
		//return Construct(Value * (Other.Value >> FracBits) + (Value * (Other.Value & FracMask)));
		return Construct((IntType)(((LongType)Value * (LongType)Other.Value) >> FracBits));
	}

	inline FixedFloat operator/ (const FixedFloat &Other) const {
		//return Construct((Value << FracBits) / Other.Value);
		return Construct((IntType)(((LongType)Value << FracBits) / (LongType)Other.Value));
	}

	inline FixedFloat& operator+= (const FixedFloat &Other) const {
		*this = *this + Other;
		return *this;
	}

	inline FixedFloat& operator-= (const FixedFloat &Other) const {
		*this = *this - Other;
		return *this;
	}

	inline FixedFloat& operator*= (const FixedFloat &Other) const {
		*this = *this * Other;
		return *this;
	}

	inline FixedFloat& operator/= (const FixedFloat &Other) const {
		*this = *this / Other;
		return *this;
	}

	inline double AsInteger() const {
		return Value >> FracBits;
	}

	inline float AsFloat() const {
		return (float)(Value >> FracBits) + (float)(Value & FracMask) * FracFlt();
	}

	inline double AsDouble() const {
		return (double)(Value >> FracBits) + (double)(Value & FracMask) * FracDbl();
	}
};

typedef FixedFloat<8, long, long long> Fixed8;
typedef FixedFloat<16, long, long long> Fixed16;


template <typename Type> struct TempVect2D {
	Type x, y;

	TempVect2D() :
		x((Type)0), y((Type)0) {}

	template <typename OtherType> TempVect2D(const OtherType& other) :
		x((Type)other.x), y((Type)other.y) {}

	template <typename TypeX, typename TypeY> TempVect2D(const TypeX x, const TypeY y) :
		x((Type)x), y((Type)y) {}

	Type SizeSquared() const {
		return x * x + y * y;
	}

	Type Size() const {
		return sqrt(SizeSquared());
	}

	void Normalize() {
		const Type magnitude = sqrt(x * x + y * y);
		const Type coefficient = 1.f / magnitude;
		x *= coefficient;
		y *= coefficient;
	}

	Type Dot(TempVect2D& other) const {
		return x * other.x + y * other.y;
	}

	// if B is to the left of this vector, result will be positive
	// if B is to the right of this vector, result will be negative
	Type Cross(const TempVect2D& B) const {
		return x * B.y - y * B.x;
	}

	bool IsRightOf(const TempVect2D& B) const {
		return Cross(B) > 0;
	}

	bool IsLeftOf(const TempVect2D& B) const {
		return Cross(B) < 0;
	}

	bool IsWithin(const TempVect2D& leftBound, const TempVect2D& rightBound) const {
		return Cross(TempVect2D(leftBound)) >= 0 && Cross(TempVect2D(rightBound)) <= 0;
	}

	void Rotate90Degrees() {
		Type newX = -y;
		y = x;
		x = newX;
	}

	// TempVect2D OPERATORS
	TempVect2D operator - (const TempVect2D& other) const {
		return TempVect2D(x - other.x, y - other.y);
	}

	void operator -= (const TempVect2D& other) {
		x -= other.x;
		y -= other.y;
	}

	TempVect2D operator + (const TempVect2D& other) const {
		return TempVect2D(x + other.x, y + other.y);
	}

	void operator += (const TempVect2D& other) {
		x += other.x;
		y += other.y;
	}

	TempVect2D operator * (const TempVect2D &other) const {
		return TempVect2D(x * other.x, y * other.y);
	}

	TempVect2D operator * (const Type scalar) const {
		return TempVect2D(x * scalar, y * scalar);
	}

	void operator *= (const Type scalar) {
		x *= scalar;
		y *= scalar;
	}
};

typedef TempVect2D<float>	Vec2;
typedef TempVect2D<int>		Int2;
typedef TempVect2D<Fixed16>	Fixed2;
