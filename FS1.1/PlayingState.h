#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "SettingsState.h"
#include <random>
#include <set>
#include <map>
#include <string>

using namespace sf;
using namespace std;

// Handles the main game loop and rendering for the scrolling text state.
void handlePlayingState(RenderWindow& window, bool& running, GameState& state);