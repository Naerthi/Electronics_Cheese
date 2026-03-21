#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

// Calibration table (based on your white paper measurements)
const int numPoints = 16;

float voltageTable[numPoints] = {
  2.60, 2.30, 1.63, 1.40, 1.15, 0.97, 0.90, 0.80,
  0.75, 0.70, 0.68, 0.63, 0.60, 0.57, 0.50, 0.40
};

float distanceTable[numPoints] = {
  0,2,3,4,5,6,7,8,9,10,11,12,13,14,15,18
};

// filter variable
float filteredVoltage = 0;

void setup() {

  Serial.begin(115200);
  matrix.begin();

}

void loop() {

  float volts = readVoltage();

  // Low-pass filter to remove spikes
  filteredVoltage = 0.8 * filteredVoltage + 0.2 * volts;

  float distance = getDistance(filteredVoltage);
  float adjusted_distance = distance - 5.48;

  Serial.print("Voltage: ");
  Serial.print(filteredVoltage);
  Serial.print(" V   Distance: ");
  Serial.print(adjusted_distance);
  Serial.println(" cm");

  delay(100);
}

float readVoltage() {

  const int samples = 20;
  long total = 0;

  for(int i = 0; i < samples; i++) {
    total += analogRead(A2);
    delay(2);
  }

  float avgReading = total / (float)samples;

  return avgReading * (5.0 / 1023.0);
}

float getDistance(float volts) {

  // Handle out-of-range values
  if(volts >= voltageTable[0]) return distanceTable[0];
  if(volts <= voltageTable[numPoints-1]) return distanceTable[numPoints-1];

  for(int i = 0; i < numPoints - 1; i++) {

    if(volts <= voltageTable[i] && volts >= voltageTable[i+1]) {

      float ratio = (volts - voltageTable[i+1]) /
                    (voltageTable[i] - voltageTable[i+1]);

      return distanceTable[i+1] +
             ratio * (distanceTable[i] - distanceTable[i+1]);
    }
  }

  return -1; // fallback
}
