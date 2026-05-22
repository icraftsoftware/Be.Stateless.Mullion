#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dwmapi.h>
#include <cstdlib>
#include "placement.h"
#include "resource.h"

// ---------------------------------------------------------------------------
// Horizontal ring: {x, w} as fractions of monitor work area.
//
// Symmetric / palindromic layout — navigating rightward from R50 wraps back
// to L50, mirroring the journey in reverse. Duplicate positions (100%, C66)
// are intentional: they serve as transition waypoints in each direction.
// Slot tracking (see g_last) is required for duplicates to navigate correctly.
//
// Default ring (33% slots are present; set INCLUDE_THIRDS=0 to omit them):
//   L33  L50  L66  100%  C66  C50  C66  100%  R66  R50  R33
// ---------------------------------------------------------------------------
#define INCLUDE_THIRDS 1

struct HSlot
{
	float x, w;
};

static const HSlot h_ring[] = {
#if INCLUDE_THIRDS
	{0.000f, 0.333f}, //  [0] L33
#endif
	{0.000f, 0.500f}, //  L50
	{0.000f, 0.667f}, //  L66
	{0.000f, 1.000f}, //  Full  (left-side waypoint)
	{0.167f, 0.667f}, //  C66
	{0.250f, 0.500f}, //  C50
	{0.167f, 0.667f}, //  C66  (right-side waypoint — same rect, different ring position)
	{0.000f, 1.000f}, //  Full (right-side waypoint)
	{0.333f, 0.667f}, //  R66
	{0.500f, 0.500f}, //  R50
#if INCLUDE_THIRDS
	{0.667f, 0.333f}, //  R33
#endif
};
static constexpr int H_RING = ARRAYSIZE(h_ring);

struct VSlot
{
	float y, h;
};

static const VSlot v_ring[] = {
#if INCLUDE_THIRDS
	{0.000f, 0.333f}, //  T33
#endif
	{0.000f, 0.500f}, //  T50
	{0.000f, 0.667f}, //  T66
	{0.000f, 1.000f}, //  Full (top-side waypoint)
	{0.167f, 0.667f}, //  C66
	{0.250f, 0.500f}, //  C50
	{0.167f, 0.667f}, //  C66 (bottom-side waypoint)
	{0.000f, 1.000f}, //  Full (bottom-side waypoint)
	{0.333f, 0.667f}, //  B66
	{0.500f, 0.500f}, //  B50
#if INCLUDE_THIRDS
	{0.667f, 0.333f}, //  B33
#endif
};
static constexpr int V_RING = ARRAYSIZE(v_ring);

// ---------------------------------------------------------------------------
// Last-placement cache — required so ring navigation through duplicate slots
// (100%, C66) steps forward correctly instead of snapping back to the first
// occurrence when FindNearest is called.
// ---------------------------------------------------------------------------
static struct
{
	HWND hwnd;
	RECT rect;
	int h_slot;
	int v_slot;
} g_last = {};

static bool SamePosition(HWND hwnd)
{
	if (hwnd != g_last.hwnd)
		return false;
	RECT cur;
	GetWindowRect(hwnd, &cur);
	return abs(cur.left - g_last.rect.left) <= 2
		&& abs(cur.top - g_last.rect.top) <= 2
		&& abs(cur.right - g_last.rect.right) <= 2
		&& abs(cur.bottom - g_last.rect.bottom) <= 2;
}

// ---------------------------------------------------------------------------

static RECT GetWorkArea(HWND hwnd)
{
	HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = {sizeof(mi)};
	GetMonitorInfo(mon, &mi);
	return mi.rcWork;
}

// Apply DWM invisible-border compensation then move the window.
// Compensates for the transparent resize frame on Windows 10+ that causes
// GetWindowRect to return a rect larger than the visible window area.
static void ApplyRect(HWND hwnd, RECT target)
{
	WINDOWPLACEMENT wp = {sizeof(wp)};
	GetWindowPlacement(hwnd, &wp);
	if (wp.showCmd == SW_SHOWMAXIMIZED)
	{
		wp.showCmd = SW_RESTORE;
		SetWindowPlacement(hwnd, &wp);
	}

	RECT win_rect, ext_frame;
	GetWindowRect(hwnd, &win_rect);
	if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS,
		&ext_frame, sizeof(RECT))))
	{
		target.left -= (ext_frame.left - win_rect.left);
		target.top -= (ext_frame.top - win_rect.top);
		target.right += (win_rect.right - ext_frame.right);
		target.bottom += (win_rect.bottom - ext_frame.bottom);
	}

	SetWindowPos(hwnd, nullptr,
		target.left, target.top,
		target.right - target.left, target.bottom - target.top,
		SWP_NOZORDER | SWP_SHOWWINDOW);
}

