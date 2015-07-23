/*

  QuitNow! users counter: a wonderful way of getting
  the total QuitNow! Community users, printing it
  to an LCD screen, and switching a LED when a new
  user signs up on the Community

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * LCD screen attached to pins 2, 3, 4, 5, 6
 * Two leds attatched to pins 8, 9

 created 22 July 2015 by Roc Boronat

*/

#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xFE, 0x31, 0xAB, 0x50, 0xBA, 0xBE };
char server[] = "powergrid.fewlaps.com";
String response;

const int ledPinHttpRequest = 8;
const int ledPinSignup = 9;

const int numRows = 2;
const int numCols = 16;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

IPAddress ip(192, 168, 2, 42); // Set the static IP address to use if the DHCP fails to assign
EthernetClient client;

void setup() {
  Serial.begin(9600);
  lcd.begin(numCols, numRows);
  clearLCDAndPrint("Fewlaps!");

  delay(1000);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  clearLCDAndPrint("Checking leds...");
  pinMode(ledPinHttpRequest, OUTPUT);
  pinMode(ledPinSignup, OUTPUT);

  checkLeds();

  clearLCDAndPrint("Starting network...");
  delay(500);
  if (Ethernet.begin(mac) == 0) {
    clearLCDAndPrint("DHCP failed!");
    Ethernet.begin(mac, ip);
  } else {
    clearLCDAndPrint("DHCP worked!");
  }

  delay(1000);
  printLocalIP();

  delay(5000);
  clearLCDAndPrint("Getting ready...");
}

void loop()
{
  delay(1000);

  if (client.connect(server, 80)) {
    client.println("GET /quitnow-server/countUsers HTTP/1.1");
    client.println("Host: powergrid.fewlaps.com");
    client.println("Connection: close");
    client.println();
  }

  // if there are incoming bytes available
  // from the server, read them and print them:
  response = "";
  while (client.available()) {
    char c = client.read();
    if (c != '?') {
      response += c;
    }
  }

  processAnswer(response);

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    client.flush();
    client.stop();
  }
}

void processAnswer(String s) {
  switchOnLed(ledPinHttpRequest);
  int index = s.indexOf("7");

  if (index != -1 && index > 100) { //To ignore invalid results
    String start = s.substring(index + 1);
    String result = start.substring(0, start.length() - 5);
    result.trim();

    Serial.println(result);
    updateQuittersLCD(result);
  }
}

void clearLCDAndPrint(String s) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(s);
}

boolean firstLinePrinted = false;
String lastPrintedValue = "";
void updateQuittersLCD(String s) {
  if (!firstLinePrinted) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("QuitNow! users:");
    firstLinePrinted = true;
  }

  if (!s.equals(lastPrintedValue)) {
    lcd.setCursor(0, 1);
    lcd.print("     " + s);
    lastPrintedValue = s;
    switchOnLed(ledPinSignup, 2000); //It's the party time!
  }
}

void printLocalIP() {
  clearLCDAndPrint("My IP address:");
  lcd.setCursor(0, 1);
  lcd.print(Ethernet.localIP());
}

void checkLeds() {
  int i = 0;
  while (i < 10) {
    switchOnLed(ledPinHttpRequest);
    switchOnLed(ledPinSignup);
    i++;
  }
}

void switchOnLed(int pin) {
  switchOnLed(pin, 50);
}
void switchOnLed(int pin, int duration) {
  digitalWrite(pin, HIGH);
  delay(duration);
  digitalWrite(pin, LOW);
}

