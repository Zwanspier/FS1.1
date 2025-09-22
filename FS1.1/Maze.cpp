#include "Maze.h"

using namespace sf;
using namespace std;

//=== MAZE CONSTRUCTOR ===
// Initializes maze structure and player components based on screen dimensions
// Sets up grid, player properties, and calculates optimal sizing
Maze::Maze(int screenWidth, int screenHeight, int cs) {
    initialize(screenWidth, screenHeight, cs);
}

//=== MAZE INITIALIZATION SYSTEM ===
// Common initialization logic used by constructor and resize
void Maze::initialize(int screenWidth, int screenHeight, int cs) {
    cellSize = cs;
    
    //=== MAZE DIMENSION CALCULATION ===
    // Calculate maze size in cells based on available screen space
    width = screenWidth / cellSize;   // Number of cells horizontally
    height = screenHeight / cellSize; // Number of cells vertically
    
    // Ensure minimum maze size to prevent crashes
    width = std::max(2, width);
    height = std::max(2, height);
    
    //=== GRID INITIALIZATION ===
    // Create 2D grid of cells, all initially unvisited with all walls intact
    grid = vector<vector<Cell>>(height, vector<Cell>(width));

    //=== PLAYER INITIALIZATION ===
    // Start player at the center of the top-left cell (maze entrance)
    playerPos = Vector2i(0, 0);                                    // Grid position
    playerPixelPos = Vector2f(static_cast<float>(cellSize) / 2.0f, static_cast<float>(cellSize) / 2.0f);  // Pixel position (cell center)
    
    //=== PLAYER VISUAL SETUP ===
    // Configure player appearance as a red circle
    player.setRadius(static_cast<float>(cellSize) / 4.0f);          // Radius scales with cell size
    player.setFillColor(Color::Red);            // Red color for visibility
    player.setOrigin(Vector2f(player.getRadius(), player.getRadius())); // Center origin for rotation

    //=== PLAYER MOVEMENT CONFIGURATION ===
    // Set movement speed to be consistent regardless of cell size
    // Speed is calculated as cells per second, then converted to pixels per second
    playerSpeed = 4.0f * static_cast<float>(cellSize); // 4 cells per second, scaled to pixels

    //=== TEXTURE LOADING ===
    // Always reload textures to ensure they're valid after resize
    loadTextures();
}

//=== MAZE RESIZE SYSTEM ===
// Safely resize the maze with new dimensions and regenerate
void Maze::resize(int screenWidth, int screenHeight, int cs) {
    // Clear existing grid to prevent any potential access issues
    grid.clear();
    
    // Reset texture state to force reload
    texturesLoaded = false;
    backgroundSprite.reset(); // Clear the optional sprite
    
    // Reinitialize with new dimensions
    initialize(screenWidth, screenHeight, cs);
    
    // Generate new maze layout
    generate();
}

//=== TEXTURE LOADING SYSTEM ===
// Load background and wall textures from files
bool Maze::loadTextures() {
    bool allLoaded = true;
    
    // Reset texture state
    texturesLoaded = false;
    backgroundSprite.reset();
    
    // Try to load background texture
    if (!backgroundTexture.loadFromFile("Images/maze_background.jpg")) {
        cerr << "Warning: Could not load maze background texture. Using default background." << endl;
        allLoaded = false;
    } else {
        // Set texture to repeat for tiling
        backgroundTexture.setRepeated(true);
        // Create sprite with the loaded texture
        backgroundSprite = Sprite(backgroundTexture);
        cout << "Loaded maze background texture successfully." << endl;
    }
    
    // Try to load wall texture
    if (!wallTexture.loadFromFile("Images/maze_wall.jpg")) {
        cerr << "Warning: Could not load wall texture. Using default walls." << endl;
        allLoaded = false;
    } else {
        // Set texture to repeat for tiling
        wallTexture.setRepeated(true);
        cout << "Loaded wall texture successfully." << endl;
    }
    
    texturesLoaded = allLoaded;
    return allLoaded;
}

