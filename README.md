# ESP32/ESP8266-General-Purpose-Webserver
An ESP32/ESP8266 based webserver with lots of examples in it

A general purpose webserver with examples of how to:

1. Display time and date
2. Display server uptime
3. Control relays
4. Control LED's
5. Get user inputs
6. Display user inputs
7. Display data using google charts
8. Read and display data from a Bosch BME280 temperature, pressure and humidity sensor
9. Read and display data from a Bosch BME680 temperature, pressure, humidity and Air Quality sensor
10. Read and display data from an SHT-31D
11. Read and display data from a DHT11 or DHT22 temperature and humdity sensor
12. Add general web pages, modify the supplied latin for your own text

Copy the server source code and sensors files to your sketch folder

Change the SSID and Password for your Router/network

Add pages as required by adding or removing '//' comments, for example to include BME280 support change change the first lines to look like this:

//#define bme680sensor

#define bme280sensor

//#define shtsensor

//#define dhtsensor

//#define relay

//#define led

//#define graph

#define inputexample

 
