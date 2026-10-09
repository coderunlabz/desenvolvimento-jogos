#include "../source/Dungeon2D.hpp"
#include "../source/TileMap.hpp"
#include "../source/Vector2D.hpp"
#include "../source/Transform2D.hpp"
#include "../source/RigidBody2D.hpp"
#include "../source/Collision2D.hpp"
#include "../source/Global.hpp"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

static int failures = 0;

static void expect(bool cond, const char* name) {
    if (!cond) {
        std::cerr << "FAIL " << name << "\n";
        ++failures;
    }
}

static int rectW(const Rect& r) { return r.right - r.left; }
static int rectH(const Rect& r) { return r.bottom - r.top; }

static void walkTree(const BSPNode* node, int& leaves, int& internals, bool& ok) {
    if (!node) {
        ok = false;
        return;
    }
    if (node->isLeaf()) {
        ++leaves;
        expect(!node->left && !node->right, "leaf has no children");
        expect(rectW(node->room) >= 5 && rectH(node->room) >= 5, "room >= 5x5");
        expect(node->room.left >= node->region.left &&
                   node->room.right <= node->region.right &&
                   node->room.top >= node->region.top &&
                   node->room.bottom <= node->region.bottom,
               "room inside region");
        return;
    }
    ++internals;
    expect(node->left && node->right, "internal has two children");
    walkTree(node->left.get(), leaves, internals, ok);
    walkTree(node->right.get(), leaves, internals, ok);
}

static bool geometryEqual(const Dungeon2D& a, const Dungeon2D& b) {
    if (a.width != b.width || a.height != b.height) return false;
    if (a.corridors.size() != b.corridors.size()) return false;
    for (std::size_t i = 0; i < a.corridors.size(); ++i) {
        const auto& c1 = a.corridors[i];
        const auto& c2 = b.corridors[i];
        if (c1.left != c2.left || c1.top != c2.top || c1.right != c2.right ||
            c1.bottom != c2.bottom) {
            return false;
        }
    }
    // Compare leaf rooms by rasterization fingerprint.
    TileMap ma, mb;
    ma.loadFromDungeon(a);
    mb.loadFromDungeon(b);
    if (ma.width() != mb.width() || ma.height() != mb.height()) return false;
    for (int i = 0; i < ma.width() * ma.height(); ++i) {
        if (ma.data()[i] != mb.data()[i]) return false;
    }
    return true;
}

static int countEmpty(const TileMap& map) {
    int n = 0;
    for (int r = 0; r < map.height(); ++r) {
        for (int c = 0; c < map.width(); ++c) {
            if (map.at(c, r) == EMPTY_TILE) ++n;
        }
    }
    return n;
}

static int bfsEmpty(const TileMap& map) {
    const int w = map.width();
    const int h = map.height();
    int startC = -1, startR = -1;
    for (int r = 0; r < h && startC < 0; ++r) {
        for (int c = 0; c < w; ++c) {
            if (map.at(c, r) == EMPTY_TILE) {
                startC = c;
                startR = r;
                break;
            }
        }
    }
    if (startC < 0) return 0;

    std::vector<char> seen(static_cast<std::size_t>(w * h), 0);
    std::queue<std::pair<int, int>> q;
    q.push({startC, startR});
    seen[static_cast<std::size_t>(startR * w + startC)] = 1;
    int visited = 0;
    const int dc[4] = {1, -1, 0, 0};
    const int dr[4] = {0, 0, 1, -1};
    while (!q.empty()) {
        const int c = q.front().first;
        const int r = q.front().second;
        q.pop();
        ++visited;
        for (int i = 0; i < 4; ++i) {
            const int nc = c + dc[i];
            const int nr = r + dr[i];
            if (nc < 0 || nr < 0 || nc >= w || nr >= h) continue;
            const std::size_t idx = static_cast<std::size_t>(nr * w + nc);
            if (seen[idx] || map.at(nc, nr) != EMPTY_TILE) continue;
            seen[idx] = 1;
            q.push(std::make_pair(nc, nr));
        }
    }
    return visited;
}

static void checkCorridors(const Dungeon2D& d) {
    for (const auto& c : d.corridors) {
        const int w = rectW(c);
        const int h = rectH(c);
        expect(w == 3 || h == 3, "corridor thickness 3 on one axis");
        expect(c.left >= 0 && c.top >= 0 && c.right <= d.width && c.bottom <= d.height,
               "corridor inside map");
    }
}

