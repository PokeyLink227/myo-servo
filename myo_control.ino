#include <Servo.h>

// Variables to control init and main loop timings
const unsigned long LOG_RATE = 50;    // log sensors every X ms
const bool WAIT_FOR_SERIAL = true;

// Pin control variables
const int BUTTON_INCREASE_PIN = 6;
const int BUTTON_DECREASE_PIN = 3;
const int THUMB_PIN = 9;
const int FINGER_1_PIN = 10;
const int FINGER_2_PIN = 11;
Servo thumb;   // Servo for thumb
Servo finger_1; // Servo for first finger
Servo finger_2; // Servo for second finger

int buttonState = HIGH;      // Current button state
int lastButtonState = HIGH;  // Previous button state

/* angle variables and functions
 * track the current angle globaly to enable non-blocking movement
*/
const unsigned long SERVO_UPDATE_RATE = 5;  // update servos every X ms
const int WIGGLE_THRESHOLD = 5; // defines the distance in degrees the target must change before we update servos
const int MAX_SPEED = 4; // defines the max amount of degrees the servos will move per update
const int MAX_ANGLE = 90; // defines the angle of the servos needed to close the hand
const int SPEED_SLOPE = MAX_SPEED / (MAX_ANGLE - WIGGLE_THRESHOLD);
int current_angle;
int target_angle;
void calculate_target(); // calculate the new target angle nased on history of sensor readings
void set_servo_target(int); // set a new target angle for the servos to move to
void update_current_angle(); // calculate the new angle to send to the servos based on distance from target angle
void update_servos(); // send new current angle to servos

// history varibles and functions
const int HISTORY_LENGTH = 25;
float history[HISTORY_LENGTH];
void history_add(float);
float history_avg();

void setup() {

  Serial.begin(115200);
  if (WAIT_FOR_SERIAL) while (!Serial); // optionally wait for serial terminal to open
  Serial.println("MyoWare Example_01_analogRead_SINGLE");

  thumb.attach(THUMB_PIN);   // Attach thumb servo to pin 2
  finger_1.attach(FINGER_1_PIN); // Attach first finger servo to pin 3
  finger_2.attach(FINGER_2_PIN); // Attach second finger servo to pin 4

  pinMode(BUTTON_INCREASE_PIN, INPUT_PULLUP);  // Enable internal pull-up resistor
  pinMode(BUTTON_DECREASE_PIN, INPUT_PULLUP);  // Enable internal pull-up resistor

  // Initialize all servos at 0 degrees
  thumb.write(0);
  finger_1.write(0);
  finger_2.write(0);

  current_angle = 0;
  target_angle = 0;
}

void loop()
{
  static unsigned long next_log = 0, next_update = 0;

  // store current time so it is consistent throughout the call
  unsigned long current_time = millis();

  history_add(analogRead(A0));
  calculate_target();

  // if (digitalRead(buttonIncreasePin) == HIGH && target_angle < MAX_ANGLE) {
  //   target_angle++;
  // }
  // if (digitalRead(buttonDecreasePin) == HIGH && target_angle > 0) {
  //   target_angle--;
  // }

  //if (current_time > next_update) {
    //update_log = current_time + UPDATE_RATE;
    update_servos(); // needs to be called every iteration
  //}

  if (current_time > next_log) {
    next_log = current_time + LOG_RATE;
    // message format [0 250 XXXX\n] 11 bytes total
    Serial.print(0); // To freeze the lower limit
    Serial.print(" ");
    Serial.print(250); // To freeze the upper limit
    Serial.print(" ");
    //Serial.print("%04.4i\n", history_avg());
    Serial.println(history_avg());
    // Serial.print(target_angle);
    // Serial.print(" ");
    // Serial.println(current_angle);
    //Serial.print("\n");
  }

  delay(5);
}

void calculate_target() {
  set_servo_target(map(history_avg(), 0, 1024, 0, 90));
}

void set_servo_target(int angle) {
  target_angle = angle;
}

void update_current_angle() {
  int diff = target_angle - current_angle;
  diff = abs(diff);

  int speed = 1;
  // TODO: check if wiggle threshold works
  //
  //if (diff > WIGGLE_THRESHOLD) speed = SPEED_SLOPE * (diff - WIGGLE_THRESHOLD);
  if (speed > MAX_SPEED) speed = MAX_SPEED;

  if (target_angle > current_angle) {
    current_angle += speed;
  } else {
    current_angle -= speed;
  }
}

void update_servos() {
  static unsigned long target_time = 0;

  // This if statement ensures a signal is only sent to the servos on some interval
  if (current_angle != target_angle && millis() >= target_time) {
    target_time = millis() + SERVO_UPDATE_RATE;

    update_current_angle();
    thumb.write(current_angle);
    finger_1.write(current_angle);
    finger_2.write(current_angle);
  }
}

/*
* Reading History related function
*/

void history_add(float new_reading) {
  static int index = 0;
  history[index] = new_reading;
  index = (index + 1) % HISTORY_LENGTH;
}

// this always takes the entire table into account so it will start skewed
float history_avg() {
  float sum = 0;
  for (int i = 0; i < HISTORY_LENGTH; i++) sum += history[i];
  return sum / HISTORY_LENGTH;
}
