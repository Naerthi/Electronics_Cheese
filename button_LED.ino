const int buttonPin = 2;   // your button
const int ledPin = 9;      // your LED

bool ledState = false;         // current LED state
bool lastButtonState = LOW;    // because you use a pull-down resistor

void setup() {
    pinMode(buttonPin, INPUT);   // external pull-down, so INPUT is correct
    pinMode(ledPin, OUTPUT);
    Serial.begin(9600);
}

void loop() {
    bool buttonState = digitalRead(buttonPin);

    // Detect rising edge (LOW -> HIGH)
    if (buttonState == HIGH && lastButtonState == LOW) {
        ledState = !ledState;          // toggle LED
        digitalWrite(ledPin, ledState);
        delay(200);                    // debounce
    }

    lastButtonState = buttonState;
    Serial.println(buttonState);
}
