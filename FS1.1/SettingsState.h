#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"

using namespace sf;
using namespace std;

extern int framerateIndex;
extern int framerateOptions[];

void handleSettingsState(RenderWindow& window, bool& running, GameState& state);

