#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <memory>
#include <cmath>

//Declaring  Constants
const int WINDOW_WIDTH = 1000; //defining window width
const int WINDOW_HEIGHT = 800; //defining window height
const std::string SNAKE_HIGHSCORE_FILE = "snake_highscores.txt"; 
const std::string FLAPPY_HIGHSCORE_FILE = "flappy_highscores.txt";
const std::string MUTE_TEXT = "Music: T to toggle";

// Texture paths
const std::string MENU_BACKGROUND = "D:/happy/mycode/Assets/Images/mainbackground.png";
const std::string SNAKE_BACKGROUND = "D:/happy/mycode/Assets/Images/snakebackground.png";
const std::string FLAPPY_BACKGROUND = "D:/happy/mycode/Assets/Images/flappybirdbackground.png";
//bird texture
const std::string BIRD_TEXTURE = "D:/happy/mycode/Assets/Images/basimbird.png";
//sound paths
const std::string GAME_OVER_SOUND = "D:/happy/mycode/Assets/Audio/gameover.wav";
const std::string POINT_SOUND = "D:/happy/mycode/Assets/Audio/eat.wav";
const std::string BG_MUSIC = "D:/happy/mycode/Assets/Audio/bgMusic.wav";

// Snake game textures
const std::string SNAKE_BODY_TEXTURE = "D:/happy/mycode/Assets/Images/snakebody.png";
const std::string SNAKE_FOOD_TEXTURE = "D:/happy/mycode/Assets/Images/food.png";

// Base Game Class
class Game {
    //Implementing encapsulation
protected:
    //declaring variables
    sf::RenderWindow& window;
    sf::Font font;
    bool gameOver;
    bool musicMuted;
    int score;
    int highScore;
    int lives;
    sf::SoundBuffer gameOverBuffer;
    sf::Sound gameOverSound;
    sf::SoundBuffer pointBuffer;
    sf::Sound pointSound;
    std::string highScoreFile;
    sf::Texture backgroundTexture;
    sf::Sprite background;
    sf::Text muteText;

public:
    Game(sf::RenderWindow& win, const std::string& hsFile, const std::string& bgPath)
        : window(win), gameOver(false), musicMuted(false),
        score(0), highScore(0), lives(3), highScoreFile(hsFile) { 
        if (!font.loadFromFile("D:/happy/mycode/Assets/Font/Arcade_R.ttf")) {
            std::cerr << "Failed to load font" << std::endl;
        }

        // Load background
        if (!backgroundTexture.loadFromFile(bgPath)) { //error message displayed  in case background doesnt work
            std::cerr << "Failed to load background texture" << std::endl;
        }
        background.setTexture(backgroundTexture);
        float scaleX = static_cast<float>(WINDOW_WIDTH) / backgroundTexture.getSize().x;
        float scaleY = static_cast<float>(WINDOW_HEIGHT) / backgroundTexture.getSize().y;
        background.setScale(scaleX, scaleY);

        loadSounds();
        loadHighScore();

        // Setup mute text
        muteText.setFont(font);
        muteText.setString(MUTE_TEXT);
        muteText.setCharacterSize(20);
        muteText.setFillColor(sf::Color::White);
        muteText.setPosition(WINDOW_WIDTH - muteText.getLocalBounds().width - 10, 10);
    }

    virtual ~Game() {} //virtual destructor to ensure objects are destroyed in correct order

