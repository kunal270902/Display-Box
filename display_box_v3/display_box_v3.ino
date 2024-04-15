#include <AccelStepper.h>
#include <MultiStepper.h>

// Upper TMC to RIGHT motor
#define motor_L_MS1 8
#define motor_L_MS2 9
#define motor_L_PDN 10
#define motor_L_STEP 11
#define motor_L_DIR 12

// Lower TMC to LEFT motor
#define motor_R_MS1 2
#define motor_R_MS2 3
#define motor_R_PDN 4
#define motor_R_STEP 5
#define motor_R_DIR 6

// Button inputs
#define button_MNL 41
#define led_MNL 43
#define button_ATM 37
#define led_ATM 39

// LS inputs
#define limit_UP_SW 35
#define limit_DWN_SW 33

// Relay control for EMs
#define relay_EM01 15
#define relay_EM02 16
#define relay_EM03 17
#define relay_EM04 14

AccelStepper stepperR(AccelStepper::DRIVER, motor_R_STEP, motor_R_DIR);
AccelStepper stepperL(AccelStepper::DRIVER, motor_L_STEP, motor_L_DIR);
MultiStepper steppers;
bool sw=0;
bool flag=0;
int stepsPerRev = 200;
int microSteps = 8;
float moveUpDown = stepsPerRev * microSteps * 9;
float moveStart = stepsPerRev * microSteps * 0.5;
float motorMaxSpeed = 1500.0;
float motorMaxAccel = 300.0;

struct ButtonStates {
  bool manualButton;
  bool autoButton;
} buttonStates;

enum State { IDLE,
             START,
             MOVE_UP,
             DROP,
             MOVE_DOWN } currentState = IDLE;

void motorUpdateStates(long position) {
  long positions[2] = { position, -position };
  steppers.moveTo(positions);
}

void motorPDN(bool control) {
  digitalWrite(motor_R_PDN, control);
  digitalWrite(motor_L_PDN, control);
}

void readButtonStates() {
  buttonStates.manualButton = digitalRead(button_MNL);
  buttonStates.autoButton = digitalRead(button_ATM);
}


void emController(bool control) {
  digitalWrite(relay_EM01, !control);
  digitalWrite(relay_EM02, !control);
  digitalWrite(relay_EM03, !control);
}

void setup() {
  Serial.begin(9600);
  pinMode(motor_R_MS1, OUTPUT);
  pinMode(motor_R_MS2, OUTPUT);
  pinMode(motor_R_PDN, OUTPUT);

  pinMode(motor_L_MS1, OUTPUT);
  pinMode(motor_L_MS2, OUTPUT);
  pinMode(motor_L_PDN, OUTPUT);

  stepperR.setMaxSpeed(motorMaxSpeed);
  stepperR.setAcceleration(motorMaxAccel);
  digitalWrite(motor_R_MS1, LOW);
  digitalWrite(motor_R_MS2, LOW);

  stepperL.setMaxSpeed(motorMaxSpeed);
  stepperL.setAcceleration(motorMaxAccel);
  digitalWrite(motor_L_MS1, LOW);
  digitalWrite(motor_L_MS2, LOW);
  motorPDN(LOW);

  steppers.addStepper(stepperR);
  steppers.addStepper(stepperL);

  pinMode(limit_UP_SW, INPUT_PULLUP);
  pinMode(limit_DWN_SW, INPUT_PULLUP);

  pinMode(button_MNL, INPUT_PULLUP);
  pinMode(button_ATM, INPUT_PULLUP);
  pinMode(led_MNL, OUTPUT);
  pinMode(led_ATM, OUTPUT);

  pinMode(relay_EM01, OUTPUT);
  pinMode(relay_EM02, OUTPUT);
  pinMode(relay_EM03, OUTPUT);

  emController(LOW);

}

void loop() {

  digitalWrite(led_MNL,LOW);
  readButtonStates();
  digitalWrite(led_MNL,LOW);
  switch (currentState) {
    case IDLE:
    motorPDN(LOW);
          if(buttonStates.autoButton == LOW)
      {
      digitalWrite(led_ATM,HIGH);     
      }
    else
       {
     digitalWrite(led_ATM,LOW);
       }
     flag==0;
     digitalWrite(led_MNL,LOW);
   motorPDN(HIGH);

   if(digitalRead(limit_DWN_SW)){
   while(digitalRead(limit_DWN_SW))
 {
     motorUpdateStates(-2000000);
     steppers.run(); 
     //break;   
    stepperL.setCurrentPosition(0);
    stepperR.setCurrentPosition(0);
 }
}
      if (buttonStates.manualButton == LOW || buttonStates.autoButton == LOW) {       
        currentState = START;
        
       delay(1000);
        if(buttonStates.manualButton==LOW)
        {
          digitalWrite(led_MNL,HIGH);
          Serial.println("IDLE");
          flag=1;
        }
         else 
        {
          digitalWrite(led_MNL,LOW);
        }
      }
      break;
    case START:
  
         if(flag==1)
        {
          digitalWrite(led_MNL,HIGH);
          Serial.println("START");
        }
         else 
        {
          digitalWrite(led_MNL,LOW);
        }
      motorPDN(HIGH);
      emController(HIGH);
      if(!digitalRead(limit_DWN_SW)){
      motorPDN(HIGH);
      emController(HIGH);
      motorUpdateStates(moveStart);
      steppers.runSpeedToPosition();
      motorUpdateStates(0);
      steppers.runSpeedToPosition();
      delay(1000);
      currentState = MOVE_UP;
      delay(1000);
      break;
      }
      delay(1000); 
      currentState = MOVE_UP;
      delay(1000);  
      break;
    case MOVE_UP:

        if(flag==1)
        {
          digitalWrite(led_MNL,HIGH);
          Serial.println("UP");
        }
        else 
        {
          digitalWrite(led_MNL,LOW);
        }
      motorUpdateStates(moveUpDown);
      while (steppers.run()) {
        if (!digitalRead(limit_UP_SW)) {
          currentState = DROP;
          break;
        }
      }
      currentState = DROP;
      break;
    case DROP:
        if(flag==1)
        {
          digitalWrite(led_MNL,HIGH);
          Serial.println("DROP");
        }
            
         else 
        {
          digitalWrite(led_MNL,LOW);
        }
      emController(LOW);
      delay(1500);
      currentState = MOVE_DOWN;
      break;
    case MOVE_DOWN:
         if(flag==1)
        {
          digitalWrite(led_MNL,HIGH);
          Serial.println("DOWN");
          flag=0;
        }
         else 
        {
          digitalWrite(led_MNL,LOW);
        }
      motorUpdateStates(0);
      while (steppers.run()) {
        if (!digitalRead(limit_DWN_SW)) {
          if(buttonStates.autoButton == LOW){
            currentState=IDLE;
            }
            break;
        }
      }
      currentState =  IDLE;
  }


}
