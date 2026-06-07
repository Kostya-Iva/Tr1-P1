#include <AccelStepper.h>

// Define motor interface type
#define motorInterfaceType 1

#define dirPin1 23
#define dirPin2 19
#define dirPin3 16

#define stepPin1 22
#define stepPin2 18
#define stepPin3 17

#define kran_x 35
#define kran_y 32
#define kran_z_1 33
#define kran_z_2 25

#define STARTbutton 14
#define STOPbutton 12
#define LED 13

AccelStepper stepper1(motorInterfaceType, stepPin1, dirPin1);
AccelStepper stepper2(motorInterfaceType, stepPin2, dirPin2);
AccelStepper stepper3(motorInterfaceType, stepPin3, dirPin3);

enum State {
  STATE_IDLE,               // Await for first press of a START 
  STATE_ZERO,               // Zero position
  STATE_HOME,               // Go to the starting position
  STATE_WAIT,               // Await for the second press of a START
  STATE_START,              // Puts end-effector to working position
  STATE_MAIN_MOVEMENT       // Activate programmed trajectory (circle)
};

State currentState = STATE_IDLE;

bool is_X_true = false;
bool is_Y_true = false;
bool is_Z_true = false;


//Parametry opóźnienia DEL
unsigned long previousMillis_WAIT = 0;
const long interval_WAIT = 1000;
const long interval_BUTTON_PRESSED = 100;
int ledState = HIGH; 
// Parametry ruchu
int predkosc_x = 150;
int predkosc_y = 150;
int kroki_x = 100;
int kroki_y = 100;
// Czas pracy, 1 cykl koła ~ 4200 ms
// Ustawiamy długość procesu w min.
int minuty = 1;
int wymagana_liczba = round((minuty*60*1000)/4200);
const int n = 100; 
long cel_x[n];
long cel_y[n];

void setup() {
  // Inicjalizacja Monitora Szeregowego (standardowa prędkość dla ESP32)
  //Serial.begin(115200);
  
  // Konfiguracja krańcówek i przycisków jako wejścia z Pull-upem
  pinMode(kran_x, INPUT_PULLUP);
  pinMode(kran_y, INPUT_PULLUP);
  pinMode(kran_z_1, INPUT_PULLUP);
  pinMode(kran_z_2, INPUT_PULLUP);
  pinMode(STARTbutton, INPUT_PULLUP);
  pinMode(STOPbutton, INPUT_PULLUP);

  stepper1.setMaxSpeed(1000);
  stepper1.setAcceleration(240);
  stepper2.setMaxSpeed(1000);
  stepper2.setAcceleration(240);

  stepper3.setMaxSpeed(1000);
  stepper3.setAcceleration(120);
  stepper3.setMinPulseWidth(3);

  pinMode(LED, OUTPUT);

/* UNUSED, it is a first iteration of creating desired trajectory
  float resolution = (2.0*PI)/n; 
  int promien = kroki_x;
  for (int i = 0; i < n; i++){
  float kat = i * resolution; 
  cel_x[i] = round(promien * cos(kat));
  cel_y[i] = round(promien * sin(kat));
*/
}

  Serial.println("IDLE");
}

