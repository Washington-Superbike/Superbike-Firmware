/**
 * Function definitions for the classes defined in PrintedData.h
*/

#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "PrintedData.h"

extern Adafruit_ILI9341 tft;

void PrintedData::printLabel() { // the definition for the PrintedData class's printLabel() function
  if (labelPtr_ != NULL) {
    tft.setTextSize(textSize_);
    tft.setCursor(labelX_, y_);
    tft.print(labelPtr_);
  }
}

template <typename T> 
void defaultValPrintFn(T *oldData, T currData, int xPos, int yPos) {
  if (*oldData != currData) {
    eraseThenPrint(*oldData, currData, xPos, yPos);
    *oldData = currData;
  }
}

template <typename T> 
void PrintedVal<T>::printData() {
  tft.setTextSize(textSize_);
  printFunction_(&oldData_, *currData_, dataX_, y_);
}

template <typename T> 
void PrintedVal<T>::incrementData() {
  *currData_ = (T)(*currData_ + 1);
}

template <typename T> 
void PrintedArr<T>::printData() {
  tft.setTextSize(textSize_);
  printFunction_(oldData_, currData_, arrLength_, dataX_, y_);
}

template <typename T> 
void PrintedArr<T>::incrementData() {
  if (currData_ != NULL) {
    for (int i = 0; i < arrLength_; i++) {
      currData_[i] = currData_[i] + 1;
    }
  }
}
