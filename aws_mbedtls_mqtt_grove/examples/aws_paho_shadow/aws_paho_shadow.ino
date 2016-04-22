/* Working on Wifi Tested Apr 2016*/
/*
  Web client

 This sketch connects to a website
 using Wi-Fi functionality on MediaTek LinkIt platform.

 Change the macro WIFI_AP, WIFI_PASSWORD, WIFI_AUTH and SITE_URL accordingly.

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 modified 20 Aug 2014
 by MediaTek Inc.
 
 modified 29 Jan 2016
 by Seeed Inc

 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <vmsock.h>
#include <net.h>
#include <mtk.h>

#include <signal.h>
#include <limits.h>
#include "aws_iot_mqtt_interface.h"
#include "aws_iot_version.h"
#include "aws_iot_shadow_interface.h"
#include "aws_iot_shadow_json_data.h"
#include "aws_iot_json_utils.h"
#include "aws_iot_log.h"
#include "aws_mtk_iot_config.h"
#ifdef connect
#undef connect
#endif
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LGPS.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <LGPRSUdp.h>

#include <LGSM.h>


/* GPRS Specific Variable Begin */
#define SITE_URL "www.google.com"
LGPRSClient client;
gpsSentenceInfoStruct info;
char buff[256];
/* GPRS Specific Variable End */
char recv[161]="On";
#include <Grove_LED_Bar.h>

#define  LEDBAR_EN  0
#define  BTN_EN     1

#define USE_GPRS true

#if LEDBAR_EN
Grove_LED_Bar bar(3, 2, 1);  // Clock pin, Data pin, Orientation
#endif

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[255] = AWS_IOT_MQTT_HOST;
/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
VMINT port = AWS_IOT_MQTT_PORT;

char cafileName[] = AWS_IOT_ROOT_CA_FILENAME;
char clientCRTName[] = AWS_IOT_CERTIFICATE_FILENAME;
char clientKeyName[] = AWS_IOT_PRIVATE_KEY_FILENAME;


#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 200

QoSLevel qos = QOS_0;
int32_t i;
IoT_Error_t rc;



LWiFiClient c;

char shadowTxBuffer[256];
char deltaBuffer[256];

void ShadowUpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
const char *pReceivedJsonDocument, void *pContextData) {

    if (status == SHADOW_ACK_TIMEOUT) {
        Serial.println("Update Timeout--");
    } else if (status == SHADOW_ACK_REJECTED) {
        Serial.println("Update RejectedXX");
    } else if (status == SHADOW_ACK_ACCEPTED) {
        Serial.println("Update Accepted !!");
    }
}

void windowActuate_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext) {
    
    if (pContext != NULL) {
        Serial.print("\r\n\r\nDelta - Led Bar Level change to ");
        Serial.println(*(unsigned char *)(pContext->pData));
        Serial.println("\r\n\r\n");

#if LEDBAR_EN
        unsigned char level = (*(unsigned char *)(pContext->pData));
        if(level>=0 && level<=10)
        bar.setLevel(level);
#endif
    }
}


MQTTClient_t mqttClient;
char *pJsonStringToUpdate;
bool __GStatusTouch = 0;
char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
size_t sizeOfJsonDocumentBuffer;

unsigned char ledBarStatus = 0;
jsonStruct_t ledbarActuator;
jsonStruct_t touchHandler;
jsonStruct_t latitudeHandler;
jsonStruct_t longitudeHandler;
ShadowParameters_t sp;

double latitude;
double longitude;

