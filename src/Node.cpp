#include "Node.h"

Node::Node(size_t x, size_t y, size_t i, size_t j):
	x(x),
	y(y),
	i(i),
	j(j),
	parent(nullptr),
	f(0),
	g(0),
	h(0),
	type(NodeType::BLANK)
{

}
