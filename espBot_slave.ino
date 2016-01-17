/*
 * TFT SCREEN PINOUTS --------------------------------------
 * GPIO5             Pin 06 (RESET)
 * GPIO2             Pin 07 (A0)
 * GPIO13 (HSPID)    Pin 08 (SDA)
 * GPIO14 (HSPICLK)  Pin 09 (SCK)
 * GPIO15 (HSPICS)   Pin 10 (CS)
 */

// ============================================================================================== 
// INCLUDES                                                                                ======
// ==============================================================================================

// NEOPIXELS ---------------------------------------
#include <Adafruit_NeoPixel.h>
#include "NeoPatterns.h"

// I2C ---------------------------------------------
#include <Wire.h>

// TFT SCREEN --------------------------------------
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>






// ==============================================================================================
// GLOBALS                                                                                 ======
// ==============================================================================================
// NEOPIXELS ---------------------------------------
const int NEO_PIN = 6;
const int NUMBER_NEO = 5;
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_NEO, NEO_PIN, NEO_GRB + NEO_KHZ800);
NeoPatterns strip(NUMBER_NEO, NEO_PIN, NEO_GRB + NEO_KHZ800, &neoComplete);
String patterns[] = { "RAINBOW", "FADE" };
const int numberOfPatterns = 2;
int currentPattern = 0;


// TFT SCREEN --------------------------------------
#define TFT_PIN_CS   10
#define TFT_PIN_DC   8
#define TFT_PIN_RST  9
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_RST);

unsigned long screenRefreshRate = 10000;
unsigned long lastScreenUpdate = 0;

String data[2];
int currentlyDisplayedService = 0;


// BUZZER ------------------------------------------
#define BUZZ_PIN 3

// ==============================================================================================
// SETUP                                                                                   ======
// ==============================================================================================
void setup() {

  // NEOPIXELS -------------------------------------
  strip.begin();
  strip.RainbowCycle(3); 
  
  // I2C -------------------------------------------
  Wire.begin(8);
  Wire.onReceive(receiveEvent); 

  // TFT SCREEN ------------------------------------
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(tft.getRotation()+3);
  Serial.println("TFT initialized");
  tftDrawText("Beep...\n Boop... \n Am I Cylon?", ST7735_WHITE);
  delay(5000);
  tone(BUZZ_PIN, 262, 250);
}

// ==============================================================================================
// LOOP                                                                                    ======
// ==============================================================================================
void loop() {
  strip.Update();
  screenTimedUpdate();
}

void neoComplete() {
  strip.ColorSet(getNeoColor("R255G0B0E"));
  strip.setLoop(false);
  strip.ColorSet(getNeoColor("R255G0B0E"));
  strip.show();
  //strip.Reverse();
}

// ==============================================================================================
// FUNCTIONS                                                                               ======
// ==============================================================================================

// HELPERS ---------------------------------------
void screenTimedUpdate() {
  if((millis() - lastScreenUpdate) > screenRefreshRate) {
    currentlyDisplayedService++;
    if(currentlyDisplayedService > 1) {
      currentlyDisplayedService = 0;
    }

    tftDrawText(data[currentlyDisplayedService], ST7735_WHITE);
    
    lastScreenUpdate = millis();
  }
}


// I2C -------------------------------------------

void receiveEvent(int howMany) {
  boolean topicReading = true;
  String topic;
  String message;
  
  while (0 < Wire.available()) { 
    char c = Wire.read();
    if(topicReading) {   
      if(c == '|') {
        topicReading = false;
      } else {
        topic += String(c);
      }
    } else {
      message += String(c);
    } 
  }

  if(topic == "CMDSCR") {
    tftDrawText(message, ST7735_WHITE);
    lastScreenUpdate = millis();
  } else if(topic == "CMDNEO") {
    if(message == "REFRESH") {
      tftDrawText(message, ST7735_WHITE);
      lastScreenUpdate = millis();
      
      strip.setLoop(true);
    } else if(message == "FADE") {
      strip.Fade(Wheel(24), Wheel(200), 100, 10);
    } else if (message == "RAINBOW") {
      strip.RainbowCycle(3); 
    } else {
      currentPattern++;
      if(currentPattern >= numberOfPatterns) {
        currentPattern = 0;
      }
      if(patterns[currentPattern] == "FADE") {
        strip.Fade(Wheel(24), Wheel(200), 100, 10);
      } else if (patterns[currentPattern] == "RAINBOW") {
        strip.RainbowCycle(3);
      }
    } 
  } else if(topic == "runmonitor") {
    strip.setPixelColor(3, getNeoColor(message));
    strip.show();
    data[0] = "Run Monitor\n" + message;
  } else if(topic == "gethomedry") {
    strip.setPixelColor(4, getNeoColor(message));
    strip.show();
    data[1] = "GetHomeDry\n" + message;
  }
  strip.show(); 
}


uint32_t getNeoColor(String color) {
  int rIndex = color.indexOf("R");
  int gIndex = color.indexOf("G");
  int bIndex = color.indexOf("B");
  int eIndex = color.indexOf("E");

  String red = color.substring(rIndex+1,gIndex);
  String green = color.substring(gIndex+1, bIndex);
  String blue = color.substring(bIndex+1, eIndex);

  return strip.Color(green.toInt(), red.toInt(), blue.toInt());
}


// NEOPIXELS ---------------------------------------
/*
void rainbow(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
*/
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}





// TFT SCREEN --------------------------------------
void tftDrawText(char *text, uint16_t color) {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(20, 30);
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text); 
}

void tftDrawText(String text, uint16_t color) {
  char tmp[50];
  text.toCharArray(tmp, 50);
  tftDrawText(tmp, color); 
}


