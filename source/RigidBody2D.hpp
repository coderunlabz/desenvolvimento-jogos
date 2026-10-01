#ifndef GAME_RIGIDBODY2D_HPP
#define GAME_RIGIDBODY2D_HPP

#include "Vector2D.hpp"

/**
 * @file RigidBody2D.hpp
 * @brief Point-mass motion state and its time integration.
 *
 * Holds position, velocity and acceleration in continuous world space and
 * advances them with SEMI-IMPLICIT (symplectic) EULER, which is stable for
 * interactive simulation:
 *
 *      velocity += acceleration * dt;
 *      position += velocity     * dt;   // uses the just-updated velocity
 *
 * The simulation uses a fixed timestep, so integrate() is called with the same
 * dt throughout a run, keeping motion reproducible.
 */
struct RigidBody2D {
    Vector2D position;      ///< World position, in pixels.
    Vector2D velocity;      ///< Pixels per second.
    Vector2D acceleration;  ///< Pixels per second squared.

    /**
     * @brief Advances the body by one fixed step (semi-implicit Euler).
     * @param dt Timestep in seconds.
     * @pre dt > 0.
     */
    void integrate(float dt) noexcept;
};

#endif // GAME_RIGIDBODY2D_HPP
