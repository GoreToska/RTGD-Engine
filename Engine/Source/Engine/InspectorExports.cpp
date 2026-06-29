#include "Engine/InspectorExports.h"

#include <vector>
#include <string>
#include <mutex>
#include <flecs/addons/meta.h>

#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Tools/Logger.h"


static std::vector<ComponentInfo> s_compCache;
static std::mutex s_cacheMutex;
static flecs::world_t *s_world = nullptr;
static uint64_t s_entityId = 0; // entity that owns the cache


static std::string GetTypeName(flecs::world_t *world, ecs_entity_t type) {
    const EcsPrimitive *prim = ecs_get(world, type, EcsPrimitive);
    if (prim) {
        switch (prim->kind) {
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
    const char *name = ecs_get_name(world, type);
    return name ? name : "unknown";
}

static bool WritePrimValue(void *ptr, flecs::world_t *world, ecs_entity_t typeId, const char *str) {
    const EcsPrimitive *prim = ecs_get(world, typeId, EcsPrimitive);
    if (!prim || !ptr || !str)
        return false;
    try {
        switch (prim->kind) {
            case EcsF32:
                *static_cast<float *>(ptr) = std::stof(str);
                return true;
            case EcsF64:
                *static_cast<double *>(ptr) = std::stod(str);
                return true;
            case EcsI8:
                *static_cast<int8_t *>(ptr) = (int8_t) std::stoi(str);
                return true;
            case EcsI16:
                *static_cast<int16_t *>(ptr) = (int16_t) std::stoi(str);
                return true;
            case EcsI32:
                *static_cast<int32_t *>(ptr) = std::stoi(str);
                return true;
            case EcsI64:
                *static_cast<int64_t *>(ptr) = std::stoll(str);
                return true;
            case EcsU8:
                *static_cast<uint8_t *>(ptr) = (uint8_t) std::stoul(str);
                return true;
            case EcsU16:
                *static_cast<uint16_t *>(ptr) = (uint16_t) std::stoul(str);
                return true;
            case EcsU32:
                *static_cast<uint32_t *>(ptr) = std::stoul(str);
                return true;
            case EcsU64:
                *static_cast<uint64_t *>(ptr) = std::stoull(str);
                return true;
            case EcsBool: {
                std::string s(str);
                *static_cast<bool *>(ptr) = (s == "true" || s == "1");
                return true;
            }
            default:
                return false;
        }
    } catch (...) { return false; }
}

static std::string ReadPrimValue(const void *ptr, flecs::world_t *world, ecs_entity_t typeId) {
    const EcsPrimitive *prim = ecs_get(world, typeId, EcsPrimitive);
    if (!prim)
        return "?";
    switch (prim->kind) {
        case EcsF32:
            return std::to_string(*static_cast<const float *>(ptr));
        case EcsF64:
            return std::to_string(*static_cast<const double *>(ptr));
        case EcsI8:
            return std::to_string(*static_cast<const int8_t *>(ptr));
        case EcsI16:
            return std::to_string(*static_cast<const int16_t *>(ptr));
        case EcsI32:
            return std::to_string(*static_cast<const int32_t *>(ptr));
        case EcsI64:
            return std::to_string(*static_cast<const int64_t *>(ptr));
        case EcsU8:
            return std::to_string(*static_cast<const uint8_t *>(ptr));
        case EcsU16:
            return std::to_string(*static_cast<const uint16_t *>(ptr));
        case EcsU32:
            return std::to_string(*static_cast<const uint32_t *>(ptr));
        case EcsU64:
            return std::to_string(*static_cast<const uint64_t *>(ptr));
        case EcsBool:
            return *static_cast<const bool *>(ptr) ? "true" : "false";
        case EcsString: {
            const char *s = *static_cast<const char * const*>(ptr);
            return s ? s : "(null)";
        }
        default:
            return "?";
    }
}

static std::string ReadFieldValue(const void *ptr, flecs::world_t *world, ecs_entity_t typeId) {
    static const ecs_entity_t strId = flecs::world(world).id<std::string>();
    if (typeId == strId)
        return *static_cast<const std::string *>(ptr);
    return ReadPrimValue(ptr, world, typeId);
}

static void *GetLiveComponentPtr(const FieldInfo &field) {
    if (!s_world || !field.entityId || !field.componentId)
        return nullptr;

    void *base = ecs_get_mut_id(s_world, field.entityId, field.componentId);
    if (!base)
        return nullptr;

    return static_cast<uint8_t *>(base) + field.byteOffset;
}

static void ReadStructFields(flecs::world_t *world,
                             const void *data,
                             ecs_entity_t typeId,
                             ecs_entity_t entityId,
                             ecs_id_t componentId,
                             size_t baseOffset,
                             std::vector<FieldInfo> &out) {
    const EcsStruct *s = ecs_get(world, typeId, EcsStruct);
    if (!s || !s->members.array)
        return;

    const ecs_member_t *members = static_cast<const ecs_member_t *>(s->members.array);

    for (int32_t i = 0; i < s->members.count; i++) {
        const ecs_member_t &m = members[i];

        FieldInfo field;
        field.name = m.name ? m.name : "?";
        field.typeName = GetTypeName(world, m.type);
        field.entityId = entityId;
        field.componentId = componentId;
        field.byteOffset = baseOffset + (size_t) m.offset;
        field.typeId = m.type;

        const void *memberData = data
                                     ? static_cast<const uint8_t *>(data) + m.offset
                                     : nullptr;

        const EcsStruct *nested = ecs_get(world, m.type, EcsStruct);
        if (nested) {
            field.isStruct = true;
            if (memberData)
                ReadStructFields(world, memberData, m.type,
                                 entityId, componentId,
                                 field.byteOffset, field.children);
        } else {
            field.isStruct = false;
            field.value = memberData
                              ? ReadFieldValue(memberData, world, m.type)
                              : "N/A";
        }

        out.push_back(std::move(field));
    }
}

static void RefreshFieldList(std::vector<FieldInfo> &fields) {
    for (auto &f: fields) {
        if (f.isStruct) {
            RefreshFieldList(f.children);
            continue;
        }

        void *ptr = GetLiveComponentPtr(f);
        if (ptr)
            f.value = ReadFieldValue(ptr, s_world, f.typeId);
    }
}

extern "C" {
INSPECTOR_API int Inspector_GetComponentCount(uint64_t entityId) {
    std::lock_guard lock(s_cacheMutex);
    s_compCache.clear();

    auto scene = RTGDEngine::SceneManager::Instance().GetActiveScene();
    if (!scene)
        return 0;

    auto &world = RTGDEngine::SceneManager::Instance().GetWorld();
    s_world = world.get_world();
    s_entityId = entityId;

    flecs::entity e(world, entityId);
    if (!e.is_valid())
        return 0;

    e.each([&](flecs::id id) {
        if (id.is_pair() || !id.is_entity())
            return;

        const ecs_type_info_t *ti = ecs_get_type_info(world, id);
        if (!ti)
            return;

        ComponentInfo comp;
        comp.name = ti->name ? ti->name : "Unknown";

        const void *compData = ecs_get_id(world, entityId, id.raw_id());
        ReadStructFields(s_world, compData, id.raw_id(),
                         entityId, id.raw_id(), 0,
                         comp.fields);

        s_compCache.push_back(std::move(comp));
    });

    return static_cast<int>(s_compCache.size());
}

INSPECTOR_API const char *Inspector_GetComponentName(uint64_t, int index) {
    std::lock_guard lock(s_cacheMutex);
    if (index < 0 || index >= (int) s_compCache.size())
        return "";

    return s_compCache[index].name.c_str();
}

INSPECTOR_API int Inspector_GetFieldCount(int c) {
    if (c < 0 || c >= (int) s_compCache.size())
        return 0;

    return (int) s_compCache[c].fields.size();
}

INSPECTOR_API const char *Inspector_GetFieldName(int c, int f) {
    if (c < 0 || c >= (int) s_compCache.size())
        return "";

    auto &fields = s_compCache[c].fields;
    if (f < 0 || f >= (int) fields.size())
        return "";

    return fields[f].name.c_str();
}

INSPECTOR_API const char *Inspector_GetFieldType(int c, int f) {
    if (c < 0 || c >= (int) s_compCache.size())
        return "";

    auto &fields = s_compCache[c].fields;
    if (f < 0 || f >= (int) fields.size())
        return "";

    return fields[f].typeName.c_str();
}

INSPECTOR_API const char *Inspector_GetFieldValue(int c, int f) {
    if (c < 0 || c >= (int) s_compCache.size())
        return "";

    auto &fields = s_compCache[c].fields;
    if (f < 0 || f >= (int) fields.size())
        return "";

    return fields[f].value.c_str();
}

INSPECTOR_API int Inspector_GetSubFieldCount(int c, int f) {
    if (c < 0 || c >= (int) s_compCache.size())
        return 0;

    auto &fields = s_compCache[c].fields;
    if (f < 0 || f >= (int) fields.size())
        return 0;

    return (int) fields[f].children.size();
}

INSPECTOR_API const char *Inspector_GetSubFieldName(int c, int f, int s) {
    if (c < 0 || c >= (int) s_compCache.size())
        return "";

    auto &ch = s_compCache[c].fields;
    if (f < 0 || f >= (int) ch.size())
        return "";

    auto &children = ch[f].children;
    if (s < 0 || s >= (int) children.size())
        return "";

    return children[s].name.c_str();
}

INSPECTOR_API const char *Inspector_GetSubFieldType(int c, int f, int s) {
    if (c < 0 || c >= (int) s_compCache.size())
        return "";

    auto &ch = s_compCache[c].fields;
    if (f < 0 || f >= (int) ch.size())
        return "";

    auto &children = ch[f].children;
    if (s < 0 || s >= (int) children.size())
        return "";

    return children[s].typeName.c_str();
}

INSPECTOR_API const char *Inspector_GetSubFieldValue(int c, int f, int s) {
    if (c < 0 || c >= (int) s_compCache.size())
        return "";

    auto &ch = s_compCache[c].fields;
    if (f < 0 || f >= (int) ch.size())
        return "";

    auto &children = ch[f].children;
    if (s < 0 || s >= (int) children.size())
        return "";

    return children[s].value.c_str();
}

static bool WriteFieldValue(void *ptr, flecs::world_t *world, ecs_entity_t typeId, const char *value) {
    static const ecs_entity_t strId = flecs::world(world).id<std::string>();
    if (typeId == strId) {
        *static_cast<std::string *>(ptr) = value;
        return true;
    }
    return WritePrimValue(ptr, world, typeId, value);
}

INSPECTOR_API int Inspector_SetFieldValue(int c, int f, const char *value) {
    std::lock_guard lock(s_cacheMutex);
    if (!s_world || c < 0 || c >= (int) s_compCache.size())
        return 0;

    auto &fields = s_compCache[c].fields;
    if (f < 0 || f >= (int) fields.size())
        return 0;

    auto &field = fields[f];
    if (field.isStruct)
        return 0;

    void *ptr = GetLiveComponentPtr(field);
    if (!ptr)
        return 0;

    bool ok = WriteFieldValue(ptr, s_world, field.typeId, value);
    if (ok) {
        ecs_modified_id(s_world, field.entityId, field.componentId);
        field.value = value;
    }

    LogInfo("Changed field value {} {} {}", c, f, value);
    return ok ? 1 : 0;
}

INSPECTOR_API int Inspector_SetSubFieldValue(int c, int f, int s, const char *value) {
    std::lock_guard lock(s_cacheMutex);

    if (!s_world || c < 0 || c >= (int) s_compCache.size())
        return 0;

    auto &fields = s_compCache[c].fields;
    if (f < 0 || f >= (int) fields.size())
        return 0;

    auto &children = fields[f].children;
    if (s < 0 || s >= (int) children.size())
        return 0;

    auto &sub = children[s];
    if (sub.isStruct)
        return 0;

    void *ptr = GetLiveComponentPtr(sub);
    if (!ptr)
        return 0;

    bool ok = WriteFieldValue(ptr, s_world, sub.typeId, value);
    if (ok) {
        ecs_modified_id(s_world, sub.entityId, sub.componentId);
        sub.value = value;
    }

    LogInfo("Changed subfield value {} {} {} {}", c, f, s, value);
    return ok ? 1 : 0;
}

INSPECTOR_API int Inspector_RefreshValues() {
    std::lock_guard lock(s_cacheMutex);

    if (s_compCache.empty() || !s_world)
        return 0;

    for (auto &comp: s_compCache)
        RefreshFieldList(comp.fields);

    return 1;
}
} // extern "C"
