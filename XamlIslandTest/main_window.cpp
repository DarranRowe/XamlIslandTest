#include "pch.h"
#include "main_window.h"
#include "resource.h"

namespace wf = winrt::Windows::Foundation;
namespace mux = winrt::Microsoft::UI::Xaml;
namespace muxx = winrt::Microsoft::UI::Xaml::XamlTypeInfo;
namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
namespace wux = winrt::Windows::UI::Xaml;
namespace wuxc = winrt::Windows::UI::Xaml::Controls;

main_window::main_window(HINSTANCE inst) : m_instance(inst)
{
}

bool main_window::create_window(int cmdshow)
{
	register_window_class();
	DWORD styles = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;

	HWND window_handle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, my_type::window_class, L"Test Window", styles, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, m_instance, this);
	if (window_handle == nullptr)
	{
		return false;
	}

	ShowWindow(window_handle, cmdshow);
	UpdateWindow(window_handle);
	SetFocus(window_handle);

	return true;
}

LRESULT main_window::handle_message(UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_NCCREATE:
	{
		if (!on_nccreate(*reinterpret_cast<CREATESTRUCTW *>(lparam)))
		{
			return FALSE;
		}
		DefWindowProcW(get_handle(), msg, wparam, lparam);
		return TRUE;
	}
	case WM_CREATE:
	{
		if (!on_create(*reinterpret_cast<CREATESTRUCTW *>(lparam)))
		{
			return -1;
		}
		return 0;
	}
	case WM_DESTROY:
	{
		on_destroy();
		return 0;
	}
	case WM_SIZE:
	{
		on_size(static_cast<UINT>(wparam), LOWORD(lparam), HIWORD(lparam));
		return 0;
	}
	default:
	{
		return my_base::handle_message(msg, wparam, lparam);
	}
	}
	//return my_base::handle_message(msg, wparam, lparam);
}

bool main_window::on_nccreate(const CREATESTRUCTW &)
{
	initialise_dpi();

	return true;
}
bool main_window::on_create(const CREATESTRUCTW &)
{
	m_native_button1.reset(CreateWindowExW(0, L"Button", L"Test Button 1", WS_TABSTOP | WS_CHILD | BS_PUSHBUTTON | BS_NOTIFY | WS_VISIBLE, 0, 0, 150, 50, get_handle(), reinterpret_cast<HMENU>(101), m_instance, nullptr));

	m_canvas = LoadControlFromResource<wuxc::Canvas>(IDR_XAML_CONTROL);
	m_xaml_button = m_canvas.FindName(L"testButton").as<wuxc::Button>();
	m_xaml_button.Height(50);
	m_xaml_button.Width(100);
	m_xaml_button.HorizontalAlignment(wux::HorizontalAlignment::Left);
	m_xaml_button.VerticalAlignment(wux::VerticalAlignment::Top);

	m_xaml_button_handle = create_desktop_window_xaml_source(WS_TABSTOP, m_canvas);
	m_xaml_button_click_revoker = m_xaml_button.Click(winrt::auto_revoke, [](wf::IInspectable const &sender, wux::RoutedEventArgs const &) {
		static int click_count = 0;
		++click_count;
		auto s = std::format(L"Click Count: {}", click_count);
		sender.as<wuxc::Button>().Content(winrt::box_value(s.c_str()));
		});
	ShowWindow(m_xaml_button_handle, SW_SHOW);

	m_native_button2.reset(CreateWindowExW(0, L"Button", L"Test Button 2", WS_TABSTOP | WS_CHILD | BS_PUSHBUTTON | BS_NOTIFY | WS_VISIBLE, 0, 0, 150, 50, get_handle(), reinterpret_cast<HMENU>(102), m_instance, nullptr));

	return true;
}
void main_window::on_destroy()
{
	clear_xaml_islands();
	m_xaml_button_click_revoker.revoke();
	m_xaml_button = nullptr;
	m_xaml_button_handle = nullptr;
	my_base::on_destroy();
}
void main_window::on_size(UINT, int, int)
{
	if (m_native_button1)
	{
		//The button is 50 vpx tall and 150 vpx wide
		//Position it at 0,0

		SetWindowPos(m_native_button1.get(), nullptr, 0, 0, static_cast<int>(150 * m_window_dpi_scale), static_cast<int>(50 * m_window_dpi_scale), SWP_NOZORDER);
	}
	if (m_xaml_button_handle)
	{
		//The button is 50 vpx tall and 150 vpx wide
		//Position it at 160,0

		SetWindowPos(m_xaml_button_handle, nullptr, static_cast<int>(160 * m_window_dpi_scale), 0, static_cast<int>(160 * m_window_dpi_scale), static_cast<int>(60 * m_window_dpi_scale), SWP_NOZORDER);
		m_xaml_button.Width(150);
		m_xaml_button.Height(50);
	}
	if (m_native_button2)
	{
		//Use 50 vpx tall and 150 vpx wide
		//Position is at 320, 0

		SetWindowPos(m_native_button2.get(), nullptr, static_cast<int>(320 * m_window_dpi_scale), 0, static_cast<int>(150 * m_window_dpi_scale), static_cast<int>(50 * m_window_dpi_scale), SWP_NOZORDER);
	}
}
void main_window::initialise_dpi()
{
	m_window_dpi = GetDpiForWindow(get_handle());
	m_window_dpi_scale = m_window_dpi / 96.f;
}
bool main_window::check_class_registered()
{
	WNDCLASSEXW wcx{ sizeof(WNDCLASSEXW) };

	auto result = GetClassInfoExW(m_instance, my_type::window_class, &wcx);
	if (!result)
	{
		DWORD last_error = GetLastError();
		if (last_error == ERROR_CLASS_DOES_NOT_EXIST)
		{
			return false;
		}
		THROW_WIN32(last_error);
	}

	return true;
}
void main_window::register_window_class()
{
	if (check_class_registered())
	{
		return;
	}

	WNDCLASSEXW wcx{ sizeof(WNDCLASSEXW) };
	wcx.hInstance = m_instance;
	wcx.lpszClassName = my_type::window_class;
	wcx.lpfnWndProc = my_base::window_proc;
	wcx.style = CS_HREDRAW | CS_VREDRAW;

	wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wcx.hCursor = reinterpret_cast<HCURSOR>(LoadImageW(nullptr, MAKEINTRESOURCEW(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE | LR_DEFAULTCOLOR));
	wcx.hIcon = reinterpret_cast<HICON>(LoadImageW(nullptr, MAKEINTRESOURCEW(OIC_SAMPLE), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE | LR_DEFAULTCOLOR));
	wcx.hIconSm = reinterpret_cast<HICON>(LoadImageW(nullptr, MAKEINTRESOURCEW(OIC_SAMPLE), IMAGE_ICON, GetSystemMetricsForDpi(SM_CXSMICON, GetDpiForWindow(get_handle())), GetSystemMetricsForDpi(SM_CYSMICON, GetDpiForWindow(get_handle())), LR_SHARED | LR_DEFAULTCOLOR));

	THROW_IF_WIN32_BOOL_FALSE(static_cast<BOOL>(RegisterClassExW(&wcx)));
}