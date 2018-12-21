#include <Stepper.h>
 
const int stepsPerRev = 256;

LiquidCrystal_I2C lcd(0x3F,16,2);

int x = 0;
int y = 0;
boolean penIsDown = false;

Stepper xAxis(stepsPerRev, 2,3,4,5); 
Stepper yAxis(stepsPerRev, 6,7,8,9); 

// CW stuff
int xPin = A1;
int yPin = A0;
int buttonPin = 2;
int xPosition = 0;
int yPosition = 0;
int buttonState = 0;
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = { 9, 8, 7, 6 };
byte colPins[COLS] = { 12, 11, 10 }; 
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
int inputState = 0;
int piezoPin = 3;
int ledPin = 4;

void setup() {
  // UP DOWN
  pinMode(10, OUTPUT); // DOWN
  pinMode(11, OUTPUT); // UP

  pinMode(12, INPUT); // UP BUTTON
  pinMode(13, INPUT); // DOWN BUTTON

  xAxis.setSpeed(60);
  yAxis.setSpeed(60);

  lcd.init();
  lcd.backlight();
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(piezoPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  calibrate();

  penDown();
    
  Serial.begin(9600);
}

void penUp() {
  penIsDown = false;
  digitalWrite(10, HIGH);
  digitalWrite(11, LOW);
}

void penDown() {
  penIsDown = true;
  digitalWrite(11, HIGH);
  digitalWrite(10, LOW);
}

// Reset head to 0, 0
void calibrate() {
  penUp();
  xAxis.step(-256);
  yAxis.step(-256);

  x = 0;
  y = 0;
}

void goTo(int nx, int ny) {
  nx = max(0, min(nx, 256));
  ny = max(0, min(nx, 256));

  xAxis.step(nx - x);
  yAxis.step(ny - y);

  x = nx;
  y = ny; 
}

void moveX(int d) {
  int nx = max(0, min(x + d, 256));
  xAxis.step(nx - x);
}

void moveY(int d) {
  int ny = max(0, min(y + d, 256));
  yAxis.step(ny - y);
}

String getPos() {
  return String(x) + " " + String(y) + " " + String(penIsDown);
}

void loop() {

  xPosition = analogRead(xPin);
  yPosition = analogRead(yPin);
  buttonState = digitalRead(buttonPin);
  

  if (inputState == 0){
    if (xPosition > 530){
      xCord += 1;
      checkUpperX();
    }
    if (xPosition < 510){
      xCord -= 1;
      checkLowerX();
    }
  }
  if (inputState == 1){
    if (yPosition > 525){
      yCord += 1;
      checkUpperY();
    }
    if (yPosition < 505){
      yCord -= 1;
      checkLowerY();
    }
  }
  if (buttonState == 0){
    if (inputState == 0){
      inputState = 1;
    }
    else if (inputState == 1){
      inputState = 0;
    }
  }
  char key = kpd.getKey();
  if(key){
    switch (key){
      case '*':
        if (inputState == 0){
          checkZeroX();
          xCord = xCord / 10;
        }
        if (inputState == 1){
          checkZeroY();
          yCord = yCord / 10;
        }
        break;
      case '#':
        if (inputState == 0){
          inputState = 1;
        }
        else if (inputState == 1){
          inputState = 0;
        }
        break;
      default:
        if (inputState == 0){
          xCord = xCord * 10 + key - 48;
          checkUpperX();
        }
        if (inputState == 1){
          yCord = yCord * 10 + key - 48;
          checkUpperY();
        }
    }
  }
  if (inputState == 0){
    lcd.setCursor(0,0);
    lcd.print("X Coordinate:");
    lcd.setCursor(0,1);
    lcd.print(String(xCord)+"  ");
  }
  if (inputState == 1){
    lcd.setCursor(0,0);
    lcd.print("Y Coordinate:");
    lcd.setCursor(0,1);
    lcd.print(String(yCord)+"  ");
  }

  // Pen position buttons


  if (digitalRead(12) == LOW) {
    Serial.write("A");
    penUp();
  }
  if (digitalRead(13) == LOW) {
    Serial.write("B");
    penDown();
  }

  // Debug 
  //penDown();
  yAxis.step(256);
  //delay(1000);
  xAxis.step(256);
  //penUp();
  //delay(1000);
  //penDown();
  yAxis.step(-256);
  //delay(1000);
  xAxis.step(-256);
  //penUp();
  //delay(1000);

  /*  
  yAxis.step(256);
  xAxis.step(256);
  penUp();
  delay(1000);
  yAxis.step(-256);
  xAxis.step(-256);
  penDown();
  delay(1000);*/


  // Serial input
  if (Serial.available()) {
    String instruction = Serial.readString();

    if (instruction == "up") {
      penUp();
    }

    if (instruction == "down") {
      penDown();
    }

    if (instruction == "xd") {
      moveX(Serial.parseInt());
    }

    if (instruction == "yd") {
      moveY(Serial.parseInt());
    }

    if (instruction == "xy") {
      goTo(Serial.parseInt(), Serial.parseInt());  
    }
    
    Serial.println(getPos());
  }
}


void beep(){
  tone(piezoPin, 500);
  delay(100);
  tone(piezoPin, 3000);
  delay(100);
  noTone(piezoPin);
}
void ledBlink(){
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(100);
}
void checkUpperX(){
  if (xCord > 255){
    xCord = 255;
    beep();
    ledBlink();
  }
}
void checkLowerX(){
  if (xCord < 0){
    xCord = 0;
    beep();
    ledBlink();
  }
}
void checkUpperY(){
  if (yCord > 255){
    yCord = 255;
    beep();
    ledBlink();
  }
}
void checkLowerY(){
  if (yCord < 0){
    yCord = 0;
    beep();
    ledBlink();
  }
}
void checkZeroX(){
  if (xCord == 0){
    beep();
    ledBlink();
  }
}
void checkZeroY(){
  if (yCord == 0){
    beep();
    ledBlink();
  }
}
