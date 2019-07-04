#include <ArduinoSTL.h>
#include <LedControl.h>
#include <vector>
#define DINPin 11
#define CSPin 12
#define CLKPin 13

struct Location
{
  byte x;
  byte y;
};

struct Tetromino
{
  Location block1;
  Location block2;
  Location block3;
  Location block4;
  Location center;
} tetromino;

LedControl matrixController(DINPin, CLKPin, CSPin, 2);
bool matrix[16][8];
unsigned long timer;
unsigned long foodTimer;
bool isTimerRunning;
bool isGameOver;
bool foodState;
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
  B01111010,
  B01011010,
  B01011010,
  B01110101
};

char prevDirection;
char direction = 'R';
Location food;
std::vector<Location> snake;

void setup()
{
  Serial.begin(9600);
  matrixController.shutdown(0, false);
  matrixController.shutdown(1, false);
  matrixController.setIntensity(0, 0);
  matrixController.setIntensity(1, 0);
  matrixController.clearDisplay(0);
  matrixController.clearDisplay(1);
  randomSeed(analogRead(0));
  isTimerRunning = true;
  foodState = true;
  isGameOver = false;
  timer = millis();
  foodTimer = millis();
}

void loop()
{
  if(Serial.available() >= 0)
  {
    char game = Serial.read();
    if(game == 'T')
    {
      tetris();
    }
    else if(game == 'S')
    {
      playSnake();
    }
  }
}

void test()
{
  matrixController.setLed(0, 0, 0, true);
  delay(500);
  matrixController.setLed(0, 0, 0, false);
  delay(500);
}

void playSnake()
{
  setupSnake();
  newFood();
  printGame();
  while (!isGameOver)
  {
    if (millis() - timer >= 750)
    {
      timer = millis();
      moveSnake();
    }
    if (Serial.available() >= 0)
    {
      char prevDirection = direction;
      char newDirection =  Serial.read();
      direction = changeDirection(prevDirection, newDirection);
    }
  }
  Serial.println("It's game over");
}

char changeDirection(char prevDirection, char newDirection)
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
  Serial.println("Setupping snake");
  Location startingBlock;
  startingBlock.x = 7;
  startingBlock.y = 2;
  snake.push_back(startingBlock);
  startingBlock.x = 7;
  startingBlock.y = 3;
  snake.push_back(startingBlock);
  startingBlock.x = 7;
  startingBlock.y = 4;
  snake.push_back(startingBlock);
}

void newFood()
{
  Serial.println("Setupping food");
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
        break;
        needNewLocation = true;
      }
      else needNewLocation = false;
    }
    
    if (!needNewLocation)
    {
      Serial.println("Food successfully set.");
      food.x = x;
      food.y = y;
      
    }
  }
  printGame();
}

void printGame()
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
  printGame();
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
  else newFrontLocation.x = frontLocation.x;
  newFrontLocation.y = frontLocation.y;
  snake.insert(snake.begin(), newFrontLocation);
  collectFood(backLocation);
}
void collectFood(Location backLocation)
{
  if (snake[0].x == food.x && snake[0].y == food.y)
  {
    snake.push_back(backLocation);
    newFood();
  }
}

void checkIfSnakeGameOver()
{
  Location front = snake.front();
  for (std::vector<Location>::iterator i = snake.begin() + 1; i != snake.end(); i++)
  {
    if (front.x == i -> x && front.y == i -> y)
    {
      isGameOver = true;
      return;
    }
  }
  isGameOver = false;
}

void tetris()
{
  getNewTetromino();
  while (!isGameOver)
  {
    if (isTimerRunning && (millis() - timer) >= 1000)
    {
      timer = millis();
      shiftDown();
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
      else if (command == 'U')
      {
        rotate();
      }
      else if (command == 'D')
      {
        fastForward();
      }
    }
  }
  gameOver();
}

void gameOver()
{
  while (Serial.available() == 0)
  {
    for (int i = 0; i < 16; i++)
    {
      matrixController.setRow(i / 8, i % 8, gameOverSprite[i]);
    }
    delay(500);
    matrixController.clearDisplay(0);
    matrixController.clearDisplay(1);
    delay(500);
  }
}
void fastForward()
{
  while (matrix[tetromino.block1.x + 1][tetromino.block1.y] != 1 &&
         matrix[tetromino.block2.x + 1][tetromino.block2.y] != 1 &&
         matrix[tetromino.block3.x + 1][tetromino.block3.y] != 1 &&
         matrix[tetromino.block4.x + 1][tetromino.block4.y] != 1 &&
         tetromino.block1.x != 15 && tetromino.block2.x != 15 &&
         tetromino.block3.x != 15 && tetromino.block4.x != 15)
  {
    shiftDown();
    delay(200);
  }
  shiftDown();
}

