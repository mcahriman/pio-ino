#include <Servo.h>

#define LED_G 2
#define LED_B 3
#define LED_R 4

#define SERVO_B_PIN 6
#define SERVO_T_PIN 5
#define LASER_PIN 7

#define JOY_PIN_COMMON 26
#define JOY_PIN_UP 25
#define JOY_PIN_DOWN 22
#define JOY_PIN_LEFT 27
#define JOY_PIN_RIGHT 24
#define JOY_PIN_BTN 23

#define JOY_DOWN 1
#define JOY_UP 2
#define JOY_LEFT 3
#define JOY_RIGHT 4
#define JOY_BTN 5

#define LASER_DEBOUNCE_TIME 300

const byte numChars = 32;
char receivedChars[numChars]; 
char responseBuf[numChars];
boolean newData = false;


char joy_status=0;
Servo servomotor_b;
Servo servomotor_t;

unsigned int servo_b_pos=0, servo_t_pos=0;
unsigned long btn_laser_debounce=0;
boolean lasers_on = false;



unsigned char r=1,g=1,b=1;
// the setup routine runs once when you press reset:
void setup() {
  pinMode(JOY_PIN_COMMON,INPUT_PULLUP);
  pinMode(JOY_PIN_UP,INPUT_PULLUP);
  pinMode(JOY_PIN_DOWN,INPUT_PULLUP);
  pinMode(JOY_PIN_LEFT,INPUT_PULLUP);
  pinMode(JOY_PIN_RIGHT,INPUT_PULLUP);
  pinMode(JOY_PIN_BTN,INPUT_PULLUP);
  pinMode(JOY_PIN_COMMON, OUTPUT);
  pinMode(LASER_PIN, OUTPUT);
  
  // initialize the digital pin as an output.
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  setColourRgb(0,0,0);
  digitalWrite(JOY_PIN_COMMON,LOW);
  digitalWrite(LASER_PIN,LOW);
  
  servomotor_b.attach(SERVO_B_PIN);
  servomotor_t.attach(SERVO_T_PIN);
  servomotor_b.write(90);
  servomotor_t.write(0);
  
  Serial.begin(9600);
  Serial.println("ROBOLASER 2017 is up and running\n");
}

const int redPin = LED_R;
const int greenPin = LED_G;
const int bluePin = LED_B;

void loop() {
  

  query_joystick();
  setColourRgb(r,g,b);
  delay(20);
  
}

void query_joystick() {
  joy_status = ( !digitalRead(JOY_PIN_UP) << JOY_UP ) |
              ( !digitalRead(JOY_PIN_DOWN) << JOY_DOWN ) |
              ( !digitalRead(JOY_PIN_LEFT) << JOY_LEFT ) |
              ( !digitalRead(JOY_PIN_RIGHT) << JOY_RIGHT ) |
              ( !digitalRead(JOY_PIN_BTN) << JOY_BTN );
              
  switch (joy_status & ~_BV(JOY_BTN)) {
    case _BV(JOY_LEFT):
      decreaseServoAngle(servomotor_b);
      break;
    case _BV(JOY_LEFT) | _BV(JOY_UP):
      increaseColour(r);
      
      decreaseServoAngle(servomotor_b);
      decreaseServoAngle(servomotor_t);
      break;
    case _BV(JOY_UP):
      decreaseServoAngle(servomotor_t);
      increaseColour(g);
      break;
    case _BV(JOY_UP) | _BV(JOY_RIGHT):
      increaseColour(b);
      increaseServoAngle(servomotor_b);
      decreaseServoAngle(servomotor_t);
      break;
    case _BV(JOY_RIGHT):
      increaseServoAngle(servomotor_b);
      break;
    case _BV(JOY_DOWN) | _BV(JOY_LEFT):
      decreaseServoAngle(servomotor_b);
      increaseServoAngle(servomotor_t);
      decreaseColour(r);
      break;
    case _BV(JOY_DOWN):
      decreaseColour(g);
      increaseServoAngle(servomotor_t);
      break;
    case _BV(JOY_RIGHT) | _BV(JOY_DOWN):
      decreaseColour(b);
      increaseServoAngle(servomotor_t);
      increaseServoAngle(servomotor_b);
      
      break;
  }
  
  if(joy_status & _BV(JOY_BTN)) {
    decreaseColour(b);
    decreaseColour(r);
    decreaseColour(g);
    
    if ( millis() - btn_laser_debounce > LASER_DEBOUNCE_TIME) {
      btn_laser_debounce = millis();
      lasers_on = !lasers_on;
      if (lasers_on) {
        analogWrite(LASER_PIN, 200);
      } else {
        analogWrite(LASER_PIN, 0);
      }
    }    
    
  }
  
  recvWithEndMarker();
  processData();
}

void increaseColour(unsigned char &c) {
  if(c<255) c++;
}


void decreaseColour(unsigned char &c) {
  if(c>0) c--;
}

void increaseServoAngle(Servo servo) {
  int servo_pos = servo.read();
  if(servo_pos<180) servo.write(servo_pos + 1);
}

void decreaseServoAngle(Servo servo) {
  int servo_pos = servo.read();
  if(servo_pos>1) servo.write(servo_pos - 1);
}

void setServoAngle(Servo servo, int servo_pos) {
  servo.write(servo_pos)  
}

void setColourRgb(unsigned char red, unsigned char green, unsigned char blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
 }
 
 
void recvWithEndMarker() {
 static byte ndx = 0;
 char endMarker = '\n';
 char rc;
 
 // if (Serial.available() > 0) {
 while (Serial.available() > 0 && newData == false) {
 rc = Serial.read();

 if (rc != endMarker) {
 receivedChars[ndx] = rc;
 ndx++;
 if (ndx >= numChars) {
 ndx = numChars - 1;
 }
 }
 else {
 receivedChars[ndx] = '\0'; // terminate the string
 ndx = 0;
 newData = true;
 }
 }
}

void processData() {
 int angleX;
 int angleY;
 if (newData == true) {
   if(sscanf(receivedChars, "T%i_%i", &angleX, &angleY) == 2) {
     sprintf(responseBuf, "ROLL: %i, YAW: %i\n", angleX, angleY);
     Serial.write(responseBuf);
     servomotor_b.setServoAngle($angleX);
     servomotor_t.setServoAngle($angleY);
   }
   newData = false;
 }
}
