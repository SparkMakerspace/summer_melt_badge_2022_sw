#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define LED1 D0
#define LED2 D4
#define LED3 D6
#define LED4 D7
#define LED5 D8

uint8_t ledArray[] = {LED5, LED1, LED2, LED4, LED3};

#define SLOW_BLINK 800 // ms
#define FAST_BLINK 20  // ms
#define LOW_TEMP 70.0
#define HIGH_TEMP 90.0

ESP8266Timer ITimer; // timer1

#define TEMP D3
OneWire oneWire(TEMP);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Blink rate calculation variables
float slope; // slope of blink rate per temperature
float offset;

uint8_t currLED; // holds which LED is now on

void blinkLEDs(); // this is a prototype of the actual function near the
                  // bottom of the file

void setup()
{
  Serial.begin(9600);
  // Put a line break to separate useful output from the garbage
  Serial.println();
  // Initialize all pins
  for (uint8_t i = 0; i < (sizeof(ledArray) / sizeof(ledArray[0])); i++)
  {
    pinMode(ledArray[i], OUTPUT);
    digitalWrite(ledArray[i], LOW);
  }
  // Initialize temperature sensor library
  sensors.begin();

  // Setup timer for light blinking; interval in microsecords
  if (ITimer.attachInterruptInterval(SLOW_BLINK * 1000, blinkLEDs))
  {
    Serial.println(F("Starting  ITimer OK"));
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));

  // Calculate blink rate slope
  slope = (FAST_BLINK - SLOW_BLINK) / (HIGH_TEMP - LOW_TEMP);
  Serial.print(F("Slope: "));
  Serial.println(slope);

  // Calculate blink rate offset
  offset = SLOW_BLINK - (slope * LOW_TEMP);
  Serial.print(F("offset: "));
  Serial.println(offset);

  currLED = 0;
  Serial.println(F("setup complete"));
}

void loop()
{
  // read temperature
  sensors.requestTemperatures();
  float temperature = sensors.getTempFByIndex(0);
  Serial.print(F("Temperature: "));
  Serial.println(temperature);
  if (temperature > HIGH_TEMP)
    temperature = HIGH_TEMP; 
  // adjust link blink delay
  unsigned long newInterval = long((slope * temperature + offset) * 1000);
  Serial.print(F("New Interval: "));
  Serial.println(newInterval);
  if (newInterval < FAST_BLINK * 1000)
    newInterval = FAST_BLINK * 1000;
  if (newInterval > SLOW_BLINK * 1000)
    newInterval = SLOW_BLINK * 1000;
  ITimer.setInterval(newInterval, blinkLEDs);
  delay(newInterval / 500); // wait between each temp update
}

void blinkLEDs()
{
  // increment which LED should be on
  currLED = (currLED == 4) ? 0 : currLED + 1;
  // set all LEDs off and then turn on currLED
  for (uint8_t i = 0; i < (sizeof(ledArray) / sizeof(ledArray[0])); i++)
  {
    digitalWrite(ledArray[i], LOW);
  }
  digitalWrite(ledArray[currLED], HIGH);
}