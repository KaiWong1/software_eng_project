/*
Kai Wong

References: https://randomnerdtutorials.com/esp32-tft-lvgl-weather-station/

*/


#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "FS.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 4
#define TFT_CS 15
#define TFT_RST 2 //disconnect this pin when uploading.
#define TFT_MISO 19         
#define TFT_MOSI 23           
#define TFT_CLK 18 

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

const char* ssid = "Kai's iPhone";
const char* password = "997724fusion";

String latitude = "41.14961";
String longitude = "-8.61099";
String location = "Porto";
String timezone = "Europe/Lisbon";

String current_date;
String last_weather_update;
String temperature;
String humidity;
int is_day;
int weather_code = 0;
String weather_description;

#define TEMP_CELCIUS 0;

#if TEMP_CELSIUS
  String temperature_unit = "";
  const char degree_symbol[] = "\u00B0C";
#else
  String temperature_unit = "&temperature_unit=fahrenheit";
  const char degree_symbol[] = "\u00B0F";
#endif


void setup() {

  //Serial.begin(9600);
  Serial.begin(115200);
  Serial.println("Booting"); 
 
  //connect to wifi
  WiFi.begin(ssid, password);
  Serial.print("connecting");
  
  tft.begin();
  //if esp32 connects to internet, go to loop
  //if else, stay on bootUp();
  if (WiFi.status() == WL_CONNECTED)
  {
    getWeatherData();
  }
  else
  {
  bootUp();
  }
}
  /* Shows the bootup screen when the ESP32 is connecting to the internet. 
  After connecting to the internet the bootup screen will go away and load
  the weather related GUI elements. 
  */
unsigned long bootUp() 
{
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0,100);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(3);
  tft.println("Connecting to");
  tft.println("the internet!");
  return micros() - start;

  if (WiFi.status() == WL_CONNECTED)
  {
    getWeatherData();
  }
  else
  {
    bootUp();
  }
    
}

unsigned long weatherGUI()
{

}

unsigned long connectToInternet()
{
  {
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) 
    {
    delay(500);
    Serial.print(".");
    }
  }

Serial.print("\nConnected to Wi-Fi network with IP Address: ");
Serial.println(WiFi.localIP());

}

void getWeatherData()
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http;
    // Construct the API endpoint
    String url = String("http://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longitude + "&current=temperature_2m,relative_humidity_2m,is_day,precipitation,rain,weather_code" + temperature_unit + "&timezone=" + timezone + "&forecast_days=1");
    http.begin(url);
    int httpCode = http.GET(); // Make the GET request

    if (httpCode > 0) 
    {
      // Check for the response
      if (httpCode == HTTP_CODE_OK) 
      {
        String payload = http.getString();
        //Serial.println("Request information:");
        //Serial.println(payload);
        // Parse the JSON to extract the time
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) 
        {
          const char* datetime = doc["current"]["time"];
          temperature = String(doc["current"]["temperature_2m"].as<float>());
          humidity = String(doc["current"]["relative_humidity_2m"].as<float>());
          is_day = String(doc["current"]["is_day"].as<int>()).toInt();
          weather_code = String(doc["current"]["weather_code"].as<int>()).toInt();
          /*Serial.println(temperature);
          Serial.println(humidity);
          Serial.println(is_day);
          Serial.println(weather_code);
          Serial.println(String(timezone));*/
          // Split the datetime into date and time
          String datetime_str = String(datetime);
          int splitIndex = datetime_str.indexOf('T');
          current_date = datetime_str.substring(0, splitIndex);
          last_weather_update = datetime_str.substring(splitIndex + 1, splitIndex + 9); // Extract time portion
        } 
        else 
        {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
        }
      }
    } 
    else 
    {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end(); // Close connection
  } 
  else 
  {
    Serial.println("Not connected to Wi-Fi");
  }

}


void loop() 
{
  tft.setRotation(0);
  /*if ESP32 connects to internet proceed with GUI elements.
  If else, then the ESP32 will keep running BootUp() in void setup(). Maybe
  add a RTOS? */

  //delay(1000);

}

