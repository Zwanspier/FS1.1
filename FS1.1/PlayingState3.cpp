#include "PlayingState3.h"

//=== DATA STRUCTURES ===
// These structs define the blueprint for game objects

// Represents a player's car with position, movement, and visual components
struct Car {
    Vector2f position;        // Current X,Y coordinates on screen
    Vector2f velocity;        // Movement speed and direction per frame
    float speed = 400.0f;     // Standard movement speed in pixels/second
    float maxSpeed = 500.0f;  // Maximum possible speed
    optional<Sprite> sprite;  // Optional sprite for car image (may be null)
    RectangleShape shape;     // Fallback rectangle if no sprite loaded
    int spriteIndex = 0;      // Index into sprite sheet for car appearance
    
    // Hitbox configuration
    float hitboxSizeMultiplier = 1.0f;

    // Constructor initializes default values
    Car() : shape({30, 50}) {
        velocity = Vector2f(0, 0);           // Start stationary
        shape.setSize(Vector2f(30, 50));     // Set rectangle dimensions
        shape.setFillColor(Color::Red);      // Default color
        shape.setOrigin(Vector2f(15, 25));   // Center origin for rotation
    }
};

// Represents AI-controlled obstacle cars with sound effects
struct Obstacle {
    Vector2f position;        // Screen coordinates
    Vector2f velocity;        // Movement vector
    optional<Sprite> sprite;  // Car image (optional)
    RectangleShape shape;     // Fallback rectangle shape
    bool active = true;       // Whether obstacle is still in play
    int spriteIndex = 0;      // Which car sprite to use from sheet
                                
    float hitboxSizeMultiplier = 0.7f;

    // Audio system - each obstacle has independent sound
    Sound* engineSound = nullptr;              // SFML Sound object pointer
    SoundBuffer* personalSoundBuffer = nullptr; // Each obstacle owns sound data
    bool soundInitialized = false;             // Has audio been set up?
    float soundVolume = 0.0f;                 // Current volume level
    
    // Constructor: creates obstacle at specified coordinates
    Obstacle(float x, float y) : position(x, y), shape({40, 40}) {
        velocity.y = 200.0f;                    // Always moves downward
        spriteIndex = rand() % 5;               // Random car type (0-4)
        shape.setSize(Vector2f(40, 40));        // Square shape
        shape.setFillColor(Color::Yellow);      // Yellow color
        shape.setOrigin(Vector2f(20, 20));      // Center origin
    }
    
    // Destructor: cleanup audio resources to prevent memory leaks
    ~Obstacle() {
        if (engineSound) {
            if (engineSound->getStatus() == Sound::Status::Playing) {
                engineSound->stop();  // Stop audio before deletion
            }
            delete engineSound;       // Free memory
        }
        if (personalSoundBuffer) {
            delete personalSoundBuffer;  // Free sound data memory
        }
    }
    
    // Copy constructor: creates new obstacle from existing 
    Obstacle(const Obstacle& other) : position(other.position), velocity(other.velocity), 
                                     shape(other.shape), active(other.active),
                                     sprite(other.sprite),
                                     spriteIndex(other.spriteIndex),
                                     hitboxSizeMultiplier(other.hitboxSizeMultiplier),
                                     soundInitialized(false), soundVolume(0.0f), 
                                     engineSound(nullptr), personalSoundBuffer(nullptr) {
        // Note: Each obstacle needs its own audio objects for proper mixing
    }
    
    // Assignment operator: copies data from another obstacle
    Obstacle& operator=(const Obstacle& other) {
        if (this != &other) {  // Prevent self-assignment
            position = other.position;
            velocity = other.velocity;
            shape = other.shape;
            active = other.active;
            spriteIndex = other.spriteIndex;
            sprite = other.sprite;
            hitboxSizeMultiplier = other.hitboxSizeMultiplier;
            // Audio objects are not copied - each obstacle maintains its own
        }
        return *this;
    }
};

// Represents a segment of the racing track
struct TrackSegment {
    Vector2f position;    // Center position of this track piece
    float width;         // Track width in pixels
    Color color;         // Track color
    RectangleShape leftWall, rightWall, road;  // Visual components
    
    // Constructor: creates track segment at specified Y position
    TrackSegment(float y, float trackWidth, float screenWidth) : width(trackWidth), 
        leftWall({10, 20}), rightWall({10, 20}), road({trackWidth, 20}) {
        position.x = screenWidth / 2.0f;  // Center horizontally
        position.y = y;                   // Set vertical position
        color = Color(102, 102, 102, 255); // Gray color
        
        // Configure road surface
        road.setSize(Vector2f(trackWidth, 20));
        road.setFillColor(Color(102, 102, 102, 255));
        road.setPosition(Vector2f(position.x - trackWidth/2, y));
        
        // Configure left barrier
        leftWall.setSize(Vector2f(10, 20));
        leftWall.setFillColor(Color::White);
        leftWall.setPosition(Vector2f(position.x - trackWidth/2 - 10, y));
        
        // Configure right barrier
        rightWall.setSize(Vector2f(10, 20));
        rightWall.setFillColor(Color::White);
        rightWall.setPosition(Vector2f(position.x + trackWidth/2, y));
    }
};

//=== UTILITY FUNCTIONS ===

// Get smaller hitbox bounds for improved collision detection
// This creates a hitbox that is smaller than the visual sprite for better gameplay feel
FloatRect getHitboxBounds(const Sprite& sprite, float sizeMultiplier) {
    FloatRect spriteBounds = sprite.getGlobalBounds();
    
    // Calculate reduced dimensions
    float reducedWidth = spriteBounds.size.x * sizeMultiplier;
    float reducedHeight = spriteBounds.size.y * sizeMultiplier;
    
    // Center the smaller hitbox within the sprite bounds
    float offsetX = (spriteBounds.size.x - reducedWidth) / 2.0f;
    float offsetY = (spriteBounds.size.y - reducedHeight) / 2.0f;
    
    return FloatRect(
        Vector2f(spriteBounds.position.x + offsetX, spriteBounds.position.y + offsetY),
        Vector2f(reducedWidth, reducedHeight)
    );
}

// Overload for RectangleShape (fallback when sprites aren't available)
FloatRect getHitboxBounds(const RectangleShape& shape, float sizeMultiplier) {
    FloatRect shapeBounds = shape.getGlobalBounds();
    
    // Calculate reduced dimensions
    float reducedWidth = shapeBounds.size.x * sizeMultiplier;
    float reducedHeight = shapeBounds.size.y * sizeMultiplier;
    
    // Center the smaller hitbox within the shape bounds
    float offsetX = (shapeBounds.size.x - reducedWidth) / 2.0f;
    float offsetY = (shapeBounds.size.y - reducedHeight) / 2.0f;
    
    return FloatRect(
        Vector2f(shapeBounds.position.x + offsetX, shapeBounds.position.y + offsetY),
        Vector2f(reducedWidth, reducedHeight)
    );
}

