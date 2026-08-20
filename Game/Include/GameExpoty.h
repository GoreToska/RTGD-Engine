//
// Created by ivan on 8/18/26.
//

#pragma once

#ifdef _WIN32
#ifdef GAME_EXPORTS
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif
#else
#define GAME_API __attribute__((visibility("default")))
#endif
