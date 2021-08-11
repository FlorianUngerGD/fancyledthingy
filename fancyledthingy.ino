/*
  DigitalReadSerial

  Reads a digital input on pin 2, prints the result to the Serial Monitor

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/DigitalReadSerial
*/
#include <Adafruit_CircuitPlayground.h>

// digital pin 2 has a pushbutton attached to it. Give it a name:
int pushButton = 2;

// the setup routine runs once when you press reset:
void setup() {
  CircuitPlayground.begin();
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);


  // setup accelerometer:
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_8_G);
}

// the loop routine runs over and over again forever:
void loop() {
  // accelerometer stuff:
  int motionX = CircuitPlayground.motionX();

  int state = - motionX;
  
  // print out the state of the accelerometer:
  Serial.println("accelerometer: " + state);
  
  int asdf = state * 10 / 40;

  for (int i = 0; i < 10; i++) {
    if(asdf > i) {
      CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(85));
    }
  }
  
  delay(100);        // delay in between reads for stability
  CircuitPlayground.clearPixels();
  
}
