# Scenario Engine

## Purpose

A scenario defines a starting state plus a set of constraints, events, and objectives.

```yaml
scenario:
  name: pedestrian_crossing

initial:
  weather: rain
  time: night
  traffic: high

event:
  delay: 4.5
  type: pedestrian_crossing

objective:
  vehicle: avoid_collision

terminate:
  after: 30s
```

## Why it matters

Scenarios let SYNTHIA:

- test edge cases
- run large numbers of variants
- generate synthetic datasets at scale
- evaluate model robustness
- discover failure modes systematically

## Variation model

A single base scenario can be randomized across dimensions such as:

- speed
- rain intensity
- visibility
- distance
- lighting
- number of pedestrians
- number of vehicles

## Reproducibility

Every scenario run should keep:

- seed
- world version
- scenario version
- model version
- hyperparameters
- metrics

## v0.1 scope

The first scenario engine should support:

- spawn objects
- vary weather
- vary traffic density
- vary pedestrian density
- batch execution of a small number of runs

This will be the basis for broader synthetic-data generation later.
