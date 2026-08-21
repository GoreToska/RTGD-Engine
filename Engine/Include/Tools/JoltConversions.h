//
// Created by ivan on 8/21/26.
//

#pragma once

#include "Alias.h"
#include <Jolt/Jolt.h>

namespace RTGDEngine {
    inline JPH::Vec3 ToVec3(const Float3 &vec) {
        return {vec.x, vec.y, vec.z};
    }

    inline JPH::RVec3 ToRVec3(const Float3 &vec) {
        return {vec.x, vec.y, vec.z};
    }

    inline JPH::Quat ToQuat(const Quaternion &quat) {
        return {quat.q.x, quat.q.y, quat.q.z, quat.q.w};
    }

    inline Float3 ToFloat3(const JPH::Vec3 &vec) {
        return {vec.GetX(), vec.GetY(), vec.GetZ()};
    }

    inline Quaternion ToQuaternion(const JPH::Quat &quat) {
        return {quat.GetX(), quat.GetY(), quat.GetZ(), quat.GetW()};
    }
}
