#pragma once

#include "tetris/TetrisGame.hpp"

#include <SFML/Graphics.hpp>

#include <string>

namespace tetris {

class Renderer {
public:
    Renderer();

    [[nodiscard]] bool hasFont() const noexcept { return hasFont_; }
    void draw(sf::RenderWindow& window, const TetrisGame& game) const;

private:
    void drawBoard(sf::RenderTarget& target, const TetrisGame& game) const;
    void drawLeftPanel(sf::RenderTarget& target, const TetrisGame& game) const;
    void drawRightPanel(sf::RenderTarget& target, const TetrisGame& game) const;
    void drawText(sf::RenderTarget& target, const std::string& value,
                  float x, float y, unsigned size, sf::Color color) const;
    void drawCenteredText(sf::RenderTarget& target, const std::string& value,
                          float centerX, float y, unsigned size, sf::Color color) const;
    void drawPanel(sf::RenderTarget& target, float x, float y, float width, float height) const;
    void drawBlock(sf::RenderTarget& target, float x, float y, float size,
                   sf::Color color, bool ghost = false) const;
    void drawMiniPiece(sf::RenderTarget& target, int type, float x, float y,
                       float cellSize, float boxWidth, float boxHeight) const;
    [[nodiscard]] sf::Color pieceColor(int type) const;

    sf::Font font_;
    bool hasFont_{false};
};

} // namespace tetris
