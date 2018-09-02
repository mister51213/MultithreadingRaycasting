#include "stdafx.h"


void Thread::ThreadFunc(void *pThis) {
	Thread &thread = *(Thread*)pThis;

	extern Game game;

	for (;;) {
		WaitForSingleObject(thread.hRun, INFINITE);
		if (thread.Abort)
			break;

		Camera &Cam = *game.pCam;
		const float slice = Cam.FoV / (float)Cam.Wid;
		const int halfwid = Cam.Wid / 2;

		EnterCriticalSection(&game.CS);
GetAnother:

		int col = game.ThreadCol;
		game.ThreadCol++;

		LeaveCriticalSection(&game.CS);

		if (col >= game.pCam->Wid)
			continue;

		float ang = (float)(col - halfwid) * slice + Cam.Dir;
		
		/*if (game.pGfx) {
			Point Org((int)(DebugX + Cam.Pos.x * DebugScl), (int)(DebugY + Cam.Pos.y * DebugScl));
			Point Dst((int)(Org.X + sin(ang) * DebugScl*4), (int)(Org.Y - cos(ang) * DebugScl*4));
			game.pGfx->DrawLine(&Pen(Color(63, 0,0,255)), Org, Dst);
		}*/

		// Type should be a templated map type
		auto hit = game.pMap->Trace(Cam.Pos, ang, nullptr);
		//auto hit = game.pMap->Trace(Cam.Pos, ang, game.pGfx);

		if (hit.pCell)
			game.pMap->Render2(Cam, hit, col, nullptr);
			//game.pMap->Render2(Cam, hit, col, game.pGfx);

		EnterCriticalSection(&game.CS);

		game.DoneCol++;

		if (game.DoneCol >= game.pCam->Wid) {
			// Last job is done! This is the last worker currently awake.
			SetEvent(game.hDone);
		} else
			goto GetAnother;

		LeaveCriticalSection(&game.CS);
	}

	SetEvent(thread.hStop);
}

//template <typename Type>
/*void Map::Scan(Camera &Cam, Graphics *pGfx) const {
	const float slice = Cam.FoV / (float)Cam.Wid;
	const int halfwid = Cam.Wid / 2;

	Cam.Clear();

	for (int col = 0; col < Cam.Wid; col++) {
		//int col = halfwid; {
		//int col = 1088; {

		float ang = (float)(col - halfwid) * slice + Cam.Dir;

		if (pGfx) {
			Point Org((int)(DebugX + Cam.Pos.x * DebugScl), (int)(DebugY + Cam.Pos.y * DebugScl));
			Point Dst((int)(Org.X + sin(ang) * DebugScl*4), (int)(Org.Y - cos(ang) * DebugScl*4));
			pGfx->DrawLine(&Pen(Color(63, 0,0,255)), Org, Dst);
		}

		//TraceHit hit = Trace(Cam.Pos, ang, pGfx);
		Hit hit = Trace(Cam.Pos, ang, pGfx);

		if (hit.pCell)
			Render(Cam, hit, col, pGfx);
	}
}*/


//void Map::Render(Camera &Cam, const TraceHit &Hit, const int Col, Graphics *pGfx) const {
//	Texture *pWall = Hit.pCell->pWallTex;
//
//	const float dist = (Hit.Pos - Cam.Pos).Size();
//
//	const float hei = (float)WallHeight / dist;
//	const int half = (int)(hei / 2.0f);
//	const int top = max(Cam.Hei / 2 - half, 0);
//	const int btm = min(Cam.Hei / 2 + half, Cam.Hei);
//
//	//const int mip = 0;
//	//const int mip = min((int)dist, pWall->Mips);
//	const float sqrDist = sqrtf(dist);
//	const int mip = min((int)(sqrDist * MIP_BIAS), pWall->Mips);
//	const int mipWid = pWall->Wid >> mip;
//	const int mipHei = pWall->Hei >> mip;
//
//	const float sclV = (float)mipHei / hei;
//
//	if (pGfx) {
//		Pen clrTrace(Color(255, 0,0, Col / 3));
//		Point Org((int)(DebugX + Cam.Pos.x * DebugScl), (int)(DebugY + Cam.Pos.y * DebugScl));
//		Point Dst((int)(DebugX + Hit.Pos.x * DebugScl), (int)(DebugY + Hit.Pos.y * DebugScl));
//
//		pGfx->DrawLine(&clrTrace, Org, Dst);
//	}
//
//	const int texU = (int)(Hit.TexU * (float)mipWid) & (mipWid - 1);
//
//	Pixel *pTex = pWall->pMip[mip] + texU;
//	const int utop = Cam.Hei / 2 - half;
//
//	const int mul = (int)(256.0f / max(sqrDist, 1.0f));
//
//	int ofs = Col + top * Cam.Wid;
//	for (int y = top; y < btm; y++, ofs += Cam.Wid) {
//		const int texV = (int)((float)(y - utop) * sclV);
//		assert(texV >= 0 && texV < mipHei);
//		Cam.pImage[ofs] = pTex[texV * mipWid].Scale(mul);
//	}
//}


