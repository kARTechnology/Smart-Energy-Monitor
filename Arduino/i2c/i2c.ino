#include <Wire.h>
#include "EmonLib.h"
#define VOLT_CAL 350

const int endUserSetupPin = 2;
const int relayPin1 = 7;
const int relayPin2 = 8;
const int relayPin3 = 6;
const int motionPin = 9;

const int voltagePin = A0;
const int currentPin1 = A1;
const int currentPin2 = A2;

EnergyMonitor emon1;
volatile int  motion, voltage, current1, current2;
volatile bool endUserSetup = false, netConnected = false;
char txt[30]; String str;

void startEndUserSetup() {
  Serial.println("endusersetup initiated by user physically...");
  digitalWrite(LED_BUILTIN, HIGH);
  endUserSetup = true;
}
void setRelayState(int relaynum, int state) {
  if (state == 1)     digitalWrite(relaynum, LOW);
  else if (state == 0)     digitalWrite(relaynum, HIGH);
}
float getVoltage() {
  emon1.calcVI(20, 500);
  return emon1.Vrms;
}
int  getMotionStatus() {
  return  digitalRead(motionPin) == true ? 1 : 0;
}
float getCurrent(int sensorIn )
{
  int readValue;                  //value read from the sensor
  int maxValue = 0;               // store max value here
  int minValue = 1024;            // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(sensorIn);
    if (readValue > maxValue)   maxValue = readValue;
    if (readValue < minValue)   minValue = readValue;
  }
  double Voltage = ((maxValue - minValue) * 5.0) / 1024.0;
  double VRMS = (Voltage / 2.0) * 0.707;
  double AmpsRMS = (VRMS * 1000) / 66; //66=sensitivity for 30A sensor.
  return AmpsRMS;
}
void setup()
{
  pinMode(motionPin, INPUT);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  setRelayState(relayPin3, 0);
  setRelayState(relayPin2, 0);
  setRelayState(relayPin1, 0);
  attachInterrupt(digitalPinToInterrupt(endUserSetupPin), startEndUserSetup, RISING); //endUserSetup button
  Wire.begin(2);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  Serial.println(F("setup complete"));
  emon1.voltage(voltagePin, VOLT_CAL, 1.7);
}
void loop()
{
  voltage =  getVoltage();
  current1 =  digitalRead(relayPin1) == 0 ? (getCurrent(currentPin1) - 0.20) * voltage : 0;
  current2 = digitalRead(relayPin2) == 0 ? (getCurrent(currentPin2) - 0.20) * voltage : 0;;
  motion = getMotionStatus();
  str = String(voltage) + "," + String(current1) + "," + String(current2) + "," + String(motion);
}

void receiveEvent(int howMany) //howMany=num. of bytes received
{
  int a = Wire.read() - '0'; //Convert char to int
  int b = Wire.read() - '0'; //Convert char to int
  Serial.print(F("Received: "));  Serial.print(a);  Serial.print(F("="));  Serial.println(b);

  switch (a)  {
    case 3:   setRelayState(relayPin3, b); break;
    case 2:   setRelayState(relayPin2, b); break;
    case 1:   setRelayState(relayPin1, b); break;
    case 0:   netConnected = b;
  }
}
void requestEvent()
{
  Serial.println(F("Requested..."));
  if (endUserSetup)
  {
    Serial.println(F("endusersetup request sent in RequestEvent..."));
    Wire.write("endusersetup^~^");
    digitalWrite(LED_BUILTIN, LOW);
    endUserSetup = false;
    return;
  }
  if (!netConnected)
  {
    Serial.println(F("Waiting for net"));
    Wire.write("Waiting for net^~^");
    return;
  }
  str = String(voltage) + "," + String(current1) + "," + String(current2) + "," + String(motion);
  Serial.println(str + " - SENT");
  int i;
  for ( i = 0 ; i < str.length() ; i++ )    txt[i] = str[i];
  txt[i++] = '^';
  txt[i++] = '~';
  txt[i++] = '^';
  Wire.write(txt);
}
