#include "PreLevelState.h"
#include "NavigationSounds.h"

//=== MAIN PRE-LEVEL STATE HANDLER ===
// Displays level introduction screen with controls and navigation options
// Serves as transition between levels and provides player orientation
void handlePreLevelState(RenderWindow& window, bool& running, GameState& currentState, GameState nextLevel)
{
    //=== PERSISTENT STATE VARIABLES ===
    // Static variables maintain state between function calls
    static Font font;              // Text rendering font
    static bool fontLoaded = false; // Font loading status flag
    
    // Input state tracking to prevent key repeat issues
    static bool enterPressed = false;  // ENTER key state
    static bool mPressed = false;      // M key (menu) state  
    static bool f1Pressed = false;     // F1 key (settings) state
    static bool initialFrame = true;   // First frame detection flag
    
    //=== FONT INITIALIZATION ===
    // One-time font loading for text rendering
    if (!fontLoaded) {
        font.openFromFile("arial.ttf");  // Load standard font
        fontLoaded = true;               // Mark as successfully loaded
    }
    
    //=== INPUT STATE INITIALIZATION ===
    // Reset input states on first frame to prevent carried-over key presses
    // This prevents immediate state transitions when entering from another level
    if (initialFrame) {
        enterPressed = Keyboard::isKeyPressed(Keyboard::Key::Enter); // Capture current state
        mPressed = Keyboard::isKeyPressed(Keyboard::Key::M);         // Capture current state
        f1Pressed = Keyboard::isKeyPressed(Keyboard::Key::F1);       // Capture current state
        initialFrame = false;  // Mark initialization as complete
    }
    
    //=== RENDERING SETUP ===
    window.clear(Color::Black);  // Clear screen with black background
    
    //=== LEVEL-SPECIFIC CONTENT GENERATION ===
    // Generate appropriate title and control instructions based on target level
    string levelTitle;                    // Display title for the level
    vector<string> controlInstructions;   // List of control instructions
    
    // Configure content based on next level destination
    switch (nextLevel) {
    case PLAYING:  // Level 1: Text scrolling game
        levelTitle = "Level 1: Speeding Lines";
        controlInstructions = {
            "Controls:",
            "Watch the screen and press the highlighted key",
            "",
            "",
            "Navigation:",
            "ESC - Return to this screen",     // NEW: ESC functionality
            "M - Return to Menu",
            "F1 - Open Settings"
        };
        break;
        
    case PLAYING2:  // Level 2: Maze navigation game
        levelTitle = "Level 2: Dark Maze";
        controlInstructions = {
            "Controls:",
            "W/A/S/D - Move through the maze",           // Movement controls
            "Find the exit to progress",                 // Objective
            "",
            "Navigation:",
            "ESC - Return to this screen",               // NEW: ESC functionality
            "Enter - Next level (when at exit)",        // Progression condition
            "M - Return to Menu",                        // Navigation
            "F1 - Open Settings"                         // Settings access
        };
        break;
        
    case PLAYING3:  // Level 3: Driving/racing game
        levelTitle = "Level 3: A Silent Drive";
        controlInstructions = {
            "Controls:",
            "A/D - Steer left and right",               // Steering controls
            "W/S - Speed up/slow down",                 // Speed controls
            "",
            "",
            "Navigation:",
            "ESC - Return to this screen",               // NEW: ESC functionality
            "R - Restart (when game over)",             // Restart mechanism
            "M - Return to Menu",                        // Navigation
            "F1 - Open Settings"                         // Settings access
        };
        break;
        
    default:  // Fallback for unknown levels
        levelTitle = "Unknown Level";
        controlInstructions = { 
            "Press ENTER to continue",
            "",
            "Navigation:",
            "ESC - Return to this screen",
            "M - Return to Menu",
            "F1 - Open Settings"
        };
        break;
    }
    
    //=== TITLE RENDERING ===
    // Create and display level title
    Text title(font, levelTitle, 72);
    title.setStyle(Text::Bold);              // Bold text style
    title.setFillColor(Color::Cyan);         // Cyan color for visibility
    
    // Center title horizontally and position in upper third of screen
    auto titleBounds = title.getLocalBounds();
    title.setOrigin(Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
    title.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 4.f));
    window.draw(title);
    
    //=== CONTROL INSTRUCTIONS RENDERING ===
    // Display control instruction list in center of screen
    float baseY = window.getSize().y / 2.f - (controlInstructions.size() * 22.5f);
    
    for (size_t i = 0; i < controlInstructions.size(); ++i) {
        Text controlText(font, controlInstructions[i], 28);
        
        // Style different types of lines with different colors and formatting
        if (controlInstructions[i] == "Controls:" || controlInstructions[i] == "Navigation:") {
            // Header lines
            controlText.setFillColor(Color::Yellow);
            controlText.setStyle(Text::Bold);
        } else if (controlInstructions[i].empty()) {
            // Skip empty lines (spacing)
            continue;
        } else if (controlInstructions[i].find("ESC") == 0) {
            // Highlight the new ESC functionality
            controlText.setFillColor(Color::Green);
            controlText.setStyle(Text::Bold);
        } else {
            // Regular instruction lines
            controlText.setFillColor(Color::White);
        }
        
        // Center text horizontally, stack vertically with spacing
        auto bounds = controlText.getLocalBounds();
        controlText.setOrigin(Vector2f(bounds.size.x / 2.f, 0));
        controlText.setPosition(Vector2f(window.getSize().x / 2.f, baseY + i * 35.f));
        window.draw(controlText);
    }
    
    //=== CONTINUATION PROMPT RENDERING ===
    // Display instruction for proceeding to the actual level
    Text continueText(font, "Press ENTER to start level", 42);
    continueText.setStyle(Text::Bold);         // Bold for emphasis
    continueText.setFillColor(Color::Green);   // Green color indicates positive action
    
    // Center in lower portion of screen
    auto continueBounds = continueText.getLocalBounds();
    continueText.setOrigin(Vector2f(continueBounds.size.x / 2.f, continueBounds.size.y / 2.f));
    continueText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y * 0.85f));
    window.draw(continueText);
    
    //=== INPUT HANDLING SYSTEM ===
    // Process user input with sound effects and state transitions
    
    // Level start input (ENTER key)
    if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
        if (!enterPressed) {  // Edge detection to prevent key repeat
            navSounds.playSelect();     // Play selection sound effect
            currentState = nextLevel;   // Transition to target level
            enterPressed = true;        // Mark key as pressed
            initialFrame = true;        // Reset for next state entry
        }
    }
    else {
        enterPressed = false;  // Reset when key released
    }
    
    // Menu navigation input (M key)
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        if (!mPressed) {  // Edge detection to prevent key repeat
            navSounds.playBack();    // Play back navigation sound
            currentState = MENU;     // Return to main menu
            initialFrame = true;     // Reset for next state entry
            mPressed = true;         // Mark key as pressed
        }
    }
    else {
        mPressed = false;  // Reset when key released
    }
    
    // Settings access input (F1 key)
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        if (!f1Pressed) {  // Edge detection to prevent key repeat
            navSounds.playSelect();        // Play selection sound effect
            extern GameState previousState;
            previousState = currentState;  // Store current state for return
            currentState = SETTINGS;       // Open settings menu
            initialFrame = true;           // Reset for next state entry
            f1Pressed = true;              // Mark key as pressed
        }
    }
    else {
        f1Pressed = false;  // Reset when key released
    }
}