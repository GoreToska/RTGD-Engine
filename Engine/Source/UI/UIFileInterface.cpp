//
// Created by ivan on 11/6/26.
//

#include "UI/UIFileInterface.h"

#include <filesystem>

#include "AssetLoader/PathResolve.h"
#include "RmlUi/Core/Types.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    Rml::FileHandle UIFileInterface::Open(const Rml::String& path)
    {
        const std::string resolved = GetAbsolutePath(path);

        auto file = std::fopen(resolved.c_str(), "rb");
        if (!file)
        {
            LogError("Failed to open file: {}", resolved);
            return Rml::FileHandle();
        }

        return reinterpret_cast<Rml::FileHandle>(file);
    }

    void UIFileInterface::Close(Rml::FileHandle file)
    {
        fclose(reinterpret_cast<FILE*>(file));
    }

    size_t UIFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file)
    {
        return fread(buffer, 1, size, reinterpret_cast<FILE*>(file));
    }

    bool UIFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
    {
        return fseek(reinterpret_cast<FILE*>(file), offset, origin) == 0;
    }

    size_t UIFileInterface::Tell(Rml::FileHandle file)
    {
        return ftell(reinterpret_cast<FILE*>(file));
    }
} // RTGDEngine
