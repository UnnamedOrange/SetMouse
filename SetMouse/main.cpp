﻿// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include <Windows.h>
#undef min
#undef max
#include <string>
#include <functional>
#include <chrono>
#include <optional>

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
		caption(L"SetMouse");
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
	/// <summary>
	/// 判断是否开启鼠标加速。
	/// </summary>
	/// <returns>一个布尔值，表示开启鼠标加速。</returns>
	static bool query_mouse_acceleration()
	{
		int params[3];
		spi_interface(SPI_GETMOUSE, params);
		return params[2];
	}
	/// <summary>
	/// 开启鼠标加速。
	/// </summary>
	static void enable_mouse_acceleration()
	{
		constexpr int params[3]{ 6, 10, 1 };
		spi_interface(SPI_SETMOUSE, params);
	}
	/// <summary>
	/// 关闭鼠标加速。
	/// </summary>
	static void disable_mouse_acceleration()
	{
		constexpr int params[3]{};
		spi_interface(SPI_SETMOUSE, params);
	}
	/// <summary>
	/// 打开或关闭鼠标加速。
	/// </summary>
	/// <param name="enable">是否打开鼠标加速。</param>
	static void set_mouse_acceleration(bool enable)
	{
		if (enable)
			enable_mouse_acceleration();
		else
			disable_mouse_acceleration();
	}

public:
	/// <summary>
	/// 获取窗口标题。
	/// </summary>
	/// <param name="hwnd">窗口句柄。</param>
	/// <returns>窗口标题。若窗口句柄无效，返回一个空字符串。</returns>
	static std::wstring MyGetWindowText(HWND hwnd)
	{
		std::wstring ret(GetWindowTextLengthW(hwnd), L'\0');
		if (ret.length())
			GetWindowTextW(hwnd, ret.data(), static_cast<int>(ret.length() + 1));
		return ret;
	}

private:
	const std::wstring matched_window_text{ L"SetMouse" };

private:
	timer_thread tt{ std::bind(&main_window::bg_routine, this), 100 };
	/// <summary>
	/// 背景例程。每过 500 毫秒执行一次。
	/// </summary>
	void bg_routine()
	{
		using namespace std::string_literals;
		bool matched = MyGetWindowText(GetForegroundWindow()) == matched_window_text;
		set_mouse_acceleration(!matched);
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
	bool previous_setting = main_window::query_mouse_acceleration();
	{
		main_window wnd;
		wnd.create();
		wnd.message_loop();
	}
	main_window::set_mouse_acceleration(previous_setting);

	// 关闭单例中用到的句柄。
	CloseHandle(hMutex);
	return 0;
}