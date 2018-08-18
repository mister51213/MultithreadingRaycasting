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

	inline bool IsSolid() const {
		return pWallTex ? true : false;
	}
};


template <int Size> class Map {
protected:
	vector<Cell>	map;

public:
	/*struct TraceHit {
		Cell	*pCell;
		//float	Dist;
		Vec2	Pos;
		int		CellX, LastX;
		int		CellY, LastY;

		TraceHit() : 
			pCell(nullptr) {}

		TraceHit(Pixel *pCell, const float Dist, const Vec2 &Pos, const int CellX, const int CellY) :
			pCell(pCell), Dist(Dist), Pos(Coord), CellX(CellX), CellY(CellY) {}
	};*/

	int WallHeight;

	Map(const int WallHeight) : WallHeight(WallHeight) {
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

	/*TraceHit Trace(const Vec2 &Origin, const Vec2 &Step) {
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
	}*/


	struct TraceHit2 {
		Cell	*pCell;
		float	TexU;
		Vec2	Pos;

		TraceHit2(Cell *pCell, const float TexU, const Vec2 &Pos) :
			pCell(pCell), TexU(TexU), Pos(Pos) {}
	};

	TraceHit2 Trace2(const Vec2 &Origin, const float Theta, Graphics *pGfx) {
		float theta = fmod(Theta + TAU, TAU);

		int quad = 0;
		for (; theta >= QTAU; theta -= QTAU, quad++);

		static Int2 oppStep[4] = {
			Int2( 1, 0),	// Quad0: East		00
			Int2( 1, 0),	// Quad1: South		01
			Int2(-1, 0),	// Quad2: West		10
			Int2(-1, 0)		// Quad3: North		11
		};

		static Int2 adjStep[4] = {
			Int2(0, -1),	// Quad0: East		00
			Int2(0,  1),	// Quad1: South		01
			Int2(0,  1),	// Quad2: West		10
			Int2(0, -1)		// Quad3: North		11
		};

		Int2 cellPos(Origin);
		const Vec2 orgPos = Origin - Vec2(cellPos);

		/*const float orgX = Origin.x - (int)Origin.x;
		const float orgY = Origin.y - (int)Origin.y;*/

		float opp = orgPos.y * tanf(theta);
		float pos = orgPos.x + opp;
		if (pos < 1.0f) {
			// Hit adjacent wall
			
		} else {
			// Hit opposite wall
			opp = tanf(QTAU - theta);
			//opp = (1.0f - orgPos.x) * tanf(QTAU - theta);
			pos = orgPos.y - (1.0f - orgPos.x) * opp;

			Int2 cellDir = oppStep[quad];

			if (pGfx) {Vec2 OppPos = Vec2(cellPos) + Vec2(1, pos);
			Point Dst(DebugX + OppPos.x * DebugScl, DebugY + OppPos.y * DebugScl);
			pGfx->DrawEllipse(&Pen(Color(255, 255,0,0)), Dst.X-2, Dst.Y-2, 4, 4);}

			for (;;) {
				cellPos += cellDir;
				if (GetCell(cellPos /* + cellDir*/).IsSolid())
					break;
				//cellPos += cellDir;

				if (pos - opp >= 0.0f) {
					// Hit the opposite wall
					//cellDir = oppStep[quad];
					pos -= opp;

					if (pGfx) {Vec2 OppPos = Vec2(cellPos) + Vec2(1, pos);
					Point Dst(DebugX + OppPos.x * DebugScl, DebugY + OppPos.y * DebugScl);
					pGfx->DrawEllipse(&Pen(Color(255, 191,0,0)), Dst.X-2, Dst.Y-2, 4, 4);}
				} else {
					// Crossed to an adjacent wall

					if (pGfx) {Vec2 OppPos = Vec2(cellPos) + Vec2(pos / opp, 0);
					Point Dst(DebugX + OppPos.x * DebugScl, DebugY + OppPos.y * DebugScl);
					pGfx->DrawEllipse(&Pen(Color(255, 0,191,0)), Dst.X-2, Dst.Y-2, 4, 4);}

					//cellDir = adjStep[quad];
					cellPos += adjStep[quad];
					if (GetCell(cellPos).IsSolid()) {
						pos /= opp;	// law of similar triangles simplified

						//return TraceHit2(nullptr, 0, Vec2());

						if (pGfx) {Vec2 OppPos = Vec2(cellPos) + Vec2(pos, 1);
						Point Dst(DebugX + OppPos.x * DebugScl, DebugY + OppPos.y * DebugScl);
						pGfx->DrawEllipse(&Pen(Color(255, 0,191,0)), Dst.X-4, Dst.Y-4, 8, 8);}

						return TraceHit2(&GetCell(cellPos), pos, Vec2(cellPos) + Vec2(pos, 1));
					}

					pos -= opp - 1.0f;

					if (pGfx) {Vec2 OppPos = Vec2(cellPos) + Vec2(1, pos);
					Point Dst(DebugX + OppPos.x * DebugScl, DebugY + OppPos.y * DebugScl);
					pGfx->DrawEllipse(&Pen(Color(255, 0,0,191)), Dst.X-2, Dst.Y-2, 4, 4);}
				}

				if (GetCell(cellPos).IsSolid())
					break;
			}

			if (pGfx) {Vec2 OppPos = Vec2(cellPos) + Vec2(0, pos);
			Point Dst(DebugX + OppPos.x * DebugScl, DebugY + OppPos.y * DebugScl);
			pGfx->DrawEllipse(&Pen(Color(255, 255,0,0)), Dst.X-4, Dst.Y-4, 8, 8);}

			// We've hit a solid (opposite) wall. Render it!
			// Render slice at pos
			return TraceHit2(&GetCell(cellPos), pos, Vec2(cellPos) + Vec2(0, pos));
			//return TraceHit2(nullptr, 0, Vec2());
		}


		return TraceHit2(nullptr, 0, Vec2());
	}

	void Scan(Camera &Cam, Graphics *pGfx) {
		const float slice = Cam.FoV / (float)Cam.Wid;
		const int halfwid = Cam.Wid / 2;

		Cam.Clear();

		for (int col = 0; col < Cam.Wid; col++) {
		//int col = halfwid; {

			float ang = (float)(col - halfwid) * slice + Cam.Dir;

			if (pGfx) {
				/*{Point Org(DebugX + Cam.Pos.x * DebugScl, DebugY + Cam.Pos.y * DebugScl);
				Point Dst(Org.X + sin(ang) * DebugScl*4, Org.Y - cos(ang) * DebugScl*4);
				pGfx->DrawLine(&Pen(Color(63, 0,0,255)), Org, Dst);}*/
			}

			//TraceHit hit = Trace(Cam.Pos, Vec2(sinf(ang), -cosf(ang)) * 0.01f);
			TraceHit2 hit = Trace2(Cam.Pos, ang, pGfx);
			
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
	
	void Render(Camera &Cam, const TraceHit2 &Hit, const int Col, Graphics *pGfx) {
		Texture *pWall = Hit.pCell->pWallTex;

		const float dist = (Hit.Pos - Cam.Pos).Size();

		const float hei = (float)WallHeight / dist;
		const int half = (int)(hei / 2.0f);
		const int top = max(Cam.Hei / 2 - half, 0);
		const int btm = min(Cam.Hei / 2 + half, Cam.Hei);

		//const int mip = 0;
		//const int mip = min((int)dist, pWall->Mips);
		const int mip = min((int)sqrtf(dist), pWall->Mips);
		const int mipWid = pWall->Wid >> mip;
		const int mipHei = pWall->Hei >> mip;

		const float sclV = (float)mipHei / hei;

		if (pGfx) {
			Pen clrTrace(Color(255, 0,0, Col / 3));
			Point Org(DebugX + Cam.Pos.x * DebugScl, DebugY + Cam.Pos.y * DebugScl);
			Point Dst(DebugX + Hit.Pos.x * DebugScl, DebugY + Hit.Pos.y * DebugScl);

			pGfx->DrawLine(&clrTrace, Org, Dst);
		}

		const int texU = (int)(Hit.TexU * (float)mipWid);
		assert(texU >= 0 && texU < mipWid);

		Pixel *pTex = pWall->pMip[mip] + texU;
		const int utop = Cam.Hei / 2 - half;

		int ofs = Col + top * Cam.Wid;
		for (int y = top; y < btm; y++, ofs += Cam.Wid) {
			const int texV = (int)((float)(y - utop) * sclV);
			assert(texV >= 0 && texV < mipHei);
			Cam.pImage[ofs] = pTex[texV * mipWid];
		}
	}

	/*void Render(Camera &Cam, const TraceHit &Hit, const int Col, Graphics *pGfx) {
		Texture *pWall = Hit.pCell->pWallTex;

		Vec2 path = Hit.Pos - Cam.Pos;

		float fracX = Hit.Pos.x - (float)Hit.CellX;
		float fracY = Hit.Pos.y - (float)Hit.CellY;
		float fracU;
		Vec2 hitPos;

		if (Hit.LastX != Hit.CellX) {
			// Crossed a vertical boundary
			fracU = fracY;
		} else {
			// Crossed a horizontal boundary
			fracU = fracX;
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

		const float sclV = (float)mipHei / hei;

		if (pGfx) {
			Pen clrTrace(Color(31, 0,0,0));
			Point Org(DebugX + Cam.Pos.x * DebugScl, DebugY + Cam.Pos.y * DebugScl);
			Point Dst(DebugX + Hit.Pos.x * DebugScl, DebugY + Hit.Pos.y * DebugScl);

			pGfx->DrawLine(&clrTrace, Org, Dst);
		}

		const int texU = (int)(fracU * (float)mipWid);
		assert(texU >= 0 && texU < mipWid);

		Pixel *pTex = pWall->pMip[mip] + texU;
		const int utop = Cam.Hei / 2 - half;

		int ofs = Col + top * Cam.Wid;
		for (int y = top; y < btm; y++, ofs += Cam.Wid) {
			const int texV = (int)((float)(y - utop) * sclV);
			assert(texV >= 0 && texV < mipHei);
			Cam.pImage[ofs] = pTex[texV * mipWid];
		}
	}*/
};
