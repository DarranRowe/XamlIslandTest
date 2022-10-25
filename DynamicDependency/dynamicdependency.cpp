#include <cinttypes>
#include "ddinternal.h"

#ifdef __cplusplus
extern"C"
{
#endif

__declspec(dllexport)
void DDOverrideAppSDK(uint16_t ver_major, uint16_t ver_minor)
{
	set_version_override(ver_major, ver_minor);
}
__declspec(dllexport)
void DDRemoveAppSDKOverride()
{
	remove_version_override();
}

__declspec(dllexport)
bool DDInitialise()
{
	return initialise_pointers_auto();
}
__declspec(dllexport)
bool DDInitialiseWin11()
{
	return initialise_pointers_win11();
}
__declspec(dllexport)
bool DDInitialiseAppSDK()
{
	return initialise_pointers_wappsdk();
}
__declspec(dllexport)
void DDCleanup()
{
	cleanup_pointers();
}
__declspec(dllexport)
bool DDIsInitialised()
{
	return is_initialised();
}
__declspec(dllexport)
bool DDIsUsingWin11()
{
	return is_using_win11();
}

__declspec(dllexport)
bool DDQueryOsCapability()
{
	return query_os_capability();
}
__declspec(dllexport)
bool DDQueryAppSDKPresenceDefault()
{
	return query_wappsdk_availability_default();
}
__declspec(dllexport)
bool DDQueryAppSDKPresenceOverride()
{
	return query_wappsdk_availability_override();
}
__declspec(dllexport)
bool DDQueryAppSDKPresence()
{
	return query_wappsdk_availability();
}

__declspec(dllexport)
HRESULT DDTryCreatePackageDependency(
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
	return call_TryCreatePackageDependency(user, packageFamilyName, minVersion, packageDependencyProcessorArchitectures, lifetimeKind, lifetimeArtifact, options, packageDependencyId);
}

__declspec(dllexport)
HRESULT DDDeletePackageDependency(
	_In_ PCWSTR packageDependencyId
)
{
	return call_DeletePackageDependency(packageDependencyId);
}

__declspec(dllexport)
HRESULT DDAddPackageDependency(
	_In_ PCWSTR packageDependencyId,
	INT32 rank,
	AddPackageDependencyOptions options,
	_Out_ PACKAGEDEPENDENCY_CONTEXT *packageDependencyContext,
	_Outptr_opt_result_maybenull_ PWSTR *packageFullName
)
{
	return call_AddPackageDependency(packageDependencyId, rank, options, packageDependencyContext, packageFullName);
}

__declspec(dllexport)
HRESULT DDRemovePackageDependency(
	_In_ PACKAGEDEPENDENCY_CONTEXT packageDependencyContext
)
{
	return call_RemovePackageDependency(packageDependencyContext);
}

__declspec(dllexport)
HRESULT DDGetResolvedPackageFullNameForPackageDependency(
	_In_ PCWSTR packageDependencyId,
	_Outptr_result_maybenull_ PWSTR *packageFullName
)
{
	return call_GetResolvedPackageFullNameForPackageDependency(packageDependencyId, packageFullName);
}

__declspec(dllexport)
HRESULT DDGetIdForPackageDependencyContext(
	_In_ PACKAGEDEPENDENCY_CONTEXT packageDependencyContext,
	_Outptr_result_maybenull_ PWSTR *packageDependencyId
)
{
	return call_GetIdForPackageDependencyContext(packageDependencyContext, packageDependencyId);
}

#ifdef __cplusplus
}
#endif