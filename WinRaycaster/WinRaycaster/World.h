#pragma once


static int DebugX, DebugY, DebugScl;


class Pixel {
public:
	unsigned argb;

	Pixel() {}

	Pixel(const unsigned argb) :
		argb(argb) {}

	inline unsigned Alpha() const {
		return argb >> 24;
	}

	inline Pixel Quarter() const {
		return (argb & 0xFCFCFCFC) >> 2;
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
		memset(pImage, -1, Wid * Hei * sizeof(Pixel));
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

		//pImage = new Pixel[Wid * Hei];
		//assert(pImage);

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
};


template <int Size> class Map {
protected:
	vector<Cell>	map;

public:
	struct TraceHit {
		Cell	*pCell;
		//float	Dist;
		Vec2	Pos;
		int		CellX, LastX;
		int		CellY, LastY;

		TraceHit() : 
			pCell(nullptr) {}

		TraceHit(Pixel *pCell, const float Dist, const Vec2 &Pos, const int CellX, const int CellY) :
			pCell(pCell), Dist(Dist), Pos(Coord), CellX(CellX), CellY(CellY) {}
	};

	int WallHeight;

	Map(const int WallHeight) : WallHeight(WallHeight) {
		map = vector<Cell>(Size * Size);
	}

	inline Cell& GetCell(const int x, const int y) {
		assert(x >= 0 && x < Size);
		assert(y >= 0 && y < Size);

		return map[x + y * Size];
	}

	inline bool InBounds(const Vec2 &Pos) const {
		return Pos.x >= 0.0f && Pos.x < (float)Size && 
			   Pos.y >= 0.0f && Pos.y < (float)Size;
	}

	TraceHit Trace(const Vec2 &Origin, const Vec2 &Step) {
		TraceHit hit;
		hit.Pos = Origin;

		hit.LastX = hit.LastY = 0;

		for (; InBounds(hit.Pos); hit.Pos += Step) {
			hit.CellX = (int)hit.Pos.x;
			hit.CellY = (int)hit.Pos.y;

			hit.pCell = map.data() + hit.CellX + hit.CellY * Size;
			
			if (hit.pCell->pWallTex) {
				//hit.Dist = (hit.Pos - Origin).Size();
				return hit;
			}

			hit.LastX = hit.CellX;
			hit.LastY = hit.CellY;
		}

		return TraceHit();
	}


	struct TraceHit2 {
		
	};

	TraceHit2 Trace2(const Vec2 &Origin, const float Theta) {
		float theta = modf(Theta + TAU, TAU);

		int quad = 0;
		for (; theta >= QTAU; theta -= QTAU, quad++);

		/*static Vec2 tmp[4] = {
			Vec2(0, -1),
			Vec2(+1, 0),
			Vec2(0, +1),
			Vec2(-1, 0)
		};*/

		const float orgX = Origin.x - (int)Origin.x;
		const float orgY = Origin.y - (int)Origin.y;

		float opp = orgY * tanf(theta);
		float hit = orgX + opp;
		if (hit < 1.0f) {
			// Hit top
			
		} else {
			// Hit right
			opp = (1.0f - orgX) * tanf(QTAU - theta);
			hit = orgY - opp;

		}


	}

	void Scan(Camera &Cam, Graphics *pGfx) {
		const float slice = Cam.FoV / (float)Cam.Wid;
		const int halfwid = Cam.Wid / 2;

		Cam.Clear();

		for (int col = 0; col < Cam.Wid; col++) {
			float ang = (float)(col - halfwid) * slice + Cam.Dir;

			TraceHit hit = Trace(Cam.Pos, Vec2(sinf(ang), -cosf(ang)) * 0.01f);
			
			if (hit.pCell) {
				// Ray hit a wall
				Render(Cam, hit, col, pGfx);
			} else {
				// Ray exited the world
				int x = 0;
			}
		}
	}

	void Debug(Graphics *pGfx) {
		SolidBrush clrVoid(Color(255,255,255));
		SolidBrush clrWall(Color(63,0,0));

		for (int y = 0; y < Size; y++)
			for (int x = 0; x < Size; x++)
				pGfx->FillRectangle(
					GetCell(x, y).pWallTex ? &clrWall : &clrVoid,
					DebugX + x * DebugScl, 
					DebugY + y * DebugScl, 
					DebugScl, 
					DebugScl);
	}

	//void Render(Camera &Cam, const TraceHit &Hit, const int Col) {
	//	Texture *pWall = Hit.pCell->pWallTex;

	//	//Cam.pImage[Col] = 0xFF000000;
	//	//Cam.pImage[Col + (Cam.Hei - 1) * Cam.Wid] = 0xFF000000;

	//	/*const float fracX = Hit.Pos.x - (float)Hit.CellX;
	//	const float fracY = Hit.Pos.y - (float)Hit.CellY;

	//	const int texU = (int)(min(fracX, fracY) * (float)pWall->Wid);*/

	//	Vec2 path = Hit.Pos - Cam.Pos;

	//	float fracX = Hit.Pos.x - (float)Hit.CellX;
	//	float fracY = Hit.Pos.y - (float)Hit.CellY;
	//	float fracU;
	//	Vec2 hitPos;

	//	if (Hit.LastX != Hit.CellX) {
	//		// Crossed a vertical boundary

	//		//const float ofs = path.y / path.x * fracX;
	//		//fracU = fracY - ofs;
	//		fracU = fracY;
	//		
	//		//hitPos = Vec2((float)Hit.CellX, (float)Hit.CellY + fracU);
	//	} else {
	//		// Crossed a horizontal boundary

	//		//const float ofs = path.x / path.y * fracY;
	//		//fracU = fracX - ofs;
	//		fracU = fracX;

	//		//hitPos = Vec2((float)Hit.CellX + fracU, (float)Hit.CellY);
	//	}

	//	hitPos = Hit.Pos;

	//	const float dist = (hitPos - Cam.Pos).Size();

	//	const float hei = (float)WallHeight / dist;
	//	const int half = (int)(hei / 2.0f);
	//	const int top = max(Cam.Hei / 2 - half, 0);
	//	const int btm = min(Cam.Hei / 2 + half, Cam.Hei);

	//	const float sclV = (float)pWall->Hei / hei;

	//	const int texU = (int)(fracU * (float)pWall->Wid);
	//	assert(texU >= 0 && texU < pWall->Wid);

	//	Pixel *pTex = pWall->pImage + texU;
	//	const int utop = Cam.Hei / 2 - half;

	//	int ofs = Col + top * Cam.Wid;
	//	for (int y = top; y < btm; y++, ofs += Cam.Wid) {
	//		//Cam.pImage[Col + y * Cam.Wid] = *Hit.pCell;
	//		//Cam.pImage[ofs] = *Hit.pCell;
	//		const int texV = (int)((float)(y - utop) * sclV);
	//		assert(texV >= 0 && texV < pWall->Hei);

	//		Cam.pImage[ofs] = pTex[texV * pWall->Wid];
	//	}
	//}

	void Render(Camera &Cam, const TraceHit &Hit, const int Col, Graphics *pGfx) {
		Texture *pWall = Hit.pCell->pWallTex;

		//Cam.pImage[Col] = 0xFF000000;
		//Cam.pImage[Col + (Cam.Hei - 1) * Cam.Wid] = 0xFF000000;

		/*const float fracX = Hit.Pos.x - (float)Hit.CellX;
		const float fracY = Hit.Pos.y - (float)Hit.CellY;

		const int texU = (int)(min(fracX, fracY) * (float)pWall->Wid);*/

		Vec2 path = Hit.Pos - Cam.Pos;

		float fracX = Hit.Pos.x - (float)Hit.CellX;
		float fracY = Hit.Pos.y - (float)Hit.CellY;
		float fracU;
		Vec2 hitPos;

		if (Hit.LastX != Hit.CellX) {
			// Crossed a vertical boundary

			//const float ofs = path.y / path.x * fracX;
			//fracU = fracY - ofs;
			fracU = fracY;

			//hitPos = Vec2((float)Hit.CellX, (float)Hit.CellY + fracU);
		} else {
			// Crossed a horizontal boundary

			//const float ofs = path.x / path.y * fracY;
			//fracU = fracX - ofs;
			fracU = fracX;

			//hitPos = Vec2((float)Hit.CellX + fracU, (float)Hit.CellY);
		}

		hitPos = Hit.Pos;

		const float dist = (hitPos - Cam.Pos).Size();

		const float hei = (float)WallHeight / dist;
		const int half = (int)(hei / 2.0f);
		const int top = max(Cam.Hei / 2 - half, 0);
		const int btm = min(Cam.Hei / 2 + half, Cam.Hei);

		//const int mip = 0;
		//const int mip = min((int)dist, pWall->Mips);
		const int mip = min((int)sqrtf(dist), pWall->Mips);
		const int mipWid = pWall->Wid >> mip;
		const int mipHei = pWall->Hei >> mip;

		//const float sclV = (float)pWall->Hei / hei;
		const float sclV = (float)mipHei / hei;

		if (pGfx) {
			Pen clrTrace(Color(31, 0,0,0));
			Point Org(DebugX + Cam.Pos.x * DebugScl, DebugY + Cam.Pos.y * DebugScl);
			Point Dst(DebugX + Hit.Pos.x * DebugScl, DebugY + Hit.Pos.y * DebugScl);

			pGfx->DrawLine(&clrTrace, Org, Dst);
		}

		//const int texU = (int)(fracU * (float)pWall->Wid);
		//assert(texU >= 0 && texU < pWall->Wid);
		const int texU = (int)(fracU * (float)mipWid);
		assert(texU >= 0 && texU < mipWid);

		Pixel *pTex = pWall->pMip[mip] + texU;
		const int utop = Cam.Hei / 2 - half;

		int ofs = Col + top * Cam.Wid;
		for (int y = top; y < btm; y++, ofs += Cam.Wid) {
			//Cam.pImage[Col + y * Cam.Wid] = *Hit.pCell;
			//Cam.pImage[ofs] = *Hit.pCell;
			const int texV = (int)((float)(y - utop) * sclV);
			//assert(texV >= 0 && texV < pWall->Hei);
			assert(texV >= 0 && texV < mipHei);

			//Cam.pImage[ofs] = pTex[texV * pWall->Wid];
			Cam.pImage[ofs] = pTex[texV * mipWid];
		}
	}
};
