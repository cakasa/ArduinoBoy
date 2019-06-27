#include <MaxMatrix.h>
int DINPin = 11;
int CSPin = 12;
int CLKPin = 13;

MaxMatrix matrixController(DINPin, CSPin, CLKPin, 1);

void setup()
{
  matrixController.init();
  matrixController.setIntensity(2);
}

void loop()
{
  getNewTetromino();
  delay(1000);
  matrixController.shiftDown(false);
  delay(1000);
  matrixController.shiftDown(false);
  delay(1000);
  matrixController.shiftDown(false);
}

void getNewTetromino()
{
  int tetrominoIndex = rand() % 7;
  switch(tetrominoIndex)
  {
    case 0:
      matrixController.setDot(2, 0, true);
      matrixController.setDot(3, 0, true);
      matrixController.setDot(4, 0, true);
      matrixController.setDot(5, 0, true);
      break;
    case 1:
      matrixController.setDot(2, 0, true);
      matrixController.setDot(2, 1, true);
      matrixController.setDot(3, 1, true);
      matrixController.setDot(4, 1, true);
      break;
    case 2:
      matrixController.setDot(4, 0, true);
      matrixController.setDot(2, 1, true);
      matrixController.setDot(3, 1, true);
      matrixController.setDot(4, 1, true);
      break;
    case 3:
      matrixController.setDot(3, 0, true);
      matrixController.setDot(4, 0, true);
      matrixController.setDot(3, 1, true);
      matrixController.setDot(4, 1, true);
      break;
    case 4:
      matrixController.setDot(3, 0, true);
      matrixController.setDot(4, 0, true);
      matrixController.setDot(2, 1, true);
      matrixController.setDot(3, 1, true);
      break;
    case 5:
      matrixController.setDot(3, 0, true);
      matrixController.setDot(4, 1, true);
      matrixController.setDot(2, 1, true);
      matrixController.setDot(3, 1, true);
      break;
    case 6:
      matrixController.setDot(2, 0, true);
      matrixController.setDot(3, 0, true);
      matrixController.setDot(3, 1, true);
      matrixController.setDot(4, 1, true);
      break;
  }
}
