#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <ctime>
#include <fstream>
#include <iostream>
using namespace std;

// Constants and global variables
const int STATE_MENU = 0;
const int STATE_PLAYING = 1;
const int STATE_GAMEOVER = 2;
const int STATE_HELP = 3;
const int ROWS = 8;
const int COLS = 8;
const int TARGET_SCORE = 250;
int grid[ROWS][COLS];
int special[ROWS][COLS];
int movesLeft = 20;
int score = 0;
int lastCrushScore = 0;
bool scoringAllowed = true;  // to make sure only player swap increase score and not cascades
int moveScore = 0;
bool win = false;            // to play win/lose music
bool matched[ROWS][COLS];
int selectedRow = -1;
int selectedCol = -1;
float visualY[ROWS][COLS];   // to create drop animation
bool isInitializing = false; // to make sure no sound during initialization
bool musicEnabled = true;
bool sfxEnabled = true;

// to add audio functionalities(sound effects and background music)
sf::Music music;
int currentMusic = -1; // 0 = menu, 1 = gameplay, 2 = victory / lose
sf::SoundBuffer crushBuffer, createBombBuffer, createStripeBuffer;
sf::SoundBuffer lineBlastBuffer, bombBlastBuffer;
sf::Sound crushSound, createBombSound, createStripeSound;
sf::Sound lineBlastSound, bombBlastSound;

// Function prototypes
void initializeGrid();
void displayGrid(sf::RenderWindow& window, sf::Texture candyTextures[6],
    sf::Texture special4Textures[6],
    sf::Texture special5Textures[6]);
void swapTiles(sf::Event& event, int& currentState, int& currentMusic);
void checkMatches();
void findHorizontalMatches();
void findVerticalMatches();
void crushAndDrop();
bool hasAnyMatch();
void saveGame();
void loadGame();
void applySpecialEffects();

