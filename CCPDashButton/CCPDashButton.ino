/*
 * ***************************************************************
 *
 *  CCPDashButton
 *
 *  2019 Fabio Di Niro
 *
 * ***************************************************************
 * This file is part of CCPDashButton.

    CCPDashButton is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Nome-Programma is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nome-Programma.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "Arduino.h"
#include "heltec.h"
#include "WiFi.h"
#include "CiscoLogo.h"
#include "PubSubClient.h"
#include "NTPClient.h"

// Include the configuration file
#include "CCPDashButton.h"




WiFiClient espClient;
WiFiUDP udpClient;
void callBack(char* topic, byte* message, unsigned int length);
PubSubClient mqttClient(mqtt_server, mqtt_port, callBack, espClient);
NTPClient timeClient(udpClient, myNTPserver, timeOffset);

// Variables
String timeStamp;
String Message;
char pubMsg[50];
int currentProgressBar = 0;
String handlerStatus;
String clusterName="";
String clusterIP="";


boolean WIFISetUp() {
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.begin(ssid, password);
  delay(100);

  byte count = 0;
  while(WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    delay(500);
    Serial.println("Connecting WiFi...");
  }

  if(WiFi.status() == WL_CONNECTED)
  {
    // Print local IP address and start web server
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  else
  {
    Serial.println("WiFi connection failed");
    return false;
  }
}

void printMessageAndQuit(char* line1, char* line2, char* line3, int lines, int wait)
{
  // Print the message on 1, 2 or 3 lines
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  switch (lines)
  {
    case 1:
      Heltec.display->drawString(64, 30, String(line1));
    break;
    case 2:
      Heltec.display->drawString(64, 15, String(line1));
      Heltec.display->drawString(64, 30, String(line2));
    break;
    case 3:
      Heltec.display->drawString(64, 15, String(line1));
      Heltec.display->drawString(64, 30, String(line2));
      Heltec.display->drawString(64, 45, String(line3));
    break;
    default:
    break;
  }
  Heltec.display->display();

  // If connected to MQTT then disconnect
  if (mqttClient.connected() )
  {
    mqttClient.disconnect();
  }

  // If connected to WiFi then stop it
  if (WiFi.status() == WL_CONNECTED)
  {
    espClient.stop();
    WiFi.mode(WIFI_OFF);
  }
  delay(wait);
  esp_deep_sleep_start();
}



boolean NTPSetup()
{
  // Get current time and date
  timeClient.begin();
  while(!timeClient.update())
  {
    timeClient.forceUpdate();
  }
}


boolean sendCommand(String command, String argument)
{
  mqttClient.publish(mqtt_topic, pubMsg, false);
}

boolean checkStatus()
{
  // Prepare the message
  timeStamp = timeClient.getFormattedTime();
  Message = timeStamp+" "+dashButtonID+" GetStatus";
  Message.toCharArray(pubMsg, 50);
  Serial.println("Publishing the message: ");
  Serial.println(Message);
  return mqttClient.publish(mqtt_topic, pubMsg, false);
}

boolean provisionCluster()
{
  // Rad the timestamp
  timeStamp = timeClient.getFormattedTime();
  // Prepare the message.
  // As CCP does not allow ":" in the cluster name so we're stripping those chars
  Message = timeStamp
  Message.remove(2,1);
  Message.remove(4,1);

  // Cluster name will be dashButtonID-timestamp (without the ":")
  clusterName = String(dashButtonID)+"-"+Message;

  // Prepare the MQTT message to be published
  Message = Message+" "+dashButtonID+" ProvisionCluster "+clusterName;
  Message.toCharArray(pubMsg, 50);
  Serial.println("Publishing the message: ");
  Serial.println(Message);
  return mqttClient.publish(mqtt_topic, pubMsg, false);
}

void followProvisioning()
{
  // Query the handler waiting for provisioning completion and complete the progress bar in
  // averageProvisioningTime seconds so to give the impression that the progress bar is really
  // coming from the provisioning process
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(64, 0, "Provisioning");
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(64, 15, clusterName);
  int progress=10;
  int stepEvery=0;
  ProgressBar(progress);
  checkStatus();
  // We have to progress bar from 10 to 100% in averageProvisioningTime seconds
  if (averageProvisioningTime < 90)
  {
    // One progress bar step every 1 second
    stepEvery=1;
  }
  else
  {
    // One progress bar step every stepEvery seconds
    stepEvery=averageProvisioningTime / 90;
  }
  // Wait for status to change while incrementing the progress bar
  // after the averageProvisioningTime we wait 2 minutes more before declaring error timeout
  int timeout=averageProvisioningTime+120;
  int counter=0;

  while ( handlerStatus == "StatusProvisioning" && counter<timeout)
  {
    // The PubSub MQTT library needs the loop() function to be called to receive the messages
    for (int i=0; i<10; i++)
    {
      mqttClient.loop();
      delay(100);
    }
    progress = counter / stepEvery + 10;
    if (progress>100)
    {
      progress=100;
    }
    ProgressBar(progress);
    counter++;
  }
  if (handlerStatus == "StatusProvisioned")
  {
    char *clust = const_cast<char*>(clusterName.c_str());
    char *ipaddress = const_cast<char*>(clusterIP.c_str());
    printMessageAndQuit(clust, ipaddress, "is ready.", 3, 30000);
  }
  else
  {
    printMessageAndQuit("Timeout", "", "", 1, 10000);
  }
}


void callBack(char* topic, byte* message, unsigned int length) {
  char timeStamp[10];
  char sender[25];
  char command[25];
  char argument[50];
  Serial.println("Received message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Split message string in
  // HH:mm:ss Sender Command Argument
  sscanf(messageTemp.c_str(), "%s %s %s %s", timeStamp, sender, command, argument);

  // Check that the message is not coming from myself
  messageTemp=String(sender);
  if (messageTemp == dashButtonID) {
    return;
  }
  // Check that the message is coming from the right handlerID
  if (messageTemp == handlerID)
  {
      messageTemp=String(command);
      if (messageTemp == "StatusUnavailable")
      {
        Serial.println("Handler is unavailable, quitting");
        printMessageAndQuit("Handler", "unavailable", "", 2, 5000);
      }

      if (messageTemp == "StatusAvailable")
      {
        handlerStatus="StatusAvailable";
      }

      if (messageTemp == "StatusProvisioning")
      {
        handlerStatus="StatusProvisioning";
        clusterName=String(argument);
      }

      if (messageTemp == "StatusProvisioned")
      {
        handlerStatus="StatusProvisioned";
        clusterIP = String(argument);
      }
  }

  // Discard the received messqge
}


boolean MQTTSetup()
{
  //mqttClient.setServer(mqtt_server, mqtt_port);
  //mqttClient.setCallback(callBack);
  if (mqttClient.connected())
    return true;
  // Up to 3 connection attempts
  int i=0;
  while (!mqttClient.connected() && i<3)
  {
    Serial.print("Attempting MQTT connection...");
    // Prepare the LWT message
    timeStamp = timeClient.getFormattedTime();
    Message = timeStamp+" "+dashButtonID+" StatusUnavailable";
    Message.toCharArray(pubMsg, 50);
    // Attempt to connect, with QoS 1 and LWT message as retained
    //if (mqttClient.connect(dashButtonID, mqtt_topic, 1, true, pubMsg)) {
    if (mqttClient.connect(dashButtonID, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Subscribe
      if ( !mqttClient.subscribe(mqtt_topic, 1) )
      {
        Serial.println("Could not subscribe to MQTT topic");
        return false;
      }
      mqttClient.loop();
      mqttClient.loop();

    } else {
      Serial.println("");
      Serial.println("Failed to reconnect to MQTT server, trying again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
    i++;
  }
  return mqttClient.connected();
}

void MQTTReconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    if (mqttClient.connect(dashButtonID, mqtt_user, mqtt_password)) {
      // ... and resubscribe
      mqttClient.subscribe(mqtt_topic);
      mqttClient.loop();
      delay(100);
      mqttClient.loop();

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 1 second");
      // Wait 1 second before retrying
      delay(1000);
    }
  }
}

void drawCiscoLogo()
{
  Heltec.display->clear();
  // Draw the logo expanding the center line
  Heltec.display->clear();
  for (int i=0; i<=31; i++)
  {
    Heltec.display->setColor(WHITE);
    Heltec.display->drawXbm(7, 4, CiscoLogo_width, CiscoLogo_height, CiscoLogo_bits);
    Heltec.display->setColor(BLACK); // alternate colors
    Heltec.display->fillRect(0, 0, 128, (DISPLAY_HEIGHT/2)-i);
    Heltec.display->fillRect(0, (DISPLAY_HEIGHT/2)+i, 128, 63);
    Heltec.display->display();
    delay(15);
  }
  Heltec.display->setColor(WHITE);
}

void ProgressBar(int stage) {
  int progress = stage % 100;
  while (currentProgressBar < progress) {
    // draw the progress bar
    Heltec.display->drawProgressBar(0, 50, 120, 10, progress);
    // draw the percentage as String
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setColor(BLACK); // alternate colors
    Heltec.display->fillRect(45, 30, 30, 20);
    Heltec.display->setColor(WHITE); // alternate colors
    Heltec.display->drawString(64, 30, String(progress) + "%");
    Heltec.display->display();
    currentProgressBar += 1;
    delay(10);
  }
}


void setup() {
  // Handler status is unknown so we consider it unavailable
  handlerStatus="StatusUnavailable";

  // Initialize serial
  Serial.begin(115200);

  // Initialize the Heltec module
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Heltec.display->clear();
  Heltec.display->setContrast(255);


  // Show Cisco Logo
  drawCiscoLogo();

  // Connect to WiFi
  if (!WIFISetUp())
  {
    printMessageAndQuit("WiFi connection", "failed", "", 2, 5000);
  }

  // Initialize time
  NTPSetup();

  // Initialize MQTT
  if (!MQTTSetup())
  {
    printMessageAndQuit("MQTT connection", "failed", "", 2, 5000);
  }

  // Check Handler status
  checkStatus();

  // The PubSub MQTT library needs the loop() function to be called to receive the messages
  if (!mqttClient.connected() )
  {
    MQTTReconnect();
  }
  for (int e=0; e<30; e++)
  {
    mqttClient.loop();
    delay(100);
  }

  if (handlerStatus =="StatusUnavailable")
  {
    printMessageAndQuit("Handler", "unavailable", "", 2, 5000);
  }

  if (handlerStatus == "StatusProvisioning")
  {
    followProvisioning();
  }

  // Request cluster provision
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(25, 2, "Requesting");
  Heltec.display->drawString(25, 15, "K8s cluster");
  ProgressBar(5);
  provisionCluster();

  checkStatus();
  // Wait up to 5 seconds to receive confirm of provisioning
  int e=0;
  while ( handlerStatus != "StatusProvisioning" && e<5)
  {
    // The PubSub MQTT library needs the loop() function to be called to receive the messages
    for (int i=0; i<10; i++)
    {
      mqttClient.loop();
      delay(100);
    }
    e++;
  }

  if (handlerStatus != "StatusProvisioning")
  {
    Serial.println("Provisioning request was not confirmed");
    printMessageAndQuit("Can't confirm", "provisioning", "request", 3, 5000);
  }


  // Follow provisioing
  Serial.println("Provisioning confirmed, waiting for completion");
  ProgressBar(10);
  followProvisioning();



}

void loop() {
  // put your main code here, to run repeatedly:
}
