/**
   @file config.h
     @author    Washington Superbike
     @date      1-March-2023
     @brief
        If you are looking to change configurations for the overall
        firmware this is the place to do it. Changing the number of CAN devices connected,
        the Screen type used, etc. All can be done from here.
     
     \todo
        CHANGE THE NUM_THERMI based on the number of thermistors that Powertain settles on.
        \n \n
*/

#ifndef _CONFIG_H
#define _CONFIG_H

/// This primarily exists to debug the changes made in the FlexCAN library.
/// If there are no devices connected on the CAN bus, the firmware crashes
/// This line can be set to 0 to ensure that the CAN bus does not bother
/// to check the CAN bus if there are 0 nodes connected.
#define CAN_NODES 0

/// Defines the type of screen to generate in Display.ino.
/// \note NOTE: COMMENT THIS OUT TO CHANGE DISPLAY TYPE TO SPEEDOMETER!
//#define USE_DEBUGGING_SCREEN

/// This exists to be changed based on the final number of thermistors
/// we settle on having in the code later.
#define NUM_THERMI 40

#endif // _CONFIG_H
