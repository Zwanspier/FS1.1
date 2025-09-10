#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"

using namespace sf;
using namespace std;

// Extern declarations for global settings variables.
extern int framerateIndex;
extern int framerateOptions[];
extern bool vsyncEnabled;
extern float gamma; // Fake gamma value for wall brightness
extern int resolutionIndex; // Now used for maze size instead of actual resolution
extern std::vector<sf::Vector2u> resolutionOptions; // Now represents maze size options
extern bool mazeNeedsRegeneration;
extern bool settingsChanged;

// Handles the settings menu UI and logic.
void handleSettingsState(RenderWindow& window, bool& running, GameState& state);
void applySettings(RenderWindow& window);

