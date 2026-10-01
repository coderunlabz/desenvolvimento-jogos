#include "Collision2D.hpp"

bool AABB::intersects(const AABB& other) const noexcept {
    // Touching edges count as overlapping: only strict separation rejects.
    return max.x >= other.min.x && other.max.x >= min.x &&
           max.y >= other.min.y && other.max.y >= min.y;
}

AABB Collision2D::bounds(const Vector2D& position) const noexcept {
    return AABB{position - halfExtents, position + halfExtents};
}
