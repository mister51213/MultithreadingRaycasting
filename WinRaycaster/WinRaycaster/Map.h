#pragma once


__declspec(selectany) float DebugX, DebugY, DebugScl;


#ifdef SHOWDEBUG

#define DbgPlot(Position, Rad, color) {										\
	if (pGfx) {																\
		Vec2 OppPos = Vec2(cellPos/*pQuad->Pos*/) + Position;							\
		int DstX = (int)(DebugX + OppPos.x * DebugScl);						\
		int DstY = (int)(DebugY + OppPos.y * DebugScl);						\
		pGfx->DrawEllipse(&Pen(color), DstX-Rad, DstY-Rad, Rad*2, Rad*2);	\
	}																		\
}

#else

#define DbgPlot(Pos, Rad, color)

#endif


struct Shape {
	enum ShapeType {
		AABB = 0,
		Sphere = 1,
		Cone = 2
	};

	Vect3 pos;
	Vect3 rot;

	union Bounds {
		Vect2 hWidHei;
		Vect2 radHei;
	};
};

template <typename Type>
struct TraceHit {
	Type		*pCell;
	FLOATTYPE	TexU;
	FLOATTYPE	TexV;
	Vect3		Pos;

	TraceHit(Type *pCell, const FLOATTYPE TexU, const FLOATTYPE TexV, const Vect3 &Pos) :
		pCell(pCell), TexU(TexU), TexV(TexV), Pos(Pos) {}
};

class Cell {
public:
	Texture *pWallTex;	// Wall is solid if assigned, Hollow if not

	Cell() :
		pWallTex(nullptr) {}

	// Should we QUERY or SKIP current cell?
	// - If a solid wall, return that surface immediately
	// - If Contains inner shapes, then query interior of cell
	// - If neither of these are true, SKIP the cell
	inline bool IsSolid() const {
		//return pWallTex ? true : false;

		return pWallTex || !innerShapes.empty();
	}

	TraceHit<Cell> Query(Vect3 pos, Int3 lastPos, Int3 cellPos);

	std::vector<Shape> innerShapes;
};


typedef TraceHit<Cell> Hit;
//typedef TraceHit<Quad> HitQ;

class GameMap {
protected:
	typedef Hit (GameMap::*TraceFunc)(Int3 cellPos, Vect3 orgPos, const FLOATTYPE theta, Graphics *pGfx) const;
	const TraceFunc TraceFuncs[4] = {&GameMap::TraceQ<0>, &GameMap::TraceQ<1>, &GameMap::TraceQ<2>, &GameMap::TraceQ<3>};
	//typedef HitQ (GameMap::*TraceFunc)(const Vect2 &Origin, const FLOATTYPE theta, Graphics *pGfx);
	//TraceFunc TraceFuncs[4] = {&GameMap::TraceQQ<0>, &GameMap::TraceQQ<1>, &GameMap::TraceQQ<2>, &GameMap::TraceQQ<3>};

	//Quad			QMap;

public:
	int		Size;
	int		WallHeight;

	vector<Cell>	Map;

	GameMap(const int Size, const int WallHeight) : 
		Size(Size), WallHeight(WallHeight) {
		Map = vector<Cell>(Size * Size * Size);

		//QMap = Quad(4);
	}
	/*
	inline Quad& Root() {
		return QMap;
	}
	*/
	inline Cell& GetCell(const Int3 &Pos) {
		assert(Pos.x >= 0 && Pos.x < Size);
		assert(Pos.y >= 0 && Pos.y < Size);
		assert(Pos.z >= 0 && Pos.z < Size);
		return Map[Pos.x + Pos.y * Size + Pos.z * Size * Size];
	}

	inline Cell* CellPtr(const Int3 &Pos) const {
		assert(Pos.x >= 0 && Pos.x < Size);
		assert(Pos.y >= 0 && Pos.y < Size);
		assert(Pos.z >= 0 && Pos.z < Size);

		return (Cell*)Map.data() + Pos.x + Pos.y * Size + Pos.z * Size * Size;
	}

	inline bool InBounds(const Vect3 &Pos) const {
		return Pos.x >= 0.0f && Pos.x < (float)Size && 
			   Pos.y >= 0.0f && Pos.y < (float)Size &&
			   Pos.z >= 0.0f && Pos.z < (float)Size;
	}

	void Debug(Graphics *pGfx);
	//void DebugQuad(Quad &Q, Graphics *pGfx);

	
	void Render(Camera &Cam, const Hit &hit, const int ofs, Graphics *pGfx) const;
	//void Render(Camera &Cam, const /*HitQ*/Hit &hit, const int Col, Graphics *pGfx) const;
	/*HitQ*/ Hit Trace(const Vect3 &Origin, const FLOATTYPE Theta, Graphics *pGfx);

