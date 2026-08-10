//
// Created by ivan on 8/6/26.
//

#pragma once
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <bit>
#include <cmath>

#include "CameraFrustum.h"

namespace RTGDEngine {
    struct BoundsView {
        const float *CenterX, *CenterY, *CenterZ;
        const float *ExtentX, *ExtentY, *ExtentZ;
        uint32_t Count;
    };

    struct FrustumSIMD {
        float NX[6], NY[6], NZ[6], AX[6], AY[6], AZ[6], D[6];

        static FrustumSIMD From(const CameraFrustum &other) {
            FrustumSIMD frustum{};
            auto &planes = other.GetPlanes();

            for (uint32_t i = 0; i < 6; ++i) {
                frustum.NX[i] = planes[i].x;
                frustum.NY[i] = planes[i].y;
                frustum.NZ[i] = planes[i].z;
                frustum.D[i] = planes[i].w;
                frustum.AX[i] = std::abs(planes[i].x);
                frustum.AY[i] = std::abs(planes[i].y);
                frustum.AZ[i] = std::abs(planes[i].z);
            }

            return frustum;
        }
    };

    inline void CullFrustum(const BoundsView &bounds, const FrustumSIMD &frustum, uint32_t begin, uint32_t end,
                            uint64_t *words) {
        uint32_t i = begin;
        while (i < end) {
            const uint32_t w = i >> 6;
            const uint32_t blockEnd = std::min(end, (w + 1) << 6);

            uint64_t word = 0;
            for (; i < blockEnd; ++i) {
                const float cx = bounds.CenterX[i];
                const float cy = bounds.CenterY[i];
                const float cz = bounds.CenterZ[i];
                const float ex = bounds.ExtentX[i];
                const float ey = bounds.ExtentY[i];
                const float ez = bounds.ExtentZ[i];

                uint32_t visible = 1;
                for (uint32_t p = 0; p < CameraFrustum::PLANE_COUNT; ++p) {
                    const float s = frustum.NX[p] * cx + frustum.NY[p] * cy + frustum.NZ[p] * cz + frustum.D[p];
                    const float r = frustum.AX[p] * ex + frustum.AY[p] * ey + frustum.AZ[p] * ez;
                    visible &= static_cast<uint32_t>(s + r >= 0.0f);
                }

                word |= static_cast<uint64_t>(visible) << (i & 63);
            }

            words[w] |= word;
        }
    }

    class VisibilityMask {
        std::vector<uint64_t> m_words = {};

    public:
        void Resize(uint32_t count) {
            m_words.assign((count + 63) / 64, 0);
        }

        void Set(uint32_t i) {
            m_words[i >> 6] |= 1ull << (i & 63);
        }

        void SetAll(uint32_t count) {
            m_words.assign((count + 63) / 64, ~0ull);
            if (const uint32_t tail = count & 63; tail != 0 && !m_words.empty()) {
                m_words.back() = (1ull << tail) - 1;
            }
        }

        [[nodiscard]] bool Test(uint32_t i) const {
            return (m_words[i >> 6] >> (i & 63)) & 1ull;
        }

        void OrWith(const VisibilityMask &other) {
            assert(m_words.size() == other.m_words.size());
            for (uint32_t i = 0; i < m_words.size(); ++i) {
                m_words[i] |= other.m_words[i];
            }
        }

        void AndWith(const VisibilityMask &other) {
            assert(m_words.size() == other.m_words.size());
            for (uint32_t i = 0; i < m_words.size(); ++i) {
                m_words[i] &= other.m_words[i];
            }
        }

        [[nodiscard]] uint64_t *Words() { return m_words.data(); }

        [[nodiscard]] const uint64_t *Words() const { return m_words.data(); }

        template<typename Fn>
        void ForEach(Fn &&fn) const {
            for (uint32_t w = 0; w < m_words.size(); ++w) {
                uint64_t bits = m_words[w];
                while (bits) {
                    const uint32_t b = std::countr_zero(bits);
                    bits &= bits - 1;
                    fn(w * 64 + b);
                }
            }
        }
    };
}
