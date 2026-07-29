#include <Arduino.h>
#include "motor_control.h"

static const uint8_t PWM_CHANNEL = 0;
static const uint32_t PWM_MAX_DUTY = (1UL << MOTOR_PWM_RESOLUTION_BITS) - 1UL;

static bool running = false;
static bool forwardDirection = true;
static float targetPercent = 25.0f;
static float actualPercent = 0.0f;
static uint32_t lastUpdateMs = 0;

static float clampPercent(float value)
{
    if (value < 0.0f)
        return 0.0f;
    if (value > 100.0f)
        return 100.0f;
    return value;
}

static void writeOutput(float percent)
{
    percent = clampPercent(percent);
    uint32_t duty = static_cast<uint32_t>((percent / 100.0f) * PWM_MAX_DUTY + 0.5f);
    ledcWrite(PWM_CHANNEL, duty);
}

void motorControlBegin()
{
    pinMode(MOTOR_DIR_PIN, OUTPUT);
    pinMode(MOTOR_ENABLE_PIN, OUTPUT);

    digitalWrite(MOTOR_ENABLE_PIN, LOW);
    digitalWrite(MOTOR_DIR_PIN, forwardDirection ? HIGH : LOW);

    ledcSetup(PWM_CHANNEL, MOTOR_PWM_FREQUENCY_HZ, MOTOR_PWM_RESOLUTION_BITS);
    ledcAttachPin(MOTOR_PWM_PIN, PWM_CHANNEL);
    writeOutput(0.0f);

    running = false;
    actualPercent = 0.0f;
    lastUpdateMs = millis();
}

void motorControlSetDirection(bool forward)
{
    if (running || actualPercent > 0.0f)
        return;

    forwardDirection = forward;
    digitalWrite(MOTOR_DIR_PIN, forwardDirection ? HIGH : LOW);
}

void motorControlSetTargetPercent(float percent)
{
    targetPercent = clampPercent(percent);
}

void motorControlStart()
{
    if (targetPercent <= 0.0f)
        return;

    digitalWrite(MOTOR_DIR_PIN, forwardDirection ? HIGH : LOW);
    digitalWrite(MOTOR_ENABLE_PIN, HIGH);
    running = true;
    lastUpdateMs = millis();
}

void motorControlStop()
{
    running = false;
    lastUpdateMs = millis();
}

void motorControlEmergencyStop()
{
    running = false;
    actualPercent = 0.0f;
    writeOutput(0.0f);
    digitalWrite(MOTOR_ENABLE_PIN, LOW);
}

void motorControlUpdate()
{
    uint32_t now = millis();
    uint32_t elapsed = now - lastUpdateMs;

    if (elapsed == 0)
        return;

    lastUpdateMs = now;

    float destination = running ? targetPercent : 0.0f;
    float rampPerMs = 100.0f / static_cast<float>(MOTOR_RAMP_TIME_MS);
    float maximumStep = rampPerMs * static_cast<float>(elapsed);

    if (actualPercent < destination)
    {
        actualPercent += maximumStep;
        if (actualPercent > destination)
            actualPercent = destination;
    }
    else if (actualPercent > destination)
    {
        actualPercent -= maximumStep;
        if (actualPercent < destination)
            actualPercent = destination;
    }

    writeOutput(actualPercent);

    if (!running && actualPercent <= 0.0f)
    {
        actualPercent = 0.0f;
        writeOutput(0.0f);
        digitalWrite(MOTOR_ENABLE_PIN, LOW);
    }
}

bool motorControlIsRunning()
{
    return running;
}

bool motorControlIsForward()
{
    return forwardDirection;
}

float motorControlGetTargetPercent()
{
    return targetPercent;
}

float motorControlGetActualPercent()
{
    return actualPercent;
}
