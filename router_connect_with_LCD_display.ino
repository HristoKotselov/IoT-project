// include the library code:
#include <LiquidCrystal.h>
#include <SPI.h>
#include <WiFi101.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(6, 4, 3, 1, 2, 0);

char ssid[] = "SKY70AFA";     //  your network SSID (name)
char pass[] = "BSCABDYR";  // your network password
int status = WL_IDLE_STATUS;     // the WiFi radio's status

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
}


void loop() {
  /* Place for Azure connection code */
  //lcd.clear();
}
