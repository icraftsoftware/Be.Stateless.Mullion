#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include "Tray.h"
#include "../../Resource.h"

using namespace Be::Stateless::Mullion::Windows::Shell;

Tray::Tray(const HWND hwnd) :
	data{
		.cbSize = sizeof(NOTIFYICONDATAW),
		.hWnd = hwnd,
		.uID = 1u,
		.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP,
		.uCallbackMessage = WmTrayNotification,
		.hIcon = LoadIcon(nullptr, IDI_APPLICATION),
	} // NOLINT(clang-diagnostic-missing-designated-field-initializers)
{
	wcscpy_s(data.szTip, ApplicationName);
	Shell_NotifyIconW(NIM_ADD, &data);
}

Tray::~Tray()
{
	Shell_NotifyIconW(NIM_DELETE, &data);
}

void Tray::HandleNotification(const LPARAM lparam) const
{
	switch (static_cast<UINT>(lparam))
	{
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
		ShowContextMenu();
		break;
	default:
		break;
	}
}

void Tray::ToggleAutostart()
{
	SetAutoStart(!GetAutoStart());
}

bool Tray::GetAutoStart()
{
	HKEY key;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, RegistryKey, 0, KEY_READ, &key) != ERROR_SUCCESS)
		return false;
	const bool found = RegQueryValueExW(key, ApplicationName, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
	RegCloseKey(key);
	return found;
}

void Tray::SetAutoStart(const bool enable)
{
	HKEY key;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, RegistryKey, 0, KEY_WRITE, &key) != ERROR_SUCCESS)
		return;
	if (enable)
	{
		wchar_t path[MAX_PATH];
		GetModuleFileNameW(nullptr, path, MAX_PATH);
		RegSetValueExW(key, ApplicationName, 0, REG_SZ,
			reinterpret_cast<const BYTE*>(path),
			static_cast<DWORD>((wcslen(path) + 1) * sizeof(wchar_t)));
	}
	else
	{
		RegDeleteValueW(key, ApplicationName);
	}
	RegCloseKey(key);
}

void Tray::ShowContextMenu() const
{
	const HMENU menu = CreatePopupMenu();
	const UINT autostartFlags = MF_STRING | (GetAutoStart() ? MF_CHECKED : MF_UNCHECKED);
	AppendMenuW(menu, autostartFlags, static_cast<UINT_PTR>(MenuCommand::AutoStart), L"Start with Windows");
	AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(MenuCommand::ReloadConfig), L"Reload Configuration");
	AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(MenuCommand::Exit), L"Exit");

	POINT pt;
	GetCursorPos(&pt);
	SetForegroundWindow(data.hWnd);
	TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_RIGHTALIGN, pt.x, pt.y, 0, data.hWnd, nullptr);
	DestroyMenu(menu);
}