//=== MAZE GENERATION SYSTEM ===
// Generates a new random maze using recursive backtracking algorithm
// Creates a perfect maze with exactly one path between any two points
void Maze::generate() {
    // Validate maze dimensions before generation
    if (width < 1 || height < 1) {
        cerr << "Error: Invalid maze dimensions for generation: " << width << "x" << height << endl;
        return;
    }
    
    // Validate grid size matches dimensions
    if (static_cast<int>(grid.size()) != height || 
        (height > 0 && static_cast<int>(grid[0].size()) != width)) {
        cerr << "Error: Grid size mismatch. Expected " << width << "x" << height 
             << ", got " << (grid.empty() ? 0 : grid[0].size()) << "x" << grid.size() << endl;
        return;
    }
    
    //=== GRID RESET ===
    // Reset all cells to unvisited state with all walls intact
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[y][x].visited = false;
            grid[y][x].walls[0] = true; // Top
            grid[y][x].walls[1] = true; // Right
            grid[y][x].walls[2] = true; // Bottom
            grid[y][x].walls[3] = true; // Left
        }
    }
    
    //=== ALGORITHM INITIALIZATION ===
    stack<pair<int, int>> stack;  // Stack for backtracking algorithm
    int x = 0, y = 0;             // Start generation from top-left corner
    grid[y][x].visited = true;    // Mark starting cell as visited
    stack.push({ x, y });         // Add starting position to stack

    //=== RANDOM NUMBER GENERATION SETUP ===
    random_device rd;             // Hardware random number generator
    mt19937 g(rd());             // Mersenne Twister generator with random seed

    //=== MAIN GENERATION LOOP ===
    // Continue until all reachable cells have been visited
    while (!stack.empty()) {
        auto [cx, cy] = stack.top();                    // Current cell coordinates
        auto neighbors = getUnvisitedNeighbors(cx, cy); // Get unvisited adjacent cells

        if (!neighbors.empty()) {
            //=== PATH CREATION ===
            // Choose random unvisited neighbor and create path to it
            uniform_int_distribution<> dist(0, static_cast<int>(neighbors.size()) - 1);
            auto [nx, ny] = neighbors[dist(g)];         // Random neighbor selection
            
            removeWall(grid[cy][cx], grid[ny][nx], cx, cy, nx, ny); // Connect cells
            grid[ny][nx].visited = true;                // Mark new cell as visited
            stack.push({ nx, ny });                     // Continue from new cell
        }
        else {
            //=== BACKTRACKING ===
            // No unvisited neighbors, backtrack to previous cell
            stack.pop();
        }
    }

    //=== EXIT ACCESSIBILITY GUARANTEE ===
    // Ensure exit is always reachable by removing strategic walls
    // This prevents impossible mazes due to generation edge cases
    
    if (width > 1) {
        // Remove wall between exit and cell to its left
        grid[height - 1][width - 1].walls[3] = false; // Left wall of exit cell
        grid[height - 1][width - 2].walls[1] = false; // Right wall of neighbor cell
    }
    if (height > 1) {
        // Remove wall between exit and cell above it
        grid[height - 1][width - 1].walls[0] = false; // Top wall of exit cell
        grid[height - 2][width - 1].walls[2] = false; // Bottom wall of neighbor cell
    }
}

