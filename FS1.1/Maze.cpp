#include "Maze.h"
#include <random>
#include <stack>
#include <algorithm>

using namespace sf;
using namespace std;

// Maze constructor: initializes grid and player.
Maze::Maze(int screenWidth, int screenHeight, int cs) : cellSize(cs) {
    width = screenWidth / cellSize;
    height = screenHeight / cellSize;
    grid = vector<vector<Cell>>(height, vector<Cell>(width));
    
    // Start player at the center of the top-left cell.
    playerPos = Vector2i(0, 0);
    playerPixelPos = Vector2f(cellSize / 2.0f, cellSize / 2.0f);
    player.setRadius(cellSize / 4.0f);
    player.setFillColor(Color::Red);
    player.setOrigin(Vector2f(player.getRadius(), player.getRadius()));
    
    // Use a consistent speed that feels good regardless of cell size
    // Speed is in cells per second rather than pixels per second
    playerSpeed = 4.0f * cellSize; // 4 cells per second, scaled to pixels
}

// Generate a new random maze using recursive backtracking.
void Maze::generate() {
    stack<pair<int, int>> stack;
    int x = 0, y = 0;
    grid[y][x].visited = true;
    stack.push({x, y});

    random_device rd;
    mt19937 g(rd()); // Random seed for a new maze every run

    // Main maze generation loop.
    while (!stack.empty()) {
        auto [cx, cy] = stack.top();
        auto neighbors = getUnvisitedNeighbors(cx, cy);

        if (!neighbors.empty()) {   
            uniform_int_distribution<> dist(0, neighbors.size() - 1);
            auto [nx, ny] = neighbors[dist(g)];
            removeWall(grid[cy][cx], grid[ny][nx], cx, cy, nx, ny);
            grid[ny][nx].visited = true;
            stack.push({nx, ny});
        } else {
            stack.pop();
        }
    }
    
    // Ensure exit is always reachable by removing walls to the left and above the exit.
    if (width > 1) {
        grid[height-1][width-1].walls[3] = false; // left wall of exit
        grid[height-1][width-2].walls[1] = false; // right wall of neighbor
    }
    if (height > 1) {
        grid[height-1][width-1].walls[0] = false; // top wall of exit
        grid[height-2][width-1].walls[2] = false; // bottom wall of neighbor
    }
}

// Smoothly update the player's position based on input and delta time.
void Maze::updatePlayer(float deltaTime, bool up, bool down, bool left, bool right) {
    Vector2f movement(0, 0);
    
    // Set movement direction based on input.
    if (up) movement.y -= 1;
    if (down) movement.y += 1;
    if (left) movement.x -= 1;
    if (right) movement.x += 1;
    
    // Instantly stop if no keys are pressed.
    if (movement.x == 0 && movement.y == 0) {
        return;
    }
    
    // Normalize diagonal movement.
    if (movement.x != 0 && movement.y != 0) {
        movement.x *= 0.707f; // 1/sqrt(2)
        movement.y *= 0.707f;
    }
    
    // Calculate the desired movement for this frame
    Vector2f desiredMovement = movement * playerSpeed * deltaTime;
    
    // Try to move in both X and Y directions separately for smoother collision handling
    Vector2f newPos = playerPixelPos;
    
    // Try horizontal movement first
    if (desiredMovement.x != 0) {
        Vector2f testPos = Vector2f(playerPixelPos.x + desiredMovement.x, playerPixelPos.y);
        if (canMoveTo(testPos.x, testPos.y)) {
            newPos.x = testPos.x;
        }
    }
    
    // Then try vertical movement
    if (desiredMovement.y != 0) {
        Vector2f testPos = Vector2f(newPos.x, playerPixelPos.y + desiredMovement.y);
        if (canMoveTo(testPos.x, testPos.y)) {
            newPos.y = testPos.y;
        }
    }
    
    // Update position
    playerPixelPos = newPos;
    
    // Update grid position for logic (e.g., win detection).
    playerPos.x = static_cast<int>(playerPixelPos.x / cellSize);
    playerPos.y = static_cast<int>(playerPixelPos.y / cellSize);
}

