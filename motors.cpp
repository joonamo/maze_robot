#include "motors.h"
#include <Arduino.h>
#include <HardwareSerial.h>

void Motors::Attach() {
    pinMode(left_fwd, OUTPUT);
    pinMode(left_rev, OUTPUT);
    pinMode(right_fwd, OUTPUT);
    pinMode(right_rev, OUTPUT);
    pinMode(left_enable, OUTPUT);
    pinMode(right_enable, OUTPUT);

    Stop();
}

void Motors::Stop() {
    analogWrite(left_enable, 0);
    analogWrite(right_enable, 0);
}

void Motors::SetSpeeds(int left, int right) {
    Serial.print(left);
    Serial.print(", ");
    Serial.println(right);
    if (left == 0)
    {
        digitalWrite(left_enable, LOW);
    }
    else
    {
        if (left > 0) 
        {
            digitalWrite(left_fwd, LOW);
            digitalWrite(left_rev, HIGH);
        } 
        else 
        {
            digitalWrite(left_fwd, HIGH);
            digitalWrite(left_rev, LOW);
        }
        int v = abs(left) + MOTOR_MIN;
        if (v > 255) v = 255;
        analogWrite(left_enable, v);
    }

    if (right == 0)
    {
        digitalWrite(right_enable, LOW);
    }
    else
    {
        if (right > 0) 
        {
            digitalWrite(right_fwd, LOW);
            digitalWrite(right_rev, HIGH);
        } 
        else 
        {
            digitalWrite(right_fwd, HIGH);
            digitalWrite(right_rev, LOW);
        }
        int v = abs(right) + MOTOR_MIN;
        if (v > 255) v = 255;
        analogWrite(right_enable, v);
    }
}