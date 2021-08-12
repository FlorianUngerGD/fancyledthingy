#include <Adafruit_CircuitPlayground.h>
//#include <Adafruit_NeoPixel.h>
#include <Adafruit_CPlay_NeoPixel.h>

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LEDS        10  // The number of available LEDs
#define BINS        32  // The number of FFT frequency bins
#define FRAMES      4   // This many FFT cycles are averaged

// Frequency ranges with 150Hz per step
#define START_MID   2
#define START_HIGH  12

// Equalizer as factor
#define EQ_LOW      8
#define EQ_MID      1
#define EQ_HIGH     1.5

#define ACCENT      3   // Accent as power
#define SMOOTH      0.8 // Smoothness as ratio of old Value

// Bounce Settings
#define SENSIBILITY_BOUNCE 30
#define THRESHOLD_BOUNCE 12
#define DECAY_BOUNCE 0.1
#define DECAY_POW_BOUNCE 3

// NeoPixel Settings
#define LED_PIN     12 // Which pin on the Arduino is connected to the NeoPixels
#define LED_COUNT  1 // How many NeoPixels are attached to the Arduino

// Declare our NeoPixel strip object:
Adafruit_CPlay_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

int smoothLow = 0;
int smoothMid = 0;
int smoothHigh = 0;

float l = 0.0f;
float m = 0.0f;
float h = 0.0f;

int bounce = 0;
float smoothIntensity = 0;

void setup() {
  CircuitPlayground.begin();
  Serial.begin(9600);
  Serial.println("Start");
  
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_8_G); // setup accelerometer

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
}

void loop() {
  setBounce();
  setSpectrum();
  //showPressureLevel(10, 50, 100, (int) (l * 255), (int) (m * 255), (int) (h * 255));
}

///////////////////////////////////////////////////////////////////////////////
void setBounce() {
  int motionX = -CircuitPlayground.motionX();
  float intensity = (float) (motionX - THRESHOLD_BOUNCE) / SENSIBILITY_BOUNCE;
  if (intensity < 0) {
    intensity = 0;
  }
  if (intensity >= smoothIntensity) {
    smoothIntensity = intensity;
  } else {
    smoothIntensity = smoothIntensity - DECAY_BOUNCE;
    if (smoothIntensity < 0) {
      smoothIntensity = 0;
    }
  }
  float newIntensity = powf(smoothIntensity, DECAY_POW_BOUNCE);
  Serial.println(newIntensity);
  CircuitPlayground.setBrightness(newIntensity * 255);
  
  strip.setBrightness(newIntensity * 255);
}

///////////////////////////////////////////////////////////////////////////////
void setSpectrum() {
  uint8_t i,j;
  uint16_t spectrum[BINS];     // FFT spectrum output buffer
  uint16_t avg[BINS];          // The average of FRAME "listens"

  // Get spectrum data over FRAMES avarage
  for(j=1; j <= FRAMES; j++) {             // We gather data FRAMES times and average it
     CircuitPlayground.mic.fft(spectrum);  // Here is the CP listen and FFT the data routine
     for(i=0; i < BINS; i++) {             // Add for an average
       if(spectrum[i] > 255) spectrum[i] = 255; // limit outlier data
       
       if(i == 0)
         avg[i] = spectrum[i];
       else
         avg[i] = avg[i] + spectrum[i];
     }
  }
  for(i=0; i < BINS; i++) {               // For each output bin average
    avg[i] = avg[i] / FRAMES;            //  divide about the number of values aaveraged
  }

  // Get total value for lows, mids and highs
  int low = 0;
  int mid = 0;
  int high = 0;
  for(i=0; i < BINS; i++) {
    if (i < START_MID) {
      low = low + avg[i] * EQ_LOW;
    } else {
      if (i < START_HIGH) {
        mid = mid + avg[i] * EQ_MID;
      } else {
        high = high + avg[i] * EQ_HIGH;
      }
    }
  }

  // Smooth in the current lows, mids and highs
  smoothLow = SMOOTH * smoothLow + (1 - SMOOTH) * low;
  smoothMid = SMOOTH * smoothMid + (1 - SMOOTH) * mid;
  smoothHigh = SMOOTH * smoothHigh + (1 - SMOOTH) * high;

  // Intensify the current freuquency
  float partialLow, partialMid, partialHigh;
  partialLow = powf((float) smoothLow, ACCENT);
  partialMid = powf((float) smoothMid, ACCENT);
  partialHigh = powf((float) smoothHigh, ACCENT);

  // Normalize values so that low + mid + high = 1
  float partialSum = partialLow + partialMid + partialHigh;
  l = partialLow / partialSum;
  m = partialMid / partialSum;
  h = partialHigh / partialSum;
  
  for (int i = 0; i < LEDS; i++) {
    CircuitPlayground.setPixelColor(i, l * 255, m * 255, h * 255);
  }
  
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, l * 255, m * 255, h * 255);
  }
}

///////////////////////////////////////////////////////////////////////////////
void showPressureLevel(int time, int min, int max, int r, int g, int b) {
  int level = (CircuitPlayground.mic.soundPressureLevel(time) - min) / ((max - min) / LEDS);
  for (int i = 0; i < LEDS; i++) {
    if (level >= i) {
      CircuitPlayground.setPixelColor(i, r, g, b);
    } else {
      CircuitPlayground.setPixelColor(i, 0, 0, 0);
    }
  }
}
