#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <optional>
#include "Windows/Shell/Tray.h"

namespace Be::Stateless::Mullion
{
	class Application
	{
	public:
		static int Run(HINSTANCE instance);

	private:
		static LRESULT CALLBACK HandleMessage(HWND window, UINT messageType, WPARAM wparam, LPARAM lparam);
		static void HandleCommand(HWND window, WPARAM wparam);
		inline static std::optional<Windows::Shell::Tray> tray; // NOLINT(clang-diagnostic-unique-object-duplication), we are a single executable
	};
}