// invoked in main thread context
void bearer_callback(VMINT handle, VMINT event, VMUINT data_account_id, void *user_data)
{
    if (VM_BEARER_WOULDBLOCK == g_bearer_hdl)
    {
        g_bearer_hdl = handle;
    }

    switch (event)
    {
        case VM_BEARER_DEACTIVATED:
        break;
        case VM_BEARER_ACTIVATING:
        break;
        case VM_BEARER_ACTIVATED:
        /************************ Add your code here ************************/

        rc = NONE_ERROR;
        i = 0;

        aws_iot_mqtt_init(&mqttClient);

        sizeOfJsonDocumentBuffer = sizeof(JsonDocumentBuffer) / sizeof(JsonDocumentBuffer[0]);

#if 0
        ledbarActuator.cb = windowActuate_Callback;
        ledbarActuator.pData = &ledBarStatus;
        ledbarActuator.pKey = "ledBarStatus";
        ledbarActuator.type = SHADOW_JSON_UINT8;

        touchHandler.cb = NULL;
        touchHandler.pKey = "Touch";
        touchHandler.pData = &__GStatusTouch;
        touchHandler.type = SHADOW_JSON_BOOL;
#endif
        
        latitudeHandler.cb = NULL;
        latitudeHandler.pKey = "latitude";
        latitudeHandler.pData = &latitude;
        latitudeHandler.type = SHADOW_JSON_DOUBLE;

        longitudeHandler.cb = NULL;
        longitudeHandler.pKey = "longitude";
        longitudeHandler.pData = &longitude;
        longitudeHandler.type = SHADOW_JSON_DOUBLE;


        sp = ShadowParametersDefault;
        sp.pMyThingName = (char*)AWS_IOT_MY_THING_NAME;
        sp.pMqttClientId = (char*)AWS_IOT_MQTT_CLIENT_ID;
        sp.pHost = HostAddress;
        sp.port = port;
        sp.pClientCRT = (char*)AWS_IOT_CERTIFICATE_FILENAME;
        sp.pClientKey = (char*)AWS_IOT_PRIVATE_KEY_FILENAME;
        sp.pRootCA = (char*)AWS_IOT_ROOT_CA_FILENAME;

        Serial.print("  . Shadow Init... ");
        rc = aws_iot_shadow_init(&mqttClient);
        if (NONE_ERROR != rc) {
            Serial.println("Error in connecting...");
        }
        Serial.println("ok");

        rc = aws_iot_shadow_connect(&mqttClient, &sp);

        if (NONE_ERROR != rc) {
            Serial.println("Shadow Connection Error");
        }

        rc = aws_iot_shadow_register_delta(&mqttClient, &ledbarActuator);

        if (NONE_ERROR != rc) {
            Serial.println("Shadow Register Delta Error");
        }

        // loop and publish a change in temp
        while (NONE_ERROR == rc) {
            rc = aws_iot_shadow_yield(&mqttClient, 1000);   //please don't try to put it lower than 1000, otherwise it may going to timeout easily and no response
            delay(1000);
            Serial.println("=======================================================================================");

            rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
            if (rc == NONE_ERROR) {
                //rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 2, &touchHandler, &ledbarActuator);
                rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 2, &latitudeHandler, &longitudeHandler);
                if (rc == NONE_ERROR) {
                    rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
                    if (rc == NONE_ERROR){
                        Serial.print("Update Shadow: ");
                        Serial.println(JsonDocumentBuffer);
                        rc = aws_iot_shadow_update(&mqttClient, AWS_IOT_MY_THING_NAME, JsonDocumentBuffer, ShadowUpdateStatusCallback, NULL, 4, true);
                    }
                }
            }
            Serial.println("*****************************************************************************************");
        }

        if (NONE_ERROR != rc) {
            Serial.println("An error occurred in the loop.");
        }

        Serial.println("Disconnecting");
        // Serial.println("Reconnect...");
        rc = aws_iot_shadow_disconnect(&mqttClient);

        if (NONE_ERROR != rc) {
            ERROR("Disconnect error");
        }

              /************************ End for your own code ************************/
        break;
        case VM_BEARER_DEACTIVATING:
        break;
        default:
        break;
    }
}

