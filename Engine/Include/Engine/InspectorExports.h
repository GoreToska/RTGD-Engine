//
// Created by gorev on 08.04.2026.
//

#pragma once

#ifdef _WIN32
#ifdef INSPECTOR_EXPORTS
#define INSPECTOR_API __declspec(dllexport)
#else
#define INSPECTOR_API __declspec(dllimport)
#endif
#else
#define INSPECTOR_API __attribute__((visibility("default")))
#endif

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#include <flecs.h>

struct FieldInfo {
    std::string name;
    std::string typeName;
    std::string value; // empty when isStruct == true
    std::vector<FieldInfo> children; // populated when isStruct == true
    bool isStruct = false;

    ecs_entity_t entityId = 0; // owning entity
    ecs_id_t componentId = 0; // ECS component id (for ecs_get_mut_id)
    size_t byteOffset = 0; // byte offset from component base
    ecs_entity_t typeId = 0;
};

struct ComponentInfo {
    std::string name;
    std::vector<FieldInfo> fields;
};


#ifdef __cplusplus
extern "C" {
#endif

INSPECTOR_API int Inspector_GetComponentCount(uint64_t entityId);

/// Name is valid until next Inspector_GetComponentCount call
INSPECTOR_API const char *Inspector_GetComponentName(uint64_t entityId, int compIndex);

INSPECTOR_API int Inspector_GetFieldCount(int compIndex);

INSPECTOR_API const char *Inspector_GetFieldName(int compIndex, int fieldIndex);

INSPECTOR_API const char *Inspector_GetFieldType(int compIndex, int fieldIndex);

INSPECTOR_API const char *Inspector_GetFieldValue(int compIndex, int fieldIndex);

INSPECTOR_API int Inspector_GetSubFieldCount(int compIndex, int fieldIndex);

INSPECTOR_API const char *Inspector_GetSubFieldName(int compIndex, int fieldIndex, int subIndex);

INSPECTOR_API const char *Inspector_GetSubFieldType(int compIndex, int fieldIndex, int subIndex);

INSPECTOR_API const char *Inspector_GetSubFieldValue(int compIndex, int fieldIndex, int subIndex);

INSPECTOR_API int Inspector_SetFieldValue(int compIndex, int fieldIndex, const char *value);

INSPECTOR_API int Inspector_SetSubFieldValue(int compIndex, int fieldIndex, int subIndex, const char *value);

INSPECTOR_API int Inspector_RefreshValues();

#ifdef __cplusplus
}
#endif
