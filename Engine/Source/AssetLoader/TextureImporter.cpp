//
// Created by gorev on 17.03.2026.
//

#include "AssetLoader/TextureImporter.h"

#include <filesystem>

#include "Tools/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include <fstream>
#include <stb_image.h>

namespace RTGDEngine
{
    TextureImportData TextureImporter::Import(const std::string& path)
    {
        std::string ext = std::filesystem::path(path).extension().string();
        std::ranges::transform(ext, ext.begin(), ::tolower);

        if (ext == ".dds")
            return ImportDDS(path);

        return ImportSTB(path);
    }

    TextureImportData TextureImporter::ImportSTB(const std::string& path)
    {
        TextureImportData result;

        // always load as RGBA
        int width, height, channels;
        stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);

        if (!pixels)
        {
            result.ErrorMessage = stbi_failure_reason();
            LogError("TextureImporter: failed to load '{}': {}",
                     path, result.ErrorMessage);
            return result;
        }

        result.Width = static_cast<uint32_t>(width);
        result.Height = static_cast<uint32_t>(height);
        result.Channels = 4;
        result.Pixels.assign(pixels, pixels + width * height * 4);
        result.Success = true;

        stbi_image_free(pixels);

        LogInfo("TextureImporter: '{}' — {}x{} RGBA", path, width, height);
        return result;
    }

    TextureImportData TextureImporter::ImportDDS(const std::string& path)
    {
        TextureImportData result;

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            result.ErrorMessage = "Cannot open file";
            LogError("TextureImporter: cannot open DDS '{}'", path);
            return result;
        }

        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        result.Pixels.resize(size);
        file.read(reinterpret_cast<char*>(result.Pixels.data()), size);

        // will be filled with GPU upload
        result.Width = 0;
        result.Height = 0;
        result.Success = true;

        LogInfo("TextureImporter: DDS '{}' — {} bytes", path, size);
        return result;
    }
} // RTGDEngine