//template <typename Type>
void Map::Render2(Camera &Cam, const Hit &hit, const int Col, Graphics *pGfx) const {
	Texture *pWall = hit.pCell->pWallTex;

	const float dist = (hit.Pos - Cam.Pos).Size();

	const float hei = (float)WallHeight / dist;
	const int half = (int)(hei / 2.0f);
	const int top  = max(Cam.Hei / 2 - half, 0);
	const int btm  = min(Cam.Hei / 2 + half, Cam.Hei);

	const float sqrDist = sqrtf(dist);
	const int mip  = min((int)(sqrDist * MIP_BIAS), pWall->Mips);
	const int mip2 = min((int)(sqrDist * MIP_BIAS) - 1, pWall->Mips);
	const FLOATTYPE mipa = sqrDist * MIP_BIAS - (FLOATTYPE)mip;

	if (pGfx) {
		Pen clrTrace(Color(255, 0,0, Col / 3));
		Point Org((int)(DebugX + Cam.Pos.x * DebugScl), (int)(DebugY + Cam.Pos.y * DebugScl));
		Point Dst((int)(DebugX + (float)hit.Pos.x * DebugScl), (int)(DebugY + (float)hit.Pos.y * DebugScl));

		pGfx->DrawLine(&clrTrace, Org, Dst);
	}

	//typedef FixedFloat<24, long, long long> TexFlt;
	typedef FLOATTYPE TexFlt;
	typedef TempVect2D<TexFlt> TexVec;

	TexVec texUV(hit.TexU, 0);

	const TexFlt sclV = 1.0f / (float)hei;
	const int utop = Cam.Hei / 2 - half;
	const int mul = (int)(256.0f / max(sqrDist, 1.0f));

	texUV.y = (TexFlt)(top - utop) * sclV;

	int ofs = Col + top * Cam.Wid;
	for (int y = top; y < btm; y++, ofs += Cam.Wid) {
		//texUV.y = (FLOATTYPE)(y - utop) * sclV;
		texUV.y += sclV;

		Pixel p = pWall->Bilerp(texUV, mip).Scale(mul);
		
		if (mip && mip != mip2) {
			Pixel q = pWall->Bilerp(texUV, mip2).Scale(mul);
			
			p = Pixel::FLerp(q, p, mipa);
		}
		
		Cam.pImage[ofs] = p;
	}
}

//template<typename FLOATTYPE>
Hit Map::Trace(const Vect &Origin, const FLOATTYPE Theta, Graphics *pGfx) const {
	//FLOATTYPE theta = fmod(Theta + TAU, TAU);
	//assert(theta >= 0.0f);

	//int quad = (int)((theta + ETAU) / QTAU /*+ FLT_EPSILON*/) & 3;
	//theta = fmodf(theta + ETAU, QTAU) - ETAU;
	
	int quad = (int)((Theta + FLOATTYPE(ETAU)) / FLOATTYPE(QTAU)) & 3;

	Int2 cellPos(Origin);
	Vect orgPos = Origin - Vect(cellPos);

	if (CellPtr(cellPos)->IsSolid())
		return Hit(nullptr, 0, Vect());

	//return TraceQ(cellPos, orgPos, Theta, pGfx, quad);
	//return TraceQ(quad, cellPos, orgPos, Theta, pGfx);

	//return (this->*TraceFuncs[quad])(cellPos, orgPos, theta, pGfx);
	return (this->*TraceFuncs[quad])(cellPos, orgPos, Theta, pGfx);
}

