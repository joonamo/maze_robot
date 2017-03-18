#include <Servo.h>
#include <Metro.h>
#define Serial Serial1

#include <stdarg.h>
void p(char *fmt, ... ){
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.println(buf);
}

void p(const __FlashStringHelper *fmt, ... ){
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
#endif
  va_end(args);
  Serial.println(buf);
}

// #define STOP_BUTTON 1
// #define DOWN_BUTTON 0
// #define UP_BUTTON 2

#define DIST_IN_F 20
int dist_f = 0;
#define DIST_IN_L 19
int dist_l = 0;
#define DIST_IN_R 18
int dist_r = 0;

#define MOTOR_OUT_L 16
#define MOTOR_OUT_R 21

#define DEBUGGER_ATTACHED_PIN 3

Servo wheel_left;
int speed_left = 0;
int left_zero = 93;
int left_fwd_max = 130;
int left_rw_max = 55;
int left_mapped;

Servo wheel_right;
int speed_right = 0;
int right_zero = 93;
int right_fwd_max = 55;
int right_rw_max = 130;
int right_mapped;

int speed = 0;
int dir = 0;
int more_space = 0;
int dir_sign = 0;

bool manual_control = true;
byte byteRead;

Metro debug_out = Metro(100);

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  wheel_left.attach(MOTOR_OUT_L);
  wheel_right.attach(MOTOR_OUT_R);

  pinMode(DEBUGGER_ATTACHED_PIN, INPUT_PULLUP);
  manual_control = digitalRead(DEBUGGER_ATTACHED_PIN) == LOW;

  // pinMode(STOP_BUTTON, INPUT_PULLUP);
  // pinMode(DOWN_BUTTON, INPUT_PULLUP);
  // pinMode(UP_BUTTON, INPUT_PULLUP);

  Serial.begin(115200);
}

void loop() {
  dist_f = analogRead(DIST_IN_F);
  dist_l = analogRead(DIST_IN_L);
  dist_r = analogRead(DIST_IN_R);
  dir = 0;

  if (Serial.available()) 
  {
    /* read the most recent byte */
    byteRead = Serial.read();
    if (byteRead == 'a')
    {
      manual_control = !manual_control;
    }
  }

  if (manual_control)
  {
    left_mapped = left_zero;
    right_mapped = right_zero;
  }
  else
  {
    more_space = dist_l - dist_r;

    if (more_space > 0)
      dir += map(more_space, 0, 300, 5, 120);
    else
      dir += map(abs(more_space), 0, 300, 5, -120);

    if (dir >= 0)
      dir_sign = 1;
    else
      dir_sign = -1;

    dir = map(dist_f, 50, 400, dir, dir_sign * 180);

    left_mapped = map_speed(
      map(dir, -90, 0, 0, 100), 
      left_rw_max, left_zero, left_fwd_max);
    right_mapped = map_speed(
      map(dir, 0, 90, 100, 0),
      right_rw_max, right_zero, right_fwd_max);
  }

  if (debug_out.check() == 1)
  {
    if (manual_control)
    {
      Serial.write("MANUAL ");
    }
    else
    {
      Serial.write("BOT ");
    }
    p(F("L %03d, F %03d, R %03d, dir %03d, left_mapped %03d, right_mapped %03d"),
      dist_l, dist_f, dist_r, dir, left_mapped, right_mapped);
  }

  wheel_right.write(right_mapped);
  wheel_left.write(left_mapped);

}

int map_speed(int v, int mi, int zero, int ma)
{
  if (v < 0)
    return map(v, -100, 0, mi, zero);
  else
    return map(v, 0, 100, zero, ma);
}