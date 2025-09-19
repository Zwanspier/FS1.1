#include "Playingstate2.h"

using namespace sf;
using namespace std;

//=== RESOLUTION-BASED MAZE SIZING SYSTEM ===
// Calculates optimal maze dimensions based on current display resolution
// This ensures the maze scales appropriately for different screen sizes
Vector2u getMazeDimensions() {
    // Access global resolution settings
    extern int resolutionIndex;            // Current resolution selection index
    extern vector<Vector2u> resolutionOptions;  // Available resolution options
    
    // Retrieve selected resolution from settings
    auto mazeSize = resolutionOptions[resolutionIndex];
    
    // Convert screen resolution to maze cell count
    // Scaling factors determine maze complexity relative to screen size
    int mazeCellsX = mazeSize.x / 40;  // Horizontal cell count (adjustable divisor)
    int mazeCellsY = mazeSize.y / 36;  // Vertical cell count (adjustable divisor)
    
    // Apply bounds to ensure reasonable maze sizes
    // Prevents mazes that are too small (unplayable) or too large (performance issues)
    mazeCellsX = max(10, min(100, mazeCellsX));  // Range: 10-100 cells wide
    mazeCellsY = max(8, min(75, mazeCellsY));    // Range: 8-75 cells tall
    
    return Vector2u(mazeCellsX, mazeCellsY);
}

//=== MAIN GAME LOOP FUNCTION ===
// Handles all logic and rendering for PlayingState2 (Level 2: Dark Maze)
// Features smooth player movement and dynamic maze generation
void handlePlayingState2(RenderWindow& window, bool& running, GameState& state)
{
    //=== DYNAMIC MAZE SIZING SYSTEM ===
    // Track maze dimension changes to trigger regeneration when needed
    static Vector2u lastMazeDims = getMazeDimensions();  // Previous frame's dimensions
    Vector2u currentMazeDims = getMazeDimensions();      // Current frame's dimensions
    
    // Calculate cell size to fit maze optimally within window bounds
    // Ensures maze uses available screen space efficiently
    static int cellSize = min(window.getSize().x / currentMazeDims.x, window.getSize().y / currentMazeDims.y);
    
    //=== MAZE MANAGEMENT SYSTEM ===
    // Static maze object maintains state between function calls
    static Maze maze(currentMazeDims.x * cellSize, currentMazeDims.y * cellSize, cellSize);
    static bool generated = false;     // Has maze been generated this session?
    static Clock clock;               // Frame timing for smooth movement
    extern bool mazeNeedsRegeneration; // External flag for settings-triggered regeneration
    
    //=== MAZE REGENERATION LOGIC ===
    // Recreate maze when resolution changes or settings request regeneration
    if (mazeNeedsRegeneration || currentMazeDims.x != lastMazeDims.x || currentMazeDims.y != lastMazeDims.y) {
        // Recalculate optimal cell size for new dimensions
        cellSize = min(window.getSize().x / currentMazeDims.x, window.getSize().y / currentMazeDims.y);
        
        // Create new maze instance with updated parameters
        maze = Maze(currentMazeDims.x * cellSize, currentMazeDims.y * cellSize, cellSize);
        maze.generate();              // Generate new maze layout
        generated = true;             // Mark as generated
        clock.restart();              // Reset timing
        mazeNeedsRegeneration = false; // Clear regeneration flag
        lastMazeDims = currentMazeDims; // Update dimension tracking
    }
    
    //=== INITIAL MAZE GENERATION ===
    // Generate maze on first run if not already created
    if (!generated) {
        maze.generate();    // Create maze layout
        generated = true;   // Mark as generated
        clock.restart();    // Initialize timing
    }
    
    //=== FRAME TIMING SYSTEM ===
    // Calculate time elapsed since last frame for smooth movement
    float deltaTime = clock.restart().asSeconds();
    
    //=== INPUT PROCESSING SYSTEM ===
    // Capture continuous input states for smooth player movement
    // Supports both WASD and arrow key layouts
    bool up = Keyboard::isKeyPressed(Keyboard::Key::W) || Keyboard::isKeyPressed(Keyboard::Key::Up);
    bool down = Keyboard::isKeyPressed(Keyboard::Key::S) || Keyboard::isKeyPressed(Keyboard::Key::Down);
    bool left = Keyboard::isKeyPressed(Keyboard::Key::A) || Keyboard::isKeyPressed(Keyboard::Key::Left);
    bool right = Keyboard::isKeyPressed(Keyboard::Key::D) || Keyboard::isKeyPressed(Keyboard::Key::Right);
    
    //=== PLAYER MOVEMENT SYSTEM ===
    // Update player position based on input and collision detection
    // Maze handles movement validation and wall collision internally
    maze.updatePlayer(deltaTime, up, down, left, right);
    
    //=== RENDERING PIPELINE ===
    window.clear(Color::Black);  // Clear screen with black background
    maze.draw(window);           // Render maze walls and passages
    maze.drawPlayer(window);     // Render player sprite/shape
    
    //=== UI AND WIN CONDITION SYSTEM ===
    // Font loading for UI text display
    static Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        font.openFromFile("arial.ttf");  // Load standard font
        fontLoaded = true;               // Mark as loaded
    }
    
    // Display victory message when player reaches maze exit
    if (maze.isAtExit()) {
        Text winText(font, "You Win! Press ENTER for next level", 50);
        winText.setFillColor(Color::Red);           // Red text for visibility
        winText.setOutlineColor(Color::Black);      // Black outline for contrast
        winText.setOutlineThickness(2.f);           // Outline thickness
        
        // Center text on screen
        auto bounds = winText.getLocalBounds();
        winText.setOrigin(Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        winText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f));
        
        window.draw(winText);  // Render victory message
    }
    
    //=== NAVIGATION CONTROL SYSTEM ===
    // Handle state transitions and menu navigation
    
    // Return to main menu
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        state = MENU;
    }
    
    // Access settings menu
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        extern GameState previousState;
        previousState = PLAYING2;  // Store current state for return
        state = SETTINGS;          // Open settings menu
    }
    
    // Level progression - advance to next level when at exit
    if (maze.isAtExit() && Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
        state = PRELEVEL3;  // Go to pre-level screen before level 3
    }
    
    // Alternative progression method (shortcut key)
    if (Keyboard::isKeyPressed(Keyboard::Key::H)) {
        state = PRELEVEL3;  // Also advance to pre-level screen
    }
}