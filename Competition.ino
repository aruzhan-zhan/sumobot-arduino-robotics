// Edge Sensors
#define leftIRPin A7   // They use analog pins.
#define rightIRPin A6  // 

// Ultrasonic Sensors
#define triggerPin 2   // Trigger uses digital pin.
#define echoPin 3     // Echo uses digital pin.

// Motors - Updated configuration
#define AIN1 4       // Left motor direction pin 1
#define AIN2 5       // Left motor direction pin 2
#define PWMA 9      // Left motor PWM pin
#define BIN1 6       // Right motor direction pin 1
#define BIN2 7      // Right motor direction pin 2
#define PWMB 10     // Right motor PWM pin

// Other constants
#define INITIAL_DELAY 3500       // Milliseconds. Standard delay for matches.
#define MAX_PULSE_DURATION 5600  // Microseconds.
#define SOUND_SPEED 0.0343       // Cm per microsecond.
#define COLOUR_THRESHOLD 350    // Threshold between white and black.

#define FORWARD 1                // Moving forward.
#define BACKWARDS 0              // Moving backward.
#define CLOCKWISE 1              // For spinning: clockwise.
#define COUNTERCLOCKWISE 0       // For spinning: counterclockwise.
int standardRotation = CLOCKWISE;  // Set based on opponent behavior.

// Variables for IR sensors
const byte IRArraySize = 2;
int leftIRArray[IRArraySize];
int rightIRArray[IRArraySize];
float leftIRAverage = 0;
float rightIRAverage = 0;

// Variables for ultrasonic sensor
const byte USArraySize = 2;
float USArray[USArraySize];
float USValuesSum = 0;

byte i = 0;  // Index for IR arrays.
byte j = 0;  // Index for US array.

void setup() {
  // Start initial delay countdown.
  long matchStart = millis();

  // Populate IR sensor arrays and calculate initial averages.
  for (byte k = 0; k < IRArraySize; k++) {
    leftIRArray[k] = analogRead(leftIRPin);
    rightIRArray[k] = analogRead(rightIRPin);
    leftIRAverage += leftIRArray[k];
    rightIRAverage += rightIRArray[k];
  }
  leftIRAverage /= IRArraySize;
  rightIRAverage /= IRArraySize;

  // Setup Ultrasonic sensor pins.
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // Populate US array.
  for (byte k = 0; k < USArraySize; k++) {
    USArray[k] = getOppDistance();
    USValuesSum += USArray[k];
  }

  // Setup Motor pins (new configuration).
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  // Wait for match start.
  while (millis() - matchStart <= INITIAL_DELAY) {
    // do nothing
  }

  // First moves – uncomment one of these if needed.
  // attackFromSide();
  // blindAttack();
  // extraDelay(1000);
}

void loop() {
  if (oppFound()) {
    attack();
  } else if (leftIROnEdge()) {
    // 1. Reverse to kill forward momentum and clear the front edge
    reverse();
    delay(500);
    
    // 2. Spin sharply away to stop backward travel and face the arena
    
    // 3. Set standard rotation for the search phase
    standardRotation = CLOCKWISE;
    
  } else if (rightIROnEdge()) {
    // 1. Reverse to kill forward momentum and clear the front edge
    reverse();
    delay(500);
    
    // 2. Spin sharply away to stop backward travel and face the arena

    
    // 3. Set standard rotation
    standardRotation = COUNTERCLOCKWISE;
    
  } else {
    revolutionSearch(200, 70);
  }
  updateIRValues();
}

// ---------- Movement Methods ----------

// Moves forward using the new motor configuration.
void attack() {
  // New forward configuration: 
  // Left: AIN1 HIGH, AIN2 LOW; Right: BIN1 LOW, BIN2 HIGH.
  move(FORWARD, 255, 255);
}

// Moves backward using the new motor configuration.
void reverse() {
  // New backward configuration: 
  // Left: AIN1 LOW, AIN2 HIGH; Right: BIN1 HIGH, BIN2 LOW.
  move(BACKWARDS, 255, 255);
}