boolean bearer_open(void* ctx){

    if(USE_GPRS)
    {
      g_bearer_hdl = vm_bearer_open(VM_APN_USER_DEFINE, NULL, bearer_callback);
    }
    else if (WIFI_USED)
    {
        g_bearer_hdl = vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE_WLAN ,  NULL, bearer_callback);
    }
    else
        Serial.println("Error: connectivity Option should be either Cellular or WiFi.");     
    if(g_bearer_hdl >= 0)
    {
        Serial.println("bearer_open OK.");    
        return true;
    }
          
//    g_bearer_hdl = vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE_WLAN,  NULL, bearer_callback);
    return true;
}


static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

void parseGPGGA(const char* GPGGAstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */

  int tmp, hour, minute, second, num ;
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    
    
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    

    if (num > 0)
    {
      sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
      Serial.println(buff);
  
      sprintf(buff, "latitude = %10.4f, longitude = %10.4f", latitude, longitude);
      Serial.println(buff); 
      
      sprintf(buff, "satellites number = %d", num);
      Serial.println(buff); 
    }
      
  }
  else
  {
    Serial.println("Not get data"); 
  }
}

void setup()
{
    delay(5000);  
    Serial.begin(115200);
    delay(5000);
    
#if USE_GPRS
  while (!LSMS.ready())
  {
    delay(1000);
  }

  LSMS.beginSMS("17327715310");
  LSMS.println("Hello from LinkIt!");
  LSMS.endSMS(); 

//  while (!LGPRS.attachGPRS("wap.cingular","WAP@CINGULARGPRS.COM", "CINGULAR1"))
  Serial.print("  . Connecting to GPRS...");
  Serial.flush();
  while (!LGPRS.attachGPRS(GPRS_APN, GPRS_USERNAME, GPRS_PASSWORD))
  {
    Serial.println("wait for SIM card ready");
    delay(5000);
  }
  Serial.print("Connecting to : " SITE_URL "...");
  if (!client.connect(SITE_URL, 80))
  {
    Serial.println("FAIL!");
    
  }
  //Serial.println("done");
  Serial.print("Sending GET request...");
  client.println("GET / HTTP/1.1");
  client.println("Host: " SITE_URL ":80");
  client.println();   
#else
  LWiFi.begin();
#endif    


    
    pinMode(6, INPUT);
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    
#if LEDBAR_EN
    bar.begin();
    bar.setLevel(0);
#endif
    while(!Serial)
    delay(100);

    // keep retrying until connected to AP
#if USE_GPRS

#else    
    Serial.print("  . Connecting to AP...");
    //while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
    if (WIFI_AUTH == LWIFI_WPA)
    {
        while (0 == LWiFi.connectWPA(WIFI_AP, WIFI_PASSWORD))
        {
            delay(1000);
        }
    }
    else if (WIFI_AUTH == LWIFI_WEP)
    {
        while (0 == LWiFi.connectWEP(WIFI_AP, WIFI_PASSWORD))
        {
            delay(1000);
        }
    }
#endif

    Serial.println("ok");


    CONNECT_IP_ADDRESS = IP_ADDRESS;
    CONNECT_PORT = port;

    LTask.remoteCall(bearer_open, NULL);
    
    Serial.println();

    LGPS.powerOn();
    Serial.println("LGPS Power on, and waiting ..."); 
  
    delay(5000);
    Serial.println("setup_ok");
}

bool statusTouch    =0;
bool statusTouchBuf = 0;

void updateTouch()
{
    static unsigned long timer_t = millis();
    
    if(millis()-timer_t < 20)
    return;

    timer_t = millis();
    
    statusTouch = digitalRead(6);
    if(statusTouch != statusTouchBuf)
    {
        statusTouchBuf = statusTouch;
        __GStatusTouch = statusTouch;
        Serial.print("__GStatusTouch - ");
        Serial.println(__GStatusTouch);
    }
}

void loop()
{
    char buf[20];
    int v;
//    updateTouch();
    Serial.println("LGPS loop"); 
    LGPS.getData(&info);
    Serial.println((char*)info.GPGGA); 
    parseGPGGA((const char*)info.GPGGA);

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
    delay(2000);    
}

