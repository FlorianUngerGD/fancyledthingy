#include <Adafruit_CircuitPlayground.h>

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
 

int smoothLow = 0;
int smoothMid = 0;
int smoothHigh = 0;

void setup() {
  CircuitPlayground.begin();
  Serial.begin(9600);
  Serial.println("Start");
}

void loop() {
  uint8_t i,j;
  uint16_t spectrum[BINS];     // FFT spectrum output buffer
  uint16_t avg[BINS];          // The average of FRAME "listens"
  
  int total = 0;
  int iterTotal = 0;
  int percentile25 = 0;
  int percentile50 = 0;
  
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
    iterTotal = iterTotal + avg[i];
    if (!percentile25 && iterTotal > total / 3)
      percentile25 = i;
    if (!percentile50 && iterTotal > total * 2 / 3)
      percentile50 = i;
      
    avg[i] = avg[i] / FRAMES;            //  divide about the number of values aaveraged
     

    total += avg[i];
  }

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

  smoothLow = SMOOTH * smoothLow + (1 - SMOOTH) * low;
  smoothMid = SMOOTH * smoothMid + (1 - SMOOTH) * mid;
  smoothHigh = SMOOTH * smoothHigh + (1 - SMOOTH) * high;

  float partialLow, partialMid, partialHigh;
  partialLow = powf((float) smoothLow, ACCENT);
  partialMid = powf((float) smoothMid, ACCENT);
  partialHigh = powf((float) smoothHigh, ACCENT);
  
  float partialSum = partialLow + partialMid + partialHigh;
  partialLow = partialLow / partialSum;
  partialMid = partialMid / partialSum;
  partialHigh = partialHigh / partialSum;


/*
  if (low >= smoothLow)
    smoothLow = low;
  else smoothLow = smoothLow - 1;
  if (mid >= smoothMid)
    smoothMid = mid;
  else smoothMid = smoothMid - 1;
  if (high >= smoothHigh)
    smoothHigh = high;
  else smoothHigh = smoothHigh - 1;
*/
/*
  for (i=0; i < BINS; i++) {
    if (i < 10) {Serial.print(" ");}
    Serial.print(i); Serial.print(" | ");
    if (avg[i] < 10) {Serial.print(" ");}
    if (avg[i] < 100) {Serial.print(" ");}
    Serial.print(avg[i]); Serial.print(" ");
    for (j=0; j < avg[i] * 300 / total; j++) {
      if (i >= START_MID && i < START_HIGH) {
        Serial.print("~");
      } else {
        Serial.print("-"); 
      }
    }
    Serial.println("");
  }
*/
  Serial.print("L ");
  for (i=0; i < (int) (partialLow * 20); i++) {
    Serial.print("|");
  }
  Serial.println("");
  
  Serial.print("M ");
  for (i=0; i < (int) (partialMid * 20); i++) {
    Serial.print("|");
  }
  Serial.println("");
  
  Serial.print("H ");
  for (i=0; i < (int) (partialHigh * 20); i++) {
    Serial.print("|");
  }
  Serial.println("");
  Serial.println("");
/*
  Serial.println((int) (partialLow * 255));
  Serial.println((int) (partialMid * 255));
  Serial.println((int) (partialHigh * 255));
  Serial.println("");
  */

  
  showPressureLevel(10, 50, 100, (int) (partialLow * 255), (int) (partialMid * 255), (int) (partialHigh * 255));
}

///////////////////////////////////////////////////////////////////////////////
void showPressureLevel(int time, int min, int max, int r, int g, int b) {
  int level = (CircuitPlayground.mic.soundPressureLevel(time) - min) / ((max - min) / LEDS);
  for (int i = 0; i < LEDS; i++) {
    if (100 >= i) {
      CircuitPlayground.setPixelColor(i, r, g, b);
    } else {
      CircuitPlayground.setPixelColor(i, 0, 0, 0);
    }
  }
}
