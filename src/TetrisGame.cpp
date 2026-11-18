#include "tetris/TetrisGame.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <random>
#include <string>
#include <utility>

namespace tetris {
namespace {
constexpr std::size_t QueuePreviewCount = 5;
constexpr float SoftDropStepSeconds = 0.035F;
constexpr std::array<int, 5> LineClearScores{0, 100, 300, 500, 800};
constexpr const char* HighScoreFilename = "highscore.txt";

[[nodiscard]] int centeredSpawnX(const TetrominoCells& cells) {
    int maxX = 0;
    for (const auto& cell : cells) {
        maxX = std::max(maxX, cell.x);
    }
    const int pieceWidth = maxX + 1;
    return (TetrisGame::BoardWidth - pieceWidth) / 2;
}
} // namespace

TetrisGame::TetrisGame()
    : randomEngine_(std::random_device{}()) {
    loadBestScore();
    restart();
}

void TetrisGame::restart() {
    for (auto& row : board_) {
        row.fill(0);
    }

    nextQueue_.clear();
    bagIndex_ = bag_.size();
    holdType_ = -1;
    canHold_ = true;
    score_ = 0;
    lines_ = 0;
    level_ = 1;
    state_ = State::Playing;
    fallTimer_ = 0.0F;
    softDropTimer_ = 0.0F;

    fillQueue();
    spawnNextPiece();
}

void TetrisGame::update(float deltaSeconds, bool softDropHeld) {
    if (state_ != State::Playing) {
        return;
    }

    // Cap large frame times so returning to the game after a window drag does not
    // make several pieces fall in one update.
    const float dt = std::clamp(deltaSeconds, 0.0F, 0.05F);

    if (softDropHeld) {
        fallTimer_ = 0.0F;
        softDropTimer_ += dt;
        while (softDropTimer_ >= SoftDropStepSeconds && state_ == State::Playing) {
            softDropTimer_ -= SoftDropStepSeconds;
            if (moveBy(0, 1)) {
                ++score_;
                updateBestScore();
            } else {
                lockCurrentPiece();
                break;
            }
        }
        return;
    }

    softDropTimer_ = 0.0F;
    fallTimer_ += dt;
    const float interval = fallInterval();
    while (fallTimer_ >= interval && state_ == State::Playing) {
        fallTimer_ -= interval;
        if (!moveBy(0, 1)) {
            lockCurrentPiece();
            break;
        }
    }
}

void TetrisGame::handleKey(sf::Keyboard::Key key) {
    using Key = sf::Keyboard::Key;

    if (state_ == State::GameOver) {
        if (key == Key::Enter || key == Key::R) {
            restart();
        }
        return;
    }

    if (key == Key::Escape || key == Key::P) {
        togglePause();
        return;
    }

    if (state_ != State::Playing) {
        if (key == Key::R) {
            restart();
        }
        return;
    }

    switch (key) {
    case Key::Left:
    case Key::A:
        (void)moveBy(-1, 0);
        break;
    case Key::Right:
    case Key::D:
        (void)moveBy(1, 0);
        break;
    case Key::Up:
    case Key::X:
        rotatePiece(true);
        break;
    case Key::Z:
        rotatePiece(false);
        break;
    case Key::Space:
        hardDrop();
        break;
    case Key::C:
        holdPiece();
        break;
    case Key::R:
        restart();
        break;
    default:
        break;
    }
}

bool TetrisGame::canPlace(const TetrominoCells& cells, int px, int py) const {
    for (const auto& cell : cells) {
        const int x = px + cell.x;
        const int y = py + cell.y;
        if (x < 0 || x >= BoardWidth || y < 0 || y >= BoardHeight) {
            return false;
        }
        if (board_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] != 0) {
            return false;
        }
    }
    return true;
}

bool TetrisGame::moveBy(int dx, int dy) {
    if (!canPlace(currentCells_, currentX_ + dx, currentY_ + dy)) {
        return false;
    }
    currentX_ += dx;
    currentY_ += dy;
    return true;
}

void TetrisGame::rotatePiece(bool clockwise) {
    const auto rotated = rotateCells(currentCells_, clockwise);
    // A compact wall-kick system: try the original position, horizontal nudges,
    // and one row upward. This makes rotations near walls less frustrating.
    constexpr std::array<std::array<int, 2>, 6> kicks{{
        {{0, 0}}, {{-1, 0}}, {{1, 0}}, {{-2, 0}}, {{2, 0}}, {{0, -1}}
    }};
    for (const auto& kick : kicks) {
        const int candidateX = currentX_ + kick[0];
        const int candidateY = currentY_ + kick[1];
        if (canPlace(rotated, candidateX, candidateY)) {
            currentCells_ = rotated;
            currentX_ = candidateX;
            currentY_ = candidateY;
            return;
        }
    }
}

