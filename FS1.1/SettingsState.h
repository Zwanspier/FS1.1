#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"

using namespace sf;
using namespace std;

// Extern declarations for global settings variables.
extern int framerateIndex;
extern int framerateOptions[];
extern float gamma; // Fake gamma value for wall brightness
extern int resolutionIndex;
extern std::vector<sf::Vector2u> resolutionOptions;
extern bool mazeNeedsRegeneration;

// Handles the settings menu UI and logic.
void handleSettingsState(RenderWindow& window, bool& running, GameState& state);

