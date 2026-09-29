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
drives it through that ABI. Game code lives in its own shared library, loaded at runtime
and hot-reloadable without restarting the engine.

## Highlights
**Multithreaded architecture.** The render thread owns the platform window and the
frame loop, and runs independently of the UI thread. Startup hands the initialization
result back through std::promise; resize and entity-picking requests cross the
thread boundary through mutex-guarded request slots and a condition variable.

**Deferred renderer with PBR.** G-buffer pass followed by a fullscreen lighting pass,
metallic-roughness workflow, HLSL shaders compiled through Diligent's abstraction
layer to D3D12 or Vulkan depending on the platform.

**Render graph.** Each pass declares what it reads and writes; the graph inserts
resource-state barriers automatically and owns transient render targets in a pool that
survives across frames and is rebuilt on resize. The whole G-buffer, the scene color
target and the shadow atlas are graph-owned, so adding an effect means adding a pass.

**Cascaded shadow maps.** Up to four cascades packed into a single depth atlas, with
practical split scheme, texel snapping against shimmering, per-cascade depth and normal
bias in world units, and blending between cascades.

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

**Rigid-body physics based on Jolt.** Box, sphere, capsule, mesh and convex hull colliders,
triggers, raycasts and shape casts, physical and virtual character controllers, and a
data-driven gameplay collision-layer matrix configured from JSON. Six two-body
constraint types (hinge, slider, fixed, distance, cone, swing-twist) cover everything
from a hinged door to a fully articulated ragdoll.

**Audio based on FMOD Studio.** Banks are loaded through the same ref-counted asset
system as meshes and textures and unloaded when the last reference goes away. Positional
sources and listeners are ECS components; one-shot events can follow an entity, and
Doppler is computed per source from frame-to-frame motion with a teleport threshold.
Mixer buses, global parameters and snapshots are exposed to gameplay code.

## Features
### Rendering
- Deferred shading with a G-buffer pass and a fullscreen lighting pass
- Physically based rendering, metallic-roughness workflow
- Render graph with declared pass inputs/outputs, automatic barriers and pooled transient targets
- Cascaded shadow maps for the directional light: atlas-based, texel-snapped, 3x3 PCF, cascade blending
- Debug line drawing, including wireframes of every collider shape
- Pipeline state factory, shader loading and caching
- Directional and point lights driven by ECS components
- Dedicated render thread with deferred resize handling
- GPU entity picking with fence-synchronized readback

### Scene and ECS
- Entity-component-system built on flecs
- Transform, camera, mesh, render, light, velocity, UUID, audio source and audio listener components
- Camera, editor camera, movement, light and timer systems
- JSON scene serialization with additive load and unload at runtime
- Event bus for engine-wide notifications
- Game code in a separate shared library, hot-reloaded with Ctrl+R

### Physics
- Rigid bodies (static, dynamic, kinematic) based on Jolt Physics
- Box, sphere, capsule, mesh and convex hull colliders, with mesh decimation for mesh/hull shapes
- Triggers with enter/stay/exit events, raycasts and sphere/box casts (single-hit and multi-hit), all layer-mask aware
- Data-driven gameplay collision layers and collision matrix, loaded from JSON
- Physical and virtual character controllers
- Hinge, slider, fixed, distance, cone and swing-twist constraints, for joints and ragdolls

### Audio
- FMOD Studio integration: bank loading, event playback, 2D and 3D positional events
- Audio source and listener ECS components, one-shot events attached to entities
- Per-source Doppler from position deltas, with a configurable max speed to ignore teleports
- Bus volume and pause, master volume, global parameters, snapshots
- Bootstrap banks configured from JSON, sample data preloaded on bank load

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
| Audio | FMOD Engine 2.03 (in repository) | FMOD Engine 2.03 (in repository) |

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

The FMOD Engine SDK for both platforms is already in `ThirdParty/FMOD/`, so no separate
download is needed. The FMOD Studio project lives in `FMOD/RTGD/`; built banks are
committed to `Assets/Audio/Desktop/`, so FMOD Studio is only needed to change the sound
content.

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
| [Jolt Physics](https://github.com/jrouwe/JoltPhysics) | Rigid body physics |
| [FMOD](https://www.fmod.com) | Audio (FMOD Studio / Core API) |
| [enkiTS](https://github.com/dougbinks/enkiTS) | Task scheduler |
| [assimp](https://github.com/assimp/assimp) | Mesh import |
| [meshoptimizer](https://github.com/zeux/meshoptimizer) | Mesh simplification for collider decimation & LOD |
| [gainput](https://github.com/jkuhlmann/gainput) | Input handling |
| [spdlog](https://github.com/gabime/spdlog) | Logging |
| [stb](https://github.com/nothings/stb) | Image loading |
| [nlohmann/json](https://github.com/nlohmann/json) | Scene serialization |
| [Avalonia](https://avaloniaui.net/) | Editor UI |
 
---
## License

Apache-2.0. See [LICENSE](LICENSE).
