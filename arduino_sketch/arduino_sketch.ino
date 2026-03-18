
// To change the amount of faders attached,
// Change this value. ( AND top_value and bot_value!!!)
#define NUMOFPINS 4
#define DEADBAND 4        // Ignore changes smaller than this

// Point function to address 0x00000000
void(*reset) (void) = 0x00000000;

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
  for (int i = 0; i < NUMOFPINS; i++) {

    int newValue = analogRead(pins[i]);

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
}