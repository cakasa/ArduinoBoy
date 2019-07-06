#include <ArduinoSTL.h>
#include <LedControl.h>
#include <vector>

#define DINPin 11  // DataIn pin for the MAX7219 module
#define CSPin 12   // Load pin for the MAX7219 module
#define CLKPin 13  // Clock pin for the MAX7219 module

// The following are all used structs within the project
// Location is used to map certain game elements to the LED matrix
struct Location
{
  byte x; // [0-15] the row index of the element
  byte y; // [0-7] the column index of the element
};

// This struct is specifically made for the tetris game and is used to
// track where the tetromino is.
struct Tetromino
{
  // All of the blocks represent one location element, so that the tetromino
  // can be displayed properly
  Location block1;
  Location block2;
  Location block3;
  Location block4;
  Location center; // The center is used to determine the rotation point of the tetromino
};

// This struct is used to locate both the A.I. paddle and the player controlled one in the Pong game
struct Paddle
{
  // It consists of two Location objects as well, they determine where the paddle is located
  Location block1;
  Location block2;
};

// Direction is a struct, used in the pong game and its purpose is to determine what
// direction the ball will move in.
struct Direction
{
  short x; // [-1...1] the row direction, -1 is upwards, 1 is downwards
  short y; // [-1...1] the column direction, -1 is left, 1 is right
};

// All of the following are sprites, which are used to display generic information
// on the LED displays when an event is fired. All of them are in binary, represents
// whether the light is on or off.

// The sprite used in the game over screen.
byte gameOverSprite[16] =
{
  B11110110,
  B10001001,
  B10111111,
  B10011001,
  B11111001,
  0,
  B10001111,
  B11011100,
  B10101111,
  B10001100,
  B10001111,
  0,
  B11110100,
  B10110100,
  B10110100,
  B11101010
};

// These sprites are only used in the Pong game.

// These are the digits from 0 to 9
byte numberSprites[10][8]
{
  {
    B00000000,  // 0
    B00111100,
    B00100100,
    B00100100,
    B00100100,
    B00100100,
    B00111100,
    B00000000
  },
  {
    B00000000, // 1
    B00001000,
    B00011000,
    B00001000,
    B00001000,
    B00001000,
    B00001000,
    B00000000
  },
  {
    B00000000, // 2
    B00111100,
    B00000100,
    B00111100,
    B00100000,
    B00100000,
    B00111100,
    B00000000
  },
  {
    B00000000, // 3
    B00111100,
    B00000100,
    B00111100,
    B00000100,
    B00000100,
    B00111100,
    B00000000
  },
  {
    B00000000, // 4
    B00100100,
    B00100100,
    B00100100,
    B00111100,
    B00000100,
    B00000100,
    B00000000
  },
  {
    B00000000, // 5
    B00111100,
    B00100000,
    B00111100,
    B00000100,
    B00000100,
    B00111100,
    00000000
  },
  {
    B00000000, // 6
    B00111100,
    B00100000,
    B00111100,
    B00100100,
    B00100100,
    B00111100,
    B00000000
  },
  {
    B00000000, // 7
    B00111100,
    B00100100,
    B00000100,
    B00000100,
    B00000100,
    B00000100,
    B00000000
  },
  {
    B00000000, // 8
    B00111100,
    B00100100,
    B00111100,
    B00100100,
    B00100100,
    B00111100,
    B00000000
  },
  {
    B00000000, //9
    B00111100,
    B00100100,
    B00111100,
    B00000100,
    B00000100,
    B00111100,
    B00000000
  }
};

// This sprite is used to tell the player that they has won the pong game.
byte winSprite[16]
{
  B01010100,
  B01010100,
  B01010100,
  B01010100,
  B00101000,
  B00111000,
  B00010000,
  B00010000,
  B00010000,
  B00010000,
  B00111000,
  B01000100,
  B01100100,
  B01010100,
  B01001100,
  B01000100
};

// This sprite is used to tell the player that they have lost the game
byte lossSprite[16]
{
  B00000000,
  B00000000,
  B10001110,
  B10001010,
  B10001010,
  B10001010,
  B11101110,
  B00000000,
  B00000000,
  B11101110,
  B10001000,
  B11101110,
  B00100010,
  B11101110,
  B00000000,
  B00000000
};

