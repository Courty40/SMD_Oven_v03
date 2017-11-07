#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <max6675.h>
#include <PID_v1.h>

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)

//LCD setup and pins
Adafruit_PCD8544 display = Adafruit_PCD8544(13, 11, 5, 7, 6);

int running = 0;
int secs = 0;
//double aggKp=4, aggKi=0.2, aggKd=1;
double consKp=2, consKi=2, consKd=1;  //PID Constants 
double Setpoint, Input, Output;

// define pins for max6675 Thermocouple chip and LED
int thermoDO = 4, thermoCS = 3, thermoCLK = 2;
int ledPin = 9;


unsigned long windowStartTime;
unsigned long windowtime;
unsigned long seccount;
int WindowSize = 5000;

PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

void setup() {
  windowStartTime = millis();
  
  windowtime = millis();
  seccount = millis();
  
  pinMode(A0, OUTPUT); // Pins that control the Solid State Relay
  pinMode(A1, OUTPUT); 
  pinMode(A2, OUTPUT); 
  pinMode(A3, OUTPUT); 
  pinMode(A4, OUTPUT); 
  pinMode(A5, INPUT_PULLUP);  // Pin for the push button

  pinMode(ledPin, OUTPUT); 

  Setpoint = 20;
  
  display.begin();
  display.setContrast(50);
  display.clearDisplay();
 
 
  digitalWrite(A0, LOW); //+   // set the SSR to safe / low
  digitalWrite(A1, LOW); // - 
  digitalWrite(A2, LOW); // +
  digitalWrite(A3, LOW); // - 
  myPID.SetOutputLimits(0, WindowSize);
  myPID.SetMode(AUTOMATIC);
  showall();
}


void loop() {
 if(digitalRead(A5) == LOW)
 {
   running = 1;
   Setpoint = 20;
   seccount = millis();
   analogWrite(ledPin, 192); 
 }

   

      
 if(millis() - windowtime > 500)
  {
   PIDUpdate(); 
   showall();
   windowtime = millis();
     
if (running == 1) {
      //updateprofile();
      Setpoint = 150;
      secs = (millis() - seccount) / 1000;
   }  
  
  //delay(100);
}
}



// ######### this is the temp profile that the oven follows ##############
void updateprofile(){
 switch (secs) {
    case 10:
      Setpoint = 150;
      break;
    case 80:
     Setpoint = 160;
      break;
    case 180:
     Setpoint = 220;
      break;      
    case 210:
     Setpoint = 235;
      break;
    case 245:
     Setpoint = 50;
     running = 0;
     analogWrite(ledPin, 0); 
      break;  
  }

}

void PIDUpdate() {
  Input = int(thermocouple.readCelsius());
  myPID.Compute();

  if(millis() - windowStartTime>WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  
  if(Output < millis() - windowStartTime) {
    digitalWrite(A0,HIGH);
    digitalWrite(A2,HIGH);
  }else{
    digitalWrite(A0,LOW);
    digitalWrite(A2,LOW); 
  }
  
  if (Setpoint > Input) {
    digitalWrite(A0,LOW);
    digitalWrite(A2,LOW); 
  }
}



void showall() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(7,0);
  //display.print("SMD Oven v03");
    display.print(millis() - windowStartTime);
    display.print(" ");
    display.print(Output);

  display.setTextSize(2);
  display.setTextColor(BLACK);  
  display.setCursor(0,10);  
  display.print(thermocouple.readCelsius());
  display.print("C"); 

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,28);
  display.print("Target=");
  display.print(Setpoint);
  display.print("C");
  
  display.setCursor(0,40);
  display.print("STOP ");
  
  display.setCursor(40,40);
  display.print(secs);
  display.print("Secs");
  display.display();

}