    virtual void handleInput() {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
            musicMuted = !musicMuted;
            sf::sleep(sf::milliseconds(200));
        }
    }

    virtual void update() = 0;
    virtual void draw() {
        window.draw(background);
        muteText.setString(std::string(MUTE_TEXT) + (musicMuted ? " (OFF)" : " (ON)"));
        window.draw(muteText);
    };
    virtual void reset() = 0; //pure virtual function to ensure each class overrides this function

    //function to handle audio files in game
    void loadSounds() {
        if (!gameOverBuffer.loadFromFile(GAME_OVER_SOUND)) {
            std::cerr << "Failed to load game over sound" << std::endl;
        }
        gameOverSound.setBuffer(gameOverBuffer);

        if (!pointBuffer.loadFromFile(POINT_SOUND)) {
            std::cerr << "Failed to load point sound" << std::endl;
        }
        pointSound.setBuffer(pointBuffer);
    }

    //function to display highscore
    void loadHighScore() {
        std::ifstream file(highScoreFile);
        if (file.is_open()) {
            file >> highScore;
            file.close();
        }
    }

    //function to keep updating high score
    void saveHighScore() {
        if (score > highScore) {
            highScore = score;
            std::ofstream file(highScoreFile);
            if (file.is_open()) {
                file << highScore;
                file.close();
            }
        }
    }

    bool isGameOver() const { return gameOver; }
    bool isMusicMuted() const { return musicMuted; }
    int getScore() const { return score; }
    int getHighScore() const { return highScore; }
};

// Snake Game
class SnakeGame : public Game {
private:
    struct SnakeSegment {
        sf::Sprite sprite;
        sf::Vector2f position;
        float rotation;
    };

    std::vector<SnakeSegment> snake;
    sf::Vector2f direction;
    sf::Sprite food;
    sf::Texture bodyTexture;
    sf::Texture foodTexture;
    float gridSize;
    sf::Clock moveClock;
    float moveDelay;

public:
    SnakeGame(sf::RenderWindow& win) : Game(win, SNAKE_HIGHSCORE_FILE, SNAKE_BACKGROUND), gridSize(32.f), moveDelay(0.15f) {
        // Load textures
        if (!bodyTexture.loadFromFile(SNAKE_BODY_TEXTURE)) {
            std::cerr << "Failed to load snake body texture" << std::endl;
        }
        if (!foodTexture.loadFromFile(SNAKE_FOOD_TEXTURE)) {
            std::cerr << "Failed to load food texture" << std::endl;
        }

        reset();
    }

    void reset() override { //overriding reset function
        snake.clear();
        gameOver = false;  
        score = 0;  //giving user an initial score of 0
        lives = 3; //giving player 3 lives

        // Initial snake
        for (int i = 0; i < 3; ++i) {
            SnakeSegment segment;
            segment.sprite.setTexture(bodyTexture);
            segment.sprite.setOrigin(bodyTexture.getSize().x / 2.0f, bodyTexture.getSize().y / 2.0f);
            segment.sprite.setScale(0.5f, 0.5f); // Scale down the sprite
            segment.position = sf::Vector2f(
                static_cast<float>(WINDOW_WIDTH) / 2.0f,
                static_cast<float>(WINDOW_HEIGHT) / 2.0f + i * gridSize
            );
            segment.rotation = 0.f;
            segment.sprite.setPosition(segment.position);
            segment.sprite.setRotation(segment.rotation);
            snake.push_back(segment);
        }

        direction = sf::Vector2f(0, -1); // Up
        spawnFood();
        moveClock.restart();
    }

