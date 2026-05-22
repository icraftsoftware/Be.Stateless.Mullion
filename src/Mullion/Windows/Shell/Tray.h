#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

namespace Be::Stateless::Mullion::Windows::Shell
{
	class Tray
	{
	public:
		explicit Tray(HWND hwnd);
		~Tray();
		Tray(const Tray&) = delete;
		Tray& operator=(const Tray&) = delete;
		Tray(Tray&&) = delete;
		Tray& operator=(Tray&&) = delete;

		void HandleNotification(LPARAM lparam) const;
		static void ToggleAutostart();

	private:
		static constexpr auto RegistryKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
		static bool GetAutoStart();
		static void SetAutoStart(bool enable);
		void ShowContextMenu() const;
		NOTIFYICONDATAW data{};
	};
}
