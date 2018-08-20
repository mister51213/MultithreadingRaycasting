#pragma once


__declspec(selectany) float DebugX, DebugY, DebugScl;

#define Plot(PlotX, PlotY, Rad, color) {									\
	if (pGfx) {																\
		Vec2 OppPos = Vec2(cellPos) + Vec2(PlotX, PlotY);					\
		int DstX = (int)(DebugX + OppPos.x * DebugScl);						\
		int DstY = (int)(DebugY + OppPos.y * DebugScl);						\
		pGfx->DrawEllipse(&Pen(color), DstX-Rad, DstY-Rad, Rad*2, Rad*2);	\
	}																		\
}


class Pixel {
protected:
	static unsigned ScaleBy(const unsigned A, const unsigned Mul) {
		return (A * Mul) >> 8;
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
};


class Camera {
public:
	Vec2	Pos;
	float	Dir;
	float	FoV;

	int		Wid;
	int		Hei;

	Pixel	*pImage;
	Bitmap	*pBmp;

	Camera(const int Wid, const int Hei) :
		Wid(Wid), Hei(Hei) {

		pImage = new Pixel[Wid * Hei];
		assert(pImage);

		pBmp = new Bitmap(Wid, Hei, PixelFormat32bppARGB);
		assert(pBmp);
	}

	void Clear() {
		assert(pImage);
		//memset(pImage, 0, Wid * Hei * sizeof(Pixel));
		for (int i = 0; i < Wid * Hei; pImage[i++] = 0xFF000000);
	}

	void Display(Graphics *pGfx) {
		assert(pImage && pBmp && pGfx);
		
		BitmapData bd;
		if (pBmp->LockBits(NULL, ImageLockModeWrite, PixelFormat32bppARGB, &bd) != Ok)
			return;
		
		assert(bd.Width == Wid && bd.Height == Hei);
		
		const int stride = abs(bd.Stride) / sizeof(Pixel);

		for (int row = 0; row < Hei; row++) {
			memcpy((Pixel*)bd.Scan0 + row * stride, pImage + row * Wid, Wid * sizeof(Pixel));
		}

		pBmp->UnlockBits(&bd);
		pGfx->DrawImage(pBmp, 0, 0);
		//pGfx->DrawImage(pBmp, 0, 0, Wid * 2, Hei * 2);
	}
};


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
};


class Cell {
public:
	Texture *pWallTex;	// Wall is solid if assigned, Hollow if not

	Cell() :
		pWallTex(nullptr) {}

	inline bool IsSolid() const {
		return pWallTex ? true : false;
	}
};


class Map {
protected:
	struct TraceHit {
		Cell	*pCell;
		float	TexU;
		Vec2	Pos;

		TraceHit(Cell *pCell, const float TexU, const Vec2 &Pos) :
			pCell(pCell), TexU(TexU), Pos(Pos) {}
	};

	typedef TraceHit (Map::*TraceFunc)(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx);
	TraceFunc TraceFuncs[4] = {&Map::TraceQ0, &Map::TraceQ1, &Map::TraceQ2, &Map::TraceQ3};

	vector<Cell>	map;

public:
	int Size;
	int WallHeight;

	Map(const int Size, const int WallHeight) : 
		Size(Size), WallHeight(WallHeight) {
		map = vector<Cell>(Size * Size);
	}

	inline Cell& GetCell(const Int2 &Pos) {
		assert(Pos.x >= 0 && Pos.x < Size);
		assert(Pos.y >= 0 && Pos.y < Size);

		return map[Pos.x + Pos.y * Size];
	}

	inline bool InBounds(const Vec2 &Pos) const {
		return Pos.x >= 0.0f && Pos.x < (float)Size && 
			   Pos.y >= 0.0f && Pos.y < (float)Size;
	}

	void Scan(Camera &Cam, Graphics *pGfx);
	void Render(Camera &Cam, const TraceHit &Hit, const int Col, Graphics *pGfx);

	TraceHit Trace(const Vec2 &Origin, const float Theta, Graphics *pGfx);
	TraceHit TraceQ0(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx);
	TraceHit TraceQ1(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx);
	TraceHit TraceQ2(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx);
	TraceHit TraceQ3(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx);

	void Debug(Graphics *pGfx) {
		SolidBrush clrVoid(Color(255,255,255));
		SolidBrush clrWall(Color(191,255,191));
		Pen clrGrid(Color(191,191,191));

		for (int y = 0; y < Size; y++)
			for (int x = 0; x < Size; x++) {
				pGfx->FillRectangle(
					GetCell(Int2(x, y)).IsSolid() ? &clrWall : &clrVoid,
					DebugX + x * DebugScl, DebugY + y * DebugScl, 
					DebugScl, DebugScl);

				if (!GetCell(Int2(x, y)).IsSolid())
					pGfx->DrawRectangle(&clrGrid,
						DebugX + x * DebugScl, DebugY + y * DebugScl, 
						DebugScl-1, DebugScl-1);
			}
	}
};
