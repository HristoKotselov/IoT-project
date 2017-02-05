// include the library code:
#include <LiquidCrystal.h>
#include <SPI.h>
#include <WiFi101.h>

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

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  if (WiFi.status() == WL_NO_SHIELD) {
    lcd.clear();
    lcd.setCursor(0, 0);
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
    delay(10000);
  }

  printWifiStatus(); // Connected status output
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
delay(2000);
//lcd.clear();
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
