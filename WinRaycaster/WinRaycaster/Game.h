#pragma once


class Game {
public:
	Critical		CS;
	HANDLE			hDone;
	int				ThreadRow;
	int				DoneRow;

	Graphics		*pGfx = nullptr;
	Camera			*pCam = nullptr;
	GameMap			*pMap = nullptr;

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
		//pCam->Pos = Vec2(3, 5);
		pCam->Dir = Vec2(0.0f * Ang2Rad, 90.0f * Ang2Rad);
		pCam->FoV = 90.0f * Ang2Rad;

		Textures.push_back(Texture(L"textures\\wall1.png"));
		//Textures.push_back(Texture(L"textures\\align.png"));
		//Textures.push_back(Texture(L"textures\\red-brick-wall-living-room-india-effect-ireland-kitchen-shaped-good-looking-uv-h-cm-jpg-iida-comp-pinterest.jpg"));

		Texture &tex = Textures[0];
		DX.NewTexture(tex.pMip[0], tex.Wid, tex.Hei);
		//DX.BindTexture(pTex);

		InitMap();

		for (int i = 0; i < THREADCOUNT; i++)
			Threads.push_back(new Thread(&CS));
	}

	void InitMap() {
		const int Size = 11, s = Size - 1;

		pMap = new GameMap(Size, SCRN_H);

		const Pixel R(0xFFFF0000);
		const Pixel G(0xFF00FF00);
		const Pixel B(0xFF00007F);

		for (int i = 0; i < Size * Size * Size; i++)
			pMap->Map[i].pWallTex = &Textures[0];

		for (int z = 1; z < s; z++)
			for (int y = 1; y < s; y++)
				for (int x = 1; x < s; x++)
					pMap->GetCell(Int3(x, y, z)).pWallTex = nullptr;

		for (int z = 1; z < s; z++) {
			if (z == 2) continue;

			pMap->GetCell(Int3(3, 3, z)).pWallTex = &Textures[0];
			pMap->GetCell(Int3(7, 3, z)).pWallTex = &Textures[0];
			pMap->GetCell(Int3(3, 7, z)).pWallTex = &Textures[0];
			pMap->GetCell(Int3(7, 7, z)).pWallTex = &Textures[0];
		}

		/*pMap->Root().pWall = &Textures[0];
		pMap->Root().Divide();
		pMap->Root()[qTL]->Divide()[qBR]->pWall = nullptr;
		pMap->Root()[qTR]->Divide()[qBL]->pWall = nullptr;
		pMap->Root()[qBL]->Divide()[qTR]->pWall = nullptr;
		pMap->Root()[qBR]->Divide()[qTL]->pWall = nullptr;

		pMap->Root()(qTL)(qTR).Divide()(qBL).pWall = nullptr;
		pMap->Root()(qTR)(qTL).Divide()(qBR).pWall = nullptr;
		pMap->Root()(qTR)(qBR).Divide()(qTL).pWall = nullptr;
		pMap->Root()(qBR)(qTR).Divide()(qBL).pWall = nullptr;
		pMap->Root()(qBR)(qBL).Divide()(qTR).pWall = nullptr;
		pMap->Root()(qBL)(qBR).Divide()(qTL).pWall = nullptr;
		pMap->Root()(qBL)(qTL).Divide()(qBR).pWall = nullptr;
		pMap->Root()(qTL)(qBL).Divide()(qTR).pWall = nullptr;*/
	}

	void Tick(float deltaTime, HWND hWnd) {
		if (!pCam)
			return;

		//pCam->Dir = 45.5f * Ang2Rad;

		pCam->Pos = Vec3(5.5, 5.5, 5.5f) + Vec3(sinf(-pCam->Dir.x * 1.5f), cosf(-pCam->Dir.x) * 1.0f, 0);
		//pCam->Pos = Vec2(5, 13);

		if (pGfx) {
			pMap->Debug(pGfx);
			//pMap->DebugQuad(pMap->Root(), pGfx);
		}

		pCam->Clear();
		ThreadRow = DoneRow = 0;

		for (Thread *thread : Threads)
			thread->Wake();

		WaitForSingleObject(hDone, INFINITE);

		pCam->Dir.x += 10.f * Ang2Rad * deltaTime;
		//pCam->Dir.y += 3.f * Ang2Rad * deltaTime;
		pCam->Dir.y = sinf(pCam->Dir.x) + PI / 2.0f;

#ifdef SHOWDEBUG
		Sleep(100);
#endif
		return;
	}
};
