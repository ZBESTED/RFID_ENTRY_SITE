#include <SPI.h> 
#include <RFID.h>
#include <Wire.h>
#include <Servo.h> 
#include <LiquidCrystal_I2C.h>

#define Size 2

RFID rfid(10, 9);       //D10:pin of tag reader SDA. D9:pin of tag reader RST 
unsigned char str[16]; 

String accessGranted[Size] = {"","14315142131013"};  //RFID serial numbers to grant access to
String n[Size] = {"Best",""};
String *p;

//call I2C//
LiquidCrystal_I2C lcd(0x27, 20, 4);

Servo lockServo;                //Servo for locking mechanism
int lockPos = 15;               //Locked position limit
int unlockPos = 75;             //Unlocked position limit
boolean locked = true;

float redLEDPin = 5;
float greenLEDPin = 6;

void setup() 
{ 
  Serial.begin(9600);     //Serial monitor is only required to get tag ID numbers and for troubleshooting
  SPI.begin();            //Start SPI communication with reader
  rfid.init();            //initialization 
  pinMode(redLEDPin, OUTPUT);     //LED startup sequence
  pinMode(greenLEDPin, OUTPUT);
  digitalWrite(redLEDPin, HIGH);
    delay(500);
  digitalWrite(greenLEDPin, HIGH);
    delay(500);
  digitalWrite(redLEDPin, LOW);
  digitalWrite(greenLEDPin, LOW);
  lcd.begin();
  lcd.display();        //Open the sc
  lcd.backlight();      //turn on backlight 
  lcd.clear();          //clear the sc
  lockServo.attach(3);
  lockServo.write(lockPos);         //Move servo into locked position
} 

void loop() 
{ 
  lcd.setCursor(3, 0);
  lcd.print("Place Keycard...");
  Serial.println("Place Keycard...");
  
  if (rfid.findCard(PICC_REQIDL, str) == MI_OK)   //Wait for a tag to be placed near the reader
  { 
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Card found!");
      Serial.print("Card found"); 
    String temp = "";                             //Temporary variable to store the read RFID number
    
    if (rfid.anticoll(str) == MI_OK)              //Anti-collision detection, read tag serial number 
    { 
     lcd.setCursor(1, 1);
      lcd.print("ID: ");
        Serial.print(" ID: "); 
      
      for (int i = 0; i < 4; i++)                 //Record and display the tag serial number 
      { 
        temp = temp + (0x0F & (str[i] >> 4)); 
        temp = temp + (0x0F & str[i]); 
          Serial.println(temp);
      }
     lcd.setCursor(4, 1);
     lcd.print(temp); 
      Serial.println (temp);
        
    if(temp == "14315142131013")
    {
      p = &n[0];
      lcd.setCursor(1, 2);
      lcd.print("Name : ");
      lcd.setCursor(8, 2);
      lcd.print(*p);
        Serial.print("Name : ");
        Serial.print(*p);
    }
      checkAccess (temp);     //Check if the identified tag is an allowed to open tag
    } 
    rfid.selectTag(str); //Lock card to prevent a redundant read, removing the line will make the sketch read cards continually
  }
 
  rfid.halt();
}

void checkAccess (String temp)    //Function to check if an identified tag is registered to allow access
{
  boolean granted = false;
  
  for (int i=0; i <= (Size-1); i++)    //Runs through all tag ID numbers registered in the array
  {
    
    if(accessGranted[i] == temp)            //If a tag is found then open/close the lock
    {
      granted = true;
      
      if (locked == true)         //If the lock is closed then open it
      {   
        lockServo.write(unlockPos);
        locked = false;
        digitalWrite(greenLEDPin, HIGH);    
        delay(500);
        digitalWrite(greenLEDPin, LOW);
        lcd.setCursor(1,3);
        lcd.print("Access Granted!");
        Serial.println ("Access Granted"); 
        delay(1500);
        lcd.clear();
          
      }
      else if(locked == false)
      {
        lockServo.write(lockPos);
        locked = true;
        digitalWrite(greenLEDPin, HIGH);      
          delay(500);
        digitalWrite(greenLEDPin, LOW); 
        lcd.clear();
        lcd.setCursor(3,1);
        lcd.print("Door has been");
        lcd.setCursor(6,2);
        lcd.print("Locked!");
          delay(1500);   
        lcd.clear();
      }
      
    }
  }
  
  if (granted == false)     //If the tag is not found
  {
  digitalWrite(redLEDPin, HIGH);      
    delay(500);
  digitalWrite(redLEDPin, LOW);
  lcd.setCursor(1,3);
  lcd.print("Access Denied!");
    Serial.println ("Access Denied");
    delay(1500);
  lcd.clear();
  }
}