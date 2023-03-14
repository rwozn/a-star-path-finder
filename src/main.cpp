#include "Window.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR cmdLine, int cmdShow)
{
	window = new Window(instance);

	window->show(cmdShow);

	delete window;

	return 0;
}
