//
// Created by gorev on 17.03.2026.
//

#pragma once
#include <string>
#include <vector>

namespace RTGDEngine
{
    struct TextureImportData
    {
        std::vector<uint8_t> Pixels;
        uint32_t Width = 0;
        uint32_t Height = 0;
        uint32_t Channels = 0; // almost always 4 (RGBA)
        bool Success = false;
        std::string ErrorMessage;
    };

    class TextureImporter
    {
    public:
        static TextureImportData Import(const std::string& path);

    private:
        static TextureImportData ImportSTB(const std::string& path);

        static TextureImportData ImportDDS(const std::string& path);
    };
} // RTGDEngine