// AABB (Axis-Aligned Bounding Box) collision detection for rectangles
bool checkCollision(const RectangleShape& rect1, const RectangleShape& rect2) {
    FloatRect bounds1 = rect1.getGlobalBounds();  // Get rectangle 1's screen bounds
    FloatRect bounds2 = rect2.getGlobalBounds();  // Get rectangle 2's screen bounds
    
    // Check for overlap in both X and Y axes
    return (bounds1.position.x < bounds2.position.x + bounds2.size.x &&
            bounds1.position.x + bounds1.size.x > bounds2.position.x &&
            bounds1.position.y < bounds2.position.y + bounds2.size.y &&
            bounds1.position.y + bounds1.size.y > bounds2.position.y);
}

// Enhanced collision detection for sprite objects with customizable hitbox size
bool checkSpriteCollision(const Sprite& sprite1, const Sprite& sprite2, float sizeMultiplier1 = 0.7f, float sizeMultiplier2 = 0.7f) {
    FloatRect bounds1 = getHitboxBounds(sprite1, sizeMultiplier1);
    FloatRect bounds2 = getHitboxBounds(sprite2, sizeMultiplier2);
    
    auto intersection = bounds1.findIntersection(bounds2);
    return intersection.has_value();  // Returns true if intersection exists
}

// Mixed collision detection (sprite vs rectangle shape)
bool checkMixedCollision(const Sprite& sprite, const RectangleShape& shape, float spriteMultiplier = 0.7f, float shapeMultiplier = 0.7f) {
    FloatRect spriteBounds = getHitboxBounds(sprite, spriteMultiplier);
    FloatRect shapeBounds = getHitboxBounds(shape, shapeMultiplier);
    
    auto intersection = spriteBounds.findIntersection(shapeBounds);
    return intersection.has_value();
}

// Enhanced collision detection for rectangles with customizable hitbox size
bool checkCollision(const RectangleShape& rect1, const RectangleShape& rect2, float sizeMultiplier1 = 0.7f, float sizeMultiplier2 = 0.7f) {
    FloatRect bounds1 = getHitboxBounds(rect1, sizeMultiplier1);
    FloatRect bounds2 = getHitboxBounds(rect2, sizeMultiplier2);
    
    // Check for overlap in both X and Y axes
    return (bounds1.position.x < bounds2.position.x + bounds2.size.x &&
            bounds1.position.x + bounds1.size.x > bounds2.position.x &&
            bounds1.position.y < bounds2.position.y + bounds2.size.y &&
            bounds1.position.y + bounds1.size.y > bounds2.position.y);
}

// Validates obstacle placement to prevent clustering
bool isPositionValid(float x, float y, const vector<Obstacle>& obstacles, float minDistance = 80.0f) {
    // Check distance to all existing obstacles
    for (const auto& obstacle : obstacles) {
        float dx = x - obstacle.position.x;  // X-axis distance
        float dy = y - obstacle.position.y;  // Y-axis distance
        float distance = sqrt(dx * dx + dy * dy);  // Euclidean distance
        
        if (distance < minDistance) {
            return false; // Too close to existing obstacle
        }
    }
    return true; // Position is acceptable
}

// Calculates Euclidean distance between two 2D points
float calculateDistance(const Vector2f& pos1, const Vector2f& pos2) {
    float dx = pos1.x - pos2.x;  // Delta X
    float dy = pos1.y - pos2.y;  // Delta Y
    return sqrt(dx * dx + dy * dy);  // Pythagorean theorem
}

