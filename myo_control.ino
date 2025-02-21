#include <Servo.h>

const unsigned long SERVO_UPDATE_RATE = 5;  // update servos every X ms
const unsigned long LOG_RATE = 50;    // log sensors every X ms
const bool WAIT_FOR_SERIAL = true;

Servo Thumb;   // Servo for thumb
Servo Finger1; // Servo for first finger
Servo Finger2; // Servo for second finger


const int buttonPin = 7;  // Button connected to pin 7
const int THUMB_PIN = 2;
const int FINGER1_PIN = 3;
const int FINGER2_PIN = 4;

int buttonState = HIGH;      // Current button state
int lastButtonState = HIGH;  // Previous button state
bool moveInProgress = false; // Track if movement is in progress
bool servoPosition90 = false; // Track if servos are at 90 degrees

int angleMagnitude;

/* angle variables and functions
 * track the current angle globaly to enable non-blocking movement
*/
enum servo_state {
  Stopped,
  Moving,
};
int current_angle;
int target_angle;
void update_servos();
void move_servos_to(int);

// history varibles and functions
const int HISTORY_LENGTH = 16;
float history[HISTORY_LENGTH];
void history_add(float);
float history_avg();

void setup() {

  Serial.begin(115200);
  if (WAIT_FOR_SERIAL) while (!Serial); // optionally wait for serial terminal to open
  Serial.println("MyoWare Example_01_analogRead_SINGLE");

  Thumb.attach(THUMB_PIN);   // Attach thumb servo to pin 2
  Finger1.attach(FINGER1_PIN); // Attach first finger servo to pin 3
  Finger2.attach(FINGER2_PIN); // Attach second finger servo to pin 4

  pinMode(buttonPin, INPUT_PULLUP);  // Enable internal pull-up resistor

  // Initialize all servos at 0 degrees
  Thumb.write(0);
  Finger1.write(0);
  Finger2.write(0);

  current_angle = 0;
  target_angle = 0;
}

void loop()
{
  static unsigned long next_log = 0, next_update = 0;

  // store current time so it is consistent throughout the call
  unsigned long current_time = millis();

  history_add(analogRead(A0));
  if(history_avg() >= 1000) {
    move_servos_to(90);
  } else {
    move_servos_to(0);
  }



  //if (current_time > next_update) {
    //update_log = current_time + UPDATE_RATE;

    //move_servo_to(map(sensorValue, 0, 1000, 0, 90)); // equivelent to toggleGradient
    update_servos(); // needs to be called every iteration
  //}

  if (current_time > next_log) {
    next_log = current_time + LOG_RATE;

    Serial.print(0); // To freeze the lower limit
    Serial.print(" ");
    Serial.print(250); // To freeze the upper limit
    Serial.print(" ");
    Serial.println(history_avg());
  }
}

void update_servos() {
  static unsigned long target_time = 0;

  if (current_angle != target_angle && millis() >= target_time) {
    target_time = millis() + SERVO_UPDATE_RATE;

    if (target_angle > current_angle) {
      current_angle++;
    } else {
      current_angle--;
    }

    Thumb.write(current_angle);
    Finger1.write(current_angle);
    Finger2.write(current_angle);
  }
}

void move_servos_to(int angle) {
  target_angle = angle;
}

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

