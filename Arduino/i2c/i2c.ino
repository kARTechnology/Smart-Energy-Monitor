#include <Wire.h>
#include "EmonLib.h"
#define VOLT_CAL 350
const int relayPin1 = 6;
const int relayPin2 = 7;
const int relayPin3 = 8;
const int motionPin = 9;
const int voltagePin = A0;
const int currentPin1 = A1;
const int currentPin2 = A2;
const int endUserSetupPin = 2;
EnergyMonitor emon1;
volatile int  motion, voltage, current1, current2;
volatile bool endUserSetup = false;
bool netConnected = false;
char txt[30]; String str;

void setRelayState(int relaynum, int state) {
  if (state == 1)     digitalWrite(relaynum, LOW);
  else if (state == 0)     digitalWrite(relaynum, HIGH);
}
float getVoltage() {
  emon1.calcVI(20, 2000);
  return emon1.Vrms;
}
int  getMotionStatus() {
  return  digitalRead(motionPin) == true ? 1 : 0;
}
void startEndUserSetup() {
  Serial.println("endusersetup initiated by user physically...");
  digitalWrite(LED_BUILTIN, HIGH);
  endUserSetup = true;
}
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(motionPin, INPUT);
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
  current1 =  getCurrent(currentPin1) * voltage ;
  current2 = getCurrent(currentPin2) * voltage;
  motion = getMotionStatus();
  str = String(voltage) + "," + String(current1) + "," + String(current2) + "," + String(motion);
  Serial.println(str);
}

int32_t readVcc()
{
  int32_t result = 5000L;

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
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  result = ADCL; // must read ADCL (low byte) first - it then locks ADCH
  result |= ADCH << 8; // // unlocks both

  result = 1125300L / result; // Back-calculate AVcc in mV; 1125300 = 1.1*1023*1000
#endif

  return (result);
}
float getCurrent(int sensorIn )
{
  const uint8_t timeSampling = 2 * (1000 / 50);
  int32_t convertedmA;
  uint16_t sampleMin = 1024, sampleMax = 0;
  int32_t Vpp, Vcc = readVcc();
  for (uint32_t timeStart = millis(); millis() - timeStart < timeSampling; )
  {
    int16_t sensorValue = analogRead(sensorIn) ;
    if (sensorValue > sampleMax) sampleMax = sensorValue;
    if (sensorValue < sampleMin) sampleMin = sensorValue;
  }
  Vpp = (((sampleMax - sampleMin) / 2) * Vcc) / 1024L;
  convertedmA = (707L * Vpp) / 66; // 1/sqrt(2) = 0.7071 = 707.1/1000
  return (convertedmA / 1000.000);
}

void receiveEvent(int howMany) //howMany=num. of bytes received
{
  int  a, b;
  a = Wire.read() - '0'; //Convert char to int
  b = Wire.read() - '0'; //Convert char to int
  Serial.print(F("Received: "));  Serial.print(a);  Serial.print(F("="));  Serial.println(b);

  switch (a)
  {
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
