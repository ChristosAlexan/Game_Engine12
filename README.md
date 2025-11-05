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
- Ray traced shadows and ray traced reflections for static and skinned meshes using DirectX Raytracing (DXR)
- BLAS refit for animated entities
- GUI using ImGUi for object manipulation
- Physics using [Nvidia PhysX](https://github.com/NVIDIA-Omniverse/PhysX)
  
---

## Roadmap / Work in Progress
- global illumination (RTXGI)  
- Hybrid lighting pipeline (deferred + ray tracing)  
- Advanced physics simulation (PhysX integration)  
- Scene graph for hierarchical transformations  
- Post-processing effects: Bloom, SSR, SSAO  

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



## Build Instructions
1. Clone the repository
2. Run install_deps.bat to download necessary dependencies
3. Open the project in Visual Studio 2026  
4. Build in x64 Debug/Release   
