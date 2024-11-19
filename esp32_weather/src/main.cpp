#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"

// TFT Display Pins
#define TFT_DC 4
#define TFT_CS 15
#define TFT_RST 2
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_CLK 18

class WeatherStation {
private:
    // Wi-Fi credentials
    const char* ssid = "Kai's iPhone";
    const char* password = "997724fusion";

    // Location and weather data
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

    // Temperature configuration
    #define TEMP_CELSIUS 0
    #if TEMP_CELSIUS
        String temperature_unit = "";
        const char degree_symbol[3] = "\u00B0C";
    #else
        String temperature_unit = "&temperature_unit=fahrenheit";
        const char degree_symbol[3] = "\u00B0F";
    #endif

    // TFT display instance
    Adafruit_ILI9341 tft;

    // Private methods
    void bootUp();
    void connectToInternet();
    void getWeatherData();
    void weatherGUI();

public:
    // Constructor
    WeatherStation() : tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO) {}

    // Public methods
    void setup();
    void loop();
};

// Private Methods Implementation
void WeatherStation::bootUp() {
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 100);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(3);
    tft.println("Connecting to");
    tft.println("the internet!");
    delay(2000);
}

void WeatherStation::connectToInternet() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi network with IP Address: " + WiFi.localIP().toString());
}

void WeatherStation::getWeatherData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = "http://api.open-meteo.com/v1/forecast?latitude=" + latitude +
                     "&longitude=" + longitude +
                     "&current=temperature_2m,relative_humidity_2m,is_day,precipitation,rain,weather_code" +
                     temperature_unit + "&timezone=" + timezone + "&forecast_days=1";
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                const char* datetime = doc["current"]["time"];
                temperature = String(doc["current"]["temperature_2m"].as<float>());
                humidity = String(doc["current"]["relative_humidity_2m"].as<float>());
                is_day = doc["current"]["is_day"].as<int>();
                weather_code = doc["current"]["weather_code"].as<int>();

                String datetime_str = String(datetime);
                int splitIndex = datetime_str.indexOf('T');
                current_date = datetime_str.substring(0, splitIndex);
                last_weather_update = datetime_str.substring(splitIndex + 1, splitIndex + 9);
            } else {
                Serial.println("JSON Parsing Error: " + String(error.c_str()));
            }
        } else {
            Serial.println("HTTP GET Failed: " + http.errorToString(httpCode));
        }
        http.end();
    } else {
        Serial.println("Not connected to Wi-Fi");
    }
}

void WeatherStation::weatherGUI() {
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("Weather Information:");
    tft.println("Location: " + location);
    tft.println("Temperature: " + temperature + degree_symbol);
    tft.println("Humidity: " + humidity + "%");
}

// Public Methods Implementation
void WeatherStation::setup() {
    Serial.begin(9600);
    Serial.println("Booting");

    tft.begin();
    tft.fillScreen(ILI9341_BLACK);
    tft.setRotation(0);

    bootUp();
    connectToInternet();

    if (WiFi.status() == WL_CONNECTED) {
        getWeatherData();
    } else {
        bootUp();
    }
}

void WeatherStation::loop() {
    if (WiFi.status() != WL_CONNECTED) {
        bootUp();
    } else {
        weatherGUI();
    }
    delay(10000);
}

// Global instance of WeatherStation
WeatherStation station;

void setup() {
    station.setup();
}

void loop() {
    station.loop();
}
