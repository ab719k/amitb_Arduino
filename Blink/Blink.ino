#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <LGPRSUdp.h>

#include <LGSM.h>

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
*/
#define ATTACH_GPRS 
#define SITE_URL "www.google.com"

LGPRSClient client;


// initialize the library instance
LSMSClass gsmAccess;
LSMSClass sms;
int sms_counter;
int sleep_counter;


const int pinLED = 3; // LED connect to D13
char recv[5]="On";

// the setup function runs once when you press reset or power the board
void setup() {

  int i = 0;
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  delay(10000);
  Serial.println();
  Serial.print("Starting...");
  while (!LSMS.ready())
  {
    delay(1000);
  }

 

#ifdef ATTACH_GPRS
  Serial.println("Starting Attach to wap.cingular");
  //while (!LGPRS.attachGPRS("isp.cingular", "", ""))
  while (!LGPRS.attachGPRS("wap.cingular","WAP@CINGULARGPRS.COM", "CINGULAR1"))
  {
    Serial.println("wait for SIM card ready");
    delay(5000);
  }
  Serial.print("Connecting to : " SITE_URL "...");
  if (!client.connect(SITE_URL, 80))
  {
    Serial.println("FAIL!");
    return;
  }
  //Serial.println("done");
  Serial.print("Sending GET request...");
  client.println("GET / HTTP/1.1");
  client.println("Host: " SITE_URL ":80");
  client.println();
#endif
  Serial.println("setup done");
  digitalWrite(13, LOW);

sms_counter=0;
}

// the loop function runs over and over again forever
void loop() {
  char buf[20];
  char buff[100];
  int v;

#if 0
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }
#endif

  while(sms_counter < 10)
  {
    Serial.println("Sending SMS");
   //if ( gsmAccess.beginSMS(PINNUMBER) == GSM_READY)
    if (LSMS.beginSMS("17327715310")) 
    {
      sprintf(buff, "Hello from LinkIt %d!", sms_counter);
      LSMS.print(buff);
      sms_counter++;
      LSMS.endSMS();
      Serial.println("SMS Sent");      
      delay(20000);      
    } else
    {
      Serial.println("SMS Not Sent");
    }
  } // end while  (sms_counter < 10)
  
  if(sms_counter > 1000)
  {
    sms_counter = 0;
  }
  
  if (LSMS.available()) // Check if there is new SMS
  {
    int j=0;
    Serial.println("There is new message.");
    LSMS.remoteNumber(buf, 20); // display Number part
    Serial.print("Number:");
    Serial.println(buf);

    Serial.print("Content:"); // display Content part
    while (true)
    {
      v = LSMS.read();
      if (v < 0)
        break;
      Serial.print((char)v);
      recv[j++]=(char)v;
    }
    Serial.println();
    
    recv[j]='\n';
    Serial.println(recv);
    
    LSMS.flush(); // delete message

  }

  //digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
//  digitalWrite(pinLED, HIGH); // LED on
  //Serial.print("ON");
  delay(20000);              // wait for a second
  //digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
//  digitalWrite(pinLED, LOW); // LED off
  //Serial.println(" OFF");
/*  
  if (recv[1] == 'n')
  {
    digitalWrite(13, HIGH);
//    digitalWrite(pinLED, HIGH); // LED on
  }
  else
  {
    digitalWrite(13, LOW);
//    digitalWrite(pinLED, LOW); // LED on
  }
  delay(1000);              // wait for a second
  //delay(10);
  */ 

}
