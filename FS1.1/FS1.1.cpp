#include <SFML/Graphics.hpp>
#include <iostream>
#include "PlayingState.h"
#include "Playingstate2.h"
#include "SettingsState.h"
#include "GameState.h"

using namespace sf;
using namespace std;

// Stores the previous game state for returning from settings.
GameState previousState = MENU;

int main()
{
    // Create the main application window in fullscreen mode.
    RenderWindow window(VideoMode::getDesktopMode(), "/Settings Puzzles/", Style::Default, State::Fullscreen);

    static bool mouseLeftPressed = false;

    // Load the font to be used for menu text.
    Font font("arial.ttf");

    // Define the menu options.
    const vector<string> options = { "Start", "Settings", "Exit" };
    int selected = 0; // Index of the currently selected menu option.
    GameState state = MENU; // Current state of the game.

    bool running = true; // Controls the main game loop.

    // Prepare the menu text objects for each option.
    vector<Text> menuTexts;
    for (size_t i = 0; i < options.size(); ++i) {
        Text t(font, options[i], 50);
        t.setStyle(Text::Italic | Text::Bold);
        t.setFillColor(Color::White);
        auto bounds = t.getLocalBounds();
        t.setOrigin(Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        t.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f + i * 80.f));
        menuTexts.push_back(t);
    }

    // Create and configure the title text.
    Text title(font, "/Setting Puzzles/", 100);
    title.setStyle(Text::Bold | Text::Underlined);
    title.setFillColor(Color::Blue);
    auto titleBounds = title.getLocalBounds();
    title.setOrigin(Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
    title.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 4.f));

    vector<string> controlLines = {
        "Controls:",
        "W/S - Navigate menu",
        "Enter/Left Click - Select",
        "W/A/S/D - Move in maze",
        "M - Return to Menu",
        "F1 - Open Settings",
    };
    vector<Text> controlTexts;
    for (size_t i = 0; i < controlLines.size(); ++i) {
        Text t(font, controlLines[i], 28);
        t.setFillColor(Color::White);
        t.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 1.5f + i * 36.f));
        t.setOrigin(Vector2f(t.getLocalBounds().size.x / 2.f, 0));
        controlTexts.push_back(t);
    }

    // Create a red circle to indicate the selected menu option.
    CircleShape selector(20.f);
    selector.setFillColor(Color::Red);

    // Track key press states to handle single key events.
    bool wPressed = false, sPressed = false, enterPressed = false;

    // Main application loop.
    while (window.isOpen())
    {
        optional<Event> event;
        // Process all window events.
        while ((event = window.pollEvent()))
        {
            if (event->is<Event::Closed>())
                window.close(); // Close window if requested.
        }

        // Handle menu navigation and selection.
        if (state == MENU) {

            Vector2i mousePosition = Mouse::getPosition(window);
            Vector2f mousePos = window.mapPixelToCoords(mousePosition);

            // Mouse hover for menu selection.
            for (size_t i = 0; i < menuTexts.size(); ++i) {
                if (menuTexts[i].getGlobalBounds().contains(mousePos)) {
                    selected = static_cast<int>(i);
                    break;
                }
            }

            // Mouse click for menu selection.
            bool isMouseLeftButtonPressed = Mouse::isButtonPressed(Mouse::Button::Left);
            if (isMouseLeftButtonPressed && !mouseLeftPressed) {
                mouseLeftPressed = true;
                switch (selected) {
                case 0: state = PLAYING; break;   // Start game
                case 1: state = SETTINGS; break;  // Open settings
                case 2: state = EXIT; window.close(); break; // Exit game
                }
            }
            else if (!isMouseLeftButtonPressed) {
                mouseLeftPressed = false;
            }

            // Keyboard navigation for menu.
            if (Keyboard::isKeyPressed(Keyboard::Key::W)) {
                if (!wPressed) {
                    selected = (selected - 1 + options.size()) % options.size();
                    wPressed = true;
                }
            }
            else {
                wPressed = false;
            }
            if (Keyboard::isKeyPressed(Keyboard::Key::S)) {
                if (!sPressed) {
                    selected = (selected + 1) % options.size();
                    sPressed = true;
                }
            }
            else {
                sPressed = false;
            }

            // Keyboard selection for menu.
            if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
                if (!enterPressed) {
                    switch (selected) {
                    case 0: state = PLAYING; break;   // Start game
                    case 1: state = SETTINGS; break;  // Open settings
                    case 2: state = EXIT; window.close(); break; // Exit game
                    }
                    enterPressed = true;
                }
            }
            else {
                enterPressed = false;
            }
        }

        window.clear(); // Clear the window for new frame.

        // Render the current state.
        if (state == MENU) {
            window.draw(title);
            for (size_t i = 0; i < menuTexts.size(); ++i) {
                window.draw(menuTexts[i]);
            }
            // Draw the selector next to the selected option.
            Vector2f pos = menuTexts[selected].getPosition();
            selector.setPosition(Vector2f(pos.x - 200.f, pos.y - 8.f));
            window.draw(selector);

            // Draw control instructions below the menu
            for (const auto& t : controlTexts) {
                window.draw(t);
            }
        }
        else if (state == PLAYING) {
            // Delegate to the scrolling text state handler.
            handlePlayingState(window, running, state);
        }
        else if (state == PLAYING2) {
            // Delegate to the maze game state handler.
            handlePlayingState2(window, running, state);
        }
        else if (state == SETTINGS) {
            // Delegate to the settings state handler.
            handleSettingsState(window, running, state);
        }
        window.display(); // Display the rendered frame.
    }
    return 0;
}
