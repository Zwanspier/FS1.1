#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "Maze.h"
#include "SettingsState.h"

using namespace sf;
using namespace std;

// Handles the main game loop and rendering for the maze game mode.
void handlePlayingState2(RenderWindow& window, bool& running, GameState& state);