#include "PlayingState.h"

void handlePlayingState(RenderWindow& window, bool& running, GameState& state)
{
	static Font font;
	static bool fontLoaded = false;
	static Text scrollingText(font, "NextLevel ", 30);
	static float textX = 0.0f;
	static float textX2 = 0.0f;
	static Clock clock;

	if (!fontLoaded) {
		font.openFromFile("arial.ttf");
		scrollingText.setFillColor(Color::White);
		textX = static_cast<float>(window.getSize().x);
		textX2 = 0.0f;
		fontLoaded = true;
		clock.restart();
	}

	float deltaTime = clock.restart().asSeconds();
	float speed = 400.0f;

	float textWidth = scrollingText.getLocalBounds().size.x;
	float textHeight = font.getLineSpacing(scrollingText.getCharacterSize());

	int numLines = static_cast<int>(window.getSize().y / textHeight) + 1;

	// Update base positions for scrolling
	textX -= speed * deltaTime;
	if (textX + textWidth < 0) {
		textX += textWidth;
	}
	textX2 += speed * deltaTime;
	if (textX2 < textWidth) {
		textX2 -= textWidth;
	}

	window.clear(Color::Black);
	for (int line = 0; line < numLines; ++line) {
		float y = line * textHeight;
		bool leftToRight = (line % 2 == 1);

		float startX = leftToRight ? textX2 : textX;
		for (float x = startX; x < window.getSize().x + textWidth; x += textWidth) {
			scrollingText.setPosition(Vector2f(x, y));
			window.draw(scrollingText);
		}
	}
	window.display();
	if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
		state = PLAYING2;
	}

	if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
		state = MENU;
	}
}