    void spawnFood() { //function to generate food 
        food.setTexture(foodTexture);
        food.setOrigin(foodTexture.getSize().x / 2.0f, foodTexture.getSize().y / 2.0f);
        food.setScale(0.5f, 0.5f); // Scale down the food

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> xDist(0, static_cast<int>((WINDOW_WIDTH / gridSize) - 1));
        std::uniform_int_distribution<> yDist(0, static_cast<int>((WINDOW_HEIGHT / gridSize) - 1));

        //function to randomly set position where food spawns
        food.setPosition(
            static_cast<float>(xDist(gen)) * gridSize + gridSize / 2,
            static_cast<float>(yDist(gen)) * gridSize + gridSize / 2
        );
    }
    //function to take user inpiut
    void handleInput() override {
        Game::handleInput(); // Handle common input first

        if (gameOver) return;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && direction.y == 0)
            direction = sf::Vector2f(0, -1);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && direction.y == 0)
            direction = sf::Vector2f(0, 1);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && direction.x == 0)
            direction = sf::Vector2f(-1, 0);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && direction.x == 0)
            direction = sf::Vector2f(1, 0);
    }
    //function to keep track if game is in session
    void update() override {
        if (gameOver) return; 

        if (moveClock.getElapsedTime().asSeconds() > moveDelay) {
            moveClock.restart();

            // Move snake
            for (int i = static_cast<int>(snake.size()) - 1; i > 0; --i) {
                snake[i].position = snake[i - 1].position;
                snake[i].rotation = snake[i - 1].rotation;
                snake[i].sprite.setPosition(snake[i].position);
                snake[i].sprite.setRotation(snake[i].rotation);
            }

            // Update head position and rotation
            snake[0].position += direction * gridSize;

            // Set rotation based on direction
            if (direction.x == 1) snake[0].rotation = 0; // Right
            else if (direction.x == -1) snake[0].rotation = 180; // Left
            else if (direction.y == -1) snake[0].rotation = 270; // Up
            else if (direction.y == 1) snake[0].rotation = 90; // Down

            snake[0].sprite.setPosition(snake[0].position);
            snake[0].sprite.setRotation(snake[0].rotation);

            // Check collisions with walls
            if (snake[0].position.x < gridSize / 2 || snake[0].position.x >= WINDOW_WIDTH - gridSize / 2 ||
                snake[0].position.y < gridSize / 2 || snake[0].position.y >= WINDOW_HEIGHT - gridSize / 2) {
                lives--;
                if (lives <= 0) {
                    gameOver = true;
                    if (!musicMuted) gameOverSound.play();
                    saveHighScore();
                }
                else {
                    // Reset position but keep score
                    snake[0].position = sf::Vector2f(
                        static_cast<float>(WINDOW_WIDTH) / 2.0f,
                        static_cast<float>(WINDOW_HEIGHT) / 2.0f
                    );
                    direction = sf::Vector2f(0, -1);
                }
            }

            // Check collision with self
            for (size_t i = 1; i < snake.size(); ++i) {
                if (snake[0].position == snake[i].position) {
                    lives--;
                    if (lives <= 0) {
                        gameOver = true;
                        if (!musicMuted) gameOverSound.play();
                        saveHighScore();
                    }
                    else {
                        // Reset position but keep score
                        snake[0].position = sf::Vector2f(
                            static_cast<float>(WINDOW_WIDTH) / 2.0f,
                            static_cast<float>(WINDOW_HEIGHT) / 2.0f
                        );
                        direction = sf::Vector2f(0, -1);
                    }
                    break;
                }
            }

            // Check collision with food
            if (snake[0].sprite.getGlobalBounds().intersects(food.getGlobalBounds())) {
                if (!musicMuted) pointSound.play();
                score += 10;

                // Add new segment
                SnakeSegment newSegment;
                newSegment.sprite.setTexture(bodyTexture);
                newSegment.sprite.setOrigin(bodyTexture.getSize().x / 2.0f, bodyTexture.getSize().y / 2.0f);
                newSegment.sprite.setScale(0.5f, 0.5f);
                newSegment.position = snake.back().position;
                newSegment.rotation = snake.back().rotation;
                newSegment.sprite.setPosition(newSegment.position);
                newSegment.sprite.setRotation(newSegment.rotation);
                snake.push_back(newSegment);

                spawnFood();
            }
        }
    }

    void draw() override {
        Game::draw(); // Draw background first

        // Draw food
        window.draw(food);

        // Draw snake
        for (auto& segment : snake) {
            window.draw(segment.sprite);
        }

        // Draw UI
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setString("Score: " + std::to_string(score));
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition(10, 10);
        window.draw(scoreText);

        sf::Text highScoreText;
        highScoreText.setFont(font);
        highScoreText.setString("High Score: " + std::to_string(highScore));
        highScoreText.setCharacterSize(20);
        highScoreText.setFillColor(sf::Color::Black);
        highScoreText.setPosition(10, 40);
        window.draw(highScoreText);

        sf::Text livesText;
        livesText.setFont(font);
        livesText.setString("Lives: " + std::to_string(lives));
        livesText.setCharacterSize(20);
        livesText.setFillColor(sf::Color::Black);
        livesText.setPosition(10, 70);
        window.draw(livesText);

        if (gameOver) {
            sf::Text gameOverText;
            gameOverText.setFont(font);
            gameOverText.setString("GAME OVER\nPress R to Restart\nPress M for Menu");
            gameOverText.setCharacterSize(30);
            gameOverText.setFillColor(sf::Color::Red);
            gameOverText.setPosition(
                static_cast<float>(WINDOW_WIDTH) / 2.0f - gameOverText.getLocalBounds().width / 2.0f,
                static_cast<float>(WINDOW_HEIGHT) / 2.0f - 50.0f
            );
            window.draw(gameOverText);
        }
    }
};

