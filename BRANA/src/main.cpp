#include <Arduino.h>

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Apple"
#define WLAN_PASS       "***********"
#define ovladani        D3
#define ovladani_close  D2

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "10.0.1.43"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, "","");

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish getBrana = Adafruit_MQTT_Publish(&mqtt,  "getBrana");
Adafruit_MQTT_Publish getBrana_close = Adafruit_MQTT_Publish(&mqtt,  "getBrana_close");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe  setBrana= Adafruit_MQTT_Subscribe(&mqtt, "setBrana");
Adafruit_MQTT_Subscribe  setBrana_close= Adafruit_MQTT_Subscribe(&mqtt, "setBrana_close");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  pinMode(ovladani, OUTPUT);
  digitalWrite(ovladani,LOW);
  pinMode(ovladani_close, OUTPUT);
  digitalWrite(ovladani_close,HIGH);
  delay(500);
  digitalWrite(ovladani_close,LOW);



  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&setBrana);
  mqtt.subscribe(&setBrana_close);

}

uint32_t x=0;
String message;

void loop() {

  MQTT_connect();



  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    // close - brana
    if (subscription == &setBrana_close) {
      message=(char *)setBrana_close.lastread;
      Serial.print(F("Got close: "));
      Serial.println(message);
      if (message=="true")
      {
       Serial.println("Zapnuto");
       digitalWrite(ovladani_close,HIGH);
     }
        delay(1000);
        getBrana_close.publish("TRUE");
        digitalWrite(ovladani_close,LOW);
        Serial.println("Vypnuto");

    }

   // open - brana
    if (subscription == &setBrana) {
      message=(char *)setBrana.lastread;
      Serial.print(F("Got: "));
      Serial.println(message);
      if (message=="true")
      {
       Serial.println("Zapnuto");
       digitalWrite(ovladani,HIGH);
     }
        delay(1000);
        getBrana.publish("TRUE");
        digitalWrite(ovladani,LOW);
        Serial.println("Vypnuto");

    }

  }


}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
