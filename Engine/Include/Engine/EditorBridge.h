//
// Created by ivan on 8/12/26.
//

#pragma once
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

#include "Tools/RTGDMacros.h"

#ifdef RTGD_EDITOR
namespace RTGDEngine {
    class EditorBridge {
        DECLARE_SINGLETON(EditorBridge);

    public:
        void Initialize();

        void PublishSnapshot(); // can be called only from render thread
        void MarkHierarchyDirty();

        void SetSelected(uint64_t id);

        uint32_t SelectedVersion() const;

        uint32_t HierarchyVersion() const;

        int CopySelectedJson(char *buf, int cap) const;

        int CopyHierarchyJson(char *buf, int cap) const;

    private:
        mutable std::mutex m_mutex;
        std::string m_selectedJson;
        std::string m_hierarchyJson;
        std::atomic<uint64_t> m_selectedID = 0;
        std::atomic<uint32_t> m_selectedVersion = 0;
        std::atomic<uint32_t> m_hierarchyVersion = 0;
        std::atomic<bool> m_hierarchyDirty = true;
    };

    DECLARE_GLOBAL_SINGLETON(EditorBridge, GEditorBridge)
} // RTGDEngine
#endif
