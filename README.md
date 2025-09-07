# Experimental 3D Game Engine (Direct3D 12)

This is an **experimental 3D game/graphics engine** built using **Direct3D 12**, with a focus on **modern rendering techniques** and **ECS-based design**.

The engine’s purpose is to serve as a platform for exploring **graphics programming, real-time rendering, and engine architecture**, supporting both **games** and **interactive simulations**.

---

## Current Features
- Entity-Component-System (ECS) architecture using [EnTT](https://github.com/skypjack/entt)  
- Deferred rendering pipeline with Physically Based Rendering (PBR)  
- Skeletal animation support with skinned mesh loading via [tinygltf](https://github.com/syoyo/tinygltf)  
- JSON-based scene save/load system  
- Ray traced shadows using DirectX Raytracing (DXR)
- GUI using ImGUi for object manipulation

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

## Screenshots / Demos
<img width="1599" height="898" alt="image" src="https://github.com/user-attachments/assets/6daae816-80a5-4a16-adf5-0bc72014997c" />
 

---

## Build Instructions
1. Clone the repository
2. Run install_deps.bat to download necessary dependencies
3. Open the project in Visual Studio 2022  
4. Build in x64 Debug/Release   
