#include <Servo.h>
#include <Metro.h>
//#define Serial Serial1
#include <ResponsiveAnalogRead.h>

#include <i2c_t3.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055_Wire1.h>
#include <utility/imumaths.h>
#include <math.h>

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

#define DIST_IN_F 20
ResponsiveAnalogRead dist_f_reader(DIST_IN_F, true);
int dist_f = 0;
#define DIST_IN_L 19
ResponsiveAnalogRead dist_l_reader(DIST_IN_L, true);
int dist_l = 0;
#define DIST_IN_R 18
ResponsiveAnalogRead dist_r_reader(DIST_IN_R, true);
int dist_r = 0;

#define MOTOR_OUT_L 16
#define MOTOR_OUT_R 21
#define MOTOR_OUT_HEAD 17

#define DEBUGGER_ATTACHED_PIN 3

Servo wheel_left;
int speed_left = 0;
int left_zero = 92;
int left_fwd_max = 130;
int left_rw_max = 55;
int left_mapped = left_zero;

Servo wheel_right;
int speed_right = 0;
int right_zero = 92;
int right_fwd_max = 55;
int right_rw_max = 130;
int right_mapped = right_zero;

Servo head;
ResponsiveAnalogRead head_dir(0, true, 0.00001);

int speed = 0;
int dir = 0;
int more_space = 0;
int dir_sign = 0;

bool manual_control = true;
int debug_light_on = 1;
byte byteRead;

Metro debug_out = Metro(100);
Adafruit_BNO055 bno = Adafruit_BNO055();
double prevYaw = 0.0;
double targetYaw = 0.0;
long LongDelta = 0;

double ShortestRot(double From, double To)
{
  double arg = fmod(To - From, 360.0);
  if (arg < 0 )  arg  = arg + 360.0;
  if (arg > 180) arg  = arg - 360.0;
  return (-arg);
}

double DoubleMap(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  wheel_left.attach(MOTOR_OUT_L);
  wheel_right.attach(MOTOR_OUT_R);
  head.attach(MOTOR_OUT_HEAD);

  pinMode(DEBUGGER_ATTACHED_PIN, INPUT_PULLUP);
  delay(100);
  manual_control = !digitalRead(DEBUGGER_ATTACHED_PIN);

  // pinMode(STOP_BUTTON, INPUT_PULLUP);
  // pinMode(DOWN_BUTTON, INPUT_PULLUP);
  // pinMode(UP_BUTTON, INPUT_PULLUP);

  p(F("Startin BNO055... "));
  while (!bno.begin())
  {
    p(F("Fail "));
    delay(100);
  }
  p(F("Done!"));

  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.setTimeout(100);
  Serial1.setTimeout(100);
}

void loop() {
  debug_light_on = 1 - debug_light_on;
  digitalWrite(13, debug_light_on);

  // dist_f_reader.update();
  // dist_l_reader.update();
  // dist_r_reader.update();
  dist_f = analogRead(DIST_IN_F);
  dist_l = analogRead(DIST_IN_L);
  dist_r = analogRead(DIST_IN_R);

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
      double r = Serial1.parseFloat();
      dir = (int)r;
      targetYaw = r;
      Serial1.read();
    }

    byteRead = Serial1.read();
  } while (Serial1.available());

  // left_mapped = map_speed(
  //   map(dir, -90, 0, 0, 100), 
  //   map(speed, 0, 255, left_zero, left_rw_max),
  //   left_zero, 
  //   map(speed, 0, 255, left_zero, left_fwd_max));
  // right_mapped = map_speed(
  //   map(dir, 0, 90, 100, 0),
  //   map(speed, 0, 255, right_zero, right_rw_max),
  //   right_zero,
  //   map(speed, 0, 255, right_zero, right_fwd_max));

  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  //const double Yaw = euler.x() * PI / 180.0;

  double Delta = ShortestRot(euler.x(), targetYaw);
  LongDelta = (long)(Delta * 1000.0);

  left_mapped = map_speed(
    map(LongDelta, -180000, 180000, 500, -500),
    left_rw_max, left_zero, left_fwd_max);
  right_mapped = map_speed(
    map(LongDelta, -180000, 180000, -500, 500),
    right_rw_max, right_zero, right_fwd_max);

  prevYaw = euler.x();
  if (debug_out.check() == 1)
  {
    print_debug();
  }

  wheel_right.write(right_mapped);
  wheel_left.write(left_mapped);
  head_dir.update(90 - dir);
  head.write(head_dir.getValue());
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
  p(F("MANUAL: %d, L %03d, F %03d, R %03d, dir %03d, speed %03d, left_mapped %03d, right_mapped %03d, Yaw: %03d, delta: %d"),
    manual_control, dist_l, dist_f, dist_r, dir, speed, left_mapped, right_mapped, (int)prevYaw, LongDelta);
}
