//
// Created by gorev on 11.03.2026.
//

#pragma once
#include <cassert>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <string>
#include <vector>


namespace RTGDEngine
{
    class ShaderLoader
    {
    public:
        static std::vector<uint32_t> LoadShader(const std::string& absolutePath)
        {
            std::ifstream file(absolutePath, std::ios::binary | std::ios::ate);
            assert(file.good());

            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<uint32_t> buffer((size + 3) / 4);
            file.read(reinterpret_cast<char*>(buffer.data()), size);
            return buffer;
        }
    };
} // RTGDEngine