	// Multi shape version
	Hit TraceS(Vect3 CamPos, const Vect3 step, Graphics *pGfx) const { 
	
		Int3 lastPos;
		for (Vect3 pos = CamPos;; pos += step) {
			Int3 cellPos(pos);

			Cell *pCell = CellPtr(cellPos);
			if (pCell->IsSolid())
				return pCell->Query(pos, lastPos, cellPos);

			lastPos = cellPos;
		}
	}

	#pragma region Basic cube version
	//Hit TraceS(Vect3 CamPos, const Vect3 step /*Vect2 CamAng*/, Graphics *pGfx) const {
	//	/*const Vect3 step = Vect3(
	//		sinf(CamAng.y) * sinf(CamAng.x),
	//		sinf(CamAng.y) * cosf(CamAng.x),
	//		-cosf(CamAng.y)
	//	) * 0.01f;*/

	//	Int3 lastPos;
	//	for (Vect3 pos = CamPos;; pos += step) {
	//		Int3 cellPos(pos);

	//		Cell *pCell = CellPtr(cellPos);
	//		if (pCell->pWallTex) {
	//			lastPos -= cellPos;
	//			
	//			Vect2 texUV(0, 0);
	//			if (lastPos.x)
	//				texUV = Vect2(pos.y - (float)cellPos.y, pos.z - (float)cellPos.z);
	//			else if (lastPos.y)
	//				texUV = Vect2(pos.x - (float)cellPos.x, pos.z - (float)cellPos.z);
	//			else
	//				texUV = Vect2(pos.x - (float)cellPos.x, pos.y - (float)cellPos.y);

	//			return Hit(pCell, texUV.x, texUV.y, pos);
	//		}
	//		lastPos = cellPos;
	//	}
	//}

#pragma endregion

	enum WhatDirs : int {
		dirPX, dirNX,
		dirPY, dirNY,
		dirPZ, dirNZ
	};
	
	typedef Vect3 (*Whatever)(const float TexU, const float TexV);
	
	static Vect3 WhatPX(const float TexU, const float TexV) { return Vect3(1, TexU, TexV); }
	static Vect3 WhatNX(const float TexU, const float TexV) { return Vect3(0, TexU, TexV); }
	static Vect3 WhatPY(const float TexU, const float TexV) { return Vect3(TexU, 1, TexV); }
	static Vect3 WhatNY(const float TexU, const float TexV) { return Vect3(TexU, 0, TexV); }
	static Vect3 WhatPZ(const float TexU, const float TexV) { return Vect3(TexU, TexV, 1); }
	static Vect3 WhatNZ(const float TexU, const float TexV) { return Vect3(TexU, TexV, 0); }

	Whatever DirFunc[6] = {WhatPX, WhatNX, WhatPY, WhatNY, WhatPZ, WhatNZ};

	Hit Trace3D(const Vect3 CamPos, const Vect2 CamDir, Graphics *pGfx) const {
		Vect2 tanUV(tanf(CamDir.x), tanf(CamDir.y));
		Vect2 comUV(tanf(FLOATTYPE(QTAU) - CamDir.x), tanf(FLOATTYPE(QTAU) - CamDir.y));

		int dir;
		Int3 cellPos(CamPos), step;
		Vect3 lclPos = CamPos - cellPos;

		if (abs(tanUV.x) < abs(comUV.x)) {		// Angle is closer to N/S
			if (abs(tanUV.y) < abs(tanUV.x)) {	// Angle is closer to U/D
				step = Int3(0, 0, tanUV.y < 0.0f ? -1 : 1);
				dir = tanUV.y < 0.0f ? dirNZ : dirPZ;
			} else {							// Angle is closer to N/S
				step = Int3(0, comUV.x < 0.0f ? -1 : 1, 0);
				dir = comUV.x < 0.0f ? dirNY : dirPY;
			}
		} else {								// Angle is closer to E/W
			if (abs(tanUV.y) < abs(comUV.x)) {	// Angle is closer to U/D
				step = Int3(0, 0, tanUV.y < 0.0f ? -1 : 1);
				dir = tanUV.y < 0.0f ? dirNZ : dirPZ;
			} else {							// Angle is closer to E/W
				step = Int3(tanUV.x < 0.0f ? -1 : 1, 0, 0);
				dir = tanUV.x < 0.0f ? dirNX : dirPX;
			}
		}

		FLOATTYPE posU = lclPos.x + tanUV.x * lclPos.y;
		FLOATTYPE posV = lclPos.y + tanUV.y * lclPos.z;

		for (;;) {
			const int adjU = (posU < 0.0f) ? -1 : (posU >= 1.0f ? 1 : 0);
			const int adjV = (posV < 0.0f) ? -1 : (posV >= 1.0f ? 1 : 0);
			
			if (!adjU && !adjV) {		// Both hit opposite
				DbgPlot(Vect2(posU, 0), 1, Color(255, 0, 0, 0));

				cellPos += step;

				Cell *pCell = CellPtr(cellPos);
				if (pCell->pWallTex) {
					return Hit(pCell, posU, posV, Vect3(cellPos) + DirFunc[dir](posU, posV));
				}

				posU += tanUV.x;
				posV += tanUV.y;
			} else if (adjU && adjV) {	// Neither hit opposite
				return Hit(nullptr, 0, 0, Vect3());
			} else if (adjU) {			// U hit adjacent, V hit opposite
				return Hit(nullptr, 0, 0, Vect3());
			} else {					// V hit adjacent, U hit opposite
				return Hit(nullptr, 0, 0, Vect3());
			}

		}

	}

