 /*=
 * 
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 
#include <EEPROM.h>
#include <Arduino.h>
#ifdef ESP8266 
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "SinricPro.h"
#include "SinricProThermostat.h"
#include "DHT.h" // https://github.com/markruys/arduino-DHT
//#include "DHTesp.h" 





#include <RH_ASK.h>  //for am radio
#ifdef RH_HAVE_HARDWARE_SPI //for am radio
#include <SPI.h> // Not actually used but needed to compile  //for am radio
#endif

//RH_ASK rf_driver;
//RH_ASK driver(2000, 0); // ESP8266 or ESP32: do not use pin 11 or 2
//RH_ASK driver(2000, 16, 0, 0); // ESP8266 or ESP32: do not use pin 11 or 2
RH_ASK driver(2000, 16, 15, 10, false); // ESP8266: do not use pin 11
int onetime=1;



#define WIFI_SSID         "Mtorabit2.4g"    
#define WIFI_PASS         "@voyager42211"
#define APP_KEY           "e45ea5d9-5e66-44a5-a5d7-14d549ae1295"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "65675679-0a26-4c20-8ef3-9bc5e4d745d6-573c5aa5-d039-43c5-848c-623e6c473c86"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define THERMOSTAT_ID     "5ffc1d2ddfd58f56bc43b275"    // production Thermostat
#define BAUD_RATE         9600                 // Change baudrate to your need
#define EVENT_WAIT_TIME  60000          // send event every 60 seconds 60000   120000 
#define DHT_PIN    2   // DHT temp sensor pin 
#define BUTTON_PIN 14   // GPIO for BUTTON to turn off thermostat
const int relayH = 13;   //pin D2
const int relayC = 12;  //pin D8
#define LED_PIN   2   // GPIO for LED (inverted)
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x3F for a 16 chars and 2 line display
DHT dht;                                      // DHT sensor

float globalTemperature;
bool globalPowerState;
float temperature1;                            // actual temperature
float humidity=0; 
float average=0; 
float lastTemperature;                        // last known temperature (for compare)
unsigned long lastEvent = (-EVENT_WAIT_TIME); // last time event has been sent
unsigned long AutoWaitTime = 900000;   //wait between auto mode switching 1800000=30min  900000=15min 450000=7.5min 60000=1min
unsigned long lastCheck = 0 ; // last time event has been sent
unsigned long LastCheckLight = 0;  //used to turn off led backlight
unsigned long ShowMode = 0;  //screen timing
unsigned long ShowTemp = 0;  //screen timing
unsigned long radiotimer =0;  //used for amradio fallback
//part of the temp averaging
const int numReadings = 10;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
byte modeset;
int fail=0;
unsigned long lastBtnPress = 0;

long heat=false;
long cool=false;
unsigned long automillis;
int SkipModeTimer=0; //used to skip mode timer on boot
int globalTemperatureI;
int averageI;
int humidityI;
int firstrun=0;
int SkipWaitTimer=0;
byte Toggle=1;
int remote=0;
String rcv;
//----------------------------




bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Thermostat %s turned %s\r\n", deviceId.c_str(), state?"on":"off");
  globalPowerState = state; 

  EEPROM.write(0,globalPowerState); //state=0 off state=1 on
  EEPROM.commit();
  digitalWrite(LED_PIN, globalPowerState?LOW:HIGH); 
    lcd.clear();
    lcd.setBacklight(255);
if (globalPowerState==true) {
  
lcd.setCursor(0, 0);
lcd.print("Thermostat");
lcd.setCursor(0, 1);  
 lcd.print("On");
 SkipModeTimer=0;
}else{
lcd.setCursor(0, 0);
lcd.print("Power");
lcd.setCursor(0, 1);
lcd.print("Off");
 lcd.setBacklight(0);
  digitalWrite(relayH, HIGH);
  digitalWrite(relayC, HIGH);
  EEPROM.commit();
}  
delay(1000);
  return true; // request handled properly
}

bool onTargetTemperature(const String &deviceId, float &temperature) {
  Serial.printf("Thermostat %s set temperature to %f\r\n", deviceId.c_str(), temperature);
  globalTemperature = temperature;
  globalTemperatureI = (int) globalTemperature; //convert float to integer
   lcd.clear();
   lcd.setBacklight(255);
lcd.setCursor(0, 0);
lcd.print("Updated Temp");
 lcd.setCursor(0, 1);  
lcd.print(globalTemperatureI);
lcd.setCursor(0, 15);
lcd.print("fahrenheit");
SkipModeTimer=0;

//EEPROM.write(11,globalTemperatureI); // save user configured temp to eeprom

 //for (int i = 0; i <  temperature.length(); ++i)
 // {
   // EEPROM.write(10 + i,  temperature[i]);
   // Serial.print( temperature[i]); Serial.print("");
 // }
 //EEPROM.put(1,globalTemperatureI);
 EEPROM.put(1,globalTemperature);

 
//Serial.println("just saved global temp to eeprom 20");

 //Serial.print(globalTemperatureI); Serial.println(" globaltermuratureI");
//Serial.print(globalTemperature); Serial.println(" globaltermurature");
//Serial.print(EEPROM.read(1));  Serial.println(" eeprom 1");
//Serial.println("all done");


}

bool onAdjustTargetTemperature(const String & deviceId, float &temperatureDelta) {
  globalTemperature += temperatureDelta;  // calculate absolut temperature
  Serial.printf("Thermostat %s changed temperature about %f to %f", deviceId.c_str(), temperatureDelta, globalTemperature);
  temperatureDelta = globalTemperature; // return absolut temperature
   globalTemperatureI = (int) globalTemperature; //convert float to integer
 // EEPROM.write(12,globalTemperatureI); // save user configured temp to eeprom
    EEPROM.put(1,globalTemperature);
  SkipModeTimer=0;
  return true;
}

bool onThermostatMode(const String &deviceId, String &mode) {
  Serial.printf("Thermostat %s set to mode %s\r\n", deviceId.c_str(), mode.c_str());
if ( mode[0] == 'A' ) {
  modeset=1;
//EEPROM.put(2,modeset); //save mode to eeprom
lcd.clear();
lcd.setBacklight(255);
lcd.setCursor(0, 0);
lcd.print("Updated Mode");
lcd.setCursor(0, 1);
lcd.print("Auto");
SkipModeTimer=0; 
}
if ( mode[0] == 'H' ) {
lcd.clear();
lcd.setBacklight(255);
lcd.setCursor(0, 0);
lcd.print("Updated Mode");
lcd.setCursor(0, 1);
lcd.print("Heater on");
digitalWrite(relayC, HIGH);  //turn off other relays
  modeset=2;
  SkipModeTimer=0;
}
if ( mode[0] == 'C' ) {
lcd.clear();
lcd.setBacklight(255);
lcd.setCursor(0, 0);
lcd.print("Updated Mode");
lcd.setCursor(0, 1);
lcd.print("Cooling on");
digitalWrite(relayH, HIGH);  //turn off other relays
  modeset=3;
  SkipModeTimer=0;
}

EEPROM.write(10,modeset); //save mode to eeprom
  return true;
  
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
}

void setupSinricPro() {

  SinricProThermostat &myThermostat = SinricPro[THERMOSTAT_ID];
 SinricPro.restoreDeviceStates(true); //updates with saved setting from sinric :) 
 //SinricPro.restoreDeviceStates(false); //updates with saved setting from sinric :) 
  myThermostat.onTargetTemperature(onTargetTemperature);
  myThermostat.onAdjustTargetTemperature(onAdjustTargetTemperature);
  myThermostat.onThermostatMode(onThermostatMode);
  myThermostat.onPowerState(onPowerState);
  
  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); });
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void onoffbutton ()  {
  unsigned long actualMillis = millis(); 
if (digitalRead(BUTTON_PIN) == LOW && actualMillis - lastBtnPress > 1000)  {  //button for power on or off toggle.
if (globalPowerState) {     // flip myPowerState: if it was true, set it to false, vice versa
globalPowerState = false;
lcd.clear();
lcd.setBacklight(255);
lcd.setCursor(0, 0);
lcd.print("Thermostat");
lcd.setCursor(0, 1);
lcd.print("Off");
delay(5000);
lcd.setBacklight(0);
digitalWrite(relayH, HIGH); //turn off relays
digitalWrite(relayC, HIGH); //turn off relays 
      SkipModeTimer=0;
    } else {
      globalPowerState = true;
lcd.clear();
lcd.setBacklight(255);
lcd.setCursor(0, 0);
lcd.print("Thermostat");
lcd.setCursor(0, 1);  
lcd.print("On"); 
delay(5000);  
      SkipModeTimer=0;
    }
   digitalWrite(LED_PIN, globalPowerState?LOW:HIGH); // if globalPowerState indicates device turned on: turn on led (builtin led uses inverted logic: LOW = LED ON / HIGH = LED OFF)   
SinricProThermostat &myThermostat = SinricPro[THERMOSTAT_ID];
myThermostat.sendPowerStateEvent(globalPowerState); 
Serial.printf("Device %s turned %s (manually via flashbutton)\r\n",  myThermostat.getDeviceId().toString().c_str(), globalPowerState?"on":"off");
lastBtnPress = actualMillis;  // update last button press variable
} 
}


void DisplayTemp() {
if (globalPowerState == false)return; // device is off...do nothing 
if (automillis - ShowTemp < 5000) return;
ShowTemp=automillis;

if (Toggle == 1) {
  lcd.clear();
    globalTemperatureI = (int) globalTemperature; //convert float to integer
    averageI = (int) average; //convert float to integer
    humidityI= (int) humidity; //convert float to integer


if (remote==0) {
lcd.setCursor(0, 0);
lcd.print("Temperature");
lcd.setCursor(12, 0);  
lcd.print(averageI);
}else{
  
lcd.setCursor(0, 0);
lcd.print("Remote Temp");
lcd.setCursor(12, 0);  
lcd.print(rcv);


//temperature1=rcv.toFloat();


  
}
lcd.setCursor(15, 0);  
lcd.print("F");
lcd.setCursor(0, 1); 
lcd.print("Humidity");
lcd.setCursor(12, 1);
lcd.print(humidityI);
lcd.setCursor(15, 1);
lcd.print("%");
 
}

if (Toggle == 2) {
Toggle=1;
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Mode");

if (modeset == 1) {   //if mode set to heat  (1=auto 2=heat 3=air)
  lcd.setCursor(0, 1);
  lcd.print("Auto");


if ((digitalRead(relayH) == LOW) || (digitalRead(relayC) == LOW)) {
  if (cool==true) {
  lcd.setCursor(8, 1);
  lcd.print("cooling");
  }else{
  lcd.setCursor(8, 1); 
  lcd.print("Heating");
 }
}else{
lcd.setCursor(8, 1);
 lcd.print("Standby");
  
}
}
if (modeset == 2) {   //if mode set to heat  (1=auto 2=heat 3=air)
    lcd.setCursor(0, 0);
    lcd.print("Mode");
    lcd.setCursor(0, 1);
    lcd.print("Heat");
if (digitalRead(relayH) == LOW) {
   lcd.setCursor(8, 1);
    lcd.print("Heating");
}else{
  lcd.setCursor(8, 1);
  lcd.print("Standby");
}

    
   
       
}
if (modeset == 3) {   //if mode set to heat  (1=auto 2=heat 3=air)
     lcd.setCursor(0, 0);
    lcd.print("Mode");
    lcd.setCursor(0, 1);
    lcd.print("AC");
if (digitalRead(relayC) == LOW)  { 
    lcd.setCursor(8, 1);
    lcd.print("Cooling");
}else{
  lcd.setCursor(8, 1);
  lcd.print("Standby");
}
}
    lcd.setCursor(8, 0);
    lcd.print("Set");
    lcd.setCursor(12, 0);
   lcd.print(globalTemperatureI);
   lcd.setCursor(15, 0);
   lcd.print("F");
 return;   
}


  Toggle=2;
}






void amradio() {
if (globalPowerState == false) return; // device is off...do nothing

uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
uint8_t buflen = sizeof(buf);

if (driver.recv(buf, &buflen)) {
remote=1;
rcv = "";
// Message with a good checksum received, dump it.
//String rcv = "";
for (int i = 0; i <buflen; i++){
  rcv +=(char)buf[i];
  }
//Serial.println("receved external temp");
//Serial.println(rcv);
radiotimer=automillis;
//temperature1=rcv.toFloat();
}
if ((automillis - radiotimer) > 450000) remote=0; //fall back to internal temp if no remote temp receved 



}

void setup() {

  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  dht.setup(DHT_PIN);
  setupWiFi();
  Wire.begin();
  EEPROM.begin(512);
  lcd.init(); // initialize the lcd
  //lcd.init();
  lcd.setBacklight(255);
  lcd.setCursor(0,0);
  lcd.print("Huseman");
  lcd.setCursor(0,1);
  lcd.print("Thermostats");
  pinMode(relayH, OUTPUT);  //heating relay
  pinMode(relayC, OUTPUT);   // cooling relay
  pinMode(LED_PIN, OUTPUT);  //on off led
  digitalWrite(relayH, HIGH); //turn off relays
  digitalWrite(relayC, HIGH); //turn off relays

  pinMode(BUTTON_PIN, INPUT_PULLUP); // GPIO 0 as input, pulled high for on off switch

delay(3000);
 lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  delay(3000);


 // initialize all the readings to 0: for temp averaging code
 temperature1 = dht.getTemperature() * 1.8f + 32;  // get actual temperature in °F
 for (int thisReading = 0; thisReading < numReadings; thisReading++) {
 readings[thisReading] = 0;}
do {
readings[readIndex] =  temperature1;
total = total + readings[readIndex];
  readIndex = readIndex + 1;
} while (readIndex <= 8);

if (!driver.init())
     Serial.println("init failed");

Serial.println();
EEPROM.get(0, globalPowerState);
EEPROM.get(1, globalTemperature);
EEPROM.get(10, modeset);

//Serial.println("reading settings from eeprom");
//Serial.print(globalPowerState); Serial.println(" globalpowerstate");
//Serial.print(globalTemperature); Serial.println(" globalTemperature");
//Serial.print(globalTemperatureI); Serial.println(" globalTemperatureI");
//Serial.print(modeset); Serial.println(" modeset");
//Serial.println();
//EEPROM.commit();
if (globalPowerState==0) {
globalPowerState == false;
lcd.clear(); 
lcd.setCursor(0, 0);
lcd.print("Power");
lcd.setCursor(0, 1);
lcd.print("Off");
lcd.setBacklight(0);
digitalWrite(relayH, HIGH);
digitalWrite(relayC, HIGH); 
 // ESP.deepSleep(10e6); // 
}else{
  globalPowerState == true;
}



setupSinricPro();


}
 
 
   
void loop() {
  SinricPro.handle();
  onoffbutton();
  handleTemperaturesensor();
  DisplayTemp();
  amradio();
  automillis = millis(); //i moved this to handtemp. if it breaks move it back 







}
