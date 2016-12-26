#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "wificonfig.h"

#define NEO_KHZ400 0x0100

#define PIN 0
#define NUM_PIXELS      10*10

// https://github.com/kit-ho/NeoPixel-WS2812b-Strip-Breathing-Code-with-Arduino
int MinBrightness = 10;       //value 0-255
int MaxBrightness = 127;      //value 0-255

int numLoops1 = 10;
int numLoops2 = 5;
//int numLoops3 = 5;
//int numLoops4 = 3;          //add new integer and value for more color's loop if needed.

int fadeInWait = 30;          //lighting up speed, steps.
int fadeOutWait = 50;         //dimming speed, steps.




// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
        
    setupOTA("LiamsHouseCeilingLightsController");
    
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

}

/*
rgbBreathe(strip.Color(insert r,g,b color code),numLoops(refer to integer above), (duration for lights to hold before dimming. insert 0 to skip hold)
rainbowBreathe(numLoops(refer to integer above),(duration for lights to hold before dimming. insert 0 to skip hold)
*/

void loop() {

    
    rgbBreathe(strip.Color(255, 255, 255), numLoops1, 0);
    //rainbowBreathe(numLoops2, 0);


    /*
    // Some example procedures showing how to display to the pixels:
    colorWipe(strip.Color(255, 0, 0), 50); // Red
    colorWipe(strip.Color(0, 255, 0), 50); // Green
    colorWipe(strip.Color(0, 0, 255), 50); // Blue
    //colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
    // Send a theater pixel chase in...
    theaterChase(strip.Color(127, 127, 127), 50); // White
    theaterChase(strip.Color(127, 0, 0), 50); // Red
    theaterChase(strip.Color(0, 0, 127), 50); // Blue
    
    rainbow(20);
    rainbowCycle(20);
    theaterChaseRainbow(50);
    */
  
    ArduinoOTA.handle();

    delay(200);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}

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

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
    uint16_t i, j;
    
    for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
        for(i=0; i< strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
    for (int j=0; j<10; j++) {  //do 10 cycles of chasing
        for (int q=0; q < 3; q++) {
            for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, c);    //turn every third pixel on
            }
            strip.show();
            
            delay(wait);
            
            for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, 0);        //turn every third pixel off
            }
        }
    }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
        for (int q=0; q < 3; q++) {
            for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
            }
            strip.show();
            
            delay(wait);
            
            for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, 0);        //turn every third pixel off
            }
        }
    }
}

void rgbBreathe(uint32_t color, uint8_t loops, uint8_t wait) {
    for (int j = 0; j < loops; j++) {
        for (uint8_t b = MinBrightness; b < MaxBrightness; b++) {
            strip.setBrightness(b * 255 / 255);
            for (uint16_t i = 0; i < strip.numPixels(); i++) {
                strip.setPixelColor(i, color);
            }
            strip.show();
            delay(fadeInWait);
        }
        strip.setBrightness(MaxBrightness * 255 / 255);
        for (uint16_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, color);
            strip.show();
            delay(wait);
        }
        for (uint8_t b = MaxBrightness; b > MinBrightness; b--) {
            strip.setBrightness(b * 255 / 255);
            for (uint16_t i = 0; i < strip.numPixels(); i++) {
                strip.setPixelColor(i, color);
            }
            strip.show();
            delay(fadeOutWait);
        }
    }
}

void rainbowBreathe(uint8_t x, uint8_t y) {
    for (int j = 0; j < x; j++) {
        for (uint8_t b = MinBrightness; b < MaxBrightness; b++) {
                strip.setBrightness(b * 255 / 255);
                for (uint8_t i = 0; i < strip.numPixels(); i++) {
                strip.setPixelColor(i, Wheel(i * 256 / strip.numPixels()));
            }
            strip.show();
            delay(fadeInWait);
        }
        strip.setBrightness(MaxBrightness * 255 / 255);
        for (uint8_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel(i * 256 / strip.numPixels()));
            strip.show();
            delay(y);
        }
        for (uint8_t b = MaxBrightness; b > MinBrightness; b--) {
            strip.setBrightness(b * 255 / 255);
            for (uint8_t i = 0; i < strip.numPixels(); i++) {
                strip.setPixelColor(i, Wheel(i * 256 / strip.numPixels()));
            }
            strip.show();
            delay(fadeOutWait);
        }
    }
}


//NeoPixel Wheel for Rainbow---------------------------------------

uint32_t Wheel(byte WheelPos) {
    WheelPos = 140 - WheelPos;       //the value here means - for 255 the strip will starts with red, 127-red will be in the middle, 0 - strip ends with red.
    if (WheelPos < 85) {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

/* --------------------------------------------------------- */

void setupOTA(char* host) {
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
    
    ArduinoOTA.setHostname(host);
    ArduinoOTA.onStart([]() {
        Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
}