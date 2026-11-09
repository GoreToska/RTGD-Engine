//
// Created by ivan on 6/29/26.
//
#include "AssetLoader/PathResolve.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __linux__
#include <dlfcn.h>
#endif

namespace RTGDEngine
{
    namespace
    {
        std::filesystem::path& MutableAssetRoot()
        {
            static std::filesystem::path root = ExecutableDir();
            return root;
        }
    }

    const std::filesystem::path& ExecutableDir()
    {
        static const std::filesystem::path dir = []
        {
#if defined(_WIN32)
            HMODULE hmod = nullptr;
            GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(&ExecutableDir), &hmod);

            wchar_t buf[MAX_PATH];
            DWORD len = GetModuleFileNameW(hmod, buf, MAX_PATH);
            if (len == 0 || len == MAX_PATH)
                return std::filesystem::current_path();
            return std::filesystem::path(buf).parent_path();
#else
            Dl_info info{};
            if (dladdr(reinterpret_cast<void*>(&ExecutableDir), &info) && info.dli_fname)
                return std::filesystem::canonical(info.dli_fname).parent_path();
            return std::filesystem::current_path();
#endif
        }();
        return dir;
    }

    const std::filesystem::path& AssetRoot()
    {
        return MutableAssetRoot();
    }

    void SetAssetRoot(const std::filesystem::path& root)
    {
        MutableAssetRoot() = root;
    }

    std::string GetAbsolutePath(std::string_view relativePath)
    {
        return PathToUtf8(AssetRoot() / PathFromUtf8(relativePath));
    }

    std::string GetRelativePath(std::string_view absolutePath)
    {
        return PathToUtf8(std::filesystem::relative(PathFromUtf8(absolutePath), AssetRoot()));
    }
}
