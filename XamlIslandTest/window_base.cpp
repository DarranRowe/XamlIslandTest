#include "pch.h"
#include "window_base.h"

#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>

namespace wf = winrt::Windows::Foundation;
namespace wux = winrt::Windows::UI::Xaml;
namespace wuxh = winrt::Windows::UI::Xaml::Hosting;
namespace wuxm = winrt::Windows::UI::Xaml::Markup;

constexpr uint16_t xamlresourcetype = 255;
constexpr auto static invalid_reason = static_cast<wuxh::XamlSourceFocusNavigationReason>(-1);
constexpr static WPARAM invalid_key = static_cast<WPARAM>(-1);

HWND window_base::get_handle() const
{
	return m_handle;
}
void window_base::set_handle(HWND handle)
{
	m_handle = handle;
}

//Takes a VK code from a WM_KEYDOWN message and converts it to
//a xaml XamlSourceFocusNavigationReason value.
//Tab goes to first or last depending on whether shift has been pressed.
wuxh::XamlSourceFocusNavigationReason get_reason_from_key(WPARAM key)
{
	auto reason = invalid_reason;
	switch(key)
	{
	case VK_TAB:
	{
		byte keyboard_state[256] = {};
		THROW_IF_WIN32_BOOL_FALSE(GetKeyboardState(keyboard_state));
		reason = (keyboard_state[VK_SHIFT] & 0x80) ? wuxh::XamlSourceFocusNavigationReason::Last : wuxh::XamlSourceFocusNavigationReason::First;
		break;
	}
	case VK_LEFT:
	{
		reason = wuxh::XamlSourceFocusNavigationReason::Left;
		break;
	}
	case VK_RIGHT:
	{
		reason = wuxh::XamlSourceFocusNavigationReason::Right;
		break;
	}
	case VK_UP:
	{
		reason = wuxh::XamlSourceFocusNavigationReason::Up;
		break;
	}
	case VK_DOWN:
	{
		reason = wuxh::XamlSourceFocusNavigationReason::Down;
		break;
	}
	}

	return reason;
}
//Takes a XamlSourceFocusNavigationReason value and converts it to a VK
//code.
WPARAM get_key_from_reason(wuxh::XamlSourceFocusNavigationReason reason)
{
	auto key = invalid_key;

	switch (reason)
	{
	case wuxh::XamlSourceFocusNavigationReason::Last:
	{
		key = VK_TAB;
		break;
	}
	case wuxh::XamlSourceFocusNavigationReason::First:
	{
		key = VK_TAB;
		break;
	}
	case wuxh::XamlSourceFocusNavigationReason::Left:
	{
		key = VK_LEFT;
		break;
	}
	case wuxh::XamlSourceFocusNavigationReason::Right:
	{
		key = VK_RIGHT;
		break;
	}
	case wuxh::XamlSourceFocusNavigationReason::Up:
	{
		key = VK_UP;
		break;
	}
	case wuxh::XamlSourceFocusNavigationReason::Down:
	{
		key = VK_DOWN;
		break;
	}
	}

	return key;
}

