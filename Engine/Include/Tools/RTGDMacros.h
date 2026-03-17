//
// Created by gorev on 17.03.2026.
//

#pragma once
#define DECLARE_SINGLETON(ClassName)                             \
public:                                                          \
static ClassName& Instance() {                              \
static ClassName instance;                              \
return instance;                                         \
}                                                            \
ClassName(const ClassName&)            = delete;            \
ClassName& operator=(const ClassName&) = delete; \
private: \
ClassName() = default;
