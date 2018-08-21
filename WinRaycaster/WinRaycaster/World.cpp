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
		
		if (game.pGfx) {
			Point Org((int)(DebugX + Cam.Pos.x * DebugScl), (int)(DebugY + Cam.Pos.y * DebugScl));
			Point Dst((int)(Org.X + sin(ang) * DebugScl*4), (int)(Org.Y - cos(ang) * DebugScl*4));
			game.pGfx->DrawLine(&Pen(Color(63, 0,0,255)), Org, Dst);
		}

		TraceHit hit = game.pMap->Trace(Cam.Pos, ang, game.pGfx);

		if (hit.pCell)
			game.pMap->Render2(Cam, hit, col, game.pGfx);

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


void Map::Scan(Camera &Cam, Graphics *pGfx) const {
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

		TraceHit hit = Trace(Cam.Pos, ang, pGfx);

		if (hit.pCell)
			Render(Cam, hit, col, pGfx);
	}
}


void Map::Render(Camera &Cam, const TraceHit &Hit, const int Col, Graphics *pGfx) const {
	Texture *pWall = Hit.pCell->pWallTex;

	const float dist = (Hit.Pos - Cam.Pos).Size();

	const float hei = (float)WallHeight / dist;
	const int half = (int)(hei / 2.0f);
	const int top = max(Cam.Hei / 2 - half, 0);
	const int btm = min(Cam.Hei / 2 + half, Cam.Hei);

	//const int mip = 0;
	//const int mip = min((int)dist, pWall->Mips);
	const float sqrDist = sqrtf(dist);
	const int mip = min((int)(sqrDist * MIP_BIAS), pWall->Mips);
	const int mipWid = pWall->Wid >> mip;
	const int mipHei = pWall->Hei >> mip;

	const float sclV = (float)mipHei / hei;

	if (pGfx) {
		Pen clrTrace(Color(255, 0,0, Col / 3));
		Point Org((int)(DebugX + Cam.Pos.x * DebugScl), (int)(DebugY + Cam.Pos.y * DebugScl));
		Point Dst((int)(DebugX + Hit.Pos.x * DebugScl), (int)(DebugY + Hit.Pos.y * DebugScl));

		pGfx->DrawLine(&clrTrace, Org, Dst);
	}

	const int texU = (int)(Hit.TexU * (float)mipWid) & (mipWid - 1);

	Pixel *pTex = pWall->pMip[mip] + texU;
	const int utop = Cam.Hei / 2 - half;

	const int mul = (int)(256.0f / max(sqrDist, 1.0f));

	int ofs = Col + top * Cam.Wid;
	for (int y = top; y < btm; y++, ofs += Cam.Wid) {
		const int texV = (int)((float)(y - utop) * sclV);
		assert(texV >= 0 && texV < mipHei);
		Cam.pImage[ofs] = pTex[texV * mipWid].Scale(mul);
	}
}


void Map::Render2(Camera &Cam, const TraceHit &Hit, const int Col, Graphics *pGfx) const {
	Texture *pWall = Hit.pCell->pWallTex;

	const float dist = (Hit.Pos - Cam.Pos).Size();

	const float hei = (float)WallHeight / dist;
	const int half = (int)(hei / 2.0f);
	const int top  = max(Cam.Hei / 2 - half, 0);
	const int btm  = min(Cam.Hei / 2 + half, Cam.Hei);

	const float sqrDist = sqrtf(dist);
	const int mip  = min((int)(sqrDist * MIP_BIAS), pWall->Mips);
	const int mip2 = min((int)(sqrDist * MIP_BIAS) - 1, pWall->Mips);
	const float mipa = sqrDist * MIP_BIAS - (float)mip;

	if (pGfx) {
		Pen clrTrace(Color(255, 0,0, Col / 3));
		Point Org((int)(DebugX + Cam.Pos.x * DebugScl), (int)(DebugY + Cam.Pos.y * DebugScl));
		Point Dst((int)(DebugX + Hit.Pos.x * DebugScl), (int)(DebugY + Hit.Pos.y * DebugScl));

		pGfx->DrawLine(&clrTrace, Org, Dst);
	}

	Vec2 texUV(Hit.TexU, 0);

	const float sclV = 1.0f / (float)hei;
	const int utop = Cam.Hei / 2 - half;
	const int mul = (int)(256.0f / max(sqrDist, 1.0f));

	int ofs = Col + top * Cam.Wid;
	for (int y = top; y < btm; y++, ofs += Cam.Wid) {
		texUV.y = (float)(y - utop) * sclV;
		
		Pixel p = pWall->Bilerp(texUV, mip).Scale(mul);
		
		if (mip && mip != mip2) {
			Pixel q = pWall->Bilerp(texUV, mip - 1).Scale(mul);
			
			p = Pixel::Lerp(q, p, mipa);
		}
		
		Cam.pImage[ofs] = p;
	}
}


TraceHit Map::Trace(const Vec2 &Origin, const float Theta, Graphics *pGfx) const {
	//float theta = fmod(Theta + TAU, TAU);
	//assert(theta >= 0.0f);

	//int quad = (int)((theta + ETAU) / QTAU /*+ FLT_EPSILON*/) & 3;
	//theta = fmodf(theta + ETAU, QTAU) - ETAU;
	
	int quad = (int)((Theta + ETAU) / QTAU) & 3;

	Int2 cellPos(Origin);
	Vec2 orgPos = Origin - Vec2(cellPos);

	if (CellPtr(cellPos)->IsSolid())
		return TraceHit(nullptr, 0, Vec2());

	//return (this->*TraceFuncs[quad])(cellPos, orgPos, theta, pGfx);
	return (this->*TraceFuncs[quad])(cellPos, orgPos, Theta, pGfx);
}