// The following variables are used by all of the games
LedControl matrixController(DINPin, CLKPin, CSPin, 2); // The controller for the LED matrix.
unsigned long timer; // timer, which uses the millis() function to determine when certain events should be fired.
bool isGameOver; // Variable, which determines whether the current game is over.
int playerScore; // Variable, used to store the player's current score.

// The following variables are used by the Tetris game
Tetromino tetromino; // The tetromino that the user currently has control of
bool tetrisMatrix[16][8]; // The matrix, used to determine which LEDs turn on
short fallingTetrominoDelay; // The delay used to determine how long it will be until the tetromino goes down one step.

// The following variables are used by the Snake game
char prevDirection; // The previous direction the snake followed.
char direction; // The current direction the snake is following
Location food; // The row and column coordinates of the food.
bool foodState; // Determines what state the blinking food is in (true -> LED on, false -> LED off)
short const scorePerFood = 10; // A constant used to determine how many points each eaten food gives.
short snakeMovementDelay; // The delay used to determine how long it will be until the snake follows the current direction again
std::vector<Location> snake; // A vector, containing all of the coordinates of the snake.

// Pong following variables are used by the Pong game

Paddle playerPaddle; // The paddle controlled by the player
Paddle aiPaddle; // The paddle controlled by the A.I.
Location ball; // The current location of the ball.
Direction ballDirection; // The direction in which the ball is going in
byte const pointsToWin = 10; // A constant, used to determine how many points each side needs to win the Pong game
short const aiMovementDelay = 220; // The constraint, which makes it impossible for the A.I. to win every game.
short ballDelay; // The delay used to determine how long it will be until the ball changes its position
unsigned long aiTimer; // An additional timer, whose purpose is to determine whether the A.I. can move its paddle yet.
int numberOfHits; // Number of times the ball was hit by either of the players. Each time it is hit, the velocity of the ball increases.
int aiScore; // An additional score counter, used for the A.I.

void setup()
{
  // Open the serial port for communication
  Serial.begin(9600);

  // Wake the LED boards up
  matrixController.shutdown(0, false);
  matrixController.shutdown(1, false);

  // Set the intensity of the display
  matrixController.setIntensity(0, 0);
  matrixController.setIntensity(1, 0);

  // Clear the display
  matrixController.clearDisplay(0);
  matrixController.clearDisplay(1);

  // Giving a random seed, so that the games spawn random items in random locations. Uses
  // the noise the A0 port
  randomSeed(analogRead(A0));
}

void loop()
{
  // Wait for a signal from the smartphone app. Once a game is selected
  // on the phone, the function responsible for the game is called.
  if (Serial.available() >= 0)
  {
    char requestedGame = Serial.read();
    if (requestedGame == 'T')
    {
      playTetris();
    }
    else if (requestedGame == 'S')
    {
      playSnake();
    }
    else if (requestedGame == 'P')
    {
      playPong();
    }
  }
}

// The following methods are used to play the SNAKE game.

// This method is the main one, responsible for running the SNAKE game.
void playSnake()
{
  setupSnake();
  
  // Start the timer
  timer = millis();
  while (!isGameOver)
  {
    // When information is sent through the mobile application, it is interpreted here.
    if (Serial.available() >= 0)
    {
      char input = Serial.read();
      // 'L' - Left; 'R' - Right; 'D' - Down; 'U' - Up
      if (input == 'L' || input == 'R' || input == 'D' || input == 'U')
      {
        char prevDirection = direction;
        char newDirection =  input;
        direction = changeSnakeDirection(prevDirection, newDirection);
      }
    }
    // When a certain time passes, move the snake in the selected direction
    if (millis() - timer >= snakeMovementDelay)
    {
      timer = millis();
      moveSnake();
    }
  }
  gameOver();
}

// This method is used to change the direction of the snake
char changeSnakeDirection(char prevDirection, char newDirection)
{
  switch (newDirection)
  {
    case 'R':
      if (prevDirection == 'L')
      {
        return 'L';
      }
      else return 'R';
      break;
    case 'L':
      if (prevDirection == 'R')
      {
        return 'R';
      }
      else return 'L';
      break;
    case 'D':
      if (prevDirection == 'U')
      {
        return 'U';
      }
      else return 'D';
      break;
    case 'U':
      if (prevDirection == 'D')
      {
        return 'D';
      }
      else return 'U';
      break;
  }
}

