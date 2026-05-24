# Experimental 3D Game Engine (Direct3D 12)

This is an **experimental 3D game/graphics engine** built using **Direct3D 12**, with a focus on **modern rendering techniques** and **ECS-based design**.

The engine’s purpose is to serve as a platform for exploring **graphics programming, real-time rendering, and engine architecture**, supporting both **games** and **interactive simulations**.

---

## Current Features
- Entity-Component-System (ECS) architecture using [EnTT](https://github.com/skypjack/entt)  
- Deferred rendering pipeline with Physically Based Rendering (PBR)  
- Skeletal animation support with skinned mesh loading via [tinygltf](https://github.com/syoyo/tinygltf)  
- JSON-based scene save/load system
- Compute shader skinning
- Ray traced shadows, ambient occlusion and reflections for static and skinned meshes using DirectX Raytracing (DXR)
- BLAS refit for animated entities
- GUI using ImGUi for object manipulation
- Physics using [Nvidia PhysX](https://github.com/NVIDIA-Omniverse/PhysX)
  
---

## Roadmap / Work in Progress
- global illumination (RTXGI)   
- Scene graph for hierarchical transformations  

---

## Project Goals
- Explore **modern rendering techniques** (PBR, ray tracing, IBL, advanced post-processing)  
- Build a **robust ECS-driven architecture** that cleanly separates engine systems  
- Integrate **physics, AI, and audio** to simulate complex real-time environments  
- Create a flexible platform for experimenting with **gameplay and simulation systems**  

---

## Showcase


https://github.com/user-attachments/assets/594bed25-847d-4d6b-9798-8e68b22c6c61





https://github.com/user-attachments/assets/6e91a4ea-0161-452a-8148-3c3259c7cda5



<img width="1390" height="960" alt="Screenshot 2026-05-23 181026" src="https://github.com/user-attachments/assets/62392d37-6bdc-4716-bb03-6a18334686d8" />


## Build Instructions
1. Clone the repository
2. Run install_deps.bat to download necessary dependencies
3. Use CMake to generate the project files. The path must be [ProjectPath]/build
4. Build in x64 Debug/Release   
