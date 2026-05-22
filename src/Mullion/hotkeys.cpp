#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "hotkeys.h"
#include "resource.h"

static HHOOK g_hook = nullptr;
static HWND g_hwnd = nullptr;
static DWORD g_consumed = 0; // vkCode of key whose keyup we must suppress

static LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode != HC_ACTION)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	auto* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
	bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
	bool up = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

	// Suppress keyup for any key we consumed on the way down
	if (up && g_consumed == kb->vkCode)
	{
		g_consumed = 0;
		return 1;
	}
	if (!down)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	bool win = (GetAsyncKeyState(VK_LWIN) & 0x8000) != 0
		|| (GetAsyncKeyState(VK_RWIN) & 0x8000) != 0;
	bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
	bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
	bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

	if (!win || !alt || ctrl)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	int direction = -1;
	bool center = false;

	switch (kb->vkCode)
	{
	case VK_LEFT:
		direction = static_cast<int>(SnapDirection::Left);
		break;
	case VK_RIGHT:
		direction = static_cast<int>(SnapDirection::Right);
		break;
	case VK_UP:
		direction = static_cast<int>(SnapDirection::Up);
		break;
	case VK_DOWN:
		direction = static_cast<int>(SnapDirection::Down);
		break;
	case 'C':
		center = true;
		break;
	default:
		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	}

	g_consumed = kb->vkCode;

	if (center)
		PostMessage(g_hwnd, WmCenter, 0, 0);
	else
		PostMessage(g_hwnd, WmSnap,
			static_cast<WPARAM>(direction),
			shift ? static_cast<long long>(SnapModes::Stretch) : 0);

	return 1; // consumed — do not pass to system or apps
}

bool Hotkeys_Install(HWND hwnd)
{
	g_hwnd = hwnd;
	g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, nullptr, 0);
	return g_hook != nullptr;
}

void Hotkeys_Uninstall()
{
	if (g_hook)
	{
		UnhookWindowsHookEx(g_hook);
		g_hook = nullptr;
	}
}
