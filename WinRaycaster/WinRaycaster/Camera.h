#pragma once


class Camera {
public:
	Vec3	Pos;
	//float	Dir;
	Vec2	Dir;
	float	FoV;

	int		Wid;
	int		Hei;

	Pixel	*pImage;
	Bitmap	*pBmp;

	Camera(const int Wid, const int Hei) :
		Wid(Wid), Hei(Hei) {

		pImage = new Pixel[Wid * Hei];
		assert(pImage);

		pBmp = new Bitmap(Wid, Hei, PixelFormat32bppARGB);
		assert(pBmp);
	}

	void Clear() {
		assert(pImage);
		memset(pImage, 0, Wid * Hei * sizeof(Pixel));
		//for (int i = 0; i < Wid * Hei; pImage[i++] = 0xFF000000);
	}

	/*void Display(Graphics *pGfx) {
		assert(pImage && pBmp && pGfx);

		BitmapData bd;
		if (pBmp->LockBits(NULL, ImageLockModeWrite, PixelFormat32bppARGB, &bd) != Ok)
			return;

		assert(bd.Width == Wid && bd.Height == Hei);

		const int stride = abs(bd.Stride) / sizeof(Pixel);

		for (int row = 0; row < Hei; row++) {
			memcpy((Pixel*)bd.Scan0 + row * stride, pImage + row * Wid, Wid * sizeof(Pixel));
		}

		pBmp->UnlockBits(&bd);
		pGfx->DrawImage(pBmp, 0, 0);
		//pGfx->DrawImage(pBmp, 0, 0, Wid * 2, Hei * 2);
	}*/
};
