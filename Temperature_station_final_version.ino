// include the library code:
#include <LiquidCrystal.h>
#include <RTCZero.h>
#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiSSLClient.h>
#include "Adafruit_MCP9808.h"

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(5, 4, 3, 1, 2, 0);
// initialize Real Time Clock
RTCZero rtc;
// initialize wifi variables
char ssid[] = "SKY70AFA";  // network SSID (name)
char pass[] = "BSCABDYR";  // network password
//for demonstration use these
//char ssid[] = "AndroidHotspot2220";     //  your network SSID (name)
//char pass[] = "a6e8a9f1f3de";  // your network password
int status = WL_IDLE_STATUS;     // the WiFi radio's status
// initialize temperature variables
int sensorPin = 13;
float currentTemp;
float lastTemp;
String temperatureStatus = "";
WiFiSSLClient client;
//Initialise the variables for the SMS functionality
//ThingSpeak server
char ThingSpeakAddress[] = "api.thingspeak.com";
//ThingSpeak variables
//API key for the Thingspeak ThingHTTP already configured
const String writeDataAPIKey = "3D8WDZDLAK8AVRYL";
const String FireApiKey = "6KH4OU270WPSTOGV";
const String ColdApiKey = "KOUPGWRNCHB96VCJ";
const String ApiKey21_22C = "0XHT58NLRKHG3XRI";
const String ApiKey23_24C = "S3D88BNWLBZ071LO";
const String HotApiKey = "VSTKJ02D7UVFD9ZH";
//variable that serves as flag to say when to send data to ThingSpeak channel
boolean dataSent;
//if DHCP fails, use a static IP
IPAddress ip(192,168,0,177);
// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
//additional veriables for LEDs
int wifiPin = 7;
int dataPin = 6;
boolean lightOn = false;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // initialize RTC
  rtc.begin();
  // no data has been sent yet
  dataSent = false;
  // setup the pins for the LEDs
  pinMode(wifiPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  if (WiFi.status() == WL_NO_SHIELD) { // if shield not present
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.print("WiFi101 shield: ");
    lcd.setCursor(0, 1);
    lcd.println("NOT PRESENT");
    return; // don't continue
  }
  
  while ( status != WL_CONNECTED) {
    // Connecting message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi network");
    lcd.setCursor(0, 1);
    lcd.print("Connecting ...");

    // Connect to WPA/WPA2 network.
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10 * 1000);
  }
    // try to detect the MCP9808 temperature sensor
    if (!tempsensor.begin()) {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }
  // set the Real Time Clock
  rtc.setEpoch(WiFi.getTime());
  // Connected status output
  printWifiStatus();
  delay(5 * 1000);
  lcd.clear();
}

void loop() {
// read temperature from sensor
float readTemp = tempsensor.readTempC();
if (lastTemp == 0) currentTemp = readTemp;
// smooth the data, in order to not have big flunctuations
else currentTemp = (readTemp + lastTemp) / 2;
lcd.setCursor(0,0);
// print on LCD the date
lcd.print(rtc.getDay());
lcd.print("/");
lcd.print(rtc.getMonth());
lcd.print("/");
lcd.print(rtc.getYear());;
lcd.setCursor(0,1);
// print on LCD  the time
if (rtc.getHours() < 10) {
if (rtc.getMinutes() < 10) {
  lcd.print("0");
  lcd.print(rtc.getHours());
  lcd.print(":");
  lcd.print("0");
  lcd.print(rtc.getMinutes());
}
else {
  lcd.print("0");
  lcd.print(rtc.getHours());
  lcd.print(":");
  lcd.print(rtc.getMinutes());
}
}
else {
  if (rtc.getMinutes() < 10) {
  lcd.print(rtc.getHours());
  lcd.print(":");
  lcd.print("0");
  lcd.print(rtc.getMinutes());
}
else {
  lcd.print(rtc.getHours());
  lcd.print(":");
  lcd.print(rtc.getMinutes());
}
}
// print on LCD the temperature
if (currentTemp < 10) lcd.setCursor(10,1);
else lcd.setCursor(9,1);
lcd.print(currentTemp); lcd.print((char)223); lcd.print("C");

// check temperature status
if (currentTemp >= 21 && currentTemp <= 22.5 && temperatureStatus != "21-22") {
  temperatureStatus = "21-22";
  sendSMS(ApiKey21_22C);
}
else if (currentTemp >= 23 && currentTemp <= 24.5 && temperatureStatus != "23-24"){
  temperatureStatus = "23-24";
  sendSMS(ApiKey23_24C);
}
else if (currentTemp < 20.8 && temperatureStatus != "Cold") {
  temperatureStatus = "Cold";
  sendSMS(ColdApiKey);
}
else if (currentTemp > 25 && currentTemp < 40 && temperatureStatus != "Hot"){
  temperatureStatus = "Hot";
  sendSMS(HotApiKey);
}
else if (currentTemp > 40 && temperatureStatus != "Fire"){
  temperatureStatus = "Fire";
  sendSMS(FireApiKey);
}
// check if the device is still connected to WiFi
if (WiFi.status() == WL_CONNECTED && !lightOn){
  digitalWrite(wifiPin, HIGH);   // sets the LED on
  lightOn = true;
}
else if (WiFi.status() != WL_CONNECTED){
  digitalWrite(wifiPin, LOW);    // sets the LED off
  lightOn = false;
  //try to reconnect
  status = WiFi.begin(ssid, pass);
}

//reset channel data flag if the time
//is right after every 10th minute of the hour
if ((int)rtc.getMinutes() % 10 == 1) {
  dataSent = false;
}
else if (!dataSent) { //send data if it has not been sent yet
if ((int)rtc.getMinutes() % 10 == 0) { //every 10th munite of the hour
  sendDataToChannel("field1=" + String(currentTemp));
  dataSent = true;
}
}
//save the current temp for further smoothing of data
lastTemp = currentTemp;
delay(5 * 1000);
}

void printWifiStatus() {
  // print the SSID of the network the device is connected to
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SSID:");
  lcd.print(WiFi.SSID());

  // print the WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  lcd.setCursor(0, 1);
  lcd.print("IP:");
  lcd.print(ip);
}

void sendSMS(String apiKey) {
  // Make a TCP connection to remote host
  // Using SSL encryption with HTTP call
  if (client.connectSSL(ThingSpeakAddress, 443))
  {
    //should look like this...
    //api.thingspeak.com/apps/thinghttp/send_request?api_key={api key}
    client.print("GET /apps/thinghttp/send_request?api_key=");
    client.print(apiKey);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(ThingSpeakAddress);
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println(F("Connection failed"));
  } 

  // Check for a response from the server
  // and route it out the serial port.
  while (client.connected())
  {
    if ( client.available() )
    {
      char c = client.read();
      Serial.print(c);
    }      
  }
  Serial.println();
  client.stop();
}

void sendDataToChannel(String data) {
  // Make a TCP connection to remote host
  // Using SSL encryption with HTTP call
  if (client.connectSSL(ThingSpeakAddress, 443)) {
    digitalWrite(dataPin, HIGH);
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeDataAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\n\n");
    client.print(data);
    digitalWrite(dataPin, LOW); 

    if (client.connected()) {
      Serial.println("Sending Data to ThingSpeak...");
      Serial.println();
    }
  }
}
