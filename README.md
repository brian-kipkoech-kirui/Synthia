# SYNTHIA

SYNTHIA is a programmable world-generation and simulation platform where humans and AI agents inhabit the same environment. The goal is to generate worlds, simulate them, capture ground-truth sensor data, and provide a testbed for training and evaluating AI systems.

## Vision

Create a world, populate it, simulate it, explore it, and learn from it.

SYNTHIA combines:

- world generation
- runtime simulation
- agent behavior
- synthetic data generation
- AI training and evaluation
- optional digital-twin workflows

## Core concept

The core product is not a static game scene. It is a world engine that can:

- generate reproducible environments from seeds
- simulate physics and environment change
- run human and AI agents in the same world
- export camera, depth, segmentation, LiDAR, and annotation datasets
- support scenario testing, evaluation, and reinforcement learning

## Repository layout

```text
SYNTHIA/
├── README.md
├── LICENSE
├── CONTRIBUTING.md
├── .gitignore
├── engine/
├── world/
├── simulation/
├── agents/
├── sensors/
├── scenarios/
├── datasets/
├── ai/
├── backend/
├── frontend/
├── tools/
├── tests/
├── docs/
└── .github/
```

## v0.1 milestone

The first milestone is intentionally narrow:

> Build a locally explorable procedural world with terrain, roads, buildings, vehicles, pedestrians, first-person control, physics, and automatic data capture.

This is the foundation for everything else.

## Architecture summary

```text
SYNTHIA
├── World Creation
├── World Runtime
├── Agent Engine
├── Human Interface
├── Synthetic Data Engine
└── AI Laboratory
```

## Initial technical direction

SYNTHIA should be built as a headless-first simulation platform with a high-fidelity visualization layer when hardware permits.

- Core simulation: C++
- Headless execution: native C++ runtime for scenarios, training, evaluation, and replay
- Visual execution: Unreal Engine 5 only for human visualization and high-fidelity inspection
- Agent core: custom C++ agent API with deterministic policy interfaces
- Simulation API: C++ service layer and optional external control interface
- Entity and world model: deterministic, seed-based, testable simulation state
- Storage: structured logs and datasets, with future MinIO/S3 support for larger artifacts
- Database: PostgreSQL for metadata, experiments, worlds, and scenario metadata
- Deployment: Docker first, with orchestration later as scale grows
- AI tooling: keep the simulation core independent from graphics; external ML tooling can be integrated later without coupling the runtime to Python

## Execution model

```text
SYNTHIA
├── HEADLESS MODE
│   └── scenario generation, batch simulation, RL loops, agent training, evaluation
└── VISUAL MODE
    └── Unreal Engine 5 for observation, debugging, and human inspection
```

The AI should not depend on graphics to learn. The system should run large numbers of simulations headlessly and only render a selected run in Unreal when a human needs to inspect it.

## Roadmap

1. Foundation and repository setup
2. Procedural terrain and world generation
3. Roads, buildings, vegetation, and lighting
4. Human controller and exploration
5. Vehicles and pedestrians
6. Weather and time simulation
7. Sensor capture and dataset export
8. Scenario engine and reproducibility
9. AI training and evaluation
10. World Data & Knowledge Engine (v0.5)
11. Autonomous research and reasoning agents (v0.6)
12. Web platform and cloud scale

## Documentation

See the docs directory for the technical design discussion:

- [docs/vision.md](docs/vision.md)
- [docs/architecture.md](docs/architecture.md)
- [docs/world-model.md](docs/world-model.md)
- [docs/agents.md](docs/agents.md)
- [docs/sensors.md](docs/sensors.md)
- [docs/scenarios.md](docs/scenarios.md)
- [docs/roadmap.md](docs/roadmap.md)

## Getting started

This repository currently contains the project foundation and technical specification. Development work should continue by:

1. setting up the engine/toolchain
2. creating the world-generation prototype
3. implementing the first demo world
4. adding agent and sensor subsystems
5. exporting datasets and scenarios

## Status

Project status: foundation and technical specification in place.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
