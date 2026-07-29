#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>

// Change these three pins to suit the final RC-1 wiring.
// Check the display board schematic before connecting the driver.
#define MOTOR_PWM_PIN 18
#define MOTOR_DIR_PIN 17
#define MOTOR_ENABLE_PIN 16

#define MOTOR_PWM_FREQUENCY_HZ 20000
#define MOTOR_PWM_RESOLUTION_BITS 10
#define MOTOR_RAMP_TIME_MS 1500

void motorControlBegin();
void motorControlSetDirection(bool forward);
void motorControlSetTargetPercent(float percent);
void motorControlStart();
void motorControlStop();
void motorControlEmergencyStop();
void motorControlUpdate();

bool motorControlIsRunning();
bool motorControlIsForward();
float motorControlGetTargetPercent();
float motorControlGetActualPercent();

#endif
