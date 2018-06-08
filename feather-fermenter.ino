// Adafruit IO Temperature & Humidity Example
// Tutorial Link: https://learn.adafruit.com/adafruit-io-basics-temperature-and-humidity
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016-2017 Adafruit Industries
// Licensed under the MIT license.
//
// Adafruit OLED FeatherWing Battery Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ DHT Libraries *******************************/
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

/************************ OLED Libraries *******************************/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_FeatherOLED.h>

/************************ DHT Config *******************************/

// pin connected to DH22 data line
#define DH22_DATA_PIN 2

// create DHT22 instance
DHT_Unified dht(DH22_DATA_PIN, DHT22);

// set up the 'temperature' and 'humidity' feeds
AdafruitIO_Feed *temperature = io.feed("temperature");
AdafruitIO_Feed *humidity = io.feed("humidity");
AdafruitIO_Feed *battery = io.feed("battery-voltage");

/************************ OLED Config *******************************/

#if defined(ARDUINO_ARCH_SAMD) || defined(__AVR_ATmega32U4__)

  // m0 & 32u4 feathers
  #define VBATPIN A7

  float getBatteryVoltage() {

    float measuredvbat = analogRead(VBATPIN);

    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage

    return measuredvbat;

  }

#elif defined(ESP8266)

  // esp8266 feather
  #define VBATPIN A0

  float getBatteryVoltage() {

    float measuredvbat = analogRead(VBATPIN);

    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage

    return measuredvbat;

  }

#elif defined(ARDUINO_STM32_FEATHER)

  // wiced feather
  #define VBATPIN PA1

  float getBatteryVoltage() {

    pinMode(VBATPIN, INPUT_ANALOG);

    float measuredvbat = analogRead(VBATPIN);

    measuredvbat *= 2;         // we divided by 2, so multiply back
    measuredvbat *= 0.80566F;  // multiply by mV per LSB
    measuredvbat /= 1000;      // convert to voltage

    return measuredvbat;

  }

#else

  // unknown platform
  float getBatteryVoltage() {
    Serial.println("warning: unknown feather. getting battery voltage failed.");
    return 0.0F;
  }

#endif

#if defined(ESP8266)
  #define BUTTON_A 0
  #define BUTTON_B 16
  #define BUTTON_C 2
  #define LED      0
#elif defined(ESP32)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
  #define LED      13
#elif defined(ARDUINO_STM32F2_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
  #define LED PB5
#elif defined(TEENSYDUINO)
  #define BUTTON_A 4
  #define BUTTON_B 3
  #define BUTTON_C 8
  #define LED 13
#elif defined(ARDUINO_FEATHER52)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
  #define LED 17
#else // 32u4, M0, and 328p
  #define BUTTON_A 9
  #define BUTTON_B 6
  #define BUTTON_C 5
  #define LED      13
#endif
 
#if (SSD1306_LCDHEIGHT != 32)
 #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// oled objects
Adafruit_FeatherOLED oled = Adafruit_FeatherOLED();
Adafruit_SSD1306 display = Adafruit_SSD1306();

// integer variable to hold current counter value
int count = 0;

void setup() {

  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  Serial.println("OLED FeatherWing test");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);
 
  // Clear the buffer.
  display.clearDisplay();
  display.display();
  
  Serial.println("IO test");
 
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
 
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("SSID: ");
  display.print(WIFI_SSID);
  display.print("\n");
  display.print("Connecting to \n Adafruit IO...");
  display.setCursor(0,0);
  display.display(); // actually display all of the above
  
  // intialize oled
  //oled.init();
  //oled.setBatteryVisible(true);
  
  // initialize dht22
  dht.begin();

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    io.connect();
    Serial.print(".");
    delay(5000);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  
}

void loop() {
  
  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  sensors_event_t event;
  dht.temperature().getEvent(&event);

  float celsius = event.temperature;
  float fahrenheit = (celsius * 1.8) + 32;

  Serial.print("celsius: ");
  Serial.print(celsius);
  Serial.println("C");

  Serial.print("fahrenheit: ");
  Serial.print(fahrenheit);
  Serial.println("F");

  // save fahrenheit (or celsius) to Adafruit IO
  temperature->save(fahrenheit);

  dht.humidity().getEvent(&event);

  Serial.print("humidity: ");
  Serial.print(event.relative_humidity);
  Serial.println("%");

  // save humidity to Adafruit IO
  humidity->save(event.relative_humidity);

  // clear the current count
  //oled.clearDisplay();

  // get the current voltage of the battery from
  // one of the platform specific functions below
  float batt = getBatteryVoltage();
  
  // save current voltage to Adafruit IO
  battery->save(batt);

  // update the battery icon
  //oled.setBattery(battery);
  //oled.renderBattery();

  display.clearDisplay();
  display.setCursor(0,0);
  display.print("SSID: ");
  display.print(WIFI_SSID);
  display.print("\n");
  display.print("temp: ");
  display.print(celsius);
  display.print("C ");
  display.print(fahrenheit);
  display.print("F\n");
  display.print("humi: ");
  display.print(event.relative_humidity);
  display.print(" % \n");
  display.print("batt: ");
  display.print(batt);
  display.setCursor(0,0);
  display.display(); // actually display all of the above
  
  // print the count value to the OLED
  //oled.print("count: ");
  //oled.println(count);

  // update the display with the new count
  //oled.display();

  // increment the counter by 1
  count++;

  if (! digitalRead(BUTTON_A)) display.print("A");
  if (! digitalRead(BUTTON_B)) display.print("B");
  if (! digitalRead(BUTTON_C)) display.print("C");
  delay(10);
  yield();
  display.display();
  
  // wait 5 seconds (5000 milliseconds == 5 seconds)
  delay(5000);
}
