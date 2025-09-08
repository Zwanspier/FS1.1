#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"

using namespace sf;
using namespace std;

// Handles the main game loop and rendering for the scrolling text state.
void handlePlayingState(RenderWindow& window, bool& running, GameState& state);