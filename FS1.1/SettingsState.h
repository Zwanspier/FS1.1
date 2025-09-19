#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"

using namespace sf;
using namespace std;

//=== GLOBAL SETTINGS DECLARATIONS ===
// External declarations for global settings variables
// These variables are defined in SettingsState.cpp and used throughout the application

//=== PERFORMANCE SETTINGS ===
extern int framerateIndex;              // Index into framerateOptions array
extern int framerateOptions[];          // Available framerate options for text speed calculation
extern bool vsyncEnabled;               // Vertical sync setting for smooth rendering

//=== VISUAL SETTINGS ===
extern float gamma;                     // Fake gamma value controlling wall brightness in maze (0.0f-2.0f)

//=== MAZE CONFIGURATION SETTINGS ===
extern int resolutionIndex;             // Index for maze size selection (repurposed from resolution)
extern vector<Vector2u> resolutionOptions;  // Available maze size options (width x height in cells)
extern bool mazeNeedsRegeneration;      // Flag indicating maze should be regenerated

//=== SYSTEM STATE TRACKING ===
extern bool settingsChanged;            // Flag indicating settings have been modified

//=== AUDIO SETTINGS ===
extern float musicVolume;               // Music and sound volume setting (0.0f to 100.0f)

//=== FUNCTION DECLARATIONS ===

// Main settings menu handler - processes input and renders settings interface
// Parameters:
//   window - SFML render window for drawing UI
//   running - application running state (for potential exit conditions)
//   state - current game state (modified when exiting settings)
void handleSettingsState(RenderWindow& window, bool& running, GameState& state);

// Applies all pending settings changes to the application
// Parameters:
//   window - SFML render window for applying window-related settings
// Note: This function handles VSync, maze regeneration, and other system changes
void applySettings(RenderWindow& window);