/*
Kai Wong

References: https://randomnerdtutorials.com/esp32-tft-lvgl-weather-station/
https://open-meteo.com/



*/




#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"

// Constants for TFT display
#define TFT_DC 4
#define TFT_CS 15
#define TFT_RST 2
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_CLK 18

class WeatherStation {
public:
    WeatherStation(const char* ssid, const char* password) 
        : ssid(ssid), password(password), tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO) {}

    void setup() {
        Serial.begin(9600);
        Serial.println("Booting");

        tft.begin();
        tft.fillScreen(ILI9341_BLACK);
        tft.setRotation(3);

        bootUp();
        connectToWiFi();
        if (WiFi.status() == WL_CONNECTED) {
            fetchWeatherData();
        }
    }

    void loop() {
        unsigned long currentMillis = millis();
        if (currentMillis - lastFetchTime >= fetchInterval) {
            fetchWeatherData();
            lastFetchTime = currentMillis;
        }

        if (WiFi.status() != WL_CONNECTED) {
            bootUp();
        } else if (newDataFetched) {
            displayWeather();
            newDataFetched = false;  // Reset the flag after displaying the data
        }
    }

private:
    const char* ssid;
    const char* password;
    Adafruit_ILI9341 tft;

    String latitude = "42.47516";
    String longitude = "-83.25074";
    String location = "Southfield";
    String timezone = "EST";

    String currentDate;
    String lastWeatherUpdate;
    String temperature;
    String humidity;
    String windSpeed;
    int isDay = 0;
    int weatherCode = 0;
    String weatherDescription;

    String temperatureUnit = "&temperature_unit=fahrenheit";
    const char* degreeSymbol = "\u00B0F";

    unsigned long lastFetchTime = 0;
    const unsigned long fetchInterval = 900000;
    bool newDataFetched = false;  // Flag to track if new data is fetched
    
    uint16_t getWeatherSeverityColor(int weatherCode) {
        if (weatherCode == 0 || weatherCode == 1) {
            return ILI9341_GREEN; // Clear or Mainly Clear
        } else if (weatherCode >= 2 && weatherCode <= 3) {
            return ILI9341_YELLOW; // Partly Cloudy or Overcast
        } else if ((weatherCode >= 51 && weatherCode <= 57) || (weatherCode >= 61 && weatherCode <= 67)) {
            return ILI9341_ORANGE; // Drizzle or Rain
        } else if ((weatherCode >= 71 && weatherCode <= 86) || (weatherCode >= 95 && weatherCode <= 99)) {
            return ILI9341_RED; // Snow, Thunderstorms, or Extreme Conditions
        } else {
            return ILI9341_WHITE; // Unknown or Neutral
        }
    }

    void bootUp() {
        tft.setRotation(3);
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(0, 100);
        tft.setTextColor(ILI9341_GREEN);
        tft.setTextSize(3);
        tft.println("Connecting to");
        tft.println("the internet!");
        delay(2000);
    }

    void connectToWiFi() {
        WiFi.begin(ssid, password);
        Serial.print("Connecting");
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.print("\nConnected to Wi-Fi network with IP Address: ");
        Serial.println(WiFi.localIP());
    }

    void fetchWeatherData() {
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            String url = String("http://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longitude + "&current=temperature_2m,relative_humidity_2m,wind_speed_10m,is_day,precipitation,rain,weather_code" + temperatureUnit + "&timezone=" + timezone + "&forecast_days=1");

            http.begin(url);
            int httpCode = http.GET();

            if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                parseWeatherData(payload);
                newDataFetched = true;  // Set the flag to true indicating new data is fetched
            } else {
                Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
        } else {
            Serial.println("Not connected to Wi-Fi");
        }
    }

    void parseWeatherData(const String& payload) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
            const char* datetime = doc["current"]["time"];
            temperature = String(doc["current"]["temperature_2m"]);
            humidity = String(doc["current"]["relative_humidity_2m"]);
            windSpeed = String(doc["current"]["wind_speed_10m"]);
            isDay = doc["current"]["is_day"];
            weatherCode = doc["current"]["weather_code"];
            updateWeatherDescription(weatherCode);

            String datetimeStr = String(datetime);
            int splitIndex = datetimeStr.indexOf('T');
            currentDate = datetimeStr.substring(0, splitIndex);
            lastWeatherUpdate = datetimeStr.substring(splitIndex + 1, splitIndex + 9);
        } else {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
        }
    }

    void updateWeatherDescription(int code) {
        switch (code) {
            case 0: weatherDescription = "CLEAR SKY"; break;
            case 1: weatherDescription = "MAINLY CLEAR"; break;
            case 2: weatherDescription = "PARTLY CLOUDY"; break;
            case 3: weatherDescription = "OVERCAST"; break;
            // Add other cases here...
            case 45:
      
      weatherDescription = "FOG";
      break;
    case 48:
      
      weatherDescription = "DEPOSITING RIME FOG";
      break;
    case 51:
     
      weatherDescription = "DRIZZLE LIGHT INTENSITY";
      break;
    case 53:
     
      weatherDescription = "DRIZZLE MODERATE INTENSITY";
      break;
    case 55:
     
      weatherDescription = "DRIZZLE DENSE INTENSITY";
      break;
    case 56:
     
      weatherDescription = "FREEZING DRIZZLE LIGHT";
      break;
    case 57:
     
      weatherDescription = "FREEZING DRIZZLE DENSE";
      break;
    case 61:
    
      weatherDescription = "RAIN SLIGHT INTENSITY";
      break;
    case 63:
     
      weatherDescription = "RAIN MODERATE INTENSITY";
      break;
    case 65:
      
      weatherDescription = "RAIN HEAVY INTENSITY";
      break;
    case 66:
    
      weatherDescription = "FREEZING RAIN LIGHT INTENSITY";
      break;
    case 67:
     
      weatherDescription = "FREEZING RAIN HEAVY INTENSITY";
      break;
    case 71:
      
      weatherDescription = "SNOW FALL SLIGHT INTENSITY";
      break;
    case 73:
     
      weatherDescription = "SNOW FALL MODERATE INTENSITY";
      break;
    case 75:
    
      weatherDescription = "SNOW FALL HEAVY INTENSITY";
      break;
    case 77:
     
      weatherDescription = "SNOW GRAINS";
      break;
    case 80:
     
      weatherDescription = "RAIN SHOWERS SLIGHT";
      break;
    case 81:
     
      weatherDescription = "RAIN SHOWERS MODERATE";
      break;
    case 82:
   
      weatherDescription = "RAIN SHOWERS VIOLENT";
      break;
    case 85:
     
      weatherDescription = "SNOW SHOWERS SLIGHT";
      break;
    case 86:
    
      weatherDescription = "SNOW SHOWERS HEAVY";
      break;
    case 95:
  
      weatherDescription = "THUNDERSTORM";
      break;
    case 96:
 
      weatherDescription = "THUNDERSTORM SLIGHT HAIL";
      break;
    case 99:
     
      weatherDescription = "THUNDERSTORM HEAVY HAIL";
      break;
            default: weatherDescription = "UNKNOWN WEATHER CODE"; break;
        }
    }

    void displayWeather() {
        tft.setRotation(3);
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(10, 10);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);

        tft.println("Weather Information:");
        tft.println("Location: " + location);
        tft.println("Date: " + currentDate);
        tft.println("Last Update: " + lastWeatherUpdate);

        // Set the weather description text color based on severity
        uint16_t severityColor = getWeatherSeverityColor(weatherCode);
        tft.setTextColor(severityColor);
        tft.setTextSize(3);
        tft.println("Weather: ");
        tft.println(weatherDescription);

        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(3);
        tft.println(" ");
        tft.println("Temp: " + temperature + "F");
        tft.println("Humidity: " + humidity + "%");
        tft.println("WindSpeed: " + windSpeed + "MPH");
    }
};

// Global instance
WeatherStation weatherStation("iphone15", "997724fusion");

void setup() {
    weatherStation.setup();
}

void loop() {
    weatherStation.loop();
}
