//
// Created by ivan on 6/29/26.
//
#pragma once
#include <filesystem>
#include <string>
#include <string_view>

#include "Engine/EngineExport.h"

namespace RTGDEngine
{
    inline std::filesystem::path PathFromUtf8(std::string_view utf8)
    {
        return {std::u8string_view(reinterpret_cast<const char8_t*>(utf8.data()), utf8.size())};
    }

    inline std::string PathToUtf8(const std::filesystem::path& path)
    {
        const std::u8string u8 = path.generic_u8string();
        return {reinterpret_cast<const char*>(u8.data()), u8.size()};
    }

    ENGINE_API const std::filesystem::path& ExecutableDir();

    ENGINE_API const std::filesystem::path& AssetRoot();

    ENGINE_API void SetAssetRoot(const std::filesystem::path& root);

    ENGINE_API std::string GetAbsolutePath(std::string_view relativePath);

    ENGINE_API std::string GetRelativePath(std::string_view absolutePath);
}
