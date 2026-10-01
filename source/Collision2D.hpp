#ifndef GAME_COLLISION2D_HPP
#define GAME_COLLISION2D_HPP

#include "Global.hpp"
#include "Vector2D.hpp"

/**
 * @file Collision2D.hpp
 * @brief Axis-aligned bounding box (AABB) collision shape and overlap test.
 *
 * This is narrow-phase only: given two boxes, do they overlap? The default
 * shape is a square of side TILE_SIZE (64 x 64 pixels).
 */

/**
 * @brief An axis-aligned box given by two opposite corners.
 * @invariant min.x <= max.x and min.y <= max.y.
 */
struct AABB {
    Vector2D min;   ///< Smaller x, smaller y (top-left; y grows downward).
    Vector2D max;   ///< Larger x, larger y (bottom-right).

    /// Overlap test; touching edges count as overlapping.
    [[nodiscard]] bool intersects(const AABB& other) const noexcept;
};

/**
 * @brief The collision shape carried by an entity: a rectangle centred on the
 *        entity's world position, stored as half-extents.
 */
struct Collision2D {
    /// Half width and half height, in pixels. Default: a 64 x 64 square.
    Vector2D halfExtents{ TILE_SIZE * 0.5f, TILE_SIZE * 0.5f };

    /**
     * @brief World-space AABB centred on @p position.
     * @return { position - halfExtents , position + halfExtents }.
     */
    [[nodiscard]] AABB bounds(const Vector2D& position) const noexcept;
};

#endif // GAME_COLLISION2D_HPP
