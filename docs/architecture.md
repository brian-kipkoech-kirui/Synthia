# SYNTHIA Architecture

## Principle

SYNTHIA should be built in layers instead of as one giant application.

```text
SYNTHIA
│
├── World Creation
│   ├── procedural generation
│   ├── world specification
│   └── seed-based reproducibility
│
├── World Users
│   ├── human operators
│   └── AI agents
│
├── World Runtime
│   ├── physics
│   ├── weather
│   ├── simulation
│   └── environment updates
│
├── Data Engine
│   ├── sensors
│   ├── events
│   └── telemetry
│
├── AI Platform
│   ├── training
│   ├── testing
│   └── evaluation
│
└── Interface Layer
    ├── API
    ├── SDK
    └── web platform
```

## Core components

### 1. World Generator

Creates procedural environments from a seed and specification.

Responsibilities:

- terrain generation
- road generation
- building placement
- vegetation generation
- lighting and weather setup

### 2. World Runtime

Runs the live simulation.

Responsibilities:

- ticking the simulation loop
- applying physics
- updating entities
- resolving collisions
- managing time and climate

### 3. Agent Engine

Controls AI behavior and human-driven actors.

Responsibilities:

- goals and memory
- navigation and path planning
- stochastic behavior models
- utility/behavior-tree logic
- RL interfaces

### 4. Human Interface

Lets people enter and manipulate the environment.

Responsibilities:

- first-person world exploration
- object interaction
- vehicle control
- researcher/debug tools

### 5. Synthetic Data Engine

Turns simulation into datasets.

Responsibilities:

- RGB capture
- depth capture
- segmentation
- object annotations
- trajectories and metadata

### 6. AI Laboratory

Runs experiments and training inside the simulated world.

Responsibilities:

- scenario generation
- evaluation loops
- experiment tracking
- failure analysis

## World state model

The simulation should be driven by a world state object that captures all relevant runtime information.

```json
{
  "time": 1734.42,
  "weather": {
    "temperature": 24.7,
    "rain": 0.82,
    "wind_speed": 13.4
  },
  "entities": [
    {
      "id": "car_001",
      "type": "vehicle",
      "position": [104.2, 0.0, 83.7],
      "velocity": [12.3, 0, 0],
      "heading": 1.57
    }
  ]
}
```

This state should be the shared source of truth for both simulation and data generation.

## Simulation loop

```text
read world state
collect inputs
update agents
apply physics
environment update
resolve collisions
capture sensors
generate events
record telemetry
advance time
```

## Abstraction principle

The simulation backend should not define the product identity.

```text
SYNTHIA API
  -> SYNTHIA World SDK
  -> SYNTHIA World Model
  -> Simulation Adapter
     -> Unreal
     -> Unity
     -> Godot
     -> custom runtime
```

This keeps the platform portable.

## Initial stack

### Core simulation

- headless C++ world engine as the primary runtime
- Unreal Engine 5 as a later visual layer and human inspection frontend

### Programming languages

- C++ for deterministic simulation, world state, agents, and run loop
- optional external tooling may be added later, but the core platform should not be Python dependent

### AI / training interfaces

- custom agent API with deterministic observation/action contracts
- future support for external learning frameworks and policy models
- no requirement for the simulation core itself to be built on Python

### Backend and data

- PostgreSQL for metadata, worlds, scenarios, and experiment tracking
- MinIO/S3-compatible object storage for datasets and artifacts
- Docker for containerized deployment

### Deployment

- Docker first
- Kubernetes later for massive distributed simulation workloads

## Design constraints

- reproducibility is mandatory
- simulation truth must be separate from rendered appearance
- the first version must remain modular and testable
- world generation must support deterministic seeds
- synthetic data must be exportable in machine-readable formats- the system must support a headless execution mode for large-scale AI workloads
- visual rendering is a secondary layer, not the foundation of the core simulation

## Execution modes

```text
SYNTHIA
├── HEADLESS MODE
│   ├── agent training
│   ├── large scenario batches
│   ├── evaluation and failure analysis
│   └── replay and logging
└── VISUAL MODE
    ├── Unreal Engine 5 inspection
    ├── human observation
    └── debugging and demonstration
```

This architecture keeps the generator, world model, agents, and evaluation loop independent from the renderer.

## Recommended development phases

### Phase 1 — Core

- C++ world engine
- headless simulation
- scenario and experiment loop
- deterministic world state
- agent action contracts
- telemetry and replay

### Phase 2 — 3D

- Unreal Engine 5 integration
- human visualization
- sensor inspection
- environment realism and debugging

### Phase 3 — AI laboratory

- multi-agent evaluation
- recording and analysis of failures
- synthetic dataset generation
- targeted scenario creation

### Phase 4 — Scale

- Dockerized batch execution
- distributed simulation workloads
- cloud orchestration for large-scale runs