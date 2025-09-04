#include "Playingstate2.h"

void handlePlayingState2(RenderWindow& window, bool& running, GameState& state)
{
	window.clear(Color::Green);
	window.display();
	
	if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
		state = MENU;
	}
}
