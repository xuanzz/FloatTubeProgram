#include <Wire.h>
#include "MS5837.h"
#include <ONE-Shield.h>

MS5837 sensor;
Motor engine(2); // buoyancy engine

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
}

void updateSensor()
{
  // Update pressure and temperature readings
  sensor.read();

  Serial.print("Pressure: ");
  Serial.print(sensor.pressure());
  Serial.println(" mbar");

  Serial.print("Depth: ");
  Serial.print(sensor.depth());
  Serial.println(" m");

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