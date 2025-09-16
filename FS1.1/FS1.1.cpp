#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "PlayingState.h"
#include "Playingstate2.h"
#include "PlayingState3.h"
#include "PreLevelState.h"
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

    // Create permanent F1 settings hint text
    Text settingsHint(font, "F1 - Settings", 24);
    settingsHint.setFillColor(Color::White);
    settingsHint.setOutlineColor(Color::Black);
    settingsHint.setOutlineThickness(5.f);
    auto hintBounds = settingsHint.getLocalBounds();
    settingsHint.setOrigin(Vector2f(hintBounds.size.x, 0)); // Right-aligned
    settingsHint.setPosition(Vector2f(window.getSize().x - 20.f, 20.f)); // Top-right corner with 20px padding

    // Create a red circle to indicate the selected menu option.
    CircleShape selector(20.f);
    selector.setFillColor(Color::Red);

    // Track key press states to handle single key events.
    bool wPressed = false, sPressed = false, enterPressed = false;

    // MOVE MUSIC VARIABLES OUTSIDE THE LOOP
    Music music;
    std::string currentSong;

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
                case 0: state = PRELEVEL1; break;   // Start game (go to pre-level screen)
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
                    case 0: state = PRELEVEL1; break;   // Start game (go to pre-level screen)
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

        // Handle music changes based on game state
        std::string desiredSong;

        if (state == MENU) {
            desiredSong = "PiecebyPiece.mp3";
        } else if (state == PLAYING) {
            desiredSong = "PiecebyPiece2.mp3";
        } else if (state == PLAYING2) {
            desiredSong = "PiecebyPiece2.mp3";
        } else if (state == PLAYING3) {
            desiredSong = "PiecebyPiece2.mp3";
        } else {
            desiredSong = "PiecebyPiece.mp3"; // fallback or silence
        }

        if (desiredSong != currentSong) {
            if (music.getStatus() == Music::Status::Playing)
                music.stop();
            if (music.openFromFile(desiredSong)) {
                music.setLooping(true);
                music.setVolume(40.f); // or your preferred volume
                music.play();
                currentSong = desiredSong;
            } else {
                std::cerr << "Failed to load " << desiredSong << std::endl;
                currentSong.clear();
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
        }
        else if (state == PRELEVEL1) {
            // Show pre-level screen for level 1
            handlePreLevelState(window, running, state, PLAYING);
        }
        else if (state == PLAYING) {
            // Delegate to the scrolling text state handler.
            handlePlayingState(window, running, state);
        }
        else if (state == PRELEVEL2) {
            // Show pre-level screen for level 2
            handlePreLevelState(window, running, state, PLAYING2);
        }
        else if (state == PLAYING2) {
            // Delegate to the maze game state handler.
            handlePlayingState2(window, running, state);
        }
        else if (state == PRELEVEL3) {
            // Show pre-level screen for level 3
            handlePreLevelState(window, running, state, PLAYING3);
        }
        else if (state == PLAYING3) {
            handlePlayingState3(window, running, state);
        }
        else if (state == SETTINGS) {
            // Delegate to the settings state handler.
            handleSettingsState(window, running, state);
        }

        // Draw the permanent settings hint on all states except settings
        if (state != SETTINGS) {
            window.draw(settingsHint);
        }

        window.display(); // Single display call per frame
    }
    return 0;
}