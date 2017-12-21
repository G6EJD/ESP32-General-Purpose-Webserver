/*
 General Purpose Webserver that can be adjusted to suit your needs
 It can control Relays, LEDs, Servos, Graph Data, Get user inputs, read a BME680, BME280, SHT-31D or DHT-11/22
 
 The examples are provided to help get you going towards your own webserver applciation
 
 The MIT License (MIT) Copyright (c) 2017 by David Bird. 
 ### The formulation and calculation method of an IAQ - Internal Air Quality index ###
 ### The provision of a general purpose webserver ###
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files 
 (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, 
 publish, distribute, but not to use it commercially for profit making or to sub-license and/or to sell copies of the Software or to 
 permit persons to whom the Software is furnished to do so, subject to the following conditions:  
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. 
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 See more at http://dsbird.org.uk 
*/

// To enable or disable any of these webpages remove or include comment '//' markers as requried, e.g to remove BME280 support '//#define bme280' to include it '#define bme280'
//#define bme680sensor
//#define bme280sensor
//#define shtsensor
//#define dhtsensor 
//#define relay
//#define led
//#define graph
#define inputexample

//################# LIBRARIES ################
#include "sensors.h"
#include <WiFi.h>
#include <ESP32WebServer.h>  //https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <WiFiClient.h>
#include <time.h>
//################ VARIABLES ################
const char* ssid      = "yourSSID";     // WiFi SSID
const char* password  = "yourPASSWORD"; // WiFi Password

String siteheading    = "ESP32 Webserver";               // Site's Main Title
String subheading     = "Sensor Readings";               // Sub-heading for all pages
String sitetitle      = "ESP32 Webserver";               // Appears on the tabe of a Web Browser
String yourfootnote   = "ESP32 Webserver Demonstration"; // A foot note e.g. "My Web Site"
String siteversion    = "v1.0";  // Version of your Website

#define sitewidth  1024  // Adjust site page width in pixels as required

String webpage = ""; // General purpose variable to hold HTML code

typedef struct {
  int    logcnt;      // Sequential log count
  String logtime;     // Time reading taken
  float  temp;        // Temperature values
  float  humi;        // Humidity values
  float  pressure;    // Pressure values
} record_type;

#define table_size 23 // ### 40 is the maximum before Google fails to interpret the request and data is lost - I've done the testing so you don't have too!
record_type sensor_1_data[table_size+1]; // Define the data array for sensor-1
//record_type sensor_2_data[table_size+1]; // Define the data array for sensor-2
//record_type sensor_n_data[table_size+1]; // Define the data array for sensor-n and so on

String field1, field4, field5 , CheckBoxChoice = "";
float  field2 ;
int    field3 ;

ESP32WebServer server(80); // Start server on port 80 (default for a web-browser, change to your requirements, e.g. 8080 perhaps, if your Router uses port 80
                           // To access server from outside of a WiFi (LAN) network e.g. on port 8080 add a rule on your Router that forwards a connection request
                           // to http://your_network_WAN_address:8080 to http://your_network_LAN_address:8080 and then you can view your ESP server from anywhere.
                           // Example http://yourhome.ip:8080 and your ESP Server is at 192.168.0.40, then the request will be directed to http://192.168.0.40:8080
void setup()
{
  Serial.begin(115200); // initialize serial communications
  Serial.println("Connect your I2C sensors to the default SDA, SCL pins for your board shown here:");
  Serial.println("I2C SDA pin = "+String(SDA));
  Serial.println("I2C SCL pin = "+String(SCL));// Connect I2C sensors to the default SDA and SCL pins! Check Serial port for details
  StartWiFi(ssid,password);
  StartTime();
  //----------------------------------------------------------------------
  Serial.println("Use this URL to connect: http://"+WiFi.localIP().toString()+"/");// Print the IP address
  // NOTE: There is an issue with ESP8266Webserver and ESP32Webserver that no client inputs from a POST request work unless the Userinput page is the first page served
  //       I do-not have a solution to this problem
  server.on("/",          homepage);   // If the user types at their browser http://192.168.0.100/ control is passed here and then to user_input, you get values for your program...
  server.on("/homepage",  homepage);   // If the user types at their browser http://192.168.0.100/homepage or via menu control is passed here and then to the homepage, etc
  server.on("/page1",     page1);      // Comment out if not required
  //server.on("/page2",     page2);      // Comment out if not required
  // And so-on if required, remember there has to be a function to call e.g. '/command4' calls 'page4' or whatever you call either of them e.g. /example4 needs a 'example4' command
  #ifdef servo
    server.on("/servo",     showServo);  // Comment out if not required
  #endif
  #ifdef graph
    server.on("/graph1",    showGraph1); // Comment out if not required
  #endif
  #ifdef relay
    server.on("/relay",     showRELAY);  // Comment out if not required
  #endif
  #ifdef led  
    server.on("/led",       showLED);    // Comment out if not required 
  #endif
  #ifdef dhtsensor
    server.on("/dht",       showDHT);    // Comment out if not required 
  #endif
  #ifdef shtsensor
    server.on("/sht31",     showSHT31);  // Comment out if not required
  #endif
  #ifdef bme280sensor
    server.on("/bme280",    showBME280); // Comment out if not required
  #endif
  #ifdef bme680sensor
    server.on("/bme680",    showBME680); // Comment out if not required
  #endif
  #ifdef inputexample
    server.on("/user",      showInput);  // Comment out if not required
    server.on("/userinput", userinput);  // Must retain if user input is required
  #endif
  server.on("/about",     about);      // If the user types at their browser http://192.168.0.100/about     or via menu control is passed here and then to the about page
  server.onNotFound(handleNotFound);   // If the user types something that is not supported, say so
  server.begin(); Serial.println(F("Webserver started...")); // Start the webserver
}