// Collision detection: check if the player can move to a given pixel position.
bool Maze::canMoveTo(float x, float y) {
    float radius = player.getRadius();
    
    // Check bounds of the maze.
    if (x - radius < 0 || x + radius >= width * cellSize ||
        y - radius < 0 || y + radius >= height * cellSize) {
        return false;
    }
    
    // Determine which cells the player would overlap.
    int leftCell = static_cast<int>((x - radius) / cellSize);
    int rightCell = static_cast<int>((x + radius) / cellSize);
    int topCell = static_cast<int>((y - radius) / cellSize);
    int bottomCell = static_cast<int>((y + radius) / cellSize);
    
    // Clamp cell indices to valid range
    leftCell = max(0, min(width - 1, leftCell));
    rightCell = max(0, min(width - 1, rightCell));
    topCell = max(0, min(height - 1, topCell));
    bottomCell = max(0, min(height - 1, bottomCell));
    
    // Check for wall collisions in all overlapping cells.
    for (int cy = topCell; cy <= bottomCell; ++cy) {
        for (int cx = leftCell; cx <= rightCell; ++cx) {
            float cellLeft = cx * cellSize;
            float cellRight = (cx + 1) * cellSize;
            float cellTop = cy * cellSize;
            float cellBottom = (cy + 1) * cellSize;
            
            // Add small epsilon for more precise collision detection
            const float epsilon = 0.1f;
            
            // Check each wall for collision with epsilon tolerance
            if (grid[cy][cx].walls[0] && y - radius < cellTop + epsilon && x + radius > cellLeft && x - radius < cellRight) {
                return false; // Top wall
            }
            if (grid[cy][cx].walls[1] && x + radius > cellRight - epsilon && y + radius > cellTop && y - radius < cellBottom) {
                return false; // Right wall
            }
            if (grid[cy][cx].walls[2] && y + radius > cellBottom - epsilon && x + radius > cellLeft && x - radius < cellRight) {
                return false; // Bottom wall
            }
            if (grid[cy][cx].walls[3] && x - radius < cellLeft + epsilon && y + radius > cellTop && y - radius < cellBottom) {
                return false; // Left wall
            }
        }
    }
    return true;
}

// Get a list of unvisited neighbors for maze generation.
vector<pair<int, int>> Maze::getUnvisitedNeighbors(int x, int y) {
    vector<pair<int, int>> neighbors;
    if (y > 0 && !grid[y-1][x].visited) neighbors.push_back({x, y-1});
    if (x < width-1 && !grid[y][x+1].visited) neighbors.push_back({x+1, y});
    if (y < height-1 && !grid[y+1][x].visited) neighbors.push_back({x, y+1});
    if (x > 0 && !grid[y][x-1].visited) neighbors.push_back({x-1, y});
    return neighbors;
}

// Remove the wall between two adjacent cells.
void Maze::removeWall(Cell& a, Cell& b, int ax, int ay, int bx, int by) {
    if (ax == bx) {
        if (ay > by) { a.walls[0] = false; b.walls[2] = false; }
        else { a.walls[2] = false; b.walls[0] = false; }
    } else if (ay == by) {
        if (ax > bx) { a.walls[3] = false; b.walls[1] = false; }
        else { a.walls[1] = false; b.walls[3] = false; }
    }
}

// Draw the maze walls and the exit marker.
void Maze::draw(RenderWindow& window) {
    extern float gamma; // Access gamma from SettingsState
    
    float thickness = 2.0f;
    // Wall color is determined by the fake gamma setting.
    int brightness = static_cast<int>((gamma / 2.0f) * 255.0f);
    brightness = max(0, min(255, brightness));
    Color wallColor(brightness, brightness, brightness);
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int px = x * cellSize;
            int py = y * cellSize;
            if (grid[y][x].walls[0]) { // top
                RectangleShape wall(Vector2f(cellSize, thickness));
                wall.setPosition(Vector2f(px, py));
                wall.setFillColor(wallColor);
                window.draw(wall);
            }
            if (grid[y][x].walls[1]) { // right
                RectangleShape wall(Vector2f(thickness, cellSize));
                wall.setPosition(Vector2f(px + cellSize - thickness, py));
                wall.setFillColor(wallColor);
                window.draw(wall);
            }
            if (grid[y][x].walls[2]) { // bottom
                RectangleShape wall(Vector2f(cellSize, thickness));
                wall.setPosition(Vector2f(px, py + cellSize - thickness));
                wall.setFillColor(wallColor);
                window.draw(wall);
            }
            if (grid[y][x].walls[3]) { // left
                RectangleShape wall(Vector2f(thickness, cellSize));
                wall.setPosition(Vector2f(px, py));
                wall.setFillColor(wallColor);
                window.draw(wall);
            }
        }
    }
    
    // Draw exit marker (always visible)
    RectangleShape exit(Vector2f(cellSize - 4, cellSize - 4));
    exit.setPosition(Vector2f((width - 1) * cellSize + 2, (height - 1) * cellSize + 2));
    exit.setFillColor(Color::Green);
    window.draw(exit);
}

// Draw the player at its current pixel position.
void Maze::drawPlayer(RenderWindow& window) {
    player.setPosition(playerPixelPos);
    window.draw(player);
}