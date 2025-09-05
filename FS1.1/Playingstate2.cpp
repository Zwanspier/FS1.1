#include "Playingstate2.h"

void handlePlayingState2(RenderWindow& window, bool& running, GameState& state)
{
	extern GameState previousState;

	window.clear(Color::Blue);
	window.display();
	
	if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
		state = MENU;
	}
	if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
		previousState = PLAYING2;
		state = SETTINGS;
	}
}
