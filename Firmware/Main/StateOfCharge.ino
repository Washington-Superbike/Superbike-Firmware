#include "StateOfCharge.h"

void SOC_FSMTransitionActions (SOCTaskData socData){
  switch (*(socData.SOC_State)) { // transitions
    case SOC_START:

      break;
    case PC_OPEN:

      break;
    case PC_CLOSE:

      break;
    case PC_JUST_CLOSED:

      break;
    default:
      *socData.SOC_State = SOC_START;
      break;
  } // transitions
}

void SOC_FSMStateActions (SOCTaskData socData){
  switch (*(socData.SOC_State)) { // state actions\
    case PC_START:
    
    case PC_OPEN:

      break;
    case PC_CLOSE:

      break;
    case PC_JUST_CLOSED:

      break;
    default:
      break;
  } // state actions

  }
}
