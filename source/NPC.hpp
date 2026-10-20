#ifndef GAME_NPC_HPP
#define GAME_NPC_HPP

#include <SFML/Graphics.hpp>
#include "Entity.hpp"

/**
 * @file NPC.hpp
 * @brief A non-player entity with a visible sprite.
 *
 * NPC is an Entity plus a sprite for on-screen display. The sprite is a public
 * member: the game loop reads and writes it directly each frame to position and
 * draw the NPC.
 */
struct NPC : Entity {
    sf::Sprite sprite;   ///< Public: positioned and drawn by the game loop.
};

#endif // GAME_NPC_HPP
