#include "Dungeon2D.hpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <stdexcept>

namespace {

constexpr int kMinRoom = 5;
constexpr int kCorridorWidth = 3;

int rectWidth(const Rect& r) noexcept {
    return r.right - r.left;
}

int rectHeight(const Rect& r) noexcept {
    return r.bottom - r.top;
}

Rect clampCorridor(Rect r, int mapW, int mapH) {
    // Keep full thickness inside the map; shift if needed, do not shrink.
    const int w = rectWidth(r);
    const int h = rectHeight(r);
    if (r.left < 0) {
        r.right -= r.left;
        r.left = 0;
    }
    if (r.top < 0) {
        r.bottom -= r.top;
        r.top = 0;
    }
    if (r.right > mapW) {
        const int shift = r.right - mapW;
        r.left -= shift;
        r.right -= shift;
    }
    if (r.bottom > mapH) {
        const int shift = r.bottom - mapH;
        r.top -= shift;
        r.bottom -= shift;
    }
    // If the map itself is smaller than the corridor thickness, clamp hard.
    r.left = std::max(0, r.left);
    r.top = std::max(0, r.top);
    r.right = std::min(mapW, r.left + w);
    r.bottom = std::min(mapH, r.top + h);
    if (rectWidth(r) < w && mapW >= w) {
        r.left = std::max(0, mapW - w);
        r.right = r.left + w;
    }
    if (rectHeight(r) < h && mapH >= h) {
        r.top = std::max(0, mapH - h);
        r.bottom = r.top + h;
    }
    return r;
}

Rect leafRoom(const BSPNode& node) {
    if (node.isLeaf()) {
        return node.room;
    }
    assert(node.left);
    return leafRoom(*node.left);
}

void placeRoom(BSPNode& leaf, std::mt19937& rng) {
    const int rw = rectWidth(leaf.region);
    const int rh = rectHeight(leaf.region);
    assert(rw >= kMinRoom && rh >= kMinRoom);

    std::uniform_int_distribution<int> widthDist(kMinRoom, rw);
    std::uniform_int_distribution<int> heightDist(kMinRoom, rh);
    const int roomW = widthDist(rng);
    const int roomH = heightDist(rng);

    std::uniform_int_distribution<int> xDist(0, rw - roomW);
    std::uniform_int_distribution<int> yDist(0, rh - roomH);
    const int ox = xDist(rng);
    const int oy = yDist(rng);

    leaf.room.left = leaf.region.left + ox;
    leaf.room.top = leaf.region.top + oy;
    leaf.room.right = leaf.room.left + roomW;
    leaf.room.bottom = leaf.room.top + roomH;
}

void connectRooms(Dungeon2D& dungeon, const Rect& a, const Rect& b, std::mt19937& rng) {
    const int ax = (a.left + a.right) / 2;
    const int ay = (a.top + a.bottom) / 2;
    const int bx = (b.left + b.right) / 2;
    const int by = (b.top + b.bottom) / 2;

    const int half = kCorridorWidth / 2; // 1
    std::uniform_int_distribution<int> coin(0, 1);
    const bool horizontalFirst = coin(rng) == 0;

    auto pushH = [&](int x0, int x1, int y) {
        Rect r;
        r.left = std::min(x0, x1);
        r.right = std::max(x0, x1) + 1;
        r.top = y - half;
        r.bottom = r.top + kCorridorWidth;
        dungeon.corridors.push_back(clampCorridor(r, dungeon.width, dungeon.height));
    };
    auto pushV = [&](int y0, int y1, int x) {
        Rect r;
        r.top = std::min(y0, y1);
        r.bottom = std::max(y0, y1) + 1;
        r.left = x - half;
        r.right = r.left + kCorridorWidth;
        dungeon.corridors.push_back(clampCorridor(r, dungeon.width, dungeon.height));
    };

    if (horizontalFirst) {
        pushH(ax, bx, ay);
        pushV(ay, by, bx);
    } else {
        pushV(ay, by, ax);
        pushH(ax, bx, by);
    }
}

std::unique_ptr<BSPNode> split(Dungeon2D& dungeon, const Rect& region, std::mt19937& rng) {
    auto node = std::make_unique<BSPNode>();
    node->region = region;

    const int w = rectWidth(region);
    const int h = rectHeight(region);
    const bool canSplitV = w >= kMinRoom * 2;
    const bool canSplitH = h >= kMinRoom * 2;

    if (!canSplitV && !canSplitH) {
        placeRoom(*node, rng);
        return node;
    }

    bool splitVertical = canSplitV;
    if (canSplitV && canSplitH) {
        if (w > h) {
            splitVertical = true;
        } else if (h > w) {
            splitVertical = false;
        } else {
            std::uniform_int_distribution<int> coin(0, 1);
            splitVertical = coin(rng) == 0;
        }
    }

    if (splitVertical) {
        // Both sides need at least kMinRoom width: cut in [left+5, right-5].
        std::uniform_int_distribution<int> cutDist(region.left + kMinRoom,
                                                   region.right - kMinRoom);
        const int cut = cutDist(rng);
        Rect leftRegion{region.left, region.top, cut, region.bottom};
        Rect rightRegion{cut, region.top, region.right, region.bottom};
        node->left = split(dungeon, leftRegion, rng);
        node->right = split(dungeon, rightRegion, rng);
    } else {
        std::uniform_int_distribution<int> cutDist(region.top + kMinRoom,
                                                   region.bottom - kMinRoom);
        const int cut = cutDist(rng);
        Rect topRegion{region.left, region.top, region.right, cut};
        Rect bottomRegion{region.left, cut, region.right, region.bottom};
        node->left = split(dungeon, topRegion, rng);
        node->right = split(dungeon, bottomRegion, rng);
    }

    connectRooms(dungeon, leafRoom(*node->left), leafRoom(*node->right), rng);
    return node;
}

} // namespace

Dungeon2D::Dungeon2D(int widthTiles, int heightTiles) {
    if (widthTiles < kMinRoom || heightTiles < kMinRoom) {
        throw std::invalid_argument(
            "Dungeon2D requires width and height of at least 5 tiles");
    }
    width = widthTiles;
    height = heightTiles;
}

void Dungeon2D::generate(unsigned seed) {
    root.reset();
    corridors.clear();

    std::mt19937 rng(seed);
    Rect whole{0, 0, width, height};
    root = split(*this, whole, rng);
}
