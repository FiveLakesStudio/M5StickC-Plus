#include <M5StickCPlus.h>
#include "UtilUltrasonicSensor.h"

float GetDistanceFeetAverage(uint8_t numSamples)
{
  float sum = 0;
  int numSamplesTaken = 0;
  float distance = UltrasonicSensorUnknownDistance;
  float neighborDifferenceThreshold = 0.4;

  float samples[numSamples];

  // Get all samples
  for (uint8_t index = 0; index < numSamples; index++) {
    distance = GetDistanceFeet();
    samples[index] = distance;
    if (distance != UltrasonicSensorUnknownDistance) {
      numSamplesTaken += 1;
    }
    delay(UltrasonicSensorSampleDelayMs); // Add a short delay between readings
  }

  // Calculate the average while excluding samples that are more than 0.4 from their neighbors
  int validSamples = 0;
  for (uint8_t index = 0; index < numSamples; index++) {
    bool includeSample = true;

    // Check the difference with the previous neighbor
    if (index > 0 && samples[index - 1] != UltrasonicSensorUnknownDistance &&
        abs(samples[index] - samples[index - 1]) > neighborDifferenceThreshold) {
      includeSample = false;
    }

    // Check the difference with the next neighbor
    if (index < numSamples - 1 && samples[index + 1] != UltrasonicSensorUnknownDistance &&
        abs(samples[index] - samples[index + 1]) > neighborDifferenceThreshold) {
      includeSample = false;
    }

    if (includeSample && samples[index] != UltrasonicSensorUnknownDistance) {
      sum += samples[index];
      validSamples += 1;
    }
  }

  if (distance == UltrasonicSensorUnknownDistance || validSamples == 0) {
    return UltrasonicSensorUnknownDistance;
  }

  return sum / validSamples;
}


// Returns UltrasonicSensorUnknownDistance (-1) if we couldn't read the value
//
float GetDistanceFeet() 
{
  digitalWrite(UltrasonicSensorTriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(UltrasonicSensorTriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(UltrasonicSensorTriggerPin, LOW);

  long duration = pulseIn(UltrasonicSensorEchoPin, HIGH);

  /*
  ** When the HC-SR04 sensor sends an ultrasonic pulse, it travels to an object, reflects off of it, and then returns to the sensor.
  ** The sensor measures the total time it takes for the pulse to travel to the object and back, called the "round-trip time." To 
  ** find the distance to the object, we need to consider only the one-way time, i.e., the time it takes for the pulse to travel 
  ** from the sensor to the object. This is why we divide the round-trip time by 2.
  **
  ** Now let's examine the 0.0344 value. The speed of sound in air is approximately 343 meters per second (m/s) or 34,300 centimeters
  ** per second (cm/s) at room temperature (20°C). When we calculate the distance, we need to convert the time measured by the sensor
  ** (in microseconds) into distance (in centimeters). To do this, we can use the following formula:
  **
  **    distance (cm) = (time (µs) * speed of sound (cm/µs)) / 2
  **
  ** Since the speed of sound is 34,300 cm/s, we need to convert it to cm/µs:
  **
  **    34,300 cm/s * (1 s / 1,000,000 µs) = 0.0343 cm/µs (rounded to 0.0344 cm/µs for simplicity)
  **
  ** Now, we can rewrite the formula as:
  **
  **    distance (cm) = (time (µs) * 0.0344) / 2
  **
  ** So, the 0.0344 / 2 expression in the code is a result of the speed of sound conversion (from cm/s to cm/µs) and
  ** considering only the one-way travel time of the ultrasonic pulse.
  */
  float distanceCm = duration * 0.0344 / 2;

  // Check if the measured distance is within the valid range
  //
  if (distanceCm < UltrasonicSensorMinDistanceCm || distanceCm > UltrasonicSensorMaxDistanceCm) 
    return UltrasonicSensorUnknownDistance;

  // Convert CM to Feet 
  // 
  float distanceFeet = distanceCm * 0.0328084;

  return distanceFeet;
}