// Main function
int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Candy Crush PF Project");

    // create textures to convert to sprites
    sf::Texture menuTexture;         // menu screen
    sf::Texture gameTexture;         // game background
    sf::Texture candyTextures[6];    // normal candies
    sf::Texture special4Textures[6]; // 3x3 bombs
    sf::Texture special5Textures[6]; // row+col bombs

    if (!menuTexture.loadFromFile("menu.png"))
    {
        cout << "Error loading menu.png!\n";
    }
    if (!gameTexture.loadFromFile("background.png"))
    {
        cout << "Error loading background.png!\n";
    }

    for (int i = 0; i < 6; i++)
    {
        string filename = "candy" + to_string(i) + ".png";
        if (!candyTextures[i].loadFromFile(filename))
        {
            cout << "Error loading " << filename << "!\n";
        }
    }

    for (int i = 0; i < 6; i++)
    {
        string s4 = "special4_candy" + to_string(i) + ".png";
        string s5 = "special5_candy" + to_string(i) + ".png";
        if (!special4Textures[i].loadFromFile(s4))
        {
            cout << "Error loading " << s4 << "!\n";
        }
        if (!special5Textures[i].loadFromFile(s5))
        {
            cout << "Error loading " << s5 << "!\n";
        }
    }

    // convert textures to sprites
    sf::Sprite menuSprite(menuTexture);
    sf::Sprite gameSprite(gameTexture);

    // Get texture sizes
    sf::Vector2u menuSize = menuTexture.getSize();
    sf::Vector2u gameSize = gameTexture.getSize();

    // Target window size
    float winW = 800.0f;
    float winH = 600.0f;

    // Scale factors (stretch to fill)
    menuSprite.setScale(winW / menuSize.x, winH / menuSize.y);
    gameSprite.setScale(winW / gameSize.x, winH / gameSize.y);

    // load font from arial.ttf
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        cout << "Error loading arial.ttf!\n";
    }

    // create text for menu hints
    sf::Text menuText;
    menuText.setFont(font);
    menuText.setCharacterSize(28);
    menuText.setFillColor(sf::Color::White);
    menuText.setPosition(190, 430);
    menuText.setOutlineColor(sf::Color::Black);
    menuText.setOutlineThickness(3);

    // create text for music/sound toggle
    sf::Text soundText;
    soundText.setFont(font);
    soundText.setCharacterSize(24);
    soundText.setFillColor(sf::Color::White);
    soundText.setPosition(580, 20);
    soundText.setOutlineColor(sf::Color::Black);
    soundText.setOutlineThickness(2);

    // create text from font for scores and moves
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 20);
    scoreText.setOutlineColor(sf::Color::Black);
    scoreText.setOutlineThickness(2);

    sf::Text movesText; // for moves
    movesText.setFont(font);
    movesText.setCharacterSize(24);
    movesText.setFillColor(sf::Color::White);
    movesText.setPosition(10, 60);
    movesText.setOutlineColor(sf::Color::Black);
    movesText.setOutlineThickness(2);

    sf::Text targetText; // for target score
    targetText.setFont(font);
    targetText.setCharacterSize(24);
    targetText.setFillColor(sf::Color::White);
    targetText.setPosition(10, 100);
    targetText.setOutlineColor(sf::Color::Black);
    targetText.setOutlineThickness(2);

    // create text for end screen
    sf::Text winText;
    winText.setFont(font);
    winText.setCharacterSize(32);
    winText.setFillColor(sf::Color(0, 150, 0));
    winText.setPosition(185, 230);
    winText.setOutlineColor(sf::Color::Black);
    winText.setOutlineThickness(1);

    sf::Text loseText;
    loseText.setFont(font);
    loseText.setCharacterSize(32);
    loseText.setFillColor(sf::Color::Red);
    loseText.setPosition(190, 230);
    loseText.setOutlineColor(sf::Color::Black);
    loseText.setOutlineThickness(1);

    // create text to hint for saving game
    sf::Text saveText;
    saveText.setFont(font);
    saveText.setCharacterSize(24);
    saveText.setFillColor(sf::Color::White);
    saveText.setPosition(10, 140);
    saveText.setOutlineColor(sf::Color::Black);
    saveText.setOutlineThickness(2);

    // create text for help screen
    sf::Text helpText;
    helpText.setFont(font);
    helpText.setCharacterSize(24);
    helpText.setFillColor(sf::Color::White);
    helpText.setPosition(50, 80);
    helpText.setOutlineColor(sf::Color::Black);
    helpText.setOutlineThickness(2);

    // load audio files from buffer and create sounds
    if (!crushBuffer.loadFromFile("square_removed.mp3"))
        cout << "Error loading square_removed.mp3!\n";
    if (!createBombBuffer.loadFromFile("wrapped_candy_created.mp3"))
        cout << "Error loading wrapped_candy_created.mp3!\n";
    if (!createStripeBuffer.loadFromFile("Striped_candy_created.mp3"))
        cout << "Error loading Striped_candy_created.mp3!\n";
    if (!lineBlastBuffer.loadFromFile("line_blast.mp3"))
        cout << "Error loading Line_blast.mp3!\n";
    if (!bombBlastBuffer.loadFromFile("Bomb_sound.mp3"))
        cout << "Error loading Bomb_sound.mp3!\n";

    crushSound.setBuffer(crushBuffer);
    createBombSound.setBuffer(createBombBuffer);
    createStripeSound.setBuffer(createStripeBuffer);
    lineBlastSound.setBuffer(lineBlastBuffer);
    bombBlastSound.setBuffer(bombBlastBuffer);

    initializeGrid(); // set the board

    int currentState = STATE_MENU;
    if (currentMusic != 0 && musicEnabled)
    {
        if (!music.openFromFile("candy_crush_menu.mp3"))
        {
            cout << "Error loading candy_crush_menu.mp3!\n";
        }
        music.setLoop(true);
        music.play();
        currentMusic = 0;
    }
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Enter)
                {
                    if (currentState == STATE_MENU)
                    {
                        initializeGrid(); // start the game
                        currentState = STATE_PLAYING;
                        if (currentMusic != 1 && musicEnabled)
                        {
                            if (!music.openFromFile("candy_crush_gameplay.mp3"))
                            {
                                cout << "Error loading candy_crush_gameplay.mp3!\n";
                            }
                            music.setLoop(true);
                            music.play();
                            currentMusic = 1;
                        }
                    }
                    else if (currentState == STATE_GAMEOVER)
                    {
                        initializeGrid(); // set the board again for next game
                        currentState = STATE_PLAYING;
                        if (currentMusic != 1 && musicEnabled)
                        {
                            if (!music.openFromFile("candy_crush_gameplay.mp3"))
                            {
                                cout << "Error loading candy_crush_gameplay.mp3!\n";
                            }
                            music.setLoop(true);
                            music.play();
                            currentMusic = 1;
                        }
                    }
                }

                if (currentState == STATE_PLAYING && event.key.code == sf::Keyboard::S)
                { // to save game and return to menu
                    saveGame();
                    currentState = STATE_MENU; // return to menu after saving
                    if (currentMusic != 0 && musicEnabled)
                    {
                        if (!music.openFromFile("candy_crush_menu.mp3"))
                        {
                            cout << "Error loading candy_crush_menu.mp3!\n";
                        }
                        music.setLoop(true);
                        music.play();
                        currentMusic = 0;
                    }
                }

                if (currentState == STATE_MENU && event.key.code == sf::Keyboard::L)
                {
                    loadGame();                    // to load saved game
                    currentState = STATE_PLAYING;  // resume saved game
                    if (currentMusic != 1 && musicEnabled)
                    {
                        if (!music.openFromFile("candy_crush_gameplay.mp3"))
                        {
                            cout << "Error loading candy_crush_gameplay.mp3!\n";
                        }
                        music.setLoop(true);
                        music.play();
                        currentMusic = 1;
                    }
                }

                if (currentState == STATE_MENU && event.key.code == sf::Keyboard::H)
                { // to go to 'How to Play' screen
                    currentState = STATE_HELP;
                }
                if (currentState == STATE_HELP && event.key.code == sf::Keyboard::Escape)
                {
                    currentState = STATE_MENU;
                }

                if (event.key.code == sf::Keyboard::M)
                { // to mute and unmute music
                    musicEnabled = !musicEnabled;

                    if (!musicEnabled)
                    {
                        music.pause(); // pause music
                    }
                    else
                    {
                        // restart correct music for currentState
                        if (currentState == STATE_MENU && currentMusic != 0)
                        {
                            if (!music.openFromFile("candy_crush_menu.mp3"))
                            {
                                cout << "Error loading candy_crush_menu.mp3!\n";
                            }
                            music.setLoop(true);
                            music.play();
                            currentMusic = 0;
                        }
                        else if (currentState == STATE_PLAYING && currentMusic != 1)
                        {
                            if (!music.openFromFile("candy_crush_gameplay.mp3"))
                            {
                                cout << "Error loading candy_crush_gameplay.mp3!\n";
                            }
                            music.setLoop(true);
                            music.play();
                            currentMusic = 1;
                        }
                        else if (currentState == STATE_GAMEOVER && currentMusic != 2)
                        { // play game over music if enabled
                            if (win)
                            {
                                if (!music.openFromFile("candy_crush_victory.mp3"))
                                {
                                    cout << "Error loading candy_crush_victory.mp3!\n";
                                }
                                music.setLoop(true);
                                music.play();
                                currentMusic = 2;
                            }
                            else
                            {
                                if (!music.openFromFile("candy_crush_lose.mp3"))
                                {
                                    cout << "Error loading candy_crush_lose.mp3!\n";
                                }
                                music.setLoop(true);
                                music.play();
                                currentMusic = 2;
                            }
                        }
                        else
                        {
                            // if currentMusic already matches, just resume
                            music.play();
                        }
                    }
                }

                if (event.key.code == sf::Keyboard::F)
                { // to mute and unmute sound
                    sfxEnabled = !sfxEnabled;
                }
                if (event.key.code == sf::Keyboard::Escape && (currentState == STATE_PLAYING || currentState == STATE_GAMEOVER))
                {
                    currentState = STATE_MENU; // return to menu
                    if (currentMusic != 0 && musicEnabled)
                    {
                        if (!music.openFromFile("candy_crush_menu.mp3"))
                        {
                            cout << "Error loading candy_crush_menu.mp3!\n";
                        }
                        music.setLoop(true);
                        music.play();
                        currentMusic = 0;
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && currentState == STATE_PLAYING)
            {
                swapTiles(event, currentState, currentMusic); // swap and check matches
            }
        }

        if (currentState == STATE_MENU)
        {
            window.clear();
            window.draw(menuSprite); // draw menu
            menuText.setString("Press 'Enter' to Start Game\nPress 'L' to load Saved Game\nPress 'H' for Help\nPress 'Esc' to Quit and return to Menu");
            soundText.setString("M: ON/OFF Music\nF: ON/OFF Sound");
            window.draw(menuText);
            window.draw(soundText);
        }
        else if (currentState == STATE_HELP)
        {
            window.clear();
            window.draw(menuSprite);

            helpText.setString(
                "How to Play:\n\n"
                "- Swap two adjacent candies with the mouse.\n"
                "- Make a line of 3+ same candies to clear them.\n"
                "- 4 in a row creates a 3x3 bomb.\n"
                "- 5 in a row creates a row+column bomb.\n"
                "- Special candies activate when part of a match.\n"
                "- Reach the target score within the moves.\n\n"
                "Controls:\n"
                "- Enter: Start / Play Again\n"
                "- S: Save game   L: Load game\n"
                "- M: Toggle music   F: Toggle sound\n"
                "- Esc: Back to Menu");

            window.draw(helpText);
        }
        else if (currentState == STATE_PLAYING)
        {
            window.clear();
            window.draw(gameSprite); // draw game background

            // write scores and moves on the window by converting to strings
            scoreText.setString("Score: " + to_string(score));
            movesText.setString("Moves: " + to_string(movesLeft));
            targetText.setString("Target: " + to_string(TARGET_SCORE));
            saveText.setString("S: Save\n");

            window.draw(scoreText);
            window.draw(movesText);
            window.draw(targetText);
            window.draw(saveText);

            displayGrid(window, candyTextures, special4Textures, special5Textures);
            for (int r = 0; r < ROWS; r++)
            {
                for (int c = 0; c < COLS; c++)
                {
                    float targetY = r * (60 + 10) + 40;  // set the target position for falling candy

                    if (visualY[r][c] < targetY)
                    {
                        visualY[r][c] += 2.0f;   // fall speed(changes position by 2.0 every frame of the falling candy to create drop animation)
                        if (visualY[r][c] > targetY)
                            visualY[r][c] = targetY;
                    }
                }
            }
        }
        else if (currentState == STATE_GAMEOVER)
        {
            window.clear();
            window.draw(gameSprite); // end screen

            string msg;
            if (win == true)
            {
                msg = "You Win!\nPress 'Enter' to play again\nor 'Esc' to return to menu";
                winText.setString(msg);
                window.draw(winText);
            }
            else
            {
                msg = "You Lose!\nPress 'Enter' to retry\nor 'Esc' to return to menu";
                loseText.setString(msg);
                window.draw(loseText);
            }
        }
        window.display();
    }

    return 0;
}

