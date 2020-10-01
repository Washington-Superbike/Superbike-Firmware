#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <elapsedMillis.h>
#include <FlexCAN_T4.h>


#define WHEEL_CIRCUMFERENCE 1 //for calculating speed
#define GEAR_RATIO 1 //for calculating rpm
#define MOTOR_MSG_1 0x0CF11E05 //motor controller message - CAN
#define MOTOR_MSG_2 0x0CF11F05 // motor controller message - CAN
#define LED_PIN 13 // pin for the teensy led (just for testing)
#define STANDBY 20 // when true, turns off the connection to the can bus from the Teensy
#define TFT_CS 10 //The display chip select pin
#define TFT_DC 9 // the display
#define TFT_RST 6 //the display reset pin
#define UPDATE_FREQUENCY 10 //the update frequency for loops on the teensy in milliseconds
#define ROLLING_AVERAGE_LEN 6 // the length of the array used for rolling average
#define CONTACTOR 17 //digital pin for contactor control
#define PRECHARGE_RELAY 18 //digital pin for pre-charge relay control

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); //the display controller

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN_bus; //access to can bus reading
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CAN_loop;

CAN_message_t reading; // data is read into this from the can bus
CAN_message_t sending; // for loopback
//All of these represent the data returned to us from the CAN bus, we can remove these as fields later
//and have them be directly passed to the display update/ error functions later
float velocity=0;
float current_used=0;
float battery_voltage=0;
float throttle=0;
float controller_temperature=0;
float motor_temperature=0;
byte controller_status=0;
byte switch_signals_status=0;
//used for refreshing display
elapsedMillis display=0;
uint16_t error_code=0;
float preChargeAverage[ROLLING_AVERAGE_LEN];

void setup() {
  //testing stuff for display
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  //starts can bus
   Serial.begin(9600);
  CAN_bus.begin();
  CAN_bus.setBaudRate(250000);
  CAN_loop.begin();
  CAN_loop.setBaudRate(250000);
  pinMode(STANDBY,OUTPUT);
  digitalWrite(STANDBY, LOW); // turns on the CAN bus transceivers
  
  //pre-charge begins
  pinMode(CONTACTOR, OUTPUT);
  pinMode(PRECHARGE_RELAY, OUTPUT);
  int testVoltages[50]={0}; 
  int precharge_voltage=72;
  for(int i=0;i<50;i++){
    testVoltages=(i/50.0)*sqrt(precharge_voltage);
  }
  bool testingCAN=true;
  //fills the rollingAverage array
  float lastPrechargeVoltage=battery_voltage;
  for(int i=0;i<ROLLING_AVERAGE_LEN;i++){
    //fills the precharge average with voltages of 100 because it measures the difference in voltage from our battery so 100 is never possible
    preChargeAverage[i]=100;
  }
  //start the pre-charge circuit
  digitalWrite(CONTACTOR, LOW);
  digitalWrite(PRECHARGE_RELAY, HIGH);
  //the pre charge circuit continues to run until the change in battery voltage over time drops to below 2 volts
  //and also if is positive, because turning on the bike while the motor controller capacitor still has charge causes it read negative average values
  float rolAverage;
  int i=0;
  while((rolAverage=average(preChargeAverage, ROLLING_AVERAGE_LEN)>2) && rolAverage>0){
     lastPrechargeVoltage=battery_voltage;
     checkCAN();
     if(lastPrechargeVoltage!=battery_voltage){ //if there is new data then shift the average by one data point
      for(int i=0;i<ROLLING_AVERAGE_LEN-1;i++){
          preChargeAverage[i]=preChargeAverage[i+1];
          //will be changed to a percentage printed on the display
          Serial.print(preChargeAverage[i]);Serial.print(":");Serial.println();
     }
        preChargeAverage[ROLLING_AVERAGE_LEN-1]=lastPrechargeVoltage-battery_voltage;
     }
     //Serial.println(average(preChargeAverage, ROLLING_AVERAGE_LEN));
     if(testingCAN){
      if(i<50){
        loopMChighVoltage(testingVoltages[i]);
      }
     }
     checkForUpdates();
     delay(UPDATE_FREQUENCY);
  }
  //turn off pre-charging circuit
  digitalWrite(CONTACTOR, HIGH);
  digitalWrite(PRECHARGE_RELAY, LOW);
  //this will be changed to a display message
  Serial.println("DONE PRECHARGING");
}

void checkForUpdates(){
  if(display>=50){
       updateDisplay();
       display=0;
    }
}

void loop() {
    checkCAN();
    //updates the display every 50 milliseconds
    checkForUpdates();
   delay(UPDATE_FREQUENCY);
}

//used for pre-charge
float average(float arr[], int len){
  float sum=0;
  for(int i=0;i<len;i++){
    sum+=arr[i];
  }
  return sum/len;
}

//checks the can bus for any new data (currenly only the motor controller messages)
void checkCAN(){
   if (CAN_bus.read(reading)!=0){ // if we read a message
      if(reading.id==MOTOR_MSG_1){ //motor controller message
        decipherMessageOne(reading); 
      }else if(reading.id==MOTOR_MSG_2){ //motot controller message
        decipherMessageTwo(reading);
      }
    }
}

//haven't implemented display yet
void updateDisplay(){
  Serial.printf("vel: %.3f\n",velocity);
  Serial.printf("current: %.3f\n",current_used);
  Serial.printf("voltage: %.3f\n",battery_voltage);
  Serial.printf("error: %x\n",error_code);
  Serial.printf("throttle: %.3f\n", throttle);
  Serial.printf("controller temp: %.3f 'C\n", controller_temperature);
  Serial.printf("motor temp: %.3f 'C\n", motor_temperature);
  Serial.printf("controller status: %x\n", controller_status);
  Serial.printf("signals status: %x\n", switch_signals_status);
}


void decipherMessageOne(CAN_message_t msg){
  velocity=RPMtoMPH((msg.buf[1]<<8)|msg.buf[0]);
  current_used=((msg.buf[3]<<8)|msg.buf[2])/10.0;
  battery_voltage=((msg.buf[5]<<8)|msg.buf[4])/10.0;
  error_code=((msg.buf[7]<<8)|msg.buf[6]);
}

void decipherMessageTwo(CAN_message_t msg){
  throttle=msg.buf[0]/255.0;
  controller_temperature=msg.buf[1]-40;
  motor_temperature=msg.buf[2]-30;
  controller_status=msg.buf[4];
  switch_signals_status=msg.buf[5];
}

float RPMtoMPH(int rpm){
  //for getting the speed of the bike in miles/hour, a little unsure of calculations bc i did them fast and moved on
  return rpm*WHEEL_CIRCUMFERENCE*GEAR_RATIO * 0.0372823; // proportional factor is meters/minute to miles per hour conversion, wheel circumference has to be in meters
}

bool digitalToggle(byte pin){
  bool state = !digitalRead(pin);
  digitalWrite(pin, state);
  return state;
}

void loopMCHighVoltage(int voltage){
    sending.id=MOTOR_MSG_1;
    voltage=voltage*10;
    memset(sending.buf,0,sizeof(sending.buf));
    sending.buf[4]=voltage&0xFF;
    sending.buf[5]=voltage>>8&0xFF;
    CAN_loop.write(sending);
}

/*
void printMessage(const CAN_message_t &msg){
  for(int i=0;i<msg.len;i++){
      Serial.print(msg.buf[i]);
      Serial.print(":");
  }
  Serial.println();
  }

*/
