import unittest

from synthia.world import World, WorldConfig


class WorldTests(unittest.TestCase):
    def test_world_seed_is_reproducible(self):
        config_a = WorldConfig(name="demo", seed=1234, climate="tropical")
        config_b = WorldConfig(name="demo", seed=1234, climate="tropical")

        world_a = World(config_a)
        world_b = World(config_b)

        self.assertEqual(world_a.seed, world_b.seed)
        self.assertEqual(world_a.world_id, world_b.world_id)
        self.assertEqual(world_a.state["time"], world_b.state["time"])

    def test_world_generates_basic_entities(self):
        world = World(WorldConfig(name="demo", seed=7, climate="urban"))

        self.assertIn("entities", world.state)
        self.assertIsInstance(world.state["entities"], list)
        self.assertGreaterEqual(len(world.state["entities"]), 1)


if __name__ == "__main__":
    unittest.main()
