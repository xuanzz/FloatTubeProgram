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
int Time;
#define NUM_LEDS 3 // number of LEDs
#define DATA_PIN 21 //port
CRGB leds[NUM_LEDS]; // array of LEDs
int pauseTime;
int riseTime;

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
    Serial.println(teamNumber + " 1st Dive Completed! Send 'a' to request the 1st data... press 'p' to profile...");
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
      int lastsec = 0;
      for (int i = 0; state < 3;)//record time in second with i
      {
        i = ((millis()-startTime)/1000);//i = time(second) in profiling.
      
        if (i == lastsec+1)
        { 
          lastsec++;
          pressureSet1[i] = sensor.pressure(0.1);
          depthSet1[i] = sensor.depth() + 0.42;
          updateSensor();
          if (pressureSet1[i] == pressureSet1[i-1])
          {
            Serial.println(i - pauseTime);
            if (!paused)
            {
              pauseTime = i;
              stop();
              paused = true;
              Serial.println("paused:" + paused);
            }
            if ((i-pauseTime) >= 45 && !rised)
            {
              riseTime = i;
              rise();
              rised = true;
              state = 2;
              if (depthSet1[i] <= 0.1)
              {
                stop();
                Serial.println("rised:" + rised);
                Time = i;
                state = 3;
              }
            } 
          }
        }
      }
    }  
  }
  if (state == 4)
  {
    dive();
    Serial.println("profile2...");
    startTime = millis();
    bool paused = false;
    bool rised = false;
    int lastsec = 0;
    for (int i = 0; state < 6;)//record time in second with i
    {
      i = ((millis()-startTime)/1000);//i = time(second) in profiling.
      if (i == lastsec+1)
      { 
        lastsec++;//lastsec+1
        pressureSet2[i] = sensor.pressure(0.1);
        depthSet2[i] = sensor.depth() + 0.42;
        updateSensor();
        if (pressureSet2[i] == pressureSet2[i-1])//when it reaches the bottom, which pressure remains the same. 
        {
          if (!paused)//add boolean paused prvent from looping. Pause at the bottom.
          {
            pauseTime = i;
            stop();
            paused = true;
            Serial.println(" Paused:" + paused);
          }
          if ((i-pauseTime) >= 45 && !rised)//rise after 45s.
          {
            rise();
            rised = true;
            state = 5;
            if (depthSet2[i] <= 0.1)//reaches the surface.
            {
              stop();
              Serial.println(" Rised:" + rised);
              Time = i;
              state = 6;
            }
          } 
        }
      }
    }
  }  
}




void dive()
{
    Serial.println("Diving...");
    unsigned long diveStart = millis();
    engine.turn(-500);
    leds[0] = CRGB::Red;
    leds[1] = CRGB::Red;
    leds[2] = CRGB::Red;
    FastLED.show();
    if ((millis()-diveStart) >= 10000)
     {
       engine.off();
     }
}

void rise()
{
    Serial.println("Diving...");
    unsigned long riseStart = millis();
    engine.turn(500);
    leds[0] = CRGB::Red;
    leds[1] = CRGB::Red;
    leds[2] = CRGB::Red;
    FastLED.show();
    if ((millis()-riseStart) >= 10000)
     {
       engine.off();
     }
}

void stop()
{
  Serial.println("Engine Stopped");
  engine.off();
}
