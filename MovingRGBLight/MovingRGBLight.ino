/*
* MovingRGBLight
* Michael duPont
* Control a moving RGB LED light as a DMX slave device
* 2014-11-27
* Note: Cannot use Serial due to compile error with the Conceptinetics library
*/

#include <Conceptinetics.h>
#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define DMX_SLAVE_CHANNELS 10    //Number of DMX channels to be used, >= what the program actually uses
#define DMX_START 1              //Number of first channel
#define NUMPIXELS 64             //Number of NeoPixels
#define LEDPIN 4                 //NeoPixel data pin
#define PANPIN 9                 //Pan servo data pin
#define TILTPIN 11               //Tilt servo data pin

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
//uint8_t Special = 0;

void setup() {
  dmx_slave.enable();
  dmx_slave.setStartAddress(DMX_START);
  strip.begin();
  strip.setBrightness(100);
  pinMode(13, OUTPUT);
  delay(2000);
}

void loop() {
  //Get control values from DMX channels
  if ( dmx_slave.getChannelValue(DMX_START) != NULL ) R = dmx_slave.getChannelValue(DMX_START);             //Red LED value
  if ( dmx_slave.getChannelValue(DMX_START+1) != NULL ) G = dmx_slave.getChannelValue(DMX_START+1);         //Green LED value
  if ( dmx_slave.getChannelValue(DMX_START+2) != NULL ) B = dmx_slave.getChannelValue(DMX_START+2);         //Blue LED value
  if ( dmx_slave.getChannelValue(DMX_START+3) != NULL ) BRT = dmx_slave.getChannelValue(DMX_START+3);       //Panel brightness
  if ( dmx_slave.getChannelValue(DMX_START+4) != NULL ) Pan = dmx_slave.getChannelValue(DMX_START+4);       //Pan servo value
  if ( dmx_slave.getChannelValue(DMX_START+5) != NULL ) Tilt = dmx_slave.getChannelValue(DMX_START+5);      //Tilt servo value
  /*if ( dmx_slave.getChannelValue(DMX_START+6) != NULL ) Special = dmx_slave.getChannelValue(DMX_START+6); //'Special' demo value
  
  
  //If Special is above 50%, run through Adafruit NeoPixel test
  //This demonstrates the panel's ability to trigger programmed
  //animations and that LEDs can still be individually controlled
  if (Special > 129) {
    colorWipe(strip.Color(255, 0, 0), 50); // Red
    colorWipe(strip.Color(0, 255, 0), 50); // Green
    colorWipe(strip.Color(0, 0, 255), 50); // Blue
    rainbow(20);
    rainbowCycle(20);
    Special = 0;`
  }
  else {*/
    
    //Map DMX values to servo rotation degrees
    Pan = map(Pan , 0 , 255 , 0 , 180);
    Tilt = map(Tilt , 0 , 255 , 0 , 180);
    
    //Servo Control
    /*The servos usually shake when left attached even when the
      values haven't changed. To prevent that shaking, the servos
      are only attached when they are meant to move. We track the
      servo's value when it last detached and wait for a new value.
      Speed is controlled by a crude accel/decel code which steps
      each servo through its turn meanwhile accounting for changes
      in the destination value*/
    
    //Pan Servo
    if (lastPan != Pan) {
      //Attach if detached
      if (!newPan) {
        panServo.attach(PANPIN);
        newPan = true;
      }
      //Crude accel/decel
      if (lastPan + 5 < Pan) lastPan = lastPan+5;
      else if (lastPan - 5 > Pan) lastPan = lastPan-5;
      else lastPan = Pan;
      //Write intermediate position to servo
      panServo.write(lastPan);
    }
    //Detach servo when done moving
    else if ((newPan)&&(panServo.read() == lastPan)) {
      panServo.detach();
      newPan = false;
    }
    
    //Tilt Servo
    if (lastTilt != Tilt) {
      //Attach if detached
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
    
    //Panel Control
    
    //Since RGB intensities don't need to be mapped, all we do
    //is multiply by the brightness percentage
    R = (R*BRT)/255;
    G = (G*BRT)/255;
    B = (B*BRT)/255;
    //Cutout if below 2%
    if (R < 5) R = 0;
    if (G < 5) G = 0;
    if (B < 5) B = 0;
    
    //Write LED vaules
    for (uint8_t i=0 ; i < NUMPIXELS ; i++) {
      strip.setPixelColor(i , strip.Color(R,G,B));
    }
    //Update NeoPixel Panel
    strip.show();
    
    
    //Sleep to let servo move and DMX channels update
    delay(60);
  //}
}

//Code below are functions from the Adafruit NeoPixel test sketch
//used in the 'special' demonstration

/*
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
}*/
