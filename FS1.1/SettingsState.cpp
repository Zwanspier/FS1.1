#include "SettingsState.h"
#include "NavigationSounds.h"

using namespace sf;
using namespace std;

//=== GLOBAL SETTINGS VARIABLES DEFINITIONS ===
// Global settings variables that persist throughout application lifetime
// These control various aspects of gameplay, performance, and user experience

//=== PERFORMANCE CONFIGURATION ===
int framerateIndex = 1;                 // Default to 60 FPS (index 1 in options array)
int framerateOptions[] = { 30, 60, 120, 144, 240 };  // Available framerate options for text speed
bool vsyncEnabled = true;               // Vertical sync enabled by default for smooth rendering

//=== VISUAL CONFIGURATION ===
float gamma = 0.0f;                     // Maze wall brightness (0.0f = invisible, 2.0f = full brightness)

//=== MAZE SIZE CONFIGURATION ===
int resolutionIndex = 2;                // Default to 1920x1080 equivalent maze size (index 2)
float musicVolume = 40.0f;              // Default music volume at 40% (0-100 range)

// Available maze size options (repurposed from resolution settings)
// Each resolution corresponds to a different maze complexity level
vector<Vector2u> resolutionOptions = {
    {1280, 720},                        // Small maze size
    {1600, 900},                        // Medium-small maze size
    {1920, 1080},                       // Medium maze size (default)
    {2560, 1440},                       // Large maze size
    {3840, 2160}                        // Extra large maze size
};

//=== STATE FLAGS ===
bool mazeNeedsRegeneration = false;     // Flag to trigger maze regeneration
bool settingsChanged = false;           // Flag indicating settings have been modified

//=== SETTINGS APPLICATION SYSTEM ===
// Applies all pending settings changes to the application systems
// Handles window properties, maze regeneration, and other system updates
void applySettings(RenderWindow& window) {
    // Static variables to track previous settings for change detection
    static int lastResolutionIndex = resolutionIndex;
    static bool lastVsyncEnabled = vsyncEnabled;

    //=== MAZE SIZE CHANGE DETECTION ===
    // Check if maze size setting has changed since last application
    bool mazeSizeChanged = (resolutionIndex != lastResolutionIndex);

    if (mazeSizeChanged) {
        // Trigger maze regeneration without changing window size
        // The maze system will pick up this flag and regenerate accordingly
        mazeNeedsRegeneration = true;
        lastResolutionIndex = resolutionIndex;  // Update tracking variable
    }

    //=== VSYNC SETTING APPLICATION ===
    // Apply VSync setting only if it has changed (avoids unnecessary API calls)
    if (vsyncEnabled != lastVsyncEnabled) {
        window.setVerticalSyncEnabled(vsyncEnabled);  // Apply VSync to window
        lastVsyncEnabled = vsyncEnabled;              // Update tracking variable
    }

    // Note: Framerate setting is not applied to window directly
    // It's used for text speed calculation in PlayingState instead
    
    // Note: Music volume is applied directly in main loop when music changes
    // This allows for real-time volume adjustment without settings application

    settingsChanged = true;  // Mark that settings have been processed
}

