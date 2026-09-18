# Project Overview

SYNTHIA is a world-generation and simulation platform for humans and AI agents.

The project is intentionally designed as a layered system, with a world model at the center and a simulation abstraction above it. This allows the platform to evolve from a local procedural demo into a broader synthetic-data and AI-evaluation system.

## Core values

- reproducibility
- simulation truth
- modular architecture
- human and AI coexistence
- synthetic-data generation
- scientific experiment tracking

## User story

A user should be able to:

- create a procedural world from a seed or natural-language prompt
- enter the world in first person
- watch AI agents behave in it
- modify environment and scenario parameters
- export perception data and metadata

## Product framing

The first product is a local world engine. The long-term product becomes a platform for world creation, experiment orchestration, synthetic-data generation, and AI evaluation.

## Minimum viable proof

The minimal proof that the concept works is:

> A human can enter a generated world while AI-controlled actors also act inside it, and the simulation can generate machine-readable observations.
