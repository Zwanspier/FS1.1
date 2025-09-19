#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <random>
#include <stack>
#include <algorithm>

using namespace sf;
using namespace std;

//=== MAZE CLASS DECLARATION ===
// The Maze class handles complete maze functionality including:
// - Procedural maze generation using recursive backtracking
// - Smooth player movement with collision detection
// - Dynamic rendering with adjustable wall visibility
// - Win condition detection and exit management
class Maze {
public:
    //=== CONSTRUCTOR ===
    // Construct a maze sized to fit within specified screen dimensions
    // Parameters:
    //   screenWidth - Available width in pixels for maze rendering
    //   screenHeight - Available height in pixels for maze rendering  
    //   cellSize - Size of each maze cell in pixels (determines maze complexity)
    Maze(int screenWidth, int screenHeight, int cellSize);

    //=== MAZE GENERATION SYSTEM ===
    // Generate a new random maze layout using recursive backtracking algorithm
    // Creates a perfect maze (no loops, single path between any two points)
    // Ensures exit is always reachable from starting position
    void generate();

    //=== RENDERING SYSTEM ===
    // Draw the complete maze structure including walls and exit marker
    // Uses gamma setting from SettingsState for dynamic wall visibility
    void draw(RenderWindow& window);

    // Draw the player at their current smooth pixel position
    // Renders player as a red circle with appropriate scaling
    void drawPlayer(RenderWindow& window);

    //=== PLAYER MOVEMENT SYSTEM ===
    // Update player position smoothly based on input and collision detection
    // Parameters:
    //   deltaTime - Time elapsed since last update (for frame-rate independent movement)
    //   up, down, left, right - Boolean input states for movement directions
    // Features:
    //   - Smooth pixel-based movement within grid cells
    //   - Diagonal movement normalization for consistent speed
    //   - Wall collision detection and response
    //   - Separate X/Y movement processing for better collision handling
    void updatePlayer(float deltaTime, bool up, bool down, bool left, bool right);

    //=== POSITION QUERY SYSTEM ===
    // Get the player's current cell position within the maze grid
    // Returns: Vector2i containing grid coordinates (not pixel coordinates)
    Vector2i getPlayerPosition() const { return playerPos; }

    //=== WIN CONDITION SYSTEM ===
    // Check if the player has reached the exit cell (bottom-right corner)
    // Returns: true if player is at exit, false otherwise
    bool isAtExit() const { return playerPos.x == width - 1 && playerPos.y == height - 1; }

private:
    //=== CELL STRUCTURE DEFINITION ===
    // Represents a single cell in the maze grid
    struct Cell {
        bool visited = false;                           // Used during maze generation algorithm
        bool walls[4] = { true, true, true, true };     // Wall states: [top, right, bottom, left]
    };

    //=== MAZE PROPERTIES ===
    int width, height, cellSize;           // Maze dimensions in cells and pixel size per cell
    vector<vector<Cell>> grid;             // 2D grid of maze cells containing wall information

    //=== PLAYER STATE ===
    Vector2i playerPos;                    // Player's current cell position (grid coordinates)
    Vector2f playerPixelPos;               // Player's smooth pixel position (for sub-cell movement)
    CircleShape player;                    // SFML shape for rendering the player
    float playerSpeed = 200.0f;            // Player movement speed in pixels per second

    //=== COLLISION DETECTION SYSTEM ===
    // Check if the player can move to a given pixel position without colliding with walls
    // Parameters:
    //   x, y - Target pixel coordinates to test
    // Returns: true if position is valid (no collision), false if blocked by walls
    // Features:
    //   - Circular collision detection using player radius
    //   - Multi-cell overlap checking for large player sprites
    //   - Epsilon tolerance for precise wall collision
    //   - Boundary checking to prevent movement outside maze
    bool canMoveTo(float x, float y);

    //=== MAZE GENERATION HELPER FUNCTIONS ===
    
    // Remove the wall between two adjacent cells during maze generation
    // Parameters:
    //   a, b - References to the two cells to connect
    //   ax, ay - Grid coordinates of first cell
    //   bx, by - Grid coordinates of second cell
    // Updates both cells' wall arrays to create passage between them
    void removeWall(Cell& a, Cell& b, int ax, int ay, int bx, int by);

    // Get a list of unvisited neighboring cells for maze generation algorithm
    // Parameters:
    //   x, y - Grid coordinates of current cell
    // Returns: Vector of coordinate pairs representing unvisited neighbors
    // Used by recursive backtracking to determine next generation step
    vector<pair<int, int>> getUnvisitedNeighbors(int x, int y);
};