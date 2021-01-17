#include <Windows.h>
#undef min
#undef max
#include <functional>
#include <chrono>

#include <utils/window.hpp>
#include <utils/timer_thread.hpp>

class main_window final : public window
{
	virtual INT_PTR WindowProc(HWND, UINT message, WPARAM wParam, LPARAM lParam) override
	{
		switch (message)
		{
			HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
			HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);

		default:
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
		return 0;
	}
	BOOL OnCreate(HWND, LPCREATESTRUCT)
	{
		return TRUE;
	}
	void OnDestroy(HWND)
	{
		PostQuitMessage(0);
	}

private:
	timer_thread tt{ std::bind(&main_window::bg_routine, this), 500 };
	void bg_routine()
	{

	}
};

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	// 设置高 DPI 支持。
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	// 单例。
	HANDLE hMutex = CreateMutexW(nullptr, FALSE, L"Global\\SetMouse-29EC5FCE-B1F3-4F30-9A7C-73BD16382FC5");
	DWORD last_error = GetLastError();
	if (last_error == ERROR_ALREADY_EXISTS ||
		last_error == ERROR_ACCESS_DENIED && !hMutex)
	{
		if (hMutex)
			CloseHandle(hMutex);
		return 0;
	}
	else if (!hMutex)
		return 1;

	// 窗口与主循环。
	main_window wnd;
	wnd.create();
	wnd.message_loop();

	// 关闭单例中用到的句柄。
	CloseHandle(hMutex);
	return 0;
}