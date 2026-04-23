#include <TFT_eSPI.h>
#include <TouchScreen.h>

#define YP 14
#define XM 4
#define YM 27
#define XP 16

// Calibration values from your corner readings
#define TS_LEFT   30    // X value at left edge
#define TS_RIGHT  1600  // X value at right edge
#define TS_TOP    3960  // Y value at top edge
#define TS_BOT    2280  // Y value at bottom edge

#define MINPRESSURE 10
#define MAXPRESSURE 4000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("Touch me!", 100, 110);
}

void loop() {
  pinMode(YP, INPUT);
  pinMode(XM, INPUT);
  pinMode(YM, INPUT);
  pinMode(XP, INPUT);
  
  TSPoint p = ts.getPoint();
  
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YM, OUTPUT);
  pinMode(XP, OUTPUT);
  
  if (abs(p.z) > MINPRESSURE && p.x > 10) {
    // Map X: left=30, right=1600 → screen 0 to 320
    int x = map(p.x, TS_LEFT, TS_RIGHT, 0, 320);
    // Map Y: top=3960, bottom=2280 → screen 0 to 240
    int y = map(p.y, TS_TOP, TS_BOT, 0, 240);
    
    x = constrain(x, 0, 320);
    y = constrain(y, 0, 240);
    
    tft.fillCircle(x, y, 5, TFT_RED);
    
    Serial.print("Mapped X="); Serial.print(x);
    Serial.print(" Y="); Serial.println(y);
  }
  delay(50);
}