/*
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


Hit Map::TraceQ(const int QuadNum, Int2 cellPos, Vect orgPos, const FLOATTYPE theta, Graphics *pGfx) const {
	//return Hit(nullptr, 0, Vect());

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

	for (Vect ofs;;) {
		if (pos < 0.0f) {
			pos += 1.0f;

			ofs[X] = 0.0f;
			ofs[Y] = FltVal - (1.0f - pos) * com;
			Plot(ofs[X], ofs[Y], 2, Color(255, 0,0,255));

			cellPos[X]--;
			if (CellPtr(cellPos)->IsSolid()) {
				//return Hit(nullptr, 0, Vect());
				pos = FltVal - (1.0f - pos) * com;

				ofs[X] = 1.0f;
				ofs[Y] = pos;
				Plot(ofs[X], ofs[Y], 6, Color(255, 0,0,255));

				return Hit(CellPtr(cellPos), pos * Flip[QuadNum][0], Vect(cellPos) + ofs);
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;

			ofs[X] = 1.0f;
			ofs[Y] = FltVal + pos * com;
			Plot(ofs[X], ofs[Y], 2, Color(255, 0,191,0));

			cellPos[X]++;
			if (CellPtr(cellPos)->IsSolid()) {
				//return Hit(nullptr, 0, Vect());
				pos = FltVal + pos * com;

				ofs[X] = 0.0f;
				ofs[Y] = pos;
				Plot(ofs[X], ofs[Y], 6, Color(255, 0,191,0));

				return Hit(CellPtr(cellPos), pos * Flip[QuadNum][1], Vect(cellPos) + ofs);
			}
		} else {
			ofs[X] = pos;
			ofs[Y] = FltVal;
			Plot(ofs[X], ofs[Y], 2.0f, Color(255, 255,0,0));

			cellPos[Y] += IntVal;
			if (CellPtr(cellPos)->IsSolid()) {
				//return Hit(nullptr, 0, Vect());
				
				ofs[X] = pos;
				ofs[Y] = 1.0f - FltVal;
				Plot(ofs[X], ofs[Y], 6, Color(255, 255,0,0));

				return Hit(CellPtr(cellPos), pos * Flip[QuadNum][2], Vect(cellPos) + ofs);
			}

			pos += opp;
		}
	}
}
*/

