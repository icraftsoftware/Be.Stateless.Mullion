#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Application.h"

using namespace Be::Stateless::Mullion;

int WINAPI WinMain(const HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	return Application::Run(hInstance);
}
