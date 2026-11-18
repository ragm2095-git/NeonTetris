#include "tetris/Renderer.hpp"
#include "tetris/TetrisGame.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <iostream>
#include <optional>

int main() {
    sf::RenderWindow window(
        sf::VideoMode({1000U, 760U}),
        "Neon Tetris | C++20",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    tetris::TetrisGame game;
    tetris::Renderer renderer;
    if (!renderer.hasFont()) {
        std::cerr << "Warning: no system font was found. The game still runs, but UI text may be hidden.\n";
    }

    sf::Clock clock;
    while (window.isOpen()) {
        const float deltaSeconds = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::F11) {
                    // The window remains a fixed-size game board by design.
                    continue;
                }
                game.handleKey(keyPressed->code);
            }
        }

        const bool softDropHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
        game.update(deltaSeconds, softDropHeld);
        renderer.draw(window, game);
    }

    return 0;
}
