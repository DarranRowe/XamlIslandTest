#include "pch.h"
#include "dynamicdependency.h"
#include <wil/resource.h>

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

#ifdef DD_USE_REGISTRY
static bool check_winui_dd_registry()
{
	wil::unique_hkey artifact;
	auto result = RegOpenKeyExW(HKEY_CURRENT_USER, g_artifact_key, 0, KEY_READ, artifact.addressof());

	if (result == ERROR_FILE_NOT_FOUND)
	{
		return false;
	}

	THROW_IF_WIN32_ERROR(result);

	return true;
}
#endif

#ifdef DD_USE_FILEPATHS
static bool check_winui_dd_file()
{
	wil::unique_cotaskmem_string laf;
	THROW_IF_FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, laf.addressof()));
	std::wstring local_app_data_string = laf.get();

	std::filesystem::path appdata_root = local_app_data_string;
	std::filesystem::path artifact = appdata_root / g_artifact_key / g_winui_value;

	if (std::filesystem::exists(artifact))
	{
		return true;
	}

	return false;
}
#endif

bool check_winui_dd()
{
#ifdef DD_USE_FILEPATHS
	return check_winui_dd_file();
#endif
#ifdef DD_USE_REGISTRY
	return check_winui_dd_registry();
#endif
}

#ifdef DD_USE_REGISTRY
static bool create_winui_dd_registry()
{
	wil::unique_hkey artifact;
	wil::unique_process_heap_string ptr;

	DWORD disposition = 0;

	auto result = RegCreateKeyExW(HKEY_CURRENT_USER, g_artifact_key, 0, 0, 0, KEY_ALL_ACCESS, nullptr, artifact.addressof(), &disposition);
	THROW_IF_WIN32_ERROR(result);

	auto package_result = DDTryCreatePackageDependency(nullptr, g_winui_package_family_name, PACKAGE_VERSION{}, g_archs, g_lifetime, g_full_artifact_key, g_options, ptr.addressof());
	if (FAILED(package_result))
	{
		artifact.reset();
		RegDeleteKeyExW(HKEY_CURRENT_USER, g_artifact_key, KEY_ALL_ACCESS, 0);
		return false;
	}

	auto idcount = (wcslen(ptr.get()) + 1) * sizeof(wchar_t);
	result = RegSetValueExW(artifact.get(), g_winui_value, 0, REG_SZ, reinterpret_cast<BYTE *>(ptr.get()), static_cast<DWORD>(idcount));
	THROW_IF_WIN32_ERROR(result);

	return true;
}
#endif

#ifdef DD_USE_FILEPATHS
static bool create_winui_dd_file()
{
	wil::unique_cotaskmem_string laf;
	THROW_IF_FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, laf.addressof()));
	std::wstring local_app_data_string = laf.get();

	std::filesystem::path appdata_root = local_app_data_string;
	std::filesystem::path artifact_path = appdata_root / g_artifact_key;
	std::filesystem::path artifact = appdata_root / g_artifact_key / g_winui_value;
	wil::unique_file artifact_file;
	wil::unique_process_heap_string ptr;
	if (!std::filesystem::exists(artifact_path))
	{
		std::filesystem::create_directories(artifact_path);
	}

	auto open_result = _wfopen_s(artifact_file.addressof(), artifact.c_str(), L"w");
	_ASSERTE(open_result == 0);
	artifact_file.reset();

	auto package_result = DDTryCreatePackageDependency(nullptr, g_winui_package_family_name, PACKAGE_VERSION{}, g_archs, g_lifetime, artifact.c_str(), g_options, ptr.addressof());
	if (FAILED(package_result))
	{
		std::filesystem::remove(artifact);
		return false;
	}

	open_result = _wfopen_s(artifact_file.addressof(), artifact.c_str(), L"w");
	_ASSERTE(open_result == 0);
	fwprintf(artifact_file.get(), L"%s", ptr.get());
	return true;
}
#endif

bool create_winui_dd()
{
#ifdef DD_USE_FILEPATHS
	return create_winui_dd_file();
#endif
#ifdef DD_USE_REGISTRY
	return create_winui_dd_registry();
#endif
}

#ifdef DD_USE_REGISTRY
static bool add_winui_dd_registry()
{
	wil::unique_hkey artifact;
	auto result = RegOpenKeyExW(HKEY_CURRENT_USER, g_artifact_key, 0, KEY_READ, artifact.addressof());

	if (result == ERROR_FILE_NOT_FOUND)
	{
		return false;
	}

	THROW_IF_WIN32_ERROR(result);

	DWORD data_size = 0;
	DWORD flags = RRF_RT_REG_SZ;
	result = RegGetValueW(artifact.get(), nullptr, g_winui_value, flags, nullptr, nullptr, &data_size);
	std::unique_ptr<wchar_t[]> value_buffer = std::make_unique<wchar_t[]>(data_size / sizeof(wchar_t));
	result = RegGetValueW(artifact.get(), nullptr, g_winui_value, flags, nullptr, value_buffer.get(), &data_size);
	wil::unique_process_heap_string full_name;

	auto package_result = DDAddPackageDependency(value_buffer.get(), 0, AddPackageDependencyOptions::AddPackageDependencyOptions_None, &g_context, full_name.addressof());
	if (FAILED(package_result))
	{
		return false;
	}
	return true;
}
#endif

#ifdef DD_USE_FILEPATHS
static bool add_winui_dd_file()
{
	wil::unique_cotaskmem_string laf;
	THROW_IF_FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, laf.addressof()));
	std::wstring local_app_data_string = laf.get();

	std::filesystem::path appdata_root = local_app_data_string;
	std::filesystem::path artifact = appdata_root / g_artifact_key / g_winui_value;
	wil::unique_file artifact_file;
	auto open_result = _wfopen_s(artifact_file.addressof(), artifact.c_str(), L"r+");
	_ASSERTE(open_result == 0);

	auto file_size = std::filesystem::file_size(artifact);

	std::unique_ptr<wchar_t[]> value_buffer = std::make_unique<wchar_t[]>(file_size + 1);
	fwscanf_s(artifact_file.get(), L"%s", value_buffer.get(), static_cast<unsigned int>(file_size + 1));

	wil::unique_process_heap_string full_name;
	auto package_result = DDAddPackageDependency(value_buffer.get(), 0, AddPackageDependencyOptions_None, &g_context, full_name.addressof());
	if (FAILED(package_result))
	{
		return false;
	}

	return true;
}
#endif

bool add_winui_dd()
{
	if (!check_winui_dd())
	{
		if (!create_winui_dd())
		{
			return false;
		}
	}

#ifdef DD_USE_FILEPATHS
	return add_winui_dd_file();
#endif
#ifdef DD_USE_REGISTRY
	return add_winui_dd_registry();
#endif
}

void remove_winui_dd()
{
	DDRemovePackageDependency(g_context);
	g_context = nullptr;
}

void delete_winui_dd()
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

	DDDeletePackageDependency(value_buffer.get());

	artifact.reset();

	result = RegDeleteTreeW(HKEY_CURRENT_USER, g_artifact_key_base);
}