void setupSnake()
{
  isGameOver = false;
  playerScore = 0;
  foodState = true;
  snakeMovementDelay = 800;
  while (!snake.empty())
  {
    snake.pop_back();
  }
  Location startingBlock;
  startingBlock.x = 7;
  startingBlock.y = 4;
  snake.push_back(startingBlock);
  startingBlock.x = 7;
  startingBlock.y = 3;
  snake.push_back(startingBlock);
  startingBlock.x = 7;
  startingBlock.y = 2;
  snake.push_back(startingBlock);
  newFood();
  printSnakeGameBoard();
}

void newFood()
{
  bool needNewLocation = true;
  byte x, y;
  while (needNewLocation)
  {
    x = random(0, 15);
    y = random(0, 7);
    for (std::vector<Location>::iterator i = snake.begin(); i != snake.end(); i++)
    {
      if (i -> x == x && i -> y == y)
      {
        needNewLocation = true;
        break;
      }
      else needNewLocation = false;
    }

    if (!needNewLocation)
    {
      food.x = x;
      food.y = y;
    }
  }
  printSnakeGameBoard();
}

void printSnakeGameBoard()
{
  matrixController.clearDisplay(0);
  matrixController.clearDisplay(1);
  foodState = !foodState;
  matrixController.setLed(food.x / 8, food.x % 8, food.y, foodState);
  for (std::vector<Location>::iterator i = snake.begin(); i != snake.end(); i++)
  {
    matrixController.setLed(i -> x / 8, i -> x % 8, i -> y, true);
  }
}

void moveSnake()
{
  turnOffOldSnakeLocation();
  switch (direction)
  {
    case 'R':
      moveSnakeRight();
      break;
    case 'L':
      moveSnakeLeft();
      break;
    case 'D':
      moveSnakeDown();
      break;
    case 'U':
      moveSnakeUp();
      break;
  }
  printSnakeGameBoard();
  checkIfSnakeGameOver();
}

void turnOffOldSnakeLocation()
{
  for (std::vector<Location>::iterator i = snake.begin() + 1; i != snake.end(); i++)
  {
    matrixController.setLed(i -> x / 8, i -> x % 8, i -> y, false);
  }
}

void moveSnakeRight()
{
  Location backLocation = snake.back();
  snake.pop_back();
  Location frontLocation = snake.front();
  Location newFrontLocation;
  newFrontLocation.x = frontLocation.x;
  newFrontLocation.y = (frontLocation.y + 1) % 8;
  snake.insert(snake.begin(), newFrontLocation);
  collectFood(backLocation);
}

void moveSnakeLeft()
{
  Location backLocation = snake.back();
  snake.pop_back();
  Location frontLocation = snake.front();
  Location newFrontLocation;
  newFrontLocation.x = frontLocation.x;
  if (frontLocation.y == 0)
  {
    newFrontLocation.y = 7;
  }
  else newFrontLocation.y = frontLocation.y - 1;
  snake.insert(snake.begin(), newFrontLocation);
  collectFood(backLocation);
}

void moveSnakeDown()
{
  Location backLocation = snake.back();
  snake.pop_back();
  Location frontLocation = snake.front();
  Location newFrontLocation;
  newFrontLocation.x = (frontLocation.x + 1) % 16;
  newFrontLocation.y = frontLocation.y;
  snake.insert(snake.begin(), newFrontLocation);
  collectFood(backLocation);
}

void moveSnakeUp()
{
  Location backLocation = snake.back();
  snake.pop_back();
  Location frontLocation = snake.front();
  Location newFrontLocation;
  if (frontLocation.x == 0)
  {
    newFrontLocation.x = 15;
  }
  else newFrontLocation.x = frontLocation.x - 1;
  newFrontLocation.y = frontLocation.y;
  snake.insert(snake.begin(), newFrontLocation);
  collectFood(backLocation);
}
void collectFood(Location backLocation)
{
  if (snake[0].x == food.x && snake[0].y == food.y)
  {
    snake.push_back(backLocation);
    addSnakeScore();
    newFood();
  }
}

void addSnakeScore()
{
  playerScore += scorePerFood;
  String stringToPrint = "C" + String(playerScore);
  Serial.print(stringToPrint);
  setNewSnakeSpeed();
}

void setNewSnakeSpeed()
{
  if (snakeMovementDelay > 300)
  {
    snakeMovementDelay -= 10;
  }
}

