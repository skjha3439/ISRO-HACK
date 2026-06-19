# Thunderbolts: SfS-Dynamics Rover Path Proctor System
### Bharatiya Antariksh Hackathon 2026 | Problem Statement #8

An open-source, risk-aware planetary pathfinding engine that integrates Chandrayaan-2 Orbiter High-Resolution Camera (OHRC) imagery data via **Shape-from-Shading (SfS)** models with an optimized C++ $A^*$ navigation kernel to proctor safe lunar traverses inside Doubly Shadowed Craters (DPSRs).

## 👥 Team Members (Team Thunderbolts)
* **Shivam Kumar Jha** - IIIT Manipur
* **Abhinav Jha** - IIIT Manipur
* **Abhinav Kumar Jha** - IIIT Manipur
* **Arihant Mishra** - IIIT Manipur

## 🚀 Core Features
* **Sub-meter Micro-Topography Analysis:** Implements a Shape-from-Shading kernel (derived from framework base *arXiv:2604.17436*) to resolve micro-hazards from grazing illumination data.
* **Energy-Aware Path Optimization:** High-performance C++ execution matrix evaluating continuous localized slope gradients and applying scaled energy penalties (`1.0 + slope / 10.0`) to paths.
* **Dynamic Safety Proctorship:** Interactive CLI allowing real-time optimization parameter configuration (Smoothness Weights $W$ and maximum traversable vehicle slope tolerance).
* **Instant Matrix Rendering:** Fast Python-based matrix processing pipeline (`matplotlib`, `numpy`) for 2D cartographic trajectory visualization.

## 🏗️ Repository Architecture
* `pathfinder_sfs.cpp` — Native C++ core engine handling priority queues, neighbor evaluations, and SfS cost-sweeps.
* `visualizer.py` — Python visualizer drawing landing vectors, slope matrices, and final calculated traverse graphs.
* `lunar_grid.csv` — Generated environment cell matrix mapping targets, safe zones, and active hazards.
* `rover_path.csv` — Optimized output tracking coordinates chosen by the $A^*$ calculation pathfinder.

## 🛠️ Execution Instructions

### 1. Compile the Core C++ Engine
```bash
g++ pathfinder_sfs.cpp -o pathfinder_sfs
