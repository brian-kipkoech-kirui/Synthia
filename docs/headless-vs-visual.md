# Headless vs Visual Execution

## Design principle

SYNTHIA should support two execution modes from the outset:

- headless mode for speed, scale, training, evaluation, and batch simulations
- visual mode for real-time observation, debugging, and human interaction via Unreal Engine 5

## Why headless must come first

The real product is not a pretty scene. The real product is a simulation environment for learning and evaluation.

Large-scale agent training and failure analysis are primarily batch workloads. They do not require high-end rendering. A headless runtime can:

- evaluate thousands or millions of scenarios
- replay deterministic runs
- record telemetry for analysis
- measure success, failure, and reward patterns
- stress-test agent behavior across seeded worlds

## Headless runtime responsibilities

The headless world engine should maintain:

- world state
- entities and behaviors
- tasks and scenarios
- observations
- actions
- rewards
- events
- experiment metadata
- replay logs

This layer is the system of truth.

## Visual runtime responsibilities

The visual runtime should only provide:

- human-readable rendering
- inspection of selected experiments
- debugging of world state
- demonstration and narrative playback

It should not define the rules of the simulation.

## Example flow

```text
100,000 scenarios
    -> headless SYNTHIA
        -> agent succeeds / fails / crashes / adapts
    -> selected run exported to Unreal
    -> human reviews the event in high fidelity
```

This is the correct product architecture for the original SYNTHIA concept.

## Recommended final direction

Use C++ as the core engine foundation, and treat Unreal as a visualization and exceptional debugging layer rather than the primary execution platform.
