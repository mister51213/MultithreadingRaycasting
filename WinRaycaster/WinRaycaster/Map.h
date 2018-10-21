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
	Type		*pCell;
	FLOATTYPE	TexU;
	FLOATTYPE	TexV;
	Vect3		Pos;

	TraceHit(Type *pCell, const FLOATTYPE TexU, const FLOATTYPE TexV, const Vect3 &Pos) :
		pCell(pCell), TexU(TexU), TexV(0.5f), Pos(Pos) {}
};

typedef TraceHit<Cell> Hit;
//typedef TraceHit<Quad> HitQ;


class GameMap {
protected:
	typedef Hit (GameMap::*TraceFunc)(Int3 cellPos, Vect3 orgPos, const FLOATTYPE theta, Graphics *pGfx) const;
	const TraceFunc TraceFuncs[4] = {&GameMap::TraceQ<0>, &GameMap::TraceQ<1>, &GameMap::TraceQ<2>, &GameMap::TraceQ<3>};
	//typedef HitQ (GameMap::*TraceFunc)(const Vect2 &Origin, const FLOATTYPE theta, Graphics *pGfx);
	//TraceFunc TraceFuncs[4] = {&GameMap::TraceQQ<0>, &GameMap::TraceQQ<1>, &GameMap::TraceQQ<2>, &GameMap::TraceQQ<3>};

	vector<Cell>	Map;
	//Quad			QMap;

public:
	int		Size;
	int		WallHeight;

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
			} else if (pos > 1.0f) {
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