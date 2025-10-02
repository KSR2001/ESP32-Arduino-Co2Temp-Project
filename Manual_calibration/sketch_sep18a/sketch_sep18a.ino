
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h" // or senseBox SCD30 library
SCD30 airSensor;
const unsigned long timeUntilForcedCalibration = 180000;
unsigned long currentMillis = 0;
unsigned long startWaitUntilForcedMillis = 0;
int frc_value = 422;
void setup() {
  Wire.begin();
  Serial.begin(9600);
  if (airSensor.begin(false) == false) {
    Serial.println("Air sensor not detected. Check wiring!");
    while (1);
  }
  delay(2000);
  Serial.println("Sensor initialized.");
  // airSensor.setMeasurementInterval(2);
  startWaitUntilForcedMillis = millis();
}



void loop() {
  currentMillis = millis();
  if (currentMillis - startWaitUntilForcedMillis >= timeUntilForcedCalibration) {
    Serial.println("Sensor calibration is starting now...");
    // Apply FRC with 400 ppm reference (outdoor air)
    if (airSensor.setForcedRecalibrationFactor(frc_value)) {
      Serial.println("FRC applied successfully at 422 ppm.");
      // led turn green
    } else {
      // led turn red0
      Serial.println("FRC failed!");
    }
    while (1)
      ;
  }
}
