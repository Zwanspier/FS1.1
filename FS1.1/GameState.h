#pragma once

//=== GAME STATE ENUMERATION ===
// Defines all possible states of the application for state machine management
// Each state represents a distinct mode of operation with specific functionality and UI
// States are used to control game flow, rendering, input handling, and transitions

enum GameState {
    //=== INTRODUCTION STATE ===
    // Initial game introduction screen explaining the purpose and concept
    // Features: Game overview, narrative setup, continue prompt
    // Purpose: Orient new players and establish game context
    // Transition: To MENU (continue to main menu)
    INTRODUCTION,
    
    //=== MAIN MENU STATE ===
    // Primary application entry point and navigation hub
    // Features: Menu selection, background music, settings access
    // Input: Mouse/keyboard navigation, selection confirmation
    // Transitions: To PRELEVEL1 (start game), SETTINGS (configure options), EXIT (quit)
    MENU,
    
    //=== PRE-LEVEL TRANSITION STATES ===
    // Intermediate states that provide level introductions and control instructions
    // These states serve as buffers between menu/levels and provide player orientation
    // Players can return to these screens from their respective levels using ESC key
    
    PRELEVEL1,    // Pre-level screen for level 1 (scrolling text game)
                  // Displays: Level title, control instructions, continue prompt
                  // Purpose: Introduce Level 1 mechanics and controls
                  // Transition: To PLAYING (start Level 1), From PLAYING (ESC key)
    
    PRELEVEL2,    // Pre-level screen for level 2 (maze navigation)
                  // Displays: Level title, movement controls, objective description
                  // Purpose: Introduce maze mechanics and navigation
                  // Transition: To PLAYING2 (start Level 2), From PLAYING2 (ESC key)
    
    PRELEVEL3,    // Pre-level screen for level 3 (driving game)
                  // Displays: Level title, driving controls, game mechanics
                  // Purpose: Introduce driving mechanics and special features
                  // Transition: To PLAYING3 (start Level 3), From PLAYING3 (ESC key)
    
    //=== ACTIVE GAMEPLAY STATES ===
    // States where primary game mechanics are active and players interact with content
    // Each level can return to its corresponding pre-level screen using ESC key
    
    PLAYING,      // Level 1: Scrolling Text Game
                  // Mechanics: Dynamic key selection, text scrolling, speed adjustment
                  // Objective: Press randomly selected key to advance
                  // Features: Configurable scroll speed, fullscreen text patterns
                  // Transition: To PRELEVEL2 (advance to next level), To PRELEVEL1 (ESC key)
    
    PLAYING2,     // Level 2: Maze Navigation Game  
                  // Mechanics: Smooth player movement, collision detection, maze generation
                  // Objective: Navigate through procedurally generated maze to exit
                  // Features: Adjustable maze size, wall visibility, win detection
                  // Transition: To PRELEVEL3 (advance to next level), To PRELEVEL2 (ESC key)
    
    PLAYING3,     // Level 3: Driving/Racing Game
                  // Mechanics: Vehicle control, obstacle avoidance, speed management
                  // Objective: Drive through obstacles, special narrative elements
                  // Features: Dynamic audio, background music interaction, alternative endings
                  // Transition: Back to MENU (completion or exit), To PRELEVEL3 (ESC key)
    
    //=== SYSTEM MANAGEMENT STATES ===
    // States for application configuration and system management
    
    SETTINGS,     // Settings and Configuration Menu
                  // Features: Graphics settings, audio controls, gameplay options
                  // Controls: VSync, framerate, gamma, maze size, music volume
                  // Purpose: Allow players to customize experience and performance
                  // Transition: Returns to previousState (stored before entering settings)
    
    EXIT          // Application Termination State
                  // Purpose: Signal application shutdown
                  // Behavior: Triggers window.close() and cleanup procedures
                  // Note: This state is typically not rendered, just processed
};

//=== STATE MACHINE NOTES ===
// 
// State Flow Overview:
// INTRODUCTION ? MENU ? PRELEVEL1 ? PLAYING ? PRELEVEL2 ? PLAYING2 ? PRELEVEL3 ? PLAYING3 ? MENU
//     ?            ?                                                                           ?
// SETTINGS ???????????????????????????????????????????????????????????????????????????????????
//
// Key Design Principles:
// - INTRODUCTION provides initial context and game overview for new players
// - Each state is self-contained with specific responsibilities
// - PRELEVEL states provide smooth transitions and player preparation
// - Players can return from levels to their pre-level screens using ESC key (? indicates bidirectional)
// - SETTINGS can be accessed from any state via F1 key (except during gameplay)
// - State transitions are controlled by user input and game completion conditions
// - Previous state tracking enables proper return from settings menu
//
// Enhanced Navigation Features:
// - ESC key in PLAYING returns to PRELEVEL1 (allows level restart/instruction review)
// - ESC key in PLAYING2 returns to PRELEVEL2 (allows level restart/instruction review)
// - ESC key in PLAYING3 returns to PRELEVEL3 (allows level restart/instruction review)
// - This provides players with easy access to instructions without losing progress
//
// State Persistence:
// - Static variables in each state handler maintain state between frames
// - Game data (scores, progress) is preserved during state transitions
// - Settings changes are applied globally and persist across state changes
//
// Thread Safety:
// - Single-threaded design with frame-based state processing
// - State changes occur at frame boundaries for consistency
// - No concurrent state access or modification