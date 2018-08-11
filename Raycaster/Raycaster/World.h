#pragma once
#include <vector>
#include "RCMath.h"

using namespace std;

struct Cell {
	// Coordinates start at TOP LEFT CORNER
	// Positive X goes RIGHT	// Positive Y goes DOWN
	Vec2 topLeft, bottomRight;

	Cell();
};

class World {
private:
	World();
	World(const float w, const float h, const Vec2 cellSize);
	void InitializeMatrix();

	static World* _world;
	float _worldWidth, _worldHeight;
	vector<vector<Cell>> _cells;
	Vec2 _cellSize;

public:
	static World * GetWorld();
	static World * GetWorld(const float w, const float h, const Vec2 cellSize);
};
