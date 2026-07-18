//
// Created by ivan on 6/29/26.
//
#pragma once
#include <filesystem>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace RTGDEngine
{
    inline const std::filesystem::path& ExecutableDir()
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
#else.
            Dl_info info{};
            if (dladdr(reinterpret_cast<void *>(&ExecutableDir), &info) && info.dli_fname)
                return std::filesystem::canonical(info.dli_fname).parent_path();
            return std::filesystem::current_path();
#endif
        }();
        return dir;
    }

    inline std::string GetAbsolutePath(const std::string& relativePath)
    {
        return (ExecutableDir() / relativePath).generic_string();
    }

    inline std::string GetRelativePath(const std::string& absolutePath)
    {
        return std::filesystem::relative(absolutePath, ExecutableDir()).generic_string();
    }
}
