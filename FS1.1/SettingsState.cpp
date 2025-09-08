#include "SettingsState.h"

using namespace sf;
using namespace std;

// Global settings variables.
int framerateIndex = 2; // Default to 60 FPS (index 2)
int framerateOptions[] = { 30, 60, 120, 144, 240 };
bool vsyncEnabled = true;
float gamma = 0.0f; // Start with black walls (invisible)
int resolutionIndex = 2;
std::vector<sf::Vector2u> resolutionOptions = {
    {1280, 720},
    {1600, 900},
    {1920, 1080},
    {2560, 1440}
};
bool mazeNeedsRegeneration = false; // Add this line

// Function to apply all settings changes
void applySettings(RenderWindow& window) {
    static int lastResolutionIndex = resolutionIndex;

    // Apply VSync setting
    window.setVerticalSyncEnabled(vsyncEnabled);
    
    // Apply framerate limit
    if (framerateOptions[framerateIndex] > 0) {
        window.setFramerateLimit(framerateOptions[framerateIndex]);
    } else {
        window.setFramerateLimit(0); // No limit
    }
    
    // Apply resolution (recreate window with current settings)
    auto res = resolutionOptions[resolutionIndex];
    window.create(VideoMode({res.x, res.y}), "/Settings Puzzles/", Style::Default, State::Fullscreen);
    
    // Reapply VSync and framerate after window recreation
    window.setVerticalSyncEnabled(vsyncEnabled);
    if (framerateOptions[framerateIndex] > 0) {
        window.setFramerateLimit(framerateOptions[framerateIndex]);
    } else {
        window.setFramerateLimit(0);
    }

    // Check if resolution changed
    if (resolutionIndex != lastResolutionIndex) {
        mazeNeedsRegeneration = true; // Signal maze to regenerate
        lastResolutionIndex = resolutionIndex;
    }
}

