#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"

using namespace sf;

//=== INTRODUCTION STATE HANDLER DECLARATION ===
// Handles the initial game introduction screen that explains the game's purpose
// Shows game overview, narrative context, and provides smooth entry to main menu
// Features: Animated text presentation, thematic background, continue prompt
void handleIntroductionState(RenderWindow& window, bool& running, GameState& state);