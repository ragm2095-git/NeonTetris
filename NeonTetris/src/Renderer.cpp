#include "tetris/Renderer.hpp"

#include "tetris/Tetromino.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

namespace tetris {
namespace {
constexpr float WindowWidth = 1000.0F;
constexpr float WindowHeight = 760.0F;
constexpr float CellSize = 30.0F;
constexpr float BoardX = 350.0F;
constexpr float BoardY = 75.0F;
constexpr float BoardPixelWidth = CellSize * static_cast<float>(TetrisGame::BoardWidth);
constexpr float BoardPixelHeight = CellSize * static_cast<float>(TetrisGame::BoardHeight);

constexpr sf::Color Background{9, 13, 27};
constexpr sf::Color PanelColor{17, 24, 43};
constexpr sf::Color PanelEdge{38, 51, 77};
constexpr sf::Color MutedText{139, 155, 184};
constexpr sf::Color BrightText{238, 244, 255};
constexpr sf::Color Accent{135, 109, 255};

[[nodiscard]] sf::Color mix(sf::Color base, sf::Color other, float amount) {
    amount = std::clamp(amount, 0.0F, 1.0F);
    const auto blend = [amount](std::uint8_t a, std::uint8_t b) {
        return static_cast<std::uint8_t>(static_cast<float>(a) * (1.0F - amount) + static_cast<float>(b) * amount);
    };
    return {blend(base.r, other.r), blend(base.g, other.g), blend(base.b, other.b), base.a};
}

} // namespace

Renderer::Renderer() {
    const std::array<std::string, 6> candidates{
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf"
    };

    for (const auto& path : candidates) {
        if (std::filesystem::exists(path) && font_.openFromFile(path)) {
            hasFont_ = true;
            break;
        }
    }
}

void Renderer::draw(sf::RenderWindow& window, const TetrisGame& game) const {
    window.clear(Background);

    // Quiet decorative grid and ambient corner glows.
    sf::VertexArray gridLines(sf::PrimitiveType::Lines);
    for (int x = 0; x <= static_cast<int>(WindowWidth); x += 50) {
        gridLines.append(sf::Vertex{{static_cast<float>(x), 0.0F}, sf::Color(20, 29, 49, 100)});
        gridLines.append(sf::Vertex{{static_cast<float>(x), WindowHeight}, sf::Color(20, 29, 49, 100)});
    }
    for (int y = 0; y <= static_cast<int>(WindowHeight); y += 50) {
        gridLines.append(sf::Vertex{{0.0F, static_cast<float>(y)}, sf::Color(20, 29, 49, 100)});
        gridLines.append(sf::Vertex{{WindowWidth, static_cast<float>(y)}, sf::Color(20, 29, 49, 100)});
    }
    window.draw(gridLines);

    sf::CircleShape glow(180.0F);
    glow.setFillColor(sf::Color(80, 50, 180, 17));
    glow.setPosition({720.0F, -150.0F});
    window.draw(glow);
    glow.setRadius(150.0F);
    glow.setFillColor(sf::Color(0, 170, 200, 13));
    glow.setPosition({-100.0F, 520.0F});
    window.draw(glow);

    drawText(window, "NEON", 40.0F, 18.0F, 30, BrightText);
    drawText(window, "TETRIS", 132.0F, 18.0F, 30, Accent);
    drawText(window, "A MODERN C++ ARCADE BUILD", 40.0F, 52.0F, 10, MutedText);

    sf::RectangleShape statusPill({132.0F, 28.0F});
    statusPill.setPosition({828.0F, 25.0F});
    statusPill.setFillColor(sf::Color(21, 35, 48));
    statusPill.setOutlineColor(sf::Color(45, 91, 91));
    statusPill.setOutlineThickness(1.0F);
    window.draw(statusPill);
    sf::CircleShape liveDot(4.0F);
    liveDot.setFillColor(game.state() == TetrisGame::State::Playing ? sf::Color(67, 231, 166) : sf::Color(255, 190, 79));
    liveDot.setPosition({841.0F, 35.0F});
    window.draw(liveDot);
    const std::string status = game.state() == TetrisGame::State::Playing ? "SYSTEM READY" :
                               game.state() == TetrisGame::State::Paused ? "PAUSED" : "GAME OVER";
    drawText(window, status, 853.0F, 32.0F, 10, BrightText);

    drawLeftPanel(window, game);
    drawRightPanel(window, game);

    sf::RectangleShape boardFrame({BoardPixelWidth + 24.0F, BoardPixelHeight + 24.0F});
    boardFrame.setPosition({BoardX - 12.0F, BoardY - 12.0F});
    boardFrame.setFillColor(sf::Color(12, 18, 33));
    boardFrame.setOutlineColor(PanelEdge);
    boardFrame.setOutlineThickness(2.0F);
    window.draw(boardFrame);

    drawBoard(window, game);

    if (game.state() != TetrisGame::State::Playing) {
        sf::RectangleShape overlay({BoardPixelWidth, BoardPixelHeight});
        overlay.setPosition({BoardX, BoardY});
        overlay.setFillColor(sf::Color(5, 8, 18, 205));
        window.draw(overlay);

        if (game.state() == TetrisGame::State::Paused) {
            drawCenteredText(window, "PAUSED", BoardX + BoardPixelWidth / 2.0F, BoardY + 245.0F, 30, BrightText);
            drawCenteredText(window, "P / ESC TO RESUME", BoardX + BoardPixelWidth / 2.0F, BoardY + 290.0F, 11, MutedText);
        } else {
            drawCenteredText(window, "GAME OVER", BoardX + BoardPixelWidth / 2.0F, BoardY + 230.0F, 27, sf::Color(255, 112, 139));
            drawCenteredText(window, "SCORE  " + std::to_string(game.score()), BoardX + BoardPixelWidth / 2.0F, BoardY + 278.0F, 17, BrightText);
            drawCenteredText(window, "ENTER OR R TO RETRY", BoardX + BoardPixelWidth / 2.0F, BoardY + 318.0F, 11, MutedText);
        }
    }

    drawText(window, "BUILD. CLEAR. REPEAT.", 40.0F, 711.0F, 10, MutedText);
    drawText(window, "C++20  /  SFML 3  /  CMAKE", 715.0F, 711.0F, 10, MutedText);
    window.display();
}

void Renderer::drawBoard(sf::RenderTarget& target, const TetrisGame& game) const {
    const auto& board = game.board();
    for (int y = 0; y < TetrisGame::BoardHeight; ++y) {
        for (int x = 0; x < TetrisGame::BoardWidth; ++x) {
            const float px = BoardX + static_cast<float>(x) * CellSize;
            const float py = BoardY + static_cast<float>(y) * CellSize;
            sf::RectangleShape cell({CellSize - 1.0F, CellSize - 1.0F});
            cell.setPosition({px + 0.5F, py + 0.5F});
            cell.setFillColor(sf::Color(15, 22, 39));
            cell.setOutlineColor(sf::Color(25, 34, 54));
            cell.setOutlineThickness(0.5F);
            target.draw(cell);

            const int value = board[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            if (value > 0) {
                drawBlock(target, px, py, CellSize, pieceColor(value - 1));
            }
        }
    }

    if (game.state() != TetrisGame::State::GameOver) {
        const int ghostY = game.ghostY();
        const sf::Color ghostColor = pieceColor(game.currentType());
        for (const auto& cell : game.currentCells()) {
            const int x = game.currentX() + cell.x;
            const int y = ghostY + cell.y;
            if (x >= 0 && x < TetrisGame::BoardWidth && y >= 0 && y < TetrisGame::BoardHeight) {
                drawBlock(target, BoardX + static_cast<float>(x) * CellSize,
                          BoardY + static_cast<float>(y) * CellSize, CellSize,
                          sf::Color(ghostColor.r, ghostColor.g, ghostColor.b, 65), true);
            }
        }
    }

    if (game.state() == TetrisGame::State::Playing || game.state() == TetrisGame::State::Paused) {
        for (const auto& cell : game.currentCells()) {
            const int x = game.currentX() + cell.x;
            const int y = game.currentY() + cell.y;
            if (x >= 0 && x < TetrisGame::BoardWidth && y >= 0 && y < TetrisGame::BoardHeight) {
                drawBlock(target, BoardX + static_cast<float>(x) * CellSize,
                          BoardY + static_cast<float>(y) * CellSize, CellSize,
                          pieceColor(game.currentType()));
            }
        }
    }
}

void Renderer::drawLeftPanel(sf::RenderTarget& target, const TetrisGame& game) const {
    drawPanel(target, 40.0F, 78.0F, 270.0F, 615.0F);
    drawText(target, "RUN SUMMARY", 60.0F, 100.0F, 12, BrightText);
    drawText(target, "YOUR CURRENT SESSION", 60.0F, 122.0F, 9, MutedText);

    sf::RectangleShape accentBar({4.0F, 72.0F});
    accentBar.setPosition({60.0F, 158.0F});
    accentBar.setFillColor(Accent);
    target.draw(accentBar);
    drawText(target, "SCORE", 76.0F, 157.0F, 10, MutedText);
    drawText(target, std::to_string(game.score()), 76.0F, 178.0F, 32, BrightText);

    sf::RectangleShape divider({230.0F, 1.0F});
    divider.setPosition({60.0F, 250.0F});
    divider.setFillColor(PanelEdge);
    target.draw(divider);

    drawText(target, "PERSONAL BEST", 60.0F, 271.0F, 10, MutedText);
    drawText(target, std::to_string(game.bestScore()), 60.0F, 292.0F, 22, sf::Color(83, 226, 190));

    divider.setPosition({60.0F, 340.0F});
    target.draw(divider);
    drawText(target, "LINES CLEARED", 60.0F, 361.0F, 10, MutedText);
    drawText(target, std::to_string(game.lines()), 60.0F, 383.0F, 23, BrightText);
    drawText(target, "LEVEL", 190.0F, 361.0F, 10, MutedText);
    drawText(target, std::to_string(game.level()), 190.0F, 383.0F, 23, BrightText);

    drawText(target, "NEXT LEVEL", 60.0F, 439.0F, 10, MutedText);
    const int linesIntoLevel = game.lines() % 10;
    sf::RectangleShape progressBg({230.0F, 8.0F});
    progressBg.setPosition({60.0F, 460.0F});
    progressBg.setFillColor(sf::Color(33, 42, 64));
    target.draw(progressBg);
    sf::RectangleShape progress({230.0F * static_cast<float>(linesIntoLevel) / 10.0F, 8.0F});
    progress.setPosition({60.0F, 460.0F});
    progress.setFillColor(Accent);
    target.draw(progress);
    drawText(target, std::to_string(10 - linesIntoLevel) + " LINES TO LEVEL UP", 60.0F, 478.0F, 9, MutedText);

    divider.setPosition({60.0F, 518.0F});
    target.draw(divider);
    drawText(target, "SCORING", 60.0F, 538.0F, 10, BrightText);
    drawText(target, "SINGLE       100 x level", 60.0F, 564.0F, 10, MutedText);
    drawText(target, "DOUBLE       300 x level", 60.0F, 586.0F, 10, MutedText);
    drawText(target, "TRIPLE        500 x level", 60.0F, 608.0F, 10, MutedText);
    drawText(target, "TETRIS        800 x level", 60.0F, 630.0F, 10, MutedText);
    drawText(target, "SOFT DROP +1  /  HARD DROP +2", 60.0F, 658.0F, 8, sf::Color(106, 126, 160));
}

void Renderer::drawRightPanel(sf::RenderTarget& target, const TetrisGame& game) const {
    drawPanel(target, 690.0F, 78.0F, 270.0F, 615.0F);
    drawText(target, "UP NEXT", 712.0F, 100.0F, 12, BrightText);
    drawText(target, "THE NEXT FIVE PIECES", 712.0F, 122.0F, 9, MutedText);

    int row = 0;
    for (const int type : game.nextQueue()) {
        const float y = 145.0F + static_cast<float>(row) * 43.0F;
        sf::RectangleShape slot({226.0F, 37.0F});
        slot.setPosition({712.0F, y});
        slot.setFillColor(row == 0 ? sf::Color(26, 32, 58) : sf::Color(14, 21, 39));
        slot.setOutlineColor(row == 0 ? sf::Color(75, 67, 126) : sf::Color(32, 43, 64));
        slot.setOutlineThickness(1.0F);
        target.draw(slot);
        drawMiniPiece(target, type, 722.0F, y + 2.0F, 15.0F, 72.0F, 33.0F);
        drawText(target, "0" + std::to_string(row + 1), 912.0F, y + 11.0F, 10, MutedText);
        ++row;
    }

    sf::RectangleShape divider({226.0F, 1.0F});
    divider.setPosition({712.0F, 370.0F});
    divider.setFillColor(PanelEdge);
    target.draw(divider);
    drawText(target, "HOLD PIECE", 712.0F, 389.0F, 12, BrightText);
    drawText(target, game.canHold() ? "READY TO SWAP" : "USED THIS TURN", 712.0F, 410.0F, 9, MutedText);

    sf::RectangleShape holdBox({226.0F, 64.0F});
    holdBox.setPosition({712.0F, 432.0F});
    holdBox.setFillColor(sf::Color(14, 21, 39));
    holdBox.setOutlineColor(PanelEdge);
    holdBox.setOutlineThickness(1.0F);
    target.draw(holdBox);
    if (game.holdType() >= 0) {
        drawMiniPiece(target, game.holdType(), 788.0F, 442.0F, 17.0F, 76.0F, 44.0F);
    } else {
        drawCenteredText(target, "EMPTY", 825.0F, 454.0F, 10, MutedText);
    }

    divider.setPosition({712.0F, 516.0F});
    target.draw(divider);
    drawText(target, "CONTROLS", 712.0F, 533.0F, 12, BrightText);

    const std::array<std::pair<std::string, std::string>, 7> controls{{
        {"A / D  or  arrows", "MOVE"},
        {"DOWN", "SOFT DROP"},
        {"UP / X", "ROTATE CW"},
        {"Z", "ROTATE CCW"},
        {"SPACE", "HARD DROP"},
        {"C", "HOLD PIECE"},
        {"P / ESC", "PAUSE"}
    }};
    float y = 557.0F;
    for (const auto& [key, action] : controls) {
        drawText(target, key, 712.0F, y, 9, sf::Color(197, 207, 229));
        drawText(target, action, 845.0F, y, 8, MutedText);
        y += 17.0F;
    }
    drawText(target, "R  RESTART", 712.0F, 678.0F, 9, sf::Color(255, 190, 79));
}

void Renderer::drawText(sf::RenderTarget& target, const std::string& value,
                        float x, float y, unsigned size, sf::Color color) const {
    if (!hasFont_) {
        return;
    }
    sf::Text text(font_, value, size);
    text.setFillColor(color);
    text.setPosition({x, y});
    target.draw(text);
}

void Renderer::drawCenteredText(sf::RenderTarget& target, const std::string& value,
                                float centerX, float y, unsigned size, sf::Color color) const {
    if (!hasFont_) {
        return;
    }
    sf::Text text(font_, value, size);
    text.setFillColor(color);
    const auto bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.0F,
                    bounds.position.y + bounds.size.y / 2.0F});
    text.setPosition({centerX, y});
    target.draw(text);
}

