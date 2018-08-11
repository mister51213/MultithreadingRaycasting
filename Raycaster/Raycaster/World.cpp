#include "World.h"

Cell::Cell() {

}

World* World::_world = nullptr;

World::World() :
	_worldWidth(100000),
	_worldHeight(100000),
	_cellSize(10,10)
{}

World::World(const float w, const float h, const Vec2 cellSize) :
	_worldWidth(w),
	_worldHeight(h),
	_cellSize(cellSize)
{}

World* World::GetWorld() {
	if (World::_world)
		return _world;
	else {
		World::_world = new World();
		_world->InitializeMatrix();
		return World::_world;
	}
}

World* World::GetWorld(const float w, const float h, const Vec2 cSize) {
	if (World::_world)
		return _world;
	else {
		World::_world = new World(w, h, cSize);
		_world->InitializeMatrix();
		return World::_world;
	}
}

void World::InitializeMatrix() {
	const int numRows = _worldHeight / _cellSize.y;
	const int numCols = _worldWidth / _cellSize.x;

	Vec2 offset(0, 0);
	_cells.resize(numRows);

	for (vector<Cell>& row : _cells) {
		row.resize(numCols);
		for (Cell& cell : row) {
			cell.topLeft = offset;
			cell.bottomRight = offset + _cellSize;
			offset.x += _cellSize.x;
		}
		offset.y += _cellSize.y;
	}
}