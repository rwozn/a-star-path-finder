#pragma once

#include <vector>

struct Node
{
	enum class NodeType
	{
		BLANK,
		OBSTACLE,
		START,
		GOAL,
		PATH
	};

	size_t i, j;

	Node* parent;

	float f, g, h;

	NodeType type;

	size_t x, y;

	std::vector<Node*> neighbors;

	Node(size_t x, size_t y, size_t i, size_t j);
};