#include "IntroductionState.h"
#include "NavigationSounds.h"

//=== INTRODUCTION STATE HANDLER ===
// Displays the initial game introduction screen explaining the purpose and concept
// Provides context for the game's theme and prepares players for the experience
void handleIntroductionState(RenderWindow& window, bool& running, GameState& state)
{
    //=== PERSISTENT STATE VARIABLES ===
    // Static variables maintain state between function calls
    static Font font;                   // Text rendering font
    static bool fontLoaded = false;     // Font loading status flag
    static Clock animationClock;        // Animation timing control
    static bool initialized = false;    // Initialization flag
    
    // Input state tracking to prevent key repeat issues
    static bool enterPressed = false;   // ENTER/SPACE key state
    static bool escPressed = false;     // ESC key state
    static bool f1Pressed = false;      // F1 key (settings) state
    
    //=== INITIALIZATION ===
    // One-time setup when entering the state
    if (!initialized) {
        animationClock.restart();       // Start animation timer
        
        // Reset input states to prevent carried-over key presses
        enterPressed = Keyboard::isKeyPressed(Keyboard::Key::Enter) || 
                      Keyboard::isKeyPressed(Keyboard::Key::Space);
        escPressed = Keyboard::isKeyPressed(Keyboard::Key::Escape);
        f1Pressed = Keyboard::isKeyPressed(Keyboard::Key::F1);
        
        initialized = true;
    }
    
    //=== FONT LOADING ===
    // Load font for text rendering
    if (!fontLoaded) {
        font.openFromFile("arial.ttf");
        fontLoaded = true;
    }
    
    //=== ANIMATION TIMING ===
    // Get elapsed time for animation effects
    float elapsedTime = animationClock.getElapsedTime().asSeconds();
    
    //=== RENDERING SETUP ===
    window.clear(Color(20, 20, 40));  // Dark blue background for atmosphere
    
    //=== GAME TITLE ===
    // Main title with emphasis
    Text title(font, "/Setting Puzzles/", 80);
    title.setStyle(Text::Bold);
    title.setFillColor(Color::Cyan);
    
    // Center title horizontally, position in upper area
    auto titleBounds = title.getLocalBounds();
    title.setOrigin(Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
    title.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 10.f));
    window.draw(title);
    
    //=== INTRODUCTION TEXT CONTENT ===
    // Define the introduction text that explains the game's purpose
    vector<string> introLines = {
        "Welcome to Setting Puzzles",
        "",
        "A journey through three unique challenges that explore",
        "the boundaries between settings and gameplay.",
        "",
        "Each level presents a different perspective on how",
        "configuration options can become part of the experience.",
        "",
        "Level 1: Text flows and speed - where reading becomes reactive",
        "Level 2: Navigation through space - where paths define possibility", 
        "Level 3: A drive with choices - where silence speaks volumes",
        "",
        "This is not just a game with settings,",
        "but an exploration of settings as narrative."
    };
    
    //=== ANIMATED TEXT RENDERING ===
    // Render introduction text with fade-in animation
    float baseY = window.getSize().y / 2.f - (introLines.size() * 25.f);
    
    for (size_t i = 0; i < introLines.size(); ++i) {
        // Calculate fade-in timing for each line
        float lineDelay = i * 0.3f;  // 0.3 second delay between lines
        float lineAlpha = std::min(1.0f, std::max(0.0f, (elapsedTime - lineDelay) * 2.0f));
        
        if (lineAlpha > 0.0f) {
            Text line(font, introLines[i], 28);
            
            // Different colors for different types of lines
            if (i == 0) {
                line.setFillColor(Color(255, 255, 0, static_cast<std::uint8_t>(255 * lineAlpha)));  // Yellow header
                line.setStyle(Text::Bold);
            } else if (introLines[i].find("Level") == 0) {
                line.setFillColor(Color(100, 255, 100, static_cast<std::uint8_t>(255 * lineAlpha))); // Green for levels
                line.setStyle(Text::Bold);
            } else if (i >= introLines.size() - 2) {
                line.setFillColor(Color(255, 200, 100, static_cast<std::uint8_t>(255 * lineAlpha))); // Orange conclusion
                line.setStyle(Text::Italic);
            } else {
                line.setFillColor(Color(200, 200, 255, static_cast<std::uint8_t>(255 * lineAlpha))); // Light blue body
            }
            
            // Center text horizontally
            auto lineBounds = line.getLocalBounds();
            line.setOrigin(Vector2f(lineBounds.size.x / 2.f, 0));
            line.setPosition(Vector2f(window.getSize().x / 2.f, baseY + i * 35.f));
            window.draw(line);
        }
    }
    
    //=== CONTINUATION PROMPT ===
    // Show continue prompt after all text has appeared
    float promptDelay = introLines.size() * 0.3f + 1.0f;  // Wait for all text plus 1 second
    if (elapsedTime > promptDelay) {
        // Pulsing effect for the continue prompt
        float pulseAlpha = (sin(elapsedTime * 3.0f) * 0.3f + 0.7f);
        
        Text continuePrompt(font, "Press ENTER or SPACE to continue", 32);
        continuePrompt.setStyle(Text::Bold);
        continuePrompt.setFillColor(Color(0, 255, 0, static_cast<std::uint8_t>(255 * pulseAlpha)));
        
        auto promptBounds = continuePrompt.getLocalBounds();
        continuePrompt.setOrigin(Vector2f(promptBounds.size.x / 2.f, promptBounds.size.y / 2.f));
        continuePrompt.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y * 0.85f));
        window.draw(continuePrompt);
        
        // Also show ESC hint
        Text escHint(font, "ESC - Skip to Menu", 20);
        escHint.setFillColor(Color(150, 150, 150, static_cast<std::uint8_t>(255 * pulseAlpha * 0.7f)));
        
        auto escBounds = escHint.getLocalBounds();
        escHint.setOrigin(Vector2f(escBounds.size.x / 2.f, escBounds.size.y / 2.f));
        escHint.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y * 0.90f));
        window.draw(escHint);
    }
    
    //=== INPUT HANDLING SYSTEM ===
    // Process user input for state transitions
    
    // Continue to main menu (ENTER or SPACE)
    if (Keyboard::isKeyPressed(Keyboard::Key::Enter) || Keyboard::isKeyPressed(Keyboard::Key::Space)) {
        if (!enterPressed) {  // Edge detection to prevent key repeat
            navSounds.playSelect();     // Play selection sound effect
            state = MENU;               // Transition to main menu
            initialized = false;        // Reset for potential re-entry
            enterPressed = true;        // Mark key as pressed
        }
    }
    else {
        enterPressed = false;  // Reset when key released
    }
    
    // Skip to main menu (ESC)
    if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
        if (!escPressed) {  // Edge detection to prevent key repeat
            navSounds.playBack();       // Play back sound effect
            state = MENU;               // Skip directly to main menu
            initialized = false;        // Reset for potential re-entry
            escPressed = true;          // Mark key as pressed
        }
    }
    else {
        escPressed = false;  // Reset when key released
    }
    
    // Settings access (F1)
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        if (!f1Pressed) {  // Edge detection to prevent key repeat
            navSounds.playSelect();        // Play selection sound effect
            extern GameState previousState;
            previousState = INTRODUCTION;  // Store current state for return
            state = SETTINGS;              // Open settings menu
            f1Pressed = true;              // Mark key as pressed
        }
    }
    else {
        f1Pressed = false;  // Reset when key released
    }
}