void Renderer::drawPanel(sf::RenderTarget& target, float x, float y,
                         float width, float height) const {
    sf::RectangleShape panel({width, height});
    panel.setPosition({x, y});
    panel.setFillColor(PanelColor);
    panel.setOutlineColor(PanelEdge);
    panel.setOutlineThickness(1.0F);
    target.draw(panel);
}

void Renderer::drawBlock(sf::RenderTarget& target, float x, float y,
                         float size, sf::Color color, bool ghost) const {
    sf::RectangleShape block({size - 3.0F, size - 3.0F});
    block.setPosition({x + 1.5F, y + 1.5F});
    if (ghost) {
        block.setFillColor(sf::Color(color.r, color.g, color.b, 18));
        block.setOutlineColor(sf::Color(color.r, color.g, color.b, 155));
        block.setOutlineThickness(1.5F);
        target.draw(block);
        return;
    }

    block.setFillColor(color);
    block.setOutlineColor(mix(color, sf::Color::Black, 0.38F));
    block.setOutlineThickness(1.0F);
    target.draw(block);

    sf::RectangleShape shine({size - 8.0F, 3.0F});
    shine.setPosition({x + 4.0F, y + 4.0F});
    shine.setFillColor(mix(color, sf::Color::White, 0.38F));
    target.draw(shine);

    sf::RectangleShape shade({size - 8.0F, 2.0F});
    shade.setPosition({x + 4.0F, y + size - 6.0F});
    shade.setFillColor(mix(color, sf::Color::Black, 0.25F));
    target.draw(shade);
}

