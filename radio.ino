String name = ""; // String to store the received name
void setup() {

  Serial.begin(9600);
  Serial1.begin(600);
}

void loop() {
  Serial1.end();
  Serial1.begin(600);
delay(500);
  if (Serial1.available()) {
    char receivedChar = Serial1.read();
    // Check for the #  to mark the start of the word
    if (receivedChar == '#') {
      // Clear the existing name
      name = "";

      // Read the next three ASCII symbols and store them in the name string
      for (int i = 0; i < 3; i++) {
        if (Serial1.available()) {
          char nextChar = Serial1.read();
          name += nextChar;
        }
      }

      // Print the received name
      Serial.print("Received name: ");
      Serial.println(name);
    }
  }
}
