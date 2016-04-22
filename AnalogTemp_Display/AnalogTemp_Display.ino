/*

Analog Temperature

Reading Tem

  Turn an LED on for one second, off for one second,
  and repeat forever.

*/

#include <LiquidCrystal.h>
#include <math.h>

// Connections:
// rs (LCD pin 4) to Arduino pin 12
// rw (LCD pin 5) to Arduino pin 11
// enable (LCD pin 6) to Arduino pin 10
// LCD pin 15 to Arduino pin 13
// LCD pins d4, d5, d6, d7 to Arduino pins 5, 4, 3, 2
LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2);

// the value of the 'other' resistor
#define SERIESRESISTOR 10000    
 
// What pin to connect the sensor to
#define THERMISTORPIN A0 

int backLight = 13;    // pin 13 will control the backlight

/*
Formula to get hte logs from the analog sensor
*/
double Thermister(int RawADC) 
{
  double Temp;
  Temp = log(((10240000/RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp ); Temp = Temp - 273.15;  // Convert Kelvin to Celcius
  return Temp;

}


void setup()
{
  Serial.begin(115200);
    
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH); // turn backlight on. Replace 'HIGH' with 'LOW' to turn it off.
  lcd.begin(16,2);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  lcd.clear();                  // start with a blank screen
  lcd.setCursor(0,0);           // set cursor to column 0, row 0 (the first row)
  lcd.print("Hello Annika");    // change this text to whatever you like. keep it clean.
  lcd.setCursor(0,1);           // set cursor to column 0, row 1
  lcd.print("Love Daddy");
  // if you have a 4 row LCD, uncomment these lines to write to the bottom rows
  // and change the lcd.begin() statement above.
  //lcd.setCursor(0,2);         // set cursor to column 0, row 2
  //lcd.print("Row 3");
  //lcd.setCursor(0,3);         // set cursor to column 0, row 3
  //lcd.print("Row 4");  
  delay(1000);  
}


// After setup() finishes, the loop() function runs over and over
// again, forever (or until you turn off or reset the Arduino).
// This is usually where the bulk of your program lives:


void loop()
{

  double an_temp = Thermister(analogRead(0))-10;
  char buff[15];
  char str_temp[6];
  char str_temp1[6];

  float reading= analogRead(0);

  Serial.print(" Analog Value =");
  Serial.print(reading);
  Serial.println(";");

  reading = (1023 / reading)  - 1;
  reading = SERIESRESISTOR / reading;
  Serial.print("Thermistor resistance "); 
  Serial.println(reading);
  Serial.println(";");

  
  Serial.print(an_temp);
  Serial.println("C");
  sprintf(buff, "T = %f C", an_temp);
  Serial.print(an_temp);  // display Fahrenheit
  Serial.println("c");
  delay(500);

  /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
  dtostrf(an_temp, 4, 2, str_temp);
  sprintf(buff,"Temp = %s C", str_temp);
  
  lcd.begin(16,2);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  lcd.clear();                  // start with a blank screen
  lcd.setCursor(0,0);           // set cursor to column 0, row 0 (the first row)
  lcd.print("Good Morning!");    // change this text to whatever you like. keep it clean.
  lcd.setCursor(0,1);           // set cursor to column 0, row 1
  lcd.print(buff);

  delay(2000);
//  an_temp = Thermister(analogRead(0));
  Serial.print(an_temp);
  an_temp =  1.8 * an_temp + 32;
  Serial.println("F - second time"); 
  dtostrf(an_temp, 4, 2, str_temp); 
  sprintf(buff, "Temp = %s F", str_temp);
  lcd.begin(16,2);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  lcd.clear();                  // start with a blank screen
  lcd.setCursor(0,0);           // set cursor to column 0, row 0 (the first row)
  lcd.print("April 13, 2006");    // change this text to whatever you like. keep it clean.
  lcd.setCursor(0,1);           // set cursor to column 0, row 1
  lcd.print(buff);
  delay(2000);  
   
  
}

