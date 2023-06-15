  int initValue = analogRead(A2); 
  float initVoltage = initValue * (3.3 / 1023.0);

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("current initial voltage is:" );
  Serial.println(initVoltage);
  int sensorValue = analogRead(A2);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.3V):
  float voltage = sensorValue * (3.3 / 1023.0);
  
  if(voltage > (initVoltage + 0.1)){
    Serial.print("South - Up");
  }

  else if(voltage < (initVoltage - 0.1)){
    Serial.print("North - Down ");
  }

  else{
    Serial.print("no field");
  }
  Serial.print(" Voltage:");
  Serial.println(voltage);
  delay(1000);
}
