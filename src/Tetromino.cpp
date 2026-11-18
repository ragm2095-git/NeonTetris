#include "tetris/Tetromino.hpp"

#include <algorithm>
#include <array>

namespace tetris {
namespace {

[[nodiscard]] TetrominoCells normalize(TetrominoCells cells) {
    int minX = cells[0].x;
    int minY = cells[0].y;
    for (const auto& cell : cells) {
        minX = std::min(minX, cell.x);
        minY = std::min(minY, cell.y);
    }
    for (auto& cell : cells) {
        cell.x -= minX;
        cell.y -= minY;
    }
    return cells;
}

constexpr std::array<TetrominoCells, 7> Shapes{{
    // I
    {{{0, 1}, {1, 1}, {2, 1}, {3, 1}}},
    // O
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    // T
    {{{1, 0}, {0, 1}, {1, 1}, {2, 1}}},
    // S
    {{{1, 0}, {2, 0}, {0, 1}, {1, 1}}},
    // Z
    {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}},
    // J
    {{{0, 0}, {0, 1}, {1, 1}, {2, 1}}},
    // L
    {{{2, 0}, {0, 1}, {1, 1}, {2, 1}}}
}};

} // namespace

TetrominoCells spawnCells(int type) {
    const int safeType = (type >= 0 && type < static_cast<int>(Shapes.size())) ? type : 0;
    return normalize(Shapes[static_cast<std::size_t>(safeType)]);
}

TetrominoCells rotateCells(const TetrominoCells& cells, bool clockwise) {
    TetrominoCells rotated = cells;
    for (auto& cell : rotated) {
        const int oldX = cell.x;
        const int oldY = cell.y;
        if (clockwise) {
            // Screen coordinates have positive y pointing downwards.
            cell.x = -oldY;
            cell.y = oldX;
        } else {
            cell.x = oldY;
            cell.y = -oldX;
        }
    }
    return normalize(rotated);
}

} // namespace tetris
