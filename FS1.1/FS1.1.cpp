#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "PlayingState.h"
#include "Playingstate2.h"
#include "PlayingState3.h"
#include "PreLevelState.h"
#include "IntroductionState.h"  // NEW: Include introduction state
#include "SettingsState.h"
#include "GameState.h"
#include "NavigationSounds.h"

using namespace sf;
using namespace std;

//=== GLOBAL GAME STATE MANAGEMENT ===
// Stores the previous game state for returning from settings menu
// This allows the settings menu to remember which state called it
GameState previousState = MENU;

//=== GLOBAL AUDIO SYSTEM ===
// Global navigation sound system - definition (not declaration)
// Provides consistent UI audio feedback throughout the application
NavigationSounds navSounds;

//=== NAVIGATION SOUNDS IMPLEMENTATION ===
// Implementation of NavigationSounds methods for UI audio feedback

// Loads all required sound files for UI interactions
// Returns true if all sounds loaded successfully, false otherwise
bool NavigationSounds::loadSounds() {
    bool allLoaded = true;
    
    //=== HOVER SOUND LOADING ===
    // Sound played when hovering over menu items or changing selection
    if (!hoverBuffer.loadFromFile("Sounds/UI_Hover.ogg")) {
        cerr << "Warning: Could not load hover sound effect" << endl;
        allLoaded = false;
    } else {
        hoverSound = Sound(hoverBuffer);  // Create sound object from buffer
    }
    
    //=== SELECT SOUND LOADING ===
    // Sound played when confirming selections or entering menus
    if (!selectBuffer.loadFromFile("Sounds/UI_Select.ogg")) {
        cerr << "Warning: Could not load select sound effect" << endl;
        allLoaded = false;
    } else {
        selectSound = Sound(selectBuffer);  // Create sound object from buffer
    }
    
    //=== BACK SOUND LOADING ===
    // Sound played when returning to previous menu or canceling actions
    if (!backBuffer.loadFromFile("Sounds/UI_Back.ogg")) {
        cerr << "Warning: Could not load back sound effect" << endl;
        allLoaded = false;
    } else {
        backSound = Sound(backBuffer);  // Create sound object from buffer
    }
    
    //=== ERROR SOUND LOADING (COMMENTED OUT) ===
    // Sound for invalid actions or error conditions
    // Currently disabled but can be uncommented if needed
    //// Try to load error sound
    //if (!errorBuffer.loadFromFile("Sounds/UI_Error.ogg") && 
    //    !errorBuffer.loadFromFile("Sounds/UI_Error.wav")) {
    //    cerr << "Warning: Could not load error sound effect" << endl;
    //    allLoaded = false;
    //} else {
    //    errorSound = Sound(errorBuffer);
    //}
    
    soundsLoaded = allLoaded;  // Store overall loading success state
    updateVolume();           // Apply current volume settings
    return allLoaded;         // Return success status
}

// Updates volume for all navigation sounds based on current settings
// Called when volume settings change or sounds are first loaded
void NavigationSounds::updateVolume() {
    if (hoverSound.has_value()) hoverSound->setVolume(soundVolume);   // Apply volume to hover sound
    if (selectSound.has_value()) selectSound->setVolume(soundVolume); // Apply volume to select sound
    if (backSound.has_value()) backSound->setVolume(soundVolume);     // Apply volume to back sound
    if (errorSound.has_value()) errorSound->setVolume(soundVolume);   // Apply volume to error sound
}

// Plays hover sound with immediate replacement of any currently playing instance
// Stops current sound before playing new one for responsive audio feedback
void NavigationSounds::playHover() {
    if (soundsLoaded && hoverSound.has_value()) {
        hoverSound->stop(); // Stop any currently playing hover sound
        hoverSound->play(); // Start the new sound immediately
    }
}

// Plays selection confirmation sound with immediate replacement
// Used for menu selections, button presses, and confirmations
void NavigationSounds::playSelect() {
    if (soundsLoaded && selectSound.has_value()) {
        selectSound->stop(); // Stop any currently playing select sound
        selectSound->play(); // Start the new sound immediately
    }
}

// Plays back/return navigation sound with immediate replacement
// Used when returning to previous menus or canceling operations
void NavigationSounds::playBack() {
    if (soundsLoaded && backSound.has_value()) {
        backSound->stop(); // Stop any currently playing back sound
        backSound->play(); // Start the new sound immediately
    }
}

