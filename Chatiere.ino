#include <SharpIR.h> // library ir
#include <SPI.h>  // library rfid
#include <MFRC522.h>  // library rfid
#include <Stepper.h>  // library stepper motor
#include <LiquidCrystal_I2C.h> // library lcd

#define STEPS 32  // setup nombre de step par rotation
#define RST_PIN 9  // set rfid pin
#define SS_PIN 10  // set rfid pin
#define IR1 A6 // define signal pin
#define IR2 A7
#define model 430
/*
  2 to 15 cm GP2Y0A51SK0F  use 1080
  4 to 30 cm GP2Y0A41SK0F / GP2Y0AF30 series  use 430
  10 to 80 cm GP2Y0A21YK0F  use 1080
  10 to 150 cm GP2Y0A60SZLF use 10150
  20 to 150 cm GP2Y0A02YK0F use 20150
  100 to 550 cm GP2Y0A710K0F  use 100550
*/

SharpIR IRSensor1(SharpIR::GP2Y0A41SK0F, A6 );  // Infrared sensor
SharpIR IRSensor2(SharpIR::GP2Y0A41SK0F, A7);
Stepper stepper1(STEPS, 5, 3, 4, 2); // stepper motor
Stepper stepper2(STEPS, 14, 7, 8, 6);
MFRC522 RfidSensor(SS_PIN, RST_PIN);  // setup rfid sensor
LiquidCrystal_I2C lcd(0x27, 16, 2);  // setup lcd

int reelSwitch = 15; // magnetic senso rpin
int switchState; // variable to store reel switch value

int h = 12;  // setup Heure
int m = 0;  // setup minute
int s = 0;  // setup seconde
int TIME = 0;
const int hs = A2;  // set pin to add Hour
const int ms = A3;  // set pin to add minute
int state1 = 0;  // variable read hs
int state2 = 0;  // variable read ms

int MoveMotor(Stepper motor)  // create function for motor moving
{
  motor.step(512);  // move motor1
  delay(10000);
  s = s + 10;

  while ((switchState = digitalRead(reelSwitch)) > 1) // wait for gate close
  {
    delay(2000);
    s = s + 2;
  }

  motor.step(-512);   //    fermeture du moteur 2
}

void setup() {

  Serial.begin(9600);  // start serial port
  SPI.begin();
  RfidSensor.PCD_Init();  // start rfid module
  stepper1.setSpeed(200);  // set motor speed
  stepper2.setSpeed(200);
  pinMode (reelSwitch, INPUT);  // setup magnetic pin
  pinMode(hs, INPUT_PULLUP);  // setup clock pushbouton
  pinMode(ms, INPUT_PULLUP);
  lcd.init();  // start lcd
  lcd.backlight();
}

void loop() {
  s = s + 1;  // start clock
  lcd.setCursor(0, 0);
  lcd.print("TIME:" );
  lcd.print(h);
  lcd.print(":");
  lcd.print(m);
  lcd.print(":");
  lcd.print(s);
  delay(1000);
  if (s >= 60)
  {
    s = 0;
    m = m + 1;
  }
  if (m >= 60)
  {
    m = 0;
    h = h + 1;
  }
  if (h >= 24)
  {
    h = 0;
  }
  state1 = digitalRead(hs);  // if pushbouton press add 1h
  if (state1 == 0)
  {
    h = h + 1;
  }
  state2 = digitalRead(ms);  // if pushbouton press add 1m and reset seconde
  if (state2 == 0)
  {
    s = 0;
    m = m + 1;
  }

  if ( ! RfidSensor.PICC_IsNewCardPresent())  // if rfid card is present
  {
    return;
  }

  if ( ! RfidSensor.PICC_ReadCardSerial())  // if rfid card is read
  {
    return;
  }
  String UID = "";

  for (byte i = 0; i < RfidSensor.uid.size; i++)  // setup rfid uid
  {
    UID.concat(String(RfidSensor.uid.uidByte[i] < 0x10 ? " 0" : " "));
    UID.concat(String(RfidSensor.uid.uidByte[i], HEX));
  }

  UID.toUpperCase();


  delay(1000);
  s = s + 1;

  unsigned long startTime = millis(); // takes the time before the loop on the library begins

  int distance1 = IRSensor1.getDistance(); // this returns the distance to the object you're measuring
  int distance2 = IRSensor2.getDistance();

  unsigned long endTime = millis() - startTime; // the following gives you the time taken to get the measurement


  if (distance1 <= 20 && (UID.substring(1) == "UID Kitty" || UID.substring(1) == "UID Indy") && distance2 > 20)  // if distance and rfid are ok
  {
    MoveMotor(stepper2);
  }
  else if (distance2 <= 20 && (UID.substring(1) == "UID Kitty" || UID.substring(1) == "UID Indy") && distance1 > 20 && (h >= 6 && h < 23)) // if distance, rfid and Hour are ok
  {
    MoveMotor(stepper1);
  }
}
