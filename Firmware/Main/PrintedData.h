/**
 * Header file for some classes to help with printing data to the display.
*/

#ifndef PRINTEDDATA_H_
#define PRINTEDDATA_H_

#include "Display.h"

// Templated function to be used by PrintedVal objects if a printFunction argument is not
// specified. Erases oldData_ and prints *currData_ (of an arbitrary type "T") at (dataX_, y_).
template <typename T>
void defaultValPrintFn(T *oldData, T currData, int xPos, int yPos);

// Abstract base class for tracking information needed to print variables to the display.
class PrintedData {
  public:
    // Constructor. Uses an initialization list.
    PrintedData(int labelX, int dataX, int y, const char *labelPtr, int textSize) :
    labelX_(labelX), dataX_(dataX), y_(y), labelPtr_(labelPtr), textSize_(textSize) { }

    // Member functions.
    void printLabel();                // prints label at (labelX_, y_)
    /* Pure virtual functions. Derived classes need to override these with their
       own implementations. */
    virtual void printData() = 0;     // prints the current data to the display
    virtual void incrementData() = 0; // increments the object's associated data variable (e.g. hv_series_voltage) by 1

  protected:
    // Data members.
    int labelX_;            // x position on screen to print label at
    int dataX_;             //                             + data
    int y_;                 // uses same y position for printing data and label
    const char *labelPtr_;  // "labelPtr" is just the label itself
    int textSize_;          // size to print the data and label in
};  // class PrintedData

// Derived class for printing single values.
template <typename T> 
class PrintedVal : public PrintedData { // PrintedVal inherits from PrintedData
  /* Defines the type "ValPrintFn" as a pointer to a function with a signature of
      'void FnName(T *, T, int, int)'
   
     These functions are responsible for erasing the previously printed data from 
     the screen, updating oldData_, and reprinting the newest data.
     
     They are called by printData() with the following arguments:
      T *oldData: a pointer to oldData_
      T currData: the latest value read from the variable pointed to by currData_
      int xPos:   dataX_
      int yPos:   y_ 
  */
  typedef void(*ValPrintFn)(T *oldData, T currData, int xPos, int yPos);
  
  public:
    PrintedVal(int labelX, int dataX, int y, const char *labelPtr, int textSize, volatile T *currData, ValPrintFn printFunction) :
    PrintedData(labelX, dataX, y, labelPtr, textSize), currData_(currData), printFunction_(printFunction) { }

    // If no ValPrintFn is passed to the constructor, use the defaultValPrintFn.
    PrintedVal(int labelX, int dataX, int y, const char *labelPtr, int textSize, volatile T *currData) :
    PrintedVal(labelX, dataX, y, labelPtr, textSize, currData, defaultValPrintFn<T>) { }

    void printData() override;
    void incrementData() override;

  private:
    T oldData_{(T)(DEFAULT_VAL)};  // stores the last printed value for erasing purposes; initialized to DEFAULT_VAL
    volatile T *currData_;         // points to the variable holding the value to be printed
    ValPrintFn printFunction_;     // points to a function to be called by printData()
};  // class PrintedVal

// Derived class for printing multiple values stored in an array.
template <typename T> 
class PrintedArr : public PrintedData {
  /* Defines the type "ArrayPrintFn" as a pointer to a function with a signature of
      'void FnName(T *, volatile T *, int, int, int)'
      
     These functions are responsible for erasing the previously printed data from 
     the screen, updating oldData_, and reprinting the newest data.
     
     They are called by printData() with the following arguments:
      T *oldDataArr:           oldData_
      volatile T *currDataArr: currData_
      int arrLength:           arrLength_
      int xPos:                dataX_
      int yPos:                y_
  */
  typedef void(*ArrayPrintFn)(T *oldDataArr, volatile T *currDataArr, int arrLength, int xPos, int yPos);
 
  public:
    PrintedArr(int labelX, int dataX, int y, const char *labelPtr, int textSize, T *oldData, T initialVal, volatile T *currData, int arrLength, ArrayPrintFn printFunction) :
    PrintedData(labelX, dataX, y, labelPtr, textSize), oldData_(oldData), currData_(currData), arrLength_(arrLength), printFunction_(printFunction) {
      // Initialize the given oldData array.
      for (int i = 0; i < arrLength_; i++) {
        oldData_[i] = initialVal;
      }
    }
  
    void printData() override;
    void incrementData() override;

  private:
    T *oldData_;                 // points to an array for storing the last printed values for erasing purposes
    volatile T *currData_;       // points to the array holding the values to be printed
    int arrLength_;              // length of the oldData_ array
    ArrayPrintFn printFunction_; // points to a function to be called by printData()
};  // class PrintedArr

#endif  // PRINTEDDATA_H_
