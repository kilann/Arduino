//************libraries**************//
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // library lcd
#include <SharpIR.h> // library ir
#include <SPI.h>  // library rfid
#include <MFRC522.h>  // library rfid
#include <Servo.h>  // library stepper motor

//************Define**************//
#define STEPS 32  // setup nombre de step par rotation
#define RST_PIN 9  // set rfid pin
#define SS_PIN 10  // set rfid pin
#define IR1 A6 // define signal pin
#define IR2 A7
#define model 430

//************************************//
LiquidCrystal_I2C lcd(0x27, 16, 2); // Display  I2C 16 x 2
SharpIR IRSensor1(SharpIR::GP2Y0A41SK0F, A6 );  // Infrared sensor
SharpIR IRSensor2(SharpIR::GP2Y0A41SK0F, A7);
Servo Motor1;  // creating Servo Motor object
Servo Motor2;
MFRC522 RfidSensor(SS_PIN, RST_PIN);

//************Button*****************//
int P1 = 7; // Button SET MENU'
int P2 = 6; // Button +
int P3 = 5; // Button -
int ref = 8;

//************Variables**************//
int hours;
int minutes;
int seconds;
int menu =0;

int pos1 = 0;  //  variable to store Motor position
int pos2 = 0;

int reelSwitch = 15; // magnetic senso rpin
int switchState; // variable to store reel switch value

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(P1,INPUT);
  pinMode(P2,INPUT);
  pinMode(P3,INPUT);
  pinMode(ref,OUTPUT);
  pinMode (reelSwitch, INPUT);

  SPI.begin();
  RfidSensor.PCD_Init();  // start rfid module
  Motor1.attach(2);  // set motor speed
  Motor2.attach(3);

  Serial.begin(9600);
  Wire.begin();
    
  int menu=0;
}
 
void loop()
{

  MenuSurveil();

  switch (menu) 
    {
        case 0:
        //do something when var equals 0
            incTime();
            DisplayDateTime();
            break;
        case 1:
            DisplaySetHour();
            break;
        case 2:
            DisplaySetMinute();
            break;

        default:
            menu=0;
        break;
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

    unsigned long startTime = millis(); // takes the time before the loop on the library begins

    int distance1 = IRSensor1.getDistance(); // this returns the distance to the object you're measuring
    int distance2 = IRSensor2.getDistance();

    unsigned long endTime = millis() - startTime; // the following gives you the time taken to get the measurement


    if (distance1 <= 20 && (UID.substring(1) == "UID Kitty" || UID.substring(1) == "UID Indy") && distance2 > 20)  // if distance and rfid are ok
    {

        for (pos2 = 0; pos2 <= 90; pos2 += 1) //goes from 0 degrees to 90 degrees
        {          // in steps of 1 degree
            Motor2.write(pos2);     //tell servo to go to position in variable 'pos'
        }
        delay(10000);
        seconds = seconds + 10;

        while ((switchState = digitalRead(reelSwitch)) > 1) // wait for gate close
        {
        delay(2000);
        seconds = seconds + 2;
        }
        for (pos2 = 90; pos2 >= 0; pos2 -= 1) //goes from 90 degrees to 0 degrees
        {
            Motor2.write(pos2);     //tell servo to go to position in variable 'pos'
        }
    }
    else if (distance2 <= 20 && (UID.substring(1) == "UID Kitty" || UID.substring(1) == "UID Indy") && distance1 > 20 && (hours >= 6 && hours < 23)) // if distance, rfid and Hour are ok
    {

        for (pos1 = 0; pos1 <= 90; pos1 += 1) //goes from 0 degrees to 90 degrees
        {          // in steps of 1 degree
            Motor1.write(pos2);     //tell servo to go to position in variable 'pos'
        }
        delay(10000);
        seconds = seconds + 10;

        while ((switchState = digitalRead(reelSwitch)) > 1) // wait for gate close
        {
        delay(2000);
        seconds = seconds + 2;
        }
        for (pos1 = 90; pos1 >= 0; pos1 -= 1) //goes from 90 degrees to 0 degrees
        {
            Motor1.write(pos1);     //tell servo to go to position in variable 'pos'
        }
    }
}

void MenuSurveil()
{
  digitalWrite(ref, LOW);
  digitalWrite(P1,HIGH);
  Serial.print(ref);
  Serial.print(P1);
  if(digitalRead(P1)== LOW)
  {
    delay(100);
    if (digitalRead(P1)== LOW)
    {
      Serial.print(P1);
      menu++;
    }
    digitalWrite(ref, HIGH);
  }
}

void DisplayDateTime()
{
    lcd.clear();

    lcd.setCursor(6,1);
    char time[17];
    sprintf(time, "  %02i:%02i:%02i", hours, minutes, seconds);
    lcd.print(time);
}
  
void DisplaySetHour()
{
// Setting the hour
  lcd.clear();
  digitalWrite(ref, LOW);
  digitalWrite(P2,HIGH);
  digitalWrite(P3,HIGH);

  if(digitalRead(P2)==LOW)
  {
    if (hours==23)
    {
      hours=0;
    }
    else
    {
      hours=hours+1;
    }
  }
   if(digitalRead(P3)==LOW)
  {
    if (hours==0)
    {
      hours=23;
    }
    else
    {
      hours=hours-1;
    }
  }
  lcd.setCursor(0,0);
  lcd.print("Set hours:");
  lcd.setCursor(0,1);
  lcd.print(hours,DEC);
  delay(200);
}

void DisplaySetMinute()
{
// Setting the minutes
  lcd.clear();
  digitalWrite(ref, LOW);
  digitalWrite(P2,HIGH);
  digitalWrite(P3,HIGH);

  if(digitalRead(P2)==LOW)
  {
    if (minutes==59)
    {
      minutes=0;
    }
    else
    {
      minutes=minutes+1;
    }
    seconds=0;
  }
   if(digitalRead(P3)==LOW)
  {
    if (minutes==0)
    {
      minutes=59;
    }
    else
    {
      minutes=minutes-1;
    }
    seconds=0;
  }
  lcd.setCursor(0,0);
  lcd.print("Set minute:");
  lcd.setCursor(0,1);
  lcd.print(minutes,DEC);
  delay(200);
}

void incTime() {
    
    delay(690);
    seconds++;

    if (seconds >= 60) 
    {
        seconds = seconds - 60;
        minutes++;

        if (minutes == 59) 
        {
        minutes = 0;
        hours++;

        if (hours == 23) 
        {
            hours = 0;
        }
    }
    }
}