void checkIfSnakeGameOver()
{
  Location front = snake.front();
  byte count = 0;
  for (std::vector<Location>::iterator i = snake.begin(); i != snake.end(); i++)
  {
    if (front.x == i -> x && front.y == i -> y)
    {
      count++;
    }
  }
  if (count > 1)
  {
    isGameOver = true;
  }
}


void playTetris()
{
  setupTetris();
  timer = millis();
  while (!isGameOver)
  {
    if (millis() - timer >= fallingTetrominoDelay)
    {
      timer = millis();
      shiftDown();
      String stringToPrint = "C" + String(playerScore);
      Serial.print(stringToPrint);
    }
    if (Serial.available() > 0)
    {
      char command = Serial.read();
      if (command == 'L')
      {
        shiftLeft();
      }
      else if (command == 'R')
      {
        shiftRight();
      }
      else if (command == 'A')
      {
        rotate(-1, 1);
      }
      else if (command == 'B')
      {
        rotate(1, -1);
      }
      else if (command == 'D')
      {
        fastForward();
      }
    }
  }
  gameOver();
}

void setupTetris()
{
  playerScore = 0;
  fallingTetrominoDelay = 1000;
  isGameOver = false;
  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      tetrisMatrix[i][j] = false;
    }
  }
  getNewTetromino();
}

void gameOver()
{
  for (int i = 0; i < 16; i++)
  {
    matrixController.setRow(i / 8, i % 8, gameOverSprite[i]);
  }
  while (Serial.available() == 0) {}
  Serial.write('V');
  matrixController.clearDisplay(0);
  matrixController.clearDisplay(1);
}

void fastForward()
{
  while (tetrisMatrix[tetromino.block1.x + 1][tetromino.block1.y] != 1 &&
         tetrisMatrix[tetromino.block2.x + 1][tetromino.block2.y] != 1 &&
         tetrisMatrix[tetromino.block3.x + 1][tetromino.block3.y] != 1 &&
         tetrisMatrix[tetromino.block4.x + 1][tetromino.block4.y] != 1 &&
         tetromino.block1.x != 15 && tetromino.block2.x != 15 &&
         tetromino.block3.x != 15 && tetromino.block4.x != 15)
  {
    shiftDown();
    delay(fallingTetrominoDelay / 7);
  }
}

void rotate(int rotationXIndex, int rotationYIndex)
{
  if (tetromino.center.x == 0 && tetromino.center.y == 0)
  {
    return;
  }
  turnOffOldLocation();

  Location tempPosition1, tempPosition2, tempPosition3, tempPosition4;
  Location relativePosition1, relativePosition2, relativePosition3, relativePosition4;
  Location newLocation1, newLocation2, newLocation3, newLocation4;
  tempPosition1.x = tetromino.block1.x - tetromino.center.x;
  tempPosition1.y = tetromino.block1.y - tetromino.center.y;
  relativePosition1.x = rotationXIndex * tempPosition1.y;
  relativePosition1.y = rotationYIndex * tempPosition1.x;
  newLocation1.x = tetromino.center.x + relativePosition1.x;
  newLocation1.y = tetromino.center.y + relativePosition1.y;

  tempPosition2.x = tetromino.block2.x - tetromino.center.x;
  tempPosition2.y = tetromino.block2.y - tetromino.center.y;
  relativePosition2.x = rotationXIndex * tempPosition2.y;
  relativePosition2.y = rotationYIndex * tempPosition2.x;
  newLocation2.x = tetromino.center.x + relativePosition2.x;
  newLocation2.y = tetromino.center.y + relativePosition2.y;

  tempPosition3.x = tetromino.block3.x - tetromino.center.x;
  tempPosition3.y = tetromino.block3.y - tetromino.center.y;
  relativePosition3.x = rotationXIndex * tempPosition3.y;
  relativePosition3.y = rotationYIndex * tempPosition3.x;
  newLocation3.x = tetromino.center.x + relativePosition3.x;
  newLocation3.y = tetromino.center.y + relativePosition3.y;

  tempPosition4.x = tetromino.block4.x - tetromino.center.x;
  tempPosition4.y = tetromino.block4.y - tetromino.center.y;
  relativePosition4.x = rotationXIndex * tempPosition4.y;
  relativePosition4.y = rotationYIndex * tempPosition4.x;
  newLocation4.x = tetromino.center.x + relativePosition4.x;
  newLocation4.y = tetromino.center.y + relativePosition4.y;

  if (newLocation1.x < 0 || newLocation1.x > 15 ||
      newLocation1.y < 0 || newLocation1.y > 7 ||
      newLocation2.x < 0 || newLocation2.x > 15 ||
      newLocation2.y < 0 || newLocation2.y > 7 ||
      newLocation3.x < 0 || newLocation3.x > 15 ||
      newLocation3.y < 0 || newLocation3.y > 7 ||
      newLocation4.x < 0 || newLocation4.x > 15 ||
      newLocation4.y < 0 || newLocation4.y > 7)
  {
    turnOnNewLocation();
    return;
  }
  else if (tetrisMatrix[newLocation1.x][newLocation1.y] == false &&
           tetrisMatrix[newLocation2.x][newLocation2.y] == false &&
           tetrisMatrix[newLocation3.x][newLocation3.y] == false &&
           tetrisMatrix[newLocation4.x][newLocation4.y] == false)
  {
    tetromino.block1 = newLocation1;
    tetromino.block2 = newLocation2;
    tetromino.block3 = newLocation3;
    tetromino.block4 = newLocation4;
  }

  turnOnNewLocation();
}

