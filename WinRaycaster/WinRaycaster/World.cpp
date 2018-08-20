#include "stdafx.h"

#define MIP_BIAS	0.8f


void Map::Scan(Camera &Cam, Graphics *pGfx) {
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


void Map::Render(Camera &Cam, const Map::TraceHit &Hit, const int Col, Graphics *pGfx) {
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
	//assert(texU >= 0 && texU < mipWid);

	if (hei >= Cam.Hei * 0.5f) {
		// This glitch is caused by floating point rounding errors.
		int wtf = 0;
	}

	Pixel *pTex = pWall->pMip[mip] + texU;
	const int utop = Cam.Hei / 2 - half;

	const int mul = (int)(255.0f / sqrDist);

	int ofs = Col + top * Cam.Wid;
	for (int y = top; y < btm; y++, ofs += Cam.Wid) {
		const int texV = (int)((float)(y - utop) * sclV);
		assert(texV >= 0 && texV < mipHei);
		Cam.pImage[ofs] = pTex[texV * mipWid].Scale(mul);
	}
}


Map::TraceHit Map::Trace(const Vec2 &Origin, const float Theta, Graphics *pGfx) {
	float theta = fmod(Theta + TAU, TAU);
	assert(theta >= 0.0f);

	int quad = (int)((theta + ETAU) / QTAU /*+ FLT_EPSILON*/) & 3;
	theta = fmodf(theta + ETAU, QTAU) - ETAU;

	Int2 cellPos(Origin);
	Vec2 orgPos = Origin - Vec2(cellPos);

	if (GetCell(cellPos).IsSolid())
		return TraceHit(nullptr, 0, Vec2());

	return (this->*TraceFuncs[quad])(cellPos, orgPos, theta, pGfx);
}


Map::TraceHit Map::TraceQ0(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) {
	float opp = tanf(theta), com = tanf(QTAU - theta);
	float pos = orgPos.x + orgPos.y * opp;

	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot(0, (1.0f - pos) * -com, 2, Color(255, 0,0,255));

			cellPos.x--;
			if (GetCell(cellPos).IsSolid()) {
				pos = (1.0f - pos) * -com;
				Plot(1, pos, 6, Color(255, 0,0,255));
				return TraceHit(&GetCell(cellPos), -pos, Vec2(cellPos) + Vec2(1, pos));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(1, pos * com, 2, Color(255, 0,191,0));

			cellPos.x++;
			if (GetCell(cellPos).IsSolid()) {
				pos *= com;
				Plot(0, pos, 6, Color(255, 0,191,0));
				return TraceHit(&GetCell(cellPos), pos, Vec2(cellPos) + Vec2(0, pos));
			}
		} else {
			Plot(pos, 0, 2, Color(255, 255,0,0));

			cellPos.y--;
			if (GetCell(cellPos).IsSolid()) {
				Plot(pos, 1, 6, Color(255, 255,0,0));
				return TraceHit(&GetCell(cellPos), pos, Vec2(cellPos) + Vec2(pos, 1));
			}

			pos += opp;
		}
	}
}


Map::TraceHit Map::TraceQ1(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) {
	float opp = tanf(theta), com = tanf(QTAU - theta);
	float pos = orgPos.y + (1.0f - orgPos.x) * opp;

	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot(1.0f - (1.0f - pos) * -com, 0, 2, Color(255, 0,0,255));

			cellPos.y--;
			if (GetCell(cellPos).IsSolid()) {
				pos = 1.0f - (1.0f - pos) * -com;
				Plot(pos, 1, 6, Color(255, 0,0,255));
				return TraceHit(&GetCell(cellPos), pos, Vec2(cellPos) + Vec2(pos, 1));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(1.0f - pos * com, 1, 2, Color(255, 0,191,0));

			cellPos.y++;
			if (GetCell(cellPos).IsSolid()) {
				pos = 1.0f - pos * com;
				Plot(pos, 0, 6, Color(255, 0,191,0));
				return TraceHit(&GetCell(cellPos), -pos, Vec2(cellPos) + Vec2(pos, 0));
			}
		} else {
			Plot(1, pos, 2, Color(255, 255,0,0));

			cellPos.x++;
			if (GetCell(cellPos).IsSolid()) {
				Plot(0, pos, 6, Color(255, 255,0,0));
				return TraceHit(&GetCell(cellPos), pos, Vec2(cellPos) + Vec2(0, pos));
			}

			pos += opp;
		}
	}
}


Map::TraceHit Map::TraceQ2(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) {
	float opp = -tanf(theta), com = -tanf(QTAU - theta);
	float pos = orgPos.x + (1.0f - orgPos.y) * opp;
	
	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot(0, 1.0f - (1.0f - pos) * -com, 2, Color(255, 0,0,255));

			cellPos.x--;
			if (GetCell(cellPos).IsSolid()) {
				pos = 1.0f - (1.0f - pos) * -com;
				Plot(1, pos, 6, Color(255, 0,0,255));
				return TraceHit(&GetCell(cellPos), -pos, Vec2(cellPos) + Vec2(1, pos));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(1, 1.0f - pos * com, 2, Color(255, 0,191,0));

			cellPos.x++;
			if (GetCell(cellPos).IsSolid()) {
				pos = 1.0f - pos * com;
				Plot(0, pos, 6, Color(255, 0,191,0));
				return TraceHit(&GetCell(cellPos), pos, Vec2(cellPos) + Vec2(0, pos));
			}
		} else {
			Plot(pos, 1, 2, Color(255, 255,0,0));

			cellPos.y++;
			if (GetCell(cellPos).IsSolid()) {
				Plot(pos, 0, 6, Color(255, 255,0,0));
				return TraceHit(&GetCell(cellPos), -pos, Vec2(cellPos) + Vec2(pos, 0));
			}

			pos += opp;
		}
	}
}


Map::TraceHit Map::TraceQ3(Int2 &cellPos, Vec2 &orgPos, const float theta, Graphics *pGfx) {
	float opp = -tanf(theta), com = -tanf(QTAU - theta);
	float pos = orgPos.y + orgPos.x * opp;

	for (;;) {
		if (pos < 0.0f) {
			pos += 1.0f;
			Plot((1.0f - pos) * -com, 0, 2, Color(255, 0,0,255));

			cellPos.y--;
			if (GetCell(cellPos).IsSolid()) {
				pos = (1.0f - pos) * -com;
				Plot(pos, 1, 6, Color(255, 0,0,255));
				return TraceHit(&GetCell(cellPos), pos, Vec2(cellPos) + Vec2(pos, 1));
			}
		} else if (pos > 1.0f) {
			pos -= 1.0f;
			Plot(pos * com, 1, 2, Color(255, 0,191,0));

			cellPos.y++;
			if (GetCell(cellPos).IsSolid()) {
				pos *= com;
				Plot(pos, 0, 6, Color(255, 0,191,0));
				return TraceHit(&GetCell(cellPos), -pos, Vec2(cellPos) + Vec2(pos, 0));
			}
		} else {
			Plot(0, pos, 2, Color(255, 255,0,0));

			cellPos.x--;
			if (GetCell(cellPos).IsSolid()) {
				Plot(1, pos, 6, Color(255, 255,0,0));
				return TraceHit(&GetCell(cellPos), -pos, Vec2(cellPos) + Vec2(1, pos));
			}

			pos += opp;
		}
	}
}