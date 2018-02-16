#include <Wire.h>
#include "EmonLib.h"
#define VOLT_CAL 350
char my_str[] = "230,1.5,2.3,1^~^";
//pins setting...
const int relayPin1 = 5;
const int relayPin2 = 6;
const int relayPin3 = 7;
const int relayPin4 = 8;
const int motionPin = 9;
const int currentPin1 = 19;
const int currentPin2 = 22;
const int voltagePin = 23;
const int endUserSetupPin = 2;

volatile bool endUserSetup = false;
bool netConnected = false;

EnergyMonitor emon1;
float voltage, current1, current2;
int  motion;


void setRelayState(int relaynum, int state) {
  if (state == 1)     digitalWrite(relaynum, LOW);
  else if (state == 0)     digitalWrite(relaynum, HIGH);
}
void startEndUserSetup() {
  Serial.println("endusersetup initiated by user physically...");
  endUserSetup = true;
}
void setup()
{
  //set digital pins
  pinMode(LED_BUILTIN, OUTPUT);       //built in led
  pinMode(relayPin1, OUTPUT);         //relay
  pinMode(relayPin2, OUTPUT);         //relay
  pinMode(relayPin3, OUTPUT);         //relay
  pinMode(relayPin4, OUTPUT);         //relay
  pinMode(motionPin, INPUT);          //motion

  //turn off relays
  setRelayState(relayPin4, 0);        //relay
  setRelayState(relayPin3, 0);        //relay
  setRelayState(relayPin2, 0);        //relay
  setRelayState(relayPin1, 0);        //relay

  //for endUserSetup
  attachInterrupt(digitalPinToInterrupt(endUserSetupPin), startEndUserSetup, RISING); //endUserSetup button

  //for I2C communication to ESP
  Wire.begin(2);                        // join i2c bus with address #2
  Wire.onRequest(requestEvent);         // register event
  Wire.onReceive(receiveEvent);         // register event

  //for Serial communication to USB-Computer
  Serial.begin(9600);                       //initializes the serial connection at 9600 bps
  Serial.println(F("setup complete"));

  //voltage library setup
  emon1.voltage(voltagePin, VOLT_CAL, 1.7); //input pin, calibration, phase_shift
}



void loop()
{
  voltage =  getVoltage();
  current1 = getCurrent(currentPin1, 66);
  current2 =  getCurrent(currentPin2, 66);
  motion = getMotionStatus();
}

float getCurrent(int sensorIn, int mVperAmp)
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(sensorIn);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the minimum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 5.0) / 1024.0;


  double  VRMS = (result / 2.0) * 0.707; //root 2 is 0.707
  return (VRMS * 1000) / mVperAmp;
}


float getVoltage() {
  emon1.calcVI(20, 2000);
  return emon1.Vrms;
}
int  getMotionStatus() {
  return  digitalRead(motionPin) ==true? 1:0;
}


void receiveEvent(int howMany) //howMany=num. of bytes received
{
  int  a, b;
  Serial.print("Received: ");
  a = Wire.read() - '0';
  b = Wire.read() - '0';
  Serial.print(a);
  Serial.print("=");
  Serial.println(b);

  switch (a)
  {
    case 5:   digitalWrite(LED_BUILTIN, b); break;       //set LED_BUILTIN
    case 4:   setRelayState(relayPin4, b); break;
    case 3:   setRelayState(relayPin3, b); break;
    case 2:   setRelayState(relayPin2, b); break;
    case 1:   setRelayState(relayPin1, b); break;
    case 0:   netConnected = b;                          //set net status
  }
}
char txt[30]; String str;
void requestEvent()
{
  Serial.println(F("Requested..."));
  if (endUserSetup)
  {
    Serial.println("endusersetup request sent in RequestEvent...");
    Wire.write("endusersetup^~^");
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

  Serial.println(F("hola"));
  int i ;
  for (   i = 0 ; i < str.length() ; i++ ) {
    txt[i] =   str[i];
  }
  txt[i++] = '^';
  txt[i++] = '~';
  txt[i++] = '^';

  Serial.println(F("hola"));

  Wire.write(txt);

}
