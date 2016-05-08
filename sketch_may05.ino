/*
 * ANNIKA SCIENCE PROJECT 2016
 * Comparing Digital and Analog Temperature.
 * 4 different sensors are used
 *  1) Grove -  Temperature Sensor using Thermistor (Analog A0)
 *  2) KY015 DHT11 Temperature and humidity sensor (Digital Pin3)  
 *  3) TMP36 Temperature Sensor (Analog A1)
 *  4) DS18B20 Digital Temperature Sensor (Digital Pin 2)
 */
 
#include "math.h"
#include <Wire.h>
#include "rgb_lcd.h"
#include <OneWire.h>


// 4) DS18B20
int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2
//Temperature chip i/o
OneWire ds(DS18S20_Pin); // on digital pin 2

// 3 TMP36
int tmp36_pin = 1; //the analog pin the TMP36's Vout (sense) pin is connected to
                        //the resolution is 10 mV / degree centigrade with a
                        //500 mV offset to allow for negative temperatures


int a;
float temperature;
int B=3975;                  //B value of the thermistor
float resistance;

// LCD - Grove Liquid Crystal Display
rgb_lcd lcd;
int i;


//KY015 DHT11 Temperature and humidity sensor 
int DHpin = 8;
byte dat [5];
byte read_data () {
  byte data;
  for (int i = 0; i < 8; i ++) {
    if (digitalRead (DHpin) == LOW) {
      while (digitalRead (DHpin) == LOW); // wait for 50us
      delayMicroseconds (30); // determine the duration of the high level to determine the data is '0 'or '1'
      if (digitalRead (DHpin) == HIGH)
        data |= (1 << (7-i)); // high front and low in the post
      while (digitalRead (DHpin) == HIGH); // data '1 ', wait for the next one receiver
     }
  }
return data;
}
 
void start_test () {
  digitalWrite (DHpin, LOW); // bus down, send start signal
  delay (30); // delay greater than 18ms, so DHT11 start signal can be detected
 
  digitalWrite (DHpin, HIGH);
  delayMicroseconds (40); // Wait for DHT11 response
 
  pinMode (DHpin, INPUT);
  while (digitalRead (DHpin) == HIGH);
  delayMicroseconds (80); // DHT11 response, pulled the bus 80us
  if (digitalRead (DHpin) == LOW);
  delayMicroseconds (80); // DHT11 80us after the bus pulled to start sending data
 
  for (int i = 0; i < 4; i ++) // receive temperature and humidity data, the parity bit is not considered
    dat[i] = read_data ();
 
  pinMode (DHpin, OUTPUT);
  digitalWrite (DHpin, HIGH); // send data once after releasing the bus, wait for the host to open the next Start signal
}


 
void setup()
{
  Serial.begin(9600);  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  i=0;
  pinMode (DHpin, OUTPUT);
   
  lcd.print("Science Project 16");
  lcd.setCursor(0, 1);
  lcd.print(" -  By Annika!"); 
    Serial.println("|=========================== Temperarture Sensor =============================|");
    Serial.println("|      DHT11       |       DS1B20       |       Resistor   |       Tmp36      |");
    Serial.println("|------------------+--------------------+------------------+------------------|");      
}
 
