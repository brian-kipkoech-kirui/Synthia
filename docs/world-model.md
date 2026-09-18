# World Model

## Concept

A world is more than a 3D scene. It is a structured model of a dynamic environment.

```text
World
├── Environment
│   ├── terrain
│   ├── buildings
│   ├── roads
│   ├── vegetation
│   └── infrastructure
├── Physics
│   ├── gravity
│   ├── collision
│   ├── friction
│   ├── materials
│   └── constraints
├── Climate
│   ├── temperature
│   ├── rain
│   ├── wind
│   ├── fog
│   └── lighting
├── Entities
│   ├── humans
│   ├── animals
│   ├── vehicles
│   ├── robots
│   └── objects
├── Rules
│   ├── traffic
│   ├── economy
│   ├── social
│   └── physical
├── Time
│   ├── day/night
│   ├── seasons
│   └── historical state
└── Sensors
    ├── cameras
    ├── LiDAR
    ├── radar
    ├── GPS
    └── IMU
```

## World identity

Each world must have a reproducible identity:

```text
WORLD_ID
SEED
VERSION
ENGINE_VERSION
SCENARIO_VERSION
```

Example:

```text
world_8fa31
seed=728194
engine=0.1.0
scenario=street_crossing_v3
```

## Deterministic generation

The same seed and version should generate the same world.

This enables:

- reproducible experiments
- debugging failures
- comparing models under controlled conditions
- scientific research and long-term validation

## Simulation truth vs rendering

The engine must maintain simulation truth separately from visuals.

For example, the object model stores:

- semantic identity
- class
- physical state
- position and velocity
- material and constraints

The renderer chooses only how the object is displayed.

This keeps data generation reliable even when the visual layer changes.
