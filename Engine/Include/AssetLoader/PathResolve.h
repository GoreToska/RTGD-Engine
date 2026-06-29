//
// Created by ivan on 6/29/26.
//
#pragma once
#include <filesystem>
#include <string>


namespace RTGDEngine {
    inline const std::filesystem::path &ExecutableDir() {
        static const std::filesystem::path dir = [] {
#if defined(_WIN32)
            wchar_t buf[MAX_PATH];
            GetModuleFileNameW(nullptr, buf, MAX_PATH);
            return std::filesystem::path(buf).parent_path();
#else
            return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
        }();
        return dir;
    }

    inline std::string GetAbsolutePath(const std::string &relativePath) {
        return (ExecutableDir() / relativePath).generic_string();
    }

    inline std::string GetRelativePath(const std::string &absolutePath) {
        return std::filesystem::relative(absolutePath, ExecutableDir()).generic_string();
    }
}
