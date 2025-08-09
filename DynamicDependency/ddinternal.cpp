#include <string>
#include <mutex>
#include <cinttypes>
#include <Windows.h>
#include <strsafe.h>
#include <winrt/base.h>
#include <wil/result.h>
#include <wil/resource.h>
#include <MddBootstrap.h>
#include <MsixDynamicDependency.h>
#include <WindowsAppSDK-VersionInfo.h>
#include "ddinternal.h"

#pragma comment(lib, "onecoreuap.lib")

#if NTDDI_VERSION < NTDDI_WIN10_CO
#error This file requires the Windows 11 SDK.
#endif

constexpr wchar_t g_apiset_namew[] = L"api-ms-win-appmodel-runtime-l1-1-5";
constexpr char g_apiset_namea[] = "api-ms-win-appmodel-runtime-l1-1-5";
constexpr wchar_t g_apiset_libw[] = L"api-ms-win-appmodel-runtime-l1-1-5.dll";
constexpr char g_apiset_liba[] = "api-ms-win-appmodel-runtime-l1-1-5.dll";
constexpr wchar_t g_bootstrapper_namew[] = L"Microsoft.WindowsAppRuntime.Bootstrap.dll";
constexpr char g_bootstrapper_namea[] = "Microsoft.WindowsAppRuntime.Bootstrap.dll";

constexpr wchar_t g_winappsdk_family_name_basew[] = L"Microsoft.WindowsAppRuntime.{}.{}_8wekyb3d8bbwe";
constexpr char g_winappsdk_family_name_basea[] = "Microsoft.WindowsAppRuntime.{}.{}_8wekyb3d8bbwe";

static internal_state g_internal_state{WINDOWSAPPSDK_RELEASE_MAJOR, WINDOWSAPPSDK_RELEASE_MINOR, 0, 0, initialisation_state::uninitialised, false};
static std::mutex g_internal_state_mutex{};

static wil::unique_hmodule g_appmodel_module = nullptr;

//The typedefs use the Windows 11 types, meaning that this requires the Windows 11 SDK
//The call functions will properly cast though.
using Win11TCPD = decltype(&TryCreatePackageDependency);
using Win11APD = decltype(&AddPackageDependency);
using Win11RPD = decltype(&RemovePackageDependency);
using Win11DPD = decltype(&DeletePackageDependency);
using Win11GRPFNFPD = decltype(&GetResolvedPackageFullNameForPackageDependency);
using Win11GIFPDC = decltype(&GetIdForPackageDependencyContext);

using MddBI2 = decltype(&MddBootstrapInitialize2);

static Win11TCPD Win11TryCreatePackageDependency = nullptr;
static Win11DPD Win11DeletePackageDependency = nullptr;
static Win11APD Win11AddPackageDependency = nullptr;
static Win11RPD Win11RemovePackageDependency = nullptr;
static Win11GRPFNFPD Win11GetResolvedPackageFullNameForPackageDependency = nullptr;
static Win11GIFPDC Win11GetIdForPackageDependencyContext = nullptr;

