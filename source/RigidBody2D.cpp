#include "RigidBody2D.hpp"

#include <cassert>

void RigidBody2D::integrate(float dt) noexcept {
    assert(dt > 0.0f);
    velocity += acceleration * dt;
    position += velocity * dt;
}
