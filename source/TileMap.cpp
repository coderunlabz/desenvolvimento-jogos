#include "TileMap.hpp"

#include "Dungeon2D.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>
#include <utility>

namespace {

void carveRect(std::vector<int>& cells, int width, int height, const Rect& r) {
    const int left = std::max(0, r.left);
    const int top = std::max(0, r.top);
    const int right = std::min(width, r.right);
    const int bottom = std::min(height, r.bottom);
    for (int row = top; row < bottom; ++row) {
        for (int col = left; col < right; ++col) {
            cells[static_cast<std::size_t>(row * width + col)] = EMPTY_TILE;
        }
    }
}

void carveLeaves(std::vector<int>& cells, int width, int height, const BSPNode* node) {
    if (node == nullptr) {
        return;
    }
    if (node->isLeaf()) {
        carveRect(cells, width, height, node->room);
        return;
    }
    carveLeaves(cells, width, height, node->left.get());
    carveLeaves(cells, width, height, node->right.get());
}

bool parseCSV(const std::string& text, int& outW, int& outH, std::vector<int>& outCells) {
    std::vector<std::vector<int>> rows;
    std::string line;
    std::istringstream in(text);

    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            return false;
        }
        if (line.back() == ',') {
            return false;
        }

        std::vector<int> row;
        std::size_t start = 0;
        while (start <= line.size()) {
            const std::size_t comma = line.find(',', start);
            const std::size_t end = (comma == std::string::npos) ? line.size() : comma;
            const std::string token = line.substr(start, end - start);
            if (token.empty()) {
                return false;
            }
            try {
                std::size_t consumed = 0;
                const int value = std::stoi(token, &consumed);
                if (consumed != token.size()) {
                    return false;
                }
                row.push_back(value);
            } catch (...) {
                return false;
            }
            if (comma == std::string::npos) {
                break;
            }
            start = comma + 1;
            if (start == line.size()) {
                // trailing comma already rejected by line.back() check, but keep safe
                return false;
            }
        }
        if (row.empty()) {
            return false;
        }
        if (!rows.empty() && row.size() != rows.front().size()) {
            return false;
        }
        rows.push_back(std::move(row));
    }

    if (rows.empty()) {
        return false;
    }

    outH = static_cast<int>(rows.size());
    outW = static_cast<int>(rows.front().size());
    outCells.clear();
    outCells.reserve(static_cast<std::size_t>(outW * outH));
    for (const auto& row : rows) {
        outCells.insert(outCells.end(), row.begin(), row.end());
    }
    return true;
}

} // namespace

void TileMap::loadFromDungeon(const Dungeon2D& dungeon) {
    m_width = dungeon.width;
    m_height = dungeon.height;
    m_cells.assign(static_cast<std::size_t>(m_width * m_height), WALL_TILE);
    carveLeaves(m_cells, m_width, m_height, dungeon.root.get());
    for (const Corridor& corridor : dungeon.corridors) {
        carveRect(m_cells, m_width, m_height, corridor);
    }
}

bool TileMap::saveCSV(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    for (int row = 0; row < m_height; ++row) {
        for (int col = 0; col < m_width; ++col) {
            if (col > 0) {
                out << ',';
            }
            out << m_cells[static_cast<std::size_t>(row * m_width + col)];
        }
        out << '\n';
    }
    return static_cast<bool>(out);
}

bool TileMap::loadCSV(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return false;
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    if (!in && !in.eof()) {
        return false;
    }

    int newW = 0;
    int newH = 0;
    std::vector<int> newCells;
    if (!parseCSV(buffer.str(), newW, newH, newCells)) {
        return false;
    }

    m_width = newW;
    m_height = newH;
    m_cells = std::move(newCells);
    return true;
}

int TileMap::width() const noexcept {
    return m_width;
}

int TileMap::height() const noexcept {
    return m_height;
}

int TileMap::at(int col, int row) const {
    assert(col >= 0 && col < m_width);
    assert(row >= 0 && row < m_height);
    return m_cells[static_cast<std::size_t>(row * m_width + col)];
}

bool TileMap::isWall(int col, int row) const {
    return at(col, row) == WALL_TILE;
}

const int* TileMap::data() const noexcept {
    return m_cells.empty() ? nullptr : m_cells.data();
}

void TileMap::toIndices(const Vector2D& world, int& col, int& row) const noexcept {
    col = static_cast<int>(std::floor(world.x / TILE_SIZE));
    row = static_cast<int>(std::floor(world.y / TILE_SIZE));
}

Vector2D TileMap::toWorld(int col, int row) const noexcept {
    return Vector2D{(static_cast<float>(col) + 0.5f) * TILE_SIZE,
                    (static_cast<float>(row) + 0.5f) * TILE_SIZE};
}

void TileMap::randomEmptyCell(int& col, int& row) const {
    std::vector<std::pair<int, int>> empties;
    empties.reserve(m_cells.size());
    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            if (m_cells[static_cast<std::size_t>(r * m_width + c)] == EMPTY_TILE) {
                empties.emplace_back(c, r);
            }
        }
    }
    assert(!empties.empty());
    thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> pick(0, empties.size() - 1);
    const auto chosen = empties[pick(rng)];
    col = chosen.first;
    row = chosen.second;
}
