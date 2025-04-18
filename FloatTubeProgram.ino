#include <Wire.h>
#include "MS5837.h"
#include <ONE-Shield.h>
#include <FastLED.h>

MS5837 sensor;
Motor engine(2);     // buoyancy engine
int state = 0;       // 0 = idle, 1 = 1st diving, 2 = 1st rising, 3 = 1st idle, 4 = 2nd diving, 5 = 2nd risingg, 6 = 2nd idle
float startTime = 0; // float start time
String teamNumber = "R9";
int pressureSet1[100];
int pressureSet2[100];
float depthSet1[100];
float depthSet2[100];
int previousPressure = 0;
int PressureDifference = 0;
int Time = 0;
#define NUM_LEDS 3 // number of LEDs
#define DATA_PIN 21 //port
CRGB leds[NUM_LEDS]; // array of LEDs
int lastsec = 0; //previous recorded time
int pauseTime = 0;
int i = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  engine.off();
  while (!sensor.init())
  {
    Serial.println("Init failed!");
    delay(3000);
  }
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  leds[0] = CRGB::Green;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Green;
  FastLED.show();

  Serial.print("enter teamNumber:");
  teamNumber = Serial.read();
}

void loop()
{
  readSerialCommand();
  updateStatus();
    // Turn the LED on, then pause
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Yellow;
  leds[2] = CRGB::Green;
  FastLED.show();
  delay(500);
  // Now turn the LED off, then pause
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  leds[2] = CRGB::Black;
  FastLED.show();
  delay(500);
}

void sendData(int dataSet)
{
  Serial.println("Sending data" + (String)dataSet + "...");
  for (int i = 0; i < Time ; i++)
  {
    if (dataSet == 1)
    {
      Serial.println(teamNumber + "\t" + i + "s\t" + pressureSet1[i] + "kpa\t" + depthSet1[i] + "meters");
    }
    else if (dataSet == 2)
    {
      Serial.println(teamNumber + "\t" + i + "s\t" + pressureSet2[i] + "kpa\t" + depthSet2[i] + "meters");
    }
    Serial.print("\t");
  }
  Serial.println();
}



void readSerialCommand()
{
  if (Serial.available() > 0)
  {
    char command = Serial.read();
    switch (command)
    {
    case 'p': // dive
      profile();
      break;
    case 'a': // 1st data request
      sendData(1);
      state = 4;
      break;
    case 'b': // 2nd data request
      sendData(2);
      state = 0;
      break;
    case 'd': // manual dive
      dive();
      delay(8000);
      stop();
      break;
    case 'r': // manual rise
      rise();
      delay(10000);
      stop();
      break;
    case 's': // stop for emergency
      stop();
      state = 0;
    default:
      break;
    }
  }
}

void updateStatus()
{
  switch (state)
  {
  case 0:
    Serial.println(teamNumber + "\tReady to dive...");
    delay(3000);
    break;
  case 1:
    // 1st diving
    break;
  case 2:
    // 1st rising
    break;
  case 3:
    leds[0] = CRGB::Yellow;
    leds[1] = CRGB::Yellow;
    leds[2] = CRGB::Yellow;
    FastLED.show();
    // 1st idle
    Serial.println(teamNumber + " 1st Dive Completed! Send 'a' to request the 1st data...");
    delay(3000);
    break;
  case 4:
    // 2nd diving
    break;
  case 5:
    // 2nd rising
    break;
  case 6:
    // 2nd idle
    leds[0] = CRGB::Yellow;
    leds[1] = CRGB::Yellow;
    leds[2] = CRGB::Yellow;
    FastLED.show();
    Serial.println(teamNumber + " 2st Dive Completed! Send 'b' to request the 2st data...");
    delay(3000);
    break;
  default:
    break;
  }
}

void updateSensor()// Update pressure and temperature readings
{
  sensor.read();
  Serial.print(teamNumber + "\t");
  Serial.print(round(((millis() - startTime) / 1000)));
  Serial.print("\t");
  Serial.print(round(sensor.pressure(0.1)));
  Serial.print("kpa\t");
  Serial.print(sensor.depth());
  Serial.println("m");
}

void profile()
{
  Serial.println("Profile...");
  if (state == 0)
  {
    Serial.println("Profile1...");
    state = 1;
    if (state == 1)
    {
      dive();
      startTime = millis();
      bool paused = false;
      bool rised = false;
      for (i = 0; state < 3; i = ((millis()-startTime)/1000))
      {
        if (i == lastsec+1)
        {
          updateSensor();
          pressureSet1[i] = sensor.pressure(0.1);
          depthSet1[i] = sensor.depth() + 0.42;
          lastsec++; 
          if (pressureSet1[i] >= pressureSet1[i-1]+0.1)
          {
            if (!paused)
            {
            pauseTime = millis()/1000;
            stop();
            paused = true;
            }
            else if ((i - pauseTime) >= 46 && !rised)
            {
              rise();
              rised = true;
              state = 2;
            }
            else if (depthSet1[i] > 0.2 && rised)
            {
              stop();
              Time = i;
              state = 3;
            }
          }
        }
      }
    }  
  }
  else if (state == 4)
  {
    Serial.println("Profile2...");
    dive();
    startTime = millis();
    int pauseTime = 0;
    bool paused = false;
    bool rised = false;
    for (i = 0; i < Time; i = ((millis()-startTime)/1000))//i = time(second) in profiling.
    {
      if (i == lastsec+1)//updateSensor and record data per second.
      {
        updateSensor();
        pressureSet2[i] = sensor.pressure(0.1);
        depthSet2[i] = sensor.depth() + 0.42;
        lastsec++;  
        if (pressureSet1[i] >= pressureSet2[i-1]-0.1)//at the botton
        {
          if (!paused)//engine stopped at the bottob. Added bool 'paused' to prevent from looping
          {
          pauseTime = millis()/1000;
          stop();
          paused = true;
          }
          else if ((i - pauseTime) >= 46 && !rised)//rise after pausing for 45 seconds. Added bool 'rised' to prevent from looping.
          {
            rise();
            rised = true;
            state = 5; 
          }
          else if (i == Time)//engine stopped when i == recorded Time in the first time
          {
            stop();
            state = 6;
          }
        }  
      }  
    }
  }
}




void dive()
{
  Serial.println("Diving...");
  engine.turn(-255);
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Red;
  leds[2] = CRGB::Red;
  FastLED.show();
  delay(10000);//  delay(10000);
  engine.off();//  engine.off();
}

void rise()
{
  Serial.println("Rising...");
  engine.turn(255);
  leds[0] = CRGB::Green;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Green;
  FastLED.show();
  delay(10500);// delay(10500);
  engine.off();// engine.off();
}

void stop()
{
  Serial.println("Engine Stopped");
  engine.off();
}
