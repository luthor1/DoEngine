# DoEngine

DoEngine is a modern, high-performance 3D game engine built from scratch using C++20. It's designed with a modular architecture and focus on data-oriented design (ECS).

## Features
- **RHI (Render Hardware Interface):** Abstracted graphics API using NVIDIA's NVRHI.
- **ECS (Entity Component System):** Powered by EnTT for efficient entity management.
- **Scripting:** C# scripting support using CoreCLR.
- **Job System:** High-performance task-based parallelism using enkiTS.
- **Custom Memory Management:** Linear, Pool, and Stack allocators to prevent fragmentation.
- **Virtual File System (VFS):** Unified access to physical and packed assets.
- **Physics:** Integrated Jolt Physics for robust simulation.
- **Audio:** Spatial audio with miniaudio support.
- **Asset Pipeline:** Binary optimization for meshes (FBX) and textures (PNG).

## Prerequisites
- **CMake** (v3.20 or later)
- **Vulkan SDK** (for NVRHI Vulkan backend)
- **.NET SDK** (for C# scripting support)
- **Visual Studio** or a standard C++20 compiler

## Getting Started
To configure and build the project, run:
```bash
cmake -B build -S .
cmake --build build
cmake --build build --target DoEngine
```

## License
MIT (or your preferred license)
