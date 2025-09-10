#include "PlayingState3.h"


void handlePlayingState3(RenderWindow& window, bool& running, GameState& state)
{
    window.clear(Color::Magenta);
    Font font("arial.ttf");

    // Handle menu and settings navigation.
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        state = MENU;
    }
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        extern GameState previousState;
        previousState = PLAYING3;
        state = SETTINGS;
    }
    if (Keyboard::isKeyPressed(Keyboard::Key::H)) {
        window.clear(Color::Green);
    }
 	// window.display();
    
}
