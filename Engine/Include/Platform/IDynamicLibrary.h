//
// Created by ivan on 8/18/26.
//

#pragma once
#include <string>

namespace RTGDEngine {
    class IDynamicLibrary {
    public:
        virtual ~IDynamicLibrary() = default;

        virtual bool Load(const std::string &path) = 0;

        virtual void Unload() = 0;

        [[nodiscard]] virtual void *GetSymbol(const std::string &name) const = 0;

        [[nodiscard]] virtual bool IsLoaded() const = 0;

        std::string &GetPath() { return m_path; };

    protected:
        std::string m_path;
    };
}
