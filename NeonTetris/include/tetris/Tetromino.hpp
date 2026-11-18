#pragma once

#include <array>

namespace tetris {

struct Cell {
    int x{};
    int y{};
};

using TetrominoCells = std::array<Cell, 4>;

// Returns the spawn orientation of piece `type` (0=I, 1=O, 2=T, 3=S, 4=Z, 5=J, 6=L),
// normalised so the top-left of its bounding box is (0, 0).
[[nodiscard]] TetrominoCells spawnCells(int type);

// Rotates a piece 90 degrees and re-normalises it to the origin.
[[nodiscard]] TetrominoCells rotateCells(const TetrominoCells& cells, bool clockwise);

} // namespace tetris
