# SYNTHIA Technical Specification v0.1

## Objective

Build a local executable that can generate a procedural 3D world and allow a human to explore it while AI-controlled actors move within it.

## Scope

The v0.1 build includes:

- terrain
- sky and lighting
- roads and buildings
- vegetation
- gravity and collisions
- basic vehicles
- basic pedestrians
- first-person human control
- weather and time
- RGB, depth, segmentation, and object-ID capture
- scenario-based world configuration

## Non-goals for v0.1

The following are explicitly out of scope for the first implementation:

- VR as a primary interface
- multiplayer
- LLM-powered NPCs as default behavior
- massive city-scale simulation
- cloud GPU orchestration
- mobile application support
- blockchain or marketplace features

## Target architecture

```text
synthia/
├── engine/
│   ├── world/
│   ├── physics/
│   ├── entities/
│   ├── environment/
│   ├── agents/
│   └── sensors/
├── generation/
│   ├── terrain/
│   ├── roads/
│   ├── buildings/
│   └── vegetation/
├── scenarios/
├── data/
│   ├── capture/
│   ├── annotation/
│   └── export/
├── ai/
│   ├── navigation/
│   ├── perception/
│   └── training/
├── api/
├── backend/
├── frontend/
├── docs/
└── tests/
```

## Core requirements

### 1. Reproducibility

Every world must be reproducible via seed and versioned configuration.

### 2. World state

The project must maintain a consistent world state object that includes entity transforms, weather, time, and simulation data.

### 3. Human interaction

A user must be able to walk through the world and interact with objects.

### 4. AI presence

Pedestrians and vehicles must move according to clearly defined navigation rules.

### 5. Data capture

The engine must output sensor data in a structured format.

### 6. Scenario variation

It should be possible to vary traffic density, weather, and pedestrian count.

## Success metric

The v0.1 milestone succeeds if a user can:

1. generate a world
2. enter the world
3. observe simulated agents moving
4. capture sensor outputs in a machine-readable form

## Next steps

1. configure the engine toolchain
2. create the first world generator prototype
3. add terrain and roads
4. add the first human controller
5. add vehicles and pedestrians
6. add data exports and scenario controls
