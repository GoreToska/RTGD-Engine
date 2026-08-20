//
// Created by ivan on 8/18/26.
//

#pragma once

#ifdef __linux__
#include "Platform/IDynamicLibrary.h"

namespace RTGDEngine {
    class LinuxDynamicLibrary : public IDynamicLibrary {
    public:
        ~LinuxDynamicLibrary() override;

        bool Load(const std::string &path) override;

        void Unload() override;

        [[nodiscard]] void *GetSymbol(const std::string &name) const override;

        [[nodiscard]] bool IsLoaded() const override;

    private:
        void *m_handle = nullptr;
    };
} // RTGDEngine

#endif
