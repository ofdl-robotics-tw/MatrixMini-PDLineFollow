#include <MatrixMini.h>
MatrixMini Mini;

//Raw light value
int leftAna_max = 540;
int leftAna_min = 28;
int rightAna_max = 770;
int rightAna_min = 32;

//init
int err = 0;
int err_old = 0;
int turns_cnt = 0;
int turns_incir_cnt = 0;

//Return normalized reflect light value
int lightRead(int pins) {
  int returnVal;
  if (pins == 0) {
    returnVal = map(analogRead(A0), leftAna_min, leftAna_max, 100, 0);
  } else {
    returnVal = map(analogRead(A1), rightAna_min, rightAna_max, 100, 0);
  }
  return constrain(returnVal, 0, 100);
}

//Read light values
void SensorDbg() {
  while (true) {
    Serial.println("-----------------------");
    Serial.print("RAW S0:");
    Serial.print(analogRead(A0));
    Serial.print(",S1:");
    Serial.println(analogRead(A1));
    Serial.print("Nor S0:");
    Serial.print(lightRead(0));
    Serial.print(",S1:");
    Serial.println(lightRead(1));
    Serial.print("D3:");
    Serial.print(digitalRead(3));
    Serial.print(",D5:");
    Serial.println(digitalRead(5));
    delay(500);
  };
}

//PD Line Follow
void LinePD(int power, float kp, float kd) {
  err = lightRead(1) - lightRead(0);
  float diff = err * kp + kd * (err - err_old);
  Mini.M1.set(power - diff);
  Mini.M2.set(power + diff);
  err_old = err;
}

//Tank Move
void moveTank(int p1, int p2) {
  Mini.M1.set(p1);
  Mini.M2.set(p2);
}

//Turns
void TurnUntilSen(int turns) {
  //Turn Left
  //delay(30);
  switch (turns) {
    case 0:
      while (lightRead(1) > 60 ) {
        moveTank(-20, 40);
      };
      break;
    case 1:
      while (lightRead(0) > 60 ) {
        moveTank(40, -20);
      };
      break;
    default:
      break;
  }
  moveTank(0, 0);
  delay(100);
  turns_cnt++;
}

//Special turns
void CircleFollow() {
  int CircleCnt = 0;
  moveTank(0, 0);
  delay(100);
  moveTank(40, 40);
  delay(600);
  while (CircleCnt <= 4) {   
    if (digitalRead(5) == 1) {
      if (CircleCnt == 2) {
        Mini.LED2.setHSV(random(0, 255), random(0, 255), random(0, 255));
        moveTank(40, -20);
        delay(700);
      }
      while (lightRead(0) > 60 ) {
        moveTank(40, -20);
      };
      CircleCnt++;
    } else {
      moveTank(30, 30);
    }
  }
  Mini.LED2.setRGB(0,0,0);
  while (lightRead(0) > 60 ) {
  LinePD(20, 0.15, 0.17);
  };
  Mini.LED2.setRGB(255,0,0);
  while (lightRead(0) < 60 ) {
  LinePD(25, 0.13, 0.17);
  };
  Mini.LED2.setRGB(0,0,255);
  turns_cnt++;
}

//Different place using different set
void LineFollow() {
  int BASE_PWR;
  float P, D;
  switch (turns_cnt) {
    case 4:
      BASE_PWR = 20;
      P = 0.13;
      D = 0.07;
      break;
    case 6:
      BASE_PWR = 25;
      P = 0.13;
      D = 0.07;
      break;
    case 7:
      BASE_PWR = 25;
      P = 0.13;
      D = 0.07;
      break;
    default:
      BASE_PWR = 50;
      P = 0.13;
      D = 0.06;
  }
  LinePD(BASE_PWR, P, D);
}

void setup() {
  Mini.begin();  // create with the default frequency 1.6KHz
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("\n Matrix Mini Line PD Test \n");
  pinMode(3, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  Mini.M1.set(0);
  Mini.M2.set(0);
  Mini.LED1.setRGB(255, 0, 0);
  
  //Wait button press
  while (true) {
    if (Mini.BTN1.get() == 1) {
      break;
    };
  }
  delay(600);
  Mini.M1.set(40);
  Mini.M2.set(40);
  delay(600);
}

void loop() {
  //SensorDbg();
  Serial.println(turns_cnt);
  if (digitalRead(3) == 1 && digitalRead(5) == 1) {
    Mini.LED1.setRGB(255, 0, 0);
    moveTank(20, 20);
    delay(1200);
    moveTank(0, 0);
  } else if (turns_cnt == 6 && digitalRead(5) == 1) {
    CircleFollow();
  } else if (digitalRead(3) == 1 && turns_cnt != 6) {
    Mini.LED1.setRGB(0, 255, 0);
    TurnUntilSen(0);
  } else if (digitalRead(5) == 1 && turns_cnt != 6) {
    Mini.LED1.setRGB(0, 0, 255);
    TurnUntilSen(1);
  } else {
    Mini.LED1.setRGB(0, 0, 0);
    LineFollow();
  }
}

//LinePD(80,0.5,0.45);
//LinePD(35,0.175,0.3);