//=== SMOOTH PLAYER MOVEMENT SYSTEM ===
// Updates player position based on input with collision detection and smooth movement
// Provides frame-rate independent movement with diagonal normalization
void Maze::updatePlayer(float deltaTime, bool up, bool down, bool left, bool right) {
    // Validate maze and player state
    if (width <= 0 || height <= 0 || grid.empty()) {
        return;
    }
    
    // Validate player position is within bounds
    if (playerPos.x < 0 || playerPos.x >= width || playerPos.y < 0 || playerPos.y >= height) {
        // Reset player to safe position if out of bounds
        playerPos = Vector2i(0, 0);
        playerPixelPos = Vector2f(static_cast<float>(cellSize) / 2.0f, static_cast<float>(cellSize) / 2.0f);
        return;
    }
    
    //=== INPUT PROCESSING ===
    // Convert boolean inputs to movement vector
    Vector2f movement(0, 0);

    // Set movement direction based on input states
    if (up) movement.y -= 1;      // Negative Y = upward movement
    if (down) movement.y += 1;    // Positive Y = downward movement
    if (left) movement.x -= 1;    // Negative X = leftward movement
    if (right) movement.x += 1;   // Positive X = rightward movement

    //=== EARLY EXIT FOR NO MOVEMENT ===
    // Stop immediately if no movement keys are pressed
    if (movement.x == 0 && movement.y == 0) {
        return;
    }

    //=== DIAGONAL MOVEMENT NORMALIZATION ===
    // Normalize diagonal movement to maintain consistent speed
    // Prevents faster movement when moving diagonally
    if (movement.x != 0 && movement.y != 0) {
        movement.x *= 0.707f; // 1/sqrt(2) - normalize to unit circle
        movement.y *= 0.707f; // 1/sqrt(2) - normalize to unit circle
    }

    //=== FRAME-RATE INDEPENDENT MOVEMENT ===
    // Calculate desired movement for this frame based on elapsed time
    Vector2f desiredMovement = movement * playerSpeed * deltaTime;

    //=== COLLISION-AWARE MOVEMENT SYSTEM ===
    // Try to move in both X and Y directions separately for smoother collision handling
    // This allows sliding along walls instead of getting completely stuck
    Vector2f newPos = playerPixelPos;

    //=== HORIZONTAL MOVEMENT PROCESSING ===
    if (desiredMovement.x != 0) {
        Vector2f testPos = Vector2f(playerPixelPos.x + desiredMovement.x, playerPixelPos.y);
        if (canMoveTo(testPos.x, testPos.y)) {
            newPos.x = testPos.x;  // Apply horizontal movement if valid
        }
    }

    //=== VERTICAL MOVEMENT PROCESSING ===
    if (desiredMovement.y != 0) {
        Vector2f testPos = Vector2f(newPos.x, playerPixelPos.y + desiredMovement.y);
        if (canMoveTo(testPos.x, testPos.y)) {
            newPos.y = testPos.y;  // Apply vertical movement if valid
        }
    }

    //=== POSITION UPDATE ===
    playerPixelPos = newPos;  // Update smooth pixel position

    //=== GRID POSITION SYNCHRONIZATION ===
    // Update grid position for logic systems (win detection, etc.)
    playerPos.x = static_cast<int>(playerPixelPos.x / static_cast<float>(cellSize));
    playerPos.y = static_cast<int>(playerPixelPos.y / static_cast<float>(cellSize));
    
    // Clamp grid position to valid bounds
    playerPos.x = std::max(0, std::min(width - 1, playerPos.x));
    playerPos.y = std::max(0, std::min(height - 1, playerPos.y));
}

