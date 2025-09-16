#include "Playingstate2.h"
#include "Maze.h"
#include "SettingsState.h"

using namespace sf;
using namespace std;

// Function to calculate maze dimensions based on resolution setting
Vector2u getMazeDimensions() {
    extern int resolutionIndex;
    extern vector<Vector2u> resolutionOptions;

    // Use the resolution setting to determine maze size
    auto mazeSize = resolutionOptions[resolutionIndex];

    // Scale down the values to reasonable maze dimensions
    // Divide by a factor to get appropriate cell counts
    int mazeCellsX = mazeSize.x / 40;  // Adjust this divisor as needed
    int mazeCellsY = mazeSize.y / 36;  // Adjust this divisor as needed

    // Ensure minimum and maximum maze sizes
    mazeCellsX = max(10, min(100, mazeCellsX));
    mazeCellsY = max(8, min(75, mazeCellsY));

    return Vector2u(mazeCellsX, mazeCellsY);
}

// Main game loop for the maze game mode with smooth player movement.
void handlePlayingState2(RenderWindow& window, bool& running, GameState& state)
{
    // Get maze dimensions based on current resolution setting
    static Vector2u lastMazeDims = getMazeDimensions();
    Vector2u currentMazeDims = getMazeDimensions();

    // Calculate cell size to fit the maze in the window
    static int cellSize = min(window.getSize().x / currentMazeDims.x, window.getSize().y / currentMazeDims.y);

    static Maze maze(currentMazeDims.x * cellSize, currentMazeDims.y * cellSize, cellSize);
    static bool generated = false;
    static Clock clock;
    extern bool mazeNeedsRegeneration;

    // Regenerate maze if settings changed or maze dimensions changed
    if (mazeNeedsRegeneration || currentMazeDims.x != lastMazeDims.x || currentMazeDims.y != lastMazeDims.y) {
        // Recalculate cell size for new dimensions
        cellSize = min(window.getSize().x / currentMazeDims.x, window.getSize().y / currentMazeDims.y);

        // Create new maze with updated dimensions
        maze = Maze(currentMazeDims.x * cellSize, currentMazeDims.y * cellSize, cellSize);
        maze.generate();
        generated = true;
        clock.restart();
        mazeNeedsRegeneration = false;
        lastMazeDims = currentMazeDims;
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
        Text winText(font, "You Win! Press ENTER for next level", 50);
        winText.setFillColor(Color::Red);
        winText.setOutlineColor(Color::Black);
        winText.setOutlineThickness(2.f);
        auto bounds = winText.getLocalBounds();
        winText.setOrigin(Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        winText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f));
        window.draw(winText);
    }

    // window.display();

    // Handle menu and settings navigation.
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        state = MENU;
    }
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        extern GameState previousState;
        previousState = PLAYING2;
        state = SETTINGS;
    }
    if (maze.isAtExit() && Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
        state = PRELEVEL3; // Go to pre-level screen before level 3
    }
    if (Keyboard::isKeyPressed(Keyboard::Key::H)) {
        state = PRELEVEL3; // Also go to pre-level screen
    }
}