// Stops the robot.
void stopBot() {
  move(FORWARD, 0, 0);
}

// Performs a revolution search.
void revolutionSearch(int highSpeed, int lowSpeed) {
  if (standardRotation == CLOCKWISE) {
    move(FORWARD, highSpeed, lowSpeed);
  } else {
    move(FORWARD, lowSpeed, highSpeed);
  }
}

// Spins in place to search for the opponent.
void spinSearch(int spinSpeed) {
  spin(standardRotation, spinSpeed);
}

// Moves the robot in the specified direction with given speeds.
void move(int direction, int leftSpeed, int rightSpeed) {
  setMotors(direction);
  analogWrite(PWMA, leftSpeed);
  analogWrite(PWMB, rightSpeed);
}

// Spins the robot in place.
void spin(int rotationDirection, int speed) {
  setMotorsSpin(rotationDirection);
  analogWrite(PWMA, speed);
  analogWrite(PWMB, speed);
}

// Sets motor directions for forward or backward motion.
void setMotors(int direction) {
  if (direction == FORWARD) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  } else { // BACKWARDS
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  }
}

// Sets motor directions for spinning.
void setMotorsSpin(int rotationDirection) {
  if (rotationDirection == CLOCKWISE) {
    // Clockwise spin: left motor forward, right motor backward.
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else {
    // Counterclockwise spin: left motor backward, right motor forward.
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  }
}

// ---------- Sensor Methods ----------

// Returns true if an object is detected within ~45 cm.
bool oppFound() {
  updateUSValues();
  
  // Calculate the actual average distance in cm
  float averageDistance = USValuesSum / USArraySize;
  
  // Check if the object is closer than 60cm, but greater than 0.1cm (to filter out noise/glitches)
  return (averageDistance > 0.1 && averageDistance < 60.0);
}

// Returns true if the left IR sensor detects an edge.
bool leftIROnEdge() {
  return leftIRAverage < COLOUR_THRESHOLD;
}

// Returns true if the right IR sensor detects an edge.
bool rightIROnEdge() {
  return rightIRAverage < COLOUR_THRESHOLD;
}

// Updates the sum of ultrasonic distance values.
void updateUSValues() {
  j = j % USArraySize;
  USValuesSum -= USArray[j];
  USArray[j] = getOppDistance();
  USValuesSum += USArray[j];
  j++;
}

// Updates the averages for the IR sensors.
void updateIRValues() {
  i = i % IRArraySize;
  
  leftIRAverage -= leftIRArray[i] / (float)IRArraySize;
  leftIRArray[i] = analogRead(leftIRPin);
  leftIRAverage += leftIRArray[i] / (float)IRArraySize;
  
  rightIRAverage -= rightIRArray[i] / (float)IRArraySize;
  rightIRArray[i] = analogRead(rightIRPin);
  rightIRAverage += rightIRArray[i] / (float)IRArraySize;
  
  i++;
}

// Returns the distance in centimeters from the ultrasonic sensor.
float getOppDistance() {
  unsigned long pulseDuration;
  float distance;
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pulseDuration = pulseIn(echoPin, HIGH, MAX_PULSE_DURATION);
  distance = pulseDuration * SOUND_SPEED * 0.5;
  return distance;
}

// ---------- Additional Movement Methods ----------

// Turns the bot to one side then reverses before attacking from the opposite side.
void attackFromSide() {
  standardRotation = !standardRotation;
  while (oppFound()) {
    revolutionSearch(255, 0);
  }
  standardRotation = !standardRotation;
  move(FORWARD, 255, 255);
  long timer = millis();
  while (!oppFound() && (millis() - timer < 300)) {
    // Do nothing
  }
  timer = millis();
  spinSearch(255);
  while (!oppFound() && (millis() - timer < 500)) {
    // Do nothing
  }
}

// Continuously attacks (use with caution).
void blindAttack() {
  while (true) {
    attack();
  }
}

// Extra delay before executing further moves.
void extraDelay(int delayAmount) {
  delay(delayAmount);
}