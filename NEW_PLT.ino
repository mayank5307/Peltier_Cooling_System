#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>

#define ONE_WIRE_BUS 4
#define RPWM 26
#define LPWM 27
#define R_EN 33
#define L_EN 32
#define BUZZER 2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// WiFi & Web Server
const char* ssid = "CANDOR"; // CHANGE ME
const char* password = "a1b2c3d4e5"; // CHANGE ME
WebServer server(80);

// PID variables
double currentTemp;
double targetTemp = 0;
double outputPWM = 0;
double Kp = 20, Ki = 0.5, Kd = 15;
PID myPID(&currentTemp, &outputPWM, &targetTemp, Kp, Ki, Kd, DIRECT);

// Profile structure
struct TempProfile 
{
  double temp;
  int durationSec;
};

TempProfile profile[] = 
{
  //{100, 7 * 60},
  //{35, 3 * 60},
  {5, 5 * 60},
  {22, 5 * 60},
  {36, 10 * 60}
};

int profileCount = sizeof(profile) / sizeof(profile[0]);
int currentStep = 0;
unsigned long stepStartMillis = 0;

void stopPeltier() 
{
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

// Web server handler to display and update the profile
void handleRoot() {
  String html = "<html><head><title>Peltier Controller</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<style>body { font-family: sans-serif; margin: 20px; } h1, h3 { color: #333; } form { background-color: #f4f4f4; padding: 20px; border-radius: 8px; }";
  html += "label { display: inline-block; width: 120px; margin-bottom: 10px; } input[type='number'] { width: 80px; padding: 5px; } input[type='submit'] { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }";
  html += "</style></head><body><h1>Peltier Temperature Profile</h1>";
  html += "<p>Current Step: " + String(currentStep + 1) + " / " + String(profileCount) + "</p>";
  html += "<p>Current Temp: " + String(currentTemp) + " &deg;C</p>";
  html += "<form action='/update' method='POST'>";

  for (int i = 0; i < profileCount; i++) {
    html += "<h3>Step " + String(i + 1);
    html += " <a href='/restart?step=" + String(i) + "' style='background-color: #008CBA; color: white; padding: 5px 10px; text-decoration: none; border-radius: 4px; font-size: 14px; margin-left: 10px;'>Restart</a></h3>";
    html += "<label>Temp (&deg;C): </label><input type='number' step='0.1' name='temp" + String(i) + "' value='" + String(profile[i].temp) + "'><br>";
    html += "<label>Duration (min): </label><input type='number' name='dur" + String(i) + "' value='" + String(profile[i].durationSec / 60) + "'><br>";
  }

  html += "<br><input type='submit' value='Update Profile'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleUpdate() {
  for (int i = 0; i < profileCount; i++) {
    String tempArg = "temp" + String(i);
    String durArg = "dur" + String(i);
    if (server.hasArg(tempArg) && server.hasArg(durArg)) {
      profile[i].temp = server.arg(tempArg).toDouble();
      profile[i].durationSec = server.arg(durArg).toInt() * 60;
    }
  }
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "Updated!");
  Serial.println("Profile updated via web interface.");
  
  display.clearDisplay();
  display.setCursor(0, 8);
  display.setTextSize(1);
  display.print("Profile Updated!");
  display.display();
  delay(1500);
}

void handleRestart() {
  if (server.hasArg("step")) {
    int s = server.arg("step").toInt();
    if (s >= 0 && s < profileCount) {
      currentStep = s;
      stepStartMillis = millis(); // Reset the timer
      
      display.clearDisplay();
      display.setCursor(0, 8);
      display.setTextSize(1);
      display.print("Restart Step ");
      display.print(currentStep + 1);
      display.display();
      delay(1000);
    }
  }
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "Restarted!");
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.on("/restart", HTTP_GET, handleRestart);
  server.begin();
  Serial.println("HTTP server started");
}

