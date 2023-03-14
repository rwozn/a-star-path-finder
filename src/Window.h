#pragma once

#include "Surface.h"

#include <map>
#include <atomic>
#include <Windows.h>

class Window: public Surface
{
	HWND windowHandle;
	
	HWND buttonGoalHandle;
	HWND buttonBlankHandle;
	HWND buttonEraseHandle;
	HWND buttonStartHandle;
	HWND buttonFindPathHandle;
	HWND buttonObstacleHandle;
	HWND buttonDrawWhileRunningHandle;

	std::vector<Node*> nodesToColor;

	Node::NodeType selectedNodeType;

	std::atomic_bool pathFinderRunning;
	
	static const std::map<Node::NodeType, COLORREF> nodeTypeToColorMap;

	void registerWindow(HINSTANCE instance);
	
	void drawNode(HDC hdc, const Node* node);

	void setButtonDrawWhileRunningState() const;

	virtual void colorNodes(const std::vector<Node*>& nodes) override;

public:
	enum class ButtonId
	{
		FIND_PATH,
		ERASE,
		BLANK,
		OBSTACLE,
		START,
		GOAL,
		DRAW_WHILE_RUNNING
	};

	Window(HINSTANCE instance, bool drawWhileRunning = true);

	void draw(HDC hdc);

	void show(int cmdShow);

	void findPathConcurrently();

	virtual void findPath() override;

	void onButtonPressed(ButtonId buttonId);

	void onMouseDownMove(size_t x, size_t y);

	void redraw(size_t x1, size_t x2, size_t y1, size_t y2);
};

extern Window* window;