#include "PlayingState.h"
#include "SettingsState.h"
#include <random>
#include <set>
#include <map>
#include <string>

// Helper to get a string for a Keyboard::Key
string keyToString(Keyboard::Key key) {
    // Convert enum to underlying integer, then to char
    int keyValue = static_cast<int>(key);
    int aValue = static_cast<int>(Keyboard::Key::A);
    return string(1, static_cast<char>('A' + (keyValue - aValue)));
}

void handlePlayingState(RenderWindow& window, bool& running, GameState& state)
{
    static Font font;
    static bool fontLoaded = false;
    static Text scrollingText(font, "", 30); // Initialize with empty string
    static float textX = 0.0f;
    static float textX2 = 0.0f;
    static Clock clock;
    extern GameState previousState;
    extern int framerateOptions[];
    extern int framerateIndex;

    // --- Random key logic ---
    static bool keyChosen = false;
    static Keyboard::Key randomKey = Keyboard::Key::Unknown;
    static bool keyPressed = false;

    // List of keys to avoid (already in use)
    static set<Keyboard::Key> reservedKeys = {
        Keyboard::Key::M, Keyboard::Key::F1, Keyboard::Key::Escape
        // Add any other keys you use for menu/settings/etc.
    };

    // List of candidate keys (A-Z, except reserved)
    static vector<Keyboard::Key> candidateKeys;
    if (candidateKeys.empty()) {
        // Use underlying integer values for comparison
        int aValue = static_cast<int>(Keyboard::Key::A);
        int zValue = static_cast<int>(Keyboard::Key::Z);

        for (int k = aValue; k <= zValue; ++k) {
            Keyboard::Key currentKey = static_cast<Keyboard::Key>(k);
            if (reservedKeys.count(currentKey) == 0)
                candidateKeys.push_back(currentKey);
        }
    }

    if (!keyChosen) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, static_cast<int>(candidateKeys.size()) - 1);
        randomKey = candidateKeys[dis(gen)];
        keyChosen = true;
    }
    // --- End random key logic ---

    if (!fontLoaded) {
        font.openFromFile("arial.ttf");
        scrollingText.setFillColor(Color::White);
        fontLoaded = true;
        textX = 0.0f;
        textX2 = 0.0f;
        clock.restart();
    }

    // Update the scrolling text to use the random key
    string scrollMsg = "NextLevel = ";
    scrollMsg += keyToString(randomKey);
    scrollMsg += " ";
    scrollingText.setString(scrollMsg);

    float deltaTime = clock.restart().asSeconds();
    int framerate = framerateOptions[framerateIndex];
    float baseSpeed = 1000.0f;
    float speed;

    // Adjust speed based on framerate setting.
    if (framerate > 0)
        speed = baseSpeed * (100.0f / framerate);
    else
        speed = baseSpeed * 2.0f;

    float textWidth = scrollingText.getLocalBounds().size.x;
    float textHeight = font.getLineSpacing(scrollingText.getCharacterSize());

    int numLines = static_cast<int>(window.getSize().y / textHeight) + 1;

    // Update base positions for scrolling.
    textX -= speed * deltaTime;
    if (textX + textWidth < 0) {
        textX += textWidth;
    }
    textX2 += speed * deltaTime;
    if (textX2 < textWidth) {
        textX2 -= textWidth;
    }

    window.clear(Color::Black);
    // Draw scrolling text lines, alternating direction.
    for (int line = 0; line < numLines; ++line) {
        float y = line * textHeight;
        bool leftToRight = (line % 2 == 1);

        float startX = leftToRight ? textX2 : textX;
        for (float x = startX; x < window.getSize().x + textWidth; x += textWidth) {
            scrollingText.setPosition(Vector2f(x, y));
            window.draw(scrollingText);
        }
    }
    // window.display();

    // Handle state transitions.
    if (Keyboard::isKeyPressed(randomKey)) {
        if (!keyPressed) {
            state = PRELEVEL2; // Go to pre-level screen before level 2
            keyChosen = false; // So a new key is chosen next time
            keyPressed = true;
        }
    }
    else {
        keyPressed = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        state = MENU;
        keyChosen = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        previousState = state;
        state = SETTINGS;
        keyChosen = false;
    }
}