void loop()
{
  char Str1[16];
  char Str2[16];
  char StringDHT11[16];
  char StringGrove[16];
  char StringDS1B20[16];
  char StringTMP36[16];
  
  i++;

// DHT11 Sensor
  start_test ();
  //Serial.print ("Current humdity");
  //Serial.print (" and temperature = ");

  //Serial.print (dat [0], DEC); // display the humidity-bit integer;
  //Serial.print ('.');
  //Serial.print (dat [1], DEC); // display the humidity decimal places;
  //Serial.print ('%');
  Serial.print ("|      ");
  //Serial.print (dat [2], DEC); // display the temperature of integer bits;
  //Serial.print ('.');
  //Serial.print (dat [3], DEC); // display the temperature of decimal places;
  //Serial.print("C       |       ");
  //Serial.println(dat[2]);
  //Serial.println(dat[3]*0.01);
  float dht11= (dat[2]-1.5 + dat[3] *0.01) * 1.8 + 32;
  Serial.print(dht11);
  Serial.print("F      |       ");
  
  //delay (3000);
  
// Reading DS18B20 
  float temp_ds18b20 = getTemp();
  //Serial.print("temp_ds18b20 = ");
  //Serial.print(temp_ds18b20);
  //Serial.print(" C (");
  temp_ds18b20 = 1.8 * temp_ds18b20 + 32;  
  Serial.print(temp_ds18b20);
  Serial.print("F       |       ");
  //delay(200); //just here to slow down the output so it is easier to read

// Thermistor    
/* 
 * Grove -  Temperature Sensor using Thermistor
 *  This sensor detects the environment temperature,
 *  Connect the signal of this sensor to A0, use the 
 *  Serial monitor to get the result.
 */

  a=analogRead(0);
  resistance=(float)(1023-a)*10000/a; //get the resistance of the sensor;
  temperature=1/(log(resistance/10000)/B+1/298.15)-273.15;//convert to temperature via datasheet&nbsp;;
  temperature = 1.8* temperature + 32;

  Serial.print(temperature);
  Serial.print("F     |      ");


//getting the voltage reading from the temperature sensor
 int reading = analogRead(tmp36_pin);       
// converting that reading to voltage, for 3.3v arduino use 3.3
 float voltage = reading * 5.0;
 voltage /= 1024.0; 
// now print out the temperature
 float tmp36c = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                         //to degrees ((voltage - 500mV) times 100)
//Serial.print(tmp36c); Serial.println(" degrees C");
// now convert to Fahrenheit
float tmp36F = (tmp36c * 9.0 / 5.0) + 32.0;

Serial.print(tmp36F); Serial.println("F      |");


 
// DISPLAY LCD
      
      // set the cursor to (0,0):
    lcd.setCursor(0, 0);
    lcd.clear();
    dtostrf(temperature, 4, 2, Str2);
    sprintf(Str1, "GROVE : %sF", Str2);
    lcd.print(Str1);
    dtostrf(temp_ds18b20, 4, 2, Str2);
    lcd.setCursor(0, 1);
    sprintf(Str1, "DS1B20: %s F", Str2);
    lcd.print(Str1);   
    
    delay(5000);
    
    lcd.setCursor(0, 0);
    lcd.clear();
    dtostrf(dht11, 4, 2, Str1);
    sprintf(StringDHT11, "DHT11: %sF", Str1);
    lcd.print(StringDHT11);
    
    dtostrf(tmp36F, 4, 2, Str2);
    lcd.setCursor(0, 1);
    sprintf(Str1, "TMP36: %sF", Str2);
    lcd.print(Str1);   
   
    if(i > 20)
    {
      lcd.setCursor(0, 0);
      lcd.clear();
      lcd.print("Science Project");
      lcd.setCursor(0, 1);
      lcd.print(" -  By Annika!"); 
      i=0;
      delay(1000);
      Serial.println("|=========================== Temperarture Sensor =============================|");
      Serial.println("|      DHT11       |       DS1B20       |       Resistor   |       Tmp36      |");
      Serial.println("|------------------+--------------------+------------------+------------------|");    
    } else
    Serial.println("|------------------+--------------------+------------------+------------------|");
    //delay(600000);
    //for (int j =1; j < 10; j++)
    delay(3000);
 }

float getTemp(){
//returns the temperature from one DS18S20 in DEG Celsius
  byte data[12];
  byte addr[8];
  if ( !ds.search(addr)) {
  //no more sensors on chain, reset search
  ds.reset_search();
  return -1000;
  }
  if ( OneWire::crc8( addr, 7) != addr[7]) {
  Serial.println("CRC is not valid!");
  return -1000;
  }
  if ( addr[0] != 0x10 && addr[0] != 0x28) {
  Serial.print("Device is not recognized");
  return -1000;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end
  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad
  for (int i = 0; i < 9; i++) { // we need 9 bytes
  data[i] = ds.read();
  }
  ds.reset_search();
  byte MSB = data[1];
  byte LSB = data[0];
  
  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  return TemperatureSum;
}

