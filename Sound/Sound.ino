#include <Adafruit_CircuitPlayground.h>

void setup() {
  // Initialize the Circuit Playground
  CircuitPlayground.begin();
  // https://www.arduino.cc/en/Tutorial/getting-started-with-ide-v2/ide-v2-serial-monitor
  Serial.begin(9600);
  Serial.println("Start");
  CircuitPlayground.clearPixels();
}

void loop() {
  // put your main code here, to run repeatedly:
  int pressure = CircuitPlayground.mic.soundPressureLevel(10);
  int strength = (pressure - 50) / 5;

  uint16_t* spectrum;
  CircuitPlayground.mic.fft(spectrum);
  char *test;
  sprintf(test, "%b", spectrum);
  Serial.println(test);
  
  CircuitPlayground.clearPixels();
  for (int i = 0; i < 10; i++) {
    if (strength > i) {
      CircuitPlayground.setPixelColor(i, 255, 255, 255);
    }
  }
}
