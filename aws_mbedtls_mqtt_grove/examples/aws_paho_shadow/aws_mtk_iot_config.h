/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file aws_mtk_iot_config.h
 * @brief AWS IoT specific configuration file
 */

#ifndef SRC_SHADOW_IOT_SHADOW_CONFIG_H_
#define SRC_SHADOW_IOT_SHADOW_CONFIG_H_

// =================================================
#define AWS_IOT_MQTT_HOST              "A3R9YEMRNQV6JJ.iot.us-east-1.amazonaws.com"
#define AWS_IOT_MQTT_PORT              8883
#define AWS_IOT_MQTT_CLIENT_ID         "LinkIt_AWS_Test"
#define AWS_IOT_MY_THING_NAME          "LinkIt_AWS_Test"
#define AWS_IOT_ROOT_CA_FILENAME      "root-CA.crt"
#define AWS_IOT_CERTIFICATE_FILENAME   "a8226f764f-certificate.pem.crt"
#define AWS_IOT_PRIVATE_KEY_FILENAME   "a8226f764f-private.pem.key"
// =================================================
/*
#define AWS_IOT_MQTT_HOST              "A2DWFZYZMAZJGZ.iot.us-east-1.amazonaws.com" ///< Customer specific MQTT HOST. The same will be used for Thing Shadow
#define AWS_IOT_MQTT_PORT              8883 ///< default port for MQTT/S
#define AWS_IOT_MQTT_CLIENT_ID         "LinkIt_AWS_Test" ///< MQTT client ID should be unique for every device
#define AWS_IOT_MY_THING_NAME 	       "LinkIt_AWS_Test" ///< Thing Name of the Shadow this device is associated with
#define AWS_IOT_ROOT_CA_FILENAME       "root-CA.crt" ///< Root CA file name
#define AWS_IOT_CERTIFICATE_FILENAME   "3b0afa1972-certificate.pem.crt" ///< device signed certificate file name
#define AWS_IOT_PRIVATE_KEY_FILENAME   "3b0afa1972-private.pem.key" ///< Device private key filename
*/
// =================================================

//set to use Wifi or GPRS
#define WIFI_USED false  //true (Wifi) or false (GPRS)

/* change Wifi settings here */
#define WIFI_AP "TTS"
#define WIFI_PASSWORD "Brcm12345"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.

/* change GPRS settings here */
#define GPRS_APN "wap.cingular"   //for AT&T
#define GPRS_USERNAME "wap@cingulargprs.com"
#define GPRS_PASSWORD "cingular1"


/* change server settings here */
//VMSTR IP_ADDRESS = "https://A2DWFZYZMAZJGZ.iot.us-east-1.amazonaws.com"; //currently only support IP address
VMSTR IP_ADDRESS =  "54.85.255.214"; //"https://A3R9YEMRNQV6JJ.iot.us-east-1.amazonaws.com";

/* end of user settings */

#endif /* SRC_SHADOW_IOT_SHADOW_CONFIG_H_ */
