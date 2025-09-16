#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"

using namespace sf;
using namespace std;

void handlePreLevelState(RenderWindow& window, bool& running, GameState& currentState, GameState nextLevel);