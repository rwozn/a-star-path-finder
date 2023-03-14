#include "Surface.h"

#include <algorithm>

void Surface::clearPath()
{
	if(pathHeadNode)
	{
		colorPath(false);

		pathHeadNode = nullptr;
	}
}

void Surface::setNeighbors(Node* node)
{
	if(!node->neighbors.empty())
		return;
	
	int a = -1;
	int b = -1;
	
	int c = 1;
	int d = 1;

	const size_t i = node->i;
	const size_t j = node->j;

	if(!i)
		a = 0;
	else if(i == nodesInRowCount - 1)
		c = 0;

	if(!j)
		b = 0;
	else if(j == nodesInColumnCount - 1)
		d = 0;

	// add neighboring nodes (8 at most) to this node's neighbors
	for(int k = a; k <= c; k++)
		for(int l = b; l <= d; l++)
			if(k || l) // if it's not the current node
				node->neighbors.push_back(nodes[i + k][j + l]);
}

// diagonal distance
float Surface::heuristic(const Node* a, const Node* b)
{
	int dx = abs((int)a->i - (int)b->i);
	int dy = abs((int)a->j - (int)b->j);

	static const float d = sqrtf(2) - 2;

	return dx + dy + d * std::min(dx, dy);
}

float Surface::calculateG(const Node* a, const Node* b)
{
	return a->g + heuristic(a, b);
}

static bool isInPath(const Node* node, const Node* pathHeadNode)
{
	while(pathHeadNode)
	{
		if(node == pathHeadNode)
			return true;

		pathHeadNode = pathHeadNode->parent;
	}

	return false;
}

void Surface::colorPath(bool filled)
{
	Node* temp = pathHeadNode;

	std::vector<Node*> nodesToColor;

	const Node::NodeType nodeType = filled ? Node::NodeType::PATH : Node::NodeType::BLANK;

	while(temp != startNode)
	{
		temp->type = nodeType;

		nodesToColor.push_back(temp);

		temp = temp->parent;
	}

	colorNodes(nodesToColor);
}

Surface::Surface(size_t nodeSize, size_t x1, size_t x2, size_t y1, size_t y2, bool shouldDrawWhileRunning):
	goalNode(nullptr),
	nodeSize(nodeSize),
	startNode(nullptr),
	pathHeadNode(nullptr),
	shouldDrawWhileRunning(shouldDrawWhileRunning),
	x1(x1),
	x2(x2),
	y1(y1),
	y2(y2),
	nodesInRowCount((x2 - x1) / nodeSize),
	nodesInColumnCount((y2 - y1) / nodeSize),
	nodes(nodesInRowCount)
{
	for(size_t i = 0; i < nodesInRowCount; i++)
	{
		nodes[i].resize(nodesInColumnCount);

		for(size_t j = 0; j < nodesInColumnCount; j++)
			nodes[i][j] = new Node(x1 + i * nodeSize, y1 + j * nodeSize, i, j);
	}
}

Surface::~Surface()
{
	for(const auto& column: nodes)
		for(Node* node: column)
			delete node;
}

static inline bool containsNode(const std::vector<Node*>& v, Node* node)
{
	return std::find(v.begin(), v.end(), node) != v.end();
}

void Surface::findPath()
{
	clearPath();

	std::vector<Node*> closedSet;

	std::vector<Node*> openSet = {startNode};

	do
	{
		Node* current = openSet[0];

		// reached goal
		if(current == goalNode)
		{
			pathHeadNode = goalNode->parent;

			break;
		}

		pathHeadNode = current;

		// draw current path
		if(shouldDrawWhileRunning)
			colorPath(true);

		setNeighbors(current);

		closedSet.push_back(current);

		openSet.erase(openSet.begin());

		for(Node* neighbor: current->neighbors)
		{
			if(neighbor->type == Node::NodeType::OBSTACLE || containsNode(closedSet, neighbor))
				continue;

			bool shorterPath = false;

			float g = calculateG(current, neighbor);

			if(!containsNode(openSet, neighbor))
			{
				openSet.push_back(neighbor);

				neighbor->h = heuristic(neighbor, goalNode);

				shorterPath = true;
			}
			else if(g < neighbor->g)
				shorterPath = true;

			if(shorterPath)
			{
				neighbor->g = g;

				neighbor->parent = current;

				neighbor->f = neighbor->g + neighbor->h;
			}
		}

		std::sort(openSet.begin(), openSet.end(), [](const Node* const a, const Node* const b) -> bool
		{
			return a->f < b->f;
		});

		// clear previous path
		if(shouldDrawWhileRunning)
			colorPath(false);
	}
	while(!openSet.empty());

	colorPath(true);
}