	template<int QuadNum>
	Hit TraceQ(Int3 cellPos, Vect3 orgPos, const FLOATTYPE theta, Graphics *pGfx) const {
		//return Hit(nullptr, 0, Vect2());

		FLOATTYPE TabFlt[] = {
			0.0, 1.0, 1.0, 0
		};

		int TabInt[] = {
			-1, 1, 1, -1
		};

		int XIdx[] = {
			0, 1, 0, 1
		};

		FLOATTYPE Flip[4][3] = {
			{-1.0,  1.0,  1.0},
			{ 1.0, -1.0,  1.0},
			{-1.0,  1.0, -1.0},
			{ 1.0, -1.0, -1.0}
		};

		const FLOATTYPE FltVal = TabFlt[QuadNum];
		const int       IntVal = TabInt[QuadNum];
		const int       X = XIdx[QuadNum], Y = 1 - X;

		FLOATTYPE opp, com;
		if (QuadNum & 1) {
			opp = tanf(FLOATTYPE(QTAU) - theta); com = tanf(theta);
		} else {
			opp = tanf(theta); com = tanf(FLOATTYPE(QTAU) - theta);
		}

		FLOATTYPE pos = orgPos[X] - (FltVal - orgPos[Y]) * opp;
		opp *= (FLOATTYPE)-IntVal;

		for (Vect3 ofs = orgPos;;) {
			if (pos < 0.0f) {
				pos += 1.0f;

				ofs[X] = 0.0f;
				ofs[Y] = FltVal - (1.0f - pos) * com;
				DbgPlot(ofs, 2, Color(255, 0,0,255));

				cellPos[X]--;
				if (CellPtr(cellPos)->IsSolid()) {
					//return Hit(nullptr, 0, Vect2());
					pos = FltVal - (1.0f - pos) * com;

					ofs[X] = 1.0f;
					ofs[Y] = pos;
					DbgPlot(ofs, 6, Color(255, 0,0,255));

					return Hit(CellPtr(cellPos), pos * Flip[QuadNum][0], 0.5f, Vect3(cellPos) + ofs);
				}
			} else if (pos >= 1.0f) {
				pos -= 1.0f;

				ofs[X] = 1.0f;
				ofs[Y] = FltVal + pos * com;
				DbgPlot(ofs, 2, Color(255, 0,191,0));

				cellPos[X]++;
				if (CellPtr(cellPos)->IsSolid()) {
					//return Hit(nullptr, 0, Vect2());
					pos = FltVal + pos * com;

					ofs[X] = 0.0f;
					ofs[Y] = pos;
					DbgPlot(ofs, 6, Color(255, 0,191,0));

					return Hit(CellPtr(cellPos), pos * Flip[QuadNum][1], 0.5f, Vect3(cellPos) + ofs);
				}
			} else {
				ofs[X] = pos;
				ofs[Y] = FltVal;
				DbgPlot(ofs, 2.0f, Color(255, 255,0,0));

				cellPos[Y] += IntVal;
				if (CellPtr(cellPos)->IsSolid()) {
					//return Hit(nullptr, 0, Vect2());
				
					ofs[X] = pos;
					ofs[Y] = 1.0f - FltVal;
					DbgPlot(ofs, 6, Color(255, 255,0,0));

					return Hit(CellPtr(cellPos), pos * Flip[QuadNum][2], 0.5f, Vect3(cellPos) + ofs);
				}

				pos += opp;
			}
		}
	}
	
