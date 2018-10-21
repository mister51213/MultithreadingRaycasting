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
		const int halfwid = Cam.Wid / 2;
		const int halfhei = Cam.Hei / 2;

		EnterCriticalSection(&game.CS);
GetAnother:

		int row = game.ThreadRow;
		//game.ThreadRow++;
		game.ThreadRow += ROWS;

		LeaveCriticalSection(&game.CS);

		if (row >= game.pCam->Wid)
			continue;

		for (int end = row + ROWS; row < end; row++) {
			float angV = (float)(row - halfhei) * slice + Cam.Dir.y;

			for (int ofs = row * Cam.Wid, col = 0; col < Cam.Wid; col++, ofs++) {
				float angU = (float)(col - halfwid) * slice + Cam.Dir.x;

				//Vect3 camPos = Cam.Pos;
				//camPos.z += row / (float)Cam.Hei;

				if (game.pGfx) {
					Point Org((int)(DebugX + Cam.Pos.x * DebugScl    ), (int)(DebugY + Cam.Pos.y * DebugScl    ));
					Point Dst((int)(Org.X  + sin(angU) * DebugScl * 4), (int)(Org.Y  - cos(angU) * DebugScl * 4));
					game.pGfx->DrawLine(&Pen(Color(63, 0,0,255)), Org, Dst);
				}

				// Type should be a templated Map type
				auto hit = game.pMap->Trace(Cam.Pos, angU, game.pGfx);

				//hit.TexV = row / (float)Cam.Hei;

				if (hit.pCell) {
					hit.Pos.z = Cam.Pos.z;

					const float dist = sqrtf((hit.Pos - Cam.Pos).Size()) / 10.0f;
					
					hit.TexV = 0.5f + tanf(angV) * dist;
					if (hit.TexV < 0.0f || hit.TexV >= 1.0f)
						continue;

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