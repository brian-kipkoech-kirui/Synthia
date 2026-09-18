#pragma once

#include <string>

namespace synthia {

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

inline bool operator==(const Vec3& lhs, const Vec3& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

struct Entity {
    std::string id;
    std::string type;
    Vec3 position;
    Vec3 velocity;
    double heading = 0.0;
    bool active = true;
    std::string metadata;
    double mass = 1.0;
    double energy = 100.0;
    double max_energy = 100.0;
    bool blocking = false;
};

}  // namespace synthia