void initializeGrid()
{
    isInitializing = true;  // to mute sound effects during initialization
    scoringAllowed = false; // no scoring during setup
    moveScore = 0;
    lastCrushScore = 0;

    srand(time(0));         // to ensure random candies every time 
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            grid[r][c] = rand() % 6; // set the grid
        }
    }
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            special[r][c] = 0; // for special candies(0 for normal candies)
        }
    }
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            visualY[r][c] = r * (60 + 10) + 40;  /*60 = tileSize, 10 = spacing, 40 = yOffset . We initially set each candy at its correct(vertical) position.
                                                  This position also acts as a target for the incoming falling candies*/
        }
    }
    while (true)  // Loop to handle matches during initialization before the start of game
    {
        for (int r = 0; r < ROWS; r++)
        {
            for (int c = 0; c < COLS; c++)
            {
                matched[r][c] = false;
            }
        }
        checkMatches();
        if (!hasAnyMatch())
        { // if no matches found, break
            break;
        }
        // if there are matches, crush and drop
        crushAndDrop();
    }
    score = 0;
    movesLeft = 20;
    win = false;
    isInitializing = false;
}

void displayGrid(sf::RenderWindow& window, sf::Texture candyTextures[6],
    sf::Texture special4Textures[6],
    sf::Texture special5Textures[6])
{
    int tileSize = 60;

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {

            int type = grid[r][c];
            if (type < 0 || type > 5)
                continue;

            // choose correct texture
            sf::Texture& tex =
                (special[r][c] == 1) ? special4Textures[type] : // 3x3 bomb
                (special[r][c] == 2) ? special5Textures[type]
                : // row+column bomb
                candyTextures[type];   // normal candies

            sf::Sprite candySprite;
            candySprite.setTexture(tex);

            // scale to tile size
            sf::Vector2u texSize = tex.getSize();
            float scaleX = tileSize / (float)(texSize.x); // float division to set scale
            float scaleY = tileSize / (float)(texSize.y);
            candySprite.setScale(scaleX, scaleY);

            // set position of individual candies(sprites)
            float x = c * (tileSize + 10) + 150;
            float y = visualY[r][c];
            candySprite.setPosition(x, y);

            window.draw(candySprite);

            if (r == selectedRow && c == selectedCol)
            {
                sf::RectangleShape outline(sf::Vector2f(tileSize, tileSize));
                outline.setPosition(x, y);
                outline.setFillColor(sf::Color::Transparent);
                outline.setOutlineThickness(3);
                outline.setOutlineColor(sf::Color::White);
                window.draw(outline);
            }
        }
    }
}

