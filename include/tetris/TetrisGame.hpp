#pragma once

#include "tetris/Tetromino.hpp"

#include <SFML/Window/Keyboard.hpp>

#include <array>
#include <cstddef>
#include <deque>
#include <random>

namespace tetris {

class TetrisGame {
public:
    static constexpr int BoardWidth = 10;
    static constexpr int BoardHeight = 20;

    using Board = std::array<std::array<int, BoardWidth>, BoardHeight>;

    enum class State { Playing, Paused, GameOver };

    TetrisGame();

    void restart();
    void update(float deltaSeconds, bool softDropHeld);
    void handleKey(sf::Keyboard::Key key);

    [[nodiscard]] State state() const noexcept { return state_; }
    [[nodiscard]] const Board& board() const noexcept { return board_; }
    [[nodiscard]] const std::deque<int>& nextQueue() const noexcept { return nextQueue_; }
    [[nodiscard]] const TetrominoCells& currentCells() const noexcept { return currentCells_; }
    [[nodiscard]] int currentType() const noexcept { return currentType_; }
    [[nodiscard]] int currentX() const noexcept { return currentX_; }
    [[nodiscard]] int currentY() const noexcept { return currentY_; }
    [[nodiscard]] int holdType() const noexcept { return holdType_; }
    [[nodiscard]] bool canHold() const noexcept { return canHold_; }
    [[nodiscard]] int score() const noexcept { return score_; }
    [[nodiscard]] int bestScore() const noexcept { return bestScore_; }
    [[nodiscard]] int lines() const noexcept { return lines_; }
    [[nodiscard]] int level() const noexcept { return level_; }
    [[nodiscard]] int ghostY() const;

private:
    [[nodiscard]] bool canPlace(const TetrominoCells& cells, int px, int py) const;
    [[nodiscard]] bool moveBy(int dx, int dy);
    void rotatePiece(bool clockwise);
    void hardDrop();
    void holdPiece();
    void spawnNextPiece();
    void fillQueue();
    [[nodiscard]] int drawFromBag();
    void lockCurrentPiece();
    [[nodiscard]] int clearFullLines();
    void togglePause();
    void updateBestScore();
    void loadBestScore();
    void saveBestScore() const;
    [[nodiscard]] float fallInterval() const;

    Board board_{};
    std::deque<int> nextQueue_;
    std::array<int, 7> bag_{};
    std::size_t bagIndex_{0};
    std::mt19937 randomEngine_;

    TetrominoCells currentCells_{};
    int currentType_{0};
    int currentX_{0};
    int currentY_{0};
    int holdType_{-1};
    bool canHold_{true};

    int score_{0};
    int bestScore_{0};
    int lines_{0};
    int level_{1};
    State state_{State::Playing};

    float fallTimer_{0.0F};
    float softDropTimer_{0.0F};
};

} // namespace tetris
