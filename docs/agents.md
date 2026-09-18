# Agent System

## Purpose

Agents represent controllable people, vehicles, robots, and AI-controlled entities inside the world.

An agent should have:

- identity
- goals
- memory
- perception
- decision-making
- actions
- relationships

## Agent model

```json
{
  "id": "person_481",
  "age": 31,
  "occupation": "shopkeeper",
  "goal": "open_shop",
  "location": "building_31",
  "memory": [],
  "schedule": [],
  "relationships": []
}
```

## Architecture

```text
Agent
├── Perception
├── Memory
├── Goals
├── Decision
├── Action
└── World Interaction
```

## Initial approach

Do not begin with LLM-powered agents for everything.

Start with:

- finite-state machines
- behavior trees
- pathfinding
- local obstacle avoidance
- utility-based decision logic

Then add LLMs for higher-level behavior when needed.

## Example pedestrian flow

```text
Home
-> walk to road
-> avoid vehicles
-> cross road
-> enter bus
-> travel
-> exit
-> walk to office
```

## Emergent behavior

The long-term goal is not to script every outcome. Instead, the system should allow complex patterns to emerge from:

- many agents
- road rules
- daily schedules
- local negotiation
- dynamic weather
- infrastructure constraints

## v0.1 scope

The first agent milestone is a basic pedestrian navigation system and a basic vehicle navigation system.

That gives the project a living world without requiring full human-like reasoning yet.