void shiftDown()
{
  turnOffOldLocation();
  if (tetromino.block1.x == 15 || tetromino.block2.x == 15 || tetromino.block3.x == 15 || tetromino.block4.x == 15)
  {
    tetrisMatrix[tetromino.block1.x][tetromino.block1.y] = 1;
    tetrisMatrix[tetromino.block2.x][tetromino.block2.y] = 1;
    tetrisMatrix[tetromino.block3.x][tetromino.block3.y] = 1;
    tetrisMatrix[tetromino.block4.x][tetromino.block4.y] = 1;
    addTetrisScore(5);
    getNewTetromino();
    return;
  }
  else if (tetrisMatrix[tetromino.block1.x + 1][tetromino.block1.y] == 1 ||
           tetrisMatrix[tetromino.block2.x + 1][tetromino.block2.y] == 1 ||
           tetrisMatrix[tetromino.block3.x + 1][tetromino.block3.y] == 1 ||
           tetrisMatrix[tetromino.block4.x + 1][tetromino.block4.y] == 1)
  {
    tetrisMatrix[tetromino.block1.x][tetromino.block1.y] = 1;
    tetrisMatrix[tetromino.block2.x][tetromino.block2.y] = 1;
    tetrisMatrix[tetromino.block3.x][tetromino.block3.y] = 1;
    tetrisMatrix[tetromino.block4.x][tetromino.block4.y] = 1;
    addTetrisScore(5);
    getNewTetromino();
    return;
  }
  tetromino.block1.x++;
  tetromino.block2.x++;
  tetromino.block3.x++;
  tetromino.block4.x++;
  tetromino.center.x++;
  turnOnNewLocation();
}

void shiftLeft()
{
  if (tetromino.block1.y == 0 || tetromino.block2.y == 0 || tetromino.block3.y == 0 || tetromino.block4.y == 0)
  {
    return;
  }
  else if (tetrisMatrix[tetromino.block1.x][tetromino.block1.y - 1] == 1 ||
           tetrisMatrix[tetromino.block2.x][tetromino.block2.y - 1] == 1 ||
           tetrisMatrix[tetromino.block3.x][tetromino.block3.y - 1] == 1 ||
           tetrisMatrix[tetromino.block4.x][tetromino.block4.y - 1] == 1)
  {
    return;
  }

  turnOffOldLocation();
  tetromino.block1.y--;
  tetromino.block2.y--;
  tetromino.block3.y--;
  tetromino.block4.y--;
  tetromino.center.y--;
  turnOnNewLocation();
}

void shiftRight()
{
  if (tetromino.block1.y == 7 || tetromino.block2.y == 7 || tetromino.block3.y == 7 || tetromino.block4.y == 7)
  {
    return;
  }
  else if (tetrisMatrix[tetromino.block1.x][tetromino.block1.y + 1] == 1 ||
           tetrisMatrix[tetromino.block2.x][tetromino.block2.y + 1] == 1 ||
           tetrisMatrix[tetromino.block3.x][tetromino.block3.y + 1] == 1 ||
           tetrisMatrix[tetromino.block4.x][tetromino.block4.y + 1] == 1)
  {
    return;
  }

  turnOffOldLocation();
  tetromino.block1.y++;
  tetromino.block2.y++;
  tetromino.block3.y++;
  tetromino.block4.y++;
  tetromino.center.y++;
  turnOnNewLocation();
}