// Flappy Bird Game
class FlappyBirdGame : public Game {
private:
    sf::Sprite bird;
    sf::Texture birdTexture;
    float birdVelocity;
    float gravity;
    std::vector<sf::RectangleShape> pipes;
    float pipeSpeed;
    float pipeGap;
    float pipeSpawnTimer;
    float pipeSpawnDelay;
    bool passedPipe;

public: // Rendering Flappy Bird 
    FlappyBirdGame(sf::RenderWindow& win) : Game(win, FLAPPY_HIGHSCORE_FILE, FLAPPY_BACKGROUND),
        birdVelocity(0.f), gravity(0.5f), pipeSpeed(3.f),
        pipeGap(200.f), pipeSpawnTimer(0.f), pipeSpawnDelay(2.f), passedPipe(false) {
        reset();
    }

    void reset() override { // Giving User inital score of 0 and 3 Lives at the start
        gameOver = false;
        score = 0;
        lives = 3;

        // Load bird texture
        if (!birdTexture.loadFromFile(BIRD_TEXTURE)) {
            std::cerr << "Failed to load bird texture" << std::endl;
        }

        bird.setTexture(birdTexture);
        bird.setScale(0.1f, 0.1f); // Adjust scale as needed
        bird.setOrigin(birdTexture.getSize().x / 2.0f, birdTexture.getSize().y / 2.0f);
        bird.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 4.0f,
            static_cast<float>(WINDOW_HEIGHT) / 2.0f
        );
        birdVelocity = 0.f;
        bird.setRotation(0);

