#pragma once

#include "window_base.h"

template <typename T>
class window_t : public window_base
{
public:
	using my_type = window_t<T>;
	using my_t = T;

protected:
	void on_destroy()
	{
		PostQuitMessage(0);
	}

	void on_activate(uint16_t state, HWND, uint16_t)
	{
		if (state == WA_INACTIVE)
		{
			m_window_focus = GetFocus();
		}
	}

	void on_setfocus(HWND)
	{
		if (m_window_focus != nullptr)
		{
			SetFocus(m_window_focus);
		}
	}

	LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_DESTROY:
		{
			on_destroy();
			return 0;
		}
		case WM_ACTIVATE:
		{
			on_activate(LOWORD(wparam), reinterpret_cast<HWND>(lparam), HIWORD(wparam));
			return 0;
		}
		case WM_SETFOCUS:
		{
			on_setfocus(reinterpret_cast<HWND>(wparam));
			return 0;
		}
		case WM_USER_QUERY_WINDOWBASE:
		{
			return query_window_base_identified;
		}
		case WM_USER_GET_WINDOWBASE_POINTER:
		{
			return reinterpret_cast<LRESULT>(static_cast<window_base *>(this));
		}
		case WM_USER_VERIFY_POINTER:
		{
			window_base *ptr = reinterpret_cast<window_base *>(lparam);
			return ptr == static_cast<window_base *>(this) ? verify_window_base_pointer_match : verify_window_base_pointer_no_match;
		}
		}
		return DefWindowProcW(get_handle(), msg, wparam, lparam);
	}

	static LRESULT CALLBACK window_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (msg != WM_NCCREATE)
		{
			my_t *that = instance_from_handle(wnd);
			if (that)
			{
				if (msg != WM_NCDESTROY)
				{
					return that->handle_message(msg, wparam, lparam);
				}
				else
				{
					auto result = that->handle_message(msg, wparam, lparam);
					process_ncdestroy(wnd);
					return result;
				}
			}
		}
		else
		{
			if (!process_nccreate(wnd, *reinterpret_cast<CREATESTRUCTW *>(lparam)))
			{
				return FALSE;
			}
			my_t *that = instance_from_handle(wnd);
			return that->handle_message(msg, wparam, lparam);
		}

		return DefWindowProcW(wnd, msg, wparam, lparam);
	}

	static my_t *instance_from_handle(HWND wnd)
	{
		my_t *ptr = reinterpret_cast<my_t *>(GetWindowLongPtrW(wnd, GWLP_USERDATA));
		return ptr;
	}
	static bool process_nccreate(HWND wnd, const CREATESTRUCTW &cs)
	{
		SetLastError(ERROR_SUCCESS);
		auto result = SetWindowLongPtrW(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs.lpCreateParams));
		_ASSERTE(result == 0);
		DWORD last_error = GetLastError();
		if (last_error != ERROR_SUCCESS)
		{
			THROW_WIN32(last_error);
		}

		my_t *ptr = instance_from_handle(wnd);
		ptr->set_handle(wnd);

		return true;
	}
	static void process_ncdestroy(HWND wnd)
	{
		SetWindowLongPtrW(wnd, GWLP_USERDATA, 0);
	}

private:
	HWND m_window_focus = nullptr;
};

template<typename T>
auto LoadControlFromFile(std::wstring const &file_name) -> T
{
	return LoadControlFromFile(file_name).as<T>();
}
template<typename T>
auto LoadControlFromResource(uint16_t id) -> T
{
	return LoadControlFromResource(id).as<T>();
}