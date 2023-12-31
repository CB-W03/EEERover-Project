#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
const int enLeft = 9;
const int enRight = 1;
const int leftDirPin = 2;
const int rightDirPin = 0;
const int ledPin = 5;
int speedLeft, speedRight;

String webpage = "<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'> <meta http-equiv='X-UA-Compatible' content='IE=edge'> <meta name='viewport' content='width=device-width, initial-scale=1.0'> <title>Rover Joystick</title></head><body style=\"overflow: hidden; font-family:'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif; color:black;\"><br> <h1 style='text-align:center'>Rover Joystick</h1> <canvas class='button' id='canvas' style='color:rgb(128, 128, 128); position:relative; top:60px' width='500' height='500'></canvas> <script> var canvas, ctx, socket;window.addEventListener('load', () => { canvas = document.getElementById('canvas'); ctx = canvas.getContext('2d'); resize(); document.addEventListener('mousedown', startDrawing); document.addEventListener('mouseup', stopDrawing); document.addEventListener('mousemove', Draw); document.addEventListener('touchstart', startDrawing); document.addEventListener('touchend', stopDrawing); document.addEventListener('touchcancel', stopDrawing); document.addEventListener('touchmove', Draw); window.addEventListener('resize', resize); socket = new WebSocket('ws://' + window.location.hostname + ':81/'); socket.onopen = function(event){ console.log('WebSocket connection established'); };});function sendCoordinates(x, y){ var coordinates = { X: x, Y: y }; socket.send(JSON.stringify(coordinates));}var width, height, radius, x_orig, y_orig;function resize() { width = window.innerWidth; radius = 75; height = radius * 6.5; ctx.canvas.width = width; ctx.canvas.height = height; background(); joystick(width / 2, height / 3);}function background() { x_orig = width / 2; y_orig = height / 3; ctx.beginPath(); ctx.arc(x_orig, y_orig, radius + 20, 0, Math.PI * 2, true); ctx.fillStyle = '#ECE5E5'; ctx.fill();}function joystick(width, height) { ctx.beginPath(); ctx.arc(width, height, radius, 0, Math.PI * 2, true); ctx.fillStyle = 'purple'; ctx.fill(); ctx.strokeStyle = 'purple'; ctx.lineWidth = 8; ctx.stroke();}let coord = { x: 0, y: 0 };let paint = false;function getPosition(event) { var mouse_x = event.clientX || event.touches[0].clientX; var mouse_y = event.clientY || event.touches[0].clientY; coord.x = mouse_x - canvas.offsetLeft; coord.y = mouse_y - canvas.offsetTop;}function is_it_in_the_circle() { var current_radius = Math.sqrt(Math.pow(coord.x - x_orig, 2) + Math.pow(coord.y - y_orig, 2)); if (radius >= current_radius){return true} else {return false}}function startDrawing(event) { paint = true; getPosition(event); if (is_it_in_the_circle()) { ctx.clearRect(0, 0, canvas.width, canvas.height); background(); joystick(coord.x, coord.y); Draw(); }}function stopDrawing() { paint = false; ctx.clearRect(0, 0, canvas.width, canvas.height); background(); joystick(width / 2, height / 3); sendCoordinates(0, 0);}function Draw(event) { if (paint) { ctx.clearRect(0, 0, canvas.width, canvas.height); background(); var x, y, speed; var angle = Math.atan2((coord.y - y_orig), (coord.x - x_orig)); if (is_it_in_the_circle()) { joystick(coord.x, coord.y); x = coord.x; y = coord.y; } else { x = radius * Math.cos(angle) + x_orig; y = radius * Math.sin(angle) + y_orig; joystick(x, y); } getPosition(event); var x_relative = Math.round(x - x_orig); var y_relative = Math.round(y - y_orig); sendCoordinates(x_relative, y_relative); }} </script></body></html>";


const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PASSWORD";

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

//this function is what will control the motors speed and direction
void processCoords(int x, int y){
  if(y < 0){
  digitalWrite(leftDirPin, HIGH);
  digitalWrite(rightDirPin, HIGH);
  speedRight = map(y, 0, -75, 0, 255);
  speedLeft = map(y, 0, -75, 0, 255);
  }
  else if(y > 0){
  digitalWrite(leftDirPin, LOW);
  digitalWrite(rightDirPin, LOW);
  speedRight = map(y, 0, 75, 0, 255);
  speedLeft = map(y, 0, 75, 0, 255);
  }
  else{
  speedRight = 0;
  speedLeft = 0;
  }
  if(x > 0){
  speedRight-=map(x, 0, 75, 0, 255);
  speedLeft+=map(x, 0, 75, 0, 255);
     if(speedRight < 0){
         speedRight = abs(speedRight);
         digitalWrite(rightDirPin, LOW);
     }
     if(speedLeft > 255){
         speedLeft = 255;
     }
  }
  else if(x < 0){
  speedRight+=map(x, 0, -75, 0, 255);
  speedLeft-=map(x, 0, -75, 0, 255);
    if(speedRight > 255){
       speedRight = 255;
    }
     if(speedLeft < 0){
       speedLeft = abs(speedLeft);
       digitalWrite(leftDirPin, LOW);
      }
  }
  
      analogWrite(enLeft, speedLeft);
      analogWrite(enRight, speedRight);
}

void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length){
  switch(type){
    case WStype_DISCONNECTED:
        Serial.println("Client " + String(num) + " disconnected");
    break;
    case WStype_CONNECTED:
        Serial.println("Client " + String(num) + " connected");
        break;
    case WStype_TEXT:
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
        else{
           int xCoord = doc["X"];
           int yCoord = doc["Y"];
          processCoords(xCoord, yCoord);
        }
        break;
  }
}


void setup(){
  Serial.begin(115200);

  pinMode(enLeft, OUTPUT);
  pinMode(enRight, OUTPUT);
  pinMode(leftDirPin, OUTPUT);
  pinMode(rightDirPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));

  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());
  analogWrite(ledPin, 25);

  server.on("/", []() {
    server.send(200, "text/html", webpage);
  });
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}
void loop(){
  server.handleClient();
  webSocket.loop();
  
}