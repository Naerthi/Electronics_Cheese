void setup() {
  Serial.begin(115200);
}

void loop() {
  int raw = analogRead(A0);
  float voltage = raw * (5.0 / 1023.0);

  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print("  Voltage: ");
  Serial.println(voltage);

  float scaled_volt = voltage*10;

  Serial.print("  Scaled volts: ");
  Serial.println(scaled_volt);

  delay(100);
}