void loop() {
  unsigned long currentMillis_WAIT = millis();
  
  //UNUSED
  //int valueSTART = digitalRead(STARTbutton);
  //int valueSTOP = digitalRead(STOPbutton);
  
  int value_x = digitalRead(kran_x);
  int value_y = digitalRead(kran_y);
  int value_z_1 = digitalRead(kran_z_1); // DOLNA
  int value_z_2 = digitalRead(kran_z_2); // GÓRNA

  digitalWrite(LED, ledState);

  switch (currentState) {
    case STATE_IDLE:

      stepper1.setSpeed(0);
      stepper2.setSpeed(0);
      stepper3.setSpeed(0);
      ledState = HIGH;
      if (digitalRead(STARTbutton) == LOW) {

        for (int i = 0; i < 3; i++)
        {
          delay(interval_BUTTON_PRESSED);
          ledState = (ledState == LOW) ? HIGH : LOW;
          digitalWrite(LED, ledState);
        }
        //Serial.println("Initializing...\n");
        ledState = LOW;
        is_X_true = false;
        is_Y_true = false;
        is_Z_true = false;
        
        // Speed for zero-ing position
        stepper1.setSpeed(200);
        stepper2.setSpeed(200);
        stepper3.setSpeed(-30); 

        currentState = STATE_ZERO;
        delay(500);
      }
      break;
    

    case STATE_ZERO:

      if (digitalRead(STOPbutton) == LOW)
        currentState = STATE_IDLE;

      if (!is_X_true) {
        if (value_x == LOW) {
          //Serial.println("X is zero\n");
          stepper1.setCurrentPosition(0);
          is_X_true = true;
        } else {
          stepper1.runSpeed();
        }
      }

      if (!is_Y_true) {
        if (value_y == LOW) {
          //Serial.println("Y is zero\n");
          stepper2.setCurrentPosition(0);
          is_Y_true = true;
        } else if (is_X_true) {
          stepper2.runSpeed();
        }
      }

      if (!is_Z_true) {
        if (value_z_1 == LOW) {
          //Serial.println("Z is zero\n");
          stepper3.setCurrentPosition(0);
          is_Z_true = true;
        } else if (is_Y_true && is_X_true) {
          stepper3.runSpeed();
        }
      }

      if (is_X_true && is_Y_true && is_Z_true) {
        //Serial.println("State Zero has Finished...\n");
        stepper1.moveTo(-150);
        stepper2.moveTo(-200);
        stepper3.setSpeed(150);
        is_Z_true = false;
        currentState = STATE_HOME;
        delay(500);
      }
      break;
    

    case STATE_HOME:
      
      if (digitalRead(STOPbutton) == LOW)
        currentState = STATE_IDLE;

      stepper1.run();
      stepper2.run();
      
      if (value_z_2 == LOW) {
        is_Z_true = true;
      } else {
        stepper3.runSpeed();
      }

      if (stepper1.distanceToGo() == 0 && stepper2.distanceToGo() == 0 && is_Z_true) {
        //Serial.println("HOME reached...");
        
        stepper1.setCurrentPosition(0);
        stepper2.setCurrentPosition(0);
        stepper3.setCurrentPosition(0);

        stepper1.setSpeed(predkosc_x);
        stepper2.setSpeed(predkosc_y);
        stepper3.setSpeed(-200);

        currentState = STATE_WAIT;
      }
      break;
    

    //TODO LED CONTROL
    case STATE_WAIT:
      
      if (digitalRead(STOPbutton) == LOW)
        currentState = STATE_IDLE;

      if (digitalRead(STARTbutton) == LOW) {
        for (int i = 0; i < 3; i++)
        {
          delay(interval_BUTTON_PRESSED);
          ledState = (ledState == LOW) ? HIGH : LOW;
          digitalWrite(LED, ledState);
          }
          currentState = STATE_START; 
          delay(1000);
        }
        
      if (currentMillis_WAIT - previousMillis_WAIT >= interval_WAIT) {
        previousMillis_WAIT = currentMillis_WAIT; // Save the last toggle time

        // Toggle LED state
        ledState = (ledState == LOW) ? HIGH : LOW;
      }
      break;
    

    case STATE_START:
      if (digitalRead(STOPbutton) == LOW)
        currentState = STATE_IDLE;

      if (value_z_1 == LOW)
        currentState = STATE_MAIN_MOVEMENT;
      else
        stepper3.runSpeed();
      break;
    

    case STATE_MAIN_MOVEMENT: {
      
      if (digitalRead(STOPbutton) == LOW)
        currentState = STATE_IDLE;

      ledState = LOW;
      digitalWrite(LED, ledState);
      while(digitalRead(kran_x) == HIGH && currentState == STATE_MAIN_MOVEMENT){
        if (digitalRead(STOPbutton) == LOW)
        currentState = STATE_IDLE;

        stepper1.runSpeed();
        }
      
      stepper1.setCurrentPosition(0);
      stepper1.moveTo(-10);
      
      while (stepper1.distanceToGo() != 0 && currentState == STATE_MAIN_MOVEMENT){
        if (digitalRead(STOPbutton) == LOW)
        currentState = STATE_IDLE;

        stepper1.run();
        }
      
      stepper1.setCurrentPosition(0);

      float obwod_kroki = 2.0 * PI * kroki_x;
      unsigned long czas_okregu_ms = (obwod_kroki / predkosc_x) * 1000;

      float max_v_x = predkosc_x;
      float max_v_y = predkosc_y;

      for (int cykl = 0; cykl < wymagana_liczba; cykl++) {
        if (currentState != STATE_MAIN_MOVEMENT) break;
        if (digitalRead(STOPbutton) == LOW)
          currentState = STATE_IDLE;
       
        unsigned long start_cyklu = millis();
        unsigned long czas_aktualny = millis();

        while (czas_aktualny - start_cyklu < czas_okregu_ms && currentState == STATE_MAIN_MOVEMENT) {
          
          if (digitalRead(STOPbutton) == LOW)
            currentState = STATE_IDLE;

          czas_aktualny = millis();
          float progres = (float)(czas_aktualny - start_cyklu) / czas_okregu_ms;
          float kat = progres * 2.0 * PI;
          float aktualna_predkosc_x = -max_v_x * sin(kat);
          float aktualna_predkosc_y = max_v_y * cos(kat);

          stepper1.setSpeed(aktualna_predkosc_x);
          stepper2.setSpeed(aktualna_predkosc_y);
          stepper1.runSpeed();
          stepper2.runSpeed();
          yield();
        }
      }

      stepper1.setSpeed(0);
      stepper2.setSpeed(0);

      stepper3.setSpeed(200); 
      while (digitalRead(kran_z_2) == HIGH && currentState == STATE_MAIN_MOVEMENT) {
        stepper3.runSpeed();
        yield();
      }
      
      currentState = STATE_IDLE;
      ledState = HIGH;
      break;
    }
  }
}