const int BUTTON = 2;
const int LED = 3;

int state = HIGH;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long time = 0;           // the last time the output pin was toggled
unsigned long debounce = 200UL;   // the debounce time, increase if the output flickers

void setup()
{
  pinMode(BUTTON, INPUT);
  pinMode(LED, OUTPUT);
}

void loop()
{
  reading = digitalRead(BUTTON);

  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time

    if (state == HIGH){
      state = LOW;
      digitalWrite(LED, HIGH);
    }
    else{
      state = HIGH;
       digitalWrite(LED, LOW);
    }
    time = millis();

  digitalWrite(LED, state);

 previous = reading;
}
