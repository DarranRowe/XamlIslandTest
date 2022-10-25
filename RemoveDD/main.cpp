#define _WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShlObj.h>
#include <appmodel.h>
#include <dynamicdependency.h>
#include <wil/resource.h>
#include <wil/result.h>
#include <wil/com.h>
#include <memory>
#include <filesystem>
#include <string>

#if NTDDI_VERSION < NTDDI_WIN10_CO
#error This application requires the Windows 11 SDK to build.
#endif

#define DD_USE_FILEPATHS

constexpr wchar_t g_winui_package_family_name[] = L"Microsoft.UI.Xaml.2.7_8wekyb3d8bbwe";
constexpr wchar_t g_winui_value[] = L"winui";
#ifdef DD_USE_FILEPATHS
PackageDependencyLifetimeKind g_lifetime = PackageDependencyLifetimeKind::PackageDependencyLifetimeKind_FilePath;
#endif
#ifdef DD_USE_REGISTRY
PackageDependencyLifetimeKind g_lifetime = PackageDependencyLifetimeKind::PackageDependencyLifetimeKind_RegistryKey;
#endif
CreatePackageDependencyOptions g_options = CreatePackageDependencyOptions::CreatePackageDependencyOptions_None;

PACKAGEDEPENDENCY_CONTEXT g_context = nullptr;

#ifdef DD_USE_FILEPATHS
constexpr wchar_t g_artifact_key_base[] = LR"(XamlIslandTest)";
#endif
#ifdef DD_USE_REGISTRY
constexpr wchar_t g_artifact_key_base[] = LR"(Software\XamlIslandTest)";
#endif

#ifdef _M_X64
PackageDependencyProcessorArchitectures g_archs = PackageDependencyProcessorArchitectures::PackageDependencyProcessorArchitectures_X64;
#ifdef _DEBUG
#ifdef DD_USE_FILEPATHS
constexpr wchar_t g_artifact_key[] = LR"(XamlIslandTest\DebugX64)";
constexpr wchar_t g_full_artifact_key[] = LR"(XamlIslandTest\DebugX64)";
#endif
#ifdef DD_USE_REGISTRY
constexpr wchar_t g_artifact_key[] = LR"(Software\XamlIslandTest\DebugX64)";
constexpr wchar_t g_full_artifact_key[] = LR"(HKCU\Software\XamlIslandTest\DebugX64)";
#endif
#else
#ifdef DD_USE_FILEPATHS
constexpr wchar_t g_artifact_key[] = LR"(XamlIslandTest\ReleaseX64)";
constexpr wchar_t g_full_artifact_key[] = LR"(XamlIslandTest\ReleaseX64)";
#endif
#ifdef DD_USE_REGISTRY
constexpr wchar_t g_artifact_key[] = LR"(Software\XamlIslandTest\ReleaseX64)";
constexpr wchar_t g_full_artifact_key[] = LR"(HKCU\Software\XamlIslandTest\ReleaseX64)";
#endif
#endif
#endif

#ifdef _M_ARM64
PackageDependencyProcessorArchitectures g_archs = PackageDependencyProcessorArchitectures::PackageDependencyProcessorArchitectures_Arm64;
#ifdef _DEBUG
#ifdef DD_USE_FILEPATHS
constexpr wchar_t g_artifact_key[] = LR"(XamlIslandTest\DebugARM64)";
constexpr wchar_t g_full_artifact_key[] = LR"(XamlIslandTest\DebugARM64)";
#endif
#ifdef DD_USE_REGISTRY
constexpr wchar_t g_artifact_key[] = LR"(Software\XamlIslandTest\DebugARM64)";
constexpr wchar_t g_full_artifact_key[] = LR"(HKCU\Software\XamlIslandTest\DebugARM64)";
#endif
#else
#ifdef DD_USE_FILEPATHS
constexpr wchar_t g_artifact_key[] = LR"(XamlIslandTest\ReleaseARM64)";
constexpr wchar_t g_full_artifact_key[] = LR"(XamlIslandTest\ReleaseARM64)";
#endif
#ifdef DD_USE_REGISTRY
constexpr wchar_t g_artifact_key[] = LR"(Software\XamlIslandTest\ReleaseARM64)";
constexpr wchar_t g_full_artifact_key[] = LR"(HKCU\Software\XamlIslandTest\ReleaseARM64)";
#endif
#endif
#endif

