#include "stdafx.h"


void GameMap::Debug(Graphics *pGfx) {
	SolidBrush clrVoid(Color(255,255,255));
	SolidBrush clrWall(Color(191,255,191));
	Pen clrGrid(Color(191,191,191));

	for (int y = 0; y < Size; y++)
		for (int x = 0; x < Size; x++) {
			pGfx->FillRectangle(
				GetCell(Int3(x, y, 1)).IsSolid() ? &clrWall : &clrVoid,
				DebugX + x * DebugScl, DebugY + y * DebugScl, 
				DebugScl, DebugScl);

			if (!GetCell(Int3(x, y, 1)).IsSolid())
				pGfx->DrawRectangle(&clrGrid,
									DebugX + x * DebugScl, DebugY + y * DebugScl, 
									DebugScl-1, DebugScl-1);
		}
}

/*
void GameMap::DebugQuad(Quad &Q, Graphics *pGfx) {
	bool done = false;

	for (int q = 0; q < 4; q++)
		if (Q.pChild[q]) {
			DebugQuad(*Q.pChild[q], pGfx);
			done = true;
		}

	if (done)
		return;

	SolidBrush clrVoid(Color(255,255,255));
	SolidBrush clrWall(Color(191,255,191));
	Pen clrGrid(Color(191,191,191));

	pGfx->FillRectangle(
		Q.IsSolid() ? &clrWall : &clrVoid,
		DebugX + Q.Pos.x * DebugScl, 
		DebugY + Q.Pos.y * DebugScl, 
		Q.Size * DebugScl, 
		Q.Size * DebugScl);

	if (!Q.IsSolid())
		pGfx->DrawRectangle(
			&clrGrid,
			DebugX + Q.Pos.x * DebugScl, 
			DebugY + Q.Pos.y * DebugScl, 
			Q.Size * DebugScl - 1, 
			Q.Size * DebugScl - 1);
}
*/

void GameMap::Render(Camera &Cam, const Hit &hit, const int ofs, Graphics *pGfx) const {
	float *pPixel = Cam.pFLOATS + ofs * 4;
	pPixel[0] = hit.TexU;
	pPixel[1] = hit.TexV;
	pPixel[2] = (hit.Pos - Cam.Pos).SizeSquared(); // distance - sqrted inside shader
	pPixel[3] = 1.0f;
	return;

	// LEGACY
	////Texture *pWall = hit.pCell->pWallTex;
	//Int2 texUV(hit.TexU * pWall->Wid, hit.TexV * pWall->Hei);
	//texUV.x &= pWall->Wid - 1; texUV.y &= pWall->Hei - 1;	// HACK TO WRAP TEXTURES

	//Pixel p = pWall->Sample(texUV, 0);

	////int ofs = ScrPos.x + ScrPos.y * Cam.Wid;
	//
	//const float sqrDist = (hit.Pos - Cam.Pos).Size();
	//const int mul = (int)(256.0f / max(sqrDist, 1.0f));

	//Cam.pImage[ofs] = p.Scale(mul);
	////Cam.pImage[ofs] = 0xFF000000 | (texUV.x << 16) | (texUV.y << 8) | mul;

}

/*void GameMap::Render(Camera &Cam, const Hit &hit, const int Col, Graphics *pGfx) const {
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
		Pen clrTrace(Color(255, 0, 0, Col / 3));
		Point Org((int)(DebugX + Cam.Pos.x * DebugScl), (int)(DebugY + Cam.Pos.y * DebugScl));
		Point Dst((int)(DebugX + (float)hit.Pos.x * DebugScl), (int)(DebugY + (float)hit.Pos.y * DebugScl));

		pGfx->DrawLine(&clrTrace, Org, Dst);
	}

	//typedef FixedFloat<24, long, long long> TexFlt;
	typedef FLOATTYPE TexFlt;
	typedef TempVect2D<TexFlt> TexVec;

	TexFlt    sclV = 1.0f / (float)hei;
	const int utop = Cam.Hei / 2 - half;
	const int mul = (int)(256.0f / max(sqrDist, 1.0f));

#ifdef BILERP

	TexVec texUV(
		hit.TexU, 
		(TexFlt)(top - utop) * sclV
	);

#else

	sclV *= (TexFlt)MipDim(mip, pWall->Wid);

	TexVec texUV(
		hit.TexU * (TexFlt)MipDim(mip, pWall->Wid), 
		(TexFlt)(top - utop) * sclV);

#endif

	int ofs = Col + top * Cam.Wid;
	//int ofs = Col * Cam.Hei + top;
	for (int y = top; y < btm; y++, ofs += Cam.Wid) {
	//for (int y = top; y < btm; y++, ofs++) {
		texUV.y += sclV;

#ifdef BILERP
		Pixel p = pWall->Bilerp(texUV, mip);//.Scale(mul);

#ifdef TRILERP
		if (mip && mip != mip2) {
			Pixel q = pWall->Bilerp(texUV, mip2);//.Scale(mul);

			p = Pixel::FLerp(q, p, mipa);
		}
#endif	// TRILERP

#else	// BILERP
		Pixel p = pWall->Sample(texUV, mip);//.Scale(mul);
#endif	// BILERP

#ifdef DARKEN
		Cam.pImage[ofs] = p.Scale(mul);
#else	// DARKEN
		Cam.pImage[ofs] = p;
#endif	// DARKEN

	}
}*/


/*HitQ*/ Hit GameMap::Trace(const Vect3 &Origin, const FLOATTYPE Theta, Graphics *pGfx) {
	const int quad = (int)((Theta + FLOATTYPE(ETAU)) / FLOATTYPE(QTAU)) & 3;

	Int3 cellPos(Origin);
	Vect3 orgPos = Origin - Vect3(cellPos);

	if (CellPtr(cellPos)->IsSolid())
		return Hit(nullptr, 0, 0, Vect3());

	return (this->*TraceFuncs[quad])(cellPos, orgPos, Theta, pGfx);
	//return (this->*TraceFuncs[quad])(Origin, Theta, pGfx);
}

TraceHit<Cell> Cell::Query(Vect3 entryPos, Int3 lastPos, Int3 cellPos, Vect3 step) {
	// case when it's just a solid wall
	if (pWallTex) {
		lastPos -= cellPos;

		Vect2 texUV(0, 0);
		if (lastPos.x)
			texUV = Vect2(entryPos.y - (float)cellPos.y, entryPos.z - (float)cellPos.z);
		else if (lastPos.y)
			texUV = Vect2(entryPos.x - (float)cellPos.x, entryPos.z - (float)cellPos.z);
		else
			texUV = Vect2(entryPos.x - (float)cellPos.x, entryPos.y - (float)cellPos.y);

		return TraceHit<Cell>(this, texUV.x, texUV.y, entryPos);

	}
	else {
	// OTHERWISE - keep stepping until we find an inner shape

	// Pick the closest shape to the trace (bad algorithm!
	for (Shape& shape : innerShapes) {
			Vect2 uv;
			Vect3 hitPos;

			for (;; entryPos += step) {
				for (Shape& shape : innerShapes) {
					if (shape.Query(entryPos, step, uv, hitPos))
						return TraceHit<Cell>(this, uv.x, uv.y, hitPos);
				}
			}
		}
	}
}