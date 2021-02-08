void handleTemperaturesensor() {


  
if (firstrun==0){   //delay 8 seconds on startup before running handletemperaturesensors to allow server updates before hand
 unsigned long actualMillis = millis();
 SkipModeTimer = 1;
  if (actualMillis < 20000) return;
}
firstrun=1;

if (globalPowerState == false) return; // device is off...do nothing

  unsigned long actualMillis = millis();
  if ((actualMillis - lastEvent) < EVENT_WAIT_TIME) {   //only check every EVENT_WAIT_TIME milliseconds
if (SkipModeTimer == 0) {  //Skips event wait time if user changes mode or temp.
goto skiptimer3;
}   
return;
}else{
lastEvent = actualMillis;       // save actual time for next compare
}
skiptimer3:

if (remote==0) {
  temperature1 = dht.getTemperature() * 1.8f + 32;  // get actual temperature in °F
  humidity = dht.getHumidity();                // get actual humidity
   Serial.print("Using internal thermometer");
}else{
 temperature1=rcv.toFloat();
 humidity = dht.getHumidity();                // get actual humidity 
Serial.println("Using external thermometer");
Serial.println(rcv);
Serial.println(temperature1);


 
}

// reading dht temp failed disable relays after 3 errors or reset count if good reading

  if (isnan(temperature1)) { 
    Serial.printf("DHT reading failed! disabling heating and cooling untill resovled\r\n");  // print error message
    if (fail > 2) {
  digitalWrite(relayH, HIGH);
  digitalWrite(relayC, HIGH);   
    }
   fail++;
    return;                                    // try again next time
  } 
fail=0;

//the temperature averaging code with 10 indexes
  total = total - readings[readIndex];  // subtract the last reading:
  readings[readIndex] =  temperature1;  // read from the sensor:
  total = total + readings[readIndex]; // add the reading to the total:
  readIndex = readIndex + 1; // advance to the next position in the array:
  if (readIndex >= numReadings) { // if we're at the end of the array...
    readIndex = 0;  // ...wrap around to the beginning:
  }
  average = total / numReadings; // calculate the average:
// end averaging code




if (SkipModeTimer == 0) {  //Skips no temp change abort send if user changes temp
goto skiptimer2;
} 
   if (average == lastTemperature) {   // if no values changed do nothing...
    Serial.print(average);   //add code hear if temp does not change for a long time to error out system!!!!!!!!!!!!!!!
    Serial.println(" no temperature change not updating server.");
    return; // if no values changed do nothing...
   }
skiptimer2:
SinricProThermostat &myThermostat = SinricPro[THERMOSTAT_ID];
bool success = myThermostat.sendTemperatureEvent  (average, humidity); // send tempurature data
if (success) {  // if event was sent successfuly, print temperature and humidity to serial
  Serial.printf("Temperature Sent: %2.1f Fahrenheit\tHumidity: %2.1f%%\r\n", average, humidity);
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("Updating Server");
 lcd.setCursor(0, 1);
 lcd.print("Successful");
 delay(5000);
  } else {  // if sending event failed, print error message
 Serial.printf("Something went wrong...could not send room tempurature Event to server!\r\n");
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("Updating Server");
 lcd.setCursor(0, 1);
 lcd.print("Failed!!!");
  delay(5000);
}
lastTemperature = average;  // save actual temperature for next compare
averageI = (int) average; //convert float to integer
humidityI= (int) humidity; //convert float to integer






SkipWaitOnUserChange:
/////////////////////// relay controls ///////////////////////
  if (modeset == 2) {   //if mode set to heat  (1=auto 2=heat 3=air)
    cool=false;                                     
    heat=true;
}

if (modeset == 3) {    //if mode set to Cool  (1=auto 2=heat 3=Cool)
if (globalTemperature < average) {        
  cool=true;                                     
  heat=false;
}
}

if (modeset == 1) {                              //if mode set to Auto-
 //Serial.println();
 //Serial.print(automillis /1000 /60);
 //Serial.print(automillis);
 //Serial.println(" automillis 30 minutes timer");
 //Serial.print(lastCheck);
 //Serial.println(" last check value");
 //Serial.print(AutoWaitTime);
 //Serial.println(" autowaittime");
 //Serial.println();

if (SkipModeTimer == 0) {
SkipModeTimer = 1 ;
goto skiptimer; 
}

if (automillis - lastCheck > AutoWaitTime) {  //mode checking wait timer.
skiptimer:
Serial.println("                                        autowaittime just elapsed  ");
lastCheck = automillis;
if (average+5 < globalTemperature)    {   
cool=false;                                     
heat=true;
}else{
heat=false;
cool=true;
}
}
}

if (heat==true){
 if (globalTemperature > average) {                  //heat
 if(abs(average - globalTemperature) >= 2) {  
 digitalWrite(relayC, HIGH);  //turn off other relays
 digitalWrite(relayH, LOW);
}
} else {
   digitalWrite(relayH, HIGH);
   lastCheck = automillis;
}   
}
if (cool==true){
if (globalTemperature < average) {          //Cool
  if(abs(globalTemperature - average) >= 2) {  
  digitalWrite(relayH, HIGH); //turn off other relay
  digitalWrite(relayC, LOW);
}
} else {
   digitalWrite(relayC, HIGH);
   lastCheck = automillis;
}
}

 //print status
Serial.println();
Serial.println("Mode 0=off 1=auto 2=heat 3=cool");
Serial.print(modeset);
if (cool==true) {
  Serial.println("-cooling mode enabled");
}
if (heat==true) {
  Serial.println("-Heating mode enabled");
}
if (cool==false && heat==false){
  Serial.println("-Standby");
}

Serial.println("Desired room temperature");
Serial.println(globalTemperature);
if (remote==0) {
Serial.println("Actual room temp - local sensor");
}else{
  Serial.println("Actual room temp Using remote sensor");
}
Serial.println(average);
  if (digitalRead(relayH) == LOW)  { 
   Serial.print("Heat relay ON   ");
  } else {
    Serial.print("Heat relay Off   ");
  }
 if (digitalRead(relayC) == LOW)  { 
   Serial.println("Cool relay ON");
  } else {
    Serial.println("Cool relay Off");
  }
 Serial.println();
 Serial.println();
 Serial.println();

//if i change from auto coooling to heat mode the ac does nto turn off.

 SkipModeTimer = 1;
}
