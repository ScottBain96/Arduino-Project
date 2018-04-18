#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int HttpPort = 80;

String voice;

int ledGreen = 9;
int ledRed = 8;
int ledBlue = 7;
bool wifiOn = false;

//bitcoin API
String bitcoinPriceString = "-.--";
float bitcoinInitValue = 0.00;
float bitcoinNewValue;
String hostBitcoin = "blockchain.info";
String urlBitcoin = "/ticker";
char serverBitcoin[] = "blockchain.info";
bool getValue = false;
bool initialValue = false;
float percentage = 0.00;

char ssid[] = "hotspotname";             // your network SSID (name)
char pass[] = "password";               // your network password
int status = WL_IDLE_STATUS;           // the Wifi radio's status


// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Welcome");
  lcd.setCursor(0, 1);
  lcd.print("Connect to WiFi?");
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize serial for Bluetooth Module
  Serial2.begin(115200);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
}

void loop()
{
  if (wifiOn)
  {
    digitalWrite(ledBlue, HIGH);
    delay(1000);
    digitalWrite(ledBlue, LOW);
    delay(1000);
  }
  if (getValue)
  {
    delay(20000);
    getBitcoinPrice();
  }
  while (Serial2.available())
  {
    //Check if there is an available byte to read
    delay(10); //Delay added to make thing stable
    char c = Serial2.read(); //Conduct a serial read
    if (c == '#')
    {
      break;
    }
    //Exit the loop when the # is detected after the word
    voice += c; //Shorthand for voice = voice + c
  }
  if (voice.length() > 0)
  {
    Serial.println(voice);
    if (voice == "Bitcoin")
    {
      getBitcoinPrice();
      getValue = true;
    }
    if (voice == "connect")
    {
      connectToWifi();
    }
    if (voice == "disconnect")
    {
      disconnectFromWifi();
    }
  }
  voice = "";
}

void connectToWifi()
{
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    lcd.clear();
    lcd.print("Connecting to");
    lcd.setCursor(0, 1);
    lcd.print(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  wifiOn = true;
  delay(1000);
  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1000);
  lcd.clear();
  lcd.print("Track which");
  lcd.setCursor(0, 1);
  lcd.print("currency?");
  printWifiStatus();
}

void getBitcoinPrice()
{
  Serial.println();
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial

  if (client.connect(serverBitcoin, HttpPort))
  {
    Serial.println("Connected to server");
    if (!initialValue)
    {
      lcd.clear();
      lcd.print("Loading API...");
    }
    // Make a HTTP request
    client.println("GET " + urlBitcoin + " HTTP/1.1");
    client.println("Host: " + hostBitcoin);
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println("Unable to connect to Bitcoin API");
  }
  delay(500);
  // if there are incoming bytes available
  // from the server, read them and print them

  while (client.available())
  {
    String s = client.readStringUntil('\n');
    if (s.indexOf("USD") > 0)
    {
      int lineStartIndex = s.indexOf("USD");
      int lineEndIndex = s.indexOf("$");
      String line = s.substring(lineStartIndex, lineEndIndex);
      int priceStartIndex = line.indexOf("15m");
      int priceEndIndex = line.indexOf(",");
      bitcoinPriceString = line.substring(priceStartIndex + 7, priceEndIndex);
      if (!initialValue)
      {
        lcd.clear();
        lcd.print("Bitcoin Price:");
        lcd.setCursor(0, 1);
        lcd.print("$" + bitcoinPriceString);
        bitcoinInitValue = bitcoinPriceString.toFloat();
        initialValue = true;
      }
      bitcoinNewValue = bitcoinPriceString.toFloat();
      Serial.print("Latest Bitcoin Price: ");
      Serial.print("$" + bitcoinPriceString);
      if (bitcoinNewValue > bitcoinInitValue)
      {
        Serial.print("The value of bitcoin has increased");
        percentage = 100 * (bitcoinNewValue - bitcoinInitValue) / bitcoinInitValue;
        lcd.clear();
        lcd.print("Bitcoin Price:");
        lcd.setCursor(0, 1);
        lcd.print("$" + bitcoinPriceString + " +" + percentage + "%");
        digitalWrite(ledGreen, HIGH);
        digitalWrite(ledRed, LOW);
      }
      if (bitcoinNewValue < bitcoinInitValue)
      {
        Serial.print("The value of bitcoin has decreased");
        percentage = 100 * (bitcoinNewValue - bitcoinInitValue) / bitcoinInitValue;
        lcd.clear();
        lcd.print("Bitcoin Price:");
        lcd.setCursor(0, 1);
        lcd.print("$" + bitcoinPriceString + " " + percentage + "%");
        digitalWrite(ledGreen, LOW);
        digitalWrite(ledRed, HIGH);
      }
    }
  }
  Serial.println();
  Serial.println("Waiting for more data...");
  delay(100); // Have a bit of patience...

  // if the server's disconnected, stop the client
  if (!client.connected())
  {
    Serial.println();
    Serial.println("Disconnecting from server...");
    client.stop();
  }
}

void disconnectFromWifi()
{
  WiFi.disconnect();
  wifiOn = false;
  getValue = false;
  digitalWrite(ledBlue, LOW);
  Serial.println("Disconnected from Wifi");
  lcd.clear();
  lcd.print("Disconnected");
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

