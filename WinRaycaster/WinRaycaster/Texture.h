#pragma once


#define MipDim(Mip, Dim)	((Dim) >> (Mip))


class Texture {
public:
	Pixel	**pMip;
	int		Wid;
	int		Hei;
	int		Mips;

	Texture() :
		pMip(nullptr), Wid(0), Hei(0), Mips(0) {}

	Texture(const wchar_t *Filename) : Texture() {
		Bitmap *pBmp = Bitmap::FromFile(Filename);
		assert(pBmp);

		BitmapData bd;
		if (pBmp->LockBits(NULL, ImageLockModeRead, PixelFormat32bppARGB, &bd) != Ok) {
			int err = GetLastError();
			return;
		}

		Wid = bd.Width;
		assert(Wid && !(Wid & (Wid - 1)));

		Hei = bd.Height;
		assert(Hei && !(Hei & (Hei - 1)));

		Mips = (int)log2((double)min(Wid, Hei));
		pMip = new Pixel*[Mips + 1];
		assert(pMip);

		for (int mip = 0; mip < Mips + 1; mip++) {
			const int pixels = MipDim(mip, Wid) * MipDim(mip, Hei);
			pMip[mip] = new Pixel[pixels];
			assert(pMip[mip]);
		}

		for (int row = 0; row < Hei; row++)
			memcpy(pMip[0] + row * Wid, (Pixel*)((char*)bd.Scan0 + row * abs(bd.Stride)), Wid * sizeof(Pixel));

		pBmp->UnlockBits(&bd);
		delete pBmp;

		for (int mip = 0; mip < Mips; mip++) {
			const int wid = MipDim(mip, Wid);
			const int hei = MipDim(mip, Hei);

			for (int yy = 0, y = 0; y < hei; yy++, y += 2) {
				Pixel *pSrc = pMip[mip    ] + y  *  wid;
				Pixel *pDst = pMip[mip + 1] + yy * (wid >> 1);

				for (int xx = 0, x = 0; x < wid; xx++, x += 2) {
					Pixel pixTL = pSrc[x      ], pixTR = pSrc[x +       1];
					Pixel pixBL = pSrc[x + wid], pixBR = pSrc[x + wid + 1];

					pDst[xx] = pixTL.Quarter() + pixTR.Quarter() + 
						pixBL.Quarter() + pixBR.Quarter();
				}
			}
		}
	}

	inline Pixel& Sample(const Int2 &Pos, const int mip) const {
		return pMip[mip][Pos.y * MipDim(mip, Wid) + Pos.x];
	}

	Pixel Bilerp(const Vec2 &TexUV, const int mip) const {
		assert(mip < Mips);
		const int mipWid = MipDim(mip, Wid);
		const int mipHei = MipDim(mip, Hei);

		//assert(TexUV.x>=0.0f);

		Vec2 mipPos = TexUV * Vec2(mipWid, mipHei);

		Int2 uvPos(mipPos);

		Vec2 uv = mipPos - uvPos;

		if (uv.x < 0.0f)
			uv.x += 1.0f;

		if (uv.y < 0.0f)
			uv.y += 1.0f;

		uvPos.x &= mipWid - 1;
		uvPos.y &= mipHei - 1;

		Int2 uvOpp = uvPos + Int2(1, 1);
		uvOpp.x &= mipWid - 1;	// Wrap using and instead of modulus. mipWid / mipHei must be a power of 2.
		uvOpp.y &= mipHei - 1;

		const Pixel &tl = Sample(Int2(uvPos.x, uvPos.y), mip);
		const Pixel &tr = Sample(Int2(uvOpp.x, uvPos.y), mip);
		const Pixel &bl = Sample(Int2(uvPos.x, uvOpp.y), mip);
		const Pixel &br = Sample(Int2(uvOpp.x, uvOpp.y), mip);

		const Pixel t = Pixel::Lerp(tl, tr, uv.x);
		const Pixel b = Pixel::Lerp(bl, br, uv.x);

		return Pixel::Lerp(t, b, uv.y);
	}

	Pixel FBilerp(const Vect2 &TexUV, const int mip) const {
		assert(mip < Mips);
		const int mipWid = MipDim(mip, Wid);
		const int mipHei = MipDim(mip, Hei);

		//assert(TexUV.x>=0.0f);

		Vect2 mipPos = TexUV * Vect2(mipWid, mipHei);

		Int2 uvPos(mipPos);

		Vect2 uv = mipPos - uvPos;

		if (uv.x < 0.0f)
			uv.x += 1.0f;

		if (uv.y < 0.0f)
			uv.y += 1.0f;

		uvPos.x &= mipWid - 1;
		uvPos.y &= mipHei - 1;

		Int2 uvOpp = uvPos + Int2(1, 1);
		uvOpp.x &= mipWid - 1;	// Wrap using and instead of modulus. mipWid / mipHei must be a power of 2.
		uvOpp.y &= mipHei - 1;

		const Pixel &tl = Sample(Int2(uvPos.x, uvPos.y), mip);
		const Pixel &tr = Sample(Int2(uvOpp.x, uvPos.y), mip);
		const Pixel &bl = Sample(Int2(uvPos.x, uvOpp.y), mip);
		const Pixel &br = Sample(Int2(uvOpp.x, uvOpp.y), mip);

		const Pixel t = Pixel::FLerp(tl, tr, uv.x);
		const Pixel b = Pixel::FLerp(bl, br, uv.x);

		return Pixel::FLerp(t, b, uv.y);
	}
};