//=== COLLISION DETECTION SYSTEM ===
// Comprehensive collision detection for circular player against maze walls
// Handles multi-cell overlaps and provides epsilon tolerance for precision
bool Maze::canMoveTo(float x, float y) {
    // Validate maze dimensions and grid
    if (width <= 0 || height <= 0 || grid.empty()) {
        return false;
    }
    
    float radius = player.getRadius();  // Get player's collision radius

    //=== BOUNDARY CHECKING ===
    // Ensure player stays within maze boundaries
    if (x - radius < 0 || x + radius >= static_cast<float>(width * cellSize) ||
        y - radius < 0 || y + radius >= static_cast<float>(height * cellSize)) {
        return false;  // Outside maze bounds
    }

    //=== MULTI-CELL OVERLAP CALCULATION ===
    // Determine which cells the player's circular collision area overlaps
    int leftCell = static_cast<int>((x - radius) / static_cast<float>(cellSize));   // Leftmost overlapped cell
    int rightCell = static_cast<int>((x + radius) / static_cast<float>(cellSize));  // Rightmost overlapped cell  
    int topCell = static_cast<int>((y - radius) / static_cast<float>(cellSize));    // Topmost overlapped cell
    int bottomCell = static_cast<int>((y + radius) / static_cast<float>(cellSize)); // Bottommost overlapped cell

    //=== CELL INDEX VALIDATION ===
    // Clamp cell indices to valid grid range to prevent array bounds errors
    leftCell = std::max(0, std::min(width - 1, leftCell));
    rightCell = std::max(0, std::min(width - 1, rightCell));
    topCell = std::max(0, std::min(height - 1, topCell));
    bottomCell = std::max(0, std::min(height - 1, bottomCell));

    //=== WALL COLLISION DETECTION ===
    // Check for wall collisions in all overlapping cells
    for (int cy = topCell; cy <= bottomCell; ++cy) {
        for (int cx = leftCell; cx <= rightCell; ++cx) {
            // Additional bounds check to prevent crashes
            if (cy < 0 || cy >= height || cx < 0 || cx >= width || 
                cy >= static_cast<int>(grid.size()) || 
                cx >= static_cast<int>(grid[cy].size())) {
                continue;
            }
            
            //=== CELL BOUNDARY CALCULATION ===
            float cellLeft = static_cast<float>(cx * cellSize);        // Left edge of current cell
            float cellRight = static_cast<float>((cx + 1) * cellSize); // Right edge of current cell
            float cellTop = static_cast<float>(cy * cellSize);         // Top edge of current cell
            float cellBottom = static_cast<float>((cy + 1) * cellSize); // Bottom edge of current cell

            //=== PRECISION TOLERANCE ===
            // Small epsilon for more precise collision detection
            // Prevents floating-point precision issues
            const float epsilon = 0.1f;

            //=== INDIVIDUAL WALL COLLISION CHECKS ===
            // Check each wall of the current cell for collision with player circle
            
            // Top wall collision
            if (grid[cy][cx].walls[0] && y - radius < cellTop + epsilon && 
                x + radius > cellLeft && x - radius < cellRight) {
                return false;
            }
            
            // Right wall collision
            if (grid[cy][cx].walls[1] && x + radius > cellRight - epsilon && 
                y + radius > cellTop && y - radius < cellBottom) {
                return false;
            }
            
            // Bottom wall collision
            if (grid[cy][cx].walls[2] && y + radius > cellBottom - epsilon && 
                x + radius > cellLeft && x - radius < cellRight) {
                return false;
            }
            
            // Left wall collision
            if (grid[cy][cx].walls[3] && x - radius < cellLeft + epsilon && 
                y + radius > cellTop && y - radius < cellBottom) {
                return false;
            }
        }
    }
    
    return true;  // No collisions detected, movement is valid
}

//=== MAZE GENERATION HELPER FUNCTIONS ===

// Gets list of unvisited neighboring cells for maze generation algorithm
// Used by recursive backtracking to determine possible expansion directions
vector<pair<int, int>> Maze::getUnvisitedNeighbors(int x, int y) {
    vector<pair<int, int>> neighbors;
    
    // Additional bounds checking
    if (x < 0 || x >= width || y < 0 || y >= height || grid.empty()) {
        return neighbors;
    }
    
    //=== NEIGHBOR VALIDATION AND COLLECTION ===
    // Check each cardinal direction for valid, unvisited neighbors
    
    // North neighbor (up)
    if (y > 0 && !grid[y - 1][x].visited) 
        neighbors.push_back({ x, y - 1 });
    
    // East neighbor (right)  
    if (x < width - 1 && !grid[y][x + 1].visited) 
        neighbors.push_back({ x + 1, y });
    
    // South neighbor (down)
    if (y < height - 1 && !grid[y + 1][x].visited) 
        neighbors.push_back({ x, y + 1 });
    
    // West neighbor (left)
    if (x > 0 && !grid[y][x - 1].visited) 
        neighbors.push_back({ x - 1, y });
    
    return neighbors;
}