wuxh::DesktopWindowXamlSource window_base::get_next_focused_island(const MSG *msg)
{
	//This only happens if we are working with a key down mesage.
	if (msg->message == WM_KEYDOWN)
	{
		//Obtains the reason from the WM_KEYDOWN's virtual key code.
		const auto key = msg->wParam;
		auto reason = get_reason_from_key(key);
		if (reason != invalid_reason)
		{
			//This figures the direction of navigation.
			//First, Down and Right moves forward in the control order.
			//Last, Left and Up moves backward in the control order.
			const bool previous = ((reason == wuxh::XamlSourceFocusNavigationReason::First) || (reason == wuxh::XamlSourceFocusNavigationReason::Down) || (reason == wuxh::XamlSourceFocusNavigationReason::Right)) ? false : true;
			//Obtains the currently focused window.
			const auto current_focused_window = GetFocus();
			//Uses the GetNextDlgTabItem to get focus next.
			//This basically searches for the next control with the WS_TABSTOP style.
			//This is where we use the direction calculated above. We want to work out
			//whether we want to get the previous or next window.
			const auto next_element = GetNextDlgTabItem(get_handle(), current_focused_window, previous);
			//Go through the cached xaml source objects.
			//If the window handle we want to change focus to matches one of the xaml sources window handles
			//then that is the xaml source that we wish to navigate to.
			for (auto &xaml_source : m_xaml_sources)
			{
				HWND island_window = get_handle(xaml_source);
				if (next_element == island_window)
				{
					return xaml_source;
				}
			}
		}
	}

	return nullptr;
}
wuxh::DesktopWindowXamlSource window_base::get_focused_island()
{
	for (auto &xaml_source : m_xaml_sources)
	{
		if (xaml_source.HasFocus())
		{
			return xaml_source;
		}
	}

	return nullptr;
}
bool window_base::navigate_focus(MSG *msg)
{
	if (const auto next_focused_island = get_next_focused_island(msg))
	{
		//This assert fires when there is a single window since the next window
		//is the first window.
		//_ASSERTE(!next_focused_island.HasFocus());
		const auto previous_focused_window = GetFocus();
		RECT rc_prev{};
		THROW_IF_WIN32_BOOL_FALSE(GetWindowRect(previous_focused_window, &rc_prev));
		HWND island_window = get_handle(next_focused_island);

		POINT pt = { rc_prev.left, rc_prev.top };
		SIZE sz = { rc_prev.right - rc_prev.left, rc_prev.bottom - rc_prev.top };
		ScreenToClient(island_window, &pt);
		const auto hint_rect = wf::Rect({static_cast<float>(pt.x), static_cast<float>(pt.y), static_cast<float>(sz.cx), static_cast<float>(sz.cy)});
		const auto reason = get_reason_from_key(msg->wParam);
		const auto request = wuxh::XamlSourceFocusNavigationRequest(reason, hint_rect);
		m_last_focus_request_id = request.CorrelationId();
		const auto result = next_focused_island.NavigateFocus(request);
		auto fm = result.WasFocusMoved();
		if (fm)
		{
			SetFocus(island_window);
		}
		return fm;
	}
	else
	{
		const bool island_is_focused = get_focused_island() != nullptr;
		byte keyboard_state[256] = {};
		THROW_IF_WIN32_BOOL_FALSE(GetKeyboardState(keyboard_state));
		const bool is_menu_modifier = (keyboard_state[VK_MENU] & 0x80);
		if (island_is_focused && !is_menu_modifier)
		{
			return false;
		}
		const bool is_dialog_message = !!IsDialogMessageW(get_handle(), msg);
		return is_dialog_message;
	}
}

//The GotFocus event.
//The message box never shows.
void window_base::on_got_focus(wuxh::DesktopWindowXamlSource const &sender, wuxh::DesktopWindowXamlSourceGotFocusEventArgs const &)
{
	MessageBoxW(get_handle(), L"GotFocusEvent", L"GotFocusEvent", MB_OK);
	HWND island_handle = get_handle(sender);

	if (GetFocus() != island_handle)
	{
		OutputDebugStringW(L"GotFocus fired, HWND does not have focus.");
	}
}

void window_base::on_take_focus_requested(wuxh::DesktopWindowXamlSource const & sender, wuxh::DesktopWindowXamlSourceTakeFocusRequestedEventArgs const &args)
{
	HWND sender_handle = get_handle(sender);

	if (args.Request().CorrelationId() != m_last_focus_request_id)
	{
		const auto reason = args.Request().Reason();
		const bool previous = ((reason == wuxh::XamlSourceFocusNavigationReason::First) || (reason == wuxh::XamlSourceFocusNavigationReason::Down) || (reason == wuxh::XamlSourceFocusNavigationReason::Right)) ? false : true;

		MSG msg{};
		msg.hwnd = sender_handle;
		msg.message = WM_KEYDOWN;
		msg.wParam = get_key_from_reason(reason);
		if (!navigate_focus(&msg))
		{
			const auto next_element = GetNextDlgTabItem(get_handle(), sender_handle, previous);
			SetFocus(next_element);
		}
	}
	else
	{
		const auto request = wuxh::XamlSourceFocusNavigationRequest(wuxh::XamlSourceFocusNavigationReason::Restore);
		m_last_focus_request_id = request.CorrelationId();
		sender.NavigateFocus(request);
		SetFocus(sender_handle);
	}
}

