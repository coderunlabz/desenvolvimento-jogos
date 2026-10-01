#ifndef GAME_GLOBAL_HPP
#define GAME_GLOBAL_HPP

/**
 * @file Global.hpp
 * @brief Project-wide identifiers, constants and representation conventions.
 *
 * This file will be the single source of reference for the project;
 *   no other file should redefine values listed here; 
 *   all files must always refer to symbols defined here.
 *
 * ---------------------------------------------------------------------------
 * COORDINATE CONVENTIONS (for the whole project)
 * ---------------------------------------------------------------------------
 *  - World, physical, coordinates are continuous, measured in PIXELS, stored
 *    as float. The Y axis grows DOWNWARD (screen convention).
 *  - The tile grid is discrete. The cell containing a world position is
 *        col = floor(world.x / TILE_SIZE),  row = floor(world.y / TILE_SIZE).
 *    The world position is the reference; a cell index is always derived from
 *    it and never stored as an independent second copy of a position.
 *  - The centre of tile (col,row) in world coordinates is
 *        ( (col + 0.5) * TILE_SIZE , (row + 0.5) * TILE_SIZE ).
 *
 * ---------------------------------------------------------------------------
 * TILE MATRIX REPRESENTATION (used by TileMap and its CSV files)
 * ---------------------------------------------------------------------------
 *  - One integer per cell. EMPTY_TILE marks a free cell, WALL_TILE a solid one.
 */

/// Integer type used for every entity identifier.
using EntityId = int;

// --- Reserved entity identifiers ------------------------------------------
/// Identifier reserved for the single player entity. It is below the
/// auto-generated range, so it can never collide with a generated id.
constexpr EntityId PLAYER_ID = 100;
/// First identifier handed out to automatically numbered entities.
constexpr EntityId INITIAL_ENTITY_ID = 1000;

// --- Tile matrix legend ----------------------------------------------------
constexpr int EMPTY_TILE = 0;   ///< A free, walkable cell.
constexpr int WALL_TILE  = 1;   ///< A solid cell (blocks movement).

// --- Spatial constants -----------------------------------------------------
/// Common default size, in pixels, of a tile, a sprite and an NPC: they are
/// all the same, 64 x 64.
constexpr float TILE_SIZE = 64.0f;

// --- Numeric tolerance -----------------------------------------------------
/// Tolerance used in floating-point comparisons.
constexpr float EPSILON = 0.0001f;

#endif // GAME_GLOBAL_HPP
