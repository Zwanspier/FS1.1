#include "PlayingState3.h"
#include <random>
#include <vector>
#include <algorithm>

// Racing game components
struct Car {
    Vector2f position;
    Vector2f velocity;
    float speed = 300.0f; // pixels per second
    float maxSpeed = 500.0f;
    RectangleShape shape;
    
    Car() {
        shape.setSize(Vector2f(30, 50));
        shape.setFillColor(Color::Red);
        shape.setOrigin(Vector2f(15, 25)); // Center the car
    }
};

struct Obstacle {
    Vector2f position;
    Vector2f velocity;
    RectangleShape shape;
    bool active = true;
    
    Obstacle(float x, float y) : position(x, y) {
        shape.setSize(Vector2f(40, 40));
        shape.setFillColor(Color::Yellow);
        shape.setOrigin(Vector2f(20, 20));
        velocity.y = 200.0f; // Move downward
    }
};

struct TrackSegment {
    Vector2f position;
    float width;
    Color color;
    RectangleShape leftWall, rightWall, road;
    
    TrackSegment(float y, float trackWidth, float screenWidth) : width(trackWidth) {
        position.x = screenWidth / 2.0f;
        position.y = y;
        color = Color(102, 102, 102, 255);
        
        // Create road
        road.setSize(Vector2f(trackWidth, 20));
        road.setFillColor(Color(102, 102, 102, 255));
        road.setPosition(Vector2f(position.x - trackWidth/2, y));
        
        // Create walls
        leftWall.setSize(Vector2f(10, 20));
        leftWall.setFillColor(Color::White);
        leftWall.setPosition(Vector2f(position.x - trackWidth/2 - 10, y));
        
        rightWall.setSize(Vector2f(10, 20));
        rightWall.setFillColor(Color::White);
        rightWall.setPosition(Vector2f(position.x + trackWidth/2, y));
    }
};

// Helper function for collision detection
bool checkCollision(const RectangleShape& rect1, const RectangleShape& rect2) {
    FloatRect bounds1 = rect1.getGlobalBounds();
    FloatRect bounds2 = rect2.getGlobalBounds();
    
    return (bounds1.position.x < bounds2.position.x + bounds2.size.x &&
            bounds1.position.x + bounds1.size.x > bounds2.position.x &&
            bounds1.position.y < bounds2.position.y + bounds2.size.y &&
            bounds1.position.y + bounds1.size.y > bounds2.position.y);
}

// Helper function to check if a position overlaps with existing obstacles
bool isPositionValid(float x, float y, const vector<Obstacle>& obstacles, float minDistance = 80.0f) { // Increased from 60.0f
    for (const auto& obstacle : obstacles) {
        float dx = x - obstacle.position.x;
        float dy = y - obstacle.position.y;
        float distance = sqrt(dx * dx + dy * dy);
        
        if (distance < minDistance) {
            return false; // Too close to existing obstacle
        }
    }
    return true; // Position is valid
}

