#include "Window.h"

#include <process.h>
#include <stdexcept>

const std::map<Node::NodeType, COLORREF> Window::nodeTypeToColorMap =
{
	{Node::NodeType::BLANK, RGB(255, 255, 255)},
	{Node::NodeType::OBSTACLE, RGB(0, 0, 0)},
	{Node::NodeType::START, RGB(255, 0, 0)},
	{Node::NodeType::GOAL, RGB(0, 255, 0)},
	{Node::NodeType::PATH, RGB(0, 0, 255)}
};

Window::Window(HINSTANCE instance, bool drawWhileRunning):
	Surface(20, 20, 860, 20, 700, drawWhileRunning),
	pathFinderRunning(false)
{
	registerWindow(instance);
}

LRESULT CALLBACK windowProcedure(HWND windowHandle, UINT message, WPARAM wparam, LPARAM lparam);

void Window::registerWindow(HINSTANCE instance)
{
	WNDCLASSEX windowClass = {};

	LPCWSTR windowClassName = L"A* path finder window class";

	windowClass.hInstance = instance;
	windowClass.cbSize = sizeof(windowClass);
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.lpszClassName = windowClassName;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	
	if(!RegisterClassEx(&windowClass))
		throw std::runtime_error("RegisterClassEx failed");

	windowHandle = CreateWindowEx
	(
		NULL,
		windowClassName,
		L"A* path finder",
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1024,
		768,
		NULL,
		NULL,
		instance,
		NULL
	);

	if(!windowHandle)
		throw std::runtime_error("CreateWindowEx failed for window");

	LPCWSTR buttonClassName = L"Button";

	buttonFindPathHandle = CreateWindowEx
	(
		NULL,
		buttonClassName,
		L"Find path",
		WS_CHILD | WS_VISIBLE,
		870,
		20,
		80,
		40,
		windowHandle,
		(HMENU)ButtonId::FIND_PATH,
		instance,
		NULL
	);

	if(!buttonFindPathHandle)
		throw std::runtime_error("CreateWindowEx failed for button find path");

	buttonEraseHandle = CreateWindowEx
	(
		NULL,
		buttonClassName,
		L"Erase surface",
		WS_CHILD | WS_VISIBLE,
		870,
		70,
		100,
		30,
		windowHandle,
		(HMENU)ButtonId::ERASE,
		instance,
		NULL
	);

	if(!buttonEraseHandle)
		throw std::runtime_error("CreateWindowEx failed for button erase");
	
	buttonBlankHandle = CreateWindowEx
	(
		NULL,
		buttonClassName,
		L"blank",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		870,
		120,
		55,
		20,
		windowHandle,
		(HMENU)ButtonId::BLANK,
		instance,
		NULL
	);

	if(!buttonBlankHandle)
		throw std::runtime_error("CreateWindowEx failed for button blank");

	buttonObstacleHandle = CreateWindowEx
	(
		NULL,
		buttonClassName,
		L"obstacle",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		870,
		170,
		65,
		20,
		windowHandle,
		(HMENU)ButtonId::OBSTACLE,
		instance,
		NULL
	);

	if(!buttonObstacleHandle)
		throw std::runtime_error("CreateWindowEx failed for button obstacle");

	buttonStartHandle = CreateWindowEx
	(
		NULL,
		buttonClassName,
		L"start",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		870,
		220,
		50,
		20,
		windowHandle,
		(HMENU)ButtonId::START,
		instance,
		NULL
	);

	if(!buttonStartHandle)
		throw std::runtime_error("CreateWindowEx failed for button start");

	buttonGoalHandle = CreateWindowEx
	(
		NULL,
		buttonClassName,
		L"goal",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		870,
		270,
		50,
		20,
		windowHandle,
		(HMENU)ButtonId::GOAL,
		instance,
		NULL
	);

	if(!buttonGoalHandle)
		throw std::runtime_error("CreateWindowEx failed for button goal");

	buttonDrawWhileRunningHandle = CreateWindowEx
	(
		NULL,
		buttonClassName,
		L"draw path while running",
		WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
		870,
		320,
		150,
		20,
		windowHandle,
		(HMENU)ButtonId::DRAW_WHILE_RUNNING,
		instance,
		NULL
	);

	if(!buttonDrawWhileRunningHandle)
		throw std::runtime_error("CreateWindowEx failed for button draw while running");

	std::vector<HWND> buttonHandles =
	{
		buttonFindPathHandle,
		buttonEraseHandle,
		buttonBlankHandle,
		buttonObstacleHandle,
		buttonStartHandle,
		buttonGoalHandle,
		buttonDrawWhileRunningHandle
	};

	WPARAM defaultFont = (WPARAM)GetStockObject(DEFAULT_GUI_FONT);

	for(HWND handle: buttonHandles)
		SendMessage(handle, WM_SETFONT, defaultFont, 0);

	selectedNodeType = Node::NodeType::OBSTACLE;

	SendMessage(buttonObstacleHandle, BM_SETCHECK, BST_CHECKED, 0);

	setButtonDrawWhileRunningState();
}