void TetrisGame::hardDrop() {
    int distance = 0;
    while (canPlace(currentCells_, currentX_, currentY_ + 1)) {
        ++currentY_;
        ++distance;
    }
    score_ += distance * 2;
    updateBestScore();
    lockCurrentPiece();
}

void TetrisGame::holdPiece() {
    if (!canHold_ || state_ != State::Playing) {
        return;
    }

    const int outgoingType = currentType_;
    if (holdType_ < 0) {
        holdType_ = outgoingType;
        spawnNextPiece();
    } else {
        currentType_ = holdType_;
        holdType_ = outgoingType;
        currentCells_ = spawnCells(currentType_);
        currentX_ = centeredSpawnX(currentCells_);
        currentY_ = 0;
        if (!canPlace(currentCells_, currentX_, currentY_)) {
            state_ = State::GameOver;
        }
    }
    canHold_ = false;
}

void TetrisGame::spawnNextPiece() {
    fillQueue();
    currentType_ = nextQueue_.front();
    nextQueue_.pop_front();
    fillQueue();

    currentCells_ = spawnCells(currentType_);
    currentX_ = centeredSpawnX(currentCells_);
    currentY_ = 0;
    canHold_ = true;
    fallTimer_ = 0.0F;
    softDropTimer_ = 0.0F;

    if (!canPlace(currentCells_, currentX_, currentY_)) {
        state_ = State::GameOver;
    }
}

void TetrisGame::fillQueue() {
    while (nextQueue_.size() < QueuePreviewCount) {
        nextQueue_.push_back(drawFromBag());
    }
}

int TetrisGame::drawFromBag() {
    if (bagIndex_ >= bag_.size()) {
        std::iota(bag_.begin(), bag_.end(), 0);
        std::ranges::shuffle(bag_, randomEngine_);
        bagIndex_ = 0;
    }
    return bag_[bagIndex_++];
}

void TetrisGame::lockCurrentPiece() {
    for (const auto& cell : currentCells_) {
        const int x = currentX_ + cell.x;
        const int y = currentY_ + cell.y;
        if (y < 0) {
            state_ = State::GameOver;
            return;
        }
        if (x >= 0 && x < BoardWidth && y < BoardHeight) {
            board_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = currentType_ + 1;
        }
    }

    const int cleared = clearFullLines();
    if (cleared > 0) {
        score_ += LineClearScores[static_cast<std::size_t>(cleared)] * level_;
        lines_ += cleared;
        level_ = lines_ / 10 + 1;
        updateBestScore();
    }
    spawnNextPiece();
}

int TetrisGame::clearFullLines() {
    int cleared = 0;
    int writeRow = BoardHeight - 1;

    for (int readRow = BoardHeight - 1; readRow >= 0; --readRow) {
        const auto& row = board_[static_cast<std::size_t>(readRow)];
        const bool full = std::ranges::all_of(row, [](int value) { return value != 0; });
        if (full) {
            ++cleared;
            continue;
        }

        if (writeRow != readRow) {
            board_[static_cast<std::size_t>(writeRow)] = board_[static_cast<std::size_t>(readRow)];
        }
        --writeRow;
    }

    for (int row = writeRow; row >= 0; --row) {
        board_[static_cast<std::size_t>(row)].fill(0);
    }
    return cleared;
}

void TetrisGame::togglePause() {
    if (state_ == State::Playing) {
        state_ = State::Paused;
    } else if (state_ == State::Paused) {
        state_ = State::Playing;
        fallTimer_ = 0.0F;
        softDropTimer_ = 0.0F;
    }
}

void TetrisGame::updateBestScore() {
    if (score_ > bestScore_) {
        bestScore_ = score_;
        saveBestScore();
    }
}

void TetrisGame::loadBestScore() {
    std::ifstream input{std::filesystem::path(HighScoreFilename)};
    if (input) {
        int value = 0;
        if (input >> value && value >= 0) {
            bestScore_ = value;
        }
    }
}

void TetrisGame::saveBestScore() const {
    std::ofstream output{std::filesystem::path(HighScoreFilename), std::ios::trunc};
    if (output) {
        output << bestScore_ << '\n';
    }
}

float TetrisGame::fallInterval() const {
    // Each level accelerates the game; clamp to keep it human-playable.
    return std::max(0.085F, 0.78F - static_cast<float>(level_ - 1) * 0.055F);
}

int TetrisGame::ghostY() const {
    int y = currentY_;
    while (canPlace(currentCells_, currentX_, y + 1)) {
        ++y;
    }
    return y;
}

} // namespace tetris