	/*
	template<int QuadNum>
	HitQ TraceQQ(const Vect2 &Origin, const FLOATTYPE theta, Graphics *pGfx) {
		//return Hit(nullptr, 0, Vect2());

		FLOATTYPE TabFlt[] = {
			0.0, 1.0, 1.0, 0
		};

		int TabInt[] = {
			-1, 1, 1, -1
		};

		int XIdx[] = {
			0, 1, 0, 1
		};

		FLOATTYPE Flip[4][3] = {
			{-1.0,  1.0,  1.0},
			{ 1.0, -1.0,  1.0},
			{-1.0,  1.0, -1.0},
			{ 1.0, -1.0, -1.0}
		};

		const FLOATTYPE FltVal = TabFlt[QuadNum];
		const int       IntVal = TabInt[QuadNum];
		const int       X = XIdx[QuadNum], Y = 1 - X;

		Int2 cellPos(Origin);

		Quad *pQuad = &QMap.FindCell(cellPos);
		if (pQuad->IsSolid())
			return HitQ(nullptr, 0, Vect2());

		cellPos = pQuad->Pos;
		Vect2 orgPos = Origin - Vect2(cellPos);

		FLOATTYPE opp, com;
		if (QuadNum & 1) {
			opp = tanf(FLOATTYPE(QTAU) - theta); com = tanf(theta);
		} else {
			opp = tanf(theta); com = tanf(FLOATTYPE(QTAU) - theta);
		}

		FLOATTYPE pos = orgPos[X] - (FltVal * (FLOATTYPE)pQuad->Size - orgPos[Y]) * opp;
		opp *= (FLOATTYPE)-IntVal;

		for (Vect2 ofs;;) {
			ofs[X] = pos;
			ofs[Y] = FltVal * (FLOATTYPE)pQuad->Size;
			DbgPlot(ofs, 1, Color(255, 0,0,0));

			if (pos < 0.0f) {
				//return HitQ(nullptr, 0, Vect2());

				ofs[X] = 0.0f;
				ofs[Y] = FltVal * (FLOATTYPE)pQuad->Size + pos * com;
				DbgPlot(ofs, 2, Color(255, 0,0,255));

				Int2 tPos = pQuad->Pos;
				tPos[X]--;
				tPos[Y] += (int)ofs[Y];
				Quad *pNext = &QMap.FindCell(tPos);

				if (pNext->IsSolid()) {
					//return HitQ(nullptr, 0, Vect2());
					DbgPlot(ofs, 6, Color(255, 0,0,255));

					return HitQ(pNext, ofs[Y] * Flip[QuadNum][0], Vect2(pQuad->Pos) + ofs);
				}

				pos -= (FLOATTYPE)(pQuad->Size - pNext->Size) * opp;
				pos += (FLOATTYPE)pNext->Size;

				pQuad = pNext;
			} else if (pos >= (FLOATTYPE)pQuad->Size) {
				//return HitQ(nullptr, 0, Vect2());

				pos -= (FLOATTYPE)pQuad->Size;

				ofs[X] = (FLOATTYPE)pQuad->Size;
				ofs[Y] = FltVal * (FLOATTYPE)pQuad->Size + pos * com;
				DbgPlot(ofs, 2, Color(255, 0,191,0));

				Int2 tPos = pQuad->Pos;
				tPos[X] += pQuad->Size;
				tPos[Y] += (int)ofs[Y];
				Quad *pNext = &QMap.FindCell(tPos);

				if (pNext->IsSolid()) {
					//return HitQ(nullptr, 0, Vect2());
					DbgPlot(ofs, 6, Color(255, 0,191,0));

					return HitQ(pNext, ofs[Y] * Flip[QuadNum][1], Vect2(pQuad->Pos) + ofs);
				}

				pQuad = pNext;
			} else {
				ofs[X] = pos;
				ofs[Y] = FltVal * (FLOATTYPE)pQuad->Size;
				DbgPlot(ofs, 2.0f, Color(255, 255,0,0));

				Int2 tPos = pQuad->Pos;
				tPos[X] += (int)pos;
				tPos[Y] += (IntVal < 0) ? IntVal : (IntVal * pQuad->Size);
				Quad *pNext = &QMap.FindCell(tPos);

				if (pNext->IsSolid()) {
					//return HitQ(nullptr, 0, Vect2());
					DbgPlot(ofs, 6, Color(255, 255,0,0));

					return HitQ(pNext, ofs[X] * Flip[QuadNum][2], Vect2(pQuad->Pos) + ofs);
				}

				pos -= (FLOATTYPE)(pNext->Pos[X] - pQuad->Pos[X]);
				pos += (FLOATTYPE)pNext->Size * opp;

				pQuad = pNext;
			}
		}
	}
	*/
};