#include <Wire.h>
#include "EmonLib.h"
#define VOLT_CAL 350
const uint8_t SENS = 66; // sensor sensitivity from datasheet in mV/A. 5A sensor=185, 20A=100, 30A=66
const uint8_t AC_FREQUENCY = 50; // in Hz

const int relayPin1 = 5;
const int relayPin2 = 6;
const int relayPin3 = 7;
const int relayPin4 = 8;
const int motionPin = 9;
const int currentPin1 = A6;
const int currentPin2 = A7;
const int voltagePin = 23;
const int endUserSetupPin = 2;

volatile bool endUserSetup = false;
bool netConnected = false;
char txt[30]; String str;

EnergyMonitor emon1;
 volatile int  motion,voltage,current1, current2;


void setRelayState(int relaynum, int state) {
  if (state == 1)     digitalWrite(relaynum, LOW);
  else if (state == 0)     digitalWrite(relaynum, HIGH);
}
void startEndUserSetup() {
  Serial.println("endusersetup initiated by user physically...");
  digitalWrite(LED_BUILTIN, HIGH);
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
  current1 = digitalRead(relayPin2) ? 0:getCurrent(currentPin1)*voltage ;
  current2 =  digitalRead(relayPin3)? 0: getCurrent(currentPin2)*voltage;
  motion = getMotionStatus();
}

int32_t readVcc()
// Calculate current Vcc in mV from the 1.1V reference voltage
{
  int32_t result = 5000L;

  // Read 1.1V reference against AVcc - hardware dependent
  // Set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif  
  
#if defined(__AVR__)
  delay(2);         // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);    // Convert, result is stored in ADC at end
  while (bit_is_set(ADCSRA,ADSC)); // measuring
  result = ADCL; // must read ADCL (low byte) first - it then locks ADCH 
  result |= ADCH<<8; // // unlocks both

  result = 1125300L / result; // Back-calculate AVcc in mV; 1125300 = 1.1*1023*1000
#endif
  
  return(result);
}
float getCurrent(int sensorIn )
{
 const uint8_t timeSampling = 2 * (1000 / AC_FREQUENCY);
  
  int32_t convertedmA;
  uint16_t sampleMin = 1024, sampleMax = 0;
  int32_t Vpp, Vcc = readVcc();   // both in milliVolts

  // Collect samples over the time sampling period and 
  // work out the min/max readings for the waveform
  for (uint32_t timeStart = millis(); millis() - timeStart < timeSampling; )
  {
    int16_t sensorValue = analogRead(sensorIn) ;
   
    if (sensorValue > sampleMax) sampleMax = sensorValue;
    if (sensorValue < sampleMin) sampleMin = sensorValue; 
  }

  // now calculate the current
  Vpp = (((sampleMax-sampleMin)/2)*Vcc)/1024L;
  convertedmA = (707L * Vpp)/SENS;  // 1/sqrt(2) = 0.7071 = 707.1/1000

 

  return(convertedmA/1000.000);
}


float getVoltage() {
  emon1.calcVI(20, 2000);
  return emon1.Vrms;
}
int  getMotionStatus() {
  return  digitalRead(motionPin) == true ? 1 : 0;
}


void receiveEvent(int howMany) //howMany=num. of bytes received
{
  int  a, b;
  a = Wire.read() - '0'; //Convert char to int
  b = Wire.read() - '0'; //Convert char to int
  Serial.print(F("Received: "));  Serial.print(a);  Serial.print(F("="));  Serial.println(b);

  switch (a)
  {
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
  str = String(voltage) + "," + String(current2) + "," + String(current1) + "," + String(motion);
  Serial.println(str);
  int i ;
  for ( i = 0 ; i < str.length() ; i++ ) {
    txt[i] = str[i];
  }
  txt[i++] = '^';
  txt[i++] = '~';
  txt[i++] = '^';

  Wire.write(txt);
}
