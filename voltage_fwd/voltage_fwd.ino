const int pins[][2] = {
  {40, 41},
  {44, 45},
  {48, 49},
  {52, 53}
};
#define PIN_COUNT 4

void setup() {
  for (unsigned i = 0; i < PIN_COUNT; ++i) {
    pinMode(pins[i][0], INPUT_PULLUP);
    pinMode(pins[i][1], OUTPUT);
  }
}

void loop() {
  for (unsigned i = 0; i < PIN_COUNT; ++i) {
    // the logic between the Nano and Arduino is inverted so we can use the pull-ups
    digitalWrite(pins[i][1], (digitalRead(pins[i][0]) == HIGH) ? LOW : HIGH);
  }

}
