#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "PutYourWifiNamehere!!";
const char* password = "DontHackMyWifi!";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int soundPins[] = {25, 26, 27, 14, 12};
const int numSounds = sizeof(soundPins) / sizeof(soundPins[0]);

WiFiServer server(80);

void setup() {
  for (int i = 0; i < numSounds; i++) {
    pinMode(soundPins[i], OUTPUT);
    digitalWrite(soundPins[i], HIGH);
  }

  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (true);
  }
  display.clearDisplay();
  display.display();

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
    updateDisplay("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  String ipAddress = WiFi.localIP().toString();
  updateDisplay("IP: " + ipAddress);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("New Client Connected");
  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  for (int i = 0; i < numSounds; i++) {
    String soundTrigger = "/sound" + String(i + 1);
    if (request.indexOf("GET " + soundTrigger) >= 0) {
      triggerSound(i);
      break;
    }
  }

String html = "<!DOCTYPE html><html>";
html += "<head>";
html += "<title>ESP32 Sound Control</title>";
html += "<style>";
html += "button {";
html += "  font-size: 24px;";
html += "  padding: 20px;";
html += "  margin: 10px;";
html += "  width: 90%;";
html += "  max-width: 300px;";
html += "}";
html += "body { text-align: center; }";
html += "</style>";
html += "</head>";
html += "<body>";
html += "<h1>ESP32 Sound Control</h1>";
for (int i = 0; i < numSounds; i++) {
  html += "<button onclick=\"location.href='/sound" + String(i + 1) + "'\">Play Sound " + String(i + 1) + "</button><br>";
}
html += "</body></html>";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(html);

  client.stop();
  Serial.println("Client Disconnected");
}

void triggerSound(int soundIndex) {
  digitalWrite(soundPins[soundIndex], LOW);
  delay(100);
  digitalWrite(soundPins[soundIndex], HIGH);

  String message = "IP: " + WiFi.localIP().toString() + "\nSound " + String(soundIndex + 1) + " Played";
  updateDisplay(message);
  Serial.println("Sound " + String(soundIndex + 1) + " Played");
}

void updateDisplay(String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(message);
  display.display();
}