TraceHit Map::TraceQ0(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) const {
	//return TraceHit(nullptr, 0, Vec2());

	float opp = tanf(theta), com = tanf(QTAU - theta);
	float pos = orgPos.x + orgPos.y * opp;

	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot(0, (1.0f - pos) * -com, 2, Color(255, 0,0,255));

			cellPos.x--;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = (1.0f - pos) * -com;
				Plot(1, pos, 6, Color(255, 0,0,255));
				return TraceHit(CellPtr(cellPos), -pos, Vec2(cellPos) + Vec2(1, pos));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(1, pos * com, 2, Color(255, 0,191,0));

			cellPos.x++;
			if (CellPtr(cellPos)->IsSolid()) {
				pos *= com;
				Plot(0, pos, 6, Color(255, 0,191,0));
				return TraceHit(CellPtr(cellPos), pos, Vec2(cellPos) + Vec2(0, pos));
			}
		} else {
			Plot(pos, 0, 2, Color(255, 255,0,0));

			cellPos.y--;
			if (CellPtr(cellPos)->IsSolid()) {
				Plot(pos, 1, 6, Color(255, 255,0,0));
				return TraceHit(CellPtr(cellPos), pos, Vec2(cellPos) + Vec2(pos, 1));
			}

			pos += opp;
		}
	}
}


TraceHit Map::TraceQ1(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) const {
	//return TraceHit(nullptr, 0, Vec2());

	float opp = tanf(QTAU - theta), com = tanf(theta);
	float pos = orgPos.y - (1.0f - orgPos.x) * opp;

	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot(1.0f - (1.0f - pos) * com, 0, 2, Color(255, 0,0,255));

			cellPos.y--;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = 1.0f - (1.0f - pos) * com;
				Plot(pos, 1, 6, Color(255, 0,0,255));
				return TraceHit(CellPtr(cellPos), pos, Vec2(cellPos) + Vec2(pos, 1));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(1.0f - pos * -com, 1, 2, Color(255, 0,191,0));

			cellPos.y++;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = 1.0f - pos * -com;
				Plot(pos, 0, 6, Color(255, 0,191,0));
				return TraceHit(CellPtr(cellPos), -pos, Vec2(cellPos) + Vec2(pos, 0));
			}
		} else {
			Plot(1, pos, 2, Color(255, 255,0,0));

			cellPos.x++;
			if (CellPtr(cellPos)->IsSolid()) {
				Plot(0, pos, 6, Color(255, 255,0,0));
				return TraceHit(CellPtr(cellPos), pos, Vec2(cellPos) + Vec2(0, pos));
			}

			pos -= opp;
		}
	}
}


TraceHit Map::TraceQ2(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) const {
	//return TraceHit(nullptr, 0, Vec2());

	float opp = tanf(theta), com = tanf(QTAU - theta);
	float pos = orgPos.x - (1.0f - orgPos.y) * opp;
	
	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot(0, 1.0f - (1.0f - pos) * com, 2, Color(255, 0,0,255));

			cellPos.x--;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = 1.0f - (1.0f - pos) * com;
				Plot(1, pos, 6, Color(255, 0,0,255));
				return TraceHit(CellPtr(cellPos), -pos, Vec2(cellPos) + Vec2(1, pos));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(1, 1.0f + pos * com, 2, Color(255, 0,191,0));

			cellPos.x++;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = 1.0f + pos * com;
				Plot(0, pos, 6, Color(255, 0,191,0));
				return TraceHit(CellPtr(cellPos), pos, Vec2(cellPos) + Vec2(0, pos));
			}
		} else {
			Plot(pos, 1, 2, Color(255, 255,0,0));

			cellPos.y++;
			if (CellPtr(cellPos)->IsSolid()) {
				Plot(pos, 0, 6, Color(255, 255,0,0));
				return TraceHit(CellPtr(cellPos), -pos, Vec2(cellPos) + Vec2(pos, 0));
			}

			pos -= opp;
		}
	}
}


TraceHit Map::TraceQ3(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) const {
	//return TraceHit(nullptr, 0, Vec2());

	float opp = tanf(QTAU - theta), com = tanf(theta);
	float pos = orgPos.y + orgPos.x * opp;

	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot((1.0f - pos) * -com, 0, 2, Color(255, 0,0,255));

			cellPos.y--;
			if (CellPtr(cellPos)->IsSolid()) {
				pos = (1.0f - pos) * -com;
				Plot(pos, 1, 6, Color(255, 0,0,255));
				return TraceHit(CellPtr(cellPos), pos, Vec2(cellPos) + Vec2(pos, 1));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(pos * com, 1, 2, Color(255, 0,191,0));

			cellPos.y++;
			if (CellPtr(cellPos)->IsSolid()) {
				pos *= com;
				Plot(pos, 0, 6, Color(255, 0,191,0));
				return TraceHit(CellPtr(cellPos), -pos, Vec2(cellPos) + Vec2(pos, 0));
			}
		} else {
			Plot(0, pos, 2, Color(255, 255,0,0));

			cellPos.x--;
			if (CellPtr(cellPos)->IsSolid()) {
				Plot(1, pos, 6, Color(255, 255,0,0));
				return TraceHit(CellPtr(cellPos), -pos, Vec2(cellPos) + Vec2(1, pos));
			}

			pos += opp;
		}
	}
}