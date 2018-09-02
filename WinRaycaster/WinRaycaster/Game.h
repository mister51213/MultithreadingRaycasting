#pragma once


class Game {
public:
	Critical		CS;
	HANDLE			hDone;
	int				ThreadCol;
	int				DoneCol;

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
		pCam->Pos = Vec2(1.5f, 1.2f);
		pCam->Dir = 0.0f * Ang2Rad;
		pCam->FoV = 90.0f * Ang2Rad;

		Textures.push_back(Texture(L"textures\\wall1.png"));
		//Textures.push_back(Texture(L"textures\\align.png"));
		//Textures.push_back(Texture(L"textures\\red-brick-wall-living-room-india-effect-ireland-kitchen-shaped-good-looking-uv-h-cm-jpg-iida-comp-pinterest.jpg"));

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

		for (int i = 0; i < Size; i++) {
			pMap->GetCell(Int2(i, 0)).pWallTex = &Textures[0];
			pMap->GetCell(Int2(0, i)).pWallTex = &Textures[0];
			pMap->GetCell(Int2(i, s)).pWallTex = &Textures[0];
			pMap->GetCell(Int2(s, i)).pWallTex = &Textures[0];
		}

		pMap->GetCell(Int2(3, 3)).pWallTex = &Textures[0];
		pMap->GetCell(Int2(7, 3)).pWallTex = &Textures[0];
		pMap->GetCell(Int2(3, 7)).pWallTex = &Textures[0];
		pMap->GetCell(Int2(7, 7)).pWallTex = &Textures[0];
	}

	void Tick(float deltaTime, HWND hWnd) {
		if (!pCam)
			return;

		pCam->Pos = Vec2(5.5, 4.5) + Vec2(sinf(-pCam->Dir * 2.5f), cosf(-pCam->Dir) * 2.0f);

		if (pGfx)
			pMap->Debug(pGfx);

		pCam->Clear();
		ThreadCol = DoneCol = 0;

		for (Thread *thread : Threads)
			thread->Wake();

		WaitForSingleObject(hDone, INFINITE);

		pCam->Dir += 10.0f * Ang2Rad * deltaTime;
		//pCam->Dir = fmod(pCam->Dir + 10.0f * Ang2Rad * deltaTime + QTAU, QTAU) - ETAU;

		return;
	}
};