void Renderer::drawMiniPiece(sf::RenderTarget& target, int type, float x, float y,
                             float cellSize, float boxWidth, float boxHeight) const {
    const auto cells = spawnCells(type);
    int maxX = 0;
    int maxY = 0;
    for (const auto& cell : cells) {
        maxX = std::max(maxX, cell.x);
        maxY = std::max(maxY, cell.y);
    }
    const float pieceWidth = static_cast<float>(maxX + 1) * cellSize;
    const float pieceHeight = static_cast<float>(maxY + 1) * cellSize;
    const float offsetX = x + (boxWidth - pieceWidth) / 2.0F;
    const float offsetY = y + (boxHeight - pieceHeight) / 2.0F;
    for (const auto& cell : cells) {
        drawBlock(target, offsetX + static_cast<float>(cell.x) * cellSize,
                  offsetY + static_cast<float>(cell.y) * cellSize, cellSize,
                  pieceColor(type));
    }
}

sf::Color Renderer::pieceColor(int type) const {
    const std::array<sf::Color, 7> colors{{
        sf::Color(70, 220, 245),  // I - cyan
        sf::Color(255, 211, 78), // O - gold
        sf::Color(177, 114, 255),// T - violet
        sf::Color(91, 229, 154), // S - green
        sf::Color(255, 91, 125), // Z - coral
        sf::Color(91, 139, 255), // J - blue
        sf::Color(255, 158, 76)  // L - orange
    }};
    if (type < 0 || type >= static_cast<int>(colors.size())) {
        return BrightText;
    }
    return colors[static_cast<std::size_t>(type)];
}

} // namespace tetris
