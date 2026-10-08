#ifndef GAME_DUNGEON2D_HPP
#define GAME_DUNGEON2D_HPP

#include <memory>
#include <vector>

/**
 * @file Dungeon2D.hpp
 * @brief Binary Space Partitioning generator that produces explicit dungeon
 *        geometry: a tree of rooms and a list of corridors.
 *
 * Dungeon2D holds information about spatial structure. generate() recursively
 * partitions the area into a binary tree; each leaf holds one room; corridors
 * join the regions. Flattening this geometry into a tile matrix is TileMap's
 * job (TileMap::loadFromDungeon reads the tree and the corridors and fills its
 * cells). All data are public, so no accessor methods are needed.
 *
 * All coordinates are in TILE units (integer cell indices), not pixels.
 */

/**
 * @brief An axis-aligned rectangle in tile coordinates.
 *
 * The rectangle is given by two opposite corners, (left, top) and
 * (right, bottom), with left <= right and top <= bottom. For an axis-aligned
 * rectangle these two corners fully determine all FOUR corners:
 *   top-left     = (left,  top)      top-right    = (right, top)
 *   bottom-left  = (left,  bottom)   bottom-right = (right, bottom).
 */
struct Rect {
    int left{0};
    int top{0};
    int right{0};
    int bottom{0};
};

/// A corridor: a long, narrow axis-aligned rectangle connecting two regions.
using Corridor = Rect;

/**
 * @brief A node of the BSP tree.
 *
 * - Internal node: covers a region and has two children (left/right); its
 *   room is unused (its area is zero / ignored).
 * - Leaf node: has no children (both null) and holds one room inside its region.
 *
 * The tree owns its children (unique_ptr), so destroying the root frees all of it.
 */
struct BSPNode {
    Rect region;                      ///< The area of space this node covers (tiles).
    Rect room;                        ///< The room carved in this node, if it is a leaf.
    std::unique_ptr<BSPNode> left;    ///< First child, or null if this is a leaf.
    std::unique_ptr<BSPNode> right;   ///< Second child, or null if this is a leaf.

    /// True when this node has no children (and therefore holds a room).
    [[nodiscard]] bool isLeaf() const noexcept {
        return !left && !right;
    }
};

/**
 * @brief Generates and stores BSP dungeon geometry.
 *
 * Constraints the generated geometry satisfies:
 *   - Corridors are 3 tiles wide.
 *   - The smallest room is 5 x 5 tiles.
 *   - CONNECTIVITY: the rooms and corridors together form a single connected
 *     space (every room is reachable from every other through corridors).
 */
class Dungeon2D {
public:
    // --- Public data (the generated structure) ----------------------------
    int   width{0};    ///< Total width of the dungeon, in tiles.
    int   height{0};   ///< Total height of the dungeon, in tiles.

    std::unique_ptr<BSPNode> root;         ///< Root of the BSP tree; null before generate().
    std::vector<Corridor>    corridors;    ///< All corridors joining the regions.

    /**
     * @param widthTiles  Width of the dungeon in tiles (> 0).
     * @param heightTiles Height of the dungeon in tiles (> 0).
     */
    Dungeon2D(int widthTiles, int heightTiles);

    /**
     * @brief Builds the BSP tree, carves the rooms and lays the corridors.
     * @param seed Seed for the pseudo-random generator; the same seed yields
     *             the same dungeon.
     * @post root points to the tree, corridors holds the connections, and the
     *       geometry satisfies the constraints above.
     */
    void generate(unsigned seed);
};

#endif // GAME_DUNGEON2D_HPP