        pipes.clear();
        pipeSpawnTimer = 0.f;
        passedPipe = false;
    }

    void spawnPipe() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> heightDist(100, WINDOW_HEIGHT - 300);

        float height = static_cast<float>(heightDist(gen));

        // Upper pipe
        sf::RectangleShape upperPipe;
        upperPipe.setSize(sf::Vector2f(80, height));
        upperPipe.setFillColor(sf::Color::Green);
        upperPipe.setPosition(static_cast<float>(WINDOW_WIDTH), 0.0f);
        pipes.push_back(upperPipe);

        // Lower pipe
        sf::RectangleShape lowerPipe;
        lowerPipe.setSize(sf::Vector2f(80, static_cast<float>(WINDOW_HEIGHT) - height - pipeGap));
        lowerPipe.setFillColor(sf::Color::Green);
        lowerPipe.setPosition(static_cast<float>(WINDOW_WIDTH), height + pipeGap);
        pipes.push_back(lowerPipe);
    }

    void handleInput() override {
        Game::handleInput();

        //function to keep track if game is in session
        if (gameOver) return;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            birdVelocity = -10.f;
            bird.setRotation(-30); // Tilt up when jumping
        }
    }

    void update() override {
        if (gameOver) return;

        // Bird physics
        birdVelocity += gravity;
        bird.move(0, birdVelocity);

        // Gradually rotate bird downward
        if (bird.getRotation() < 90 && birdVelocity > 0) {
            bird.rotate(2.0f);
        }

        // Check collisions with ground or ceiling
        if (bird.getPosition().y <= 0 ||
            bird.getPosition().y + bird.getGlobalBounds().height / 2 >= WINDOW_HEIGHT) {
            lives--;
            if (lives <= 0) {
                gameOver = true;
                if (!musicMuted) gameOverSound.play();
                saveHighScore();
            }
            else {
                // Reset bird position
                bird.setPosition(
                    static_cast<float>(WINDOW_WIDTH) / 4.0f,
                    static_cast<float>(WINDOW_HEIGHT) / 2.0f
                );
                birdVelocity = 0.f;
                bird.setRotation(0);
            }
        }

        // Pipe spawning
        pipeSpawnTimer += 0.01f;
        if (pipeSpawnTimer >= pipeSpawnDelay) {
            pipeSpawnTimer = 0;
            spawnPipe();
        }

        // Pipe movement and collision
        for (auto it = pipes.begin(); it != pipes.end(); ) {
            it->move(-pipeSpeed, 0);

            // Check collision with bird
            if (bird.getGlobalBounds().intersects(it->getGlobalBounds())) {
                lives--;
                if (lives <= 0) {
                    gameOver = true;
                    if (!musicMuted) gameOverSound.play();
                    saveHighScore();
                }
                else {
                    // Reset bird position
                    bird.setPosition(
                        static_cast<float>(WINDOW_WIDTH) / 4.0f,
                        static_cast<float>(WINDOW_HEIGHT) / 2.0f
                    );
                    birdVelocity = 0.f;
                    bird.setRotation(0);
                    it = pipes.erase(it);
                    continue;
                }
            }

            // Check if bird passed the pipe
            if (!passedPipe && it->getPosition().x + it->getSize().x < bird.getPosition().x) {
                passedPipe = true;
                if (!musicMuted) pointSound.play();
                score += 5;
            }

            // Remove off-screen pipes
            if (it->getPosition().x + it->getSize().x < 0) {
                it = pipes.erase(it);
                passedPipe = false;
            }
            else {
                ++it;
            }
        }
    }

    void draw() override {
        Game::draw();

        // Draw pipes
        for (auto& pipe : pipes) {
            window.draw(pipe);
        }

        // Draw bird
        window.draw(bird);

        // Draw UI
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setString("Score: " + std::to_string(score));
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10, 10);
        window.draw(scoreText);

        sf::Text highScoreText;
        highScoreText.setFont(font);
        highScoreText.setString("High Score: " + std::to_string(highScore));
        highScoreText.setCharacterSize(20);
        highScoreText.setFillColor(sf::Color::White);
        highScoreText.setPosition(10, 40);
        window.draw(highScoreText);

        sf::Text livesText;
        livesText.setFont(font);
        livesText.setString("Lives: " + std::to_string(lives));
        livesText.setCharacterSize(20);
        livesText.setFillColor(sf::Color::White);
        livesText.setPosition(10, 70);
        window.draw(livesText);

        if (gameOver) {
            sf::Text gameOverText;
            gameOverText.setFont(font);
            gameOverText.setString("GAME OVER\nPress R to Restart\nPress M for Menu");
            gameOverText.setCharacterSize(30);
            gameOverText.setFillColor(sf::Color::Red);
            gameOverText.setPosition(
                static_cast<float>(WINDOW_WIDTH) / 2.0f - gameOverText.getLocalBounds().width / 2.0f,
                static_cast<float>(WINDOW_HEIGHT) / 2.0f - 50.0f
            );
            window.draw(gameOverText);
        }
    }
};

