/************************************************************
ping-lights
Modified from SparkFun ESP8266 AT library - Ping Demo

This pings a destination and will display on a NeoPixel LED
array.
************************************************************/

#include <SoftwareSerial.h>
#include <SparkFunESP8266WiFi.h>

#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 6
#define LED_COUNT 40
#define PING_COUNT LED_COUNT-1

#define BRIGHTNESS 64
#define PING_EXPECTED 15
#define PING_MAX 50

// Replace these two character strings with the name and
// password of your WiFi network.
const char mySSID[] = "";
const char myPSK[] = "";

char destServer[] = "sparkfun.com";
IPAddress googleDNS(8, 8, 8, 8);

// Create an instance of the Adafruit_NeoPixel class called "leds".
// That'll be what we refer to from here on...
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

int pingHistory[PING_COUNT];

void setup() 
{
  Serial.begin(9600);

  leds.begin();  // Call this to start up the LED strip.
  clearLEDs();   // This function, defined below, turns all LEDs off...
  leds.show();   // ...but the LEDs don't actually update until you call this.

  for (int i = 0; i < PING_COUNT; i++) {
    pingHistory[i] = 0;
  }
  
  leds.setPixelColor(LED_COUNT-1, leds.Color(0, 0, 15));
  leds.show();
  
  // initializeESP8266() verifies communication with the WiFi
  // shield, and sets it up.
  initializeESP8266();

  // connectESP8266() connects to the defined WiFi network.
  connectESP8266();
  
  leds.setPixelColor(LED_COUNT-1, leds.Color(15, 15, 0));
  leds.show();

  // displayConnectInfo prints the Shield's local IP
  // and the network it's connected to.
  displayConnectInfo();

  
  delay(1000);
  Serial.print("Pinging ");
  Serial.println(googleDNS);
}

void loop() 
{
  int ping = esp8266.ping(googleDNS);
  Serial.println(ping);

  addPing(ping);

  delay(500);
  leds.setPixelColor(LED_COUNT-1, leds.Color(0, 15, 0));
  leds.show();
  delay(500);
  leds.setPixelColor(LED_COUNT-1, leds.Color(0, 0, 0));
  leds.show();
}

void addPing(int ping)
{
  int pingAdjusted = ping;
  // first shift all down one
  for (int i = PING_COUNT-1; i > 0; i--) {
    pingHistory[i] = pingHistory[i-1];
  }
  pingHistory[0] = pingAdjusted;

  // set colors
  for (int i = 0; i < PING_COUNT; i++) {
    leds.setPixelColor(i, PingWheel(pingHistory[i]));
  }  
  leds.show();
  
}

// green to red gradient
uint32_t PingWheel(int ping) {
  int WheelPos = (ping - PING_EXPECTED) * (BRIGHTNESS / (PING_MAX - PING_EXPECTED));

  if (WheelPos < 0) {
    return leds.Color(0, 0, 0);
  } else if (WheelPos <= BRIGHTNESS/2) {
    return leds.Color(0, WheelPos, 0);
  } else if (WheelPos <= BRIGHTNESS) {
    return leds.Color(WheelPos, BRIGHTNESS-WheelPos, 0);
  } else if (WheelPos <= BRIGHTNESS*2) {
    return leds.Color(BRIGHTNESS, 0, 0);
  } else {
    return leds.Color(BRIGHTNESS, 0, 0);
  }  
}

uint32_t ColorWheel(int WheelPos) {
  if(WheelPos < 64) {
   return leds.Color(0, 255, WheelPos*4);
  } else if(WheelPos < 128) {
   WheelPos -= 64;
   return leds.Color(0, 255-(WheelPos*4), 255);
  } else if(WheelPos < 192) {
   WheelPos -= 128;
   return leds.Color(WheelPos*4, 0, 255);
  } else if(WheelPos < 256) {
   WheelPos -= 192;
   return leds.Color(255, 0, 255-(WheelPos*4));
  } else if(WheelPos < 320) {
   WheelPos -= 256;
   return leds.Color(255, WheelPos*4, 0);
  } else if(WheelPos < 384) {
   WheelPos -= 320;
   return leds.Color(255-(WheelPos*4), 255, 0);
  }
}

// Sets all LEDs to off, but DOES NOT update the display;
// call leds.show() to actually turn them off after this.
void clearLEDs()
{
  for (int i=0; i<LED_COUNT; i++)
  {
    leds.setPixelColor(i, 0);
  }
}


void initializeESP8266()
{
  // esp8266.begin() verifies that the ESP8266 is operational
  // and sets it up for the rest of the sketch.
  // It returns either true or false -- indicating whether
  // communication was successul or not.
  // true
  int test = esp8266.begin();
  if (test != true)
  {
    Serial.println(F("Error talking to ESP8266."));
  }
  Serial.println(F("ESP8266 Shield Present"));
}

void connectESP8266()
{
  // The ESP8266 can be set to one of three modes:
  //  1 - ESP8266_MODE_STA - Station only
  //  2 - ESP8266_MODE_AP - Access point only
  //  3 - ESP8266_MODE_STAAP - Station/AP combo
  // Use esp8266.getMode() to check which mode it's in:
  int retVal = esp8266.getMode();
  if (retVal != ESP8266_MODE_STA)
  { // If it's not in station mode.
    // Use esp8266.setMode([mode]) to set it to a specified
    // mode.
    retVal = esp8266.setMode(ESP8266_MODE_STA);
    if (retVal < 0)
    {
      Serial.println(F("Error setting mode."));
    }
  }
  Serial.println(F("Mode set to station"));

  // esp8266.status() indicates the ESP8266's WiFi connect
  // status.
  // A return value of 1 indicates the device is already
  // connected. 0 indicates disconnected. (Negative values
  // equate to communication errors.)
  retVal = esp8266.status();
  if (retVal <= 0)
  {
    Serial.print(F("Connecting to "));
    Serial.println(mySSID);
    // esp8266.connect([ssid], [psk]) connects the ESP8266
    // to a network.
    // On success the connect function returns a value >0
    // On fail, the function will either return:
    //  -1: TIMEOUT - The library has a set 30s timeout
    //  -3: FAIL - Couldn't connect to network.
    retVal = esp8266.connect(mySSID, myPSK);
    if (retVal < 0)
    {
      Serial.println(F("Error connecting"));
      leds.setPixelColor(LED_COUNT-1, leds.Color(15, 0, 0));
      leds.show();
      errorLoop();
    }
  }
}

void errorLoop()
{
  for (;;) {    
  }
}

void displayConnectInfo()
{
  char connectedSSID[24];
  memset(connectedSSID, 0, 24);
  // esp8266.getAP() can be used to check which AP the
  // ESP8266 is connected to. It returns an error code.
  // The connected AP is returned by reference as a parameter.
  int retVal = esp8266.getAP(connectedSSID);
  if (retVal > 0)
  {
    Serial.print(F("Connected to: "));
    Serial.println(connectedSSID);
  } else {
      leds.setPixelColor(LED_COUNT-1, leds.Color(15, 0, 0));
      leds.show();    
  }

  // esp8266.localIP returns an IPAddress variable with the
  // ESP8266's current local IP address.
  IPAddress myIP = esp8266.localIP();
  Serial.print(F("My IP: ")); Serial.println(myIP);
}

