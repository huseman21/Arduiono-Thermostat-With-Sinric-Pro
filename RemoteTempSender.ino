#include <RH_ASK.h>
#include <SPI.h>
#include "DHT.h"
#define DHT_PIN    11
DHT dht;
RH_ASK driver(2000, 2);
float h;
float t;
int temp;
unsigned long lastEvent;





#include <PCD8544.h>
static PCD8544 lcd;
static int counter = 0;

static const byte ledPin = 8; //used to enable backlight on lcd



// A bitmap graphic (10x2) of a thermometer...
static const byte THERMO_WIDTH = 10;
static const byte THERMO_HEIGHT = 2;
static const byte thermometer[] = { 0x00, 0x00, 0x48, 0xfe, 0x01, 0xfe, 0x00, 0x02, 0x05, 0x02,
                                    0x00, 0x00, 0x62, 0xff, 0xfe, 0xff, 0x60, 0x00, 0x00, 0x00
                                  };
// A custom "degrees" symbol...
static const byte DEGREES_CHAR = 1;
static const byte degrees_glyph[] = { 0x00, 0x07, 0x05, 0x07, 0x00 };


static const byte THERMOT_WIDTH = 10;
static const byte THERMOT_HEIGHT = 3;
static const byte antenna[] = {0x11, 0x0e, 0xfa, 0x0e, 0x11};

void setup() {
  Serial.begin(9600);
  driver.init(); //define ask driver
  dht.setup(DHT_PIN);
  Serial.print("Started");
  lcd.begin(84, 48);
  // Register the custom symbol...
  lcd.createChar(DEGREES_CHAR, degrees_glyph);
  pinMode(ledPin, OUTPUT);
lcd.setContrast(50);
  //analogReference(INTERNAL);
  //digitalWrite(ledPin, HIGH); //turns on backlight
  lcd.setCursor(0, 0);
  lcd.print("Thermometer");
  lcd.setCursor(0, 1);
  lcd.print("Created by");
  lcd.setCursor(0, 3);
  lcd.print("Paul Huseman");
  delay(5000);
  lcd.setCursor(0, 0);
  lcd.print("           ");
  lcd.setCursor(0, 1);
  lcd.print("         ");
  lcd.setCursor(0, 3);
  lcd.print("            ");






}


void TransmitTemp() {
  unsigned long actualMillis = millis();
  if (actualMillis - lastEvent < 30000 ) return; //send every minute. 60000
  lastEvent = actualMillis;       // save actual time for next compare

  

  h = dht.getHumidity();
  t = dht.getTemperature() * 1.8f + 32;
  delay(500);
  if (isnan(t)) {
    Serial.print("DHT reading failed! \r\n");  // print error message
    return;                                    // try again next time
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print("°F");
  Serial.print(" Transmitted");
  Serial.println();

lcd.setCursor(15, 5);
  lcd.print("Transmiting");

  //convert float to integer
  temp = (int) t;
  char cstr[16];
  itoa(temp , cstr, 10);
  const char *msg = cstr;

  //const char *msg = "Hello World 123456789";
  driver.send((uint8_t *)msg, strlen(msg));
  driver.waitPacketSent();



  delay(3000);

  lcd.setCursor(15, 5);
  lcd.print("           ");
}


void display() {
  digitalWrite(ledPin, HIGH); //turns on backlight
  // Draw the thermometer bitmap at the bottom left corner...
  lcd.setCursor(0, 48 / 8 - THERMO_HEIGHT);
  lcd.drawBitmap(thermometer, THERMO_WIDTH, THERMO_HEIGHT);

  //lcd.setCursor(70, 48/8 - THERMOT_HEIGHT);
  //lcd.drawBitmap(antenna, THERMOT_WIDTH, THERMOT_HEIGHT);



  lcd.setCursor(77, 16);
  lcd.drawBitmap(antenna, 10, 1);

  lcd.setCursor(10, 0);
  lcd.print("Tempurature");
  lcd.setCursor(31, 1);
  lcd.print(t, 1);
  lcd.print(" \001F ");
  lcd.setCursor(18, 3);
  lcd.print("Humidity");
  lcd.setCursor(30, 4);
  lcd.print(h);
  lcd.print(" % ");

  //lcd.setCursor(0, 0);
  // lcd.print("Hello, World!");
  //lcd.setCursor(0, 1);
  // lcd.print(counter, DEC);
  // lcd.write(' ');
  // counter++;


  // delay(200);
}






void loop() {
  display();
  TransmitTemp();








}
