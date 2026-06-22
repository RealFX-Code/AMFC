
// To change the amount of faders attached,
// Change this value. ( AND top_value and bot_value!!!)
#define NUMOFPINS 4
#define DEADBAND 4        // Ignore changes smaller than this

// Point function to address 0x00000000
void(*reset) (void) = 0x00000000;

const int pins[NUMOFPINS] = { A0, A1, A2, A3 };
int pinValues[NUMOFPINS] = { 0, 0, 0, 0 };
unsigned long lastSent[NUMOFPINS] = { 0, 0, 0, 0 };

void setup() {
  Serial.begin(115200);
  // Wait one second before starting starting loop();
  Serial.setTimeout(100);
  for (int i = 0; i < 10; i++) {
    Serial.print("Reading for \"CALIBRATE\"... try ");
    Serial.print(i);
    Serial.print("...\n");
    if (Serial.readString() == "CALIBRATE") {
      calibrate();
      // Wait indefinitely until reset
      for(;;) {
        if(Serial.readString() == "RESET") {
          reset();
        }
      }
    }
  }
  // Poll every 1ms
  Serial.setTimeout(1);
}

void calibrate() {
  int top_value[NUMOFPINS] = { 0, 0, 0, 0 };
  int bot_value[NUMOFPINS] = { 0, 0, 0, 0 };

  // Tell the computer we're ready
  Serial.println("READY4CAL");

  // Begin Calibrate
  
  Serial.println("WAIT4READY");
  for(;;) {
    if(Serial.readString() == "READY") {
      break;
    }
  }
  
  for (int i = 0; i < NUMOFPINS; i++) {
    bot_value[i] = analogRead(pins[i]);
  }

  Serial.println("WAIT4READY");
  for(;;) {
    if(Serial.readString() == "READY") {
      break;
    }
  }

  for (int i = 0; i < NUMOFPINS; i++) {
    top_value[i] = analogRead(pins[i]);
  }

  // Send values to computer
  for (int i = 0; i < NUMOFPINS; i++) {
    Serial.print(i);
    Serial.print(",");
    Serial.print(bot_value[i]);
    Serial.print(",");
    Serial.print(top_value[i]);
    Serial.print("\n");
  }
  Serial.println("DONEWCAL");
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
    Serial.print("\n");
  }
}