// Plays error sound for invalid actions with immediate replacement
// Used for boundary conditions, invalid inputs, or failed operations
void NavigationSounds::playError() {
    if (soundsLoaded && errorSound.has_value()) {
        errorSound->stop(); // Stop any currently playing error sound
        errorSound->play(); // Start the new sound immediately
    }
}

//=== MAIN APPLICATION ENTRY POINT ===
// Central game loop handling all states and core application logic
int main()
{
    //=== WINDOW INITIALIZATION ===
    // Create the main application window in fullscreen mode
    // Uses desktop resolution for optimal display compatibility
    RenderWindow window(VideoMode::getDesktopMode(), "/Settings Puzzles/", Style::Default, State::Fullscreen);

    //=== INPUT STATE TRACKING ===
    // Static variable to track mouse button state for edge detection
    static bool mouseLeftPressed = false;

    //=== FONT SYSTEM INITIALIZATION ===
    // Load the primary font for all menu text rendering
    Font font("arial.ttf");

    //=== AUDIO SYSTEM INITIALIZATION ===
    // Initialize navigation sound system for UI feedback
    navSounds.loadSounds();

    //=== MAIN MENU CONFIGURATION ===
    // Define the primary menu options available to the player
    const vector<string> options = { "Start", "Settings", "Exit" };
    int selected = 0;           // Index of the currently selected menu option
    int previousSelected = -1;  // Track previous selection for hover sound logic
    GameState state = INTRODUCTION;  // NEW: Start with introduction instead of MENU
    
    bool running = true;        // Controls the main game loop execution

    //=== MENU TEXT OBJECTS PREPARATION ===
    // Create and configure text objects for each menu option
    vector<Text> menuTexts;
    for (size_t i = 0; i < options.size(); ++i) {
        Text t(font, options[i], 50);              // Create text with font and size
        t.setStyle(Text::Italic | Text::Bold);     // Apply styling
        t.setFillColor(Color::White);              // Set text color
        
        // Calculate text bounds for centering
        auto bounds = t.getLocalBounds();
        t.setOrigin(Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));  // Center origin
        
        // Position text in center column with vertical spacing
        t.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f + i * 80.f));
        menuTexts.push_back(t);  // Add to menu text collection
    }

    //=== TITLE TEXT CONFIGURATION ===
    // Create and configure the main application title
    Text title(font, "/Setting Puzzles/", 100);
    title.setStyle(Text::Bold | Text::Underlined);  // Bold and underlined styling
    title.setFillColor(Color::Blue);                // Blue color for distinction
    
    // Center title horizontally and position in upper portion of screen
    auto titleBounds = title.getLocalBounds();
    title.setOrigin(Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
    title.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 4.f));

    //=== PERSISTENT UI ELEMENTS ===
    // Create permanent F1 settings hint text visible on most screens
    Text settingsHint(font, "F1 - Settings", 24);
    settingsHint.setFillColor(Color::White);        // White text
    settingsHint.setOutlineColor(Color::Black);     // Black outline for visibility
    settingsHint.setOutlineThickness(5.f);          // Thick outline
    
    // Position in top-right corner with padding
    auto hintBounds = settingsHint.getLocalBounds();
    settingsHint.setOrigin(Vector2f(hintBounds.size.x, 0)); // Right-aligned origin
    settingsHint.setPosition(Vector2f(window.getSize().x - 20.f, 20.f)); // Top-right with 20px padding

    //=== MENU SELECTION INDICATOR ===
    // Create a red circle to visually indicate the selected menu option
    CircleShape selector(20.f);          // 20 pixel radius circle
    selector.setFillColor(Color::Red);   // Red color for visibility

    //=== INPUT STATE MANAGEMENT ===
    // Track key press states to handle single key events and prevent key repeat
    bool wPressed = false, sPressed = false, enterPressed = false, f1Pressed = false;

    //=== BACKGROUND MUSIC SYSTEM ===
    // Music variables declared outside loop to maintain state across frames
    Music music;            // SFML Music object for background music
    string currentSong;     // Track currently loaded song to prevent redundant loading

    //=== EXTERNAL SETTINGS ACCESS ===
    // Access the music volume setting from SettingsState for dynamic volume control
    extern float musicVolume;

    //=== MAIN APPLICATION LOOP ===
    // Primary game loop - continues until window is closed
    while (window.isOpen())
    {
        //=== EVENT PROCESSING SYSTEM ===
        // Process all pending window events each frame
        optional<Event> event;
        while ((event = window.pollEvent()))
        {
            if (event->is<Event::Closed>())
                window.close(); // Close window if user requests exit
        }

        //=== DYNAMIC AUDIO VOLUME MANAGEMENT ===
        // Update sound effects volume based on current music volume setting
        // Uses 80% of music volume for sound effects to maintain audio balance
        extern float musicVolume;
        navSounds.soundVolume = musicVolume * 0.8f; // Calculate sound effect volume
        navSounds.updateVolume();                   // Apply volume changes

        //=== MAIN MENU INPUT AND NAVIGATION ===
        // Handle all menu interactions when in main menu state
        if (state == MENU) {

            //=== MOUSE POSITION TRACKING ===
            // Get mouse position for hover detection and menu selection
            Vector2i mousePosition = Mouse::getPosition(window);
            Vector2f mousePos = window.mapPixelToCoords(mousePosition);

            //=== MOUSE HOVER DETECTION SYSTEM ===
            // Check if mouse is hovering over any menu item and play sound on change
            for (size_t i = 0; i < menuTexts.size(); ++i) {
                if (menuTexts[i].getGlobalBounds().contains(mousePos)) {
                    if (selected != static_cast<int>(i)) {
                        navSounds.playHover(); // Play hover sound when selection changes
                    }
                    selected = static_cast<int>(i);  // Update selected menu item
                    break;  // Exit loop once hover target found
                }
            }

            //=== MOUSE CLICK HANDLING SYSTEM ===
            // Process left mouse button clicks for menu selection
            bool isMouseLeftButtonPressed = Mouse::isButtonPressed(Mouse::Button::Left);
            if (isMouseLeftButtonPressed && !mouseLeftPressed) {
                mouseLeftPressed = true;        // Mark button as pressed
                navSounds.playSelect();         // Play selection sound
                
                // Execute action based on selected menu item
                switch (selected) {
                case 0: state = PRELEVEL1; break;   // Start game (go to pre-level screen)
                case 1: 
                    previousState = MENU;           // Store current state
                    state = SETTINGS;               // Open settings menu
                    break;
                case 2: state = EXIT; window.close(); break; // Exit application
                }
            }
            else if (!isMouseLeftButtonPressed) {
                mouseLeftPressed = false;  // Reset when button released
            }

            //=== KEYBOARD NAVIGATION SYSTEM ===
            // Handle W key (up navigation) with sound feedback
            if (Keyboard::isKeyPressed(Keyboard::Key::W)) {
                if (!wPressed) {  // Edge detection to prevent key repeat
                    // Move selection up with wraparound
                    selected = (selected - 1 + static_cast<int>(options.size())) % static_cast<int>(options.size());
                    navSounds.playHover(); // Play hover sound for keyboard navigation
                    wPressed = true;       // Mark key as pressed
                }
            }
            else {
                wPressed = false;  // Reset when key released
            }
            
            // Handle S key (down navigation) with sound feedback
            if (Keyboard::isKeyPressed(Keyboard::Key::S)) {
                if (!sPressed) {  // Edge detection to prevent key repeat
                    // Move selection down with wraparound
                    selected = (selected + 1) % static_cast<int>(options.size());
                    navSounds.playHover(); // Play hover sound for keyboard navigation
                    sPressed = true;       // Mark key as pressed
                }
            }
            else {
                sPressed = false;  // Reset when key released
            }

            //=== KEYBOARD SELECTION SYSTEM ===
            // Handle ENTER key for menu item activation
            if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
                if (!enterPressed) {  // Edge detection to prevent key repeat
                    navSounds.playSelect(); // Play select sound
                    
                    // Execute action based on selected menu item
                    switch (selected) {
                    case 0: state = PRELEVEL1; break;   // Start game (go to pre-level screen)
                    case 1: 
                        previousState = MENU;           // Store current state
                        state = SETTINGS;               // Open settings menu
                        break;
                    case 2: state = EXIT; window.close(); break; // Exit application
                    }
                    enterPressed = true;  // Mark key as pressed
                }
            }
            else {
                enterPressed = false;  // Reset when key released
            }

            //=== SETTINGS SHORTCUT SYSTEM ===
            // Handle F1 key for direct settings access
            if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
                if (!f1Pressed) {  // Edge detection to prevent key repeat
                    navSounds.playSelect();  // Play selection sound
                    previousState = MENU;    // Store current state
                    state = SETTINGS;        // Open settings menu
                    f1Pressed = true;        // Mark key as pressed
                }
            }
            else {
                f1Pressed = false;  // Reset when key released
            }
        }

        //=== DYNAMIC BACKGROUND MUSIC SYSTEM ===
        // Handle music changes based on current game state for immersive experience
        string desiredSong;

        // Select appropriate background music for each game state
        if (state == INTRODUCTION) {
            desiredSong = "Sounds/PiecebyPiece.mp3";      // Introduction music
        } else if (state == MENU) {
            desiredSong = "Sounds/PiecebyPiece.mp3";      // Menu music
        } else if (state == PLAYING) {
            desiredSong = "Sounds/PiecebyPiece2.mp3";     // Level 1 music
        } else if (state == PLAYING2) {
            desiredSong = "Sounds/PiecebyPiece2.mp3";     // Level 2 music
        } else if (state == PLAYING3) {
            desiredSong = "Sounds/PiecebyPiece2.mp3";     // Level 3 music
        } else {
            desiredSong = "Sounds/PiecebyPiece.mp3";      // Fallback music
        }

        // Change music only when transitioning to different song
        if (desiredSong != currentSong) {
            // Stop currently playing music
            if (music.getStatus() == Music::Status::Playing)
                music.stop();
                
            // Attempt to load and play new music
            if (music.openFromFile(desiredSong)) {
                music.setLooping(true);           // Enable continuous looping
                music.setVolume(musicVolume);     // Apply current volume setting
                music.play();                     // Start playback
                currentSong = desiredSong;        // Update current song tracking
            } else {
                std::cerr << "Failed to load " << desiredSong << std::endl;
                currentSong.clear();              // Clear on failure
            }
        }

        // Update volume if music is playing (handles real-time volume changes)
        if (music.getStatus() == Music::Status::Playing) {
            music.setVolume(musicVolume);
        }

        //=== FRAME RENDERING PREPARATION ===
        window.clear(); // Clear the window for new frame rendering

        //=== STATE-BASED RENDERING SYSTEM ===
        // Render appropriate content based on current game state
        if (state == INTRODUCTION) {
            // NEW: Show introduction screen explaining the game's purpose
            handleIntroductionState(window, running, state);
        }
        else if (state == MENU) {
            //=== MAIN MENU RENDERING ===
            // Draw title and all menu options
            window.draw(title);                    // Application title
            for (size_t i = 0; i < menuTexts.size(); ++i) {
                window.draw(menuTexts[i]);         // Individual menu items
            }
            
            // Draw selection indicator next to selected option
            Vector2f pos = menuTexts[selected].getPosition();
            selector.setPosition(Vector2f(pos.x - 200.f, pos.y - 8.f));  // Position left of text
            window.draw(selector);             // Red circle indicator
        }
        else if (state == PRELEVEL1) {
            // Show pre-level screen for level 1
            handlePreLevelState(window, running, state, PLAYING);
        }
        else if (state == PLAYING) {
            // Delegate to the scrolling text state handler
            handlePlayingState(window, running, state);
        }
        else if (state == PRELEVEL2) {
            // Show pre-level screen for level 2
            handlePreLevelState(window, running, state, PLAYING2);
        }
        else if (state == PLAYING2) {
            // Delegate to the maze game state handler
            handlePlayingState2(window, running, state);
        }
        else if (state == PRELEVEL3) {
            // Show pre-level screen for level 3
            handlePreLevelState(window, running, state, PLAYING3);
        }
        else if (state == PLAYING3) {
            // Delegate to the driving game state handler
            handlePlayingState3(window, running, state);
        }
        else if (state == SETTINGS) {
            // Delegate to the settings state handler
            handleSettingsState(window, running, state);
        }

        //=== PERSISTENT UI OVERLAY ===
        // Draw the permanent settings hint on all states except settings menu and introduction
        if (state != SETTINGS && state != INTRODUCTION) {
            window.draw(settingsHint);  // F1 - Settings hint
        }

        //=== FRAME PRESENTATION ===
        window.display(); // Present completed frame to screen
    }
    return 0;  // Successful application termination
}