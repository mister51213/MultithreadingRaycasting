#pragma once

/*
struct Cell {
	// Coordinates start at TOP LEFT CORNER
	// Positive X goes RIGHT	// Positive Y goes DOWN
	Vec2 topLeft, bottomRight;

	Cell();
};

class World {
private:
	World();
	World(const float w, const float h, const Vec2 cellSize);
	void InitializeMatrix();

	static World* _world;
	float _worldWidth, _worldHeight;
	vector<vector<Cell>> _cells;
	Vec2 _cellSize;

public:
	static World * GetWorld();
	static World * GetWorld(const float w, const float h, const Vec2 cellSize);
};
*/

class Pixel {
public:
	unsigned argb;

	Pixel() {}

	Pixel(const unsigned argb) :
		argb(argb) {}

	inline unsigned Alpha() const {
		return argb >> 24;
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
	}
};


template <int Size> class Map {
protected:
	vector<Pixel>	map;

public:
	struct TraceHit {
		Pixel	*pCell;
		Vec2	Coord;
		float	Dist;

		TraceHit(Pixel *pCell, const Vec2 &Coord, const float Dist) :
			pCell(pCell), Coord(Coord), Dist(Dist) {}
	};

	int WallHeight;

	Map(const int WallHeight) : WallHeight(WallHeight) {
		map = vector<Pixel>(Size * Size, 0);
	}

	inline Pixel& Cell(const int x, const int y) {
		assert(x >= 0 && x < Size);
		assert(y >= 0 && y < Size);

		return map[x + y * Size];
	}

	TraceHit Trace(const Vec2 &Origin, const Vec2 &Step) {
		const float size = (float)Size;
		
		Vec2 pos = Origin;
		for (; pos.x >= 0.0f && pos.x < size && pos.y >= 0.0f && pos.y < size; pos += Step) {
			Pixel &cell = map[(int)pos.x + (int)pos.y * Size];
			
			if (cell.Alpha())
				return TraceHit(&cell, pos, (pos - Origin).Size());
		}

		return TraceHit(nullptr, pos, 0);
	}

	void Scan(Camera &Cam) {
		const float slice = Cam.FoV / (float)Cam.Wid;
		const int halfwid = Cam.Wid / 2;

		Cam.Clear();

		for (int col = 0; col < Cam.Wid; col++) {
			float ang = (float)(col - halfwid) * slice + Cam.Dir;

			TraceHit hit = Trace(Cam.Pos, Vec2(sinf(ang), -cosf(ang)) * 0.01f);
			
			if (hit.pCell) {
				// Ray hit a wall
				Render(Cam, hit, col);
			} else {
				// Ray exited the world
				int x = 0;
			}
		}
	}

	void Render(Camera &Cam, const TraceHit &Hit, const int Col) {
		const int hei = (int)((float)WallHeight / (Hit.Dist /* + 1.0f*/));
		const int half = hei / 2;
		const int top = max(Cam.Hei / 2 - half, 0);
		const int btm = min(Cam.Hei / 2 + half, Cam.Hei);
		
		Cam.pImage[Col] = 0xFF000000;
		Cam.pImage[Col + (Cam.Hei - 1) * Cam.Wid] = 0xFF000000;

		int ofs = Col + top * Cam.Wid;
		for (int y = top; y < btm; y++, ofs += Cam.Wid) {
			//Cam.pImage[Col + y * Cam.Wid] = *Hit.pCell;
			Cam.pImage[ofs] = *Hit.pCell;
		}
	}
};
