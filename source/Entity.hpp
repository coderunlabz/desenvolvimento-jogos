#ifndef GAME_ENTITY_HPP
#define GAME_ENTITY_HPP

#include "Global.hpp"
#include "Vector2D.hpp"
#include "RigidBody2D.hpp"
#include "Collision2D.hpp"

/**
 * @file Entity.hpp
 * @brief A world object: a rigid body together with a collision shape.
 *
 * Entity composes one RigidBody2D and one Collision2D, both public, so callers
 * read and write motion and shape directly. It also carries an active flag and
 * a numeric identifier.
 */
struct Entity {
    RigidBody2D body;        ///< Motion state (position, velocity, acceleration).
    Collision2D collision;   ///< Collision shape (half-extents).

    /// While false, the entity is ignored by simulation and collision passes.
    bool active{true};

    /// Numeric identifier. Zero means "not yet assigned".
    EntityId id{0};

    virtual ~Entity() = default;

    /// Convenience: current world-space bounding box (position + shape).
    [[nodiscard]] AABB bounds() const noexcept {
        return collision.bounds(body.position);
    }
};

#endif // GAME_ENTITY_HPP
