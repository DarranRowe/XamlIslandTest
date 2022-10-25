#pragma once

#ifndef _WINDOWS_
#define _WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#ifndef _APPMODEL_H_
#include <appmodel.h>
#endif

#include <cinttypes>

#ifdef __cplusplus
extern"C"
{
#endif

//Sets the version override for the Windows App SDK version.
//This defaults to the version of the SDK this library was built against.
void DDOverrideAppSDK(uint16_t, uint16_t);
//Removes the version override.
void DDRemoveAppSDKOverride();
//Initialises the library using the default method.
bool DDInitialise();
//Tries to initialise using the Windows 11 API only.
bool DDInitialiseWin11();
//Tries to initialise using the Windows App SDK only.
bool DDInitialiseAppSDK();
//Cleans up the library.
void DDCleanup();
//Queries if the library has been initialised.
bool DDIsInitialised();
//Queries if the library is using the Windows 11 API.
bool DDIsUsingWin11();
//Queries if Windows has the Dynamic Dependency API.
bool DDQueryOsCapability();
//Queries if the Windows App SDK that the library was built
//against is installed.
bool DDQueryAppSDKPresenceDefault();
//Queries if the Windows App SDK version that we overrided
//is installed.
bool DDQueryAppSDKPresenceOverride();
//Queries if one of the expected versions is available.
bool DDQueryAppSDKPresence();

HRESULT DDTryCreatePackageDependency(
	PSID user,
	_In_ PCWSTR packageFamilyName,
	PACKAGE_VERSION minVersion,
	PackageDependencyProcessorArchitectures packageDependencyProcessorArchitectures,
	PackageDependencyLifetimeKind lifetimeKind,
	PCWSTR lifetimeArtifact,
	CreatePackageDependencyOptions options,
	_Outptr_result_maybenull_ PWSTR *packageDependencyId
	);

HRESULT DDDeletePackageDependency(
	_In_ PCWSTR packageDependencyId
	);

HRESULT DDAddPackageDependency(
	_In_ PCWSTR packageDependencyId,
	INT32 rank,
	AddPackageDependencyOptions options,
	_Out_ PACKAGEDEPENDENCY_CONTEXT *packageDependencyContext,
	_Outptr_opt_result_maybenull_ PWSTR *packageFullName
	);

HRESULT DDRemovePackageDependency(
	_In_ PACKAGEDEPENDENCY_CONTEXT packageDependencyContext
	);

HRESULT DDGetResolvedPackageFullNameForPackageDependency(
	_In_ PCWSTR packageDependencyId,
	_Outptr_result_maybenull_ PWSTR *packageFullName
	);

HRESULT DDGetIdForPackageDependencyContext(
	_In_ PACKAGEDEPENDENCY_CONTEXT packageDependencyContext,
	_Outptr_result_maybenull_ PWSTR *packageDependencyId
	);

#ifdef __cplusplus
}
#endif