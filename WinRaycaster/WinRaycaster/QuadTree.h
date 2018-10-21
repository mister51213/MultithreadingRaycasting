#pragma once


enum QuadCellNum : int {
	qTL, qTR, qBL, qBR
};


class Quad {
public:
	int		Depth;
	int		Size;
	Int2	Pos;
	Quad	*pParent = nullptr;
	//Quad	*pNeighbor[4] = {nullptr};
	Quad	*pChild[4] = {nullptr};

	Texture	*pWall = nullptr;

	Quad() {}

	Quad(const int MaxDepth) :
	//	Quad(nullptr, MaxDepth, 1 << MaxDepth, Int2(0, 0)) {}
		Depth(MaxDepth), Size(1 << Depth), Pos(Int2(0, 0)) {}

	//Quad(Quad *pParent, const int Depth, const int Size, const Int2 &Pos) : 
	//	pParent(pParent), Depth(Depth), Size(Size), Pos(Pos) {}

	Quad(Quad *pParent, const Int2 RelPos) :
		pParent(pParent), Depth(pParent->Depth - 1), Size(1 << Depth), Pos(pParent->Pos + RelPos), 
		pWall(pParent->pWall) {}

	inline Quad* operator[] (const int Child) {
		return pChild[Child];
	}

	inline Quad& operator() (const int Child) {
		return *pChild[Child];
	}

	// This WILL return a pointer to a Quad that contains the neighboring Quad, but it may be at some lower depth.
	inline Quad* AscendWest(Quad *pFrom, const int Edge, const int Axis) {
		for (;;) {
			// Technically won't allow hitting the outer edge of the root node.
			_ASSERTE(pFrom->pParent);

			// Is Edge within the bounds of our parent?
			if (Edge > pFrom->pParent->Pos[Axis]) {
				// If so, we can check siblings.
				return pFrom->pParent;
			} else {
				// If not, we ascend and check cousins.
				pFrom = pFrom->pParent;
			}
		}
	}

	// This WILL return a pointer to a Quad that contains the neighboring Quad, but it may be at some lower depth.
	inline Quad* AscendEast(Quad *pFrom, const int Edge, const int Axis) {
		for (;;) {
			// Technically won't allow hitting the outer edge of the root node.
			_ASSERTE(pFrom->pParent);

			// Is Edge within the bounds of our parent?
			if (Edge < pFrom->pParent->Pos[Axis] + pFrom->pParent->Size) {
				// If so, we can check siblings.
				return pFrom->pParent;
			} else {
				// If not, we ascend and check cousins.
				pFrom = pFrom->pParent;
			}
		}
	}
/*
	inline Quad* DescendNorth(const int Pos) {
		const int LorR = (Pos >> (Depth - 1));
		const int child = 0 + LorR;
		return pChild[child] ? pChild[child]->DescendNorth(Pos) : this;
	}

	inline Quad* DescendSouth(const int Pos) {
		const int LorR = (Pos >> (Depth - 1));
		const int child = 2 + LorR;
		return pChild[child] ? pChild[child]->DescendSouth(Pos) : this;
	}
*/
	inline Quad* DescendH(const int Position, const int TorB) {
		const int LorR = ((Position - Pos.y) >> (Depth - 1));	// LorB = Pos / (Size / 2)
		const int child = TorB + LorR;
		return pChild[child] ? pChild[child]->DescendH(Position, TorB) : this;
	}

	inline Quad* DescendV(const int Pos, const int LorR) {
		const int TorB = (Pos >> (Depth - 1)) << 1;
		const int child = TorB + LorR;
		return pChild[child] ? pChild[child]->DescendV(Pos, TorB) : this;
	}

	Quad* StepEast() const {


	}

	bool IsSolid() const {
		return pWall ? true : false;
	}

	bool Subdivide() {
		// Depth 0 is the limit
		if (!Depth)
			return false;

		const int half = Size >> 1;
		pChild[qTL] = new Quad(this, /* Depth - 1, half, Pos + */ Int2(0,    0   ));
		pChild[qTR] = new Quad(this, /* Depth - 1, half, Pos + */ Int2(half, 0   ));
		pChild[qBL] = new Quad(this, /* Depth - 1, half, Pos + */ Int2(0,    half));
		pChild[qBR] = new Quad(this, /* Depth - 1, half, Pos + */ Int2(half, half));

		return true;
	}

	Quad& Divide() {
		Subdivide();
		return *this;
	}

	Quad& FindCell(const Int2 &Position) {
		if (!Depth)
			return *this;

		const Int2 RelPos = Position - Pos;

		const int LorR = (RelPos.x >> (Depth - 1));
		const int TorB = (RelPos.y >> (Depth - 1)) << 1;
		const int child = TorB + LorR;

		return pChild[child] ? pChild[child]->FindCell(Position) : *this;
	}
};