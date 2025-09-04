#include <SFML/Graphics.hpp>
#include <iostream>
#include "PlayingState.h"
#include "Playingstate2.h"
#include "SettingsState.h"
#include "GameState.h"

using namespace sf;
using namespace std;


int main()
{
    // Create the main application window with a fixed size and title
    RenderWindow window(VideoMode({ 1280, 1024 }), "The Menu Game");

	static bool mouseLeftPressed = false;

    // Load the font to be used for menu text
    Font font("arial.ttf");

    // Define the menu options
    const vector<string> options = { "Start", "Settings", "Exit" };
    int selected = 0; // Index of the currently selected menu option
    GameState state = MENU; // Current state of the game

    bool running = true; // Controls the main game loop

    // Prepare the menu text objects for each option
    vector<Text> menuTexts;
    for (size_t i = 0; i < options.size(); ++i) {
        Text t(font, options[i], 50); // Create text with font, string, and size
        t.setStyle(Text::Italic | Text::Bold);
        t.setFillColor(Color::White);
        auto bounds = t.getLocalBounds();
        t.setOrigin(Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f)); // Center origin
        t.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f + i * 80.f)); // Position each option
        menuTexts.push_back(t);
    }

    // Create and configure the title text
    Text title(font, "The Menu Game", 100);
    title.setStyle(Text::Bold | Text::Underlined);
    title.setFillColor(Color::Blue);
    auto titleBounds = title.getLocalBounds();
    title.setOrigin(Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
    title.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 4.f));

    // Create a red circle to indicate the selected menu option
    CircleShape selector(20.f);
    selector.setFillColor(Color::Red);

    // Track key press states to handle single key events
    bool wPressed = false, sPressed = false, enterPressed = false;

    // Main application loop
    while (window.isOpen())
    {
        optional<Event> event;
        // Process all window events
        while ((event = window.pollEvent()))
        {
            if (event->is<Event::Closed>())
                window.close(); // Close window if requested
        }

        // Handle menu navigation and selection
        if (state == MENU) {

			Vector2i mousePosition = Mouse::getPosition(window);
			Vector2f mousePos = window.mapPixelToCoords(mousePosition);

            for (size_t i = 0; i < menuTexts.size(); ++i) {
                if (menuTexts[i].getGlobalBounds().contains(mousePos)) {
                    selected = static_cast<int>(i);
                    break;
                }
			}

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


            // Move selection up
            if (Keyboard::isKeyPressed(Keyboard::Key::W)) {
                if (!wPressed) {
                    selected = (selected - 1 + options.size()) % options.size();
                    wPressed = true;
                }
            }
            else {
                wPressed = false;
            }
            // Move selection down
            if (Keyboard::isKeyPressed(Keyboard::Key::S)) {
                if (!sPressed) {
                    selected = (selected + 1) % options.size();
                    sPressed = true;
                }
            }
            else {
                sPressed = false;
            }

            // Handle menu option selection
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

        window.clear(); // Clear the window for new frame

        // Render the current state
        if (state == MENU) {
            window.draw(title); // Draw the title
            for (size_t i = 0; i < menuTexts.size(); ++i) {
                window.draw(menuTexts[i]); // Draw each menu option
            }
            // Draw the selector next to the selected option
            Vector2f pos = menuTexts[selected].getPosition();
            selector.setPosition(Vector2f(pos.x - 200.f, pos.y - 8.f));
            window.draw(selector);
        }
        else if (state == PLAYING) {
            // Delegate to the playing state handler
            handlePlayingState(window, running, state);
        }
        else if (state == PLAYING2) {
            // Delegate to the second playing state handler
            handlePlayingState2(window, running, state);
		}
        else if (state == SETTINGS) {
            // Delegate to the settings state handler
            handleSettingsState(window, running, state);
        }

        window.display(); // Display the rendered frame
    }
    return 0;
}
