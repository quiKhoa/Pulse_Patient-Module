#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include "MAX30100.h"
#include "MAX30100_PulseOximeter.h"
#include <ArduinoJson.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define REPORTING_PERIOD_MS 1000

const char *EQUIPMENT_ID = "equipment-001";

MAX30100 maxim;
PulseOximeter pox;

const char *ssid = "TP-Link_08AEa";
const char *password = "dejavu01";
const char *serverName = "http://14.225.207.82:3000/api/equipments/listen-sensor-data";

String systolic_pressure;
String diastolic_pressure;
String pulse_rate;
String temperature_body;
String BPM;
String SpO2;

uint32_t tsLastReport = 0;

// String sensor_data;
// bool Sr;
// String machineid = "MR1";
// String room_number= "A1";
// String bed_number= "1";

HTTPClient http;
WiFiClient client;

// Callback routine is executed when a pulse is detected
void onBeatDetected()
{
  Serial.print("Heart rate:");
  Serial.print(pox.getHeartRate());
  Serial.print("bpm / SpO2:");
  Serial.print(pox.getSpO2());
  Serial.println("%");

  String postData = buildJSONStringData(pox.getHeartRate(), pox.getSpO2());

  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/json"); // Set the content type if needed
  http.POST(postData);
  http.end();
  maxim.resetFifo();
  // tsLastReport = millis();
}

String buildJSONStringData(int heartbeat, int spo2)
{
  DynamicJsonDocument doc(JSON_OBJECT_SIZE(4));
  JsonObject data = doc.to<JsonObject>();
  data["heartbeat"] = heartbeat;
  data["spo2"] = spo2;
  data["timestamp"] = millis();
  data["id"] = EQUIPMENT_ID;
  String output;
  serializeJson(doc, output);
  Serial.println(output);
  return output;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(16, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to SSID:");
  Serial.print(ssid);

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("Patient Monitor Wifi 01"); // password protected ap //access 192.168.4.1
  if (!res)
    Serial.println("Failed to connect");
  else
    Serial.println("Connected... :)");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected Successfully!!");
  Serial.println(WiFi.localIP());

  Serial.print("Initializing pulse oximeter..");

  // Initialize sensor
  if (!pox.begin())
  {
    Serial.print("Failed");
    for (;;)
      ;
  }
  else
  {
    Serial.println("Success");
  }

  // http.begin(serverName);
  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  // put your main code here, to run repeatedly:
  pox.update();
}