#ifdef _M_IX86
PackageDependencyProcessorArchitectures g_archs = PackageDependencyProcessorArchitectures::PackageDependencyProcessorArchitectures_X86;
#ifdef _DEBUG
#ifdef DD_USE_FILEPATHS
constexpr wchar_t g_artifact_key[] = LR"(XamlIslandTest\DebugX86)";
constexpr wchar_t g_full_artifact_key[] = LR"(XamlIslandTest\DebugX86)";
#endif
#ifdef DD_USE_REGISTRY
constexpr wchar_t g_artifact_key[] = LR"(Software\XamlIslandTest\DebugX86)";
constexpr wchar_t g_full_artifact_key[] = LR"(HKCU\Software\XamlIslandTest\DebugX86)";
#endif
#else
#ifdef DD_USE_FILEPATHS
constexpr wchar_t g_artifact_key[] = LR"(XamlIslandTest\ReleaseX86)";
constexpr wchar_t g_full_artifact_key[] = LR"(XamlIslandTest\ReleaseX86)";
#endif
#ifdef DD_USE_REGISTRY
constexpr wchar_t g_artifact_key[] = LR"(Software\XamlIslandTest\ReleaseX86)";
constexpr wchar_t g_full_artifact_key[] = LR"(HKCU\Software\XamlIslandTest\ReleaseX86)";
#endif
#endif
#endif

void check_and_remove_registry()
{
	wil::unique_hkey artifact;
	auto result = RegOpenKeyExW(HKEY_CURRENT_USER, g_artifact_key, 0, KEY_ALL_ACCESS, artifact.addressof());

	if (result == ERROR_FILE_NOT_FOUND)
	{
		return;
	}

	THROW_IF_WIN32_ERROR(result);

	DWORD data_size = 0;
	DWORD flags = RRF_RT_REG_SZ;
	result = RegGetValueW(artifact.get(), nullptr, g_winui_value, flags, nullptr, nullptr, &data_size);
	std::unique_ptr<wchar_t[]> value_buffer = std::make_unique<wchar_t[]>(data_size / sizeof(wchar_t));
	result = RegGetValueW(artifact.get(), nullptr, g_winui_value, flags, nullptr, value_buffer.get(), &data_size);

	THROW_IF_FAILED(DDDeletePackageDependency(value_buffer.get()));

	artifact.reset();

	result = RegDeleteTreeW(HKEY_CURRENT_USER, g_artifact_key_base);
}
void check_and_remove_file()
{
	wil::unique_cotaskmem_string laf;
	THROW_IF_FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, laf.addressof()));
	std::wstring local_app_data_string = laf.get();

	std::filesystem::path appdata_root = local_app_data_string;
	std::filesystem::path artifact_dir = appdata_root / g_artifact_key;
	std::filesystem::path artifact = appdata_root / g_artifact_key / g_winui_value;

	if (!std::filesystem::exists(artifact_dir))
	{
		return;
	}

	if (!std::filesystem::exists(artifact))
	{
		return;
	}

	wil::unique_file artifact_file;
	auto open_result = _wfopen_s(artifact_file.addressof(), artifact.c_str(), L"r+");
	_ASSERTE(open_result == 0);

	auto file_size = std::filesystem::file_size(artifact);

	std::unique_ptr<wchar_t[]> value_buffer = std::make_unique<wchar_t[]>(file_size + 1);
	fwscanf_s(artifact_file.get(), L"%s", value_buffer.get(), static_cast<unsigned int>(file_size + 1));

	THROW_IF_FAILED(DDDeletePackageDependency(value_buffer.get()));

	artifact_file.reset();

	std::filesystem::remove(artifact);
}

int wmain()
{
	DDInitialiseWin11();

#ifdef DD_USE_FILEPATHS
	check_and_remove_file();
#endif
#ifdef DD_USE_REGISTRY
	check_and_remove_registry();
#endif

	DDCleanup();
	return 0;
}