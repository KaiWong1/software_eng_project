/*
Kai Wong

References: https://randomnerdtutorials.com/esp32-tft-lvgl-weather-station/
https://open-meteo.com/


Requirements:

1. Product shall be able to accept weather data
   - Shall recieve weather updates from online sources

2. Device shall be able to show current weather

3. Device shall be able to predict weather 

4. Product shall alert users 
   - Winds 50 MPH shall activate a green LED
   - Winds 100+ MPH shall activate a yellow LED and produce a 3000 HZ tone
     for 2 seconds at 5-second intervals
   - Winds 180+ MPH shall activate a red LED and produce a 4000 HZ tone 
     for 2 seconds, then a 6000 HZ tone for 2 seconds

5. Product shall be powered by a lithium ion battery



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
#define BUTTON_PIN 5 // GPIO pin for menu toggle button


#define GREEN_LED_PIN 12 // GPIO pin for green LED
#define YELLOW_LED_PIN 13 // GPIO pin for yellow LED
#define RED_LED_PIN 14 // GPIO pin for red LED
#define BUZZER_PIN 27  // GPIO pin for the buzzer


enum MenuState {
    CURRENT_WEATHER,
    SEVEN_DAY_FORECAST
};

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

        pinMode(BUTTON_PIN, INPUT_PULLUP); // Configure the button pin
    }

    void loop() {
        handleButtonInput(); // Check button state for menu switching

        unsigned long currentMillis = millis();

        // Schedule weather data fetching
        if (currentMillis - lastFetchTime >= fetchInterval) {
            fetchWeatherData();
            lastFetchTime = currentMillis;
        }

        // Schedule wind alert handling
        if (currentMillis - lastAlertCheckTime >= alertCheckInterval) {
            handleWindAlerts();
            lastAlertCheckTime = currentMillis;
        }

        if (WiFi.status() != WL_CONNECTED) {
            bootUp();
        } else if (newDataFetched) {
            displayCurrentMenu();
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

    unsigned long lastAlertCheckTime = 0; // Track last alert handling time
    const unsigned long alertCheckInterval = 100; // Interval in milliseconds

    MenuState menuState = CURRENT_WEATHER; // Track current menu state

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
            String url = String("http://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longitude + "&current=temperature_2m,relative_humidity_2m,wind_speed_10m,is_day,precipitation,rain,weather_code" + temperatureUnit + "&timezone=" + timezone + "&forecast_days=7");

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
        DynamicJsonDocument doc(2048); // Adjusted size for 7-day forecast data
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
            // Parse current weather data
            const char* datetime = doc["current"]["time"];
            temperature = String(doc["current"]["temperature_2m"]);
            humidity = String(doc["current"]["relative_humidity_2m"]);
            windSpeed = String(doc["current"]["wind_speed_10m"]);
            isDay = doc["current"]["is_day"];
            weatherCode = doc["current"]["weather_code"];
    
            updateWeatherDescription(weatherCode);

            // Parse 7-day forecast (only weather codes for simplicity)
            for (int i = 0; i < 7; i++) {
                forecastWeatherCodes[i] = doc["daily"]["weather_code"][i];
            }

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
            
            default: weatherDescription = "UNKNOWN WEATHER CODE"; 
            break;
        }
    }


    void displayCurrentMenu() {
        switch (menuState) {
            case CURRENT_WEATHER:
                displayWeather();
                break;
            case SEVEN_DAY_FORECAST:
                display7DayForecast();
                break;
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

        tft.setTextColor(getWeatherSeverityColor(weatherCode));
        tft.setTextSize(3);
        tft.println("Weather: ");
        tft.println(weatherDescription);
        tft.println(" ");

        tft.setTextColor(ILI9341_WHITE);
        tft.println("Temp: " + temperature + "F");
        tft.println("Humidity: " + humidity + "%");
        tft.println("WindSpeed: " + windSpeed + "MPH");
    }

    void display7DayForecast() {
        tft.setRotation(3);
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(10, 10);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);

        tft.println("7-Day Forecast:");
        for (int i = 0; i < 7; i++) {
            tft.println("Day " + String(i + 1) + ": Code " + String(forecastWeatherCodes[i]));
        }
    }

    void handleButtonInput() {
        static bool buttonPressed = false;

        if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
            buttonPressed = true;
            toggleMenu();
        } else if (digitalRead(BUTTON_PIN) == HIGH && buttonPressed) {
            buttonPressed = false;
        }
    }

    void toggleMenu() {
        if (menuState == CURRENT_WEATHER) {
            menuState = SEVEN_DAY_FORECAST;
        } else {
            menuState = CURRENT_WEATHER;
        }

        displayCurrentMenu(); // Immediately refresh the display
    }

    uint16_t getWeatherSeverityColor(int weatherCode) {
        if (weatherCode == 0 || weatherCode == 1) {
            return ILI9341_GREEN; // Clear or Mainly Clear
        } else if (weatherCode >= 2 && weatherCode <= 3) {
            return ILI9341_YELLOW; // Partly Cloudy or Overcast
        } else {
            return ILI9341_WHITE; // Default
        }
    }

    int forecastWeatherCodes[7] = {0}; // Array for 7-day forecast weather codes

    void handleWindAlerts() {
        float windSpeedValue = windSpeed.toFloat(); // Convert wind speed to float

        // Reset all LEDs and buzzer
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
        noTone(BUZZER_PIN);

        if (windSpeedValue >= 180.0) {
          //if (windSpeedValue < 180.0) {
            // Red LED and dual-tone alert
            digitalWrite(RED_LED_PIN, HIGH);
            tone(BUZZER_PIN, 4000, 2000); // Play 4000 Hz tone for 2 seconds
            //delay(2000);
            Serial.println("Buzzing at 4000HZ"); //for debugging purposes
            tone(BUZZER_PIN, 6000, 2000); // Play 6000 Hz tone for 2 seconds
            Serial.println("Buzzign at 6000HZ");
        } else if (windSpeedValue >= 100.0) {
            // Yellow LED and single-tone alert
            digitalWrite(YELLOW_LED_PIN, HIGH);
            tone(BUZZER_PIN, 3000, 2000); // Play 3000 Hz tone for 2 seconds
            delay(5000); // 5-second interval
        } else if (windSpeedValue >= 50.0) {
            // Green LED alert
            digitalWrite(GREEN_LED_PIN, HIGH);
        }
    }

    
};

// Global instance
WeatherStation weatherStation("iphone15", "997724fusion");

void setup() {
    weatherStation.setup();
        // Configure pins for LEDs and buzzer
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

}

void loop() {
    weatherStation.loop();
}
