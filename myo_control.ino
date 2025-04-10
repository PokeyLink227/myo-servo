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
int current_angle;
int target_angle;
void calculate_target(); // calculate the new target angle nased on history of sensor readings
void update_servos(); // send new angles to servos
void calc_new_angle(); // calculate the new angle to send to the servos based on distance from target angle
void move_servos_to(int); // set a new target angle for the servos to move to
const int WIGGLE_THRESHOLD = 5; // defines the distance in degrees the target must change before we update servos
const int MAX_SPEED = 4; // defines the max amount of degrees the servos will move per update
const int MAX_ANGLE = 90; // defines the angle of the servos needed to close the hand
const int SPEED_SLOPE = MAX_SPEED / (MAX_ANGLE - WIGGLE_THRESHOLD);

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
  calculate_target();

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
    //erial.print("%04.4i\n", history_avg());
    Serial.println(history_avg());
    //Serial.print("\n");
  }
}

void calculate_target() {
  move_servos_to(map(history_avg(), 0, 1024, 0, 90));
}

void calc_new_angle() {
  int diff = target_angle - current_angle;
  diff = abs(diff);
  
  int speed = 0;
  if (diff > WIGGLE_THRESHOLD) speed = SPEED_SLOPE * (diff - WIGGLE_THRESHOLD);
  if (speed > MAX_SPEED) speed = MAX_SPEED;

  if (target_angle > current_angle) {
    current_angle += speed;
  } else {
    current_angle -= speed;
  }
}

void update_servos() {
  static unsigned long target_time = 0;

  if (current_angle != target_angle && millis() >= target_time) {
    target_time = millis() + SERVO_UPDATE_RATE;
    calc_new_angle();

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

