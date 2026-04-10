//
// Created by gorev on 08.04.2026.
//

#include "Engine/InspectorExports.h"

#include <flecs.h>
#include <vector>
#include <string>
#include <mutex>
#include <flecs/addons/meta.h>

#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Tools/Logger.h"

static std::vector<ComponentInfo> s_compCache;
static std::mutex s_cacheMutex;

static std::string ReadFieldValue(flecs::world_t* world, const void* compData, const ecs_member_t& member)
{
    const void* ptr = static_cast<const uint8_t*>(compData) + member.offset;

    const EcsPrimitive* prim = static_cast<const EcsPrimitive*>(
        ecs_get(world, member.type, EcsPrimitive)
    );

    if (prim)
    {
        switch (prim->kind)
        {
            case EcsF32:
                return std::to_string(*static_cast<const float*>(ptr));
            case EcsF64:
                return std::to_string(*static_cast<const double*>(ptr));
            case EcsI8:
                return std::to_string(*static_cast<const int8_t*>(ptr));
            case EcsI16:
                return std::to_string(*static_cast<const int16_t*>(ptr));
            case EcsI32:
                return std::to_string(*static_cast<const int32_t*>(ptr));
            case EcsI64:
                return std::to_string(*static_cast<const int64_t*>(ptr));
            case EcsU8:
                return std::to_string(*static_cast<const uint8_t*>(ptr));
            case EcsU16:
                return std::to_string(*static_cast<const uint16_t*>(ptr));
            case EcsU32:
                return std::to_string(*static_cast<const uint32_t*>(ptr));
            case EcsU64:
                return std::to_string(*static_cast<const uint64_t*>(ptr));
            case EcsBool:
                return *static_cast<const bool*>(ptr) ? "true" : "false";
            case EcsString:
            {
                const char* s = *static_cast<const char* const*>(ptr);
                return s ? s : "(null)";
            }
            default:
                break;
        }
    }

    const EcsStruct* nested = static_cast<const EcsStruct*>(
        ecs_get(world, member.type, EcsStruct)
    );

    if (nested)
        return "{...}"; // recursive

    return "?";
}

static std::string GetTypeName(flecs::world_t* world, ecs_entity_t type)
{
    const EcsPrimitive* prim = static_cast<const EcsPrimitive*>(
        ecs_get(world, type, EcsPrimitive)
    );

    if (prim)
    {
        switch (prim->kind)
        {
            case EcsF32:
                return "float";
            case EcsF64:
                return "double";
            case EcsI8:
                return "int8";
            case EcsI16:
                return "int16";
            case EcsI32:
                return "int32";
            case EcsI64:
                return "int64";
            case EcsU8:
                return "uint8";
            case EcsU16:
                return "uint16";
            case EcsU32:
                return "uint32";
            case EcsU64:
                return "uint64";
            case EcsBool:
                return "bool";
            case EcsString:
                return "string";
            default:
                break;
        }
    }

    const char* name = ecs_get_name(world, type);
    return name ? name : "unknown";
}

extern "C"
{
int Inspector_GetComponentCount(uint64_t entityId)
{
    std::lock_guard lock(s_cacheMutex);
    s_compCache.clear();

    auto scene = RTGDEngine::SceneManager::Instance().GetActiveScene();
    if (!scene)
        return 0;

    auto& world = scene->GetWorld();
    flecs::entity e(world, entityId);
    if (!e.is_valid())
        return 0;

    e.each([&](flecs::id id)
    {
        if (id.is_pair() || !id.is_entity())
            return;

        const ecs_type_info_t* type_info = ecs_get_type_info(world, id);
        if (!type_info)
            return;

        ComponentInfo comp;
        comp.name = type_info->name ? type_info->name : "Unknown";

        const EcsStruct* s = static_cast<const EcsStruct*>(
            ecs_get_id(world, id.raw_id(), ecs_id(EcsStruct))
        );

        if (s && s->members.array)
        {
            const void* compData = ecs_get_id(world, entityId, id.raw_id());
            const ecs_member_t* members = static_cast<const ecs_member_t*>(s->members.array);

            for (int32_t i = 0; i < s->members.count; i++)
            {
                const ecs_member_t& m = members[i];
                FieldInfo field;
                field.name = m.name ? m.name : "?";
                field.typeName = GetTypeName(world, m.type);
                field.value = compData ? ReadFieldValue(world, compData, m) : "N/A";
                comp.fields.push_back(field);
            }
        }

        s_compCache.push_back(std::move(comp));
    });

    return static_cast<int>(s_compCache.size());
}

INSPECTOR_API const char* Inspector_GetComponentName(uint64_t, int index)
{
    std::lock_guard lock(s_cacheMutex);

    if (index < 0 || index >= s_compCache.size())
        return "";
    return s_compCache[index].name.c_str();
}

int Inspector_GetFieldCount(int compIndex)
{
    if (compIndex < 0 || compIndex >= s_compCache.size())
        return 0;
    return static_cast<int>(s_compCache[compIndex].fields.size());
}

const char* Inspector_GetFieldName(int compIndex, int fieldIndex)
{
    if (compIndex < 0 || compIndex >= s_compCache.size())
        return "";

    auto& fields = s_compCache[compIndex].fields;

    if (fieldIndex < 0 || fieldIndex >= fields.size())
        return "";

    return fields[fieldIndex].name.c_str();
}

const char* Inspector_GetFieldType(int compIndex, int fieldIndex)
{
    if (compIndex < 0 || compIndex >= s_compCache.size())
        return "";

    auto& fields = s_compCache[compIndex].fields;

    if (fieldIndex < 0 || fieldIndex >= fields.size())
        return "";

    LogInfo("Field type - {}", fields[fieldIndex].typeName);
    return fields[fieldIndex].typeName.c_str();
}

const char* Inspector_GetFieldValue(int compIndex, int fieldIndex)
{
    if (compIndex < 0 || compIndex >= s_compCache.size())
        return "";

    auto& fields = s_compCache[compIndex].fields;

    if (fieldIndex < 0 || fieldIndex >= fields.size())
        return "";

    return fields[fieldIndex].value.c_str();
}
}