// Handles the settings menu, including VSync, framerate, and fake gamma.
void handleSettingsState(RenderWindow& window, bool& running, GameState& state)
{
    static int selected = 0;
    static bool upPressed = false, downPressed = false, leftPressed = false, rightPressed = false, enterPressed = false;
    static bool mouseLeftPressed = false;
    static bool mouseRightPressed = false;

    extern GameState previousState;

    // Menu options: VSync, Framerate, Gamma, Resolution, Apply Changes, Back
    const vector<string> options = { "VSync: ", "Framerate Limit: ", "Gamma: ", "Resolution: ", "Apply Changes", "Back" };

    static Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        font.openFromFile("arial.ttf");
        fontLoaded = true;
    }

    window.clear(Color::Black);

    // Store text objects for mouse interaction
    vector<Text> textObjects;

    // Render each menu option, updating text for current values.
    for (size_t i = 0; i < options.size(); ++i) {
        Text text(font, options[i], 40);
        text.setFillColor(i == selected ? Color::Yellow : Color::White);
        text.setPosition(Vector2f(100.f, 200.f + i * 60.f));

        if (i == 0) {
            text.setString(options[i] + (vsyncEnabled ? "On" : "Off"));
        }
        else if (i == 1) {
            text.setString(options[i] + to_string(framerateOptions[framerateIndex]));
        }
        else if (i == 2) {
            // Display gamma as percentage (0.0 = 0%, 2.0 = 100%)
            int gammaPercent = static_cast<int>((gamma / 2.0f) * 100.0f);
            text.setString(options[i] + to_string(gammaPercent) + "%");
        }
        else if (i == 3) {
            text.setString(options[i] + to_string(resolutionOptions[resolutionIndex].x) + "x" + to_string(resolutionOptions[resolutionIndex].y));
        }
        // Options 4 (Apply Changes) and 5 (Back) use their default text
        
        textObjects.push_back(text);
        window.draw(text);
    }

    // Mouse hover detection for menu selection.
    Vector2i mousePosition = Mouse::getPosition(window);
    Vector2f mousePos = window.mapPixelToCoords(mousePosition);

    for (size_t i = 0; i < textObjects.size(); ++i) {
        if (textObjects[i].getGlobalBounds().contains(mousePos)) {
            selected = static_cast<int>(i);
            break;
        }
    }

    // Mouse click handling for menu actions.
    bool isMouseLeftButtonPressed = Mouse::isButtonPressed(Mouse::Button::Left);
    bool isMouseRightButtonPressed = Mouse::isButtonPressed(Mouse::Button::Right);

    if (isMouseLeftButtonPressed && !mouseLeftPressed) {
        mouseLeftPressed = true;

        if (selected == 0) {
            vsyncEnabled = !vsyncEnabled;
        }
        else if (selected == 1) {
            if (framerateIndex < static_cast<int>(size(framerateOptions)) - 1) framerateIndex++;
        }
        else if (selected == 2) {
            gamma += 0.1f;
            if (gamma > 2.0f) gamma = 2.0f;
        }
        else if (selected == 3) {
            resolutionIndex++;
            if (resolutionIndex >= static_cast<int>(size(resolutionOptions))) resolutionIndex = 0;
        }
        else if (selected == 4) {
            // Apply all settings changes
            applySettings(window);
        }
        else if (selected == 5) {
            state = MENU;
        }
    }
    else if (!isMouseLeftButtonPressed) {
        mouseLeftPressed = false;
    }

    if (isMouseRightButtonPressed && !mouseRightPressed) {
        mouseRightPressed = true;

        if (selected == 1) {
            if (framerateIndex > 0) framerateIndex--;
        }
        else if (selected == 2) {
            gamma -= 0.1f;
            if (gamma < 0.0f) gamma = 0.0f;
        }
        else if (selected == 3) {
            resolutionIndex--;
            if (resolutionIndex < 0) resolutionIndex = static_cast<int>(size(resolutionOptions)) - 1;
        }
    }
    else if (!isMouseRightButtonPressed) {
        mouseRightPressed = false;
    }

    window.display();

    // Keyboard navigation for menu.
    if (Keyboard::isKeyPressed(Keyboard::Key::W)) {
        if (!upPressed) {
            selected = (selected - 1 + options.size()) % options.size();
            upPressed = true;
        }
    }
    else upPressed = false;

    if (Keyboard::isKeyPressed(Keyboard::Key::S)) {
        if (!downPressed) {
            selected = (selected + 1) % options.size();
            downPressed = true;
        }
    }
    else downPressed = false;

    if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
        if (!leftPressed) {
            if (selected == 1 && framerateIndex > 0) framerateIndex--;
            else if (selected == 2) {
                gamma -= 0.1f;
                if (gamma < 0.0f) gamma = 0.0f;
            }
            else if (selected == 3) {
                resolutionIndex--;
                if (resolutionIndex < 0) resolutionIndex = static_cast<int>(size(resolutionOptions)) - 1;
            }
            leftPressed = true;
        }
    }
    else leftPressed = false;

    if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
        if (!rightPressed) {
            if (selected == 1 && framerateIndex < static_cast<int>(size(framerateOptions)) - 1) framerateIndex++;
            else if (selected == 2) {
                gamma += 0.1f;
                if (gamma > 2.0f) gamma = 2.0f;
            }
            else if (selected == 3) {
                resolutionIndex++;
                if (resolutionIndex >= static_cast<int>(size(resolutionOptions))) resolutionIndex = 0;
            }
            rightPressed = true;
        }
    }
    else rightPressed = false;

    if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
        if (!enterPressed) {
            if (selected == 0) {
                vsyncEnabled = !vsyncEnabled;
            }
            else if (selected == 4) {
                // Apply all settings changes
                applySettings(window);
            }
            else if (selected == 5) {
                state = previousState;
            }
            enterPressed = true;
        }
    } else enterPressed = false;

    if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
        state = previousState;
    }
}