void getNewTetromino()
{
  clearFullRows();
  int tetrominoIndex = random(0, 7);
  switch (tetrominoIndex)
  {
    case 0:                      //  - - o o o o - -
      tetromino.block1.x = 0;
      tetromino.block1.y = 2;
      tetromino.block2.x = 0;
      tetromino.block2.y = 3;
      tetromino.block3.x = 0;
      tetromino.block3.y = 4;
      tetromino.block4.x = 0;
      tetromino.block4.y = 5;
      tetromino.center.x = 0;
      tetromino.center.y = 3;
      break;
    case 1:                    //  - - o - - - - -
      tetromino.block1.x = 0;  //  - - o o o - - -
      tetromino.block1.y = 2;
      tetromino.block2.x = 1;
      tetromino.block2.y = 2;
      tetromino.block3.x = 1;
      tetromino.block3.y = 3;
      tetromino.block4.x = 1;
      tetromino.block4.y = 4;
      tetromino.center.x = 1;
      tetromino.center.y = 3;
      break;
    case 2:                   // - - - - o - - -
      tetromino.block1.x = 0; // - - o o o - - -
      tetromino.block1.y = 4;
      tetromino.block2.x = 1;
      tetromino.block2.y = 2;
      tetromino.block3.x = 1;
      tetromino.block3.y = 3;
      tetromino.block4.x = 1;
      tetromino.block4.y = 4;
      tetromino.center.x = 1;
      tetromino.center.y = 3;
      break;
    case 3:
      tetromino.block1.x = 0; // - - - o o - - -
      tetromino.block1.y = 3; // - - - o o - - -
      tetromino.block2.x = 0;
      tetromino.block2.y = 4;
      tetromino.block3.x = 1;
      tetromino.block3.y = 3;
      tetromino.block4.x = 1;
      tetromino.block4.y = 4;
      tetromino.center.x = 0;
      tetromino.center.y = 0;
      break;
    case 4:
      tetromino.block1.x = 0; // - - - o - - - -
      tetromino.block1.y = 3; // - - o o o - - -
      tetromino.block2.x = 1;
      tetromino.block2.y = 2;
      tetromino.block3.x = 1;
      tetromino.block3.y = 3;
      tetromino.block4.x = 1;
      tetromino.block4.y = 4;
      tetromino.center.x = 1;
      tetromino.center.y = 3;
      break;
    case 5:
      tetromino.block1.x = 0; // - - - o o - -
      tetromino.block1.y = 3; // - - o o - - -
      tetromino.block2.x = 0;
      tetromino.block2.y = 4;
      tetromino.block3.x = 1;
      tetromino.block3.y = 2;
      tetromino.block4.x = 1;
      tetromino.block4.y = 3;
      tetromino.center.x = 1;
      tetromino.center.y = 3;
      break;
    default:
      tetromino.block1.x = 0; // - - o o - - - -
      tetromino.block1.y = 2; // - - - o o - - -
      tetromino.block2.x = 0;
      tetromino.block2.y = 3;
      tetromino.block3.x = 1;
      tetromino.block3.y = 3;
      tetromino.block4.x = 1;
      tetromino.block4.y = 4;
      tetromino.center.x = 1;
      tetromino.center.y = 3;
      break;
  }
  turnOnNewLocation();
  checkIfGameOver();
}

void checkIfGameOver()
{
  if (tetrisMatrix[tetromino.block1.x][tetromino.block1.y] == true ||
      tetrisMatrix[tetromino.block2.x][tetromino.block2.y] == true ||
      tetrisMatrix[tetromino.block3.x][tetromino.block3.y] == true ||
      tetrisMatrix[tetromino.block4.x][tetromino.block4.y] == true)
  {
    matrixController.clearDisplay(0);
    matrixController.clearDisplay(1);
    isGameOver = true;
  }
}

