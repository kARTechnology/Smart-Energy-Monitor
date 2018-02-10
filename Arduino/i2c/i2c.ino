#include <Wire.h>
#include "EmonLib.h"
#define VOLT_CAL 350
//pins setting...
const int relayPin1 = 5;
const int relayPin2 = 6;
const int relayPin3 = 7;
const int relayPin4 = 8;
const int motionPin = 9;
const int currentPin1 = A7;
const int currentPin2 = A6;
const int voltagePin = 23;
const int endUserSetupPin = 2;

volatile bool endUserSetup = false;
bool netConnected = false;
char temp[30]; 
String str;

EnergyMonitor emon1;
float voltage, current1, current2;
int  motion;

void setRelayState(int relaynum, int state) {
  if (state == 1)          digitalWrite(relaynum, LOW);
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
  voltage  =  getVoltage();
  current1 = getCurrent(currentPin1, 66); 
  current2 = getCurrent(currentPin2, 66);
  motion   = getMotionStatus();
}

float getCurrent(int sensorIn, int mVperAmp)
{
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(sensorIn);
    // see if you have a new maxValue
    if (readValue > maxValue)
      maxValue = readValue; /*record the maximum sensor value*/
    if (readValue < minValue)
      minValue = readValue; /*record the minimum sensor value*/
  }
  // Subtract min from max and get peak to peak current
  float p2p = ((maxValue - minValue) * 5.0) / 1024.0;
  double  rms = (p2p / 2.0) * 0.707; // 2.5V DC offset For a sine wave, the rms value is 0.707 times (that is root 2) the peak value
  return (rms * 1000) / mVperAmp;
}

float getVoltage() {
  emon1.calcVI(20, 2000);
  return emon1.Vrms;
}

int  getMotionStatus() {
  return  digitalRead(motionPin) == true ? 1 : 0;
}

/* i2c events */
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
  str = String(voltage) + "," + String(current1*voltage) + "," + String(current2*voltage) + "," + String(motion);
  Serial.println(str);

  // Convert String to char array to send via i2c
  int i;
  for (   i = 0 ; i < str.length() ; i++ ) {
    temp[i] =   str[i];
  }
  temp[i++] = '^';
  temp[i++] = '~';
  temp[i++] = '^';
  Wire.write(temp);
}