void swapTiles(sf::Event& event, int& currentState, int& currentMusic)
{
    int mouseX = event.mouseButton.x;
    int mouseY = event.mouseButton.y;

    int xOffset = 150;
    int yOffset = 40;
    int tileSize = 60;
    int tileSpacing = tileSize + 10;

    int col = (mouseX - xOffset) / tileSpacing;
    int row = (mouseY - yOffset) / tileSpacing;

    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
    {
        if (selectedRow == -1 && selectedCol == -1)   // if no candy is selected before
        {
            selectedRow = row;
            selectedCol = col;
        }
        else   // if one candy is selected before
        {
            int dr = row - selectedRow;
            int dc = col - selectedCol;

            if ((abs(dr) == 1 && dc == 0) || (abs(dc) == 1 && dr == 0))   // if other selection is one tile up, down, right, left
            {
                int r1 = selectedRow, c1 = selectedCol;
                int r2 = row, c2 = col;

                // swap colors
                int temp = grid[r1][c1];
                grid[r1][c1] = grid[r2][c2];
                grid[r2][c2] = temp;

                // swap specials
                temp = special[r1][c1];
                special[r1][c1] = special[r2][c2];
                special[r2][c2] = temp;

                for (int r = 0; r < ROWS; r++)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        matched[r][c] = false;
                    }
                }
                // check for matches after a swap
                checkMatches();

                // if no matches after swap, revert the swap
                if (!hasAnyMatch())
                {
                    // revert colors
                    temp = grid[r1][c1];
                    grid[r1][c1] = grid[r2][c2];
                    grid[r2][c2] = temp;

                    // revert specials
                    temp = special[r1][c1];
                    special[r1][c1] = special[r2][c2];
                    special[r2][c2] = temp;
                }
                else
                {
                    movesLeft--;
                    scoringAllowed = true;
                    moveScore = 0; // reset for this move

                    while (true)   // to take care of swap match and automatic matches(cascades)
                    {
                        for (int r = 0; r < ROWS; r++)
                        {
                            for (int c = 0; c < COLS; c++)
                            {
                                matched[r][c] = false;
                            }
                        }
                        lastCrushScore = 0;
                        checkMatches();
                        if (!hasAnyMatch())
                        {
                            break;
                        }
                        crushAndDrop(); // add to moveScore only when scoringAllowed == true
                        scoringAllowed = false;
                    }
                    score += moveScore; // after all cascades, update score only once

                    // after all matches
                    if (score >= TARGET_SCORE)
                    {
                        win = true;
                        currentState = STATE_GAMEOVER;

                        if (musicEnabled && currentMusic != 2)
                        {
                            if (!music.openFromFile("candy_crush_victory.mp3"))
                            {
                                cout << "Error loading candy_crush_victory.mp3!\n";
                            }
                            music.setLoop(true);
                            music.play();
                            currentMusic = 2;
                        }
                    }
                    else if (movesLeft == 0)
                    {
                        win = false;
                        currentState = STATE_GAMEOVER;

                        if (musicEnabled && currentMusic != 2)
                        {
                            if (!music.openFromFile("candy_crush_lose.mp3"))
                            {
                                cout << "Error loading candy_crush_lose.mp3!\n";
                            }
                            music.setLoop(true);
                            music.play();
                            currentMusic = 2;
                        }
                    }
                }
                selectedRow = -1;
                selectedCol = -1;
            }
            else
            {
                selectedRow = row;
                selectedCol = col;
            }
        }
    }
}