void rotate()
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
  relativePosition1.x = tempPosition1.y;
  relativePosition1.y = -1 * tempPosition1.x;
  newLocation1.x = tetromino.center.x + relativePosition1.x;
  newLocation1.y = tetromino.center.y + relativePosition1.y;

  tempPosition2.x = tetromino.block2.x - tetromino.center.x;
  tempPosition2.y = tetromino.block2.y - tetromino.center.y;
  relativePosition2.x = tempPosition2.y;
  relativePosition2.y = -1 * tempPosition2.x;
  newLocation2.x = tetromino.center.x + relativePosition2.x;
  newLocation2.y = tetromino.center.y + relativePosition2.y;

  tempPosition3.x = tetromino.block3.x - tetromino.center.x;
  tempPosition3.y = tetromino.block3.y - tetromino.center.y;
  relativePosition3.x = tempPosition3.y;
  relativePosition3.y = -1 * tempPosition3.x;
  newLocation3.x = tetromino.center.x + relativePosition3.x;
  newLocation3.y = tetromino.center.y + relativePosition3.y;

  tempPosition4.x = tetromino.block4.x - tetromino.center.x;
  tempPosition4.y = tetromino.block4.y - tetromino.center.y;
  relativePosition4.x = tempPosition4.y;
  relativePosition4.y = -1 * tempPosition4.x;
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
  else if (matrix[newLocation1.x][newLocation1.y] == false &&
           matrix[newLocation2.x][newLocation2.y] == false &&
           matrix[newLocation3.x][newLocation3.y] == false &&
           matrix[newLocation4.x][newLocation4.y] == false)
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
    matrix[tetromino.block1.x][tetromino.block1.y] = 1;
    matrix[tetromino.block2.x][tetromino.block2.y] = 1;
    matrix[tetromino.block3.x][tetromino.block3.y] = 1;
    matrix[tetromino.block4.x][tetromino.block4.y] = 1;
    getNewTetromino();
    return;
  }
  else if (matrix[tetromino.block1.x + 1][tetromino.block1.y] == 1 ||
           matrix[tetromino.block2.x + 1][tetromino.block2.y] == 1 ||
           matrix[tetromino.block3.x + 1][tetromino.block3.y] == 1 ||
           matrix[tetromino.block4.x + 1][tetromino.block4.y] == 1)
  {
    matrix[tetromino.block1.x][tetromino.block1.y] = 1;
    matrix[tetromino.block2.x][tetromino.block2.y] = 1;
    matrix[tetromino.block3.x][tetromino.block3.y] = 1;
    matrix[tetromino.block4.x][tetromino.block4.y] = 1;
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
  else if (matrix[tetromino.block1.x][tetromino.block1.y - 1] == 1 ||
           matrix[tetromino.block2.x][tetromino.block2.y - 1] == 1 ||
           matrix[tetromino.block3.x][tetromino.block3.y - 1] == 1 ||
           matrix[tetromino.block4.x][tetromino.block4.y - 1] == 1)
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
  else if (matrix[tetromino.block1.x][tetromino.block1.y + 1] == 1 ||
           matrix[tetromino.block2.x][tetromino.block2.y + 1] == 1 ||
           matrix[tetromino.block3.x][tetromino.block3.y + 1] == 1 ||
           matrix[tetromino.block4.x][tetromino.block4.y + 1] == 1)
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
  if (matrix[tetromino.block1.x][tetromino.block1.y] == true ||
      matrix[tetromino.block2.x][tetromino.block2.y] == true ||
      matrix[tetromino.block3.x][tetromino.block3.y] == true ||
      matrix[tetromino.block4.x][tetromino.block4.y] == true)
  {
    matrixController.clearDisplay(0);
    matrixController.clearDisplay(1);
    isGameOver = true;
  }
}

void clearFullRows()
{
  Serial.println("Checking if rows are full");
  for (int i = 0; i < 16; i++)
  {
    bool isRowFull = true;
    for (int j = 0; j < 8; j++)
    {
      if (matrix[i][j] == false)
      {
        isRowFull = false;
        break;
      }
    }

    if (isRowFull)
    {
      Serial.println("There are full rows.");
      for (int j = 0; j < 8 ; j++)
      {
        matrix[i][j] = false;
      }
      lightAlreadyPlacedLeds();
      delay(200);

      for (int l = 0; l < 8; l++)
      {
        if (i != 0)
        {
          matrix[i][l] = matrix[i - 1][l];
          matrix[i - 1][l] = false;
        }

        lightAlreadyPlacedLeds();
      }
      i--;
    }
  }
}

void turnOffOldLocation()
{
  Location block = tetromino.block1;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, false);
  block = tetromino.block2;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, false);
  block = tetromino.block3;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, false);
  block = tetromino.block4;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, false);
  Serial.println("OFF");
  lightAlreadyPlacedLeds();
}

void turnOnNewLocation()
{
  Location block = tetromino.block1;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, true);
  block = tetromino.block2;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, true);
  block = tetromino.block3;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, true);
  block = tetromino.block4;
  matrixController.setLed(block.x / 8, block.x % 8, block.y, true);
  Serial.println("ON");
  lightAlreadyPlacedLeds();
}

void lightAlreadyPlacedLeds()
{
  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (matrix[i][j] == 1)
      {
        matrixController.setLed(i / 8, i % 8, j, true);
      }
    }
  }

  Serial.println("Already placed tetrominos are lit up");
}
