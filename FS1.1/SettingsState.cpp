#include "SettingsState.h"

void handleSettingsState(RenderWindow& window, bool& running, GameState& state)
{
	static int selected = 0;
	static bool vsyncEnabled = false;
	static int frameratelimit = 60;
	static float gamma = 1.0f;
	static bool upPressed = false, downPressed = false, leftPressed = false, rightPressed = false, enterPressed = false;
	static bool mouseLeftPressed = false;
	static bool mouseRightPressed = false;

	const vector<string> options = { "VSync: ", "Framerate Limit: ", "Back" };

	static Font font;
	static bool fontLoaded = false;
	if (!fontLoaded) {
		font.openFromFile("arial.ttf");
		fontLoaded = true;
	}

	window.clear(Color::Black);

	// Store text objects for mouse interaction
	vector<Text> textObjects;

	for (size_t i = 0; i < options.size(); ++i) {
		Text text(font, options[i], 40);
		text.setFillColor(i == selected ? Color::Yellow : Color::White);
		text.setPosition(Vector2f(100.f, 200.f + i * 60.f));

		if (i == 0) {
			text.setString(options[i] + (vsyncEnabled ? "On" : "Off"));
		}
		else if (i == 1) {
			text.setString(options[i] + to_string(frameratelimit));
		}
		
		textObjects.push_back(text);
		window.draw(text);
	}

	// Mouse hover detection
	Vector2i mousePosition = Mouse::getPosition(window);
	Vector2f mousePos = window.mapPixelToCoords(mousePosition);

	// Check if mouse is hovering over any menu item
	for (size_t i = 0; i < textObjects.size(); ++i) {
		if (textObjects[i].getGlobalBounds().contains(mousePos)) {
			selected = static_cast<int>(i);
			break;
		}
	}

	// Mouse click handling for both left and right buttons
	bool isMouseLeftButtonPressed = Mouse::isButtonPressed(Mouse::Button::Left);
	bool isMouseRightButtonPressed = Mouse::isButtonPressed(Mouse::Button::Right);

	// Only trigger on click, not hold
	if (isMouseLeftButtonPressed && !mouseLeftPressed) {
		mouseLeftPressed = true;

		if (selected == 0) {
			vsyncEnabled = !vsyncEnabled;
			window.setVerticalSyncEnabled(vsyncEnabled);
		}
		else if (selected == 1) {
			if (frameratelimit < 240) frameratelimit += 10;
			window.setFramerateLimit(frameratelimit);
		}
		else if (selected == 2) {
			state = MENU;
		}
	}
	else if (!isMouseLeftButtonPressed) {
		mouseLeftPressed = false;
	}

	// Optional: Right mouse button can also be used for decrease (mirrors left click on left half)
	if (isMouseRightButtonPressed && !mouseRightPressed) {
		mouseRightPressed = true;

		if (selected == 1) {
			if (frameratelimit > 0) frameratelimit -= 10;
			window.setFramerateLimit(frameratelimit);
		}
	}
	else if (!isMouseRightButtonPressed) {
		mouseRightPressed = false;
	}

	window.display();

	// Keyboard Navigation
	if (Keyboard::isKeyPressed(Keyboard::Key::W)) {
		if (!upPressed) {
			selected = (selected - 1 + options.size()) % options.size();
			upPressed = true;
		}
	}
	else upPressed = false;

	if (Keyboard::isKeyPressed(Keyboard::Key::S)) {
		if (!downPressed) {
			selected = (selected + 1) % options.size();
			downPressed = true;
		}
	}
	else downPressed = false;

	if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
		if (!leftPressed) {
			if (selected == 1 && frameratelimit > 0) frameratelimit -= 10;
			leftPressed = true;
		}
	}
	else leftPressed = false;

	if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
		if (!rightPressed) {
			if (selected == 1 && frameratelimit < 240) frameratelimit += 10;
			rightPressed = true;
		}
	}
	else rightPressed = false;

	if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
		if (!enterPressed) {
			if (selected == 0) {
				vsyncEnabled = !vsyncEnabled;
				window.setVerticalSyncEnabled(vsyncEnabled);
			}
			else if (selected == 1) {
				window.setFramerateLimit(frameratelimit);
			}
			else if (selected == 2) {
				state = MENU;
			}
			enterPressed = true;
		}
	} else enterPressed = false;

	if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
		state = MENU;
	}
}
