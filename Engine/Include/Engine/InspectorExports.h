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

struct FieldInfo
{
    std::string name;
    std::string typeName;
    std::string value;                // empty when isStruct == true
    std::vector<FieldInfo> children;  // populated when isStruct == true
    bool isStruct = false;
};

struct ComponentInfo
{
    std::string name;
    std::vector<FieldInfo> fields;
};


static void ReadStructFields(flecs::world_t* world,
                             const void*    data,
                             ecs_entity_t   typeId,
                             std::vector<FieldInfo>& out);

/*#ifdef __cplusplus
extern "C"
{
#endif

INSPECTOR_API int Inspector_GetComponentCount(uint64_t entityId);

/// Name is valid until next Inspector_GetComponentCount call
INSPECTOR_API const char* Inspector_GetComponentName(uint64_t entityId, int index);

INSPECTOR_API int Inspector_GetFieldCount(int compIndex);

INSPECTOR_API const char* Inspector_GetFieldName(int compIndex, int fieldIndex);

INSPECTOR_API const char* Inspector_GetFieldType(int compIndex, int fieldIndex);

INSPECTOR_API const char* Inspector_GetFieldValue(int compIndex, int fieldIndex);
#ifdef __cplusplus
}
#endif*/
