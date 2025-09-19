#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"
#include <SFML/Audio.hpp>
#include <random>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace sf;
using namespace std;

void handlePlayingState3(RenderWindow& window, bool& running, GameState& state);