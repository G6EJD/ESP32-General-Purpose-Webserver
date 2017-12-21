/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
#include <Wire.h>
#include <SPI.h>

#ifdef bme680sensor
  // BME680 Support using I2C
  #include <Adafruit_Sensor.h>
  #include "Adafruit_BME680.h"
  #define SEALEVELPRESSURE_HPA (1013.25)
  Adafruit_BME680 bme680; // I2C
#endif

#ifdef bme280sensor
  // BME280 Support using I2C
  #include <Adafruit_Sensor.h>
  #include "Adafruit_BME280.h"
  #define SEALEVELPRESSURE_HPA (1013.25)
  Adafruit_BME280 bme280; // I2C
#endif

#ifdef shtsensor
  // SHT31 Support using I2C
  #include "Adafruit_SHT31.h"
  #define sht31_address 0x44 // I2C address of SHT31 sensor
  Adafruit_SHT31 sht31 = Adafruit_SHT31();
#endif

#ifdef dhtsensor
  #include "DHT.h"
  #define DHTPIN 2 
  //#define DHTTYPE DHT11   // DHT 11 
  //#define DHTTYPE DHT21   // DHT 21 (AM2301) 
  #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321 
  DHT dht(DHTPIN, DHTTYPE);
#endif

#ifdef led
  #define LED_BUILTIN 2    // Needed for ESP32 LOLIN Board
  boolean LED = false;
#endif

#ifdef relay
  // RELAY Controls
  #define Relay_pin 4
  boolean RELAY = false;
#endif

#ifdef servo
  // Servo Control
  #define Servo_pin 13
  #define SERVO_MIN_PULSEWIDTH 1000 // Minimum pulse width in microsecond 
  #define SERVO_MAX_PULSEWIDTH 2000 // Maximum pulse width in microsecond 
  #define SERVO_MAX_DEGREE     180  // Maximum angle in degree upto which servo can rotate 
#endif