void clearFullRows()
{
  for (int i = 0; i < 16; i++)
  {
    bool isRowFull = true;
    for (int j = 0; j < 8; j++)
    {
      if (tetrisMatrix[i][j] == false)
      {
        isRowFull = false;
        break;
      }
    }

    if (isRowFull)
    {
      for (int j = 0; j < 8 ; j++)
      {
        tetrisMatrix[i][j] = false;
      }
      lightAlreadyPlacedLeds();
      delay(200);

      for (int k = i; k > 0 ; k--)
      {
        for (int l = 0; l < 8; l++)
        {
          tetrisMatrix[k][l] = tetrisMatrix[k - 1][l];
          tetrisMatrix[k - 1][l] = false;

        }
      }
      lightAlreadyPlacedLeds();
      addTetrisScore(15);
      i--;
    }
  }
}

void addTetrisScore(short scoreToAdd)
{
  playerScore += scoreToAdd;
  setTetrisSpeed();
}

void setTetrisSpeed()
{
  if (fallingTetrominoDelay > 300)
  {
    fallingTetrominoDelay = 1000 - playerScore;
  }
}

void turnOffOldLocation()
{
  lightAlreadyPlacedLeds();
  Location block = tetromino.block1;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, false);
  block = tetromino.block2;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, false);
  block = tetromino.block3;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, false);
  block = tetromino.block4;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, false);
}

void turnOnNewLocation()
{
  lightAlreadyPlacedLeds();
  Location block = tetromino.block1;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, true);
  block = tetromino.block2;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, true);
  block = tetromino.block3;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, true);
  block = tetromino.block4;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, true);
}

void lightAlreadyPlacedLeds()
{
  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      matrixController.setLed(i / 8, i % 8, j, tetrisMatrix[i][j]);
    }
  }
}

// The following methods are used for playing the Pong game
void playPong()
{
  setupPong();
  timer = millis();
  aiTimer = millis();
  while (!isGameOver)
  {
    if (millis() - timer > ballDelay)
    {
      moveBall();
      timer = millis();
    }
    if (millis() - aiTimer > aiMovementDelay)
    {
      moveAi();
      aiTimer = millis();
    }
    if (Serial.available() > 0)
    {
      char dir = Serial.read();
      if (dir == 'R' || dir == 'L')
      {
        movePlayer(dir);
      }
    }
  }
  displayPongWinner();
}

void displayPongWinner()
{
  if (playerScore == pointsToWin)
  {
    displayWin();
  }
  else if (aiScore == pointsToWin)
  {
    displayLoss();
  }
  while (Serial.available() == 0) {}
  Serial.write('V');
}

void displayWin()
{
  for (int i = 0; i < 16; i++)
  {
    matrixController.setRow(i / 8, i % 8, winSprite[i]);
  }
}

void displayLoss()
{
  for (int i = 0; i < 16; i++)
  {
    matrixController.setRow(i / 8, i % 8, lossSprite[i]);
  }
}

void setupPong()
{
  String scoreToSend = "C0:0";
  Serial.print(scoreToSend);
  isGameOver = false;
  playerScore = 0;
  aiScore = 0;

  newRound();
}

void newRound()
{
  ballDelay = 250;
  numberOfHits = 0;
  ball.x = random(1, 8);
  ball.y = random(1, 7);
  ballDirection.y = random(-1, 2);
  ballDirection.x = 1;
  playerPaddle.block1.x = 15;
  playerPaddle.block1.y = 3;
  playerPaddle.block2.x = 15;
  playerPaddle.block2.y = 4;
  aiPaddle.block1.x = 0;
  aiPaddle.block1.y = 3;
  aiPaddle.block2.x = 0;
  aiPaddle.block2.y = 4;

  matrixController.clearDisplay(0);
  matrixController.clearDisplay(1);
  matrixController.setLed(0, 0, aiPaddle.block1.y, true);
  matrixController.setLed(0, 0, aiPaddle.block2.y, true);
  matrixController.setLed(1, 7, playerPaddle.block1.y, true);
  matrixController.setLed(1, 7, playerPaddle.block2.y, true);
  matrixController.setLed(ball.x / 8, ball.x % 8, ball.y, true);

  delay(1000);
}

