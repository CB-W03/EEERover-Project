#define USE_WIFI_NINA   false 
#define USE_WIFI101     true

#include <WiFiWebServer.h>

const char ssid[] = "WIFI_NAME";
const char pass[] = "WIFI_PASSWORD";
const int groupNumber = 0; // Set your group number to make the IP address constant - only do this on the EEERover network

//IR sensor variables:
const int IRPin = A0;  // Analog input pin
volatile unsigned long startTime;
volatile unsigned long endTime;
volatile bool pulseStarted = false;
unsigned long pulseDuration;

//Mag sensor variables:
int initValue = analogRead(A2);
float initVoltage = initValue * (3.3 / 1023.0);

//RF sensor variables:
String name = ""; // String to store the received name


WiFiWebServer server(80);

String irSensorData = "";      // String variable for IR sensor data
String magSensorData = "";     // String variable for magnetism sensor data
String rfSensorData = "";      // String variable for RF sensor data

const char webpage[] = 
"<html> <head> <title>Euphorix</title> <style> table{width:100%}th,td{}th button{cursor:pointer}td button{cursor:pointer}td input{width:100%}.add-row button{cursor:pointer} </style> </head> <body> <table> <thead> <tr> <th><button onclick=\"copyText('Name')\">Name</button></th> <th><button onclick=\"copyText('Age')\">Age</button></th> <th><button onclick=\"copyText('Magnetism')\">Magnetism</button></th> </tr> </thead> <tbody id=\"table-body\"> <tr> <td><input type=\"text\" id=\"name-0\"></td> <td><input type=\"text\" id=\"age-0\"></td> <td><input type=\"text\" id=\"magnetism-0\"></td> </tr> </tbody> </table> <div class=\"add-row\"> <button onclick=\"addRow()\">Add Row</button> </div> <script> let rowCounter=1; function copyText(column){fetch('/data?column='+column).then(response => response.text()).then(text => {const input=document.getElementById(`${column.toLowerCase()}-${rowCounter-1}`);input.value=text;});} function addRow(){const tableBody=document.getElementById(\"table-body\");const newRow=document.createElement(\"tr\");newRow.innerHTML=`<td><input type=\"text\" id=\"name-${rowCounter}\"></td><td><input type=\"text\" id=\"age-${rowCounter}\"></td><td><input type=\"text\" id=\"magnetism-${rowCounter}\"></td>`;tableBody.appendChild(newRow);rowCounter++;} </script> </body> </html>";

String getTextFromBackend(String column) {
  if (column == "Name") {
    return rfSensorData;      // Return RF sensor data
  } else if (column == "Age") {
    return irSensorData;      // Return IR sensor data
  } else if (column == "Magnetism") {
    return magSensorData;     // Return magnetism sensor data
  }
  return "";        // Return empty string if column is not recognized
}

void handleRoot() {
  String webpageWithSensorData = webpage;
  webpageWithSensorData.replace("SENSOR_DATA_NAME", rfSensorData);
  webpageWithSensorData.replace("SENSOR_DATA_AGE", irSensorData);
  webpageWithSensorData.replace("SENSOR_DATA_MAGNETISM", magSensorData);

  server.send(200, "text/html", webpageWithSensorData);
}

void handleDataRequest() {
  if (server.hasArg("column")) {
    String column = server.arg("column");
    String sensorData = getTextFromBackend(column);
    server.send(200, "text/plain", sensorData);
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void readIR() {
   int inputValue = analogRead(IRPin);  // Read the analog input value

  if (inputValue > 500) {
    // Rising edge detected (peak value > 100mV)
    if (!pulseStarted) {
      startTime = micros();
      pulseStarted = true;
      delay(1); //preventing noise as being detected as rising edge
    } else {
      endTime = micros();
      pulseDuration = endTime - startTime;
      String pulseDurationString = String(pulseDuration);

      if(pulseDuration > 1035){
        irSensorData = pulseDurationString;

      }
      else{
          irSensorData = "Retry";

      }
      pulseStarted = false;
    }
  }
    Serial.print("ir loop end" );
  Serial.println(pulseDuration / 10);
}

void readMag() {

  int sensorValue = analogRead(A2);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.3V):
  float voltage = sensorValue * (3.3 / 1023.0);
  
  if(voltage > (initVoltage + 0.1)){
    magSensorData = "South - Up";
        Serial.println("South");

  }

  else if(voltage < (initVoltage - 0.1)){
    magSensorData = "North - Down ";
    Serial.println("North");
  }

  else{
    magSensorData = "no field";
  }
 
}

void readRF() {
  Serial1.end();
  Serial1.begin(600);
  delay(500);
if (Serial1.available()) {
    char receivedChar = Serial1.read();
    // Check for the '#' symbol to mark the start of the word
    if (receivedChar == '#') {
      // Clear the existing name
      name = "";

      // Read the next three ASCII symbols and store them in the 'name' string
      for (int i = 0; i < 3; i++) {
        if (Serial1.available()) {
          char nextChar = Serial1.read();
          name += nextChar;
        }
      }

    }
  }  
  rfSensorData = name;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(9600);
  Serial1.begin(600);

  while (!Serial && millis() < 10000);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  if (groupNumber)
    WiFi.config(IPAddress(192, 168, 0, groupNumber + 1));

  Serial.print("Connecting to WPA SSID: ");
  Serial.println(ssid);

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }

  server.on("/", handleRoot);
  server.on("/data", handleDataRequest);

  // server.onNotFound([]() {
  //   String message = "File Not Found\n\n";
  //   message += "URI: " + server.uri() + "\n";
  //   message += "Method: " + (server.method() == HTTP_GET ? "GET" : "POST") + "\n";
  //   message += "Arguments: " + server.args() + "\n";
  //   for (uint8_t i = 0; i < server.args(); i++) {
  //     message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  //   }
  //   server.send(404, "text/plain", message);
  // });

  server.begin();
  
  Serial.print("HTTP server started @ ");
  Serial.println(static_cast<IPAddress>(WiFi.localIP()));
}

void loop() {
  server.handleClient();

  // Update sensor data
  readIR();
  readMag();
  readRF();

}