// High Scores Screen
class HighScoresScreen {
private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Text title;
    sf::Text snakeHighScoreText;
    sf::Text flappyHighScoreText;
    sf::Text backText;
    int snakeHighScore;
    int flappyHighScore;
    sf::Texture backgroundTexture;
    sf::Sprite background;

public:
    HighScoresScreen(sf::RenderWindow& win) : window(win), snakeHighScore(0), flappyHighScore(0) {
        if (!font.loadFromFile("D:/happy/mycode/Assets/Font/Arcade_R.ttf")) {
            std::cerr << "Failed to load font" << std::endl;
        }

        // Load background (using menu background)
        if (!backgroundTexture.loadFromFile(MENU_BACKGROUND)) {
            std::cerr << "Failed to load background texture" << std::endl;
        }
        background.setTexture(backgroundTexture);
        // Scale background to fit window
        float scaleX = static_cast<float>(WINDOW_WIDTH) / backgroundTexture.getSize().x;
        float scaleY = static_cast<float>(WINDOW_HEIGHT) / backgroundTexture.getSize().y;
        background.setScale(scaleX, scaleY);

        loadHighScores();
        setupText();
    }
    // function to update display high score 
    void loadHighScores() {
        std::ifstream snakeFile(SNAKE_HIGHSCORE_FILE);
        if (snakeFile.is_open()) {
            snakeFile >> snakeHighScore;
            snakeFile.close();
        }

        std::ifstream flappyFile(FLAPPY_HIGHSCORE_FILE);
        if (flappyFile.is_open()) {
            flappyFile >> flappyHighScore;
            flappyFile.close();
        }
    }

    void setupText() {
        title.setFont(font);
        title.setString("HIGH SCORES");
        title.setCharacterSize(50);
        title.setFillColor(sf::Color::Cyan);
        title.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - title.getLocalBounds().width / 2.0f,
            100.0f
        );

        snakeHighScoreText.setFont(font);
        snakeHighScoreText.setString("Eat The Emoji: " + std::to_string(snakeHighScore));
        snakeHighScoreText.setCharacterSize(30);
        snakeHighScoreText.setFillColor(sf::Color::White);
        snakeHighScoreText.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - snakeHighScoreText.getLocalBounds().width / 2.0f,
            200.0f
        );

        flappyHighScoreText.setFont(font);
        flappyHighScoreText.setString("Basim Bird: " + std::to_string(flappyHighScore));
        flappyHighScoreText.setCharacterSize(30);
        flappyHighScoreText.setFillColor(sf::Color::White);
        flappyHighScoreText.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - flappyHighScoreText.getLocalBounds().width / 2.0f,
            250.0f
        );

        backText.setFont(font);
        backText.setString("Press B to go back");
        backText.setCharacterSize(20);
        backText.setFillColor(sf::Color::White);
        backText.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - backText.getLocalBounds().width / 2.0f,
            350.0f
        );
    }

    void handleInput(bool& showHighScores) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
            showHighScores = false;
        }
    }

    void draw() {
        window.draw(background);
        window.draw(title);
        window.draw(snakeHighScoreText);
        window.draw(flappyHighScoreText);
        window.draw(backText);
    }
};

// Instructions Screen
class InstructionsScreen {
private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Text title;
    sf::Text snakeInstructions;
    sf::Text flappyInstructions;
    sf::Text backText;
    sf::Texture backgroundTexture;
    sf::Sprite background;

public:
    InstructionsScreen(sf::RenderWindow& win) : window(win) {
        if (!font.loadFromFile("D:/happy/mycode/Assets/Font/Arcade_R.ttf")) {
            std::cerr << "Failed to load font" << std::endl;
        }

        // Load background (using menu background)
        if (!backgroundTexture.loadFromFile(MENU_BACKGROUND)) {
            std::cerr << "Failed to load background texture" << std::endl;
        }
        background.setTexture(backgroundTexture);
        // Scale background to fit window
        float scaleX = static_cast<float>(WINDOW_WIDTH) / backgroundTexture.getSize().x;
        float scaleY = static_cast<float>(WINDOW_HEIGHT) / backgroundTexture.getSize().y;
        background.setScale(scaleX, scaleY);

        setupText();
    }

    void setupText() {
        title.setFont(font);
        title.setString("\n\nINSTRUCTIONS");
        title.setCharacterSize(50);
        title.setFillColor(sf::Color::Cyan);
        title.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - title.getLocalBounds().width / 2.0f,
            50.0f
        );

