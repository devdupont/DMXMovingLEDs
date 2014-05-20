/*
* MovingRGBLight
* Michael duPont
* Control a moving RGB LED light as a DMX slave device
* 2014-05-06
* Note: Cannot use Serial due to compile error with the Conceptinetics library
*/

#include <Conceptinetics.h>
#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define DMX_SLAVE_CHANNELS 10
#define LEDPIN 4
#define PANPIN 9
#define PANMAX 507
#define PANMIN 100
#define TILTPIN 11
#define NUMPIXELS 64

DMX_Slave dmx_slave( DMX_SLAVE_CHANNELS );
Servo panServo;
Servo tiltServo;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

uint8_t R = 0;
uint8_t G = 0;
uint8_t B = 0;
uint8_t BRT = 0;
uint8_t Pan = 0;
uint8_t lastPan = 0;
bool newPan = false;
uint8_t Tilt = 0;
uint8_t lastTilt = 0;
bool newTilt = false;
uint8_t Special = 0;

void setup() {
  dmx_slave.enable();
  dmx_slave.setStartAddress(1);
  strip.begin();
  strip.setBrightness(100);
  pinMode(13, OUTPUT);
  delay(2000);
}

void loop() {
  //Get control values from DMX channels
  if ( dmx_slave.getChannelValue(1) != NULL ) R = dmx_slave.getChannelValue(1);
  if ( dmx_slave.getChannelValue(2) != NULL ) G = dmx_slave.getChannelValue(2);
  if ( dmx_slave.getChannelValue(3) != NULL ) B = dmx_slave.getChannelValue(3);
  if ( dmx_slave.getChannelValue(4) != NULL ) BRT = dmx_slave.getChannelValue(4);
  if ( dmx_slave.getChannelValue(5) != NULL ) Pan = dmx_slave.getChannelValue(5);
  if ( dmx_slave.getChannelValue(6) != NULL ) Tilt = dmx_slave.getChannelValue(6);
  if ( dmx_slave.getChannelValue(7) != NULL ) Special = dmx_slave.getChannelValue(7);
  
  if (Special > 129) {
    colorWipe(strip.Color(255, 0, 0), 50); // Red
    colorWipe(strip.Color(0, 255, 0), 50); // Green
    colorWipe(strip.Color(0, 0, 255), 50); // Blue
    rainbow(20);
    rainbowCycle(20);
    Special = 0;`
  }
  else {
    Pan = map(Pan , 0 , 255 , 0 , 180);
    Tilt = map(Tilt , 0 , 255 , 0 , 180);
    
    if (BRT > 128) digitalWrite(13, HIGH);
    else digitalWrite(13, LOW);
    //Write servo values
    
    if (lastPan != Pan) {
      if (!newPan) {
        panServo.attach(PANPIN);
        newPan = true;
      }
      if (lastPan + 5 < Pan) lastPan = lastPan+5;
      else if (lastPan - 5 > Pan) lastPan = lastPan-5;
      else lastPan = Pan;
      panServo.write(lastPan);
    }
    else if ((newPan)&&(panServo.read() == lastPan)) {
      panServo.detach();
      newPan = false;
    }
    
    if (lastTilt != Tilt) {
      if (!newTilt) {
        tiltServo.attach(TILTPIN);
        newTilt = true;
      }
      if (lastTilt + 5 < Tilt) lastTilt = lastTilt+5;
      else if (lastTilt - 5 > Tilt) lastTilt = lastTilt-5;
      else lastTilt = Tilt;
      tiltServo.write(lastTilt);
    }
    else if ((newTilt)&&(tiltServo.read() == lastTilt)) {
      tiltServo.detach();
      newTilt = false;
    }
    
    //tiltServo.write(Tilt);
    R = (R*BRT)/255;
    G = (G*BRT)/255;
    B = (B*BRT)/255;
    if (R < 5) R = 0;
    if (G < 5) G = 0;
    if (B < 5) B = 0;
    
    //Write LED vaules
    for (uint8_t i=0 ; i < NUMPIXELS ; i++) {
      strip.setPixelColor(i , strip.Color(R,G,B));
    }
    strip.show();
    
    delay(60);
  }
}

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

uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