void checkMatches()
{
    findHorizontalMatches();
    findVerticalMatches();
    applySpecialEffects();
}

void findHorizontalMatches()
{
    for (int r = 0; r < ROWS; r++)
    {
        int runLength = 1;
        for (int c = 1; c < COLS; c++)
        {
            if (grid[r][c] == grid[r][c - 1])
            {
                runLength++;
            }
            else
            {
                if (runLength >= 3)
                {
                    int start = c - runLength; // starting index of run >= 3
                    int end = c - 1;           // ending index of the run
                    int specialCol = -1;       // hold column for special candy so we don't mark it for removal

                    if (runLength == 3)
                    {
                        lastCrushScore += 10;
                    }
                    else if (runLength == 4)
                    {
                        lastCrushScore += 20;
                        specialCol = start + 1;
                        if (special[r][specialCol] == 0)
                        {
                            special[r][specialCol] = 1; // 3x3 bomb
                            if (!isInitializing && sfxEnabled)
                                createBombSound.play(); // play 4-match creation sound
                        }
                    }
                    else
                    { // runLength >= 5
                        lastCrushScore += 30;
                        specialCol = start + 2;
                        if (special[r][specialCol] == 0)
                        {
                            special[r][specialCol] = 2; // row+col bomb
                            if (!isInitializing && sfxEnabled)
                                createStripeSound.play(); // play 5-match creation sound
                        }
                    }
                    for (int k = start; k <= end; k++)
                    {
                        if (k != specialCol)
                            matched[r][k] = true;
                    }
                }
                runLength = 1; // reset for next run
            }
        }

        // handle run ending at last column(to not go out of bounds)
        if (runLength >= 3)
        {
            int start = COLS - runLength;
            int end = COLS - 1;
            int specialCol = -1;

            if (runLength == 3)
            {
                lastCrushScore += 10;
            }
            else if (runLength == 4)
            {
                lastCrushScore += 20;
                specialCol = start + 1;
                if (special[r][specialCol] == 0)
                {
                    special[r][specialCol] = 1;
                    if (!isInitializing && sfxEnabled)
                        createBombSound.play();
                }
            }
            else
            { // runLength >= 5
                lastCrushScore += 30;
                specialCol = start + 2;
                if (special[r][specialCol] == 0)
                {
                    special[r][specialCol] = 2;
                    if (!isInitializing && sfxEnabled)
                        createStripeSound.play();
                }
            }

            for (int k = start; k <= end; k++)
            {
                if (k != specialCol)
                    matched[r][k] = true;
            }
        }
    }
}