//=== MAIN GAME LOOP FUNCTION ===
// Handles all logic and rendering for PlayingState3
void handlePlayingState3(RenderWindow& window, bool& running, GameState& state)
{
    // External reference to global music volume setting
    extern float musicVolume;
    
    //=== PERSISTENT STATE VARIABLES ===
    // Static variables maintain state between function calls
    
    // Font loading
    static Font font;
    static bool fontLoaded = false;
    
    // Background system
    static Texture backgroundTexture;      // Image data
    static Sprite* backgroundSprite = nullptr;  // Display object (pointer for lazy initialization)
    static bool backgroundLoaded = false;       // Loading status flag
    static float backgroundOffset1 = 0.0f;      // Primary scrolling offset
    static float backgroundOffset2 = 0.0f;      // Secondary offset for seamless loop
    
    // Sprite sheet system for car graphics
    static Texture carSpriteSheet;           // Contains all car images
    static bool carSpriteSheetLoaded = false; // Loading status
    static vector<IntRect> carSpriteRects;   // Defines sub-rectangles for each car
    static const int SPRITE_WIDTH = 32;      // Individual sprite dimensions
    static const int SPRITE_HEIGHT = 64;
    static const int SPRITES_PER_ROW = 5;    // Layout of sprite sheet
    static const int TOTAL_CAR_SPRITES = 5;
    
    // Audio system
    static Music engineMusic;                    // Background engine sound
    static bool engineMusicLoaded = false;
    static float lastGameSpeed = 200.0f;         // Previous frame's speed for audio adjustments
    
    // Obstacle audio template
    static SoundBuffer masterObstacleEngineBuffer;     // Template sound data
    static bool masterObstacleEngineBufferLoaded = false;
    static const float MAX_OBSTACLE_SOUND_DISTANCE = 800.0f; // Maximum audible range
    static const float MIN_OBSTACLE_SOUND_DISTANCE = 100.0f; // Distance for full volume
    
    // Special narrative system (triggers after 10 seconds of silence)
    static Clock musicOffTimer;            // Tracks duration of music being off
    static bool musicWasOff = false;       // Previous frame's music state
    static Clock textDisplayTimer;         // Controls text sequence timing
    static bool textSequenceStarted = false;
    static int currentTextIndex = -1;      // Index of currently displayed message
    static bool textSequenceCompleted = false;
    static bool levelTimersInitialized = false; // Prevents re-initialization
    
    // User interaction system
    static bool helpRequested = false;     // Player requested help display
    static bool playerOutOfCar = false;    // Player exited vehicle
    static bool hKeyPressed = false;       // Input state tracking (prevents key repeat)
    static bool fKeyPressed = false;
    
    // Abandoned car visualization (when player exits)
    static RectangleShape carShape({30, 50});        // Simple rectangle fallback
    static std::optional<Sprite> abandonedCarSprite; // Sprite copy for abandoned car
    static Vector2f carPosition;                     // Where car was left
    static bool carShapeInitialized = false;
    
    // Narrative text content
    static const vector<string> secretTexts = {
		"Blah blah blah...",

        /*"That's better.",
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
        "I'll be here if you need me."*/
    };
    
    //=== GAME STATE VARIABLES ===
    // These reset when game restarts
    static Clock clock;                    // Frame timing
    static Clock gameTimer;               // Total session time
    static Car player;                    // Player's car object
    static vector<TrackSegment> track;    // Collection of track pieces
    static vector<Obstacle> obstacles;    // Active obstacle cars
    static bool gameInitialized = false;  // Initialization flag

    // Obstacle generation system
    static float lastObstacleDistance = 0.0f;  // Distance when last obstacle was created
    static float nextObstacleDistance = 300.0f; // Distance threshold for next obstacle

    // Core game metrics
    static float gameSpeed = 200.0f;       // Current scrolling speed
    static float trackWidth = 400.0f;      // Race track width
    static int score = 0;                  // Player score (based on distance)
    static bool gameOver = false;          // Game state flag
    static float totalDistance = 0.0f;     // Cumulative distance traveled
    
    // Random number generation system
    static random_device rd;
    static mt19937 gen(rd());
    
    //=== ONE-TIME LEVEL INITIALIZATION ===
    // Execute only once when entering this game state
    if (!levelTimersInitialized) {
        musicOffTimer.restart();                    // Begin tracking music state
        textDisplayTimer.restart();                // Initialize text timing
        musicWasOff = (musicVolume <= 0.0f);       // Record initial music state
        textSequenceStarted = false;
        currentTextIndex = -1;
        textSequenceCompleted = false;
        helpRequested = false;
        playerOutOfCar = false;
        levelTimersInitialized = true;             // Prevent re-execution
    }
    
    //=== ASSET LOADING ===
    // Car sprite sheet loading and processing
    if (!carSpriteSheetLoaded) {
        if (carSpriteSheet.loadFromFile("Images/Cars.png")) {
            carSpriteSheetLoaded = true;
            
            // Parse sprite sheet into individual car rectangles
            carSpriteRects.clear();
            
            // Calculate dimensions of each sprite
            Vector2u textureSize = carSpriteSheet.getSize();
            int actualSpriteWidth = textureSize.x / SPRITES_PER_ROW;
            int actualSpriteHeight = textureSize.y;
            
            cout << "Texture size: " << textureSize.x << "x" << textureSize.y << endl;
            cout << "Calculated sprite size: " << actualSpriteWidth << "x" << actualSpriteHeight << endl;
            
            // Create rectangle definitions for each car sprite
            for (int i = 0; i < TOTAL_CAR_SPRITES; ++i) {
                int col = i; // Column in sprite sheet (horizontal layout)
                
                // Define rectangle bounds for this sprite
                IntRect rect({col * actualSpriteWidth, 0}, {actualSpriteWidth, actualSpriteHeight});
                carSpriteRects.push_back(rect);
                
                cout << "Car " << i << " rect: (" << rect.position.x << ", " << rect.position.y 
                     << ", " << rect.size.x << ", " << rect.size.y << ")" << endl;
            }
        } else {
            cerr << "Failed to load Images/Cars.png" << endl;
            carSpriteSheetLoaded = false;
        }
    }
    
    // Background texture loading with scaling
    if (!backgroundLoaded) {
        if (backgroundTexture.loadFromFile("Images/grass.png")) {
            // Create sprite object from loaded texture
            backgroundSprite = new Sprite(backgroundTexture);
            
            // Calculate scaling to fit window
            Vector2u windowSize = window.getSize();
            Vector2u textureSize = backgroundTexture.getSize();
            
            float scaleX = static_cast<float>(windowSize.x) / textureSize.x;
            float scaleY = static_cast<float>(windowSize.y) / textureSize.y;
            
            backgroundSprite->setScale(Vector2f(scaleX, scaleY));
            backgroundLoaded = true;
        } 
        else {
            cerr << "Failed to load grass.png" << endl;
        }
    }
    
    // Engine music initialization
    if (!engineMusicLoaded) {
        if (engineMusic.openFromFile("Sounds/Engine4.ogg")) {
            engineMusic.setLooping(true);                    // Enable continuous loop
            float initialVolume = (60.0f / 100.0f) * musicVolume;  // UPDATED: Increased initial volume from 30% to 60%
            engineMusic.setVolume(initialVolume);            // Set volume based on global setting
            engineMusicLoaded = true;
        } else {
            cerr << "Failed to load Engine4.ogg" << endl;
            engineMusicLoaded = false;
        }
    }
        
    // Obstacle sound template loading
    if (!masterObstacleEngineBufferLoaded) {
        if (masterObstacleEngineBuffer.loadFromFile("Sounds/Engine1.2.ogg")) {
            masterObstacleEngineBufferLoaded = true;
        } else {
            cerr << "Failed to load Engine1.2.ogg for obstacles" << endl;
            masterObstacleEngineBufferLoaded = false;
        }
    }
    
    //=== GAME INITIALIZATION ===
    // Set up game objects and initial state
    if (!gameInitialized) {
        // Load text font
        if (!fontLoaded) {
            if (!font.openFromFile("arial.ttf")) {
                cerr << "Failed to load arial.ttf" << endl;
            }
            fontLoaded = true;
        }
        
        // Configure player car sprite
        if (carSpriteSheetLoaded && !carSpriteRects.empty()) {
            player.sprite = Sprite(carSpriteSheet);
            player.spriteIndex = 0;                          // Use first car design
            player.sprite->setTextureRect(carSpriteRects[player.spriteIndex]);
            
            // Calculate appropriate scaling
            Vector2u textureSize = carSpriteSheet.getSize();
            int actualSpriteWidth = textureSize.x / SPRITES_PER_ROW;
            
            float scale = 30.0f / actualSpriteWidth;         // Target width of 30 pixels
            player.sprite->setScale(Vector2f(scale, scale));
            player.sprite->setOrigin(Vector2f(actualSpriteWidth / 2.0f, textureSize.y / 2.0f));
            
            cout << "Player sprite initialized with scale: " << scale << endl;
        }
        
        // Position player at bottom-center of screen
        player.position = Vector2f(static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) * 0.8f);
        
        // Initialize fallback car shape
        if (!carShapeInitialized) {
            carShape.setSize(Vector2f(30, 50));
            carShape.setFillColor(Color::Red);
            carShape.setOrigin(Vector2f(15, 25));
            carShapeInitialized = true;
        }
        
        // Generate initial track segments extending upward
        for (int i = 0; i < 50; ++i) {
            track.emplace_back(-i * 20.0f, trackWidth, static_cast<float>(window.getSize().x));
        }
        
        // Reset obstacle generation parameters
        lastObstacleDistance = 0.0f;
        nextObstacleDistance = 300.0f;
        
        // Initialize timing systems
        clock.restart();
        gameTimer.restart();
        totalDistance = 0.0f;
        
        // Start background audio
        if (engineMusicLoaded && !playerOutOfCar) {
            engineMusic.play();
        }
        
        gameInitialized = true;
    }

    //=== FRAME TIMING ===
    // Calculate time elapsed since last frame for smooth animation
    float deltaTime = clock.restart().asSeconds();
    
    //=== BACKGROUND ANIMATION ===
    // Implement parallax scrolling effect
    if (backgroundLoaded && backgroundSprite && !gameOver && !playerOutOfCar) {
        float backgroundScrollSpeed = gameSpeed * 0.3f;  // Slower than foreground for depth
        backgroundOffset1 += backgroundScrollSpeed * deltaTime;
        backgroundOffset2 += backgroundScrollSpeed * deltaTime;
        
        // Handle wrap-around for infinite scrolling
        Vector2u textureSize = backgroundTexture.getSize();
        Vector2f scale = backgroundSprite->getScale();
        float scaledHeight = textureSize.y * scale.y;
        
        if (backgroundOffset1 >= scaledHeight) {
            backgroundOffset1 -= scaledHeight;  // Reset to create seamless loop
        }
        if (backgroundOffset2 >= scaledHeight) {
            backgroundOffset2 -= scaledHeight;
        }
    }
    
    //=== DYNAMIC AUDIO SYSTEM ===
    // Adjust engine sound based on vehicle speed and global volume settings
    // All engine sounds in Level 3 now respect the global musicVolume setting from settings menu
    if (engineMusicLoaded) {
        if (!playerOutOfCar && !gameOver) {
            // Normalize speed to 0-1 range for audio calculations
            float speedRatio = (gameSpeed - 50.0f) / (1000.0f - 50.0f);
            speedRatio = max(0.0f, min(1.0f, speedRatio));  // Clamp to valid range
            
            // Higher speed increases pitch (realistic engine behavior)
            float pitch = 0.8f + (speedRatio * 0.6f);       // Range: 0.8 to 1.4
            engineMusic.setPitch(pitch);
            
            // UPDATED: Increased base volume for louder engine sound - Range: 40 to 80 (was 20 to 40)
            float baseVolume = 40.0f + (speedRatio * 40.0f);    // Range: 40 to 80
            float adjustedVolume = (baseVolume / 100.0f) * musicVolume;  // Scale by global volume
            engineMusic.setVolume(adjustedVolume);
            
            // Ensure music continues playing
            if (engineMusic.getStatus() != Music::Status::Playing) {
                engineMusic.play();
            }
        } else {
            // Silence engine when not driving
            if (engineMusic.getStatus() == Music::Status::Playing) {
                engineMusic.stop();
            }
        }
    }
    
    //=== NARRATIVE TRIGGER SYSTEM ===
    // Special event when background music is disabled
    bool musicCurrentlyOff = (musicVolume <= 0.0f);
    if (musicCurrentlyOff) {
        if (!musicWasOff) {
            musicOffTimer.restart();  // Begin timing silence period
        }
        // Trigger narrative after 10 seconds of silence
        if (!textSequenceStarted && musicOffTimer.getElapsedTime().asSeconds() >= 10.0f) {
            textSequenceStarted = true;
            textDisplayTimer.restart();
            currentTextIndex = 0;
        }
    } else {
        // Reset narrative if music returns
        if (musicWasOff) {
            textSequenceStarted = false;
            currentTextIndex = -1;
            textSequenceCompleted = false;
        }
    }
    musicWasOff = musicCurrentlyOff;  // Store state for next frame
    
    //=== TEXT SEQUENCE CONTROLLER ===
    // Manages timing of narrative messages (9 seconds display, 1 second gap)
    if (textSequenceStarted && !textSequenceCompleted) {
        float textElapsedTime = textDisplayTimer.getElapsedTime().asSeconds();
        
        int cycleIndex = static_cast<int>(textElapsedTime / 10.0f);  // 10-second cycles
        float cycleTime = textElapsedTime - (cycleIndex * 10.0f);   // Position within cycle
        
        if (cycleIndex < static_cast<int>(secretTexts.size())) {
            if (cycleTime < 9.0f) {
                currentTextIndex = cycleIndex;  // Display message
            } else {
                currentTextIndex = -1;          // Gap period
            }
        } else {
            textSequenceCompleted = true;
            currentTextIndex = static_cast<int>(secretTexts.size()) - 1;  // Final message
        }
    }
    
    //=== INPUT HANDLING ===
    // Help system toggle (available after narrative completion)
    if (textSequenceCompleted) {
        if (Keyboard::isKeyPressed(Keyboard::Key::H)) {
            if (!hKeyPressed) {  // Edge detection to prevent key repeat
                helpRequested = !helpRequested;
                hKeyPressed = true;
            }
        } else {
            hKeyPressed = false;
        }
    }
    
    // Vehicle exit system
    if (textSequenceCompleted && gameSpeed <= 55.0f && !playerOutOfCar) {
        if (Keyboard::isKeyPressed(Keyboard::Key::F)) {
            if (!fKeyPressed) {
                playerOutOfCar = true;
                carPosition = player.position;  // Store car location
                
                // Preserve car sprite for abandoned vehicle rendering
                if (carSpriteSheetLoaded && player.sprite.has_value()) {
                    abandonedCarSprite = player.sprite;
                }
                
                // Reconfigure player as pedestrian
                player.shape.setSize(Vector2f(20, 30));
                player.shape.setOrigin(Vector2f(10, 15));
                fKeyPressed = true;
            }
        } else {
            fKeyPressed = false;
        }
    }
    
    //=== PEDESTRIAN MOVEMENT SYSTEM ===
    if (playerOutOfCar) {
        // Process movement input (WASD or arrow keys)
        if (Keyboard::isKeyPressed(Keyboard::Key::A) || Keyboard::isKeyPressed(Keyboard::Key::Left)) {
            player.position.x -= 150.0f * deltaTime;  // Move left
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::D) || Keyboard::isKeyPressed(Keyboard::Key::Right)) {
            player.position.x += 150.0f * deltaTime;  // Move right
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::W) || Keyboard::isKeyPressed(Keyboard::Key::Up)) {
            player.position.y -= 150.0f * deltaTime;  // Move up
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::S) || Keyboard::isKeyPressed(Keyboard::Key::Down)) {
            player.position.y += 150.0f * deltaTime;  // Move down
        }
        
        // Boundary constraints (allow slight off-screen movement)
        player.position.x = max(0.0f - 20.0f, min(static_cast<float>(window.getSize().x) + 20.0f, player.position.x));
        player.position.y = max(0.0f, min(static_cast<float>(window.getSize().y) - 30.0f, player.position.y));
        
        // Level exit condition
        if (player.position.x < -15 || player.position.x > static_cast<float>(window.getSize().x) + 15) {
            // Cleanup all audio systems
            if (engineMusicLoaded && engineMusic.getStatus() == Music::Status::Playing) {
                engineMusic.stop();
            }
            
            for (auto& obstacle : obstacles) {
                if (obstacle.soundInitialized && obstacle.engineSound && obstacle.engineSound->getStatus() == Sound::Status::Playing) {
                    obstacle.engineSound->stop();
                }
            }
            
            // Cleanup graphics resources
            if (backgroundSprite) {
                delete backgroundSprite;
                backgroundSprite = nullptr;
                backgroundLoaded = false;
            }
            
            // Transition to menu state
            state = MENU;
            gameInitialized = false;
            levelTimersInitialized = false;
            return;
        }
    }
    
    //=== DRIVING MECHANICS ===
    if (!gameOver && !playerOutOfCar) {
        //--- Steering Input ---
        if (Keyboard::isKeyPressed(Keyboard::Key::A) || Keyboard::isKeyPressed(Keyboard::Key::Left)) {
            player.velocity.x = -player.speed;  // Steer left
        }
        else if (Keyboard::isKeyPressed(Keyboard::Key::D) || Keyboard::isKeyPressed(Keyboard::Key::Right)) {
            player.velocity.x = player.speed;   // Steer right
        }
        else {
            player.velocity.x = 0;              // No steering input
        }
        
        //--- Speed Control ---
        if (Keyboard::isKeyPressed(Keyboard::Key::W) || Keyboard::isKeyPressed(Keyboard::Key::Up)) {
            gameSpeed = min(gameSpeed + 100.0f * deltaTime, 1000.0f);  // Accelerate
        }
        else if (Keyboard::isKeyPressed(Keyboard::Key::S) || Keyboard::isKeyPressed(Keyboard::Key::Down)) {
            gameSpeed = max(gameSpeed - 100.0f * deltaTime, 50.0f);    // Decelerate
        }

        //--- Score Calculation ---
        totalDistance += gameSpeed * deltaTime;           // Accumulate distance
        score = static_cast<int>(totalDistance / 10.0f);  // Convert to score units

        //--- Player Movement ---
        player.position.x += player.velocity.x * deltaTime;  // Apply horizontal movement
        
        // Track boundary enforcement
        float trackLeft = static_cast<float>(window.getSize().x) / 2.0f - trackWidth / 2.0f;
        float trackRight = static_cast<float>(window.getSize().x) / 2.0f + trackWidth / 2.0f;
        player.position.x = max(trackLeft + 15, min(trackRight - 15, player.position.x));
        
        //--- Track Animation System ---
        // Move track segments downward to simulate forward motion
        for (auto& segment : track) {
            segment.position.y += gameSpeed * deltaTime;
            // Update visual component positions
            segment.road.setPosition(Vector2f(segment.position.x - segment.width/2, segment.position.y));
            segment.leftWall.setPosition(Vector2f(segment.position.x - segment.width/2 - 10, segment.position.y));
            segment.rightWall.setPosition(Vector2f(segment.position.x + segment.width/2, segment.position.y));
        }
        
        //--- Track Segment Management ---
        // Remove segments that have scrolled off screen
        track.erase(remove_if(track.begin(), track.end(), 
            [&](const TrackSegment& seg) { return seg.position.y > static_cast<float>(window.getSize().y) + 50; }), track.end());
        
        // Add new segments at the top to maintain continuous track
        while (track.size() < 50) {
            float newY = track.empty() ? -20 : track.front().position.y - 20;
            track.insert(track.begin(), TrackSegment(newY, trackWidth, static_cast<float>(window.getSize().x)));
        }
        
        //=== OBSTACLE GENERATION SYSTEM ===
        float distanceSinceLastObstacle = totalDistance - lastObstacleDistance;

        // Spawn new obstacles based on distance traveled
        if (distanceSinceLastObstacle >= nextObstacleDistance) {
            // Randomize next spawn distance
            uniform_real_distribution<float> distanceDist(200.0f, 500.0f);
            nextObstacleDistance = distanceDist(gen);
            lastObstacleDistance = totalDistance;
            
            // Determine number of obstacles to spawn
            uniform_int_distribution<int> countDist(1, 2);
            int numObstacles = countDist(gen);
            
            // Define spawn area within track bounds
            uniform_real_distribution<float> xDist(trackLeft + 30, trackRight - 30);
            
            // Attempt to place each obstacle
            for (int i = 0; i < numObstacles; ++i) {
                int attempts = 0;
                const int maxAttempts = 10;  // Prevent infinite loops
                
                while (attempts < maxAttempts) {
                    float newX = xDist(gen);   // Random X position
                    float newY = -50.0f;       // Spawn above screen
                    
                    // Validate position doesn't conflict with existing obstacles
                    if (isPositionValid(newX, newY, obstacles, 80.0f)) {
                        obstacles.emplace_back(newX, newY);
                        
                        // Configure obstacle sprite
                        if (carSpriteSheetLoaded && !carSpriteRects.empty()) {
                            Obstacle& newObstacle = obstacles.back();
                            
                            // Validate sprite index bounds
                            if (newObstacle.spriteIndex >= 0 && newObstacle.spriteIndex < static_cast<int>(carSpriteRects.size())) {
                                newObstacle.sprite = Sprite(carSpriteSheet);
                                newObstacle.sprite->setTextureRect(carSpriteRects[newObstacle.spriteIndex]);
                                
                                // Calculate scaling for obstacle size
                                Vector2u textureSize = carSpriteSheet.getSize();
                                int actualSpriteWidth = textureSize.x / SPRITES_PER_ROW;
                                
                                float scale = 40.0f / actualSpriteWidth;  // Target 40px width
                                newObstacle.sprite->setScale(Vector2f(scale, scale));
                                newObstacle.sprite->setOrigin(Vector2f(actualSpriteWidth / 2.0f, textureSize.y / 2.0f));
                                
                                cout << "Obstacle sprite " << newObstacle.spriteIndex << " initialized with scale: " << scale << endl;
                            } else {
                                cerr << "Invalid sprite index: " << newObstacle.spriteIndex << endl;
                            }
                        }
                        break;  // Successfully placed obstacle
                    }
                    attempts++;
                }
            }
        }
        
        //=== OBSTACLE UPDATE SYSTEM ===
        for (auto& obstacle : obstacles) {
            // Move obstacle with combined speed (game speed + obstacle speed)
            obstacle.position.y += (gameSpeed + obstacle.velocity.y) * deltaTime;
            
            // Update visual representation
            if (carSpriteSheetLoaded && obstacle.sprite.has_value()) {
                obstacle.sprite->setPosition(obstacle.position);
            } else {
                obstacle.shape.setPosition(obstacle.position);
            }
            
            //--- Obstacle Audio Initialization ---
            if (masterObstacleEngineBufferLoaded && !obstacle.soundInitialized) {
                // Create personal copy of sound data (prevents interference)
                obstacle.personalSoundBuffer = new SoundBuffer(masterObstacleEngineBuffer);
                obstacle.engineSound = new Sound(*obstacle.personalSoundBuffer);
                obstacle.engineSound->setLooping(true);
                
                // Add random pitch variation for audio diversity
                float pitchVariation = 0.9f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
                obstacle.engineSound->setPitch(pitchVariation);
                
                // Add random volume variation
                float volumeVariation = 0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
                obstacle.soundVolume = volumeVariation;
                
                obstacle.soundInitialized = true;
            }
            
            //--- Spatial Audio System ---
            if (obstacle.soundInitialized && obstacle.engineSound) {
                float distance = calculateDistance(obstacle.position, player.position);
                
                if (distance <= MAX_OBSTACLE_SOUND_DISTANCE) {
                    // Calculate volume based on distance (inverse relationship)
                    float volumeRatio = 1.0f - (distance - MIN_OBSTACLE_SOUND_DISTANCE) / (MAX_OBSTACLE_SOUND_DISTANCE - MIN_OBSTACLE_SOUND_DISTANCE);
                    volumeRatio = max(0.0f, min(1.0f, volumeRatio));
                    
                    // UPDATED: Increased base volume for louder obstacle engines - from 20.0f to 40.0f
                    float baseVolume = volumeRatio * 40.0f * obstacle.soundVolume;  // Base volume calculation (doubled)
                    float adjustedVolume = (baseVolume / 100.0f) * musicVolume;     // Scale by global volume
                    obstacle.engineSound->setVolume(adjustedVolume);
                    
                    // Start playback if volume is sufficient (adjusted threshold for global volume)
                    float minimumThreshold = (2.0f / 100.0f) * musicVolume;  // UPDATED: Increased threshold from 1.0f to 2.0f
                    if (obstacle.engineSound->getStatus() != Sound::Status::Playing && adjustedVolume > minimumThreshold) {
                        obstacle.engineSound->play();
                    }
                    
                    // Dynamic pitch based on relative speed
                    float relativeSpeed = (gameSpeed + obstacle.velocity.y) / 400.0f;
                    float basePitch = 0.9f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
                    float speedPitch = basePitch + (relativeSpeed * 0.3f);
                    obstacle.engineSound->setPitch(speedPitch);
                    
                } else {
                    // Stop audio when too distant
                    if (obstacle.engineSound->getStatus() == Sound::Status::Playing) {
                        obstacle.engineSound->stop();
                    }
                }
            }
        }
        
        //--- Obstacle Cleanup ---
        // Remove obstacles that have moved off-screen
        obstacles.erase(remove_if(obstacles.begin(), obstacles.end(),
            [&](Obstacle& obs) { 
                bool shouldRemove = obs.position.y > static_cast<float>(window.getSize().y) + 50;
                if (shouldRemove && obs.soundInitialized && obs.engineSound && obs.engineSound->getStatus() == Sound::Status::Playing) {
                    obs.engineSound->stop();  // Stop audio before removal
                }
                return shouldRemove;
            }), obstacles.end());
        
        //=== ENHANCED COLLISION DETECTION SYSTEM ===
        // Update player's collision bounds
        if (carSpriteSheetLoaded && player.sprite.has_value()) {
            player.sprite->setPosition(player.position);
        } else {
            player.shape.setPosition(player.position);
        }
        
        // Check collision with each obstacle using smaller hitboxes
        for (const auto& obstacle : obstacles) {
            bool collision = false;
            
            // Use appropriate collision method based on available graphics with reduced hitbox size
            if (carSpriteSheetLoaded && player.sprite.has_value() && obstacle.sprite.has_value()) {
                // Both use sprites - use enhanced sprite collision with smaller hitboxes
                collision = checkSpriteCollision(*player.sprite, *obstacle.sprite, 
                                               player.hitboxSizeMultiplier, obstacle.hitboxSizeMultiplier);
            } else if (carSpriteSheetLoaded && player.sprite.has_value() && !obstacle.sprite.has_value()) {
                // Player has sprite, obstacle uses shape - mixed collision
                collision = checkMixedCollision(*player.sprite, obstacle.shape, 
                                              player.hitboxSizeMultiplier, obstacle.hitboxSizeMultiplier);
            } else if (!player.sprite.has_value() && carSpriteSheetLoaded && obstacle.sprite.has_value()) {
                // Player uses shape, obstacle has sprite - mixed collision (reversed parameters)
                collision = checkMixedCollision(*obstacle.sprite, player.shape, 
                                              obstacle.hitboxSizeMultiplier, player.hitboxSizeMultiplier);
            } else {
                // Both use shapes - enhanced shape collision with smaller hitboxes
                collision = checkCollision(player.shape, obstacle.shape, 
                                         player.hitboxSizeMultiplier, obstacle.hitboxSizeMultiplier);
            }
            
            if (collision) {
                gameOver = true;
                break;  // Exit collision loop early
            }
        }
    }
    
    //=== AUDIO CLEANUP ===
    // Silence obstacle sounds during game over or pedestrian mode
    if (gameOver || playerOutOfCar) {
        for (auto& obstacle : obstacles) {
            if (obstacle.soundInitialized && obstacle.engineSound && obstacle.engineSound->getStatus() == Sound::Status::Playing) {
                obstacle.engineSound->stop();
            }
        }
    }
    
    // Store speed for next frame's audio calculations
    lastGameSpeed = gameSpeed;
    
    //=== RENDERING PIPELINE ===
    // Clear screen with black background
    window.clear(Color::Black);
    
    //--- Background Layer ---
    if (backgroundLoaded && backgroundSprite) {
        // Primary background instance
        backgroundSprite->setPosition(Vector2f(0.0f, backgroundOffset1));
        window.draw(*backgroundSprite);
        
        // Secondary instance for seamless scrolling
        Vector2u textureSize = backgroundTexture.getSize();
        Vector2f scale = backgroundSprite->getScale();
        float scaledHeight = textureSize.y * scale.y;
        
        backgroundSprite->setPosition(Vector2f(0.0f, backgroundOffset1 - scaledHeight));
        window.draw(*backgroundSprite);
    }
    
    //--- Track Layer ---
    static RectangleShape fullRoad({trackWidth, static_cast<float>(window.getSize().y) + 100});
    static RectangleShape leftWall({10, static_cast<float>(window.getSize().y) + 100});
    static RectangleShape rightWall({10, static_cast<float>(window.getSize().y) + 100});
    static bool roadInitialized = false;
    
    // Initialize road graphics (one-time setup)
    if (!roadInitialized) {
        fullRoad.setSize(Vector2f(trackWidth, static_cast<float>(window.getSize().y) + 100));
        if (backgroundLoaded) {
            fullRoad.setFillColor(Color(102, 102, 102, 255));  // Gray road surface
        }
        
        leftWall.setSize(Vector2f(10, static_cast<float>(window.getSize().y) + 100));
        leftWall.setFillColor(Color::White);
        
        rightWall.setSize(Vector2f(10, static_cast<float>(window.getSize().y) + 100));
        rightWall.setFillColor(Color::White);
        
        roadInitialized = true;
    }
    
    // Animate road surface for speed illusion
    static float roadOffset = 0;
    if (!gameOver && !playerOutOfCar && gameSpeed > 55.0f) {
        roadOffset += gameSpeed * deltaTime;
        if (roadOffset >= 50) roadOffset -= 50;  // Reset for continuous animation
    }
    
    // Position and render track elements
    float centerX = static_cast<float>(window.getSize().x) / 2.0f;
    fullRoad.setPosition(Vector2f(centerX - trackWidth/2, -50 + roadOffset));
    leftWall.setPosition(Vector2f(centerX - trackWidth/2 - 10, -50 + roadOffset));
    rightWall.setPosition(Vector2f(centerX + trackWidth/2, -50 + roadOffset));
    
    window.draw(fullRoad);
    window.draw(leftWall);
    window.draw(rightWall);
    
    //--- Obstacle Layer ---
    for (const auto& obstacle : obstacles) {
        if (carSpriteSheetLoaded && obstacle.sprite.has_value()) {
            window.draw(*obstacle.sprite);  // Render sprite
        } else {
            window.draw(obstacle.shape);   // Render fallback rectangle
        }
    }
    
    //--- Player/Vehicle Layer ---
    if (playerOutOfCar) {
        // Render abandoned vehicle at stored location
        if (carSpriteSheetLoaded && abandonedCarSprite.has_value()) {
            abandonedCarSprite->setPosition(carPosition);
            window.draw(*abandonedCarSprite);
        } else {
            carShape.setPosition(carPosition);
            window.draw(carShape);
        }
        
        // Render pedestrian player
        player.shape.setFillColor(Color::Blue);
        player.shape.setPosition(player.position);
        window.draw(player.shape);
    } else {
        // Render player in vehicle
        if (carSpriteSheetLoaded && player.sprite.has_value()) {
            player.sprite->setPosition(player.position);
            window.draw(*player.sprite);
        } else {
            player.shape.setFillColor(Color::Red);
            player.shape.setPosition(player.position);
            window.draw(player.shape);
        }
    }
    
    //--- UI Text Layer ---
    // Render narrative text sequence
    if (textSequenceStarted && currentTextIndex >= 0 && currentTextIndex < static_cast<int>(secretTexts.size())) {
        Text secretText(font, secretTexts[currentTextIndex], 32);
        secretText.setFillColor(Color::Cyan);
        secretText.setOutlineColor(Color::Black);
        secretText.setOutlineThickness(2.f);
        
        float textX = 50.0f;
        float textY = 200.0f;
        
        secretText.setPosition(Vector2f(textX, textY));
        window.draw(secretText);
    }
    
    // Render help system hint
    if (textSequenceCompleted && !helpRequested) {
        Text helpHint(font, "H - Help", 20);
        helpHint.setFillColor(Color::White);
        helpHint.setOutlineColor(Color::Black);
        helpHint.setOutlineThickness(2.f);
        auto hintBounds = helpHint.getLocalBounds();
        helpHint.setOrigin(Vector2f(hintBounds.size.x, 0));  // Right-align
        helpHint.setPosition(Vector2f(static_cast<float>(window.getSize().x) - 20.f, 50.f));
        window.draw(helpHint);
    }
    
    // Render help instructions overlay
    if (helpRequested) {
        // Main title
        Text helpText1(font, "To end this level:", 28);
        helpText1.setFillColor(Color::Yellow);
        helpText1.setOutlineColor(Color::Black);
        helpText1.setOutlineThickness(2.f);
        auto bounds1 = helpText1.getLocalBounds();
        helpText1.setOrigin(Vector2f(bounds1.size.x / 2.f, bounds1.size.y / 2.f));
        helpText1.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f - 80));
        window.draw(helpText1);
        
        // Step-by-step instructions
        Text helpText2(font, "Stop the car", 24);
        helpText2.setFillColor(Color::White);
        helpText2.setOutlineColor(Color::Black);
        helpText2.setOutlineThickness(2.f);
        auto bounds2 = helpText2.getLocalBounds();
        helpText2.setOrigin(Vector2f(bounds2.size.x / 2.f, bounds2.size.y / 2.f));
        helpText2.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f - 40));
        window.draw(helpText2);
        
        Text helpText3(font, "Get some fresh air", 24);
        helpText3.setFillColor(Color::White);
        helpText3.setOutlineColor(Color::Black);
        helpText3.setOutlineThickness(2.f);
        auto bounds3 = helpText3.getLocalBounds();
        helpText3.setOrigin(Vector2f(bounds3.size.x / 2.f, bounds3.size.y / 2.f));
        helpText3.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f));
        window.draw(helpText3);
        
        Text helpText4(font, "Leave this place", 24);
        helpText4.setFillColor(Color::White);
        helpText4.setOutlineColor(Color::Black);
        helpText4.setOutlineThickness(2.f);
        auto bounds4 = helpText4.getLocalBounds();
        helpText4.setOrigin(Vector2f(bounds4.size.x / 2.f, bounds4.size.y / 2.f));
        helpText4.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f + 40));
        window.draw(helpText4);
        
        // Dynamic status feedback
        if (gameSpeed <= 55.0f && !playerOutOfCar) {
            Text statusText(font, "Car stopped! Press F to get out", 20);
            statusText.setFillColor(Color::Green);
            statusText.setOutlineColor(Color::Black);
            statusText.setOutlineThickness(2.f);
            auto statusBounds = statusText.getLocalBounds();
            statusText.setOrigin(Vector2f(statusBounds.size.x / 2.f, statusBounds.size.y / 2.f));
            statusText.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f + 80));
            window.draw(statusText);
        } else if (playerOutOfCar) {
            Text statusText(font, "Be free from this nightmare", 20);
            statusText.setFillColor(Color::Cyan);
            statusText.setOutlineColor(Color::Black);
            statusText.setOutlineThickness(2.f);
            auto statusBounds = statusText.getLocalBounds();
            statusText.setOrigin(Vector2f(statusBounds.size.x / 2.f, statusBounds.size.y / 2.f));
            statusText.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f + 80));
            window.draw(statusText);
        }
        
        // Help close instruction
        Text closeHelp(font, "Press H again to close help", 16);
        closeHelp.setFillColor(Color::White);
        closeHelp.setPosition(Vector2f(20, static_cast<float>(window.getSize().y) - 40));
        window.draw(closeHelp);
    }
    
    // Vehicle exit prompt
    if (textSequenceCompleted && !helpRequested && gameSpeed <= 55.0f && !playerOutOfCar) {
        Text exitHint(font, "Press F to exit car", 20);
        exitHint.setFillColor(Color::Green);
        exitHint.setOutlineColor(Color::Black);
        exitHint.setOutlineThickness(2.f);
        auto exitBounds = exitHint.getLocalBounds();
        exitHint.setOrigin(Vector2f(exitBounds.size.x / 2.f, exitBounds.size.y / 2.f));
        exitHint.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) - 100));
        window.draw(exitHint);
    }
    
    // Core game UI elements
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
    
    //=== GAME OVER INTERFACE ===
    if (gameOver) {
        // Game over message
        Text gameOverText(font, "GAME OVER! Distance: " + to_string(score) + "m", 36);
        gameOverText.setFillColor(Color::Red);
        gameOverText.setOutlineColor(Color::Black);
        gameOverText.setOutlineThickness(3.f);
        auto bounds = gameOverText.getLocalBounds();
        gameOverText.setOrigin(Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        gameOverText.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f - 40));
        window.draw(gameOverText);
        
        // Restart prompt
        Text restartText(font, "Press R to restart", 32);
        restartText.setFillColor(Color::White);
        restartText.setOutlineColor(Color::Black);
        restartText.setOutlineThickness(2.f);
        auto restartBounds = restartText.getLocalBounds();
        restartText.setOrigin(Vector2f(restartBounds.size.x / 2.f, restartBounds.size.y / 2.f));
        restartText.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f + 20));
        window.draw(restartText);
        
        //=== RESTART SYSTEM ===
        if (Keyboard::isKeyPressed(Keyboard::Key::R)) {
            // Audio cleanup before reset
            for (auto& obstacle : obstacles) {
                if (obstacle.soundInitialized && obstacle.engineSound && obstacle.engineSound->getStatus() == Sound::Status::Playing) {
                    obstacle.engineSound->stop();
                }
            }
            
            // Reset all game state to initial values
            gameOver = false;
            obstacles.clear();
            track.clear();
            score = 0;
            totalDistance = 0.0f;
            gameSpeed = 200.0f;
            player.position = Vector2f(static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) * 0.8f);
            player.shape.setSize(Vector2f(30, 50));
            player.shape.setOrigin(Vector2f(15, 25));
            gameTimer.restart();
            roadInitialized = false;
            gameInitialized = false;
            helpRequested = false;
            playerOutOfCar = false;
            
            // Clear abandoned car reference
            abandonedCarSprite.reset();
        }
    }

    //=== NAVIGATION CONTROLS ===
    // Static variables for input state tracking to prevent key repeat
    static bool mPressed = false;        // M key state
    static bool f1Pressed = false;      // F1 key state
    static bool escPressed = false;     // ESC key state (NEW)
    
    // NEW: Return to pre-level screen (ESC key)
    if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
        if (!escPressed) {  // Edge detection to prevent key repeat
            // Complete cleanup before state transition
            if (engineMusicLoaded && engineMusic.getStatus() == Music::Status::Playing) {
                engineMusic.stop();
            }
            
            for (auto& obstacle : obstacles) {
                if (obstacle.soundInitialized && obstacle.engineSound && obstacle.engineSound->getStatus() == Sound::Status::Playing) {
                    obstacle.engineSound->stop();
                }
            }
            
            if (backgroundSprite) {
                delete backgroundSprite;
                backgroundSprite = nullptr;
                backgroundLoaded = false;
            }
            
            state = PRELEVEL3;              // Return to Level 3 pre-level screen
            gameInitialized = false;        // Reset for potential restart
            levelTimersInitialized = false; // Reset level timers
            escPressed = true;              // Mark key as pressed
        }
    }
    else {
        escPressed = false;  // Reset when key released
    }
    
    // Return to main menu (M key)
    if (Keyboard::isKeyPressed(Keyboard::Key::M)) {
        if (!mPressed) {  // Edge detection to prevent key repeat
            // Complete cleanup before state transition
            if (engineMusicLoaded && engineMusic.getStatus() == Music::Status::Playing) {
                engineMusic.stop();
            }
            
            for (auto& obstacle : obstacles) {
                if (obstacle.soundInitialized && obstacle.engineSound && obstacle.engineSound->getStatus() == Sound::Status::Playing) {
                    obstacle.engineSound->stop();
                }
            }
            
            if (backgroundSprite) {
                delete backgroundSprite;
                backgroundSprite = nullptr;
                backgroundLoaded = false;
            }
            
            state = MENU;
            gameInitialized = false;
            levelTimersInitialized = false;
            mPressed = true;                // Mark key as pressed
        }
    }
    else {
        mPressed = false;  // Reset when key released
    }
    
    // Access settings menu (F1 key)
    if (Keyboard::isKeyPressed(Keyboard::Key::F1)) {
        if (!f1Pressed) {  // Edge detection to prevent key repeat
            extern GameState previousState;
            previousState = PLAYING3;
            state = SETTINGS;
            f1Pressed = true;              // Mark key as pressed
        }
    }
    else {
        f1Pressed = false;  // Reset when key released
    }
}