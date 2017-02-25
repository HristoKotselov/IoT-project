// include the library code:
#include <LiquidCrystal.h>
#include <RTCZero.h>
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ThingSpeak.h>
#include "Adafruit_MCP9808.h"

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(5, 4, 3, 1, 2, 0);
// initialize Real Time Clock
RTCZero rtc;
// Change these values to set the current initial time
const byte seconds = 0;
const byte minutes = 4;
const byte hours = 2;
// Change these values to set the current initial date
const byte day = 19;
const byte month = 2;
const byte year = 17;
// initialize wifi variables
//char ssid[] = "SKY70AFA";     //  your network SSID (name)
//char pass[] = "BSCABDYR";  // your network password
//for demonstration - use these
char ssid[] = "AndroidHotspot2220";     //  your network SSID (name)
char pass[] = "a6e8a9f1f3de";  // your network password
int status = WL_IDLE_STATUS;     // the WiFi radio's status
// initialize temperature variables
int sensorPin = 13;
int tempC;
int tempCbeforepoint, tempCafterpoint;
String temperatureStatus = "";
WiFiServer server(80);
WiFiClient client = server.available();
//Initialise the variables for the SMS functionality
//ThingSpeak server
char ThingSpeakAddress[] = "api.thingspeak.com";
//ThingSpeak variables
const String writeDataAPIKey = "3D8WDZDLAK8AVRYL";
boolean dataSent;
//if DHCP fails, use a static IP
IPAddress ip(192,168,0,177);
//API key for the Thingspeak ThingHTTP already configured
const String FireApiKey = "6KH4OU270WPSTOGV";
const String ColdApiKey = "KOUPGWRNCHB96VCJ";
const String ApiKey21_22C = "0XHT58NLRKHG3XRI";
const String ApiKey23_24C = "S3D88BNWLBZ071LO";
//the number the message should be sent to
const String sendNumber = "+447593699374";

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  rtc.begin(); // initialize RTC
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);
  dataSent = false;

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

    if (!tempsensor.begin()) {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }

  printWifiStatus(); // Connected status output
  delay(5 * 1000);
  lcd.clear();
}

void loop() {
float c = tempsensor.readTempC();
int decimalTemp = int(c);
lcd.setCursor(0,0);
lcd.print(rtc.getDay()); lcd.print("/"); lcd.print(rtc.getMonth()); lcd.print("/"); lcd.print(rtc.getYear());;
lcd.setCursor(0,1);
if (rtc.getMinutes() < 10) {
  lcd.print(rtc.getHours()); lcd.print(":"); lcd.print("0"); lcd.print(rtc.getMinutes());
}
else {
lcd.print(rtc.getHours()); lcd.print(":"); lcd.print(rtc.getMinutes());
}
if (decimalTemp < 10) lcd.setCursor(10,1);
else lcd.setCursor(9,1);
lcd.print(c); lcd.print((char)223); lcd.print("C");

if (decimalTemp >= 21 && decimalTemp <= 22 && temperatureStatus != "21-22") {
  temperatureStatus = "21-22";
  sendSMS(sendNumber, ApiKey21_22C);
}
else if (decimalTemp >= 23 && decimalTemp <= 24 && temperatureStatus != "23-24"){
  temperatureStatus = "23-24";
  sendSMS(sendNumber, ApiKey23_24C);
}
else if (decimalTemp < 20 && temperatureStatus != "Cold") {
  temperatureStatus = "Cold";
  sendSMS(sendNumber, ColdApiKey);
}
else if (decimalTemp > 40 && temperatureStatus != "Fire"){
  temperatureStatus = "Fire";
  sendSMS(sendNumber, FireApiKey);
}

String temp = String(c);
if ((int)rtc.getMinutes() % 10 == 1 || (int)rtc.getMinutes() % 10 == 6) {
  dataSent = false;
}
else if (!dataSent) {
if ((int)rtc.getMinutes() % 10 == 0 || (int)rtc.getMinutes() % 10 == 5) {
  saveRecordedTemp("field1=" + temp);
  dataSent = true;
}
}
delay(10 * 1000);
lcd.clear();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SSID: ");
  lcd.print(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  lcd.setCursor(0, 1);
  lcd.print("IP: ");
  lcd.print(ip);
}

void sendSMS(String number, String apiKey) {
  // Make a TCP connection to remote host
  if (client.connect(ThingSpeakAddress, 80))
  {
    //should look like this...
    //api.thingspeak.com/apps/thinghttp/send_request?api_key={api key}&number={send to number}&message={text body}
    client.print("GET /apps/thinghttp/send_request?api_key=");
    client.print(apiKey);
    client.print("&number=");
    client.print(number);
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

  // Check for a response from the server, and route it
  // out the serial port.
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

void saveRecordedTemp(String data) {
  if (client.connect(ThingSpeakAddress, 80)) {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeDataAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\n\n");
    client.print(data);

    if (client.connected()) {
      Serial.println("Sending Data to ThingSpeak...");
      Serial.println();
    }
  }
}
