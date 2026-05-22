#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdlib>
#include "mouse_hook.h"

// Pixels the physical mouse must travel past the primary→secondary edge before crossing.
// Matches DMT Cursor__MinStickyForce = 514.
static constexpr LONG STICKY_FORCE = 514;

static HHOOK g_hook = nullptr;
static HMONITOR g_primary_mon = nullptr;
static RECT g_primary_rect = {};
static RECT g_virtual_desktop = {};
static POINT g_prev_pt = {};
static bool g_sticky = false;
static POINT g_sticky_edge = {};
static bool g_sticky_vertical = false; // true = left/right edge, false = top/bottom
static LONG g_push_accum = 0; // accumulated pixels pushed past the edge
static bool g_setting_cursor = false;

static void InitPrimary()
{
	g_primary_mon = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = {sizeof(mi)};
	GetMonitorInfo(g_primary_mon, &mi);
	g_primary_rect = mi.rcMonitor;

	// Virtual desktop bounding rect — used to detect which edges have monitors beyond them
	g_virtual_desktop.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	g_virtual_desktop.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	g_virtual_desktop.right = g_virtual_desktop.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	g_virtual_desktop.bottom = g_virtual_desktop.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

	GetCursorPos(&g_prev_pt);
}

// Matches DMT: apply friction at any edge where the virtual desktop extends beyond the primary.
static bool EdgeHasFriction(const POINT& newPt)
{
	if (newPt.x < g_primary_rect.left)
		return g_virtual_desktop.left < g_primary_rect.left;
	if (newPt.x >= g_primary_rect.right)
		return g_virtual_desktop.right > g_primary_rect.right;
	if (newPt.y < g_primary_rect.top)
		return g_virtual_desktop.top < g_primary_rect.top;
	if (newPt.y >= g_primary_rect.bottom)
		return g_virtual_desktop.bottom > g_primary_rect.bottom;
	return false;
}

static LRESULT CALLBACK LLMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode != HC_ACTION || wParam != WM_MOUSEMOVE || g_setting_cursor)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	auto* ms = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
	POINT newPt = ms->pt;
	POINT prevPt = g_prev_pt;
	g_prev_pt = newPt;

	bool ctrl = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;

	if (g_sticky)
	{
		if (ctrl)
		{
			// Left Ctrl held: pass through freely (matches DMT FreeMovementKey)
			g_sticky = false;
			return CallNextHookEx(nullptr, nCode, wParam, lParam);
		}

		// If physical mouse moved back onto the primary, release cleanly
		if (PtInRect(&g_primary_rect, newPt))
		{
			g_sticky = false;
			g_push_accum = 0;
			return CallNextHookEx(nullptr, nCode, wParam, lParam);
		}

		// Accumulate push past the edge (matches DMT TotalForce logic)
		LONG delta = g_sticky_vertical
			? abs(newPt.x - g_sticky_edge.x)
			: abs(newPt.y - g_sticky_edge.y);
		g_push_accum += delta;

		if (g_push_accum >= STICKY_FORCE)
		{
			// Break through: place cursor just past the edge by the overshoot amount
			LONG overshoot = g_push_accum - STICKY_FORCE;
			POINT release = g_sticky_edge;
			if (g_sticky_vertical)
				release.x += (newPt.x > g_sticky_edge.x) ? overshoot : -overshoot;
			else
				release.y += (newPt.y > g_sticky_edge.y) ? overshoot : -overshoot;

			g_sticky = false;
			g_push_accum = 0;
			g_setting_cursor = true;
			SetCursorPos(release.x, release.y);
			g_setting_cursor = false;
			return 1;
		}

		// Still within threshold — hold cursor at edge
		g_setting_cursor = true;
		SetCursorPos(g_sticky_edge.x, g_sticky_edge.y);
		g_setting_cursor = false;
		return 1;
	}

	// Detect crossing from primary into virtual desktop space beyond it
	if (!ctrl
		&& PtInRect(&g_primary_rect, prevPt)
		&& !PtInRect(&g_primary_rect, newPt)
		&& EdgeHasFriction(newPt))
	{
		g_sticky = true;
		g_push_accum = 0;
		g_sticky_vertical = (newPt.x < g_primary_rect.left || newPt.x >= g_primary_rect.right);
		g_sticky_edge = {
			max(g_primary_rect.left, min(g_primary_rect.right - 1, newPt.x)),
			max(g_primary_rect.top, min(g_primary_rect.bottom - 1, newPt.y))
		};

		g_setting_cursor = true;
		SetCursorPos(g_sticky_edge.x, g_sticky_edge.y);
		g_setting_cursor = false;
		return 1;
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

bool MouseHook_Install()
{
	InitPrimary();
	g_hook = SetWindowsHookEx(WH_MOUSE_LL, LLMouseProc, nullptr, 0);
	return g_hook != nullptr;
}

void MouseHook_Uninstall()
{
	if (g_hook)
	{
		UnhookWindowsHookEx(g_hook);
		g_hook = nullptr;
	}
}
