#include <myWifiHelper.h>
#include <myPushButton.h>
#include <TaskScheduler.h>
#include <Adafruit_NeoPixel.h>

char versionText[] = "Liams House Ceiling Lights v2.0";


#define     WIFI_HOSTNAME               "/liamshouse/ceiling-lights"

#define     TOPIC_ONLINE                "/liamshouse/ceiling_lights/online"
#define     TOPIC_COMMAND               "/liamshouse/ceiling_lights/command"
#define     TOPIC_EVENT                 "/liamshouse/ceiling_lights/event"
#define     TOPIC_TIMESTAMP             "/dev/timestamp"

#define     TOUCH_SIG_PIN   0

//--------------------------------------------------------------------------------

#define     PIXEL_PIN   2
#define     PIXEL_COUNT 140
#define		BRIGHT_MAX	256

// 0 - 83mA
// 4 - 148mA
// 10 - 240mA 15.7
// 20 - 356mA 13.8ma/led?  (356-80 / 20)
// 140 - (15mA * 140 + 83mA) 2.1A


Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRBW + NEO_KHZ800);

int gamma1[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

//---------------------------------------------------------------

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

void mqttcallback_timestamp(byte* payload, unsigned int length) {
	wifiHelper.mqttPublish(TOPIC_ONLINE, "1");
}

void mqttcallback_command(byte *payload, unsigned int length) {

    JsonObject& root = wifiHelper.mqttGetJson(payload);
    const char* command = root["command"];
    const char* value = root["value"];

    if (strcmp(command, "LED") == 0) {
     	if (strcmp(value, "FADE_WHITE_IN") == 0) {
     		fadeInToWhite(10);
     	}
     	else if (strcmp(value, "FADE_WHITE_OUT") == 0) {
     		fadeOutFromWhite(10);
     	}
     	else if (strcmp(value, "WHITE_ON") == 0) {
     		fullWhite();
     	}
     	else if (strcmp(value, "OFF") == 0) {
     		colorWipe(strip.Color(0,0,0,0), 0);
     	}
     	else if (strcmp(value, "RAINBOW") == 0) {
     		rainbowCycle(10);
     	}
    }
}

//--------------------------------------------------------------------------------

Scheduler runner;

#define RUN_ONCE    2

bool notificationActive = false;

// void tClearNotification_callback();
// Task tClearNotification(2000, RUN_ONCE, &tClearNotification_callback, &runner, false);
// void tClearNotification_callback() {
//     if (tClearNotification.isLastIteration()) {
// 	    display.clear();
//         display.displayOff();
//         notificationActive = false;
//     }
// }

//--------------------------------------------------------------------------------


void button_callback(int eventCode, int eventParam);
#define NO_PULLUP       false
#define WITH_PULLUP		true
#define LOW_IS_LOW    	LOW    
#define LOW_IS_HIGH   	HIGH    
myPushButton touch(TOUCH_SIG_PIN, WITH_PULLUP, 2000, LOW_IS_HIGH, button_callback);

void button_callback(int eventCode, int eventParam) {

    switch (eventParam) {
        case touch.EV_BUTTON_PRESSED:
            wifiHelper.mqttPublish(TOPIC_EVENT, "EV_BUTTON_PRESSED");
            Serial.println("EV_BUTTON_PRESSED");
            break;
        case touch.EV_HELD_FOR_LONG_ENOUGH:
            wifiHelper.mqttPublish(TOPIC_EVENT, "EV_HELD_FOR_LONG_ENOUGH");
            Serial.println("EV_HELD_FOR_LONG_ENOUGH");
            break;
        case touch.EV_RELEASED:
        case touch.ST_WAITING_FOR_RELEASE_FROM_HELD_TIME:
            wifiHelper.mqttPublish(TOPIC_EVENT, "EV_RELEASED");
            Serial.println("EV_RELEASED or ST_WAITING_FOR_RELEASE_FROM_HELD_TIME");
            break;
        default:    
            //Serial.println("some event");
            break;
    }
}

//--------------------------------------------------------------------------------
 
void setup() {

    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

    Serial.begin(9600);
    delay(100);
    Serial.println("Booting");
    Serial.println(versionText);

    wifiHelper.setupWifi();

    wifiHelper.setupOTA(WIFI_HOSTNAME);

    delay(300);

    wifiHelper.setupMqtt();
    wifiHelper.mqttAddSubscription(TOPIC_TIMESTAMP, mqttcallback_timestamp);
    wifiHelper.mqttAddSubscription(TOPIC_COMMAND, mqttcallback_command);

    delay(100);   

    //pulseWhite(50);
}

void loop() {

    wifiHelper.loopMqtt();

    runner.execute();

    touch.serviceEvents();

    ArduinoOTA.handle();
}

//--------------------------------------------------------------------------------

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
	for(uint16_t i=0; i<strip.numPixels(); i++) {
		strip.setPixelColor(i, c);
		strip.show();
		delay(wait);
	}
}

void pulseWhite(uint8_t wait) {
	for(int j = 0; j < BRIGHT_MAX ; j++) {
		for(uint16_t i=0; i<strip.numPixels(); i++) {
			strip.setPixelColor(i, strip.Color(0,0,0, gamma1[j] ) );
		}
		delay(wait);
		strip.show();
	}

	for(int j = BRIGHT_MAX-1; j >= 0 ; j--){
		for(uint16_t i=0; i<strip.numPixels(); i++) {
			strip.setPixelColor(i, strip.Color(0,0,0, gamma1[j] ) );
		}
		delay(wait);
		strip.show();
	}
	}

void fullWhite() {

	for(uint16_t i=0; i<strip.numPixels(); i++) {
		strip.setPixelColor(i, strip.Color(0,0,0, BRIGHT_MAX-1 ) );
	}
	strip.show();
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
	uint16_t i, j;

	for(j=0; j<BRIGHT_MAX * 5; j++) { // 5 cycles of all colors on wheel
		for(i=0; i< strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel(((i * BRIGHT_MAX / strip.numPixels()) + j) & BRIGHT_MAX-1));
		}
		strip.show();
		delay(wait);
	}
}

// Input a value 0 to BRIGHT_MAX-1 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = BRIGHT_MAX-1 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(BRIGHT_MAX-1 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, BRIGHT_MAX-1 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, BRIGHT_MAX-1 - WheelPos * 3, 0,0);
}

void fadeInToWhite(uint8_t wait) {
	for(int j = 0; j < BRIGHT_MAX ; j++){
		for(uint16_t i=0; i<strip.numPixels(); i++) {
			strip.setPixelColor(i, strip.Color(0,0,0, gamma1[j] ) );
		}
		delay(wait);
		strip.show();
	}
}

void fadeOutFromWhite(uint8_t wait) {
	for(int j = BRIGHT_MAX-1; j >= 0 ; j--){
		for(uint16_t i=0; i<strip.numPixels(); i++) {
			strip.setPixelColor(i, strip.Color(0,0,0, gamma1[j] ) );
		}
		delay(wait);
		strip.show();
	}
}

uint8_t red(uint32_t c) {
  return (c >> 8);
}
uint8_t green(uint32_t c) {
  return (c >> 16);
}
uint8_t blue(uint32_t c) {
  return (c);
}

