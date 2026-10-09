#include <Arduino.h>

const uint8_t BUTTON_PIN     = 4;
const uint8_t POT_PIN        = 34;
const uint8_t STATUS_LED_PIN = 18;
const uint8_t PWM_LED_PIN    = 19;

const uint32_t PWM_FREQ_HZ   = 5000;
const uint8_t  PWM_RES_BITS  = 8;
const uint8_t  PWM_CHANNEL   = 0; 

bool buttonPressed = false;
bool pwmReady = false;
int rawInput = 0;
int requestedDuty = 0;
int appliedDuty = 0;

void readInputs();
void processInputs();
void updateOutputs();

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  analogReadResolution(12);
  analogSetPinAttenuation(POT_PIN, ADC_11db);

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  pwmReady = ledcAttach(PWM_LED_PIN, PWM_FREQ_HZ, PWM_RES_BITS);
  if (pwmReady) ledcWrite(PWM_LED_PIN, 0);
#else
  ledcSetup(PWM_CHANNEL, PWM_FREQ_HZ, PWM_RES_BITS);
  ledcAttachPin(PWM_LED_PIN, PWM_CHANNEL);
  pwmReady = true;
  ledcWrite(PWM_CHANNEL, 0);
#endif

  if (!pwmReady) {
    Serial.println("PWM setup failed.");
  }
}

void loop() {
  readInputs();
  processInputs();
  updateOutputs();
  delay(20);
}

void readInputs() {
  buttonPressed = (digitalRead(BUTTON_PIN) == LOW);
  rawInput = analogRead(POT_PIN);
}

void processInputs() {
  requestedDuty = constrain(map(rawInput, 0, 4095, 0, 255), 0, 255);
  appliedDuty = (buttonPressed && pwmReady) ? requestedDuty : 0;
}

void updateOutputs() {
  digitalWrite(STATUS_LED_PIN, (buttonPressed && pwmReady) ? HIGH : LOW);

  if (!pwmReady) return;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PWM_LED_PIN, appliedDuty);
#else
  ledcWrite(PWM_CHANNEL, appliedDuty);
#endif

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 200) {
    lastPrint = millis();
    Serial.printf("Raw ADC= %d\tRequested= %d\tApplied PWM= %d\n",
                  rawInput, requestedDuty, appliedDuty);
  }
}