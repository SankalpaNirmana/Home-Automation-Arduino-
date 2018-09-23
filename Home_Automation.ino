#include <ESP8266WiFi.h>
#include <ArduinoJson.h> // Get the Json response
#include <WiFiClientSecure.h> // Secure library the WiFiClient.h
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
#include "DHT.h"

//pin define
#define DHTPIN D0
#define DHTTYPE DHT11

//Edit relevant fields
const char* ssid     = "SSID"; 
const char* password = "PASSWORD";
const char* host = "www.mywebsite.com";

int GAS = 0;
const char* authcode = "Add Authentication code here";

DHT dht(DHTPIN, DHTTYPE);
String url;
int count = 0;

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  delay(100);
  pinMode(A0 , INPUT);  //GAS
  pinMode(D3, INPUT);  //Motion
  pinMode(D4, OUTPUT);  //4 Channel Relay
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);

  // Set initial status as OFF
  digitalWrite(D4, 1);
  digitalWrite(D5, 1);
  digitalWrite(D6, 1);
  digitalWrite(D7, 1);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  digitalWrite(D1, 1);
  delay(500);
  digitalWrite(D1, 0);
  delay(500);
}


void loop() {
  WiFiClientSecure client;
  const int httpPort = 443;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  url = "/api/auth/read_all.php?id=1"; //API path to get bulb status
  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(1500);
  String section = "header";

  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);
    if (section == "header") { // headers..

      if (line == "\n") { // skips the empty space at the beginning
        section = "json";
      }
    }
    else if (section == "json") { // print the good stuff
      section = "ignore";
      String result = line.substring(1);

      // Parse JSON values
      int size = result.length() + 1;
      char json[size];
      result.toCharArray(json, size);
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& json_parsed = jsonBuffer.parseObject(json);
      if (!json_parsed.success())
      {
        Serial.println("parseObject() failed");
        return;
      }
      String auth = json_parsed["auth"][0]["pass"];
      if (auth == authcode) {
        Serial.println("authentication accepted");
        for (int i = 0; i < 4; i++) {
          bulbs();
        }
        gas();
        temp();
        motion();
      } else {
        Serial.println("authentication denied");
        digitalWrite(D1, 0);
      }
    }
  }
}


void bulbs() {
  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  if (count == 0) {
    url = "/api/relay/read_all.php?id=1";
    count = count + 1;
    Serial.println("Here1");
  }
  else if (count == 1) {
    url = "/api/relay/read_all.php?id=2";
    count = count + 1;
    Serial.println("Here2");
  }
  else if (count == 2) {
    url = "/api/relay/read_all.php?id=3";
    count = count + 1;
    Serial.println("Here3");
  }
  else if (count == 3) {
    url = "/api/relay/read_all.php?id=4";
    count = count + 1;
    Serial.println("Here4");
  }
  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(1500);
  String section = "header";
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);
    // we’ll parse the HTML body here
    if (section == "header") { // headers..

      if (line == "\n") { // skips the empty space at the beginning
        section = "json";
      }
    }
    else if (section == "json") { // print the good stuff
      section = "ignore";
      String result = line.substring(1);

      // Parse JSON
      int size = result.length() + 1;
      char json[size];
      result.toCharArray(json, size);
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& json_parsed = jsonBuffer.parseObject(json);
      if (!json_parsed.success())
      {
        Serial.println("parseObject() failed");
        return;
      }
      String led = json_parsed["led"][0]["status"];

      if (count == 1) {
        if (led == "on") {
          digitalWrite(D4, 0);
          delay(100);
          Serial.println("Living Room : On..!");
        }
        else if (led == "off") {
          digitalWrite(D4, 1);
          delay(100);
          Serial.println("Living Room : Off..!");
        }
      }
      else if (count == 2) {
        if (led == "on") {
          digitalWrite(D5, 0);
          Serial.println("Room 1 : On..!");
        }
        else if (led == "off") {
          digitalWrite(D5, 1);
          Serial.println("Room 1 : Off..!");
        }
      }
      else if (count == 3) {
        if (led == "on") {
          digitalWrite(D6, 0);
          Serial.println("Room 2 : On..!");
        }
        else if (led == "off") {
          digitalWrite(D6, 1);
          Serial.println("Room 2 : Off..!");
        }
      }
      else if (count == 4) {
        if (led == "on") {
          digitalWrite(D7, 0);
          Serial.println("Kitchen : On..!");
        }
        else if (led == "off") {
          digitalWrite(D7, 1);
          Serial.println("Kitchen : Off..!");
        }
        count = 0;
      }
      if (count == 4)
        count = 0;
    }
  }
  Serial.println();
  Serial.println("closing connection");
  delay(1000);
}

void temp() {
  //read humidity
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperature=");
  lcd.println(t);
  lcd.setCursor(0, 1);
  lcd.print("Humidity   =");
  lcd.println(h);
  if (isnan(h) && isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/api/weather/insert.php?temp=" + String(t) + "&hum=" + String(h);
  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(3000);

  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
  delay(500);
}

//Gas Function
void gas() {
  GAS = analogRead(A0);
  Serial.print("Gas Leaking : ");
  Serial.println(GAS);
  if (GAS > 300 ) {
    digitalWrite (8, 1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Gas leakage");
    delay(2000);
    lcd.clear();
  }
}

//Motion detection
void motion() {
  int val = digitalRead(D3);
  if (val == 1){
    Serial.println("Motion detected");
    digitalWrite (8, 1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Motion detected");
    delay(2000);
    lcd.clear();
    }
}

