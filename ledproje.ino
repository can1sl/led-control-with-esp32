#include <WiFi.h>
#include <ESP32Servo.h>

const char* ssid = "OzcanElektrik";
const char* password = "ozcan2022";
WiFiServer server(80);

const int outputPin = 25;
const int buzzerPin = 14;
const int servoPin = 14;
Servo servoMotor;

String pinState = "off";
int servoAngle = 0;

void setup() {
  Serial.begin(115200);
  pinMode(outputPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  servoMotor.attach(servoPin);

  digitalWrite(outputPin, HIGH);
  digitalWrite(buzzerPin, LOW);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  handleClient();
  moveServo();
}

void handleClient() {
  WiFiClient client = server.available();
  if (client) {
    unsigned long currentTime = millis();
    unsigned long previousTime = currentTime;

    Serial.println("New Client.");
    String currentLine = "";

    while (client.connected() && (currentTime - previousTime) <= 2000) {
      currentTime = millis();

      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        currentLine += c;

        if (currentLine.endsWith("\r\n\r\n")) {
          processRequest(client, currentLine);
          break;
        }
      }
    }

    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void processRequest(WiFiClient client, String request) {
  Serial.println("Processing request: " + request);
  
  if (request.indexOf("/on") != -1) {
    handleOnRequest();
  } else if (request.indexOf("/off") != -1) {
    handleOffRequest();
  } else if (request.indexOf("/angle") != -1) {
    handleAngleRequest(request);
  }
  
  sendResponse(client);
}

void handleOnRequest() {
  Serial.println("GPIO pin on");
  pinState = "on";
  digitalWrite(outputPin, LOW);
  digitalWrite(buzzerPin, HIGH);
  servoAngle = 180;
  servoMotor.write(servoAngle);
}

void handleOffRequest() {
  Serial.println("GPIO pin off");
  pinState = "off";
  digitalWrite(outputPin, HIGH);
  digitalWrite(buzzerPin, LOW);
  servoAngle = 0;
  servoMotor.write(servoAngle);
}

void handleAngleRequest(String request) {
  int index = request.indexOf("/angle=");
  if (index != -1) {
    String angleValue = request.substring(index + 7);
    servoAngle = angleValue.toInt();
    Serial.println("Servo angle set to: " + String(servoAngle));
    servoMotor.write(servoAngle);
  }
}

void moveServo() {
}

void sendResponse(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE html>");
  client.println("<html lang=\"en\">");
  client.println("<head>");
  client.println("</head>");
  client.println("<body>");

  client.println("<header>");
  client.println("<h1>Tubitak 2204/A Evcil Hayvan Besleme Otomasyon Projesi</h1>");
  client.println("<h2>Can Ahmet Soydal</h2>");
  client.println("</header>");

  client.println("<div class=\"container\"> <br> <br>");
  client.println("<p>LED GPIO " + String(outputPin) + " - State " + pinState + "</p>");
  client.println("<p>Buzzer GPIO " + String(buzzerPin) + " - State " + (digitalRead(buzzerPin) == HIGH ? "on" : "off") + "</p>");

  if (pinState == "off") {
    client.println("<p><a href=\"/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    client.println("<p><a href=\"/off\"><button class=\"button button2\">OFF</button></a></p>");
  }
  
  client.println("<p>Current Servo Angle: " + String(servoAngle) + "</p>");
  client.println("<p>Set Servo Angle: <input type=\"range\" min=\"0\" max=\"180\" value=\"" + String(servoAngle) + "\" step=\"1\" onchange=\"updateServoAngle(this.value)\"></p>");
  
  client.println("</div>");

  client.println("<script>");
  client.println("function updateServoAngle(angle) {");
  client.println("  var xhttp = new XMLHttpRequest();");
  client.println("  xhttp.open('GET', '/angle=' + angle, true);");
  client.println("  xhttp.send();");
  client.println("}");
  client.println("</script>");

  client.println("</body></html>");
}
