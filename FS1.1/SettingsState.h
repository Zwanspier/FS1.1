#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"

using namespace sf;
using namespace std;

// Forward declaration
class SpeedrunTimer;

// Extern declarations for global settings variables.
extern int framerateIndex;
extern int framerateOptions[];
extern bool vsyncEnabled;
extern float gamma; // Fake gamma value for wall brightness
extern int resolutionIndex; // Now used for maze size instead of actual resolution
extern vector<Vector2u> resolutionOptions; // Now represents maze size options
extern bool mazeNeedsRegeneration;
extern bool settingsChanged;
extern float musicVolume; // Add music volume setting (0.0f to 100.0f)
extern bool speedrunModeEnabled; // Enable/disable speedrun timer
extern bool speedrunTimerVisible; // Show/hide speedrun timer display
extern SpeedrunTimer speedrunTimer; // Global speedrun timer instance

// Handles the settings menu UI and logic.
void handleSettingsState(RenderWindow& window, bool& running, GameState& state);
void applySettings(RenderWindow& window);