void findVerticalMatches()
{
    for (int c = 0; c < COLS; c++)
    {
        int runLength = 1;
        for (int r = 1; r <= ROWS; r++)
        {
            if (grid[r][c] == grid[r - 1][c])
            {
                runLength++;
            }
            else
            {
                if (runLength >= 3)
                {
                    int start = r - runLength;
                    int end = r - 1;
                    int specialRow = -1;

                    if (runLength == 3)
                    {
                        lastCrushScore += 10;
                    }
                    else if (runLength == 4)
                    {
                        lastCrushScore += 20;
                        specialRow = start + 1;
                        if (special[specialRow][c] == 0)
                        {
                            special[specialRow][c] = 1;
                            if (!isInitializing && sfxEnabled)
                                createBombSound.play();
                        }
                    }
                    else
                    { // runLength >= 5
                        lastCrushScore += 30;
                        specialRow = start + 2;
                        if (special[specialRow][c] == 0)
                        {
                            special[specialRow][c] = 2;
                            if (!isInitializing && sfxEnabled)
                                createStripeSound.play();
                        }
                    }

                    for (int k = start; k <= end; k++)
                    {
                        if (k != specialRow)
                            matched[k][c] = true;
                    }
                }
                runLength = 1;
            }
        }

        // handle run ending at last row
        if (runLength >= 3)
        {
            int start = ROWS - runLength;
            int end = ROWS - 1;
            int specialRow = -1;

            if (runLength == 3)
            {
                lastCrushScore += 10;
            }
            else if (runLength == 4)
            {
                lastCrushScore += 20;
                specialRow = start + 1;
                if (special[specialRow][c] == 0)
                {
                    special[specialRow][c] = 1;
                    if (!isInitializing && sfxEnabled)
                        createBombSound.play();
                }
            }
            else
            { // runLength >= 5
                lastCrushScore += 30;
                specialRow = start + 2;
                if (special[specialRow][c] == 0)
                {
                    special[specialRow][c] = 2;
                    if (!isInitializing && sfxEnabled)
                        createStripeSound.play();
                }
            }

            for (int k = start; k <= end; k++)
            {
                if (k != specialRow)
                    matched[k][c] = true;
            }
        }
    }
}

