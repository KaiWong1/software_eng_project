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

// For the Adafruit shield, these are the default pins.
#define TFT_DC 4
#define TFT_CS 15
#define TFT_RST 2 // Disconnect this pin when uploading.
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

#define TEMP_CELCIUS 0

#if TEMP_CELCIUS
  String temperature_unit = "";
  const char degree_symbol[] = "\u00B0C";
#else
  String temperature_unit = "&temperature_unit=fahrenheit";
  const char degree_symbol[] = "\u00B0F";
#endif

void weatherGUI() 
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  tft.println("Weather Information:");
  tft.println("Location: " + location);
  tft.println("Temperature: " + temperature + degree_symbol);
  tft.println("Humidity: " + humidity + "%");
}

void setup() 
{
  Serial.begin(115200);
  Serial.println("Booting");

  // Initialize TFT
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(0);

  // Connect to Wi-Fi
  connectToInternet();

  if (WiFi.status() == WL_CONNECTED) 
  {
    getWeatherData();
  } 
  else 
  {
    bootUp();
  }
}

void bootUp() 
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 100);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(3);
  tft.println("Connecting to");
  tft.println("the internet!");

  connectToInternet();
}

void connectToInternet() 
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\nConnected to Wi-Fi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void getWeatherData() 
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http;
    String url = "http://api.open-meteo.com/v1/forecast?latitude=" + latitude +
                 "&longitude=" + longitude +
                 "&current_weather=true&timezone=" + timezone + temperature_unit;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) 
    {
      if (httpCode == HTTP_CODE_OK) 
      {
        String payload = http.getString();
        StaticJsonDocument<1024> doc;

        DeserializationError error = deserializeJson(doc, payload);
        if (!error) 
        {
          temperature = String(doc["current_weather"]["temperature"]);
          humidity = String(doc["current_weather"]["humidity"]);
          is_day = doc["current_weather"]["is_day"];
          weather_code = doc["current_weather"]["weather_code"];

          Serial.println("Weather Data Retrieved:");
          Serial.println("Temperature: " + temperature + degree_symbol);
          Serial.println("Humidity: " + humidity);
        } 
        else 
        {
          Serial.print("JSON Parsing failed: ");
          Serial.println(error.c_str());
        }
      }
    } 
    else 
    {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } 
  else 
  {
    Serial.println("Not connected to Wi-Fi");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) 
  {
    bootUp();
  } 
  else 
  {
    // Placeholder for weather GUI
    weatherGUI();
  }

  delay(10000); // Update every 10 seconds
}