void moveBall()
{
  bool isYSet = false;
  if (ball.y == 0)
  {
    if (ballDirection.y == -1)
    {
      ballDirection.y = 1;
      isYSet = true;
    }
  }
  else if (ball.y == 7)
  {
    if (ballDirection.y == 1)
    {
      ballDirection.y = -1;
      isYSet = true;
    }
  }
  if (ball.x == 1 && ballDirection.x == -1)
  {
    if (aiPaddle.block1.y == ball.y + ballDirection.y || aiPaddle.block2.y == ball.y + ballDirection.y)
    {
      ballDirection.x = 1;
      numberOfHits++;
      if (!isYSet)
      {
        ballDirection.y = random(-1, 2);
      }
      setBallSpeed();
    }
    else
    {
      playerScore++;
      checkIfPongGameOver();
      return;
    }
  }
  else if (ball.x == 14 && ballDirection.x == 1)
  {
    if (playerPaddle.block1.y == ball.y + ballDirection.y || playerPaddle.block2.y == ball.y + ballDirection.y)
    {
      ballDirection.x = -1;
      numberOfHits++;
      if (!isYSet)
      {
        ballDirection.y = random(-1, 2);
      }
      setBallSpeed();
    }
    else
    {
      aiScore++;
      checkIfPongGameOver();
      return;
    }
  }

  matrixController.setLed(ball.x / 8, ball.x % 8, ball.y, false);
  ball.x += ballDirection.x;
  ball.y += ballDirection.y;
  matrixController.setLed(ball.x / 8, ball.x % 8, ball.y, true);
}

void checkIfPongGameOver()
{
  if (playerScore == pointsToWin || aiScore == pointsToWin)
  {
    isGameOver = true;
    return;
  }
  else
  {
    displayScore();
  }
}

void setBallSpeed()
{
  if (ballDelay >= 100)
  {
    ballDelay = 250 - numberOfHits * 10;
  }
}

void moveAi()
{
  if (ballDirection.x == -1)
  {
    if (ball.y == 0 && ball.x == 1 && aiPaddle.block1.y == 0)
    {
      return;
    }
    if (ball.y == 7 && ball.x == 1 && aiPaddle.block2.y == 7)
    {
      return;
    }

    matrixController.setLed(0, 0, aiPaddle.block1.y, false);
    matrixController.setLed(0, 0, aiPaddle.block2.y, false);
    if (ballDirection.y == 0)
    {
      if (ball.y < aiPaddle.block1.y)
      {
        aiPaddle.block1.y--;
        aiPaddle.block2.y--;
      }
      else if (ball.y > aiPaddle.block2.y)
      {
        aiPaddle.block1.y++;
        aiPaddle.block2.y++;
      }
    }
    else if (ballDirection.y == 1)
    {
      if (ball.y < aiPaddle.block1.y)
      {
        aiPaddle.block1.y--;
        aiPaddle.block2.y--;
      }
      else if (ball.y > aiPaddle.block1.y && aiPaddle.block2.y < 7)
      {
        aiPaddle.block1.y++;
        aiPaddle.block2.y++;
      }
    }
    else
    {
      if (ball.y < aiPaddle.block2.y && aiPaddle.block1.y > 0)
      {
        aiPaddle.block1.y--;
        aiPaddle.block2.y--;
      }
      else if (ball.y > aiPaddle.block2.y)
      {
        aiPaddle.block1.y++;
        aiPaddle.block2.y++;
      }
    }

    matrixController.setLed(0, 0, aiPaddle.block1.y, true);
    matrixController.setLed(0, 0, aiPaddle.block2.y, true);
  }
}

void movePlayer(char dir)
{
  matrixController.setLed(1, 7, playerPaddle.block1.y, false);
  matrixController.setLed(1, 7, playerPaddle.block2.y, false);
  if (dir == 'R')
  {
    if (playerPaddle.block2.y < 7)
    {
      playerPaddle.block1.y++;
      playerPaddle.block2.y++;
    }
  }
  else if (dir == 'L')
  {
    if (playerPaddle.block1.y > 0)
    {
      playerPaddle.block1.y--;
      playerPaddle.block2.y--;
    }
  }
  matrixController.setLed(1, 7, playerPaddle.block1.y, true);
  matrixController.setLed(1, 7, playerPaddle.block2.y, true);
}

void displayScore()
{
  matrixController.clearDisplay(0);
  matrixController.clearDisplay(1);
  String scoresToSend = "C" + String(playerScore) + ":" + String(aiScore);
  Serial.print(scoresToSend);
  for (int i = 0; i < 8; i++)
  {
    matrixController.setRow(0, i, numberSprites[aiScore][i]);
    matrixController.setRow(1, i, numberSprites[playerScore][i]);
  }
  while (Serial.available() == 0) {}
  Serial.read();
  newRound();
}