void setup() 
{
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Halt execution if OLED fails to initialize
  }
  display.clearDisplay();
  sensors.begin();

  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  stopPeltier();
  myPID.SetOutputLimits(0, 255);
  myPID.SetMode(AUTOMATIC);

  display.setTextSize(2); // Larger text for starting message
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 8);
  display.print("Starting");
  display.display();
  delay(2000);

  // --- WiFi Setup ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Connecting to WiFi...");
  display.display();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(retries++ > 20) { // ~10 second timeout
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("WiFi Connect Failed!");
        display.display();
        delay(3000);
        break; // Continue without WiFi
    }
  }

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    // Show WiFi name
    display.clearDisplay();
    display.setCursor(0, 8);
    display.setTextSize(1);
    display.print("WiFi: ");
    display.print(ssid);
    display.display();
    delay(2000);

    // Show IP address
    display.clearDisplay();
    display.setCursor(0, 8);
    display.print("IP: ");
    display.print(WiFi.localIP());
    display.display();
    delay(2000);

    setupWebServer();
  }

  display.clearDisplay();
  display.display();

  stepStartMillis = millis(); // Initialize the step timer

  Serial.println("Temperature Program Started");
}

void loop() 
{
  if(WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }

  unsigned long elapsedSec = (millis() - stepStartMillis) / 1000;
  int remainingTime = profile[currentStep].durationSec - elapsedSec;
  if (remainingTime < 0) remainingTime = 0;

  sensors.requestTemperatures();
  currentTemp = sensors.getTempCByIndex(0);
  targetTemp = profile[currentStep].temp;
  myPID.Compute();

  // Display on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("Set: ");
  display.print(targetTemp, 1);

  display.setCursor(70, 0);
  display.print("Step:");
  display.print(currentStep + 1);
  display.print("/");
  display.print(profileCount);

  display.setCursor(0, 16); // Mimics second row (y=1 on LCD)
  display.print("Now: ");
  display.print(currentTemp, 1);
  
  display.setCursor(80, 16); // Placed equivalent to col 11 on old LCD
  display.print(remainingTime / 60);
  display.print(":");
  int sec = remainingTime % 60;
  if (sec < 10) display.print("0");
  display.print(sec);
  display.display(); // Push buffer to screen

  // Serial Monitor Output
  Serial.print("Step ");
  Serial.print(currentStep + 1);
  Serial.print("/");
  Serial.print(profileCount);
  Serial.print(",Target: ");
  Serial.print(targetTemp);
  Serial.print("°C, Current: ");
  Serial.print(currentTemp);
  Serial.print("°C, Time Left: ");
  Serial.print(remainingTime / 60);
  Serial.print(":");
  Serial.print(remainingTime % 60);
  Serial.print("s, ");

  // Control logic
  if (currentTemp > targetTemp + 0.2) 
  {
    myPID.SetControllerDirection(REVERSE);
    analogWrite(RPWM, (int)outputPWM);
    analogWrite(LPWM, 0);
    Serial.print("COOL, PWM: ");
    Serial.println(outputPWM);
  } 
  else if (currentTemp < targetTemp - 0.5) 
  {
    myPID.SetControllerDirection(DIRECT);
    analogWrite(RPWM, 0);
    analogWrite(LPWM, (int)outputPWM);
    Serial.print("HEAT, PWM: ");
    Serial.println(outputPWM);
  } 
  else 
  {
    stopPeltier();
    Serial.println("STOP");
  }

  // Step advancement logic
  if (elapsedSec >= profile[currentStep].durationSec) {
    currentStep++;
    if (currentStep >= profileCount) {
      stopPeltier();
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 8);
      display.print("Process Done!");
      display.display();

      Serial.println("All Steps Completed");
      Serial.println("Activating Buzzer");

      // Beep buzzer
      for (int i = 0; i < 5; i++) {
        digitalWrite(BUZZER, HIGH);
        delay(500);
        digitalWrite(BUZZER, LOW);
        delay(500);
      }
      while (currentStep >= profileCount) {
        if(WiFi.status() == WL_CONNECTED) server.handleClient(); // Keep serving requests so a Restart is possible
        delay(10);
      }
    } else {
      stepStartMillis = millis(); // Reset timer for the new step
      Serial.println("\nMoving to next step");
      display.clearDisplay();
      display.setCursor(0, 8);
      display.print("Next Step...");
      display.display();
      delay(1000); // Pause to show step change
    }
  }
}