static int FindNearestH(float nx, float nw)
{
	int best = 0;
	float best_d = 1e9f;
	for (int i = 0; i < H_RING; ++i)
	{
		float dx = h_ring[i].x - nx, dw = h_ring[i].w - nw;
		float d = dx * dx + dw * dw;
		if (d < best_d)
		{
			best_d = d;
			best = i;
		}
	}
	return best;
}

static int FindNearestV(float ny, float nh)
{
	int best = 0;
	float best_d = 1e9f;
	for (int i = 0; i < V_RING; ++i)
	{
		float dy = v_ring[i].y - ny, dh = v_ring[i].h - nh;
		float d = dy * dy + dh * dh;
		if (d < best_d)
		{
			best_d = d;
			best = i;
		}
	}
	return best;
}

static bool IsValidTarget(HWND hwnd)
{
	if (!hwnd || !IsWindowVisible(hwnd))
		return false;
	if (hwnd == GetDesktopWindow() || hwnd == GetShellWindow())
		return false;
	return true;
}

void PlaceWindow(int direction, bool snapSecondaryAxis)
{
	HWND hwnd = GetForegroundWindow();
	if (!IsValidTarget(hwnd))
		return;

	RECT work = GetWorkArea(hwnd);
	float wa_w = static_cast<float>(work.right - work.left);
	float wa_h = static_cast<float>(work.bottom - work.top);

	RECT win;
	GetWindowRect(hwnd, &win);
	float nx = (win.left - work.left) / wa_w;
	float ny = (win.top - work.top) / wa_h;
	float nw = (win.right - win.left) / wa_w;
	float nh = (win.bottom - win.top) / wa_h;

	RECT target = win;

	if (direction == static_cast<unsigned long long>(SnapDirection::Left) || direction == static_cast<unsigned long
		long>(
		SnapDirection::Right))
	{
		int cur = SamePosition(hwnd) ? g_last.h_slot : FindNearestH(nx, nw);
		int next = (direction == static_cast<unsigned long long>(SnapDirection::Right))
			? (cur + 1) % H_RING
			: (cur + H_RING - 1) % H_RING;
		target.left = work.left + static_cast<LONG>(h_ring[next].x * wa_w);
		target.right = target.left + static_cast<LONG>(h_ring[next].w * wa_w);
		if (snapSecondaryAxis)
		{
			target.top = work.top;
			target.bottom = work.bottom;
		}
		ApplyRect(hwnd, target);
		GetWindowRect(hwnd, &g_last.rect);
		g_last.hwnd = hwnd;
		g_last.h_slot = next;
	}
	else
	{
		int cur = SamePosition(hwnd) ? g_last.v_slot : FindNearestV(ny, nh);
		int next = (direction == static_cast<unsigned long long>(SnapDirection::Down))
			? (cur + 1) % V_RING
			: (cur + V_RING - 1) % V_RING;
		target.top = work.top + static_cast<LONG>(v_ring[next].y * wa_h);
		target.bottom = target.top + static_cast<LONG>(v_ring[next].h * wa_h);
		if (snapSecondaryAxis)
		{
			target.left = work.left;
			target.right = work.right;
		}
		ApplyRect(hwnd, target);
		GetWindowRect(hwnd, &g_last.rect);
		g_last.hwnd = hwnd;
		g_last.v_slot = next;
	}
}

void CenterWindow()
{
	HWND hwnd = GetForegroundWindow();
	if (!IsValidTarget(hwnd))
		return;

	RECT work = GetWorkArea(hwnd);
	RECT win;
	GetWindowRect(hwnd, &win);

	LONG ww = win.right - win.left;
	LONG wh = win.bottom - win.top;
	LONG cx = work.left + (work.right - work.left - ww) / 2;
	LONG cy = work.top + (work.bottom - work.top - wh) / 2;

	RECT target = {cx, cy, cx + ww, cy + wh};
	ApplyRect(hwnd, target);

	GetWindowRect(hwnd, &g_last.rect);
	g_last.hwnd = hwnd;
	// Don't update slot indices — centering is position-only, not ring navigation
}
