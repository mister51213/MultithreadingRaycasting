#pragma once


class Pixel {
protected:
	static unsigned ScaleBy(const unsigned A, const unsigned Mul) {
		return (A * Mul) >> 8;
	}

	static unsigned LerpBy(const unsigned A, const unsigned B, const unsigned Alpha) {
		// C = A * alpha + (1 - Alpha) * B
		// C = A + (B - A) * Alpha  << Faster
		return A + (((B - A) * Alpha) >> 8);
	}

	static FLOATTYPE FLerpBy(const FLOATTYPE &A, const FLOATTYPE &B, const FLOATTYPE &Alpha) {
		return A + (B - A) * Alpha;
	}

public:
	unsigned argb;

	Pixel() {}

	Pixel(const unsigned argb) :
		argb(argb) {}

	Pixel(const unsigned A, const unsigned R, const unsigned G, const unsigned B) :
		argb((A << 24) | (R << 16) | (G << 8) | B) {}

	inline unsigned Alpha() const {
		return argb >> 24;
	}

	inline unsigned Red() const {
		return (argb >> 16) & 0x0FFUL;
	}

	inline unsigned Green() const {
		return (argb >> 8) & 0x0FFUL;
	}

	inline unsigned Blue() const {
		return argb & 0x0FFUL;
	}

	inline Pixel Quarter() const {
		return (argb & 0xFCFCFCFC) >> 2;
	}

	inline Pixel Scale(const int Mul) const {
		return Pixel(255, ScaleBy(Red(), Mul), ScaleBy(Green(), Mul), ScaleBy(Blue(), Mul));
	}

	inline Pixel operator+ (const Pixel &Other) const {
		return argb + Other.argb;
	}

	inline static Pixel Lerp(const Pixel &A, const Pixel &B, const float Alpha) {
		const unsigned alpha = (unsigned)(Alpha * 256.0f);
		return Pixel(255, 
					 LerpBy(A.Red(),   B.Red(),   alpha),
					 LerpBy(A.Green(), B.Green(), alpha), 
					 LerpBy(A.Blue(),  B.Blue(),  alpha));
	}

	inline static Pixel FLerp(const Pixel &A, const Pixel &B, const FLOATTYPE Alpha) {
		return Pixel(255, 
					 FLerpBy(A.Red(),   B.Red(),   Alpha),
					 FLerpBy(A.Green(), B.Green(), Alpha), 
					 FLerpBy(A.Blue(),  B.Blue(),  Alpha));
	}
};
