#include "PreLevelState.h"

void handlePreLevelState(RenderWindow& window, bool& running, GameState& currentState, GameState nextLevel)
{
    static Font font;
    static bool fontLoaded = false;
    static bool enterPressed = false;
    static bool initialFrame = true; // Add this to track the first frame

    if (!fontLoaded) {
        font.openFromFile("arial.ttf");
        fontLoaded = true;
    }

    // Reset on state entry
    if (initialFrame) {
        enterPressed = Keyboard::isKeyPressed(Keyboard::Key::Enter); // Set to true if ENTER is currently pressed
        initialFrame = false;
    }

    window.clear(Color::Black);

    // Determine level title and controls based on the next level
    string levelTitle;
    vector<string> controlInstructions;

    switch (nextLevel) {
    case PLAYING:
        levelTitle = "Level 1: Fast Lines";
        controlInstructions = {
            "Controls:",
            "M - Return to Menu",
            "F1 - Open Settings"
        };
        break;
    case PLAYING2:
        levelTitle = "Level 2: Dark Maze";
        controlInstructions = {
            "Controls:",
            "W/A/S/D - Move",
            "Enter - Next level (when at exit)",
            "M - Return to Menu",
            "F1 - Open Settings"
        };
        break;
    case PLAYING3:
        levelTitle = "Level 3: A Silent Drive";
        controlInstructions = {
            "Controls:",
            "A/D - Steer",
            "W/S - Speed up/slow down",
            "R - Restart (when game over)",
            "M - Return to Menu",
            "F1 - Open Settings"
        };
        break;
    default:
        levelTitle = "Unknown Level";
        controlInstructions = { "Press ENTER to continue" };
        break;
    }

    // Draw level title
    Text title(font, levelTitle, 72);
    title.setStyle(Text::Bold);
    title.setFillColor(Color::Cyan);
    auto titleBounds = title.getLocalBounds();
    title.setOrigin(Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
    title.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 3.f));
    window.draw(title);

    // Draw control instructions
    for (size_t i = 0; i < controlInstructions.size(); ++i) {
        Text controlText(font, controlInstructions[i], 36);
        controlText.setFillColor(i == 0 ? Color::Yellow : Color::White);
        if (i == 0) controlText.setStyle(Text::Bold);

        auto bounds = controlText.getLocalBounds();
        controlText.setOrigin(Vector2f(bounds.size.x / 2.f, 0));
        controlText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f + i * 45.f));
        window.draw(controlText);
    }

    // Draw continue instruction
    Text continueText(font, "Press ENTER to start level", 42);
    continueText.setStyle(Text::Bold);
    continueText.setFillColor(Color::Green);
    auto continueBounds = continueText.getLocalBounds();
    continueText.setOrigin(Vector2f(continueBounds.size.x / 2.f, continueBounds.size.y / 2.f));
    continueText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y * 0.8f));
    window.draw(continueText);

    // window.display();

    // Handle input
    if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
        if (!enterPressed) {
            currentState = nextLevel;
            enterPressed = true;
            initialFrame = true; // Reset for next time this state is entered
        }
    }
    else {
        enterPressed = false;
    }

    // Handle menu and settings navigation
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        currentState = MENU;
        initialFrame = true; // Reset for next time this state is entered
    }
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        extern GameState previousState;
        previousState = currentState;
        currentState = SETTINGS;
        initialFrame = true; // Reset for next time this state is entered
    }
}