        snakeInstructions.setFont(font);
        snakeInstructions.setString(
            "\n\n\n\n\n\t---EAT THE EMOJI---\n"
            "Use arrow keys to move\n"
            "Eat red food to grow\n"
            "Don't hit walls or yourself\n"
            "You have 3 lives\n"
        );
        snakeInstructions.setCharacterSize(20);
        snakeInstructions.setFillColor(sf::Color::White);
        snakeInstructions.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - snakeInstructions.getLocalBounds().width / 2.0f,
            120.0f
        );

        flappyInstructions.setFont(font);
        flappyInstructions.setString(
            "\n\n\n\n\n\n\t\t  ---BASIM BIRD---\n"
            "\tPress SPACE to jump\n"
            "\tNavigate through pipes\n"
            "\tEach passed pipe gives 5 points\n"
            "\tYou have 3 lives\n"
        );
        flappyInstructions.setCharacterSize(20);
        flappyInstructions.setFillColor(sf::Color::White);
        flappyInstructions.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - flappyInstructions.getLocalBounds().width / 2.0f,
            250.0f
        );

        backText.setFont(font);
        backText.setString("\n\n\n\n\n\n\n\n\n\nPress B to go back");
        backText.setCharacterSize(20);
        backText.setFillColor(sf::Color::White);
        backText.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - backText.getLocalBounds().width / 2.0f,
            350.0f
        );
    }

    void handleInput(bool& showInstructions) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
            showInstructions = false;
        }
    }

    void draw() {
        window.draw(background);
        window.draw(title);
        window.draw(snakeInstructions);
        window.draw(flappyInstructions);
        window.draw(backText);
    }
};

// Main Menu
class MainMenu {
private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Text title;
    sf::Text snakeText;
    sf::Text flappyText;
    sf::Text instructionsText;
    sf::Text highScoresText;
    sf::Text exitText;
    int selectedItem;
    std::vector<sf::Text> menuItems;
    sf::Clock keyPressClock;
    float keyPressDelay;
    bool keyProcessed;
    sf::Texture backgroundTexture;
    sf::Sprite background;

public:
    MainMenu(sf::RenderWindow& win)
        : window(win), selectedItem(0), keyPressDelay(0.2f), keyProcessed(false) {
        if (!font.loadFromFile("D:/happy/mycode/Assets/Font/Arcade_R.ttf")) {
            std::cerr << "Failed to load font" << std::endl;
        }

        // Load background
        if (!backgroundTexture.loadFromFile(MENU_BACKGROUND)) {
            std::cerr << "Failed to load background texture" << std::endl;
        }
        background.setTexture(backgroundTexture);
        // Scale background to fit window
        float scaleX = static_cast<float>(WINDOW_WIDTH) / backgroundTexture.getSize().x;
        float scaleY = static_cast<float>(WINDOW_HEIGHT) / backgroundTexture.getSize().y;
        background.setScale(scaleX, scaleY);

        setupText();
        updateSelection();
    }
    // Function used to display all text throughout the code
    void setupText() {
        title.setFont(font);
        title.setString("\n\nARCADE SIMULATOR");
        title.setCharacterSize(50);
        title.setFillColor(sf::Color::Cyan);
        title.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - title.getLocalBounds().width / 2.0f,
            50.0f
        );

