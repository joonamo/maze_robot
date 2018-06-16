#pragma once

#define MOTOR_MIN 50

class Motors {
public:
    static const int left_fwd = 5;
    static const int left_rev = 6;
    static const int right_fwd = 3;
    static const int right_rev = 4;

    static const int left_enable = 23;
    static const int right_enable = 22;

    void Attach();
    void Stop();
    // Ranges -255 ... 255
    void SetSpeeds(int left, int right);
};