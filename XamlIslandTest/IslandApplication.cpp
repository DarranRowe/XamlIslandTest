#include "pch.h"
#include "IslandApplication.h"
#include "IslandApplication.g.cpp"

namespace wf = winrt::Windows::Foundation;
namespace wfc = winrt::Windows::Foundation::Collections;
namespace ws = winrt::Windows::System;
namespace wuxh = winrt::Windows::UI::Xaml::Hosting;
namespace wuxi = winrt::Windows::UI::Xaml::Interop;
namespace wuxm = winrt::Windows::UI::Xaml::Markup;

namespace winrt::XamlIslandTest::implementation
{
	//Constructs with metadata providers.
	//This stores any provided providers and then initialises the manager.
	IslandApplication::IslandApplication(wfc::IVector<wuxm::IXamlMetadataProvider> const& providers)
	{
		for (auto provider : providers)
		{
			m_providers.Append(provider);
		}

		Initialize();
	}
	IslandApplication::~IslandApplication()
	{
		Close();
	}
	//Returns a reference to the WindowsXamlManager.
	//Returning the IClosable interface is fine because the only operation you
	//can perform is to close it.
	wf::IClosable IslandApplication::WindowsXamlManager() const
	{
		return m_xamlmanager;
	}
	bool IslandApplication::IsDisposed() const
	{
		return m_isclosed;
	}
	void IslandApplication::Initialize()
	{
		//If this has an outer object, obtain the metadata provider from this so it can participate in out
		//metadata lookup.
		const auto out = outer();
		if constexpr(out)
		{
			wuxm::IXamlMetadataProvider provider{};
			winrt::check_hresult(out->QueryInterface(winrt::guid_of<wuxm::IXamlMetadataProvider>(), winrt::put_abi(provider)));
			m_providers.Append(provider);
		}

		//Initialise the manager.
		m_xamlmanager = wuxh::WindowsXamlManager::InitializeForCurrentThread();
	}
	//Returns the metadata provider collection.
	wfc::IVector<wuxm::IXamlMetadataProvider> IslandApplication::MetadataProviders()
	{
		return m_providers;
	}
	//Closes the application.
	//This destroys the manager and providers and then exits the Xaml application.
	void IslandApplication::Close()
	{
		if (m_isclosed)
		{
			return;
		}

		m_isclosed = true;

		m_xamlmanager.Close();
		m_providers.Clear();
		m_xamlmanager = nullptr;
		Exit();
	}

	//Implements the metadata provider interface.
	//This just passes any requests through to the contained metadata provider collection.
	wuxm::IXamlType IslandApplication::GetXamlType(wuxi::TypeName const &type)
	{
		for (const auto &provider : m_providers)
		{
			const auto result = provider.GetXamlType(type);
			if (result)
			{
				return result;
			}
		}

		return nullptr;
	}
	wuxm::IXamlType IslandApplication::GetXamlType(winrt::hstring const &fullname)
	{
		for (const auto &provider : m_providers)
		{
			const auto result = provider.GetXamlType(fullname);
			if (result)
			{
				return result;
			}
		}

		return nullptr;
	}
	winrt::com_array<wuxm::XmlnsDefinition> IslandApplication::GetXmlnsDefinitions()
	{
		std::vector<wuxm::XmlnsDefinition> definitions;

		for (const auto &provider : m_providers)
		{
			auto defs = provider.GetXmlnsDefinitions();
			for (const auto &def : defs)
			{
				definitions.push_back(def);
			}
		}

		return winrt::com_array<wuxm::XmlnsDefinition>(definitions.begin(), definitions.end());
	}
}
