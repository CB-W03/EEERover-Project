const int IRPin = A0;  // Analog input pin 
volatile unsigned long startTime;
volatile unsigned long endTime;
volatile bool pulseStarted = false;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int inputValue = analogRead(IRPin);  // Read the analog input value

  if (inputValue > 500) {
    // Rising edge detected (peak value > 100mV)
    if (!pulseStarted) {
      startTime = micros();
      pulseStarted = true;
      delay(1);
    } else {
      endTime = micros();
      unsigned long pulseDuration = endTime - startTime;

      if(pulseDuration > 1035){
      Serial.print("Age: ");
      Serial.print(pulseDuration / 10);
      Serial.println(" years old");
      }

      pulseStarted = false;
    }
  }
}