extern"C"
{
static bool query_wappsdk_availability_version(uint16_t ver_major, uint16_t ver_minor)
{
	auto formatted_string = std::format(g_winappsdk_family_name_basew, ver_major, ver_minor);

	//Use GetPackagesByPackageFamily to check for the framework package availability.
	UINT32 count = 0;
	UINT32 buffer = 0;
	auto gpbpfresult = GetPackagesByPackageFamily(formatted_string.c_str(), &count, nullptr, &buffer, nullptr);
	//The function returns 87 (ERROR_INSUFFICIENT_BUFFER) on success and there are some available
	//packages. It is not possible to blindly check for ERROR_SUCCESS.
	_ASSERTE(gpbpfresult == ERROR_SUCCESS || gpbpfresult == ERROR_INSUFFICIENT_BUFFER);

	if (count == 0)
	{
		//When count is 0, this means that there are no packages
		//with this package family, so return false to indicate this.
		return false;
	}

	if (count > 0)
	{
		//This is lacking at the moment. This assumes that the entire runtime
		//is present.
		//If the DDLM package isn't available then there will be failures.
		return true;
	}

	return false;
}

static constexpr uint32_t make_major_minor(uint16_t ver_major, uint16_t ver_minor)
{
	return static_cast<uint32_t>(ver_major) << 16 | static_cast<uint32_t>(ver_minor);
}

static bool init_wappsdk(uint16_t ver_major, uint16_t ver_minor)
{
	{
		std::scoped_lock<std::mutex> sl{ g_internal_state_mutex };
		//This function calls MddBootstrapInitialize, this means we will have a
		//dependency on the bootstrapper library, meaning that the library is
		//guaranteed to be loaded by the time this function is called.
		//Because of this, we can just use GetModuleHandle to get the handle.
		HMODULE bootstrapper_instance = GetModuleHandleW(g_bootstrapper_namew);
		_ASSERTE(bootstrapper_instance != nullptr);

		//Check for the existance of MddBootstrapInitialize2, this is preferred if available.
		MddBI2 ptr_MddBootstrapInitialize2 = reinterpret_cast<MddBI2>(GetProcAddress(bootstrapper_instance, "MddBootstrapInitialize2"));
		if (ptr_MddBootstrapInitialize2)
		{
			//The empty string for the release version means that we only want release versions, not preview or experimental.
			//The empty PACKAGE_VERSION means that we are fine with any version of the SDK.
			if (FAILED(ptr_MddBootstrapInitialize2(make_major_minor(ver_major, ver_minor), L"", {}, MddBootstrapInitializeOptions_OnPackageIdentity_NOOP | MddBootstrapInitializeOptions_OnError_DebugBreak_IfDebuggerAttached)))
			{
				return false;
			}
		}
		else
		{
			//The empty string for the release version means that we only want release versions, not preview or experimental.
			//The empty PACKAGE_VERSION means that we are fine with any version of the SDK.
			if (FAILED(MddBootstrapInitialize(make_major_minor(ver_major, ver_minor), L"", {})))
			{
				return false;
			}
		}

		g_internal_state.dd_init_state = initialisation_state::initialised_with_app_sdk;
	}

	return true;
}

static void cleanup_wappsdk()
{
	if (is_initialised())
	{
		_ASSERTE(!is_using_win11());

		{
			std::scoped_lock<std::mutex> sl{ g_internal_state_mutex };
			g_internal_state.dd_init_state = initialisation_state::uninitialised;

			MddBootstrapShutdown();
		}
	}
}

static bool init_win11()
{
	{
		std::scoped_lock<std::mutex> sl{ g_internal_state_mutex };
		//Load the appmodel API set library.
		g_appmodel_module.reset(LoadLibraryW(g_apiset_libw));
		_ASSERTE(g_appmodel_module != nullptr);

		Win11TryCreatePackageDependency = reinterpret_cast<Win11TCPD>(GetProcAddress(g_appmodel_module.get(), "TryCreatePackageDependency"));
		_ASSERTE(Win11TryCreatePackageDependency != nullptr);
		Win11RemovePackageDependency = reinterpret_cast<Win11RPD>(GetProcAddress(g_appmodel_module.get(), "RemovePackageDependency"));
		_ASSERTE(Win11RemovePackageDependency != nullptr);
		Win11AddPackageDependency = reinterpret_cast<Win11APD>(GetProcAddress(g_appmodel_module.get(), "AddPackageDependency"));
		_ASSERTE(Win11AddPackageDependency != nullptr);
		Win11DeletePackageDependency = reinterpret_cast<Win11DPD>(GetProcAddress(g_appmodel_module.get(), "DeletePackageDependency"));
		_ASSERTE(Win11DeletePackageDependency != nullptr);
		Win11GetResolvedPackageFullNameForPackageDependency = reinterpret_cast<Win11GRPFNFPD>(GetProcAddress(g_appmodel_module.get(), "GetResolvedPackageFullNameForPackageDependency"));
		_ASSERTE(Win11GetResolvedPackageFullNameForPackageDependency != nullptr);
		Win11GetIdForPackageDependencyContext = reinterpret_cast<Win11GIFPDC>(GetProcAddress(g_appmodel_module.get(), "GetIdForPackageDependencyContext"));
		_ASSERTE(Win11GetIdForPackageDependencyContext != nullptr);

		g_internal_state.dd_init_state = initialisation_state::initialised_with_win11;
	}
	return true;
}

static void cleanup_win11()
{
	{
		std::scoped_lock<std::mutex> sl{ g_internal_state_mutex };

		g_internal_state.dd_init_state = initialisation_state::uninitialised;
		Win11TryCreatePackageDependency = nullptr;
		Win11RemovePackageDependency = nullptr;
		Win11AddPackageDependency = nullptr;
		Win11DeletePackageDependency = nullptr;
		Win11GetResolvedPackageFullNameForPackageDependency = nullptr;
		Win11GetIdForPackageDependencyContext = nullptr;

		g_appmodel_module.reset(nullptr);
	}
}

void set_version_override(uint16_t ver_major, uint16_t ver_minor)
{
	{
		std::scoped_lock<std::mutex> sl{g_internal_state_mutex};
		g_internal_state.wappsdk_override_major = ver_major;
		g_internal_state.wappsdk_override_minor = ver_minor;
		g_internal_state.override_set = true;
	}
}

void remove_version_override()
{
	{
		std::scoped_lock<std::mutex> sl{ g_internal_state_mutex };
		g_internal_state.wappsdk_override_major = 0;
		g_internal_state.wappsdk_override_minor = 0;
		g_internal_state.override_set = false;
	}
}

bool query_os_capability()
{
	//Checks if the API set is available.
	//It must be at least 1.1.5 for the
	//Dynamic Dependency API to be part of the
	//contract.
	return IsApiSetImplemented(g_apiset_namea);
}

bool query_wappsdk_availability_override()
{
	uint16_t override_major{}, override_minor{};
	{
		std::scoped_lock<std::mutex> sl{ g_internal_state_mutex };

		if (!g_internal_state.override_set)
		{
			return false;
		}

		override_major = g_internal_state.wappsdk_override_major;
		override_minor = g_internal_state.wappsdk_override_minor;
	}

	return query_wappsdk_availability_version(override_major, override_minor);
}

bool query_wappsdk_availability_default()
{
	uint16_t default_major{}, default_minor{};
	{
		std::scoped_lock<std::mutex> sl{ g_internal_state_mutex };

		default_major = g_internal_state.wappsdk_default_major;
		default_minor = g_internal_state.wappsdk_default_minor;
	}

	return query_wappsdk_availability_version(default_major, default_minor);
}

bool query_wappsdk_availability()
{
	bool query_override = false;
	{
		std::scoped_lock<std::mutex> sl{ g_internal_state_mutex };

		if (g_internal_state.override_set)
		{
			query_override = true;
		}
	}

	return query_override == true ? query_wappsdk_availability_override() : query_wappsdk_availability_default();
}

bool initialise_pointers_win11()
{
	if (!query_os_capability())
	{
		return false;
	}

	return init_win11();
}

bool initialise_pointers_wappsdk()
{
	//This queries the version to use from the state.
	//If a version override is set then that is used, otherwise
	//this will use the verison of the Windows App SDK that this
	//library was built against.
	uint16_t major_version{}, minor_version{};

	{
		std::scoped_lock<std::mutex> sl{ g_internal_state_mutex };

		if (g_internal_state.override_set)
		{
			major_version = g_internal_state.wappsdk_override_major;
			minor_version = g_internal_state.wappsdk_override_minor;
		}
		else
		{
			major_version = g_internal_state.wappsdk_default_major;
			minor_version = g_internal_state.wappsdk_default_minor;
		}
	}

	//Check if the version is available.
	if (!query_wappsdk_availability_version(major_version, minor_version))
	{
		return false;
	}

	return init_wappsdk(major_version, minor_version);
}

bool initialise_pointers_auto()
{
	//This checks for O/S support first.
	//If this is supported by Windows then
	//we use the Windows 11 Dynamic Dependency API.
	//If not then we fall back to the Windows App
	//SDK.

	if (query_os_capability())
	{
		return init_win11();
	}

	return initialise_pointers_wappsdk();
}

void cleanup_pointers()
{
	if (is_initialised())
	{
		if (is_using_win11())
		{
			cleanup_win11();
		}
		else
		{
			cleanup_wappsdk();
		}
	}
}

bool is_initialised()
{
	return (g_internal_state.dd_init_state == initialisation_state::initialised_with_app_sdk) || (g_internal_state.dd_init_state == initialisation_state::initialised_with_win11);
}
bool is_using_win11()
{
	return g_internal_state.dd_init_state == initialisation_state::initialised_with_win11;
}

HRESULT call_TryCreatePackageDependency(
	PSID user,
	_In_ PCWSTR packageFamilyName,
	PACKAGE_VERSION minVersion,
	PackageDependencyProcessorArchitectures packageDependencyProcessorArchitectures,
	PackageDependencyLifetimeKind lifetimeKind,
	PCWSTR lifetimeArtifact,
	CreatePackageDependencyOptions options,
	_Outptr_result_maybenull_ PWSTR *packageDependencyId
	)
{
	_ASSERTE(is_initialised() == true);
	if (is_using_win11())
	{
		_ASSERTE(Win11TryCreatePackageDependency != nullptr);
		return Win11TryCreatePackageDependency(user, packageFamilyName, minVersion, packageDependencyProcessorArchitectures, lifetimeKind, lifetimeArtifact, options, packageDependencyId);
	}
	return MddTryCreatePackageDependency(
		user,
		packageFamilyName,
		minVersion,
		static_cast<MddPackageDependencyProcessorArchitectures>(packageDependencyProcessorArchitectures),
		static_cast<MddPackageDependencyLifetimeKind>(lifetimeKind),
		lifetimeArtifact,
		static_cast<MddCreatePackageDependencyOptions>(options),
		packageDependencyId
	);
}

HRESULT call_DeletePackageDependency(
	_In_ PCWSTR packageDependencyId
	)
{
	_ASSERTE(is_initialised() == true);
	if (is_using_win11())
	{
		_ASSERTE(Win11DeletePackageDependency != nullptr);
		return Win11DeletePackageDependency(packageDependencyId);
	}
	MddDeletePackageDependency(packageDependencyId);
	return S_OK;
}

HRESULT call_AddPackageDependency(
	_In_ PCWSTR packageDependencyId,
	INT32 rank,
	AddPackageDependencyOptions options,
	_Out_ PACKAGEDEPENDENCY_CONTEXT *packageDependencyContext,
	_Outptr_opt_result_maybenull_ PWSTR *packageFullName
	)
{
	_ASSERTE(is_initialised() == true);
	if (is_using_win11())
	{
		_ASSERTE(Win11AddPackageDependency != nullptr);
		return Win11AddPackageDependency(packageDependencyId, rank, options, packageDependencyContext, packageFullName);
	}
	return MddAddPackageDependency(
		packageDependencyId,
		rank,
		static_cast<MddAddPackageDependencyOptions>(options),
		reinterpret_cast<MDD_PACKAGEDEPENDENCY_CONTEXT *>(packageDependencyContext),
		packageFullName
	);
}

HRESULT call_RemovePackageDependency(
	_In_ PACKAGEDEPENDENCY_CONTEXT packageDependencyContext
	)
{
	_ASSERTE(is_initialised() == true);
	if (is_using_win11())
	{
		_ASSERTE(Win11RemovePackageDependency != nullptr);
		return Win11RemovePackageDependency(packageDependencyContext);
	}
	MddRemovePackageDependency(reinterpret_cast<MDD_PACKAGEDEPENDENCY_CONTEXT>(packageDependencyContext));
	return S_OK;
}

HRESULT call_GetResolvedPackageFullNameForPackageDependency(
	_In_ PCWSTR packageDependencyId,
	_Outptr_result_maybenull_ PWSTR *packageFullName
	)
{
	_ASSERTE(is_initialised() == true);
	if (is_using_win11())
	{
		_ASSERTE(Win11GetResolvedPackageFullNameForPackageDependency != nullptr);
		return Win11GetResolvedPackageFullNameForPackageDependency(packageDependencyId, packageFullName);
	}
	return MddGetResolvedPackageFullNameForPackageDependency(
		packageDependencyId,
		packageFullName
	);
}

HRESULT call_GetIdForPackageDependencyContext(
	_In_ PACKAGEDEPENDENCY_CONTEXT packageDependencyContext,
	_Outptr_result_maybenull_ PWSTR *packageDependencyId
	)
{
	_ASSERTE(is_initialised() == true);
	if (is_using_win11())
	{
		_ASSERTE(Win11GetIdForPackageDependencyContext != nullptr);
		return Win11GetIdForPackageDependencyContext(packageDependencyContext, packageDependencyId);
	}
	return MddGetIdForPackageDependencyContext(
		reinterpret_cast<MDD_PACKAGEDEPENDENCY_CONTEXT>(packageDependencyContext),
		packageDependencyId
	);
}
}