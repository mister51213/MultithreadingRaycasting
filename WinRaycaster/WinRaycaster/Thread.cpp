#include "stdafx.h"


#define ROWS	1


void Thread::ThreadFunc(void *pThis) {
	Thread &thread = *(Thread*)pThis;

	extern Game game;

	for (;;) {
		WaitForSingleObject(thread.hRun, INFINITE);
		if (thread.Abort)
			break;

		Camera &Cam = *game.pCam;
		const float slice = Cam.FoV / (float)Cam.Wid;
		const int halfWid = Cam.Wid / 2;
		const int halfHei = Cam.Hei / 2;

		EnterCriticalSection(&game.CS);
GetAnother:

		int row = game.ThreadRow;
		//game.ThreadRow++;
		game.ThreadRow += ROWS;

		LeaveCriticalSection(&game.CS);

		if (row >= game.pCam->Hei)
			continue;

		XMMATRIX matView = XMMatrixRotationRollPitchYaw(Cam.Dir.y, Cam.Dir.x, 0);

		for (int end = row + ROWS; row < end; row++) {
			//float angV = (float)(row - halfHei) * slice /*+ PI / 2.0f;*/ + Cam.Dir.y;
			float angV = (float)(row - halfHei) * slice + PI / 2.0f;

			for (int ofs = row * Cam.Wid, col = 0; col < Cam.Wid; col++, ofs++) {
				//float angU = (float)(col - halfWid) * slice + Cam.Dir.x;
				float angU = (float)(col - halfWid) * slice;

				//XMMATRIX matView = XMMatrixRotationRollPitchYaw(angV, angU, 0);

				XMVECTOR vecStep = XMVectorSet(
					sinf(angV) * sinf(angU),
					sinf(angV) * cosf(angU),
					cosf(angV),
					0.0f
				) * 0.025f;

				//XMVECTOR vecStep = XMVectorSet(1.0f, 0, 0, 0);

				vecStep = XMVector3Transform(vecStep, matView);
				const Vect3 step(XMVectorGetX(vecStep), XMVectorGetZ(vecStep), XMVectorGetY(vecStep));

				/*Vect3 step(
					sinf(angV) * sinf(angU),
					sinf(angV) * cosf(angU),
					-cosf(angV)
				);

				_ASSERTE(abs(step.Size() - 1.0f) < 0.0001f);


				step *= 0.01f;*/

				//Vect3 camPos = Cam.Pos;
				//camPos.z += row / (float)Cam.Hei;

				if (game.pGfx) {
					Point Org((int)(DebugX + Cam.Pos.x * DebugScl    ), (int)(DebugY + Cam.Pos.y * DebugScl    ));
					//Point Dst((int)(Org.X  + sin(angU) * DebugScl * 4), (int)(Org.Y  - cos(angU) * DebugScl * 4));
					Point Dst = Point(Org.X + step.x * DebugScl * 4.0f, Org.Y + step.z * DebugScl * 4.0f);
					game.pGfx->DrawLine(&Pen(Color(63, 0,0,255)), Org, Dst);
				}

				// Type should be a templated Map type
				//auto hit = game.pMap->Trace(Cam.Pos, angU, game.pGfx);
				auto hit = game.pMap->TraceS(Cam.Pos, step /*Vect2(angU, angV)*/, game.pGfx);
				
				//auto hit = game.pMap->Trace3D(Cam.Pos, Vect2(angU, angV), game.pGfx);
				//auto hit = game.pMap->Trace3D(Cam.Pos, step, game.pGfx);

				//hit.TexV = row / (float)Cam.Hei;

				if (hit.pCell) {
					/*hit.Pos.z = Cam.Pos.z;

					const float dist = sqrtf((hit.Pos - Cam.Pos).Size()) / 10.0f;
					
					hit.TexV = 0.5f + tanf(angV) * dist;
					if (hit.TexV < 0.0f || hit.TexV >= 1.0f)
						continue;
					*/
					game.pMap->Render(Cam, hit, ofs, nullptr);
				}
			}
		}

		EnterCriticalSection(&game.CS);

		//game.DoneRow++;
		game.DoneRow += ROWS;

		if (game.DoneRow >= game.pCam->Hei) {
			// Last job is done! This is the last worker currently awake.
			SetEvent(game.hDone);
		} else
			goto GetAnother;

		LeaveCriticalSection(&game.CS);
	}

	SetEvent(thread.hStop);
}