        snakeText.setFont(font);
        snakeText.setString("\n\n\nEat The Emoji");
        snakeText.setCharacterSize(30);
        snakeText.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - snakeText.getLocalBounds().width / 2.0f,
            150.0f
        );

        flappyText.setFont(font);
        flappyText.setString("\n\n\nBasim Bird");
        flappyText.setCharacterSize(30);
        flappyText.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - flappyText.getLocalBounds().width / 2.0f,
            200.0f
        );

        instructionsText.setFont(font);
        instructionsText.setString("\n\n\nInstructions");
        instructionsText.setCharacterSize(30);
        instructionsText.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - instructionsText.getLocalBounds().width / 2.0f,
            250.0f
        );

        highScoresText.setFont(font);
        highScoresText.setString("\n\n\nHigh Scores");
        highScoresText.setCharacterSize(30);
        highScoresText.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - highScoresText.getLocalBounds().width / 2.0f,
            300.0f
        );

        exitText.setFont(font);
        exitText.setString("\n\n\nExit");
        exitText.setCharacterSize(30);
        exitText.setPosition(
            static_cast<float>(WINDOW_WIDTH) / 2.0f - exitText.getLocalBounds().width / 2.0f,
            350.0f
        );

        menuItems = { snakeText, flappyText, instructionsText, highScoresText, exitText };
    }
    // Function to handel user input
    void handleInput() {
        if (keyPressClock.getElapsedTime().asSeconds() < keyPressDelay) {
            return;
        }

        bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
        bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);

        if ((upPressed || downPressed) && !keyProcessed) {
            if (upPressed) {
                selectedItem = (selectedItem - 1 + static_cast<int>(menuItems.size())) % static_cast<int>(menuItems.size());
            }
            else if (downPressed) {
                selectedItem = (selectedItem + 1) % static_cast<int>(menuItems.size());
            }

            updateSelection();
            keyPressClock.restart();
            keyProcessed = true;
        }
        else if (!upPressed && !downPressed) {
            keyProcessed = false;
        }
    }
    
    void updateSelection() {
        for (size_t i = 0; i < menuItems.size(); ++i) {
            if (i == static_cast<size_t>(selectedItem)) {
                menuItems[i].setFillColor(sf::Color::Red);
            }
            else {
                menuItems[i].setFillColor(sf::Color::White);
            }
        }
    }

    int getSelectedItem() const {
        return selectedItem;
    }
    // Function to render background and Title
    void draw() {
        window.draw(background);
        window.draw(title);
        for (const auto& item : menuItems) {
            window.draw(item);
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Arcade Simulator");
    window.setFramerateLimit(60);

    std::unique_ptr<MainMenu> menu = std::make_unique<MainMenu>(window);
    std::unique_ptr<Game> currentGame;
    std::unique_ptr<HighScoresScreen> highScoresScreen;
    std::unique_ptr<InstructionsScreen> instructionsScreen;

    int gameState = 0; 
    bool showHighScores = false;
    bool showInstructions = false;

    sf::Music bgMusic;
    if (!bgMusic.openFromFile(BG_MUSIC)) {
        std::cerr << "Failed to load background music" << std::endl;
    }
    bgMusic.setLoop(true);
    bgMusic.play(); // Function call for background music

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (gameState != 0 && currentGame && currentGame->isGameOver()) {
                    if (event.key.code == sf::Keyboard::R) {
                        currentGame->reset();
                    }
                    else if (event.key.code == sf::Keyboard::M) {
                        currentGame.reset();
                        gameState = 0;
                        menu = std::make_unique<MainMenu>(window);
                    }
                }
            }
        }

   
        if (currentGame) {
            bgMusic.setVolume(currentGame->isMusicMuted() ? 0 : 100);
        }
        else {
            bgMusic.setVolume(100); 
        }
        // function to clear screen 
        window.clear();


        if (showHighScores) {
            if (!highScoresScreen) {
                highScoresScreen = std::make_unique<HighScoresScreen>(window);
            }
            highScoresScreen->handleInput(showHighScores);
            highScoresScreen->draw();
        }
        else if (showInstructions) {
            if (!instructionsScreen) {
                instructionsScreen = std::make_unique<InstructionsScreen>(window);
            }
            instructionsScreen->handleInput(showInstructions);
            instructionsScreen->draw();
        }
        else if (gameState == 0) {
            menu->handleInput();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                int selected = menu->getSelectedItem();
                if (selected == 0) {
                    currentGame = std::make_unique<SnakeGame>(window);
                    gameState = 1;
                }
                else if (selected == 1) {
                    currentGame = std::make_unique<FlappyBirdGame>(window);
                    gameState = 2;
                }
                else if (selected == 2) {
                    showInstructions = true;
                }
                else if (selected == 3) {
                    showHighScores = true;
                }
                else if (selected == 4) {
                    window.close();
                }
            }

            menu->draw();
        }
        else { 
            if (currentGame) {
                currentGame->handleInput();
                currentGame->update();
                currentGame->draw();
            }
        }

        window.display();
    }

    return 0;
}
