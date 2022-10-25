#pragma once

//Internal header that exposes the Dynamic Dependency API.
//It prefers the Windows 11 API if available, falling back to the 
//Windows App SDK versions if not running on Windows 11.

#ifndef _WINDOWS_
#include <Windows.h>
#endif
#ifndef _APPMODEL_H_
#include <appmodel.h>
#endif

#ifdef __cplusplus
extern"C"
{
#endif

enum class initialisation_state
{
	unknown,
	uninitialised,
	initialised_with_app_sdk,
	initialised_with_win11
};

struct internal_state
{
	uint16_t wappsdk_default_major;
	uint16_t wappsdk_default_minor;
	uint16_t wappsdk_override_major;
	uint16_t wappsdk_override_minor;
	initialisation_state dd_init_state;
	bool override_set;
};

//Initialises the library's function pointers.
bool initialise_pointers_auto();
bool initialise_pointers_win11();
bool initialise_pointers_wappsdk();
//Cleans up the function pointers.
void cleanup_pointers();
//Sets the Windows App SDK version to use.
void set_version_override(uint16_t, uint16_t);
//Removes any version override
void remove_version_override();
//Queries if the library has been initialised.
bool is_initialised();
//Queries if the library is using the Windows 11 API.
bool is_using_win11();
//Queries if the Windows 11 API set is available.
bool query_os_capability();
//Queries the presence of the Windows App SDK.
bool query_wappsdk_availability_override();
//Queries the presence of the Windows App SDK.
bool query_wappsdk_availability_default();
//Queries the presence of the Windows App SDK.
bool query_wappsdk_availability();

//Calls an available TryCreatePackageDependency
HRESULT call_TryCreatePackageDependency(
	PSID user,
	_In_ PCWSTR packageFamilyName,
	PACKAGE_VERSION minVersion,
	PackageDependencyProcessorArchitectures packageDependencyProcessorArchitectures,
	PackageDependencyLifetimeKind lifetimeKind,
	PCWSTR lifetimeArtifact,
	CreatePackageDependencyOptions options,
	_Outptr_result_maybenull_ PWSTR *packageDependencyId
	);

//Calls an available DeletePackageDependency
HRESULT call_DeletePackageDependency(
	_In_ PCWSTR packageDependencyId
	);

//Calls an available AddPackageDependency
HRESULT call_AddPackageDependency(
	_In_ PCWSTR packageDependencyId,
	INT32 rank,
	AddPackageDependencyOptions options,
	_Out_ PACKAGEDEPENDENCY_CONTEXT *packageDependencyContext,
	_Outptr_opt_result_maybenull_ PWSTR *packageFullName
	);

//Calls an available RemovePackageDependency
HRESULT call_RemovePackageDependency(
	_In_ PACKAGEDEPENDENCY_CONTEXT packageDependencyContext
	);

//Calls an available GetResolvedPackageFullNameForPackageDependency
HRESULT call_GetResolvedPackageFullNameForPackageDependency(
	_In_ PCWSTR packageDependencyId,
	_Outptr_result_maybenull_ PWSTR *packageFullName
	);

//Calls an available GetIdForPackageDependencyContext
HRESULT call_GetIdForPackageDependencyContext(
	_In_ PACKAGEDEPENDENCY_CONTEXT packageDependencyContext,
	_Outptr_result_maybenull_ PWSTR *packageDependencyId
	);

#ifdef __cplusplus
}
#endif