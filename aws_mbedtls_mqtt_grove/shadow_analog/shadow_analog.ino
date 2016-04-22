
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

#define  TEMP_EN    1

#if TEMP_EN
const int B=4275;                 // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A0;     // Grove - Temperature Sensor connect to A5
#endif

float getTemp()
{
#if TEMP_EN
    float a = analogRead(pinTempSensor);
    a = a*5.0/3.3;
    float R = 1023.0/((float)a)-1.0;
    R = 100000.0*R;
    float t=1.0/(log(R/100000.0)/B+1/298.15)-273.15;//convert to temperature via datasheet ;
    return t;
#else
    return 21.7;
#endif
}

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
    }
}


MQTTClient_t mqttClient;
char *pJsonStringToUpdate;
float temperature = 0.0;
char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
size_t sizeOfJsonDocumentBuffer;

unsigned char ledBarStatus = 0;
jsonStruct_t ledbarActuator;
jsonStruct_t temperatureHandler;
ShadowParameters_t sp;

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

        ledbarActuator.cb = windowActuate_Callback;
        ledbarActuator.pData = &ledBarStatus;
        ledbarActuator.pKey = "ledBarStatus";
        ledbarActuator.type = SHADOW_JSON_UINT8;

        temperatureHandler.cb = NULL;
        temperatureHandler.pKey = "temperature";
        temperatureHandler.pData = &temperature;
        temperatureHandler.type = SHADOW_JSON_FLOAT;

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
                rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 2, &temperatureHandler, &ledbarActuator);
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
    g_bearer_hdl = vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE_WLAN,  NULL, bearer_callback);
    return true;
}

void setup()
{
    LWiFi.begin();
    Serial.begin(9600);

    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

    while(!Serial)
    delay(100);

    // keep retrying until connected to AP
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

    Serial.println("ok");

    CONNECT_IP_ADDRESS = IP_ADDRESS;
    CONNECT_PORT = port;

    LTask.remoteCall(bearer_open, NULL);
    
    Serial.println();
    
    delay(5000);
    Serial.println("setup_ok");
}

void updateTemp()
{
    static unsigned long timer_t = millis();
    
    // update per 2000ms
    if(millis()-timer_t > 2000)
    {
        Serial.println("get tmp");
        timer_t = millis();
        temperature = getTemp();
        Serial.print("update temp: ");
        Serial.println(temperature);
    }
}

void loop()
{
    updateTemp();
}

