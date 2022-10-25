#include "pch.h"

#include "dynamicdependency.h"
#include "appdd.h"
#include "main_application.h"
#include "main_window.h"

struct com_init
{
	com_init()
	{
		winrt::init_apartment(winrt::apartment_type::single_threaded);
	}

	~com_init()
	{
		winrt::uninit_apartment();
	}
};
struct dd_init
{
	dd_init()
	{
		DDInitialiseWin11();
	}
	~dd_init()
	{
		DDCleanup();
	}
};
com_init g_apartment_init;
dd_init g_dynamic_dependency_init;

namespace xit = winrt::XamlIslandTest;
namespace wf = winrt::Windows::Foundation;
namespace wux = winrt::Windows::UI::Xaml;
namespace wuxc = winrt::Windows::UI::Xaml::Controls;
namespace wuxh = winrt::Windows::UI::Xaml::Hosting;
namespace wuxm = winrt::Windows::UI::Xaml::Markup;
namespace mux = winrt::Microsoft::UI::Xaml;

int application_main(HINSTANCE inst, LPWSTR, int cmdshow)
{
	int ret_val = 0;
	//This scope makes sure that the main_app reference is out of scope
	//when we destroy it. We don't want any dangling references.
	{
		mux::XamlTypeInfo::XamlControlsXamlMetaDataProvider md;
		main_application &main_app = main_application::get_application();
		main_app.initialise_xaml_host({ md });
		mux::Controls::XamlControlsResources res;
		main_app.merge_resources({ res });

		main_window window(inst);
		window.create_window(cmdshow);
		ret_val = main_app.run_message_loop();
	}

	main_application::get_application().close();

	return ret_val;
}

int WINAPI wWinMain(_In_ HINSTANCE inst, _In_opt_ HINSTANCE, _In_ LPWSTR cmdline, _In_ int cmdshow)
{
	int ret_val = 0;

	try
	{
		add_winui_dd();

		ret_val = application_main(inst, cmdline, cmdshow);

		remove_winui_dd();
	}
	catch (...)
	{

	}
	return ret_val;
}