#pragma once


#define FPS		60

//#define SCRN_W	1920
//#define SCRN_H	1080

//#define SCRN_W	1280
//#define SCRN_H	720

#define SCRN_W	960
#define SCRN_H	540

//#define MIP_BIAS	(2.0f)
//#define MIP_BIAS	2048.f/1080.f*1.1f
#define MIP_BIAS	(256.f/(float)SCRN_H*1.1f)

#define THREADCOUNT	1
#define FLOATTYPE	FixedX

typedef FixedFloat<20, long, long long> FixedX;

typedef FixedFloat<8,  long, long long> Fixed8;
typedef FixedFloat<16, long, long long> Fixed16;
typedef FixedFloat<24, long, long long> Fixed24;

typedef TempVect2D<float>		Vec2;
typedef TempVect2D<int>			Int2;
typedef TempVect2D<FLOATTYPE>	Vect;


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

	Pixel FBilerp(const Vect &TexUV, const int mip) const {
		assert(mip < Mips);
		const int mipWid = MipDim(mip, Wid);
		const int mipHei = MipDim(mip, Hei);

		//assert(TexUV.x>=0.0f);

		Vect mipPos = TexUV * Vect(mipWid, mipHei);

		Int2 uvPos(mipPos);

		Vect uv = mipPos - uvPos;

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


class Cell {
public:
	Texture *pWallTex;	// Wall is solid if assigned, Hollow if not

	Cell() :
		pWallTex(nullptr) {}

	inline bool IsSolid() const {
		return pWallTex ? true : false;
	}
};


template <typename Type>
struct TraceHit {
//	typedef TempVect2D<Type> Vect;
	Cell	*pCell;
	//float	TexU;
	Type TexU;
	//Vec2	Pos;
	Vect Pos;

	//TraceHit(Cell *pCell, const float TexU, const Vec2 &Pos) :
	//	pCell(pCell), TexU(TexU), Pos(Pos) {}

	TraceHit(Cell *pCell, const Type TexU, const Vect &Pos) :
		pCell(pCell), TexU(TexU), Pos(Pos) {}
};

typedef TraceHit<FLOATTYPE> Hit;

//template <typename Type>
class Map {
protected:
	//typedef TraceHit<Type> Hit;
	//typedef TempVect2D<Type> Vect;
	typedef Hit (Map::*TraceFunc)(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx) const;
	//typedef TraceHit (Map::*TraceFunc)(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) const;
	//const TraceFunc TraceFuncs[4] = {&Map::TraceQ0, &Map::TraceQ1, &Map::TraceQ2, &Map::TraceQ3};

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

	inline Cell* CellPtr(const Int2 &Pos) const {
		assert(Pos.x >= 0 && Pos.x < Size);
		assert(Pos.y >= 0 && Pos.y < Size);

		return (Cell*)map.data() + Pos.x + Pos.y * Size;
	}

	//inline bool InBounds(const Vec2 &Pos) const {
	inline bool InBounds(const Vect &Pos) const {
		return Pos.x >= 0.0f && Pos.x < (float)Size && 
			   Pos.y >= 0.0f && Pos.y < (float)Size;
	}

	//void Scan(Camera &Cam, Graphics *pGfx) const;
	//void Render(Camera &Cam, const TraceHit &Hit, const int Col, Graphics *pGfx) const;
	void Render2(Camera &Cam, const Hit &Hit, const int Col, Graphics *pGfx) const;

	//TraceHit Trace(const Vec2 &Origin, const float Theta, Graphics *pGfx) const;
	//TraceHit TraceQ0(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) const;
	//TraceHit TraceQ1(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) const;
	//TraceHit TraceQ2(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) const;
	//TraceHit TraceQ3(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) const;
	Hit Trace(const Vect &Origin, const FLOATTYPE Theta, Graphics *pGfx) const;
	Hit TraceQ(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx, const int QuadNum) const;

/*	Hit TraceQ0(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx) const;
	Hit TraceQ1(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx) const;
	Hit TraceQ2(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx) const;
	Hit TraceQ3(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx) const; */

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


typedef CRITICAL_SECTION Critical;

class Thread {
protected:
	HANDLE		hRun;		// Main thread will raise this event (Wake) when there's work to do (or we should stop).
	HANDLE		hStop;		// This thread will raise this event when it has stopped working (forever).
	Critical	*pCS;
	bool		Abort;

	static void ThreadFunc(void *pThis);

public:
	Thread(Critical *pCS) : pCS(pCS), Abort(false) {
		hRun = CreateEvent(NULL, FALSE, FALSE, NULL);
		hStop = CreateEvent(NULL, TRUE, FALSE, NULL);

		_beginthread(ThreadFunc, 0, this);
	}

	~Thread() {
		Abort = true;
		SetEvent(hRun);
		WaitForSingleObject(hStop, INFINITE);
		CloseHandle(hStop);
		CloseHandle(hRun);
	}

	void Wake() {
		SetEvent(hRun);
	}
};


class Game {
public:
	//Input input;
	Critical		CS;
	HANDLE			hDone;
	int				ThreadCol;
	int				DoneCol;

	Graphics		*pGfx = nullptr;
	Camera			*pCam = nullptr;
	Map				*pMap = nullptr;

	vector<Texture> Textures;

	vector<Thread*>	Threads;

	Game() {
		InitializeCriticalSectionAndSpinCount(&CS, 256);
		hDone = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	~Game() {
		CloseHandle(hDone);
		DeleteCriticalSection(&CS);

		for (Thread *thread : Threads)
			delete thread;
	}

	void InitGame() {
		pCam = new Camera(SCRN_W, SCRN_H);
		pCam->Pos = Vec2(1.5f, 1.2f);
		pCam->Dir = 0.0f * Ang2Rad;
		pCam->FoV = 90.0f * Ang2Rad;

		//HRESULT res = input.Initialize(hInst, hWnd, SCRN_W, SCRN_H);
		Textures.push_back(Texture(L"textures\\wall1.png"));
		//Textures.push_back(Texture(L"textures\\align.png"));
		//Textures.push_back(Texture(L"textures\\red-brick-wall-living-room-india-effect-ireland-kitchen-shaped-good-looking-uv-h-cm-jpg-iida-comp-pinterest.jpg"));
		

		InitMap();

		for (int i = 0; i < THREADCOUNT; i++)
			Threads.push_back(new Thread(&CS));
	}

	void InitMap() {
		const int Size = 11, s = Size - 1;

		pMap = new Map(Size, SCRN_H);

		const Pixel R(0xFFFF0000);
		const Pixel G(0xFF00FF00);
		const Pixel B(0xFF00007F);

		for (int i = 0; i < Size; i++) {
			pMap->GetCell(Int2(i, 0)).pWallTex = &Textures[0];
			pMap->GetCell(Int2(0, i)).pWallTex = &Textures[0];
			pMap->GetCell(Int2(i, s)).pWallTex = &Textures[0];
			pMap->GetCell(Int2(s, i)).pWallTex = &Textures[0];
		}

		pMap->GetCell(Int2(3, 3)).pWallTex = &Textures[0];
		pMap->GetCell(Int2(7, 3)).pWallTex = &Textures[0];
		pMap->GetCell(Int2(3, 7)).pWallTex = &Textures[0];
		pMap->GetCell(Int2(7, 7)).pWallTex = &Textures[0];
	}

	void Tick(float deltaTime, HWND hWnd) {
		if (!pCam)
			return;

		pCam->Pos = Vec2(5.5, 4.5) + Vec2(sinf(-pCam->Dir * 2.5f), cosf(-pCam->Dir) * 2.0f);

		//pMap->Debug(pGfx);
		//pMap->Scan(*pCam, pGfx);

		//pMap->Scan(*pCam, nullptr);


		pCam->Clear();
		ThreadCol = DoneCol = 0;

		for (Thread *thread : Threads)
			thread->Wake();

		WaitForSingleObject(hDone, INFINITE);


		pCam->Dir += 10.0f * Ang2Rad * deltaTime;
		//pCam->Dir = fmod(pCam->Dir + 10.0f * Ang2Rad * deltaTime + QTAU, QTAU) - ETAU;

		return;
	}

	void Render() {
		pCam->Display(pGfx);
	}
};


enum QuadCellNum : int {
	qTL, qTR, qBL, qBR
};


class Quad {
public:
	int		Depth;
	int		Size;
	Int2	Pos;
	Quad	*pParent = nullptr;
	//Quad	*pNeighbor[4] = {nullptr};
	Quad	*pChild[4] = {nullptr};

	Quad(const int MaxDepth) :
		Quad(nullptr, MaxDepth, 1 << MaxDepth, Int2(0, 0)) {}

	Quad(Quad *pParent, const int Depth, const int Size, const Int2 &Pos) : 
		pParent(pParent), Depth(Depth), Size(Size), Pos(Pos) {}

	inline Quad* operator[] (const int Child) {
		return pChild[Child];
	}

	bool Subdivide() {
		// Depth 0 is the limit
		if (!Depth)
			return false;

		const int half = Size >> 1;
		pChild[qTL] = new Quad(this, Depth - 1, half, Pos + Int2(0,    0   ));
		pChild[qTR] = new Quad(this, Depth - 1, half, Pos + Int2(half, 0   ));
		pChild[qBL] = new Quad(this, Depth - 1, half, Pos + Int2(0,    half));
		pChild[qBR] = new Quad(this, Depth - 1, half, Pos + Int2(half, half));
		
		return true;
	}

	Quad* FindCell(const Int2 &Position) {
		if (!Depth)
			return this;

		const Int2 RelPos = Position - Pos;
		/*
		const int half = Size >> 1;
		int LorR = RelPos.x < half ? 0 : 1;
		int TorB = RelPos.y < half ? 0 : 2;
		int child = TorB + LorR;
		*/

		int LorR = (RelPos.x >> (Depth - 1));
		int TorB = (RelPos.y >> (Depth - 1)) << 1;
		int child = TorB + LorR;

		return pChild[child] ? pChild[child]->FindCell(Position) : this;
	}
};