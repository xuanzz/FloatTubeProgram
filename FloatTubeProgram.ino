#include <Wire.h>
#include "MS5837.h"
#include <ONE-Shield.h>

MS5837 sensor;
Motor engine(2);     // buoyancy engine
int state = 0;       // 0 = idle, 1 = 1st diving, 2 = 1st rising, 3 = 1st idle, 4 = 2nd diving, 5 = 2nd risingg, 6 = 2nd idle
float startTime = 0; // float start time
String teamNumber = "R01";
int pressureSet1[100];
int pressureSet2[100];
float depthSet1[100];
float depthSet2[100];
int previousPressure = 0;
int PressureDifference = 0;
int cycle = 28;

void setup()
{
  Serial1.begin(9600);
  Wire.begin();
  engine.off();
  while (!sensor.init())
  {
    Serial1.println("Init failed!");
    delay(3000);
  }
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)
}

void loop()
{
  readSerialCommand();
  updateStatus();
}

void sendData(int dataSet)
{
  Serial1.println("Sending data" + (String)dataSet + "...");
  for (int i = 0; i < cycle; i++)
  {
    if (dataSet == 1)
    {
      Serial1.println(teamNumber + "\t" + i*5 + "s\t" + pressureSet1[i] + "kpa\t" + depthSet1[i] + "meters");
    }
    else if (dataSet == 2)
    {
      Serial1.println(teamNumber + "\t" + i*5 + "s\t" + pressureSet2[i] + "kpa\t" + depthSet2[i] + "meters");
    }
    Serial1.print("\t");
  }
  Serial1.println();
}

void readSerialCommand()
{
  if (Serial1.available() > 0)
  {
    char command = Serial1.read();
    switch (command)
    {
    case 'p': // dive
      profile();
      break;
    case 'a': // 1st data request
      sendData(1);
      break;
    case 'b': // 2nd data request
      sendData(2);
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
    Serial1.println(teamNumber + "\tReady to dive...");
    delay(3000);
    break;
  case 1:
    // 1st diving
    break;
  case 2:
    // 1st rising
    break;
  case 3:
    // 1st idle
    Serial1.println(teamNumber + " 1st Dive Completed! Send 'a' to request the 1st data...");
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
    Serial1.println(teamNumber + " 2st Dive Completed! Send 'b' to request the 2st data...");
    delay(3000);
    break;
  default:
    break;
  }
}

void updateSensor()
{
  // Update pressure and temperature readings
  sensor.read();
  Serial1.print(teamNumber + "\t");
  Serial1.print(round(((millis() - startTime) / 1000)));
  Serial1.print("\t");
  Serial1.print(round(sensor.pressure(100)));
  Serial1.print("kpa\t");

  Serial1.print(sensor.depth() + 0.42);
  Serial1.println("meters");
}

void profile()
{
  Serial1.println("Profile...");
  if (state == 0)
  {
    Serial1.println("Profile1...");
    state = 1;
    if (state == 1)
    {
      dive();
      for (int i = 0; i < cycle; i++)
      {
        updateSensor();
        pressureSet1[i] = sensor.pressure(100);
        depthSet1[i] = sensor.depth() + 0.42;
        if (i == 2 || i == cycle / 2 + 1)
        stop(); //stop the engine at 10s
        if (i == cycle / 2 - 1)
        {
          rise();
          state = 2;
        }
        delay(5000);

      }
      state = 3;
    }
  }
  else if (state == 3)
  {
    Serial1.println("Profile2...");
    state = 4;
    if (state == 4)
    {
      dive();
      for (int i = 0; i < cycle; i++)
      {
        updateSensor();
        pressureSet2[i] = sensor.pressure(100);
        depthSet2[i] = sensor.depth() + 0.42;
        if (i == 2 || i == cycle / 2 + 1)
        stop(); //stop the engine at 10s
        if (i == cycle / 2 - 1)
        {
          rise();
          state = 5;
        }
        delay(5000);
      }
      state = 6;
    }
  }
}

void dive()
{
  Serial1.println("Diving...");
  engine.turn(-255);
//  delay(10000);
//  engine.off();
}

void rise()
{
  Serial1.println("Rising...");
  engine.turn(255);
  // delay(10500);
  // engine.off();
}

void stop()
{
  Serial1.println("Engine Stopped");
  engine.off();
}