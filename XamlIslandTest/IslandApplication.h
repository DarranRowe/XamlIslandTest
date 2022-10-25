#pragma once
#include "IslandApplication.g.h"

namespace winrt::XamlIslandTest::implementation
{
	struct IslandApplication : IslandApplicationT<IslandApplication, winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider>
	{
		IslandApplication() = default;

		IslandApplication(winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> const& providers);
		~IslandApplication();

		winrt::Windows::Foundation::IClosable WindowsXamlManager() const;
		bool IsDisposed() const;
		void Initialize();
		winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> MetadataProviders();
		void Close();

		winrt::Windows::UI::Xaml::Markup::IXamlType GetXamlType(winrt::Windows::UI::Xaml::Interop::TypeName const &type);
		winrt::Windows::UI::Xaml::Markup::IXamlType GetXamlType(winrt::hstring const &fullname);
		winrt::com_array<winrt::Windows::UI::Xaml::Markup::XmlnsDefinition> GetXmlnsDefinitions();

	private:

		bool m_isclosed = false;
		winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager m_xamlmanager = nullptr;
		winrt::Windows::Foundation::Collections::IVector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> m_providers = winrt::single_threaded_vector<winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider>();
	};
}
namespace winrt::XamlIslandTest::factory_implementation
{
	struct IslandApplication : IslandApplicationT<IslandApplication, implementation::IslandApplication>
	{
	};
}