//=== MAIN SETTINGS MENU HANDLER ===
// Handles the complete settings menu interface including rendering and input processing
// Provides comprehensive control over all game settings with visual and audio feedback
void handleSettingsState(RenderWindow& window, bool& running, GameState& state)
{
    //=== MENU STATE VARIABLES ===
    // Static variables maintain menu state between function calls
    static int selected = 0;           // Currently selected menu item index
    static int previousSelected = -1;  // Previous selection for hover sound detection
    
    // Input state tracking to prevent key repeat issues
    static bool upPressed = false, downPressed = false;           // Vertical navigation
    static bool leftPressed = false, rightPressed = false;       // Horizontal value adjustment
    static bool enterPressed = false, escapePressed = false;     // Confirmation and cancellation
    static bool mouseLeftPressed = false, mouseRightPressed = false;  // Mouse interaction

    //=== EXTERNAL STATE ACCESS ===
    extern GameState previousState;  // Access to return state when exiting settings

    //=== MENU STRUCTURE DEFINITION ===
    // Define all available settings options in display order
    const vector<string> options = { 
        "VSync: ",           // Toggle vertical synchronization
        "Text Speed: ",      // Adjust text scrolling speed via framerate
        "Wall Visibility: ", // Control maze wall brightness
        "Maze Size: ",       // Select maze complexity/size
        "Volume: ",          // Adjust music and sound volume
        "Apply Changes",     // Apply all pending settings
        "Back"              // Return to previous menu
    };

    //=== FONT SYSTEM INITIALIZATION ===
    // One-time font loading for settings menu text
    static Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        font.openFromFile("arial.ttf");  // Load standard application font
        fontLoaded = true;               // Mark as loaded
    }

    //=== RENDERING PREPARATION ===
    window.clear(Color::Black);  // Clear screen with black background

    //=== MENU TEXT RENDERING SYSTEM ===
    // Store text objects for mouse interaction detection
    vector<Text> textObjects;

    // Render each menu option with current values and appropriate styling
    for (size_t i = 0; i < options.size(); ++i) {
        Text text(font, options[i], 40);  // Create text object
        
        // Apply color based on selection state
        text.setFillColor(i == selected ? Color::Yellow : Color::White);
        text.setPosition(Vector2f(100.f, 200.f + i * 60.f));  // Vertical menu layout

        //=== DYNAMIC VALUE DISPLAY SYSTEM ===
        // Update text content based on current setting values
        if (i == 0) {
            // VSync setting display
            text.setString(options[i] + (vsyncEnabled ? "On" : "Off"));
        }
        else if (i == 1) {
            // Text speed (framerate) setting display
            text.setString(options[i] + to_string(framerateOptions[framerateIndex]));
        }
        else if (i == 2) {
            // Wall visibility (gamma) setting display as percentage
            int gammaPercent = static_cast<int>((gamma / 2.0f) * 100.0f);
            text.setString(options[i] + to_string(gammaPercent) + "%");
        }
        else if (i == 3) {
            // Maze size setting display
            text.setString(options[i] + to_string(resolutionOptions[resolutionIndex].x) + 
                          "x" + to_string(resolutionOptions[resolutionIndex].y));
        }
        else if (i == 4) {
            // Music volume setting display as percentage
            int volumePercent = static_cast<int>(musicVolume);
            text.setString(options[i] + to_string(volumePercent) + "%");
        }
        // Options 5 (Apply Changes) and 6 (Back) use their default text

        textObjects.push_back(text);  // Store for mouse interaction
        window.draw(text);            // Render to screen
    }

    //=== MOUSE INTERACTION SYSTEM ===
    // Handle mouse hover detection for menu selection with audio feedback
    Vector2i mousePosition = Mouse::getPosition(window);
    Vector2f mousePos = window.mapPixelToCoords(mousePosition);

    // Check if mouse is hovering over any menu item
    for (size_t i = 0; i < textObjects.size(); ++i) {
        if (textObjects[i].getGlobalBounds().contains(mousePos)) {
            if (selected != static_cast<int>(i)) {
                navSounds.playHover(); // Play hover sound when selection changes
            }
            selected = static_cast<int>(i);  // Update selected item
            break;  // Exit loop once hover target found
        }
    }

    //=== MOUSE CLICK HANDLING SYSTEM ===
    // Process left mouse button clicks for setting adjustments and navigation
    bool isMouseLeftButtonPressed = Mouse::isButtonPressed(Mouse::Button::Left);
    bool isMouseRightButtonPressed = Mouse::isButtonPressed(Mouse::Button::Right);

    if (isMouseLeftButtonPressed && !mouseLeftPressed) {
        mouseLeftPressed = true;  // Mark button as pressed

        //=== LEFT CLICK ACTIONS ===
        // Handle left click actions for each menu item
        if (selected == 0) {
            // Toggle VSync setting
            vsyncEnabled = !vsyncEnabled;
            navSounds.playSelect();
        }
        else if (selected == 1) {
            // Increase framerate setting with boundary checking
            if (framerateIndex < static_cast<int>(size(framerateOptions)) - 1) {
                framerateIndex++;
                navSounds.playSelect();
            } else {
                navSounds.playError(); // Can't increase further
            }
        }
        else if (selected == 2) {
            // Increase gamma (wall visibility) setting with boundary checking
            if (gamma < 2.0f) {
                gamma += 0.1f;
                if (gamma > 2.0f) gamma = 2.0f;  // Clamp to maximum
                navSounds.playSelect();
            } else {
                navSounds.playError(); // Already at maximum
            }
        }
        else if (selected == 3) {
            // Cycle through resolution (maze size) options
            resolutionIndex++;
            if (resolutionIndex >= static_cast<int>(size(resolutionOptions))) 
                resolutionIndex = 0;  // Wrap around to first option
            navSounds.playSelect();
        }
        else if (selected == 4) {
            // Increase music volume with boundary checking
            if (musicVolume < 100.0f) {
                musicVolume += 10.0f;  // Increase by 10%
                if (musicVolume > 100.0f) musicVolume = 100.0f;  // Clamp to maximum
                navSounds.playSelect();
            } else {
                navSounds.playError(); // Already at maximum
            }
        }
        else if (selected == 5) {
            // Apply all settings changes
            applySettings(window);
            navSounds.playSelect();
        }
        else if (selected == 6) {
            // Return to previous menu
            navSounds.playBack();
            state = previousState;
        }
    }
    else if (!isMouseLeftButtonPressed) {
        mouseLeftPressed = false;  // Reset when button released
    }

    //=== RIGHT CLICK HANDLING SYSTEM ===
    // Process right mouse button clicks for decreasing values
    if (isMouseRightButtonPressed && !mouseRightPressed) {
        mouseRightPressed = true;  // Mark button as pressed

        //=== RIGHT CLICK ACTIONS ===
        // Handle right click actions for decreasing setting values
        if (selected == 1) {
            // Decrease framerate setting with boundary checking
            if (framerateIndex > 0) {
                framerateIndex--;
                navSounds.playSelect();
            } else {
                navSounds.playError(); // Can't decrease further
            }
        }
        else if (selected == 2) {
            // Decrease gamma (wall visibility) setting with boundary checking
            if (gamma > 0.0f) {
                gamma -= 0.1f;
                if (gamma < 0.0f) gamma = 0.0f;  // Clamp to minimum
                navSounds.playSelect();
            } else {
                navSounds.playError(); // Already at minimum
            }
        }
        else if (selected == 3) {
            // Cycle backwards through resolution (maze size) options
            resolutionIndex--;
            if (resolutionIndex < 0) 
                resolutionIndex = static_cast<int>(size(resolutionOptions)) - 1;  // Wrap to last option
            navSounds.playSelect();
        }
        else if (selected == 4) {
            // Decrease music volume with boundary checking
            if (musicVolume > 0.0f) {
                musicVolume -= 10.0f;  // Decrease by 10%
                if (musicVolume < 0.0f) musicVolume = 0.0f;  // Clamp to minimum
                navSounds.playSelect();
            } else {
                navSounds.playError(); // Already at minimum
            }
        }
    }
    else if (!isMouseRightButtonPressed) {
        mouseRightPressed = false;  // Reset when button released
    }

    //=== FRAME PRESENTATION ===
    window.display();  // Present completed settings menu frame

    //=== KEYBOARD NAVIGATION SYSTEM ===
    // Handle keyboard input for menu navigation with sound feedback
    
    // W key - Move selection up
    if (Keyboard::isKeyPressed(Keyboard::Key::W)) {
        if (!upPressed) {  // Edge detection to prevent key repeat
            selected = (selected - 1 + static_cast<int>(options.size())) % static_cast<int>(options.size());
            navSounds.playHover();  // Play hover sound for navigation
            upPressed = true;       // Mark key as pressed
        }
    }
    else upPressed = false;  // Reset when key released

    // S key - Move selection down
    if (Keyboard::isKeyPressed(Keyboard::Key::S)) {
        if (!downPressed) {  // Edge detection to prevent key repeat
            selected = (selected + 1) % static_cast<int>(options.size());
            navSounds.playHover();  // Play hover sound for navigation
            downPressed = true;     // Mark key as pressed
        }
    }
    else downPressed = false;  // Reset when key released

    //=== KEYBOARD VALUE ADJUSTMENT SYSTEM ===
    // A key - Decrease values (left direction)
    if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
        if (!leftPressed) {  // Edge detection to prevent key repeat
            //=== DECREASE VALUE LOGIC ===
            if (selected == 1 && framerateIndex > 0) {
                // Decrease framerate
                framerateIndex--;
                navSounds.playSelect();
            }
            else if (selected == 2) {
                // Decrease gamma (wall visibility)
                if (gamma > 0.0f) {
                    gamma -= 0.1f;
                    if (gamma < 0.0f) gamma = 0.0f;
                    navSounds.playSelect();
                } else {
                    navSounds.playError();  // Already at minimum
                }
            }
            else if (selected == 3) {
                // Cycle backwards through maze sizes
                resolutionIndex--;
                if (resolutionIndex < 0) 
                    resolutionIndex = static_cast<int>(size(resolutionOptions)) - 1;
                navSounds.playSelect();
            }
            else if (selected == 4) {
                // Decrease music volume
                if (musicVolume > 0.0f) {
                    musicVolume -= 10.0f;
                    if (musicVolume < 0.0f) musicVolume = 0.0f;
                    navSounds.playSelect();
                } else {
                    navSounds.playError();  // Already at minimum
                }
            }
            else if (selected == 1 && framerateIndex <= 0) {
                navSounds.playError(); // Can't decrease framerate further
            }
            leftPressed = true;  // Mark key as pressed
        }
    }
    else leftPressed = false;  // Reset when key released

    // D key - Increase values (right direction)
    if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
        if (!rightPressed) {  // Edge detection to prevent key repeat
            //=== INCREASE VALUE LOGIC ===
            if (selected == 1 && framerateIndex < static_cast<int>(size(framerateOptions)) - 1) {
                // Increase framerate
                framerateIndex++;
                navSounds.playSelect();
            }
            else if (selected == 2) {
                // Increase gamma (wall visibility)
                if (gamma < 2.0f) {
                    gamma += 0.1f;
                    if (gamma > 2.0f) gamma = 2.0f;
                    navSounds.playSelect();
                } else {
                    navSounds.playError();  // Already at maximum
                }
            }
            else if (selected == 3) {
                // Cycle forward through maze sizes
                resolutionIndex++;
                if (resolutionIndex >= static_cast<int>(size(resolutionOptions))) 
                    resolutionIndex = 0;
                navSounds.playSelect();
            }
            else if (selected == 4) {
                // Increase music volume
                if (musicVolume < 100.0f) {
                    musicVolume += 10.0f;
                    if (musicVolume > 100.0f) musicVolume = 100.0f;
                    navSounds.playSelect();
                } else {
                    navSounds.playError();  // Already at maximum
                }
            }
            else if (selected == 1 && framerateIndex >= static_cast<int>(size(framerateOptions)) - 1) {
                navSounds.playError(); // Can't increase framerate further
            }
            rightPressed = true;  // Mark key as pressed
        }
    }
    else rightPressed = false;  // Reset when key released

    //=== KEYBOARD CONFIRMATION SYSTEM ===
    // ENTER key - Activate selected menu item
    if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
        if (!enterPressed) {  // Edge detection to prevent key repeat
            if (selected == 0) {
                // Toggle VSync setting
                vsyncEnabled = !vsyncEnabled;
                navSounds.playSelect();
            }
            else if (selected == 5) {
                // Apply all settings changes
                applySettings(window);
                navSounds.playSelect();
            }
            else if (selected == 6) {
                // Return to previous menu
                navSounds.playBack();
                state = previousState;
            }
            enterPressed = true;  // Mark key as pressed
        }
    }
    else enterPressed = false;  // Reset when key released

    //=== KEYBOARD CANCELLATION SYSTEM ===
    // ESCAPE key - Return to previous menu
    if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
        if (!escapePressed) {  // Edge detection to prevent key repeat
            navSounds.playBack();       // Play back navigation sound
            state = previousState;      // Return to calling state
            escapePressed = true;       // Mark key as pressed
        }
    }
    else escapePressed = false;  // Reset when key released
}