//=== WALL REMOVAL SYSTEM ===
// Removes walls between two adjacent cells to create passages during maze generation
// Updates both cells' wall arrays to maintain consistency
void Maze::removeWall(Cell& a, Cell& b, int ax, int ay, int bx, int by) {
    //=== VERTICAL ALIGNMENT (SAME COLUMN) ===
    if (ax == bx) {
        if (ay > by) { 
            // Cell A is below cell B
            a.walls[0] = false; // Remove top wall of cell A
            b.walls[2] = false; // Remove bottom wall of cell B
        }
        else { 
            // Cell A is above cell B
            a.walls[2] = false; // Remove bottom wall of cell A
            b.walls[0] = false; // Remove top wall of cell B
        }
    }
    //=== HORIZONTAL ALIGNMENT (SAME ROW) ===
    else if (ay == by) {
        if (ax > bx) { 
            // Cell A is right of cell B
            a.walls[3] = false; // Remove left wall of cell A
            b.walls[1] = false; // Remove right wall of cell B
        }
        else { 
            // Cell A is left of cell B
            a.walls[1] = false; // Remove right wall of cell A
            b.walls[3] = false; // Remove left wall of cell B
        }
    }
}

//=== MAZE RENDERING SYSTEM ===
// Draws the complete maze structure including background, walls and exit marker  
// Uses dynamic wall brightness based on gamma setting and textures when available
void Maze::draw(RenderWindow& window) {
    // Validate maze dimensions before rendering
    if (width <= 0 || height <= 0 || grid.empty()) {
        return;
    }
    
    //=== EXTERNAL SETTINGS ACCESS ===
    extern float gamma; // Access gamma setting from SettingsState

    //=== BACKGROUND RENDERING ===
    // Only render background if textures are valid and loaded
    if (texturesLoaded && backgroundSprite.has_value()) {
        // Additional validation that the texture is valid
        if (backgroundTexture.getSize().x > 0 && backgroundTexture.getSize().y > 0) {
            // Calculate how many times to tile the background texture
            int mazePixelWidth = width * cellSize;
            int mazePixelHeight = height * cellSize;
            
            // Set texture rectangle to cover entire maze area
            backgroundSprite->setTextureRect(IntRect({0, 0}, {mazePixelWidth, mazePixelHeight}));
            backgroundSprite->setPosition(Vector2f(0.0f, 0.0f));
            window.draw(*backgroundSprite);
        }
    }

    //=== WALL RENDERING CONFIGURATION ===
    // Check if walls should be visible (gamma > 0)
    bool wallsVisible = (gamma > 0.0f);
    
    if (wallsVisible) {
        float thickness = 2.0f;  // Wall thickness in pixels
        
        // Calculate wall color based on gamma setting (0.0f = invisible, 2.0f = white)
        int brightness = static_cast<int>((gamma / 2.0f) * 255.0f);
        brightness = std::max(0, std::min(255, brightness));          // Clamp to valid RGB range
        Color wallColor(brightness, brightness, brightness); // Grayscale color - use int values directly

        //=== WALL RENDERING LOOP ===
        // Draw walls for each cell in the maze grid
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Additional bounds check
                if (y >= static_cast<int>(grid.size()) || x >= static_cast<int>(grid[y].size())) {
                    continue;
                }
                
                float px = static_cast<float>(x * cellSize);  // Pixel X coordinate of cell
                float py = static_cast<float>(y * cellSize);  // Pixel Y coordinate of cell
                
                //=== INDIVIDUAL WALL RENDERING ===
                // Draw each wall if it exists in the current cell
                
                if (grid[y][x].walls[0]) { // Top wall
                    RectangleShape wall(Vector2f(static_cast<float>(cellSize), thickness));
                    wall.setPosition(Vector2f(px, py));
                    
                    // Only use texture if textures are loaded and wall texture is valid
                    if (texturesLoaded && wallTexture.getSize().x > 0 && wallTexture.getSize().y > 0) {
                        // Use texture for wall
                        wall.setTexture(&wallTexture);
                        wall.setTextureRect(IntRect({0, 0}, {cellSize, static_cast<int>(thickness)}));
                        // Apply gamma as a color overlay
                        wall.setFillColor(Color(brightness, brightness, brightness, 255));
                    } else {
                        // Use solid color for wall
                        wall.setFillColor(wallColor);
                    }
                    window.draw(wall);
                }
                if (grid[y][x].walls[1]) { // Right wall
                    RectangleShape wall(Vector2f(thickness, static_cast<float>(cellSize)));
                    wall.setPosition(Vector2f(px + static_cast<float>(cellSize) - thickness, py));
                    
                    if (texturesLoaded && wallTexture.getSize().x > 0 && wallTexture.getSize().y > 0) {
                        wall.setTexture(&wallTexture);
                        wall.setTextureRect(IntRect({0, 0}, {static_cast<int>(thickness), cellSize}));
                        wall.setFillColor(Color(brightness, brightness, brightness, 255));
                    } else {
                        wall.setFillColor(wallColor);
                    }
                    window.draw(wall);
                }
                if (grid[y][x].walls[2]) { // Bottom wall
                    RectangleShape wall(Vector2f(static_cast<float>(cellSize), thickness));
                    wall.setPosition(Vector2f(px, py + static_cast<float>(cellSize) - thickness));
                    
                    if (texturesLoaded && wallTexture.getSize().x > 0 && wallTexture.getSize().y > 0) {
                        wall.setTexture(&wallTexture);
                        wall.setTextureRect(IntRect({0, 0}, {cellSize, static_cast<int>(thickness)}));
                        wall.setFillColor(Color(brightness, brightness, brightness, 255));
                    } else {
                        wall.setFillColor(wallColor);
                    }
                    window.draw(wall);
                }
                if (grid[y][x].walls[3]) { // Left wall
                    RectangleShape wall(Vector2f(thickness, static_cast<float>(cellSize)));
                    wall.setPosition(Vector2f(px, py));
                    
                    if (texturesLoaded && wallTexture.getSize().x > 0 && wallTexture.getSize().y > 0) {
                        wall.setTexture(&wallTexture);
                        wall.setTextureRect(IntRect({0, 0}, {static_cast<int>(thickness), cellSize}));
                        wall.setFillColor(Color(brightness, brightness, brightness, 255));
                    } else {
                        wall.setFillColor(wallColor);
                    }
                    window.draw(wall);
                }
            }
        }
    }

    //=== EXIT MARKER RENDERING ===
    // Draw green exit marker in bottom-right corner (always visible)
    RectangleShape exit(Vector2f(static_cast<float>(cellSize - 4), static_cast<float>(cellSize - 4)));  // Slightly smaller than cell
    exit.setPosition(Vector2f(static_cast<float>((width - 1) * cellSize + 2), static_cast<float>((height - 1) * cellSize + 2))); // Centered in exit cell
    exit.setFillColor(Color::Green);  // Green color for easy identification
    window.draw(exit);
}

//=== PLAYER RENDERING SYSTEM ===
// Draws the player at their current smooth pixel position
// Updates position and renders the player circle
void Maze::drawPlayer(RenderWindow& window) {
    player.setPosition(playerPixelPos);  // Update player position to current pixel coordinates
    window.draw(player);                 // Render player circle to screen
}