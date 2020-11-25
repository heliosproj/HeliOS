/*
 * Include the standard HeliOS header for Arduino sketches. This header
 * includes the required HeliOS header files automatically.
 */
#include <HeliOS_Arduino.h>

volatile int ledState = 0;
volatile int buttonState = 0;
const int buttonPin = 2;
const int ledPin = 4;

void taskBlink(xTaskId id_) {

  if (ledState) {

    digitalWrite(LED_BUILTIN, LOW);

    ledState = 0;
  } else {

    digitalWrite(LED_BUILTIN, HIGH);

    ledState = 1;
  }
}

void buttonRead(xTaskId id_) {
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
} 

void setup() {

  xTaskId id = 0;

  xHeliOSSetup();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  id = xTaskAdd("TASKBLINK", &taskBlink);

  xTaskWait(id);

  xTaskSetTimer(id, 2000000);

  id = xTaskAdd("BUTTON", &buttonRead);

  xTaskStart(id);
}

void loop() {

  xHeliOSLoop();
}
