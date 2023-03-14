#pragma once

#include "Node.h"

class Surface
{
protected:
	Node* goalNode;

	Node* startNode;

	size_t nodeSize;

	Node* pathHeadNode;

	bool shouldDrawWhileRunning;

	size_t x1, x2, y1, y2;

	size_t nodesInRowCount;

	size_t nodesInColumnCount;

	std::vector<std::vector<Node*>> nodes;

	void clearPath();

	void colorPath(bool filled);

	void setNeighbors(Node* node);

	float heuristic(const Node* a, const Node* b);

	float calculateG(const Node* a, const Node* b);

	virtual void colorNodes(const std::vector<Node*>& nodes) = 0;

public:
	Surface(size_t nodeSize, size_t x1, size_t x2, size_t y1, size_t y2, bool shouldDrawWhileRunning);

	~Surface();

	virtual void findPath();
};
