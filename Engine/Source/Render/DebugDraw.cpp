//
// Created by ivan on 8/27/26.
//

#include "Render/DebugDraw.h"

namespace RTGDEngine
{
    void DebugDraw::DrawLine(const Float3& a, const Float3& b, const Float4& color, float duration)
    {
        using namespace std::chrono;

        if (duration == 0.0f)
        {
            m_lines.push_back({a, color});
            m_lines.push_back({b, color});
            return;
        }

        auto expire = duration < 0.0f
                          ? steady_clock::time_point::max()
                          : steady_clock::now() + std::chrono::duration_cast<steady_clock::duration>(
                                std::chrono::duration<float>(duration));

        m_timedLines.push_back({{a, color}, {b, color}, expire});
    }

    void DebugDraw::DrawArc(const Float3& center, const Float3& right, const Float3& up, float radius,
                            float sweepRadians, int segments, const Float4& color, float duration)
    {
        Float3 prev = center + right * radius;
        for (int i = 1; i <= segments; ++i)
        {
            float t = sweepRadians * i / segments;
            Float3 p = center + right * (cosf(t) * radius) + up * (sinf(t) * radius);
            DrawLine(prev, p, color, duration);
            prev = p;
        }
    }

    void DebugDraw::DrawBox(const Float3& center, const Float3& halfExtents, const Quaternion& rotation,
                            const Float4& color, float duration)
    {
        Float3 right, up, fwd;
        GetBasis(rotation, right, up, fwd);
        Float3 c[8];
        int i = 0;
        for (int bx = -1; bx <= 1; bx += 2)
        {
            for (int by = -1; by <= 1; by += 2)
            {
                for (int bz = -1; bz <= 1; bz += 2)
                {
                    c[i++] = center + right * (bx * halfExtents.x) + up * (by * halfExtents.y) + fwd * (
                                 bz * halfExtents.z);
                }
            }
        }

        static constexpr int edges[12][2] = {
            {0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3}, {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };

        for (auto& edge: edges)
            DrawLine(c[edge[0]], c[edge[1]], color, duration);
    }

    void DebugDraw::DrawSphere(const Float3& center, float radius, const Float4& color, float duration)
    {
        DrawArc(center, {1, 0, 0}, {0, 1, 0}, radius, 2 * Diligent::PI, 24, color, duration);
        DrawArc(center, {1, 0, 0}, {0, 0, 1}, radius, 2 * Diligent::PI, 24, color, duration);
        DrawArc(center, {0, 1, 0}, {0, 0, 1}, radius, 2 * Diligent::PI, 24, color, duration);
    }

    void DebugDraw::DrawCapsule(const Float3& center, float radius, float halfHeight, const Quaternion& rotation,
                                const Float4& color, float duration)
    {
        Float3 right, up, fwd;
        GetBasis(rotation, right, up, fwd);
        Float3 top = center + up * halfHeight;
        Float3 bottom = center - up * halfHeight;

        DrawArc(top, right, fwd, radius, 2 * Diligent::PI, 12, color, duration);
        DrawArc(bottom, right, fwd, radius, 2 * Diligent::PI, 12, color, duration);
        DrawArc(top, right, up, radius, Diligent::PI, 12, color, duration);
        DrawArc(top, fwd, up, radius, Diligent::PI, 12, color, duration);
        DrawArc(bottom, right, -up, radius, Diligent::PI, 12, color, duration);
        DrawArc(bottom, fwd, -up, radius, Diligent::PI, 12, color, duration);

        DrawLine(top + right * radius, bottom + right * radius, color, duration);
        DrawLine(top - right * radius, bottom - right * radius, color, duration);
        DrawLine(top + fwd * radius, bottom + fwd * radius, color, duration);
        DrawLine(top - fwd * radius, bottom - fwd * radius, color, duration);
    }

    std::vector<LineVertex> DebugDraw::TakeLines()
    {
        using namespace std::chrono;

        std::vector<LineVertex> result;
        std::swap(result, m_lines);

        auto now = steady_clock::now();
        std::erase_if(m_timedLines, [now](const TimedLine& line){return line.Expire <= now;});

        result.reserve(result.size() + m_timedLines.size() * 2);

        for (auto& line: m_timedLines)
        {
            result.push_back(line.A);
            result.push_back(line.B);
        }

        return result;
    }

    void DebugDraw::ClearPersistent()
    {
        m_timedLines.clear();
    }
} // RTGDEngine
