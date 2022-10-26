#include "pch.h"

#include "dynamicdependency.h"
#include "appdd.h"
#include "main_application.h"
#include "main_window.h"

//Controls Com/WinRT lifetime.
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
//Controls the WinUI dynamic dependency lifetime.
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

namespace muxx = winrt::Microsoft::UI::Xaml::XamlTypeInfo;
namespace muxc = winrt::Microsoft::UI::Xaml::Controls;

void except_filter()
{
	try
	{
		throw;
	}
	catch (winrt::hresult_error &)
	{
	}
	catch (wil::ResultException &)
	{
	}
	catch (std::exception &)
	{
	}
}

int application_main(HINSTANCE inst, LPWSTR, int cmdshow)
{
	int ret_val = 0;
	//This scope makes sure that the main_app reference is out of scope
	//when we destroy it. We don't want any dangling references.
	{
		//Load the WinUI metadata provider.
		muxx::XamlControlsXamlMetaDataProvider md;
		//Obtains the application.
		main_application &main_app = main_application::get_application();
		//Initialise the application.
		//This passes the metadata provider through to the Xaml application.
		main_app.initialise_xaml_host({ md });
		//Loads the WinUI control resources.
		muxc::XamlControlsResources res;
		//Merge the resources into the xaml merged dictionaries.
		main_app.merge_resources({ res });

		//Create and show the main window.
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
		except_filter();
	}
	return ret_val;
}