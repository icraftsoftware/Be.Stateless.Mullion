#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

constexpr auto ApplicationName = L"Mullion";

constexpr UINT WmCenter = WM_APP + 1;
constexpr UINT WmSnap = WM_APP + 2;
constexpr UINT WmTrayNotification = WM_APP + 3;

enum class MenuCommand // NOLINT(performance-enum-size)
{
	AutoStart = 100,
	Exit = 101,
	ReloadConfig = 102
};

enum class SnapDirection // NOLINT(performance-enum-size)
{
	Down = 0,
	Left = 1,
	Right = 2,
	Up = 3
};

enum class SnapModes // NOLINT(performance-enum-size)
{
	None = 0,
	Stretch = 0x1
};
