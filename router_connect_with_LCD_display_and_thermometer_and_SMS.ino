// include the library code:
#include <LiquidCrystal.h>
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiSSLClient.h>
#include <WiFiUdp.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(6, 4, 3, 1, 2, 0);

// initialize wifi variables
char ssid[] = "SKY70AFA";     //  your network SSID (name)
char pass[] = "BSCABDYR";  // your network password
int status = WL_IDLE_STATUS;     // the WiFi radio's status
// initialize temperature variables
int sensorPin = 0;
int tempC, tempF;
int tempCbeforepoint, tempCafterpoint, tempFbeforepoint, tempFafterpoint;
String temperatureStatus;
WiFiServer server(80);
WiFiClient client;
//Initialise the variables for the SMS functionality
//thingspeak server
char SMSserver[] = "api.thingspeak.com";
//if DHCP fails, use a static IP
IPAddress ip(192,168,0,177);
//API key for the Thingspeak ThingHTTP already configured
const String FireApiKey = "6KH4OU270WPSTOGV";
const String ColdApiKey = "KOUPGWRNCHB96VCJ";
const String ApiKey21_22C = "0XHT58NLRKHG3XRI";
const String ApiKey23_24C = "S3D88BNWLBZ071LO";
//the number the message should be sent to
const String sendNumber = "+447593699374";

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

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

  printWifiStatus(); // Connected status output
  delay(5 * 1000);
  lcd.clear();
}

void loop() {
tempC = get_temperature(7);
tempCbeforepoint = tempC / 10;
tempCafterpoint = tempC % 10;
tempF = celsius_to_fahrenheit(tempC);
tempFbeforepoint = tempF / 10;
tempFafterpoint = tempF % 10;
lcd.setCursor(0,0);
lcd.print(tempCbeforepoint); lcd.print(".");lcd.print(tempCafterpoint); lcd.print((char)223); lcd.print("C");
lcd.setCursor(0,1);
lcd.print(tempFbeforepoint); lcd.print(".");lcd.print(tempFafterpoint); lcd.print((char)223); lcd.print("F");
delay(5 * 1000);
//lcd.clear();

client = server.available();   // listen for incoming clients
if (tempCbeforepoint >= 21 && tempCbeforepoint <= 22 && temperatureStatus != "21-22") {
  temperatureStatus = "21-22";
  sendSMS(sendNumber, ApiKey21_22C);
}
else if (tempCbeforepoint >= 23 && tempCbeforepoint <= 24 && temperatureStatus != "23-24"){
  temperatureStatus = "23-24";
  sendSMS(sendNumber, ApiKey23_24C);
}
else if (tempCbeforepoint < 20 && temperatureStatus != "Cold") {
  temperatureStatus = "Cold";
  sendSMS(sendNumber, ColdApiKey);
}
else if (tempCbeforepoint > 40 && temperatureStatus != "Fire"){
  temperatureStatus = "Fire";
  sendSMS(sendNumber, FireApiKey);
}
delay(30 * 1000);
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
  if (client.connect(SMSserver, 80))
  {

    //should look like this...
    //api.thingspeak.com/apps/thinghttp/send_request?api_key={api key}&number={send to number}&message={text body}
    client.print("GET /apps/thinghttp/send_request?api_key=");
    client.print(apiKey);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(SMSserver);
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

int get_temperature(int pin) {
// We need to tell the function which pin the sensor is hooked up to. We're using
// the variable pin for that above
// Read the value on that pin
int temperature = analogRead(pin);
// Calculate the temperature based on the reading and send that value back
float voltage = temperature * 5.0;
voltage = voltage / 1024.0;
return ((voltage - 0.5) * 100);
}
int celsius_to_fahrenheit(int temp) {
return (temp * 9 / 5) + 32;
}
