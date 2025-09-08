#include "Playingstate2.h"
#include "Maze.h"
#include "SettingsState.h"

using namespace sf;
using namespace std;

// Main game loop for the maze game mode with smooth player movement.
void handlePlayingState2(RenderWindow& window, bool& running, GameState& state)
{
    static Maze maze(window.getSize().x, window.getSize().y, 32); // Screen-sized maze
    static bool generated = false;
    static Clock clock;

    // Regenerate maze if needed (e.g., after resolution change)
    if (mazeNeedsRegeneration) {
        maze = Maze(window.getSize().x, window.getSize().y, 32);
        maze.generate();
        generated = true;
        clock.restart();
        mazeNeedsRegeneration = false;
    }

    if (!generated) {
        maze.generate();
        generated = true;
        clock.restart();
    }

    float deltaTime = clock.restart().asSeconds();

    // Get continuous input states for smooth movement.
    bool up = Keyboard::isKeyPressed(Keyboard::Key::W) || Keyboard::isKeyPressed(Keyboard::Key::Up);
    bool down = Keyboard::isKeyPressed(Keyboard::Key::S) || Keyboard::isKeyPressed(Keyboard::Key::Down);
    bool left = Keyboard::isKeyPressed(Keyboard::Key::A) || Keyboard::isKeyPressed(Keyboard::Key::Left);
    bool right = Keyboard::isKeyPressed(Keyboard::Key::D) || Keyboard::isKeyPressed(Keyboard::Key::Right);

    // Update player position.
    maze.updatePlayer(deltaTime, up, down, left, right);

    window.clear(Color::Black);
    maze.draw(window);
    maze.drawPlayer(window);

    // Show win message if player reaches the exit.
    static Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        font.openFromFile("arial.ttf");
        fontLoaded = true;
    }

    if (maze.isAtExit()) {
        Text winText(font, "You Win! Press M for Menu", 50);
        winText.setFillColor(Color::Red);
        auto bounds = winText.getLocalBounds();
        winText.setOrigin(Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        winText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f));
        window.draw(winText);
    }

    window.display();

    // Handle menu and settings navigation.
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        state = MENU;
    }
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        extern GameState previousState;
        previousState = PLAYING2;
        state = SETTINGS;
    }
}