/*
//template<typename FLOATTYPE>
Hit Map::TraceQ0(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx) const {
	//return Hit(nullptr, 0, Vect());

	FLOATTYPE opp = tanf(theta), com = tanf(FLOATTYPE(QTAU) - theta);
	FLOATTYPE pos = orgPos.x + orgPos.y * opp;

	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot(0, (1.0f - pos) * -com, 2, Color(255, 0,0,255));

			cellPos.x--;
			if (CellPtr(cellPos)->IsSolid()) {
				//return Hit(nullptr, 0, Vect());
				pos = (1.0f - pos) * -com;
				Plot(1, pos, 6, Color(255, 0,0,255));
				return Hit(CellPtr(cellPos), -pos, Vect(cellPos) + Vect(1, pos));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(1, pos * com, 2, Color(255, 0,191,0));

			cellPos.x++;
			if (CellPtr(cellPos)->IsSolid()) {
				pos *= com;
				Plot(0, pos, 6, Color(255, 0,191,0));
				return Hit(CellPtr(cellPos), pos, Vect(cellPos) + Vect(0, pos));
			}
		} else {
			Plot(pos, 0, 2, Color(255, 255,0,0));

			cellPos.y--;
			if (CellPtr(cellPos)->IsSolid()) {
				Plot(pos, 1, 6, Color(255, 255,0,0));
				return Hit(CellPtr(cellPos), pos, Vect(cellPos) + Vect(pos, 1));
			}

			pos += opp;
		}
	}
}


//template<typename FLOATTYPE>
Hit Map::TraceQ1(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx) const {
	//return Hit(nullptr, 0, Vect());

	FLOATTYPE opp = tanf(FLOATTYPE(QTAU) - theta), com = tanf(theta);
	FLOATTYPE pos = orgPos.y - (1.0f - orgPos.x) * opp;

	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot(1.0f - (1.0f - pos) * com, 0, 2, Color(255, 0,0,255));

			cellPos.y--;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = 1.0f - (1.0f - pos) * com;
				Plot(pos, 1, 6, Color(255, 0,0,255));
				return Hit(CellPtr(cellPos), pos, Vect(cellPos) + Vect(pos, 1));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(1.0f - pos * -com, 1, 2, Color(255, 0,191,0));

			cellPos.y++;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = 1.0f - pos * -com;
				Plot(pos, 0, 6, Color(255, 0,191,0));
				return Hit(CellPtr(cellPos), -pos, Vect(cellPos) + Vect(pos, 0));
			}
		} else {
			Plot(1, pos, 2, Color(255, 255,0,0));

			cellPos.x++;
			if (CellPtr(cellPos)->IsSolid()) {
				Plot(0, pos, 6, Color(255, 255,0,0));
				return Hit(CellPtr(cellPos), pos, Vect(cellPos) + Vect(0, pos));
			}

			pos -= opp;
		}
	}
}

//template<typename FLOATTYPE>
typename Hit Map::TraceQ2(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx) const {
	//return Hit(nullptr, 0, Vect());

	FLOATTYPE opp = tanf(theta), com = tanf(FLOATTYPE(QTAU) - theta);
	FLOATTYPE pos = orgPos.x - (1.0f - orgPos.y) * opp;
	
	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot(0.0f, 1.0f - (1.0f - pos) * com, 2, Color(255, 0,0,255));

			cellPos.x--;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = 1.0f - (1.0f - pos) * com;
				Plot(1, pos, 6, Color(255, 0,0,255));
				return Hit(CellPtr(cellPos), -pos, Vect(cellPos) + Vect(1, pos));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(1.0f, 1.0f + pos * com, 2, Color(255, 0,191,0));

			cellPos.x++;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = 1.0f + pos * com;
				Plot(0, pos, 6, Color(255, 0,191,0));
				return Hit(CellPtr(cellPos), pos, Vect(cellPos) + Vect(0, pos));
			}
		} else {
			Plot(pos, 1.0f, 2.0f, Color(255, 255,0,0));

			cellPos.y += 1;
			if (CellPtr(cellPos)->IsSolid()) {
				Plot(pos, 0, 6, Color(255, 255,0,0));
				return Hit(CellPtr(cellPos), -pos, Vect(cellPos) + Vect(pos, 0));
			}

			pos -= opp;
		}
	}
}


//template<typename FLOATTYPE>
Hit Map::TraceQ3(Int2 &cellPos, Vect &orgPos, const FLOATTYPE theta, Graphics *pGfx) const {
	//return Hit(nullptr, 0, Vect());

	FLOATTYPE opp = tanf(FLOATTYPE(QTAU) - theta), com = tanf(theta);
	FLOATTYPE pos = orgPos.y + orgPos.x * opp;

	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot((1.0f - pos) * -com, 0, 2, Color(255, 0,0,255));

			cellPos.y--;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = (1.0f - pos) * -com;
				Plot(pos, 1, 6, Color(255, 0,0,255));
				return Hit(CellPtr(cellPos), pos, Vect(cellPos) + Vect(pos, 1));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(pos * com, 1, 2, Color(255, 0,191,0));

			cellPos.y++;
			if (CellPtr(cellPos)->IsSolid()) {
				pos *= com;
				Plot(pos, 0, 6, Color(255, 0,191,0));
				return Hit(CellPtr(cellPos), -pos, Vect(cellPos) + Vect(pos, 0));
			}
		} else {
			Plot(0, pos, 2, Color(255, 255,0,0));

			cellPos.x--;
			if (CellPtr(cellPos)->IsSolid()) {
				Plot(1, pos, 6, Color(255, 255,0,0));
				return Hit(CellPtr(cellPos), -pos, Vect(cellPos) + Vect(1, pos));
			}

			pos += opp;
		}
	}
}
*/