void Window::drawNode(HDC hdc, const Node* node)
{
	HBRUSH newBrush = CreateSolidBrush(nodeTypeToColorMap.at(node->type));

	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, newBrush);

	Rectangle(hdc, node->x, node->y, node->x + nodeSize, node->y + nodeSize);

	SelectObject(hdc, oldBrush);

	DeleteObject(newBrush);
}

void Window::setButtonDrawWhileRunningState() const
{
	SendMessage(buttonDrawWhileRunningHandle, BM_SETCHECK, shouldDrawWhileRunning ? BST_CHECKED : BST_UNCHECKED, 0);
}

void Window::colorNodes(const std::vector<Node*>& nodes)
{
	if(nodes.empty())
		return;
	
	nodesToColor = nodes;

	Node* first = nodes[0];

	size_t x1 = first->x;
	size_t y1 = first->y;

	size_t x2 = x1;
	size_t y2 = y1;

	for(size_t i = 1; i < nodes.size(); i++)
	{
		Node* node = nodes[i];

		if(node->x < x1)
			x1 = node->x;
		else if(node->x > x2)
			x2 = node->x;

		if(node->y < y1)
			y1 = node->y;
		else if(node->y > y2)
			y2 = node->y;
	}

	redraw(x1, x2 + nodeSize, y1, y2 + nodeSize);
}

void Window::draw(HDC hdc)
{
	// if there are nodes to color then draw only them.
	// otherwise redraw everything
	if(!nodesToColor.empty())
	{
		for(Node* node: nodesToColor)
			drawNode(hdc, node);

		nodesToColor.clear();

		return;
	}

	HPEN newPen = CreatePen(PS_SOLID, 10, RGB(0, 0, 0));

	HPEN oldPen = (HPEN)SelectObject(hdc, newPen);

	Rectangle(hdc, x1, y1, x2, y2);

	SelectObject(hdc, oldPen);

	DeleteObject(newPen);

	for(const auto& column: nodes)
		for(Node* node: column)
			drawNode(hdc, node);
}

void Window::show(int cmdShow)
{
	ShowWindow(windowHandle, cmdShow);

	UpdateWindow(windowHandle);

	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);

		DispatchMessage(&msg);
	}
}

void Window::onMouseDownMove(size_t x, size_t y)
{
	if(pathFinderRunning || x < x1 || x > x2 || y < y1 || y > y2)
		return;

	clearPath();

	// prevent i, j out of bounds
	if(x == x2)
		x--;
	
	if(y == y2)
		y--;

	const size_t i = (x - x1) / nodeSize;
	const size_t j = (y - y1) / nodeSize;

	Node* node = nodes[i][j];

	// no need to color node with the same type
	if(node->type == selectedNodeType)
		return;

	node->type = selectedNodeType;

	if(node == startNode)
		startNode = nullptr;
	else if(node == goalNode)
		goalNode = nullptr;

	// change start/goal node
	if(selectedNodeType == Node::NodeType::START)
	{
		if(startNode)
		{
			startNode->type = Node::NodeType::BLANK;

			nodesToColor.push_back(startNode);
		}

		startNode = node;
	}
	else if(selectedNodeType == Node::NodeType::GOAL)
	{
		if(goalNode)
		{
			goalNode->type = Node::NodeType::BLANK;

			nodesToColor.push_back(goalNode);
		}

		goalNode = node;
	}
	
	nodesToColor.push_back(node);

	colorNodes(nodesToColor);
}

