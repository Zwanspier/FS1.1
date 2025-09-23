#include "PlayingState.h"

//=== UTILITY FUNCTIONS ===

// Converts a Keyboard::Key enum to its corresponding character representation
// Used to display which key the player needs to press to advance
string keyToString(Keyboard::Key key) {
    // Convert enum to underlying integer, then calculate offset from 'A'
    int keyValue = static_cast<int>(key);
    int aValue = static_cast<int>(Keyboard::Key::A);
    return string(1, static_cast<char>('A' + (keyValue - aValue)));
}

//=== MAIN GAME LOOP FUNCTION ===
// Handles all logic and rendering for PlayingState (Level 1: Speeding Lines)
void handlePlayingState(RenderWindow& window, bool& running, GameState& state)
{
    //=== PERSISTENT STATE VARIABLES ===
    // Static variables maintain state between function calls
    
    // Font system
    static Font font;                    // Text rendering font
    static bool fontLoaded = false;      // Font loading status
    
    // Scrolling text system
    static Text scrollingText(font, "", 30);  // Main text object for scrolling display
    static float textX = 0.0f;               // Primary horizontal scroll position
    static float textX2 = 0.0f;              // Secondary position for seamless scrolling
    static Clock clock;                      // Frame timing system
    
    // Input state tracking for proper edge detection
    static bool mPressed = false;            // M key state
    static bool f1Pressed = false;           // F1 key state
    static bool escPressed = false;          // ESC key state (NEW)
    
    // External references to global game state
    extern GameState previousState;    // For returning from settings menu
    extern int framerateOptions[];     // Available framerate settings
    extern int framerateIndex;         // Current framerate selection
    
    //=== RANDOM KEY SELECTION SYSTEM ===
    // Dynamically chooses which key player must press to advance
    static bool keyChosen = false;              // Has a key been selected this session?
    static Keyboard::Key randomKey = Keyboard::Key::Unknown;  // Currently active key
    static bool keyPressed = false;             // Input state tracking
    
    // Keys that are reserved for navigation and cannot be used for progression
    static set<Keyboard::Key> reservedKeys = {
        Keyboard::Key::M,        // Return to menu
        Keyboard::Key::F1,       // Open settings
        Keyboard::Key::Escape    // Return to pre-level screen (NEW)
        // Additional reserved keys can be added here as needed
    };
    
    // Available keys for random selection (A-Z minus reserved keys)
    static vector<Keyboard::Key> candidateKeys;
    if (candidateKeys.empty()) {
        // Generate list of usable keys from A to Z
        int aValue = static_cast<int>(Keyboard::Key::A);
        int zValue = static_cast<int>(Keyboard::Key::Z);
        
        for (int k = aValue; k <= zValue; ++k) {
            Keyboard::Key currentKey = static_cast<Keyboard::Key>(k);
            // Only include keys not in reserved set
            if (reservedKeys.count(currentKey) == 0)
                candidateKeys.push_back(currentKey);
        }
    }
    
    // Select random key if none chosen yet
    if (!keyChosen) {
        random_device rd;                    // Hardware random number generator
        mt19937 gen(rd());                   // Mersenne Twister generator
        uniform_int_distribution<> dis(0, static_cast<int>(candidateKeys.size()) - 1);
        randomKey = candidateKeys[dis(gen)]; // Select random valid key
        keyChosen = true;                    // Mark selection as complete
    }
    
    //=== FONT AND TEXT INITIALIZATION ===
    // One-time setup for text rendering system
    if (!fontLoaded) {
        font.openFromFile("arial.ttf");           // Load standard font
        scrollingText.setFillColor(Color::White); // Set text color to white
        fontLoaded = true;                        // Mark font as loaded
        textX = 0.0f;                            // Initialize scroll positions
        textX2 = 0.0f;
        clock.restart();                         // Begin frame timing
    }
    
    //=== TEXT CONTENT GENERATION ===
    // Create dynamic message showing which key to press
    string scrollMsg = "NextLevel = ";
    scrollMsg += keyToString(randomKey);  // Convert key enum to displayable character
    scrollMsg += " ";                     // Add spacing for visual separation
    scrollingText.setString(scrollMsg);   // Apply message to text object
    
    //=== FRAME TIMING AND SPEED CALCULATION ===
    float deltaTime = clock.restart().asSeconds();  // Time since last frame
    int framerate = framerateOptions[framerateIndex]; // Current FPS setting
    float baseSpeed = 1000.0f;                       // Base scrolling speed
    float speed;
    
    // Adjust scrolling speed based on framerate setting
    // Higher framerate = slower speed for consistent visual movement
    if (framerate > 0)
        speed = baseSpeed / (100.0f / framerate);
    else
        speed = baseSpeed * 2.0f;  // Special case for unlimited framerate
    
    //=== TEXT MEASUREMENT FOR SCROLLING ===
    float textWidth = scrollingText.getLocalBounds().size.x;  // Width of text string
    float textHeight = font.getLineSpacing(scrollingText.getCharacterSize()); // Line height
    
    // Calculate how many text lines fit on screen vertically
    int numLines = static_cast<int>(window.getSize().y / textHeight) + 1;
    
    //=== SCROLLING ANIMATION SYSTEM ===
    // Update horizontal positions for continuous scrolling effect
    
    // Primary scroll position (left-to-right movement)
    textX -= speed * deltaTime;
    if (textX + textWidth < 0) {
        textX += textWidth;  // Reset position for seamless loop
    }
    
    // Secondary scroll position (right-to-left movement)
    textX2 += speed * deltaTime;
    if (textX2 < textWidth) {
        textX2 -= textWidth;  // Reset position for seamless loop
    }
    
    //=== RENDERING PIPELINE ===
    window.clear(Color::Black);  // Clear screen with black background
    
    // Draw scrolling text pattern across entire screen
    for (int line = 0; line < numLines; ++line) {
        float y = line * textHeight;              // Vertical position for this line
        bool leftToRight = (line % 2 == 1);       // Alternate scroll direction per line
        
        // Choose scroll position based on direction
        float startX = leftToRight ? textX2 : textX;
        
        // Draw repeated text instances across screen width
        for (float x = startX; x < window.getSize().x + textWidth; x += textWidth) {
            scrollingText.setPosition(Vector2f(x, y));
            window.draw(scrollingText);
        }
    }
    
    //=== INPUT HANDLING SYSTEM ===
    
    // Check for level progression key press
    if (Keyboard::isKeyPressed(randomKey)) {
        if (!keyPressed) {  // Edge detection to prevent key repeat
            state = PRELEVEL2;    // Advance to pre-level screen for level 2
            keyChosen = false;    // Reset key selection for next session
            keyPressed = true;    // Mark key as pressed
        }
    }
    else {
        keyPressed = false;  // Reset press state when key released
    }
    
    // NEW: Return to pre-level screen (ESC key)
    if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
        if (!escPressed) {  // Edge detection to prevent key repeat
            state = PRELEVEL1;     // Return to Level 1 pre-level screen
            keyChosen = false;     // Reset key selection for restart
            escPressed = true;     // Mark key as pressed
        }
    }
    else {
        escPressed = false;  // Reset when key released
    }
    
    // Return to main menu (M key)
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        if (!mPressed) {  // Edge detection to prevent key repeat
            state = MENU;        // Return to main menu
            keyChosen = false;   // Reset key selection
            mPressed = true;     // Mark key as pressed
        }
    }
    else {
        mPressed = false;  // Reset when key released
    }
    
    // Open settings menu (F1 key)
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        if (!f1Pressed) {  // Edge detection to prevent key repeat
            previousState = state;  // Store current state for return
            state = SETTINGS;       // Open settings menu
            keyChosen = false;      // Reset key selection
            f1Pressed = true;       // Mark key as pressed
        }
    }
    else {
        f1Pressed = false;  // Reset when key released
    }
}