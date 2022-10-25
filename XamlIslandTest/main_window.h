#pragma once

#ifndef _WINDOWS_
#define _WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#ifndef WINRT_Windows_UI_Xaml_Controls_H
#include <winrt/Windows.UI.Xaml.Controls.h>
#endif
#ifndef WINRT_Microsoft_UI_Xaml_Controls_H
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#endif

#include "window_t.h"

class main_window : public window_t<main_window>
{
public:
	using my_base = window_t<main_window>;
	using my_type = main_window;
	explicit main_window(HINSTANCE inst);

	bool create_window(int cmdshow);

protected:
	friend class my_base;
	LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam);

	bool on_nccreate(const CREATESTRUCTW &);
	bool on_create(const CREATESTRUCTW &);
	void on_destroy();
	void on_size(UINT state, int cx, int cy);
private:
	void initialise_dpi();
	bool check_class_registered();
	void register_window_class();

	HINSTANCE m_instance = nullptr;
	inline static const wchar_t window_class[] = L"XamlIslandTestClass";
	winrt::Windows::UI::Xaml::Controls::Canvas m_canvas = nullptr;
	winrt::Windows::UI::Xaml::Controls::Button m_xaml_button = nullptr;
	HWND m_xaml_button_handle = nullptr;
	winrt::Windows::UI::Xaml::Controls::Button::Click_revoker m_xaml_button_click_revoker{};
	wil::unique_hwnd m_native_button1 = nullptr;
	wil::unique_hwnd m_native_button2 = nullptr;
	uint32_t m_window_dpi = 0;
	float_t m_window_dpi_scale = 0.f;
};