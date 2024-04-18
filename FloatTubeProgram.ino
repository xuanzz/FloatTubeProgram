#include <Wire.h>
#include "MS5837.h"
#include <ONE-Shield.h>

MS5837 sensor;
Motor engine(2); // buoyancy engine
int state = 0;   // 0 = idle, 1 = 1st diving, 2 = 1st rising, 3 = 1st idle, 4 = 2nd diving, 5 = 2nd risingg, 6 = 2nd idle
float startTime = 0; // float start time
String teamNumber="R01";

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
}

void loop()
{
  readSerialCommand();
  updateStatus();
}

void readSerialCommand()
{
  if (Serial.available() > 0)
  {
    char command = Serial.read();
    switch (command)
    {
    case 'd': // dive
      dive();
      break;
    case 'r': // rise
      rise();
      break;
    default:
      break;
    }
  }
}

void updateStatus(){
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
    // 1st idle
    Serial.println(teamNumber + "1st Dive Completed! Send 'a' to request the 1st data...");
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
    Serial.println(teamNumber + "2st Dive Completed! Send 'b' to request the 2st data...");
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
  Serial.print(round(((millis()-startTime)/1000)));
  Serial.print("\t");
  Serial.print(round(sensor.pressure(100)));
  Serial.print("kpa\t");

  Serial.print(sensor.depth());
  Serial.println("meters");

  delay(1000);
}

void dive()
{
  Serial.println("Diving...");
  engine.turn(100);
  delay(2000);
  engine.off();
}

void rise()
{
  Serial.println("Rising...");
  engine.turn(-100);
  delay(2200);
  engine.off();
}