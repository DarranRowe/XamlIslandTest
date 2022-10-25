#pragma once

#define _WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#include <sdkddkver.h>
#include <Windows.h>
#include <ShlObj.h>

#undef GetCurrentTime

#include <cinttypes>
#include <filesystem>
#include <vector>
#include <string>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include <winrt/XamlIslandTest.h>
#include <wil/cppwinrt_helpers.h>
#include <wil/result.h>
#include <wil/resource.h>
#include <wil/com.h>