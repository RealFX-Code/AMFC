
// To change the amount of faders attached,
// Change this value. ( AND top_value and bot_value!!!)
#define NUMOFPINS 4
#define DEADBAND 4        // Ignore changes smaller than this

// Point function to address 0x00000000
void(*reset) (void) = 0x00000000;

// Set this to 1 to enable use of the SSD1306 oled screen
#define USE_OLED 0

#if USE_OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
#endif

const int pins[NUMOFPINS] = { A0, A1, A2, A3 };
int pinValues[NUMOFPINS] = { 0, 0, 0, 0 };
unsigned long lastSent[NUMOFPINS] = { 0, 0, 0, 0 };

int top_value[NUMOFPINS] = {
  0,
  0,
  0,
  0
};

int bot_value[NUMOFPINS] = {
  0,
  0,
  0,
  0
};

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(250);

#if USE_OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // I2C Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer, set small font size and print 4 lines of text to display
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Put sliders to bottom");
  display.display();
#endif

  // Begin Calibrate
  Serial.println("Send READY when all sliders are in the BOTTOM position...");
  
  for(;;) {
    if(Serial.readString() == "READY") {
      break;
    }
  }
  
  for (int i = 0; i < NUMOFPINS; i++) {
    bot_value[i] = analogRead(pins[i]);
    Serial.print(i);
    Serial.print(" bottom = ");
    Serial.print(bot_value[i]);
    Serial.print("\n");
  }

#if USE_OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Put sliders to top");
  display.display();
#endif

  Serial.println("Send READY when all sliders are in the TOP position...");
  
  for(;;) {
    if(Serial.readString() == "READY") {
      break;
    }
  }

  for (int i = 0; i < NUMOFPINS; i++) {
    top_value[i] = analogRead(pins[i]);
    Serial.print(i);
    Serial.print(" top = ");
    Serial.print(top_value[i]);
    Serial.print("\n");
  }
  Serial.print("Calibration data:");
  for (int i = 0; i < NUMOFPINS; i++) {
    Serial.print("  Slider ");
    Serial.print(i);
    Serial.print(": bot=");
    Serial.print(bot_value[i]);
    Serial.print(" top=");
    Serial.print(top_value[i]);
  }
  Serial.print("\n");
  Serial.setTimeout(1);
}

void loop() {
  if(Serial.readString() == "RESET") {
    reset();
  }

#if USE_OLED
  // Clear screen and move cursor to top left
  display.clearDisplay();
  display.setCursor(0,0);
#endif
  for (int i = 0; i < NUMOFPINS; i++) {

    int newValue = analogRead(pins[i]);

#if USE_OLED
    char output[100];
    // %04d = 04 digit double
    sprintf(output, "Slider %i: %04d/%04d", i, newValue, top_value[i]);
    display.println(output);
#endif

    if (newValue == pinValues[i])
      continue;

    bool atLimit = (newValue <= DEADBAND || newValue >= 1023 - DEADBAND);
    if (abs(newValue - pinValues[i]) < DEADBAND && !atLimit) continue;

    pinValues[i] = newValue;

    Serial.print(i);
    Serial.print(",");
    Serial.print(newValue);
    Serial.print(",");
    Serial.print(bot_value[i]);
    Serial.print(",");
    Serial.print(top_value[i]);
    Serial.print("\n");
  }
#if USE_OLED
  display.display();
#endif
}
