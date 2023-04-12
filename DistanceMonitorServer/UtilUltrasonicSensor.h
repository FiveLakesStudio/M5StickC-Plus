#ifndef UTIL_ULTRASONIC_SENSOR_H
#define UTIL_ULTRASONIC_SENSOR_H

#define UltrasonicSensorTriggerPin 26
#define UltrasonicSensorEchoPin 36
const float UltrasonicSensorMinDistanceCm = 2;
const float UltrasonicSensorMaxDistanceCm = 400;
const float UltrasonicSensorUnknownDistance = -1;
const uint8_t UltrasonicSensorSampleCount = 5;
const unsigned long UltrasonicSensorSampleDelayMs = 10;

float GetDistanceFeetAverage(uint8_t numSamples);
float GetDistanceFeet() ;

#endif


