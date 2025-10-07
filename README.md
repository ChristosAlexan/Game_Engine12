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
- Ray traced shadows for static and skinned meshes using DirectX Raytracing (DXR)
- BLAS rebuild for animated entities
- GUI using ImGUi for object manipulation
- Physics using [Nvidia PhysX](https://github.com/NVIDIA-Omniverse/PhysX)
  
---

## Roadmap / Work in Progress
- Ray traced reflections and global illumination (RTXGI)  
- Hybrid lighting pipeline (deferred + ray tracing)  
- Advanced physics simulation (PhysX integration)  
- Scene graph for hierarchical transformations  
- Post-processing effects: Bloom, SSR, HBAO+  

---

## Project Goals
- Explore **modern rendering techniques** (PBR, ray tracing, IBL, advanced post-processing)  
- Build a **robust ECS-driven architecture** that cleanly separates engine systems  
- Integrate **physics, AI, and audio** to simulate complex real-time environments  
- Create a flexible platform for experimenting with **gameplay and simulation systems**  

---

## Showcase
BLAS rebuild for skinned meshes

https://github.com/user-attachments/assets/650d4ee7-dd54-4406-b15b-6ee7eae12df7

Multiple light types with ray traced shadows


https://github.com/user-attachments/assets/c56da566-f1b1-4c77-a806-4428bbaf84f7



PBR rendering

<img width="1585" height="891" alt="Screenshot 2025-09-28 034615" src="https://github.com/user-attachments/assets/fe1b1135-d480-47fd-85ec-b9313667b8fa" />

---

Physics



https://github.com/user-attachments/assets/8f1afb74-fdba-4e87-8c4c-cb77a1199aab



## Build Instructions
1. Clone the repository
2. Run install_deps.bat to download necessary dependencies
3. Open the project in Visual Studio 2026  
4. Build in x64 Debug/Release   
