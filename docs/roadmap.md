# SYNTHIA Roadmap

## Milestone 0: Foundation

Establish the repository and architecture.

Goals:

- set up repo structure
- define the vision and technical principles
- specify the v0.1 target
- document the world and simulation model

## Milestone 1: Unreal world

Build the local runtime foundation.

- create a basic Unreal project
- configure a playable environment
- add terrain and lighting

## Milestone 2: Procedural terrain

Generate terrain and environment features algorithmically.

- terrain heightmap
- ground materials
- basic vegetation
- environment scale and boundaries

## Milestone 3: Procedural roads

Build the road network.

- roads and intersections
- lane layout
- driveable surfaces
- collision boundaries

## Milestone 4: Procedural buildings

Add buildings and urban structures.

- building footprints
- procedural density
- rooftops and entrances
- city layout

## Milestone 5: Human controller

Add a first-person character.

- move and turn
- jump and crouch
- interact with objects
- explore a world

## Milestone 6: Vehicles

Add basic autonomous and player vehicles.

- vehicle movement
- steering
- collision handling
- stopping and following rules

## Milestone 7: Pedestrians

Add moving agents.

- simple pathfinding
- obstacle avoidance
- navigation toward goals

## Milestone 8: Weather and time

Add environmental dynamics.

- day/night cycle
- rain
- fog
- wind and lighting changes

## Milestone 9: Sensors

Add synthetic perception systems.

- camera capture
- depth capture
- segmentation
- object IDs

## Milestone 10: Dataset exporter

Export data in a machine-readable structure.

- images
- masks
- metadata
- manifest
- annotations

## Milestone 11: Scenario engine

Support multiple simulation runs from one world definition.

- variable conditions
- reproducible seeds
- experiment configuration
- metrics aggregation

## Milestone 12: AI agents

Add more advanced agent logic.

- utility-based behaviors
- path planning
- memory and goals
- hierarchical decision processes

## Milestone 13: Batch simulation

Support repeated execution.

- large scenario batches
- randomized generation
- event logging
- internal result comparison

## Milestone 14: ML training

Enable learning loops.

- synthetic-data training
- evaluation metrics
- automatic failure discovery
- retraining pipelines

## Milestone 15: World Data & Knowledge Engine (v0.5)

Turn heterogeneous observations into a structured world model.

- ingest internet, market, climate, and public data streams
- extract entities, events, and relationships
- preserve provenance and confidence
- maintain world-state snapshots over time
- generate hypotheses from correlated signals
- support replay, counterfactuals, and simulation over indexed information

## Milestone 16: Autonomous Research / Reasoning Agents (v0.6)

Build agents that continuously perceive, search, correlate, hypothesize, simulate, and decide.

- observe and search across sources
- index and structure facts
- generate candidate explanations
- simulate alternative futures
- learn from outcomes and update memory
- make decisions beyond explicitly programmed rules

## Milestone 17: Web platform

Provide user tooling.

- world management
- run controls
- dashboards
- dataset browsing

## Milestone 16: Cloud simulation

Scale the architecture and infrastructure later.

- distributed simulation runs
- cloud storage
- orchestration and experiments
- enterprise deployment