void handlePlayingState3(RenderWindow& window, bool& running, GameState& state)
{
    // Access music volume from settings
    extern float musicVolume;
    
    // Static variables for persistent state (these persist across game restarts)
    static Font font;
    static bool fontLoaded = false;
    
    // Music-related timers that should persist across game restarts
    static Clock musicOffTimer; // Timer for tracking music off time
    static bool musicWasOff = false; // Track previous music state
    static Clock textDisplayTimer; // Timer for text sequence
    static bool textSequenceStarted = false;
    static int currentTextIndex = -1; // -1 means no text shown yet
    static bool textSequenceCompleted = false;
    static bool levelTimersInitialized = false; // Track if we've initialized the level-persistent timers
    
    // Help system variables
    static bool helpRequested = false;
    static bool playerOutOfCar = false;
    static bool hKeyPressed = false;
    static bool fKeyPressed = false;
    
    // Car object to show when player exits
    static RectangleShape carShape;
    static Vector2f carPosition;
    static bool carShapeInitialized = false;
    
    // Text messages to display
    static const vector<string> secretTexts = {
        "Can you hear that.",
        "That moment where everything goes quiet.",
        "Isn't it soothing.",
        "All the noise washed away.",
        "Nothing to distract you anymore.",
        "Nothing but the sound of the engine and the endless road ahead.",
        "It does get boring after a while though.",
        "Maybe you should turn the music back on.",
        "Or perhaps not...",
        "The choice is yours.",
        "You can also keep driving in silence.",
        "Or you can go the next level.",
        "If there is one...",
        "I'm sure you'll figure it out.",
        "I'll be here if you need me."
    };
    
    // Game-specific variables (these reset with game restarts)
    static Clock clock;
    static Clock gameTimer;
    static Car player;
    static vector<TrackSegment> track;
    static vector<Obstacle> obstacles;
    static bool gameInitialized = false;

    // Obstacle spawning variables (these should reset with game restarts)
    static float lastObstacleDistance = 0.0f;
    static float nextObstacleDistance = 300.0f;

    // Game state
    static float gameSpeed = 200.0f; // Base speed of the world moving toward player
    static float trackWidth = 400.0f; // Increased from 200.0f to 400.0f (doubled width)
    static int score = 0;
    static bool gameOver = false;
    static float totalDistance = 0.0f; // Track total distance traveled
    
    // Random number generation
    static random_device rd;
    static mt19937 gen(rd());
    
    // Initialize level-persistent timers only once per level entry
    if (!levelTimersInitialized) {
        musicOffTimer.restart();
        textDisplayTimer.restart();
        musicWasOff = (musicVolume <= 0.0f);
        textSequenceStarted = false;
        currentTextIndex = -1;
        textSequenceCompleted = false;
        helpRequested = false;
        playerOutOfCar = false;
        levelTimersInitialized = true;
    }
    
    // Initialize game on first run or restart
    if (!gameInitialized) {
        if (!fontLoaded) {
            font.openFromFile("arial.ttf");
            fontLoaded = true;
        }
        
        // Initialize player car
        player.position = Vector2f(window.getSize().x / 2.0f, window.getSize().y * 0.8f);
        
        // Initialize separate car shape for when player exits
        if (!carShapeInitialized) {
            carShape.setSize(Vector2f(30, 50));
            carShape.setFillColor(Color::Red);
            carShape.setOrigin(Vector2f(15, 25));
            carShapeInitialized = true;
        }
        
        // Generate initial track segments
        for (int i = 0; i < 50; ++i) {
            track.emplace_back(-i * 20.0f, trackWidth, window.getSize().x);
        }
        
        // Reset obstacle spawning variables
        lastObstacleDistance = 0.0f;
        nextObstacleDistance = 300.0f;
        
        clock.restart();
        gameTimer.restart();
        totalDistance = 0.0f; // Reset distance
        gameInitialized = true;
    }

    float deltaTime = clock.restart().asSeconds();
    
    // Check music state and update timer
    bool musicCurrentlyOff = (musicVolume <= 0.0f);
    if (musicCurrentlyOff) {
        if (!musicWasOff) {
            // Music just turned off, restart timer
            musicOffTimer.restart();
        }
        // Music has been off - check if it's been long enough to start text sequence
        if (!textSequenceStarted && musicOffTimer.getElapsedTime().asSeconds() >= 60.0f) {
            textSequenceStarted = true;
            textDisplayTimer.restart();
            currentTextIndex = 0; // Start with first text
        }
    } else {
        // Music is on, reset everything if it was previously off
        if (musicWasOff) {
            textSequenceStarted = false;
            currentTextIndex = -1;
            textSequenceCompleted = false;
        }
    }
    musicWasOff = musicCurrentlyOff;
    
    // Update text sequence timing with 1-second gaps
    if (textSequenceStarted && !textSequenceCompleted) {
        float textElapsedTime = textDisplayTimer.getElapsedTime().asSeconds();
        
        // Each text cycle: 9 seconds visible + 1 second gap = 10 seconds total
        int cycleIndex = static_cast<int>(textElapsedTime / 10.0f);
        float cycleTime = textElapsedTime - (cycleIndex * 10.0f);
        
        if (cycleIndex < static_cast<int>(secretTexts.size())) {
            if (cycleTime < 9.0f) {
                // Text is visible for first 9 seconds of each 10-second cycle
                currentTextIndex = cycleIndex;
            } else {
                // 1-second gap (no text shown)
                currentTextIndex = -1;
            }
        } else {
            textSequenceCompleted = true;
            currentTextIndex = static_cast<int>(secretTexts.size()) - 1; // Keep showing last text
        }
    }
    
    // Handle help system input
    if (textSequenceCompleted) {
        // Handle H key for help
        if (Keyboard::isKeyPressed(Keyboard::Key::H)) {
            if (!hKeyPressed) {
                helpRequested = !helpRequested; // Toggle help display
                hKeyPressed = true;
            }
        } else {
            hKeyPressed = false;
        }
    }
    
    // Handle F key to get out of car (works regardless of help menu state)
    if (textSequenceCompleted && gameSpeed <= 55.0f && !playerOutOfCar) { // Car must be nearly stopped
        if (Keyboard::isKeyPressed(Keyboard::Key::F)) {
            if (!fKeyPressed) {
                playerOutOfCar = true;
                carPosition = player.position; // Save car position
                // Change player shape to represent a person
                player.shape.setSize(Vector2f(20, 30)); // Smaller size for person
                player.shape.setOrigin(Vector2f(10, 15)); // Adjust origin
                fKeyPressed = true;
            }
        } else {
            fKeyPressed = false;
        }
    }
    
    // Check for end condition - player walks off screen
    if (playerOutOfCar) {
        // Allow player to move as a person (slower movement with full directional control)
        if (Keyboard::isKeyPressed(Keyboard::Key::A) || Keyboard::isKeyPressed(Keyboard::Key::Left)) {
            player.position.x -= 150.0f * deltaTime; // Slower walking speed
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::D) || Keyboard::isKeyPressed(Keyboard::Key::Right)) {
            player.position.x += 150.0f * deltaTime; // Slower walking speed
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::W) || Keyboard::isKeyPressed(Keyboard::Key::Up)) {
            player.position.y -= 150.0f * deltaTime; // Move up
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::S) || Keyboard::isKeyPressed(Keyboard::Key::Down)) {
            player.position.y += 150.0f * deltaTime; // Move down
        }
        
        // Keep player within screen boundaries
        player.position.x = max(0.0f - 20.0f, min((float)window.getSize().x + 20.0f, player.position.x)); // Allow slight off-screen for exit
        player.position.y = max(0.0f, min((float)window.getSize().y - 30.0f, player.position.y)); // Keep within vertical bounds
        
        // Check if player walked off screen (only horizontally for level exit)
        if (player.position.x < -15 || player.position.x > window.getSize().x + 15) {
            // End the level - go to end screen or next level
            state = MENU; // You can change this to a specific end state if you create one
            gameInitialized = false;
            levelTimersInitialized = false;
            return;
        }
    }
    
    if (!gameOver && !playerOutOfCar) {
        // Handle player input (only when in car)
        if (Keyboard::isKeyPressed(Keyboard::Key::A) || Keyboard::isKeyPressed(Keyboard::Key::Left)) {
            player.velocity.x = -player.speed;
        }
        else if (Keyboard::isKeyPressed(Keyboard::Key::D) || Keyboard::isKeyPressed(Keyboard::Key::Right)) {
            player.velocity.x = player.speed;
        }
        else {
            player.velocity.x = 0;
        }
        
        // Accelerate/Decelerate
        if (Keyboard::isKeyPressed(Keyboard::Key::W) || Keyboard::isKeyPressed(Keyboard::Key::Up)) {
            gameSpeed = min(gameSpeed + 100.0f * deltaTime, 1000.0f);
        }
        else if (Keyboard::isKeyPressed(Keyboard::Key::S) || Keyboard::isKeyPressed(Keyboard::Key::Down)) {
            gameSpeed = max(gameSpeed - 100.0f * deltaTime, 50.0f);
        }

        // Update distance traveled based on current speed
        totalDistance += gameSpeed * deltaTime;
        
        // Update score based on distance (convert pixels to meters and round)
        score = static_cast<int>(totalDistance / 10.0f); // 10 pixels = 1 meter

        // Update player position
        player.position.x += player.velocity.x * deltaTime;
        
        // Keep player within track boundaries
        float trackLeft = window.getSize().x / 2.0f - trackWidth / 2.0f;
        float trackRight = window.getSize().x / 2.0f + trackWidth / 2.0f;
        player.position.x = max(trackLeft + 15, min(trackRight - 15, player.position.x));
        
        // Update track segments (move them down)
        for (auto& segment : track) {
            segment.position.y += gameSpeed * deltaTime;
            segment.road.setPosition(Vector2f(segment.position.x - segment.width/2, segment.position.y));
            segment.leftWall.setPosition(Vector2f(segment.position.x - segment.width/2 - 10, segment.position.y));
            segment.rightWall.setPosition(Vector2f(segment.position.x + segment.width/2, segment.position.y));
        }
        
        // Remove track segments that have gone off screen and add new ones
        track.erase(remove_if(track.begin(), track.end(), 
            [&](const TrackSegment& seg) { return seg.position.y > window.getSize().y + 50; }), track.end());
        
        // Add new track segments at the top
        while (track.size() < 50) {
            float newY = track.empty() ? -20 : track.front().position.y - 20;
            track.insert(track.begin(), TrackSegment(newY, trackWidth, window.getSize().x));
        }
        
        // Generate obstacles based on distance traveled with irregular spacing
        // Check if we've traveled far enough for the next obstacle spawn
        float distanceSinceLastObstacle = totalDistance - lastObstacleDistance;

        if (distanceSinceLastObstacle >= nextObstacleDistance) {
            // Generate irregular distance interval for next spawn (200-500 pixels of travel)
            uniform_real_distribution<float> distanceDist(200.0f, 500.0f);
            nextObstacleDistance = distanceDist(gen);
            lastObstacleDistance = totalDistance; // Update last spawn distance
            
            // Reduce number of obstacles per spawn (1-2 instead of 1-3)
            uniform_int_distribution<int> countDist(1, 2);
            int numObstacles = countDist(gen);
            
            // Try to spawn obstacles without overlapping
            uniform_real_distribution<float> xDist(trackLeft + 30, trackRight - 30);
            
            for (int i = 0; i < numObstacles; ++i) {
                int attempts = 0;
                const int maxAttempts = 10; // Prevent infinite loops
                
                while (attempts < maxAttempts) {
                    float newX = xDist(gen);
                    float newY = -50.0f; // Spawn above screen
                    
                    // Check if this position is valid (not overlapping with existing obstacles)
                    if (isPositionValid(newX, newY, obstacles, 80.0f)) {
                        obstacles.emplace_back(newX, newY);
                        break; // Successfully placed obstacle
                    }
                    attempts++;
                }
            }
        }
        
        // Update obstacles
        for (auto& obstacle : obstacles) {
            obstacle.position.y += (gameSpeed + obstacle.velocity.y) * deltaTime;
            obstacle.shape.setPosition(obstacle.position);
        }
        
        // Remove obstacles that have gone off screen
        obstacles.erase(remove_if(obstacles.begin(), obstacles.end(),
            [&](const Obstacle& obs) { return obs.position.y > window.getSize().y + 50; }), obstacles.end());
        
        // Collision detection using custom function
        player.shape.setPosition(player.position);
        for (const auto& obstacle : obstacles) {
            if (checkCollision(player.shape, obstacle.shape)) {
                gameOver = true;
                break;
            }
        }
    }
    
    // Rendering
    window.clear(Color::Black); // Background
    
    // Draw a single full-screen road
    static RectangleShape fullRoad;
    static RectangleShape leftWall, rightWall;
    static bool roadInitialized = false;
    
    if (!roadInitialized) {
        // Create full-screen road
        fullRoad.setSize(Vector2f(trackWidth, window.getSize().y + 100)); // Extra height for scrolling
        fullRoad.setFillColor(Color(102, 102, 102, 255));
        
        // Create walls
        leftWall.setSize(Vector2f(10, window.getSize().y + 100));
        leftWall.setFillColor(Color::White);
        
        rightWall.setSize(Vector2f(10, window.getSize().y + 100));
        rightWall.setFillColor(Color::White);
        
        roadInitialized = true;
    }
    
    // Update road position for scrolling effect (only if player is in car and not stopped)
    static float roadOffset = 0;
    if (!gameOver && !playerOutOfCar && gameSpeed > 55.0f) {
        roadOffset += gameSpeed * deltaTime;
        if (roadOffset >= 50) roadOffset -= 50; // Reset every 50 pixels for seamless loop
    }
    
    // Position road and walls
    float centerX = window.getSize().x / 2.0f;
    fullRoad.setPosition(Vector2f(centerX - trackWidth/2, -50 + roadOffset));
    leftWall.setPosition(Vector2f(centerX - trackWidth/2 - 10, -50 + roadOffset));
    rightWall.setPosition(Vector2f(centerX + trackWidth/2, -50 + roadOffset));
    
    // Draw road and walls
    window.draw(fullRoad);
    window.draw(leftWall);
    window.draw(rightWall);
    
    // Draw obstacles
    for (const auto& obstacle : obstacles) {
        window.draw(obstacle.shape);
    }
    
    // Draw car (always visible, either as player or as stationary car)
    if (playerOutOfCar) {
        // Draw the stationary car at saved position
        carShape.setPosition(carPosition);
        window.draw(carShape);
        
        // Draw player as person (blue color, smaller size)
        player.shape.setFillColor(Color::Blue);
        player.shape.setPosition(player.position);
        window.draw(player.shape);
    } else {
        // Draw player as car (red color)
        player.shape.setFillColor(Color::Red);
        player.shape.setPosition(player.position);
        window.draw(player.shape);
    }
    
    // Draw secret text sequence if active and not in gap period
    if (textSequenceStarted && currentTextIndex >= 0 && currentTextIndex < static_cast<int>(secretTexts.size())) {
        Text secretText(font, secretTexts[currentTextIndex], 32);
        secretText.setFillColor(Color::Cyan);
        secretText.setOutlineColor(Color::Black);
        secretText.setOutlineThickness(2.f);
        
        // Position text on the left side of the screen
        float textX = 50.0f; // Left margin
        float textY = 200.0f; // Fixed position
        
        secretText.setPosition(Vector2f(textX, textY));
        window.draw(secretText);
    }
    
    // Draw "Press H for help" when text sequence is completed
    if (textSequenceCompleted && !helpRequested) {
        Text helpHint(font, "H - Help", 20);
        helpHint.setFillColor(Color::White);
        helpHint.setOutlineColor(Color::Black);
        helpHint.setOutlineThickness(2.f);
        auto hintBounds = helpHint.getLocalBounds();
        helpHint.setOrigin(Vector2f(hintBounds.size.x, 0)); // Right-aligned
        helpHint.setPosition(Vector2f(window.getSize().x - 20.f, 50.f)); // Below F1 settings
        window.draw(helpHint);
    }
    
    // Draw help instructions when requested
    if (helpRequested) {
        Text helpText1(font, "To end this level:", 28);
        helpText1.setFillColor(Color::Yellow);
        helpText1.setOutlineColor(Color::Black);
        helpText1.setOutlineThickness(2.f);
        auto bounds1 = helpText1.getLocalBounds();
        helpText1.setOrigin(Vector2f(bounds1.size.x / 2.f, bounds1.size.y / 2.f));
        helpText1.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f - 80));
        window.draw(helpText1);
        
        Text helpText2(font, "Stop the car", 24);
        helpText2.setFillColor(Color::White);
        helpText2.setOutlineColor(Color::Black);
        helpText2.setOutlineThickness(2.f);
        auto bounds2 = helpText2.getLocalBounds();
        helpText2.setOrigin(Vector2f(bounds2.size.x / 2.f, bounds2.size.y / 2.f));
        helpText2.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f - 40));
        window.draw(helpText2);
        
        Text helpText3(font, "Get some fresh air", 24);
        helpText3.setFillColor(Color::White);
        helpText3.setOutlineColor(Color::Black);
        helpText3.setOutlineThickness(2.f);
        auto bounds3 = helpText3.getLocalBounds();
        helpText3.setOrigin(Vector2f(bounds3.size.x / 2.f, bounds3.size.y / 2.f));
        helpText3.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f));
        window.draw(helpText3);
        
        Text helpText4(font, "Leave this place", 24);
        helpText4.setFillColor(Color::White);
        helpText4.setOutlineColor(Color::Black);
        helpText4.setOutlineThickness(2.f);
        auto bounds4 = helpText4.getLocalBounds();
        helpText4.setOrigin(Vector2f(bounds4.size.x / 2.f, bounds4.size.y / 2.f));
        helpText4.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f + 40));
        window.draw(helpText4);
        
        // Show status
        if (gameSpeed <= 55.0f && !playerOutOfCar) {
            Text statusText(font, "Car stopped! Press F to get out", 20);
            statusText.setFillColor(Color::Green);
            statusText.setOutlineColor(Color::Black);
            statusText.setOutlineThickness(2.f);
            auto statusBounds = statusText.getLocalBounds();
            statusText.setOrigin(Vector2f(statusBounds.size.x / 2.f, statusBounds.size.y / 2.f));
            statusText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f + 80));
            window.draw(statusText);
        } else if (playerOutOfCar) {
            Text statusText(font, "Be free from this nightmare", 20);
            statusText.setFillColor(Color::Cyan);
            statusText.setOutlineColor(Color::Black);
            statusText.setOutlineThickness(2.f);
            auto statusBounds = statusText.getLocalBounds();
            statusText.setOrigin(Vector2f(statusBounds.size.x / 2.f, statusBounds.size.y / 2.f));
            statusText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f + 80));
            window.draw(statusText);
        }
        
        Text closeHelp(font, "Press H again to close help", 16);
        closeHelp.setFillColor(Color::White);
        closeHelp.setPosition(Vector2f(20, window.getSize().y - 40));
        window.draw(closeHelp);
    }
    
    // Show "Press F to exit car" hint when car is stopped (regardless of help menu)
    if (textSequenceCompleted && !helpRequested && gameSpeed <= 55.0f && !playerOutOfCar) {
        Text exitHint(font, "Press F to exit car", 20);
        exitHint.setFillColor(Color::Green);
        exitHint.setOutlineColor(Color::Black);
        exitHint.setOutlineThickness(2.f);
        auto exitBounds = exitHint.getLocalBounds();
        exitHint.setOrigin(Vector2f(exitBounds.size.x / 2.f, exitBounds.size.y / 2.f));
        exitHint.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y - 100));
        window.draw(exitHint);
    }
    
    // Draw UI
    Text scoreText(font, "Distance: " + to_string(score) + "m", 36);
    scoreText.setFillColor(Color::White);
    scoreText.setOutlineColor(Color::Black);
    scoreText.setOutlineThickness(2.f);
    scoreText.setPosition(Vector2f(20, 20));
    window.draw(scoreText);
    
    Text speedText(font, "Speed: " + to_string(static_cast<int>(gameSpeed)) + " px/s", 24);
    speedText.setFillColor(Color::White);
    speedText.setOutlineColor(Color::Black);
    speedText.setOutlineThickness(2.f);
    speedText.setPosition(Vector2f(20, 70));
    window.draw(speedText);
    
    // Debug info (optional - remove in final version)
    if (musicCurrentlyOff && !textSequenceStarted) {
        float timeRemaining = 60.0f - musicOffTimer.getElapsedTime().asSeconds();
        if (timeRemaining > 0) {
            Text debugText(font, "Music off time: " + to_string(static_cast<int>(timeRemaining)) + "s remaining", 20);
            debugText.setFillColor(Color::Yellow);
            debugText.setPosition(Vector2f(20, 110));
            window.draw(debugText);
        }
    }
    
    if (gameOver) {
        Text gameOverText(font, "GAME OVER! Distance: " + to_string(score) + "m", 36);
        gameOverText.setFillColor(Color::Red);
        gameOverText.setOutlineColor(Color::Black);
        gameOverText.setOutlineThickness(3.f);
        auto bounds = gameOverText.getLocalBounds();
        gameOverText.setOrigin(Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        gameOverText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f - 40));
        window.draw(gameOverText);
        
        Text restartText(font, "Press R to restart", 32);
        restartText.setFillColor(Color::White);
        restartText.setOutlineColor(Color::Black);
        restartText.setOutlineThickness(2.f);
        auto restartBounds = restartText.getLocalBounds();
        restartText.setOrigin(Vector2f(restartBounds.size.x / 2.f, restartBounds.size.y / 2.f));
        restartText.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f + 20));
        window.draw(restartText);
        
        // Restart game (but keep level-persistent timers running)
        if (Keyboard::isKeyPressed(Keyboard::Key::R)) {
            gameOver = false;
            obstacles.clear();
            track.clear();
            score = 0;
            totalDistance = 0.0f;
            gameSpeed = 200.0f;
            player.position = Vector2f(window.getSize().x / 2.0f, window.getSize().y * 0.8f);
            // Reset player shape back to car size
            player.shape.setSize(Vector2f(30, 50));
            player.shape.setOrigin(Vector2f(15, 25));
            gameTimer.restart();
            roadInitialized = false;
            gameInitialized = false;
            helpRequested = false;
            playerOutOfCar = false;
        }
    }

    // Handle menu and settings navigation
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        state = MENU;
        gameInitialized = false;
        levelTimersInitialized = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        extern GameState previousState;
        previousState = PLAYING3;
        state = SETTINGS;
    }
}