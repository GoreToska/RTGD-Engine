//
// Created by ivan on 8/18/26.
//

#pragma once
#include "Platform/IDynamicLibrary.h"

#ifdef _WIN32

namespace RTGDEngine {
    class WindowsDynamicLibrary : public IDynamicLibrary {
    public:
        ~WindowsDynamicLibrary() override;

        bool Load(const std::string &path) override;

        void Unload(const std::string &path) override;

        [[nodiscard]] void *GetSymbol(const std::string &name) const override;

        [[nodiscard]] bool IsLoaded() const override;

    private:
        HMODULE m_handle;
    };
} // RTGDEditor

#endif
