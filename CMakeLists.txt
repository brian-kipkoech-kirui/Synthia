cmake_minimum_required(VERSION 3.16)
project(Synthia LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(synthia
    src/World.cpp
    src/ExperimentRecord.cpp
    src/ScenarioRunner.cpp
)

target_include_directories(synthia
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(synthia_headless
    apps/synthia_headless.cpp
)
target_link_libraries(synthia_headless PRIVATE synthia)

enable_testing()

add_executable(test_world_core tests/test_world_core.cpp)
target_link_libraries(test_world_core PRIVATE synthia)
add_test(NAME test_world_core COMMAND test_world_core)

add_executable(test_task_lifecycle tests/test_task_lifecycle.cpp)
target_link_libraries(test_task_lifecycle PRIVATE synthia)
add_test(NAME test_task_lifecycle COMMAND test_task_lifecycle)

add_executable(test_episode_replay tests/test_episode_replay.cpp)
target_link_libraries(test_episode_replay PRIVATE synthia)
add_test(NAME test_episode_replay COMMAND test_episode_replay)

add_executable(test_scenario_runner tests/test_scenario_runner.cpp)
target_link_libraries(test_scenario_runner PRIVATE synthia)
add_test(NAME test_scenario_runner COMMAND test_scenario_runner)

add_executable(test_determinism tests/test_determinism.cpp)
target_link_libraries(test_determinism PRIVATE synthia)
add_test(NAME test_determinism COMMAND test_determinism)

add_executable(test_multi_agent_world tests/test_multi_agent_world.cpp)
target_link_libraries(test_multi_agent_world PRIVATE synthia)
add_test(NAME test_multi_agent_world COMMAND test_multi_agent_world)

add_executable(test_world_model tests/test_world_model.cpp)
target_link_libraries(test_world_model PRIVATE synthia)
add_test(NAME test_world_model COMMAND test_world_model)

add_executable(test_physics_engine tests/test_physics_engine.cpp)
target_link_libraries(test_physics_engine PRIVATE synthia)
add_test(NAME test_physics_engine COMMAND test_physics_engine)