bool window_base::focus_navigate(MSG *msg)
{
	return navigate_focus(msg);
}

std::vector<wuxh::DesktopWindowXamlSource> window_base::get_xaml_sources()
{
	std::vector<wuxh::DesktopWindowXamlSource> sources;

	for (auto &xaml_source : m_xaml_sources)
	{
		sources.push_back(xaml_source);
	}

	return sources;
}
HWND window_base::create_desktop_window_xaml_source(DWORD extra_styles, const wux::UIElement &content)
{
	wuxh::DesktopWindowXamlSource desktop_source;
	auto interop = desktop_source.as<IDesktopWindowXamlSourceNative2>();
	winrt::check_hresult(interop->AttachToWindow(get_handle()));
	HWND xaml_source_handle{};
	winrt::check_hresult(interop->get_WindowHandle(&xaml_source_handle));
	const DWORD ex_style = static_cast<DWORD>(GetWindowLongPtrW(xaml_source_handle, GWL_STYLE)) | extra_styles;
	SetWindowLongPtrW(xaml_source_handle, GWL_STYLE, static_cast<LONG_PTR>(ex_style));

	desktop_source.Content(content);
	m_take_focus_event_tokens.emplace(std::make_pair(xaml_source_handle, desktop_source.TakeFocusRequested({ this, &window_base::on_take_focus_requested })));
	//The GotFocus event is wired up here.
	m_got_focus_event_tokens.emplace(std::make_pair(xaml_source_handle, desktop_source.GotFocus({ this, &window_base::on_got_focus })));
	m_xaml_sources.push_back(desktop_source);

	return xaml_source_handle;
}
void window_base::clear_xaml_islands()
{
	for (auto &xaml_source : m_xaml_sources)
	{
		HWND xaml_source_handle = get_handle(xaml_source);
		auto take_focus_it = m_take_focus_event_tokens.find(xaml_source_handle);
		_ASSERTE(take_focus_it != m_take_focus_event_tokens.end());
		xaml_source.TakeFocusRequested((*take_focus_it).second);
		auto got_focus_it = m_got_focus_event_tokens.find(xaml_source_handle);
		_ASSERTE(got_focus_it != m_got_focus_event_tokens.end());
		xaml_source.GotFocus((*got_focus_it).second);
		xaml_source.Close();
	}
	m_take_focus_event_tokens.clear();
	m_got_focus_event_tokens.clear();
	m_xaml_sources.clear();
}

HWND window_base::get_handle(wuxh::DesktopWindowXamlSource const &source)
{
	HWND island_handle{};

	winrt::check_hresult(source.as<IDesktopWindowXamlSourceNative>()->get_WindowHandle(&island_handle));
	return island_handle;
}

wux::UIElement LoadControlFromFile(std::wstring const &file_name)
{
	std::filesystem::path file_path = file_name;
	if (!std::filesystem::exists(file_path))
	{
		THROW_HR(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
	}

	auto file_size = std::filesystem::file_size(file_path);

	wil::unique_file xaml_control;
	std::wstring file_content;

	auto result = _wfopen_s(xaml_control.addressof(), file_path.c_str(), L"rb");
	if (result != 0)
	{
		THROW_HR(E_FAIL);
	}
	
	while (!feof(xaml_control.get()))
	{
		int val = fgetc(xaml_control.get());
		if (val != EOF)
		{
			file_content += static_cast<wchar_t>(val);
		}
	}

	if (file_content.size() < file_size)
	{
		THROW_HR(E_FAIL);
	}

	return wuxm::XamlReader::Load(static_cast<winrt::hstring>(file_content)).as<wux::UIElement>();
}
wux::UIElement LoadControlFromResource(uint16_t id)
{
	auto resource_handle = FindResourceW(nullptr, MAKEINTRESOURCEW(id), MAKEINTRESOURCEW(xamlresourcetype));
	THROW_LAST_ERROR_IF(!resource_handle);

	HGLOBAL resource_data = LoadResource(nullptr, resource_handle);
	THROW_LAST_ERROR_IF(!resource_data);

	auto data = static_cast<char *>(LockResource(resource_data));
	auto hstr_data = winrt::to_hstring(data);
	return wuxm::XamlReader::Load(hstr_data).as<wux::UIElement>();
}