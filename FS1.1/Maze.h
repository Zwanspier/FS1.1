#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

// The Maze class handles maze generation, rendering, and player movement.
class Maze {
public:
    // Construct a maze sized to the screen, with a given cell size.
    Maze(int screenWidth, int screenHeight, int cellSize);

    // Generate a new random maze.
    void generate();

    // Draw the maze walls.
    void draw(RenderWindow& window);

    // Draw the player at its current position.
    void drawPlayer(RenderWindow& window);

    // Update player position smoothly based on input and delta time.
    void updatePlayer(float deltaTime, bool up, bool down, bool left, bool right);

    // Get the player's current cell position.
    Vector2i getPlayerPosition() const { return playerPos; }

    // Check if the player is at the exit cell.
    bool isAtExit() const { return playerPos.x == width - 1 && playerPos.y == height - 1; }

private:
    // Represents a single cell in the maze.
    struct Cell {
        bool visited = false;              // Used during maze generation.
        bool walls[4] = { true, true, true, true }; // top, right, bottom, left
    };

    int width, height, cellSize;           // Maze dimensions and cell size.
    vector<vector<Cell>> grid;             // 2D grid of maze cells.
    Vector2i playerPos;                    // Player's current cell position.
    Vector2f playerPixelPos;               // Player's smooth pixel position.
    CircleShape player;                    // SFML shape for rendering the player.
    float playerSpeed = 200.0f;            // Player movement speed in pixels/sec.

    // Check if the player can move to a given pixel position (collision detection).
    bool canMoveTo(float x, float y);

    // Remove the wall between two adjacent cells.
    void removeWall(Cell& a, Cell& b, int ax, int ay, int bx, int by);

    // Get a list of unvisited neighbors for maze generation.
    vector<pair<int, int>> getUnvisitedNeighbors(int x, int y);
};