/* Adding a new command:
 *  Add to setup server response triggers:  
 *    server.on("/graph2",    showGraph2);
 *  Add the response function to the trigger, e.g. 
 *    void showGraph2(){ // gets called when the server gets /graph2
 *     your commands/functions/html, use one of my examples as a template
 *    }
 *  Add to the menu  
      void append_HTML_header() {
      webpage += " <a href='/graph2'>Graph2</a>";
 * That's it.  
 * 
 * Removing menu items, do the reverse of above, delete the trigger, delete the response function, delete the menu item from append_HTML_header
*/

void loop() {
  server.handleClient();
  // Fill array with data for the graph to display
  for (int i = 0; i <= table_size; i++){
     sensor_1_data[i].logcnt  = i;
     sensor_1_data[i].logtime = GetTime();            // Google Charts requires time is this format "11/12/17 22:01:00" if you then want to display time on the x-axis
     sensor_1_data[i].temp    = random(18, 25);       // Serial.print(sensor_1_data[i].temp);
     sensor_1_data[i].humi    = random(40,60)/100.0F; // Serial.println(sensor_1_data[i].humi);
  }
}

void handleNotFound(){
  String message = "The request entered could not be found, please try again with a different option\n";
  server.send(404, "text/plain", message);
}

void homepage(){
  append_HTML_header(); 
  webpage += "<P class='style2'>This is the server home page</p><br>";
  webpage += "<p class='style2'>";
  webpage += "Ego tam in schola et populo puto me dolor ob eam. Sed hoc non est verum. In facto, in schola tres annos conpescere luctor.";
  webpage += "Autem, duobus annis ante me factus est: et paucis placuit ad schola gravis circa commutationes. Primum, ego sum placuit facti";
  webpage += "sunt interested in est quod quicquid docuit, quidquid aliis putavit. Item placuit ut numquam laborare cotidie aliquid";
  webpage += "assignationis. Numquam tibi placuit, cadere numquam post. Denique, ut ad ludum et placuit et amicis res est fun. Post haec in";
  webpage += "effectum ducenda mutationes, et factus est particeps activae in Curabitur aliquet ultricies disputant, quaeritote. Tunc mihi in";
  webpage += "test turpis oriri coepit. Ego adhuc recordabor quod aliquis fecit fun de me primum quod 'non fuit dolor.";
  webpage += "'Quam excitando! Videtur mihi esse in rem operatus difficile dolor sit simpliciter et ens interested. Etenim video nova discendi";
  webpage += "laborem etiam interesse. Infeliciter, doctrina non auxilium vos adepto a video ludum non bonam, in collegium vel adepto a officium.";
  webpage += "</p><br>";
  webpage += "<p>This page was displayed on : "+GetTime()+" Hr</p>";
  String Uptime = (String(millis()/1000/60/60))+":";
  Uptime += (((millis()/1000/60%60)<10)?"0"+String(millis()/1000/60%60):String(millis()/1000/60%60))+":";
  Uptime += ((millis()/1000%60)<10)?"0"+String(millis()/1000%60):String(millis()/1000%60);
  webpage += "<p>Uptime: " + Uptime + "</p>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

void page1(){
  append_HTML_header(); 
  webpage += "<H3>This is the server Page-1</H3>";
  webpage += "<P class='style2'>This is the server home page</p>";
  webpage += "<p class='style2'>";
  webpage += "Ego tam in schola et populo puto me dolor ob eam. Sed hoc non est verum. In facto, in schola tres annos conpescere luctor.";
  webpage += "Autem, duobus annis ante me factus est: et paucis placuit ad schola gravis circa commutationes. Primum, ego sum placuit facti";
  webpage += "sunt interested in est quod quicquid docuit, quidquid aliis putavit. Item placuit ut numquam laborare cotidie aliquid";
  webpage += "assignationis. Numquam tibi placuit, cadere numquam post. Denique, ut ad ludum et placuit et amicis res est fun. Post haec in";
  webpage += "effectum ducenda mutationes, et factus est particeps activae in Curabitur aliquet ultricies disputant, quaeritote. Tunc mihi in";
  webpage += "test turpis oriri coepit. Ego adhuc recordabor quod aliquis fecit fun de me primum quod 'non fuit dolor.";
  webpage += "'Quam excitando! Videtur mihi esse in rem operatus difficile dolor sit simpliciter et ens interested. Etenim video nova discendi";
  webpage += "laborem etiam interesse. Infeliciter, doctrina non auxilium vos adepto a video ludum non bonam, in collegium vel adepto a officium.";
  webpage += "</p>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

void page2(){
  append_HTML_header(); 
  webpage += "<H3>This is the server Page-2</H3>";
  webpage += "<p class='style2'>";
  webpage += "Ego tam in schola et populo puto me dolor ob eam. Sed hoc non est verum. In facto, in schola tres annos conpescere luctor.";
  webpage += "Autem, duobus annis ante me factus est: et paucis placuit ad schola gravis circa commutationes. Primum, ego sum placuit facti";
  webpage += "sunt interested in est quod quicquid docuit, quidquid aliis putavit. Item placuit ut numquam laborare cotidie aliquid";
  webpage += "assignationis. Numquam tibi placuit, cadere numquam post. Denique, ut ad ludum et placuit et amicis res est fun. Post haec in";
  webpage += "effectum ducenda mutationes, et factus est particeps activae in Curabitur aliquet ultricies disputant, quaeritote. Tunc mihi in";
  webpage += "test turpis oriri coepit. Ego adhuc recordabor quod aliquis fecit fun de me primum quod 'non fuit dolor.";
  webpage += "'Quam excitando! Videtur mihi esse in rem operatus difficile dolor sit simpliciter et ens interested. Etenim video nova discendi";
  webpage += "laborem etiam interesse. Infeliciter, doctrina non auxilium vos adepto a video ludum non bonam, in collegium vel adepto a officium.";
  webpage += "</p>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

#ifdef servo
void showServo() {
  int ServoPosition = 0;
  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_HTML_header();
  String IPaddress = WiFi.localIP().toString();
  webpage += "<h3>Enter position of Servo required then select Enter</h3>";
  webpage += "<form action=\"http://"+IPaddress+"/servo\" method=\"POST\">";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 0'>-45&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 5'>-40&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 10'>-35&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 15'>-30&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 20'>-25&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 25'>-20&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 30'>-15&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 35'>-10&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 40'>-5&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 45'>0&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 50'>5&deg;";
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 55'>10&deg;"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 60'>15&deg;"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 65'>20&deg;"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 70'>25&deg;"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 75'>30&deg;"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 80'>35&deg;"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 85'>40&deg;"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value=' 90'>45&deg;"; 
  webpage += "<br><br><input type='submit' value='Enter'><br><br>";
  webpage += "</form></body>";
  append_HTML_footer();
  server.send(200, "text/html", webpage); // Send a response to the client to enter their inputs, if needed, Enter=defaults
  if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i <= server.args(); i++ ) {
      String Argument_Name   = server.argName(i);
      String client_response = server.arg(i);
      if (Argument_Name == "CheckBoxChoice") { 
        CheckBoxChoice = client_response; // Checking for more than one check-box being selected too, 'a' if more than one 
        if (isValidNumber(CheckBoxChoice)) ServoPosition = CheckBoxChoice.toInt(); else ServoPosition = 0;
      }
    }
  }
  Serial.println("Servo Position choice was : "+String(ServoPosition));
  pinMode(Servo_pin,OUTPUT);
  // There is no Servo control for the ESP32 so use the LED PWM function! We use 10-bit resolution
  ledcSetup(0, 50, 10); // (Channel, Frequency, Resolution = 8, 10 or 16 bit
  ledcAttachPin(Servo_pin, 0);
  ledcWriteTone(0, 50); // Set the initial Frequency of 50HZ or a 20mS servo frame-rate
  int dutyCycle = map(ServoPosition/90.0 * 1024, 0, 1023, 1.1*1024/50, 4*1024/50);// A proportion of the mS demanded e.g. 1ms to 2mS when using 10-bit or 1024 steps
  //int dutyCycle = map(ServoPosition/90.0 * 256, 0, 255, 1*256/50, 4*256/50);  // A proportion of the mS demanded e.g. 1ms to 2mS when using 8-bit or 256 steps
  ledcWrite(0, dutyCycle);
  Serial.println("Set a duty cycle of: "+String(dutyCycle));
  webpage = "";
  homepage();
}
#endif

void about(){
  append_HTML_header(); 
  webpage += "<H3>This is the server About page</H3>";
  webpage += "<p class='style2'>";
  webpage += "Ego tam in schola et populo puto me dolor ob eam. Sed hoc non est verum. In facto, in schola tres annos conpescere luctor.";
  webpage += "Autem, duobus annis ante me factus est: et paucis placuit ad schola gravis circa commutationes. Primum, ego sum placuit facti";
  webpage += "sunt interested in est quod quicquid docuit, quidquid aliis putavit. Item placuit ut numquam laborare cotidie aliquid";
  webpage += "assignationis. Numquam tibi placuit, cadere numquam post. Denique, ut ad ludum et placuit et amicis res est fun. Post haec in";
  webpage += "effectum ducenda mutationes, et factus est particeps activae in Curabitur aliquet ultricies disputant, quaeritote. Tunc mihi in";
  webpage += "test turpis oriri coepit. Ego adhuc recordabor quod aliquis fecit fun de me primum quod 'non fuit dolor.";
  webpage += "'Quam excitando! Videtur mihi esse in rem operatus difficile dolor sit simpliciter et ens interested. Etenim video nova discendi";
  webpage += "laborem etiam interesse. Infeliciter, doctrina non auxilium vos adepto a video ludum non bonam, in collegium vel adepto a officium.";
  webpage += "</p>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

#ifdef relay
void showRELAY(){
  append_HTML_header();
  webpage += "<H3>RELAY Status</H3>";
  pinMode(Relay_pin, OUTPUT);  // Initialize the RELAY pin as an output
  digitalWrite(Relay_pin, LOW);
  subheading = "RELAY is  ";
  RELAY = !RELAY; // Toggle RELAY state
  // NOTE: On some boards you may need to reverse the commands digitalWrite(Relay_pin, LOW); to digitalWrite(Relay_pin, HIGH); and vice-versa
  if (RELAY == true) {
    digitalWrite(Relay_pin, HIGH);  // Turn the RELAY on, set pin HIGH, but need to be reversed 
    subheading += "ON";
  }
  else {
    digitalWrite(Relay_pin, LOW);   // Turn the RELAY off, set pin LOW, RELAY maybe active low so may need to be reversed!
    subheading += "OFF";
  }
  webpage += "<p class='style1'><p>" + subheading +"</p>";
  append_HTML_footer(); 
  server.send(200, "text/html", webpage); 
  Serial.println("Switched RELAY state"); 
}
#endif

#ifdef led
void showLED() {
  append_HTML_header();
  webpage += "<H3>LED Status</H3>";
  pinMode(LED_BUILTIN, OUTPUT);  // Initialize the LED_BUILTIN pin as an output 
  subheading = "LED is  ";
  LED = !LED; // Toggle LED state
  // NOTE: On some boards you may need to reverse the commands digitalWrite(Relay_pin, LOW); to digitalWrite(Relay_pin, HIGH); and vice-versa
  if (LED == true) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED on, set pin LOW on most boards
    subheading += "ON";
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off, set pin HIGH, LED is active low!
    subheading += "OFF";
  }
  webpage += "<p class='style1'><p>" + subheading +"</p>";
  append_HTML_footer(); 
  server.send(200, "text/html", webpage); 
  Serial.println("Switched LED state"); 
} 
#endif

#ifdef dhtsensor
void showDHT(){
  String SensorStatus = "OK";
  dht.begin();
  float Temperature;
  float Humidity;
  Temperature = dht.readTemperature();
  Humidity    = dht.readHumidity();
  if (isnan(Temperature) || isnan(Humidity)) { 
    Serial.println("Failed to read from DHT sensor!"); 
    SensorStatus = "#### DHT Sensor not connected, check sensor and wiring ####";
  } 
  append_HTML_header();
  webpage += "<H3>Sensor Readings</H3>";
  subheading = "DHT Temperature & Humidity on pin "+String(DHTPIN);
  webpage += "<H3>"+subheading+"<br></H3>";
  webpage += "<p class='style1'>";
  if (SensorStatus == "OK") {
    Temperature = dht.readTemperature();
    Humidity    = dht.readHumidity();
    webpage += "<table><tr><th>Temperature</th><th>Humidity</th><tr>";
    webpage += "<td>"+String(Temperature,2)+"&degC</td>";
    webpage += "<td>"+String(Humidity,2)+" %</td>";
    webpage += "</tr></table>";
  }
  else webpage += SensorStatus;
  webpage += "</p>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}
#endif

#ifdef shtsensor
void showSHT31(){
  String SensorStatus = "OK";
  Wire.begin(SDA,SCL);
  sht31.begin(sht31_address);
  float Temperature = sht31.readTemperature();
  float Humidity    = sht31.readHumidity();
  if (!sht31.begin(sht31_address) || isnan(Temperature)) {
    Serial.println("Could not find a valid SHT30 sensor, check wiring!");
    SensorStatus = "#### SHT31 Sensor not connected, check sensor and wiring ####";
  }
  delay(1000); // Slight delay to let sensor start
  Temperature = sht31.readTemperature();
  Humidity    = sht31.readHumidity();
  append_HTML_header();
  webpage += "<H3>Sensor Readings</H3>";
  subheading = "Sensirion SHT31";
  webpage += "<H3>"+subheading+"<br></H3>";
  webpage += "<p class='style1'>";
  if (SensorStatus == "OK") {
    Temperature = sht31.readTemperature();
    Humidity    = sht31.readHumidity();
    webpage += "<table><tr><th>Temperature</th><th>Humidity</th><tr>";
    webpage += "<td>"+String(Temperature,2)+"&degC</td>";
    webpage += "<td>"+String(Humidity,2)+" %</td>";
    webpage += "</tr></table>";
  }
  else webpage += SensorStatus;
  webpage += "</p>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}
#endif

#ifdef bme280sensor
void showBME280(){
  String SensorStatus = "OK";
  if (!bme280.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    SensorStatus = "#### BME280 Sensor not connected, check sensor and wiring ####";
  }
  delay(1000); // Slight delay to let sensor start
  append_HTML_header();
  webpage += "<H3>Sensor Readings</H3>";
  subheading = "Bosch BME280";
  webpage += "<H3>"+subheading+"<br></H3>";
  webpage += "<p class='style1'>";
  if (SensorStatus == "OK") {
    float Temperature = bme280.readTemperature();
    float Humidity    = bme280.readHumidity();
    float Pressure    = bme280.readPressure()/100.00F;
    float Altitude    = bme280.readAltitude(SEALEVELPRESSURE_HPA);
    webpage += "<p style='background-color:orange;'>*** BME280 Readings ***</P>";
    webpage += "<table style='font-family:arial,sans-serif;font-size:16px;border-collapse:collapse;text-align:center;width:90%;margin-left:auto;margin-right:auto;'>";
    webpage += "<tr>";
    webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>Temperature</th>";
    webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>Humidity</th>";
    webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>Pressure</th>";
    webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>~Altitude</th>";
    webpage += "</tr>";
    webpage += "<tr>";
    webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+String(Temperature,2)+"&deg;C</td>";
    webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+String(Humidity,2)+" %</td>";
    webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+String(Pressure,2)+" hPa</td>";
    webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+String(Altitude,2)+" Metres</td>";
    webpage += "</tr></table>";
  }
  else webpage += SensorStatus;
  webpage += "</p>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}
#endif

#ifdef bme680sensor
void showBME680(){
  String SensorStatus = "OK";
  if (!bme680.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    SensorStatus = "#### BME680 Sensor not connected, check sensor and wiring ####";
  }
  append_HTML_header();
  webpage += "<H3>Sensor Readings</H3>";
  subheading = "Bosch BME680";
  webpage += "<H3>"+subheading+"<br></H3>";
  webpage += "<p class='style1'>";
  if (SensorStatus == "OK") {
    // Set up oversampling and filter initialization
    bme680.setTemperatureOversampling(BME680_OS_8X);
    bme680.setHumidityOversampling(BME680_OS_2X);
    bme680.setPressureOversampling(BME680_OS_4X);
    bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme680.setGasHeater(320, 150); // 320*C for 150 ms
    if (! bme680.performReading()) {
      Serial.println("Failed to perform reading :(");
      SensorStatus = "A problem occurred whilst reading the sensor";
    }
    else
    {
      float Temperature    = bme680.readTemperature();
      float Humidity       = bme680.readHumidity();
      float Pressure       = bme680.readPressure()/100.00F;
      float Altitude       = bme680.readAltitude(SEALEVELPRESSURE_HPA);
      float Gas_resistance = bme680.readGas();
      if (Gas_resistance == 0) { // Sometimes the sensor is not ready for a Gas reading, so try again
        bme680.performReading();
        Gas_resistance = bme680.readGas();
      }
      for (int i = 1; i < 11; i++){ // Take 10 readings to warm up the sensor
        Gas_resistance = bme680.readGas();
        delay(200);
      }
      webpage += "<p style='background-color:orange;'>*** BME680 may require multiple readings for a stable IAQ ***</P>";
      webpage += "<table style='font-family:arial,sans-serif;font-size:16px;border-collapse:collapse;text-align:center;width:90%;margin-left:auto;margin-right:auto;'>";
      webpage += "<tr>";
      webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>Temperature</th>";
      webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>Humidity</th>";
      webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>Pressure</th>";
      webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>Gas</th>";
      webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>IAQ</th>";
      webpage += " <th style='border:0px solid black;text-align:left;padding:2px;'>~Altitude</th>";
      webpage += "</tr>";
      webpage += "<tr>";
      webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+String(Temperature,2)+"&deg;C</td>";
      webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+String(Humidity,2)+" %</td>";
      webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+String(Pressure,2)+" hPa</td>";
      webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+String(Gas_resistance,2)+" Ohms</td>";
      webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+IAQ(Gas_resistance,Humidity)+"</td>";
      webpage += " <td style='border:0px solid black;text-align:left;padding:2px;'>"+String(Altitude,2)+" Metres</td>";
      webpage += "</tr></table>";
    }
  }
  else webpage += SensorStatus;
  webpage += "</p>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

String IAQ(float current_gas, float current_humidity){
// The MIT License (MIT) Copyright (c) 2017 by David Bird.
// The calculation of this IAQ Internal Air Quality index using the BME680
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
// (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, but not to use it commercially for profit making or to sub-license and/or to sell copies of the Software or to
// permit persons to whom the Software is furnished to do so, subject to the following conditions: 
//   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// See more at http://dsbird.org.uk
 
  //Calculate humidity contribution to IAQ index
  float gas_reference = current_gas; 
  float hum_reference = 40; 
  float hum_score = 0;
  float gas_score = 0;
  if (current_humidity >= 38 && current_humidity <= 42)
    hum_score = 0.25*100; // Humidity +/-5% around optimum 
  else
  { //sub-optimal
    if (current_humidity < 38) 
      hum_score = 0.25/hum_reference*current_humidity*100;
    else
    {
      hum_score = ((-0.25/(100-hum_reference)*current_humidity)+0.416666)*100;
    }
  }
  
  //Calculate gas contribution to IAQ index
  int gas_lower_limit = 50;     // Bad air quality limit
  int gas_upper_limit = 50000;  // Good air quality limit
  if (gas_reference > gas_upper_limit) gas_reference = gas_upper_limit;
  if (gas_reference < gas_lower_limit) gas_reference = gas_lower_limit;
  gas_score = (0.75/(gas_upper_limit-gas_lower_limit)*gas_reference -(gas_lower_limit*(0.75/(gas_upper_limit-gas_lower_limit))))*100;
  Serial.println(hum_score); // 0 to 0.25 or 25% max
  Serial.println(gas_score); // 0 to 0.75 or 75% max
  //Combine results for the final IAQ index value (0-100% where 100% is good quality air)
  float air_quality_score = hum_score + gas_score;
  return CalculateIAQ(air_quality_score);
}

String CalculateIAQ(float score){
  String IAQ_text = "Air quality is ";
  score = (100-score)*5;
  if      (score > 300)                  IAQ_text += "Hazardous";
  else if (score > 200 && score <= 300 ) IAQ_text += "Very Unhealthy";
  else if (score > 175 && score <= 200 ) IAQ_text += "Unhealthy";
  else if (score > 150 && score <= 175 ) IAQ_text += "Unhealthy for Sensitive Groups";
  else if (score >  50 && score <= 150 ) IAQ_text += "Moderate";
  else if (score >= 00 && score <=  50 ) IAQ_text += "Good";
  return IAQ_text;
}
#endif

#ifdef inputexample
void userinput() {
  String field1_response, field2_response, field3_response, field4_response, field5_response;
  CheckBoxChoice = "";
  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_HTML_header();
  String IPaddress = WiFi.localIP().toString();
  webpage += "<h3>User Input, enter values then select Enter</h3>";
  webpage += "<form action=\"http://"+IPaddress+"/userinput\" method=\"POST\">";
  webpage += "<table style='font-family:arial,sans-serif;font-size:16px;border-collapse:collapse;text-align:center;width:90%;margin-left:auto;margin-right:auto;'>";
  webpage += "<tr>";
  webpage += "<th style='border:0px solid black;text-align:left;padding:2px;'>Input Field 1</th>";
  webpage += "<th style='border:0px solid black;text-align:left;padding:2px;'>Input Field 2</th>";
  webpage += "<th style='border:0px solid black;text-align:left;padding:2px;'>Input Field 3</th>";
  webpage += "<th style='border:0px solid black;text-align:left;padding:2px;'>Input Field 4</th>";
  webpage += "</tr>";
  webpage += "<tr>";
  webpage += "<td style='border:0px solid black;text-align:left;padding:2px;'><input type='text' name='field1' value='Your defaults'></td>";
  webpage += "<td style='border:0px solid black;text-align:left;padding:2px;'><input type='text' name='field2' value='3.141592654'></td>";
  webpage += "<td style='border:0px solid black;text-align:left;padding:2px;'><input type='text' name='field3' value='1234'></td>";
  webpage += "<td style='border:0px solid black;text-align:left;padding:2px;'><input type='text' name='field4' value='Text entry'></td>";
  webpage += "</tr>";
  webpage += "</table><br><br>";
  webpage += "Input field 5<br><input type='text' name='field5' value='field-5-default'><br><br>";
  // And so-on
  webpage += "<input type='checkbox' name='CheckBoxChoice' value='a'>Option-A"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value='b'>Option-B"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value='c'>Option-C"; 
  webpage += "<input type='checkbox' name='CheckBoxChoice' value='d'>Option-D"; 
  // And so-on  
  webpage += "<br><br><input type='submit' value='Enter'><br><br>";
  webpage += "</form></body>";
  append_HTML_footer();
  server.send(200, "text/html", webpage); // Send a response to the client to enter their inputs, if needed, Enter=defaults
  if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      String Argument_Name   = server.argName(i);
      String client_response = server.arg(i);
      if (Argument_Name == "field1") field1_response = client_response;
      if (Argument_Name == "field2") field2_response = client_response;
      if (Argument_Name == "field3") field3_response = client_response;
      if (Argument_Name == "field4") field4_response = client_response;
      if (Argument_Name == "field5") field5_response = client_response;
      if (Argument_Name == "CheckBoxChoice") { 
         if (client_response.length() > 1) 
            CheckBoxChoice = "a"; else CheckBoxChoice = client_response; // Checking for more than one check-box being selected too, 'a' if more than one 
      } 
    }
  }
  field1 = field1_response;
  field2_response.trim(); // Remove any leading spaces
  field3_response.trim(); // Remove any leading spaces
  if (isValidNumber(field2_response)) field2 = field2_response.toFloat();
  if (isValidNumber(field3_response)) field3 = field3_response.toInt();
  field4 = field4_response;
  field5 = field5_response;
  Serial.println("   Field1 Input was : "+field1);
  Serial.println("   Field2 Input was : "+String(field2,6));
  Serial.println("   Field3 Input was : "+String(field3));
  Serial.println("   Field4 Input was : "+field4);
  Serial.println("   Field5 Input was : "+field5);
  Serial.println("Checkbox choice was : "+CheckBoxChoice);
  webpage = "";
  homepage();
}

boolean isValidNumber(String str) {
  str.trim();
  if(!(str.charAt(0) == '+' || str.charAt(0) == '-' || isDigit(str.charAt(0)))) return false; // Failed if not starting with +- or a number
  for(byte i=1;i<str.length();i++) {if(!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) return false;} // Anything other than a number or . is a failure
  return true;
}

void showInput(){
  append_HTML_header(); 
  webpage += "<H3>This was the User Input</H3>";
  webpage += "<p class='style2'>";
  webpage += "Field-1 user input was: " + field1+"<br>";
  webpage += "Field-2 user input was: " + String(field2,6)+"<br>";
  webpage += "Field-3 user input was: " + String(field3)+"<br>";
  webpage += "Field-4 user input was: " + field4+"<br>";
  webpage += "Field-5 user input was: " + field5+"<br>";
  webpage += "   Checkbox choice was: " + CheckBoxChoice+"<br>";
  webpage += "</p>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}
#endif

void StartWiFi(const char* ssid, const char* password) {
  int connAttempts = 0;
  Serial.print(F("\r\nConnecting to: ")); Serial.println(String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500); Serial.print(".");
    if (connAttempts > 20) {Serial.println("Failed to connect to WiFi");}
    connAttempts++;
  }
  Serial.print(F("WiFi connected at: "));
  Serial.println(WiFi.localIP());
}

void StartTime(){
  configTime(0, 0, "0.uk.pool.ntp.org", "time.nist.gov");
  setenv("TZ", "GMT0BST,M3.5.0/01,M10.5.0/02",1); // Set for your locale
  delay(200);
  GetTime();
}

String GetTime(){
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    StartTime();
  }
  //See http://www.cplusplus.com/reference/ctime/strftime/
  //Serial.println(&timeinfo, "%a %b %d %Y   %H:%M:%S"); // Displays: Saturday, June 24 2017 14:05:49
  char output[50];
  strftime(output, 50, "%d/%m/%y %H:%M:%S", &timeinfo); // Format needed for Google Charts is "11/12/17 22:01:00"; //dd/mm/yy hh:hh:ss
  return output;
}

void append_HTML_header() {
  webpage  = "";
  webpage += "<!DOCTYPE html><html><head>";
  webpage += "<meta http-equiv='refresh' content='600'>"; // 5-min refresh time, test needed to prevent auto updates repeating some commands
  webpage += "<style>";
  webpage += "body {width:"+String(sitewidth)+"px;margin:0 auto;font-family:arial;font-size:14px;text-align:center;color:blue;background-color:#F7F2Fd;}";
  webpage += "h1 {background-color:#ffc66e;margin:16px 30px;}"; // Orange background
  webpage += "h3 {color:#9370DB;font-size:24px;width:auto;}";
  webpage += ".navbar{overflow:hidden;background-color:#558ED5;position:fixed;top:0;width:"+String(sitewidth)+"px;margin-left:30px;}";
  webpage += ".navbar a {float:left;display:block;color:yellow;text-align:center;padding:10px 12px;text-decoration: none;font-size:17px;}";
  webpage += ".main{padding:0px;margin:16px 30px;height:1000px;width:"+String(sitewidth)+"px;}";
  webpage += ".style1{text-align:center;font-size:16px;background-color:#FFE4B5;}";
  webpage += ".style2{text-align:left;font-size:16px;background-color:#F7F2Fd;width:auto;margin:0 auto;}";
  // Note: You cannot include (table, tr, td, or th) styles if you want Google Charts to work!
  webpage += "</style>";
  webpage += "</head><body>";
  webpage += "<div class='navbar'>";
  // For each new page you add or remove, make sure there is a menu item to call it or remove it when not used
  webpage += " <a href='/homepage'>Home</a>";
  webpage += " <a href='/page1'>Page1</a>";
  #ifdef servo
    webpage += " <a href='/servo'>Servo</a>";
  #endif
  #ifdef inputexample
    webpage += " <a href='/userinput'>Get Input</a>";
    webpage += " <a href='/user'>Show Input</a>";
  #endif
  #ifdef graph
    webpage += " <a href='/graph1'>Graph1</a>";
  #endif
  #ifdef relay
    webpage += " <a href='/relay'>RELAY</a>";
  #endif
  #ifdef led
    webpage += " <a href='/led'>LED</a>";
  #endif
  #ifdef dhtsensor
    webpage += " <a href='/dht'>DHT</a>";
  #endif 
  #ifdef shtsensor
    webpage += " <a href='/sht31'>SHT31</a>";
  #endif
  #ifdef bme280sensor
    webpage += " <a href='/bme280'>BME280</a>";
  #endif
  #ifdef bme680sensor
    webpage += " <a href='/bme680'>BME680</a>";
  #endif
  webpage += " <a href='/about'>About</a>";
  webpage += "</div>";
  webpage += "<br><title>"+sitetitle+"</title><br>";
  webpage += "<div class='main'><h1>"+siteheading+" "+ siteversion + "</h1>";
}

void append_short_HTML_header() { // Needed because Google Charts appears to limit the amount of data that can be sent to its server
  webpage  = "";
  webpage += "<!DOCTYPE html><html><head>";
  webpage += "<style>";
  webpage += "body {width:"+String(sitewidth)+"px;margin:0 auto;font-family:arial;font-size:14px;text-align:center;color:blue;background-color:#F7F2Fd;}";
  webpage += "h1 {background-color:#d8d8d8;margin:16px 30px;}";
  webpage += "h3 {color:#9370DB;font-size:24px;width:auto;}";
  webpage += ".navbar{overflow:hidden;background-color:#558ED5;position:fixed;top:0;width:"+String(sitewidth)+"px;margin-left:30px;}";
  webpage += ".navbar a {float:left;display:block;color:yellow;text-align:center;padding:14px 16px;text-decoration: none;font-size:17px;}";
  webpage += ".main{padding:0px;margin:16px 30px;height:1000px;width:"+String(sitewidth)+"px;}";
  webpage += ".style1{text-align:center;font-size:16px;background-color:#FFE4B5;}";
  webpage += ".style2{text-align:left;font-size:16px;background-color:#F7F2Fd;width:auto;margin:0 auto;}";
  // Note: You cannot include table, tr, td, or th styles if you want Google Charts to work!
  webpage += "</style></head>";
  webpage += "<body>";
  webpage += "<div class='navbar'>";
  webpage += " <a href='/homepage'>Home</a>";
  webpage += "</div>";
  webpage += "<title>"+sitetitle+"</title>";
  webpage += "<div class='main'>";
}

void append_HTML_footer(){ 
  webpage += "<footer><p>"+yourfootnote+"<br>";
  webpage += "&copy;"+String(char(byte(0x40>>1)))+String(char(byte(0x88>>1)))+String(char(byte(0x5c>>1)))+String(char(byte(0x98>>1)))+String(char(byte(0x5c>>1)));
  webpage += String(char((0x84>>1)))+String(char(byte(0xd2>>1)))+String(char(0xe4>>1))+String(char(0xc8>>1))+String(char(byte(0x40>>1)));
  webpage += String(char(byte(0x64/2)))+String(char(byte(0x60>>1)))+String(char(byte(0x62>>1)))+String(char(0x6e>>1))+"</p></footer>";
  webpage += "</div></body></html>";
}

#ifdef graph
void showGraph1(){ // Displays data in the array called sensor_1_data
  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_short_HTML_header();
  // Graphdata1("Graph Title",Min_Value,Max_Value,Autoscale) Autoscale = true means data will be displayed and scaled according to the data, if false Min_Value and Max_Value are used
  Graphdata1("Graph of Temperature & Humidity",-10,30,false); // Construct HTML and scripts to graph data, needs a header and footer though
  append_HTML_footer();
  server.send(200, "text/html", webpage);
  webpage = ""; // Free memory before next call
}

/*
void showGraph2(){ / /Displays data in the array called sensor_1_data, or call Graphdata2 by reference with an array so it's a generic graphing routine
  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_short_HTML_header();
  // Graphdata2("Graph Title",Min_Value,Max_Value,Autoscale) Autoscale = true means data will be displayed and scaled according to the data, if false Min_Value and Max_Value are used
  Graphdata2("Graph of Temperature & Humidity",-10,30,false); // Construct HTML and scripts to graph data, needs a header and footer though
  append_HTML_footer();
  server.send(200, "text/html", webpage);
  webpage = ""; // Free memory before next call
}
*/

void Graphdata1(String title, int min_value, int max_value, bool Autoscale){
  String data_lable_1  = " Readings";
  String data_lable_2  = "Temperature";
  String data_lable_3  = "Humidity";
  String Left_y_units  = "Temperature Deg-C";
  String Right_y_units = "Humidity %";
  // Processes a clients request for a graph of the data in array called sensor_1_data[]
  // See google charts api for more details. To load the APIs, include the following script in the header of your web page.
  // <script type="text/javascript" src="https://www.google.com/jsapi"></script>
  // To autoload APIs manually, you need to specify the list of APIs to load in the initial <script> tag, rather than in a separate google.load call for each API. For instance, the object declaration to auto-load version 1.0 of the Search API (English language) and the local search element, would look like: {
  // This would be compressed to: {"modules":[{"name":"search","version":"1.0","language":"en"},{"name":"elements","version":"1.0","packages":["
  // See https://developers.google.com/chart/interactive/docs/basic_load_libs
  // https://developers.google.com/loader/ // https://developers.google.com/chart/interactive/docs/basic_load_libs
  // https://developers.google.com/chart/interactive/docs/basic_preparing_data
  // https://developers.google.com/chart/interactive/docs/reference#google.visualization.arraytodatatable and See appendix-A
  // data format is: [field-name,field-name,field-name] then [data,data,data], e.g. [12, 20.5, 70.3]
  webpage += F("<br><br><script type=\"text/javascript\" src=\"https://www.google.com/jsapi?autoload={'modules':[{'name':'visualization','version':'1','packages':['corechart']}]}\"></script>");
  webpage += F("<script type=\"text/javascript\"> google.setOnLoadCallback(drawChart);");
  webpage += F("function drawChart() {");
   webpage += F("var data = google.visualization.arrayToDataTable(");
   webpage += "[['"+data_lable_1+"','"+data_lable_2+"','"+data_lable_3+"'],";    
   for (int i = 0; i <= table_size; i=i+1) {
     webpage += "['" + String(sensor_1_data[i].logcnt) + "'," + String(float(sensor_1_data[i].temp),1) + "," + String(float(sensor_1_data[i].humi),1) + "],"; 
   }
   webpage += "]);";
//-----------------------------------
   webpage += F("var options = {");
    webpage += "title:'" + title + "',titleTextStyle:{fontName:'Arial', fontSize:20, color: 'Maroon'},";
    webpage += F("legend:{position:'bottom'},colors:['red','blue'],backgroundColor:'#F7F2Fd',chartArea:{width:'85%',height:'50%'},"); 
    webpage += "hAxis:{titleTextStyle:{color:'Purple',bold:true,fontSize:16},slantedText:true,slantedTextAngle:25,showTextEvery:2,title:'"+String(table_size+1)+data_lable_1+"'},";
    webpage += "minorGridlines:{fontSize:8,format:'d/M/YY',units:{hours:{format:['hh:mm a','ha']},minutes:{format:['HH:mm a Z', ':mm']}}},"; //to display  x-axis in time units
    // or use minorGridlines:{units:{hours:{format:['hh:mm a','ha']},minutes:{format:['HH:mm a Z', ':mm']}}  to display x-axis in time units
    webpage += F("vAxes:");
    if (Autoscale) {
      webpage += "{0:{viewWindowMode:'explicit',gridlines:{color:'black'}, title:'"+Left_y_units+"',format:'##.##'},"; 
      webpage += " 1:{gridlines:{color:'transparent'},viewWindow:{min:0,max:1},title:'"+Right_y_units+"',format:'##%'},},"; 
    }
    else {
      webpage += F("{0:{viewWindowMode:'explicit',viewWindow:{min:");
      webpage += String(min_value)+",max:"+String(max_value)+"},gridlines:{color:'black'},title:'"+Left_y_units+"',format:'##.##'},";
      webpage += " 1:{gridlines:{color:'transparent'},viewWindow:{min:0,max:1},title:'"+Right_y_units+"',format:'##%'},},"; 
    }
    webpage += F("series:{0:{targetAxisIndex:0},1:{targetAxisIndex:1},},curveType:'none'};");
    webpage += F("var chart = new google.visualization.LineChart(document.getElementById('line_chart'));chart.draw(data, options);");
  webpage += F("}");
  webpage += F("</script>");
  webpage += F("<div id=\"line_chart\" style=\"width:1020px; height:600px\"></div>");
}
#endif
