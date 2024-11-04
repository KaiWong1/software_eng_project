#include "earth.h"
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson.git
#include <NTPClient.h>           //https://github.com/taranais/NTPClient

TFT_eSPI tft = TFT_eSPI();      


// JPEG decoder library
#include <JPEGDecoder.h>

// Return the minimum of two values a and b
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

// Include the sketch header file that contains the image stored as an array of bytes
// More than one image array could be stored in each header file.
#include "jpeg1.h"
// Count how many times the image is drawn for test purposes
uint32_t icount = 0;

#define TFT_GREY 0x5AEB
#define lightblue 0x01E9
#define darkred 0xA041
#define blue 0x5D9B
#include "Orbitron_Medium_20.h"
#include <WiFi.h>

#include <WiFiUdp.h>
#include <HTTPClient.h>

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;


const char* ssid     = "TP-Link_3200";  // your SSID
const char* password = "95001121379884265554";  //Your Password

String town="Chennai";              
String Country="IN";                
const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q="+town+","+Country+"&units=metric&APPID=";
const String key = "add82e4e24d449f3a522f06621a3aaeb"; //paste your API key here
String payload=""; //whole json 
String tmp="" ; //temperatur
String hum="" ; //humidity
  

StaticJsonDocument<1000> doc;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

int backlight[5] = {10,30,60,120,220};
byte b=1;

void setup(void) {
  Serial.begin(115200);
  tft.begin();
  tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);///
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BLACK, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, backlight[b]);


  tft.print("Connecting to ");
  tft.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    tft.print(".");
  }
  
  tft.println("");
  tft.println("WiFi connected.");
  tft.println("IP address: ");
  tft.println(WiFi.localIP());
  delay(3000);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setSwapBytes(true);
            
          tft.setCursor(2, 392, 1);
          tft.println(WiFi.localIP());
          
//          tft.setCursor(80, 282, 2);
//          tft.println("SEC:");
          tft.setTextColor(TFT_WHITE,lightblue);
          tft.setFreeFont(&Orbitron_Medium_20);
           tft.setCursor(4, 300, 2);
          tft.println("TEMP:");

          tft.setCursor(4, 352, 2);
          tft.println("HUM: ");
          tft.setFreeFont(&Orbitron_Medium_20);
          tft.setTextColor(TFT_WHITE,TFT_BLACK);

            tft.setFreeFont(&Orbitron_Medium_20);
            tft.setCursor(6, 82);
           tft.println(town);
           
          tft.setCursor(4, 420, 2);
        //  tft.setFreeFont(&Orbitron_Light_18);
          tft.setTextColor(TFT_CYAN,TFT_BLACK);
          tft.println("IOT BASED INTERNET");
          tft.println("WEATHER DATA OUTSIDE");
          tft.println("OPEN WEATHER");
          
// Initialize a NTPClient to get time
  timeClient.begin(); 
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(19800);   /*EDDITTTTTTTTTTTTTTTTTTTTTTTT                      */
  getData();
  drawArrayJpeg(Earth, sizeof(Earth), 160, 290); 
  delay(500);
}
int i=0;
String tt="";
int count=0;

void animation(){

  // The image is 300 x 300 pixels so we do some sums to position image in the middle of the screen!
  // Doing this by reading the image width and height from the jpeg info is left as an exercise!
  int x = (tft.width()  - 300) / 2 - 1;
  int y = (tft.height() - 300) / 2 - 1;

  drawArrayJpeg(Frames00, sizeof(Frames00), 0, 88); 
  drawArrayJpeg(Frames04, sizeof(Frames04), 0, 88); 
  drawArrayJpeg(Frames08, sizeof(Frames08), 0, 88); 
  drawArrayJpeg(Frames12, sizeof(Frames12), 0, 88); 
  drawArrayJpeg(Frames16, sizeof(Frames16), 0, 88); 
  drawArrayJpeg(Frames20, sizeof(Frames20), 0, 88); 
  drawArrayJpeg(Frames24, sizeof(Frames24), 0, 88); 
  drawArrayJpeg(Frames28, sizeof(Frames28), 0, 88); 
  drawArrayJpeg(Frames32, sizeof(Frames32), 0, 88); 
  drawArrayJpeg(Frames36, sizeof(Frames36), 0, 88); 
  drawArrayJpeg(Frames40, sizeof(Frames40), 0, 88); 
  drawArrayJpeg(Frames44, sizeof(Frames44), 0, 88); 

  }
void loop() {

 
   if(count==0)
   getData();
   count++;
   if(count>200)
   count=0;
   
   tft.fillRect(1,302,62,30,darkred);
    tft.setFreeFont(&Orbitron_Light_32);
    tft.setCursor(2, 329);
         tft.println(tmp.substring(0,3));

         tft.fillRect(1,356,88,30,darkred);
         tft.setFreeFont(&Orbitron_Light_32);
         tft.setCursor(2, 383);
         tft.println(hum+"%");

           tft.setTextColor(TFT_ORANGE,TFT_BLACK);
           tft.setTextFont(2);
           tft.setCursor(6, 44);
           tft.println(dayStamp);
           tft.setTextColor(TFT_WHITE,TFT_BLACK);

          while(!timeClient.update()) {
          timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

 
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
 
 
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
         
         tft.setFreeFont(&Orbitron_Light_32);
         String current=timeStamp.substring(0,5);
         if(current!=tt)
         {
          tft.fillRect(3,8,120,30,TFT_BLACK);
          tft.setCursor(5, 34);
          tft.println(timeStamp.substring(0,5));
          tt=timeStamp.substring(0,5);
         }
  
  delay(80);
   animation();

  
}


void getData()
{
    tft.fillRect(1,170,64,20,TFT_BLACK);
    tft.fillRect(1,210,64,20,TFT_BLACK);
   if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient http;
 
    http.begin(endpoint + key); //Specify the URL
    int httpCode = http.GET();  //Make the request
 
    if (httpCode > 0) { //Check for the returning code
 
         payload = http.getString();
       // Serial.println(httpCode);
        Serial.println(payload);
        
      }
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
  }
 char inp[1000];
 payload.toCharArray(inp,1000);
 deserializeJson(doc,inp);
  
  String tmp2 = doc["main"]["temp"];
  String hum2 = doc["main"]["humidity"];
  String town2 = doc["name"];
  tmp=tmp2;
  hum=hum2;
  
   Serial.println("Temperature"+String(tmp));
   Serial.println("Humidity"+hum);
   Serial.println(town);
   
 }


//####################################################################################################
// Draw a JPEG on the TFT pulled from a program memory array
//####################################################################################################
void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos) {

  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);
  
  
  renderJPEG(x, y);
  
  Serial.println("#########################");
}

//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void renderJPEG(int xpos, int ypos) {

  // retrieve infomration about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.read()) {
    
    // save a pointer to the image block
    pImg = JpegDec.pImage ;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    tft.startWrite();

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
    {

      // Now set a MCU bounding window on the TFT to push pixels into (x, y, x + width - 1, y + height - 1)
      tft.setAddrWindow(mcu_x, mcu_y, win_w, win_h);

      // Write all MCU pixels to the TFT window
      while (mcu_pixels--) {
        // Push each pixel to the TFT MCU area
        tft.pushColor(*pImg++);
      }

    }
    else if ( (mcu_y + win_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding

    tft.endWrite();
  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime;

  // print the results to the serial port
  Serial.print(F(  "Total render time was    : ")); Serial.print(drawTime); Serial.println(F(" ms"));
  Serial.println(F(""));
}

     
