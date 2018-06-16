#include <Servo.h>
#include <Metro.h>
//#define Serial Serial1
#include <ResponsiveAnalogRead.h>
#include "Motors.h"

#include <stdarg.h>
void p(char *fmt, ... ){
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.println(buf);
  Serial1.println(buf);
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
  Serial1.println(buf);
}

// #define STOP_BUTTON 1
// #define DOWN_BUTTON 0
// #define UP_BUTTON 2

#define DIST_IN_F 14
ResponsiveAnalogRead dist_f_reader(DIST_IN_F, true);
int dist_f = 0;
#define DIST_IN_L 16
ResponsiveAnalogRead dist_l_reader(DIST_IN_L, true);
int dist_l = 0;
#define DIST_IN_R 15
ResponsiveAnalogRead dist_r_reader(DIST_IN_R, true);
int dist_r = 0;

#define MOTOR_OUT_HEAD 9
#define MOTOR_OUT_TAIL 10

#define DEBUGGER_ATTACHED_PIN 33 // disabled for now

int speed_left = 0;
int left_zero = 0;
int left_fwd_max = 255;
int left_rw_max = -255;
int left_mapped = left_zero;

int speed_right = 0;
int right_zero = 0;
int right_fwd_max = 255;
int right_rw_max = -255;
int right_mapped = right_zero;

Servo head;
Servo tail;
ResponsiveAnalogRead head_dir(0, true, 0.00001);
int tail_dir = 0;
int tail_dir_dir = 1;
const int tail_speed = 30;

int speed = 0;
int dir = 0;
int more_space = 0;
int dir_sign = 0;

bool manual_control = true;
int debug_light_on = 1;
byte byteRead;

Metro debug_out = Metro(10000);

Motors motors = Motors();

void setup() {
  delay(1000);
  motors.Attach();

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  head.attach(MOTOR_OUT_HEAD);
  tail.attach(MOTOR_OUT_TAIL);

  pinMode(DEBUGGER_ATTACHED_PIN, INPUT_PULLUP);
  delay(100);
  manual_control = !digitalRead(DEBUGGER_ATTACHED_PIN);

  // pinMode(STOP_BUTTON, INPUT_PULLUP);
  // pinMode(DOWN_BUTTON, INPUT_PULLUP);
  // pinMode(UP_BUTTON, INPUT_PULLUP);

  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.setTimeout(100);
  Serial1.setTimeout(100);
}

void loop() {


  debug_light_on = 1 - debug_light_on;
  // digitalWrite(13, debug_light_on);
  // motors.SetSpeeds(tail_dir_dir * 64, tail_dir_dir * 64);
  // tail_dir_dir *= -1;
  // delay(1000);
  // return;

  dist_f_reader.update();
  dist_l_reader.update();
  dist_r_reader.update();
  dist_f = dist_f_reader.getValue(); // analogRead(DIST_IN_F);
  dist_l = dist_l_reader.getValue(); // analogRead(DIST_IN_L);
  dist_r = dist_r_reader.getValue(); // analogRead(DIST_IN_R);

  byteRead = 0;
  if (Serial1.available()) 
  {
    /* read the most recent byte */
    byteRead = Serial1.read();
    if (byteRead == 'a')
    {
      manual_control = !manual_control;
      debug_out.reset();
    }
    else if (byteRead == 'q')
    {
      print_debug();
    }
  }

  if (manual_control)
  {
    do
    {
      if (byteRead == 's')
      {
        long r = Serial1.parseInt();
        speed = r;
        Serial1.read();
      }
      else if (byteRead == 'd')
      {
        long r = Serial1.parseInt();
        dir = r;
        Serial1.read();
      }

      byteRead = Serial1.read();
    } while (Serial1.available());

    left_mapped = map_speed(
      map(dir, -90, 0, 0, 100), 
      map(speed, 0, 255, left_zero, left_rw_max),
      left_zero, 
      map(speed, 0, 255, left_zero, left_fwd_max));
    right_mapped = map_speed(
      map(dir, 0, 90, 100, 0),
      map(speed, 0, 255, right_zero, right_rw_max),
      right_zero,
      map(speed, 0, 255, right_zero, right_fwd_max));
  }
  else
  {
    dir = 0;

    more_space = dist_l - dist_r;

    if (more_space > 0)
      dir += map(more_space, 0, 300, 5, 180);
    else
      dir += map(abs(more_space), 0, 300, 5, -180);

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
    
    if (debug_out.check() == 1)
    {
      print_debug();
    }

  }

  motors.SetSpeeds(left_mapped, right_mapped);
  head_dir.update(90 - dir);
  head.write(head_dir.getValue());

  tail_dir += tail_dir_dir;
  tail.write(tail_dir / tail_speed);
  if (tail_dir >= 180 * tail_speed) {
    tail_dir_dir = -1;
  }
  else if (tail_dir <= 0) {
    tail_dir_dir = 1;
  }
}

int map_speed(int v, int mi, int zero, int ma)
{
  if (v < 0)
    return map(v, -100, 0, mi, zero);
  else
    return map(v, 0, 100, zero, ma);
}

void print_debug()
{
  p(F("MANUAL: %d, L %03d, F %03d, R %03d, dir %03d, speed %03d, left_mapped %03d, right_mapped %03d"),
    manual_control, dist_l, dist_f, dist_r, dir, speed, left_mapped, right_mapped);
}
