// LibreLeds

#include "TM1651.h"
// Settings
const String usb_name = "batterydisplay:42013";
#define CLK 3 //pins definitions for TM1651 and can be changed to other ports
#define DIO 2
TM1651 batteryDisplay(CLK, DIO);

// Serial Settings
String command;
String readString; //String Buffer

//Timing
unsigned long currentTime = 0;
unsigned long previousTime = 0;

//State
int battery_state = 0;
const int battery_brightness = 7;

bool led_state = false;
int led_level = 0;
int led_level_off = 0;
int led_animation_speed = 0;
int led_animation = 0;

const int SLOW_BLINK = 1000;
const int FAST_BLINK = 500;
const int NO_ANIMATION = 0;
const int BLINKING = 1;
const int CHARGING = 2;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  batteryDisplay.init();
  batteryDisplay.set(battery_brightness); //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
}

void loop()
{
  animation_loop();
  serialManager();
}

void animation_loop()
{
  currentTime = millis();
  if(led_animation > 0){  
    if (currentTime - previousTime > led_animation_speed) {
      previousTime = currentTime;
      if (led_state) {

        batteryDisplay.displayLevel(led_level);
        led_state = false;
      } else {
        batteryDisplay.displayLevel(led_level_off);
        led_state = true;

      }
    }
  }
}

//Equivalent of explode in PHP (use for serial commands parsing)
String splitString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}



void displayState() {
  Serial.println(battery_state);
}

void changeState(int batt_value) {
  if (batt_value == 0) {
    led_level = 0;
    led_animation = NO_ANIMATION;
  }

  //Fast blink
  if ( (batt_value > 0) && (batt_value < 10)) {
    led_level = 1;
    led_animation = BLINKING; 
    led_animation_speed = FAST_BLINK;
    led_level_off = 0;
  }

  //Slow Blink
  if ( (batt_value >= 10) && (batt_value < 20)) {
    led_level = 1;
    led_animation = BLINKING;   
    led_animation_speed = SLOW_BLINK;
    led_level_off = 0;
  }

  //Double Red
  if ( (batt_value >= 20) && (batt_value < 30)) {
    led_level = 1;
    led_animation = NO_ANIMATION;
  }

  //Yellow 1
  if ( (batt_value >= 30) && (batt_value < 40)) {
    led_level = 3;
    led_animation = NO_ANIMATION;
  }

  //Yellow 2
  if ( (batt_value >= 40) && (batt_value < 50)) {
    led_level = 5;
    led_animation = NO_ANIMATION;  
  }

  //Yellow 3
  if ( (batt_value >= 50) && (batt_value < 60)) {
    led_level = 6;
    led_animation = NO_ANIMATION;  
  }

  //Double Green 1
  if ( (batt_value >= 60) && (batt_value < 70)) {
    led_level = 7;
    led_animation = NO_ANIMATION;  
  }

  //Double Green 2
  if ( (batt_value >= 70) && (batt_value < 80)) {
    led_level = 8;
    led_animation = BLINKING;
    led_animation_speed = FAST_BLINK;
    led_level_off = 7;
  }

  //Blue
  if ( (batt_value >= 80) && (batt_value < 90)) {
    led_level = 8;
    led_animation = NO_ANIMATION;
  }

  //Blue Slow Blink
  if ( (batt_value >= 90) && (batt_value < 100)) {
    led_level = 10;
    led_animation = FAST_BLINK;
    led_level_off = 8;
  }

  if (batt_value == 100){
    led_level = 10;
    led_animation = NO_ANIMATION;
  }

 //Charging
  if (batt_value > 100) {
    led_level = 10;
  }
  battery_state = batt_value;
  batteryDisplay.displayLevel(led_level);
}

void serialManager(){
  while (Serial.available()) {
    delay(5);
    if (Serial.available() > 0)
    {
      char c = Serial.read();
      readString += c;
    }
  }

  if (readString.length() > 0)
  {
    bool isLevel = isDigit(readString[0]);

    //Get level
    if (isLevel) {
      changeState(readString.toInt());
      Serial.println(battery_state);
    }
    else {
      //Get global commands
      if (readString[0] == '/') {
        if (readString == "/info")
        {
          Serial.println(usb_name);
        }
        //Get status
        if (readString == "/status") {
          Serial.println(battery_state);
        }
      }
    }
  }
  readString = "";
}

