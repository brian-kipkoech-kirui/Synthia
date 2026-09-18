# Agent Interface

## Core contract

The SYNTHIA agent interface is intentionally narrow and general.

```cpp
class Agent {
public:
    virtual Observation observe(const World& world) = 0;
    virtual Action decide(const Observation& observation) = 0;
    virtual ~Agent() = default;
};
```

This contract protects the simulation engine from any specific agent implementation.

## Why this matters

The world does not need to know whether the agent is:

- a rule-based controller
- a policy network
- a reinforcement-learning model
- a future LLM-driven planner

The core engine only receives an observation and emits an action.

## Separation of concerns

```text
Agent Interface
    -> observation
    -> decision
    -> action
    -> reward semantics

Simulation Engine
    -> world update
    -> rules enforcement
    -> task evaluation
    -> metadata and replay
```

This keeps the system modular and testable.

## v0.1 test agent

The first agent in the repo is a deterministic test agent that uses the same contract as any future model:

- observe world state
- decide a valid action
- submit the action through the world engine
- collect simulation output

This is the correct conceptual role for the test agent: it is not the product, only the validation harness.