void crushAndDrop()
{
    if (scoringAllowed)
    {
        moveScore += lastCrushScore; // update moveScore during first cascade and reset lastCrushScore for next crush
    }
    lastCrushScore = 0;

    bool anyCrushed = false;  // to create sound after every crush./
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (matched[r][c] == true)
            {
                grid[r][c] = -1; // create empty spaces
                anyCrushed = true;
            }
        }
    }
    if (anyCrushed && !isInitializing && sfxEnabled)
    {
        crushSound.play(); // square removed sound
    }

    int tileSize = 60;
    int spacing = tileSize + 10;

    for (int c = 0; c < COLS; c++)
    {
        int writeRow = ROWS - 1;
        // move existing candies down(empty spaces to the top)
        for (int r = ROWS - 1; r >= 0; r--)
        {
            if (grid[r][c] != -1)
            {
                grid[writeRow][c] = grid[r][c];
                special[writeRow][c] = special[r][c]; // to move special candies
                visualY[writeRow][c] = visualY[r][c]; // carry visual position along y-axis
                writeRow--;
            }
        }
        // create new candies above the board
        for (int r = writeRow; r >= 0; r--)
        {
            grid[r][c] = rand() % 6;
            special[r][c] = 0;        // new candies are normal
            visualY[r][c] = -spacing; // start above and fall down
        }
    }
}

bool hasAnyMatch()
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (matched[r][c] == true)
                return true;
        }
    }
    return false;
}

void saveGame()
{
    ofstream fout("save.txt");
    if (!fout)
    {
        cout << "Could not open save.txt for writing!\n";
        return;
    }

    fout << score << ' ' << movesLeft << '\n';
    fout << '\n';
    // save grid
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            fout << grid[r][c] << ' ';
        }
        fout << '\n';
    }
    fout << '\n';
    // save special
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            fout << special[r][c] << ' ';
        }
        fout << '\n';
    }
    fout.close();
}

void loadGame()
{
    ifstream fin("save.txt");
    if (!fin)
    {
        cout << "Error loading save.txt!\n";
    }

    fin >> score >> movesLeft;
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            fin >> grid[r][c];
        }
    }
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            fin >> special[r][c];
        }
    }
    fin.close();

    // reset selection and matches
    selectedRow = -1, selectedCol = -1;
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            matched[r][c] = false;
        }
    }
    win = false;
}

void applySpecialEffects()
{
    // create a copy for matching of special candies to not expand while iterating
    bool specialMatched[ROWS][COLS];
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            specialMatched[r][c] = false;
        }
    }
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {

            // if 3x3 bomb is matched
            if (matched[r][c] == true && special[r][c] == 1)
            {
                if (!isInitializing && sfxEnabled)
                    bombBlastSound.play(); // 3x3 bomb sound

                // mark a 3x3 region around(and including) (r,c)
                int dr[9] = { 0, 1, -1, 0, 0, 1, -1, 1, -1 };
                int dc[9] = { 0, 0, 0, 1, -1, 1, -1, -1, 1 };

                for (int i = 0; i < 9; i++)
                { // mark in each direction(up, down , right, left etc.)
                    int otherR = r + dr[i];
                    int otherC = c + dc[i];

                    if (otherR >= 0 && otherR < ROWS && otherC >= 0 && otherC < COLS)
                    {
                        specialMatched[otherR][otherC] = true;
                    }
                }
            }
            // if row+column bomb is matched
            else if (matched[r][c] == true && special[r][c] == 2)
            {
                if (!isInitializing && sfxEnabled)
                    lineBlastSound.play(); // striped candy blast sound

                for (int specialRow = 0; specialRow < ROWS; specialRow++)
                {
                    specialMatched[specialRow][c] = true; // mark row of special candy
                }
                for (int specialCol = 0; specialCol < COLS; specialCol++)
                {
                    specialMatched[r][specialCol] = true; // mark column of special candy
                }
            }
        }
    }
    // merge specialMatched into matched
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (specialMatched[r][c] == true)
            {
                matched[r][c] = true;
            }
        }
    }
}
