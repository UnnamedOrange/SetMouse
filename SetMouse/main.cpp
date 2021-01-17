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
	static void spi_interface(UINT uiAction, const int pvParam[3])
	{
		// 含危险代码。该函数不对外暴露，以保证安全性。
		if (!SystemParametersInfoW(uiAction, NULL, const_cast<int*>(pvParam), NULL))
			throw std::runtime_error("fail to SystemParametersInfoW.");
	}
public:
	static bool query_mouse_acceleration()
	{
		int params[3];
		spi_interface(SPI_GETMOUSE, params);
		return params[2];
	}
	static void enable_mouse_acceleration()
	{
		constexpr int params[3]{ 6, 10, 1 };
		spi_interface(SPI_SETMOUSE, params);
	}
	static void disable_mouse_acceleration()
	{
		constexpr int params[3]{};
		spi_interface(SPI_SETMOUSE, params);
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