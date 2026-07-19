# RTGD-Engine
A cross-platform 3D game engine with a standalone editor, written from scratch in C++20.
Runs on Windows (Direct3D 12) and Linux (Vulkan).

## What is this
A personal project built to explore the parts of engine development that are hard to
reach in day-to-day work: multithreaded rendering, handle-based resource management,
runtime reflection and getting engine data across a language boundary into a
separate editor process.

The engine is a shared library with a C ABI. The editor is a separate C#/Avalonia
application that hosts the engine's rendering surface as a native child window and
drives it through that ABI. Game code will live in its own module, but it is WIP for now.

## Highlights
**Multithreaded architecture.** The render thread owns the platform window and the
frame loop, and runs independently of the UI thread. Startup hands the initialization
result back through std::promise; resize and entity-picking requests cross the
thread boundary through mutex-guarded request slots and a condition variable.

**Deferred renderer with PBR.** G-buffer pass followed by a fullscreen lighting pass,
metallic-roughness workflow, HLSL shaders compiled through Diligent's abstraction
layer to D3D12 or Vulkan depending on the platform.

**GPU-based entity picking.** The editor requests a pick at screen coordinates; the
render thread reads back the entity ID from the G-buffer using a fence for
synchronization and returns the resolved ECS entity. No CPU-side raycasting.

**Handle-based resource management.** Meshes, textures and materials live in slot
pools with generation counters and reference counts. Handles are validated against
generations, so stale handles are detected rather than dereferenced. Destruction is
deferred to a safe point in the frame.

**Asynchronous asset pipeline.** Mesh and texture imports are dispatched to a job
system built on enkiTS and completed via callbacks. Assets are deduplicated by
normalized path, so requesting the same file twice returns the same handle.

**Scene serialization.** Scenes serialize to JSON with entity hierarchies, component
values, and asset references by path. Scenes can be loaded and unloaded additively at
runtime without restarting the engine.

## Features
### Rendering
- Deferred shading with a G-buffer pass and a fullscreen lighting pass
- Physically based rendering, metallic-roughness workflow
- Pipeline state and G-buffer factories, shader loading and caching
- Directional and point lights driven by ECS components
- Dedicated render thread with deferred resize handling
- GPU entity picking with fence-synchronized readback

### Scene and ECS
- Entity-component-system built on flecs
- Transform, camera, mesh, render, light, velocity and UUID components
- Camera, editor camera, movement, light and timer systems
- JSON scene serialization with additive load and unload at runtime
- Event bus for engine-wide notifications

### Assets
- glTF mesh import through assimp, texture import through stb_image
- Slot pools with generation counters, reference counting and deferred destruction
- Asynchronous import on a job system with completion callbacks
- Path-normalized deduplication, custom .mat material format

### Editor
- Separate C# / Avalonia application
- Scene hierarchy with entity renaming and deletion
- Asset browser, viewport hosting the engine's native surface
- Input injection from editor to engine

### Platform
- Windows: Win32 windowing, Direct3D 12
- Linux: X11 windowing, Vulkan
- Embedded-window mode for hosting inside the editor
- CMake build, dependencies as git submodules

## Building
### Requirements
| | Windows | Linux |
|---|---|---|
| Compiler | MSVC 2022 (C++20) | GCC 11+ or Clang 14+ (C++20) |
| CMake | 3.20+ | 3.20+ |
| Graphics | Windows 10+ with D3D12 | Vulkan SDK |
| Editor | .NET 10 SDK | .NET 10 SDK |

### Engine and standalone runtime
```bash
git clone --recursive https://github.com/GoreToska/RTGD-Engine.git
cd RTGD-Engine
 
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

If you cloned without `--recursive` (it happens):
 
```bash
git submodule update --init --recursive
```

The standalone runtime is written to `build/bin/`. Assets and shaders are synced there
automatically as part of the build.

### Editor
Build the engine first — the editor loads the native library from the CMake output
directory.
 
```bash
cd Editor
dotnet run -c Release
```

## Third-party libraries
| Library | Purpose |
|---|---|
| [Diligent Engine](https://github.com/DiligentGraphics/DiligentEngine) | Graphics API abstraction (D3D12 / Vulkan) |
| [flecs](https://github.com/SanderMertens/flecs) | ECS and runtime reflection |
| [enkiTS](https://github.com/dougbinks/enkiTS) | Task scheduler |
| [assimp](https://github.com/assimp/assimp) | Mesh import |
| [gainput](https://github.com/jkuhlmann/gainput) | Input handling |
| [spdlog](https://github.com/gabime/spdlog) | Logging |
| [stb](https://github.com/nothings/stb) | Image loading |
| [nlohmann/json](https://github.com/nlohmann/json) | Scene serialization |
| [Avalonia](https://avaloniaui.net/) | Editor UI |
 
---

## License

Apache-2.0. See [LICENSE](LICENSE).
