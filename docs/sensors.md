# Synthetic Sensors

## Goal

Every simulated actor should be able to generate sensor data that can be used for training and evaluation.

## Sensor types

### Camera

- RGB
- depth
- segmentation
- object IDs

### LiDAR

- point clouds
- range and distance
- object association

### Radar

- velocity
- distance
- reflection

### IMU

- acceleration
- rotation
- angular velocity

### GPS

- latitude
- longitude
- altitude

## Data generation

The whole point is grounding perception in the underlying world state.

For every image or sensor sample, the system should preserve:

- ground truth labels
- object positions
- classes
- bounding boxes
- segmentation
- depth
- trajectories

## v0.1 dataset targets

The first dataset export should include:

- RGB images
- depth maps
- segmentation masks
- object IDs
- metadata and manifest

This is enough to prove the synthetic-data pipeline.
