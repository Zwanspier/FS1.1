#pragma once
#include <SFML/Audio.hpp>
#include <optional>

using namespace sf;
using namespace std;

//=== NAVIGATION SOUNDS SYSTEM ===
// Comprehensive audio feedback system for user interface interactions
// Provides consistent sound effects across all menu systems and navigation
// Supports dynamic volume control and graceful fallback for missing audio files

struct NavigationSounds {
    //=== AUDIO BUFFER STORAGE ===
    // Sound buffers store raw audio data loaded from files
    // Each buffer corresponds to a specific UI interaction type
    // Buffers are persistent and shared between multiple Sound instances
    
    SoundBuffer hoverBuffer;    // Audio data for menu item hover/selection change
                               // File: "Sounds/UI_Hover.ogg" 
                               // Usage: When mouse hovers over items or keyboard changes selection
    
    SoundBuffer selectBuffer;   // Audio data for confirmation and positive actions
                               // File: "Sounds/UI_Select.ogg"
                               // Usage: Menu item activation, level start, settings confirmation
    
    SoundBuffer backBuffer;     // Audio data for navigation backward and cancellation
                               // File: "Sounds/UI_Back.ogg"  
                               // Usage: Return to previous menu, cancel operations, ESC key
    
    SoundBuffer errorBuffer;    // Audio data for invalid actions and boundary conditions
                               // File: "Sounds/UI_Error.ogg" (currently disabled)
                               // Usage: Attempting invalid operations, hitting limits
    
    //=== SOUND INSTANCE MANAGEMENT ===
    // Optional Sound objects that can play the loaded audio buffers
    // Using std::optional allows for graceful handling of loading failures
    // Each sound can be played independently with individual volume/pitch control
    
    optional<Sound> hoverSound;    // Playable hover sound instance
                                  // State: Active when hoverBuffer loads successfully
                                  // Behavior: Immediate replacement on rapid hover changes
    
    optional<Sound> selectSound;   // Playable selection confirmation sound instance  
                                  // State: Active when selectBuffer loads successfully
                                  // Behavior: Immediate replacement prevents sound overlap
    
    optional<Sound> backSound;     // Playable back navigation sound instance
                                  // State: Active when backBuffer loads successfully  
                                  // Behavior: Indicates successful return navigation
    
    optional<Sound> errorSound;    // Playable error notification sound instance
                                  // State: Currently disabled (buffer loading commented out)
                                  // Behavior: Would indicate invalid user actions
    
    //=== SYSTEM STATE MANAGEMENT ===
    // Configuration and status tracking for the audio system
    
    bool soundsLoaded = false;     // Master flag indicating overall audio system status
                                  // true: All required sounds loaded successfully
                                  // false: One or more sounds failed to load
                                  // Usage: Guards all playback operations
    
    float soundVolume = 100.0f;    // Master volume level for all navigation sounds
                                  // Range: 0.0f (silent) to 100.0f (full volume)
                                  // Updated: Dynamically from music volume settings
                                  // Applied: To all sound instances via updateVolume()
    
    //=== CONSTRUCTOR ===
    // Default constructor uses compiler-generated implementation
    // All members are initialized with their default values
    // Actual initialization occurs in loadSounds() method
    NavigationSounds() = default;
    
    //=== AUDIO SYSTEM INTERFACE ===
    // Public methods for audio system management and playback control
    
    //=== INITIALIZATION SYSTEM ===
    // Attempts to load all required audio files and initialize sound objects
    // Returns: true if all sounds loaded successfully, false otherwise  
    // Side Effects: Sets soundsLoaded flag, creates Sound instances, applies initial volume
    // Error Handling: Continues loading other sounds if individual files fail
    // Usage: Called once during application startup
    bool loadSounds();
    
    //=== VOLUME MANAGEMENT SYSTEM ===
    // Updates volume for all active sound instances based on current soundVolume setting
    // Called: When volume settings change or after initial sound loading
    // Behavior: Only affects sounds that loaded successfully (optional check)
    // Range: Applies soundVolume (0.0f-100.0f) directly to SFML sound objects
    void updateVolume();
    
    //=== AUDIO PLAYBACK INTERFACE ===
    // Methods for triggering specific UI sound effects with intelligent behavior
    
    // Plays hover/selection change sound with immediate replacement
    // Usage: Menu navigation, mouse hover events, keyboard selection changes
    // Behavior: Stops any currently playing hover sound before starting new playback
    // Purpose: Provides responsive audio feedback without sound overlap
    void playHover();
    
    // Plays selection confirmation sound with immediate replacement  
    // Usage: Menu item activation, level transitions, settings confirmation
    // Behavior: Stops any currently playing select sound before starting new playback
    // Purpose: Provides clear confirmation feedback for positive actions
    void playSelect();
    
    // Plays back navigation sound with immediate replacement
    // Usage: Return to previous menu, cancel operations, ESC key presses
    // Behavior: Stops any currently playing back sound before starting new playback  
    // Purpose: Provides audio feedback for navigation backward/cancellation
    void playBack();
    
    // Plays error notification sound with immediate replacement
    // Usage: Invalid operations, boundary conditions, failed actions
    // Behavior: Stops any currently playing error sound before starting new playback
    // Purpose: Provides negative feedback for invalid user actions
    // Note: Currently disabled due to commented-out buffer loading
    void playError();
};

//=== GLOBAL INSTANCE DECLARATION ===
// External declaration of the global navigation sounds instance
// Defined in: FS1.1.cpp (actual instantiation)
// Scope: Available throughout the entire application
// Usage: Accessed via 'navSounds' identifier in all source files
// Lifetime: Exists for entire application duration
// Thread Safety: Single-threaded access only
extern NavigationSounds navSounds;

//=== SYSTEM ARCHITECTURE NOTES ===
//
// Design Principles:
// - Centralized audio management for consistent UI feedback
// - Graceful degradation when audio files are missing
// - Immediate sound replacement prevents audio overlap and lag
// - Volume control integrated with global settings system
// - Optional-based design allows for partial loading success
//
// Performance Considerations:  
// - Sound buffers loaded once and reused for all playback
// - Minimal memory footprint with optional sound instances
// - Immediate stop/play prevents memory leaks from overlapping sounds
// - Volume updates only applied to successfully loaded sounds
//
// Error Handling Strategy:
// - Individual sound loading failures don't prevent system operation
// - Master soundsLoaded flag guards all playback operations
// - Console warnings for missing files aid in debugging
// - System continues functioning with reduced audio feedback
//
// Integration Points:
// - Volume synchronized with global musicVolume setting (80% ratio)
// - Called from all menu systems and navigation interfaces
// - Consistent behavior across different game states and contexts
// - F1 settings access provides real-time volume adjustment