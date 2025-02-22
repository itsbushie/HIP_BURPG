#include <HX711.h>
#include <SPI.h>
#include <SD.h>

/*
I defined all the Pin #'s here so you won't need to remember them
Refer to the schematic if you do not know where everything is wired to
I also imported all the libraries needed. When we actually upload the code to the arduino we will need to download these libraries
*/
#define HX711_CLK_PIN 2 
#define HX711_DOUT_PIN 3
#define EMATCH_PIN 4
#define SOLENOID_PIN 7
#define SD_CS_PIN 8
#define BUTTON_PIN 9

HX711 force;
File dataFile;
unsigned long startTime;
unsigned long buttonPressStartTime = 0;
bool buttonHeld = false;
bool solenoidActivated;
bool ignitionStarted;
bool launchActive;

void setup(){
    //Serial.begin(9600) starts the capture of data of the Microcontroller, 9600 is the bit rate (rate of information sent/recieved)
    Serial.begin(9600);
    solenoidActivated = false;
    ignitionStarted = false;

    //force.begin captures the data from the HX711 which is wired from the load cell. The Dataout Pin goes to Arduino and Arduino supplies CLK
    //CLK is the clock rate of the processor.

    //uncomment below!!!!
    //force.begin(HX711_DOUT_PIN, HX711_CLK_PIN);

    //Declares the Output pin objects
    pinMode(SOLENOID_PIN, OUTPUT);
    pinMode(EMATCH_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    //Here to make sure that the Solenoid and Ematch is off
    //Setting the voltage to low ensures they are off before launch
    digitalWrite(SOLENOID_PIN, LOW);
    digitalWrite(EMATCH_PIN, LOW);

    // Here to make sure that the SD card is reading properly, if it doesn't I added an infinite loop to basically "freeze" the launch
    // Here for safety/data reasons
    
    /*
    if (!SD.begin(SD_CS_PIN)){
      Serial.println("SD card not working properly");
      while (1);
    }
    Serial.println("SD card works and initialized");
    
    Serial.println("Trial BELOW");
    */
}

/*
Begins by reading the LoadCell data coming from HX711
Logs the data into a file on the SD Card
Starts the timer using the millis function
Opening the Solenoid and igniting for about 1000 ms
if that amount of time has passed close the Solenoid and stop igniting the Ematch
*/

void loop(){
  
  /*
  float loadCellReading = readloadCell();
  logData(loadCellReading);
  */

  unsigned long currentTime = millis();

  if (digitalRead(BUTTON_PIN) == LOW && !buttonHeld){
    buttonPressStartTime = currentTime;
    buttonHeld = true;
  }

  if (digitalRead(BUTTON_PIN) == HIGH && buttonHeld){
    unsigned long buttonHoldDuration = currentTime - buttonPressStartTime;
    buttonHeld = false;

    if (buttonHoldDuration > 1000){
      Serial.println("Button held for one second. Activating launch.");
      startLaunch();
    }
    else{
      Serial.println("Button pressed briefly.");
      stopLaunch();
    }
  }
  if (launchActive){
    if (!solenoidActivated && !ignitionStarted){
      startTime = millis();
      openSolenoid();
      startIgnition();
      solenoidActivated = true;
      ignitionStarted = true; 
      Serial.println(millis());
    }

    if (solenoidActivated && ignitionStarted && (millis() - startTime > 2000)){
      closeSolenoid();
      killIgnition();
      Serial.println(millis());
      launchActive = false;
    }
  }
  
}

//returns the data from the loadcell
float readloadCell(){
  return analogRead(A0);
}

//Helper function to write the data from the loadcell into a file within the SD card
void logData(float loadCellValue){
  dataFile = SD.open("flightData.txt", FILE_WRITE);
  if (dataFile){
    dataFile.println(String("Load Cell: ") + loadCellValue);
    dataFile.flush();
    dataFile.close();
    Serial.println("Data logged.");
  }
  else {
    Serial.println("Failed to open file.");
  }
} 

void startLaunch(){
  launchActive = true;
  Serial.println("Launch Started");
}

void stopLaunch(){
  closeSolenoid();
  killIgnition();
  launchActive = false;
  solenoidActivated = false;
  ignitionStarted = false;
  Serial.println("Launch Sequence Stopped.");
}

//Passes voltage to MOSFET to open Solenoid
void openSolenoid(){
    digitalWrite(SOLENOID_PIN, HIGH);
    Serial.println("Solenoid Opened");
}

//Depletes voltage to MOSFET to close Solenoid
void closeSolenoid(){
    digitalWrite(SOLENOID_PIN, LOW);
    Serial.println("Solenoid Closed.");
}

//Passes voltage to MOSFET to Ignite EMATCH
void startIgnition(){
  digitalWrite(EMATCH_PIN, HIGH);
  Serial.println("Nozzle Ignited.");
}

//Depletes voltage to MOSFET to stop sparking the EMATCH
void killIgnition(){
  digitalWrite(EMATCH_PIN, LOW);
  Serial.println("Killed ignition.");
}