void Window::findPath()
{
	pathFinderRunning = true;

	Surface::findPath();

	pathFinderRunning = false;
}

void __cdecl pathFinderThread(void* parameter)
{
	((Window*)parameter)->findPath();

	_endthread();
}

void Window::findPathConcurrently()
{
	_beginthread(pathFinderThread, 0, this);
}

void Window::onButtonPressed(ButtonId buttonId)
{
	switch(buttonId)
	{
	case ButtonId::FIND_PATH:
		if(pathFinderRunning)
			break;

		if(!startNode)
		{
			MessageBox(NULL, L"Set start node", L"Error", MB_ICONERROR);

			break;
		}

		if(!goalNode)
		{
			MessageBox(NULL, L"Set goal node", L"Error", MB_ICONERROR);

			break;
		}

		findPathConcurrently();

		break;

	case ButtonId::ERASE:
		if(pathFinderRunning)
			break;

		// clear all colored nodes

		clearPath();

		if(startNode)
		{
			startNode->type = Node::NodeType::BLANK;

			nodesToColor.push_back(startNode);

			startNode = nullptr;
		}

		if(goalNode)
		{
			goalNode->type = Node::NodeType::BLANK;

			nodesToColor.push_back(goalNode);

			goalNode = nullptr;
		}

		for(const auto& column: nodes)
			for(Node* node: column)
				if(node->type == Node::NodeType::OBSTACLE)
				{
					node->type = Node::NodeType::BLANK;

					nodesToColor.push_back(node);
				}
		
		colorNodes(nodesToColor);

		break;

	case ButtonId::BLANK:
		selectedNodeType = Node::NodeType::BLANK;

		break;

	case ButtonId::OBSTACLE:
		selectedNodeType = Node::NodeType::OBSTACLE;

		break;

	case ButtonId::START:
		selectedNodeType = Node::NodeType::START;

		break;

	case ButtonId::GOAL:
		selectedNodeType = Node::NodeType::GOAL;

		break;

	case ButtonId::DRAW_WHILE_RUNNING:
		if(pathFinderRunning)
			break;

		shouldDrawWhileRunning = !shouldDrawWhileRunning;

		setButtonDrawWhileRunningState();

		break;
	}
}

void Window::redraw(size_t x1, size_t x2, size_t y1, size_t y2)
{
	RECT rect = {};

	rect.left = x1;
	rect.top = y1;
	rect.right = x2;
	rect.bottom = y2;

	RedrawWindow(windowHandle, &rect, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

Window* window = nullptr;

LRESULT CALLBACK windowProcedure(HWND windowHandle, UINT message, WPARAM wparam, LPARAM lparam)
{
	static bool mouseDown = false;

	switch(message)
	{
	case WM_LBUTTONDOWN:
		mouseDown = true;

	case WM_MOUSEMOVE:
		if(mouseDown)
			window->onMouseDownMove(LOWORD(lparam), HIWORD(lparam));

		break;

	case WM_LBUTTONUP:
		mouseDown = false;

		break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;

		HDC hdc = BeginPaint(windowHandle, &paint);

		window->draw(hdc);

		EndPaint(windowHandle, &paint);

		break;
	}

	case WM_COMMAND:
		window->onButtonPressed((Window::ButtonId)wparam);

		break;

	case WM_CLOSE:
		DestroyWindow(windowHandle);

		break;

	case WM_DESTROY:
		PostQuitMessage(0);

		break;

	default:
		return DefWindowProc(windowHandle, message, wparam, lparam);
	}

	return 0;
}
