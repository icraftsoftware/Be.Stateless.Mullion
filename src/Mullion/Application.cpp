#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Application.h"
#include "Resource.h"
#include "hotkeys.h"
#include "mouse_hook.h"
#include "placement.h"

using namespace Be::Stateless::Mullion;
using namespace Be::Stateless::Mullion::Windows::Shell;

int Application::Run(const HINSTANCE instance)
{
	// opt into physical pixels — required for accurate placement on high-DPI/mixed-DPI setups.
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	const HANDLE singleInstanceGuard = CreateMutexW(nullptr, TRUE, ApplicationName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(singleInstanceGuard);
		return 0;
	}

	const WNDCLASSEXW windowClass{
		.cbSize = sizeof(WNDCLASSEXW),
		.lpfnWndProc = HandleMessage,
		.hInstance = instance,
		.lpszClassName = ApplicationName,
	}; // NOLINT(clang-diagnostic-missing-designated-field-initializers)
	RegisterClassExW(&windowClass);

	const HWND messageWindow = CreateWindowExW(0, ApplicationName, ApplicationName, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, instance, nullptr);
	if (!messageWindow)
		return 1;

	tray.emplace(messageWindow);
	Hotkeys_Install(messageWindow);
	MouseHook_Install();

	MSG message;
	while (GetMessage(&message, nullptr, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	CloseHandle(singleInstanceGuard);
	return static_cast<int>(message.wParam);
}

LRESULT CALLBACK Application::HandleMessage(const HWND window, const UINT messageType, const WPARAM wparam, const LPARAM lparam)
{
	switch (messageType)
	{
	case WmTrayNotification:
		tray->HandleNotification(lparam); // NOLINT(bugprone-unchecked-optional-access): shell sends WmTrayNotification only after NIM_ADD, which runs inside tray.emplace()
		return 0;
	case WM_COMMAND:
		HandleCommand(window, wparam);
		return 0;
	case WmSnap:
		PlaceWindow(static_cast<int>(wparam), (lparam & static_cast<long long>(SnapModes::Stretch)) != 0);
		return 0;
	case WmCenter:
		CenterWindow();
		return 0;
	case WM_DESTROY:
		tray.reset();
		Hotkeys_Uninstall();
		MouseHook_Uninstall();
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(window, messageType, wparam, lparam);
	}
}

void Application::HandleCommand(const HWND window, const WPARAM wparam)
{
	switch (static_cast<MenuCommand>(LOWORD(wparam)))
	{
	case MenuCommand::AutoStart:
		Tray::ToggleAutostart();
		break;
	case MenuCommand::ReloadConfig:
		// TODO: reload layout/hotkey config from file
		break;
	case MenuCommand::Exit:
		DestroyWindow(window);
		break;
	}
}
