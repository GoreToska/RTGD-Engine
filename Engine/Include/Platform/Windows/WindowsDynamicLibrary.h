//
// Created by ivan on 8/18/26.
//

#pragma once
#ifdef _WIN32

#include "Platform/IDynamicLibrary.h"

#include <Windows.h>

namespace RTGDEngine
{
    class WindowsDynamicLibrary : public IDynamicLibrary
    {
    public:
        ~WindowsDynamicLibrary() override;

        bool Load(const std::string& fullPath) override;

        void Unload() override;

        [[nodiscard]] void* GetSymbol(const std::string& name) const override;

        [[nodiscard]] bool IsLoaded() const override;

    private:
        HMODULE m_handle = {};
    };
} // RTGDEditor

#endif
