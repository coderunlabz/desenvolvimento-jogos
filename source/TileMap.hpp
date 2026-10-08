#ifndef GAME_TILEMAP_HPP
#define GAME_TILEMAP_HPP

#include <string>
#include <vector>
#include "Global.hpp"
#include "Vector2D.hpp"

class Dungeon2D;

/**
 * @file TileMap.hpp
 * @brief The tile matrix: storage, CSV persistence and coordinate translation.
 *
 * The map is one integer per cell (EMPTY_TILE / WALL_TILE), stored ROW-MAJOR
 * (index = row * width + col), following the representation conventions of
 * Global.hpp. It also converts between continuous world coordinates and
 * discrete cell indices.
 *
 * The matrix is produced by RASTERIZING a Dungeon2D: loadFromDungeon walks the
 * dungeon's BSP tree and corridor list and writes the corresponding cells.
 */
class TileMap {
public:
    TileMap() = default;

    /**
     * @brief Builds the matrix from a dungeon's geometry.
     * @details Sizes the matrix to the dungeon (width x height tiles), fills it
     *          with WALL_TILE, then carves EMPTY_TILE cells for every room in
     *          the BSP tree's leaves and for every corridor in the dungeon's
     *          corridor list.
     */
    void loadFromDungeon(const Dungeon2D& dungeon);

    // --- CSV persistence ---------------------------------------------------
    /**
     * @brief Saves the matrix as CSV: one row per line, integers separated by
     *        single commas, NO trailing comma, row-major (row 0 first).
     * @return true on success.
     */
    [[nodiscard]] bool saveCSV(const std::string& path) const;

    /**
     * @brief Loads a matrix written in the exact saveCSV() format.
     * @return true on success; on failure the map is left unchanged.
     */
    [[nodiscard]] bool loadCSV(const std::string& path);

    // --- Queries -----------------------------------------------------------
    [[nodiscard]] int  width() const noexcept;
    [[nodiscard]] int  height() const noexcept;

    /// Tile value at a cell. @pre in-range indices.
    [[nodiscard]] int  at(int col, int row) const;

    /// True if the cell holds WALL_TILE. @pre in-range indices.
    [[nodiscard]] bool isWall(int col, int row) const;

    /// Row-major pointer to the raw matrix (size width*height).
    [[nodiscard]] const int* data() const noexcept;

    // --- Continuous <-> discrete translation -------------------------------
    /// Cell containing a world position: col=floor(x/TILE_SIZE), row=floor(y/TILE_SIZE).
    void toIndices(const Vector2D& world, int& col, int& row) const noexcept;

    /// World coordinates of the CENTRE of a cell.
    [[nodiscard]] Vector2D toWorld(int col, int row) const noexcept;

    /**
     * @brief Returns the indices of a randomly chosen EMPTY_TILE cell.
     * @param[out] col Column of the chosen empty cell.
     * @param[out] row Row of the chosen empty cell.
     * @pre At least one EMPTY_TILE cell exists.
     */
    void randomEmptyCell(int& col, int& row) const;

private:
    int              m_width{0};
    int              m_height{0};
    std::vector<int> m_cells;   ///< Row-major, size m_width * m_height.
};

#endif // GAME_TILEMAP_HPP
