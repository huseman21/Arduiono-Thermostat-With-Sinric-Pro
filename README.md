This is a home thermostat created to run on an ESP8266 using the Sinricpro alexa voice interface. For more information see https://sinric.pro/index.html

thermostat files include, --Just add both files to your project and compile, the RemoteTempSender.ino is standallone for the remote temurature sending addon project.
thermostat.ino
handleTemperaturesensor.ino


Other components used in the thermostat are:
LiquidCrystal_I2c.h for lcd display SongHe IIC I2C TWI 1602 Serial LCD Module Display for Arduino R3 Mega 2560 16x2
DHT.h temperature sensor DHT11
RH_ask.h Rf 433mhz transmitter/receiver module for recieving remote tempurature readings




I also included a remote tempurature sender that runs on any standard arduino board

Other components used in the remote temp sender:
PCD8544.h to drive the Nokia 5110 serial LCD
DHT.h temperature sensor DHT11
RH_ask.h Rf 433mhz transmitter/receiver module for sending remote tempurature readings


As with most arduino projects these require many addon libraries.  If anyone shows interest in this code I may include links and such for the libraries.  For now you can view the code and find what libraries should be included.  I compiled this using arduino version 1.8.13  If you need any help just ask.