static void testCtorRejects() {
    bool threw = false;
    try {
        Dungeon2D d(0, 10);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    expect(threw, "reject 0x10");

    threw = false;
    try {
        Dungeon2D d(4, 10);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    expect(threw, "reject 4x10");

    threw = false;
    try {
        Dungeon2D d(10, 4);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    expect(threw, "reject 10x4");

    threw = false;
    try {
        Dungeon2D d(-1, 10);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    expect(threw, "reject negative");
}

static void testGenerate(int w, int h, unsigned seed, const char* label) {
    Dungeon2D d(w, h);
    d.generate(seed);
    expect(d.root != nullptr, label);
    int leaves = 0, internals = 0;
    bool ok = true;
    walkTree(d.root.get(), leaves, internals, ok);
    expect(leaves >= 1, label);
    checkCorridors(d);

    TileMap map;
    map.loadFromDungeon(d);
    expect(map.width() == w && map.height() == h, label);
    const int empty = countEmpty(map);
    expect(empty > 0, label);
    expect(bfsEmpty(map) == empty, label);

    Dungeon2D d2(w, h);
    d2.generate(seed);
    expect(geometryEqual(d, d2), label);

    const std::size_t corridorCount = d.corridors.size();
    d.generate(seed);
    expect(d.corridors.size() == corridorCount, label);
    expect(geometryEqual(d, d2), label);
}

static void testManualRaster() {
    Dungeon2D d(10, 10);
    d.root = std::make_unique<BSPNode>();
    d.root->region = Rect{0, 0, 10, 10};
    d.root->room = Rect{1, 1, 6, 6}; // 5x5
    d.corridors.push_back(Rect{5, 3, 9, 6}); // width 4, height 3 horizontal-ish

    TileMap map;
    map.loadFromDungeon(d);
    expect(map.at(1, 1) == EMPTY_TILE, "room cell empty");
    expect(map.at(5, 5) == EMPTY_TILE, "room corner empty");
    expect(map.at(0, 0) == WALL_TILE, "outside wall");
    expect(map.at(6, 4) == EMPTY_TILE, "corridor empty");
    expect(map.at(9, 9) == WALL_TILE, "far wall");
}

static void testCSV() {
    Dungeon2D dungeon(20, 15);
    dungeon.generate(42);
    TileMap map;
    map.loadFromDungeon(dungeon);

    const std::string path = "tests/_map.csv";
    expect(map.saveCSV(path), "saveCSV ok");

    TileMap loaded;
    expect(loaded.loadCSV(path), "loadCSV ok");
    expect(loaded.width() == map.width() && loaded.height() == map.height(), "csv size");
    bool cellsMatch = true;
    for (int r = 0; r < map.height(); ++r) {
        for (int c = 0; c < map.width(); ++c) {
            if (loaded.at(c, r) != map.at(c, r)) {
                cellsMatch = false;
            }
        }
    }
    expect(cellsMatch, "csv cell match");

    std::ifstream in(path);
    std::string line;
    std::getline(in, line);
    expect(!line.empty() && line.back() != ',', "no trailing comma");

    Dungeon2D keepDungeon(8, 8);
    keepDungeon.generate(1);
    TileMap keep;
    keep.loadFromDungeon(keepDungeon);
    const int oldW = keep.width();
    const int oldH = keep.height();
    const int old0 = keep.at(0, 0);
    expect(!keep.loadCSV("tests/_missing.csv"), "missing file");
    expect(keep.width() == oldW && keep.height() == oldH && keep.at(0, 0) == old0,
           "map intact after fail");

    const std::string bad = "tests/_bad.csv";
    {
        std::ofstream out(bad);
        out << "1,2,\n";
    }
    expect(!keep.loadCSV(bad), "trailing comma rejected");
    expect(keep.width() == oldW && keep.at(0, 0) == old0, "intact after bad csv");

    {
        std::ofstream out(bad);
        out << "1,2\n3\n";
    }
    expect(!keep.loadCSV(bad), "ragged rows rejected");

    {
        std::ofstream out(bad);
        out << "1,x\n";
    }
    expect(!keep.loadCSV(bad), "bad token rejected");

    {
        std::ofstream out(bad);
    }
    expect(!keep.loadCSV(bad), "empty rejected");

    expect(!map.saveCSV("tests/_no_such_dir/out.csv"), "save bad path");
}

static void testCoords() {
    TileMap map;
    int c = 0, r = 0;
    map.toIndices(Vector2D{0.0f, 0.0f}, c, r);
    expect(c == 0 && r == 0, "world 0,0");
    map.toIndices(Vector2D{63.99f, 63.99f}, c, r);
    expect(c == 0 && r == 0, "world 63.99");
    map.toIndices(Vector2D{64.0f, 64.0f}, c, r);
    expect(c == 1 && r == 1, "world 64");
    map.toIndices(Vector2D{-0.1f, -0.1f}, c, r);
    expect(c == -1 && r == -1, "world -0.1");
    map.toIndices(Vector2D{-64.0f, -64.0f}, c, r);
    expect(c == -1 && r == -1, "world -64");

    Vector2D centre = map.toWorld(0, 0);
    expect(centre.equals(Vector2D{32.0f, 32.0f}), "centre 0,0");
    centre = map.toWorld(-1, -1);
    expect(centre.equals(Vector2D{-32.0f, -32.0f}), "centre -1,-1");

    map.toIndices(map.toWorld(3, -2), c, r);
    expect(c == 3 && r == -2, "roundtrip indices");
}

static void testRandomEmpty() {
    TileMap map;
    const std::string path = "tests/_one_empty.csv";
    {
        std::ofstream out(path);
        out << "1,1,1\n1,0,1\n1,1,1\n";
    }
    expect(map.loadCSV(path), "load one empty");
    for (int i = 0; i < 20; ++i) {
        int c = -1, r = -1;
        map.randomEmptyCell(c, r);
        expect(c == 1 && r == 1, "only empty cell");
    }

    Dungeon2D d(30, 30);
    d.generate(7);
    map.loadFromDungeon(d);
    for (int i = 0; i < 50; ++i) {
        int c = 0, r = 0;
        map.randomEmptyCell(c, r);
        expect(c >= 0 && c < map.width() && r >= 0 && r < map.height(), "rand in range");
        expect(map.at(c, r) == EMPTY_TILE, "rand is empty");
    }
}

static void testQueriesNonSquare() {
    Dungeon2D d(30, 5);
    d.generate(3);
    TileMap map;
    map.loadFromDungeon(d);
    expect(map.width() == 30 && map.height() == 5, "non-square size");
    for (int r = 0; r < map.height(); ++r) {
        for (int c = 0; c < map.width(); ++c) {
            expect(map.data()[r * map.width() + c] == map.at(c, r), "data matches at");
            expect(map.isWall(c, r) == (map.at(c, r) == WALL_TILE), "isWall coherent");
        }
    }
}

static void testRegressions() {
    Vector2D a{3.0f, 4.0f};
    expect(std::abs(a.length() - 5.0f) < EPSILON, "s1 length");
    expect(a.normalized().equals(Vector2D{0.6f, 0.8f}), "s1 normalized");

    RigidBody2D body;
    body.acceleration = Vector2D{2.0f, 0.0f};
    body.integrate(0.5f);
    expect(body.velocity.equals(Vector2D{1.0f, 0.0f}), "s2 vel");
    expect(body.position.equals(Vector2D{0.5f, 0.0f}), "s2 pos");

    AABB box{Vector2D{0, 0}, Vector2D{10, 10}};
    AABB touch{Vector2D{10, 0}, Vector2D{20, 10}};
    expect(box.intersects(touch), "s2 touching");
}

int main() {
    testCtorRejects();
    testGenerate(5, 5, 0, "5x5");
    testGenerate(5, 30, 1, "5x30");
    testGenerate(30, 5, 2, "30x5");
    for (unsigned seed : {0u, 1u, 42u, 2026u}) {
        testGenerate(30, 30, seed, "30x30");
        testGenerate(80, 60, seed, "80x60");
    }
    testManualRaster();
    testQueriesNonSquare();
    testCoords();
    testRandomEmpty();

    testCSV();

    testRegressions();

    if (failures != 0) {
        std::cerr << failures << " failed\n";
        return 1;
    }
    std::cout << "ok sessao3\n";
    return 0;
}
