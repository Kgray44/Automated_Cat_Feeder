#include <WiFi.h>
#include <WiFiAP.h>
#include "esp_wifi.h"
#include <DNSServer.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <time.h>
#include <Preferences.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>

#include "html.h"
#include "bitmap_icons.h"

//motor driver pins
#define PGOOD     17  //IO17
#define PRST      18  //IO18
#define sleep_pin 38  //IO38
#define en_pin    21  //IO21
#define ph_pin    47  //IO47
#define fault_pin 13  //IO13
#define ipropi    14  //IO14

#define SCREEN_WIDTH    128   // OLED display width, in pixels
#define SCREEN_HEIGHT   64    // OLED display height, in pixels
#define OLED_RESET      -1    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS  0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SDA 1
#define SCL 2

#define jamcurrent 350 //current in mA for jam
#define jamFixAttemptLimit 10 //number of attempts to fix the jam

bool jammed = false;
int jamFixAttempt = 0;
float smoothedCurrent_mA = 0;
float smoothingFactor = 0.1;  // smaller = smoother

// Wi-Fi Configuration
const char* ssid = "";//don't fill
const char* password = "";//don't fill
const char* ap_ssid = "catfeeder_AP";
const char* ap_password = "makers4ever";

// Login credentials
const char* login_username = "cats";
const char* login_password = "maker";

bool automated = false;
bool cat_recognition = false;
bool motorOn = false;
bool feeder_on = false;
unsigned long startmillis = 0;
unsigned long lastms2 = 0;
unsigned long scheduleMillis = 0;
unsigned long setupmillis = 0;
unsigned long internetmillis = 0;
unsigned long lastSend = 0;
int motorTime = 5;

bool tog=false;
bool printed=false;

//WiFi AP
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1); // SoftAP IP
DNSServer dnsServer;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Async Web Server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool isAuthenticated(AsyncWebServerRequest *request) {
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie");
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      return true;
    }
  }
  return false;
}

void notifyClients() {
  String json = "{\"automated\":";
  json += automated ? "true" : "false";
  json += ",\"cat_recognition\":";
  json += cat_recognition ? "true" : "false";
  json += ",\"motorOn\":";
  json += motorOn ? "true" : "false";
  json += ",\"current\":";
  json += smoothedCurrent_mA;
  json += "}";
  ws.textAll(json);
  Serial.println(json);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char *)data, "toggleAutomated") == 0) {
      Serial.println("toggleAutomated");
    } else if (strcmp((char *)data, "toggleCat_recognition") == 0) {
      Serial.println("toggleCat_recognition");
    } else if (strcmp((char *)data, "getStatus") == 0) {
      // If the client requests the status, send it
      notifyClients();
      Serial.println("Sent status update to client");
    }
    notifyClients();
    Serial.println("handled websocket message...");
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.println("WebSocket client connected");
      notifyClients();
      break;
    case WS_EVT_DISCONNECT:
      Serial.println("WebSocket client disconnected");
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

String makeScheduleRows() {
  String days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  String html;
  for (int d = 0; d < 7; d++) {
    html += "<tr><td>" + days[d] + "</td><td class='time-slot'>";
    for (int i = 0; i < 4; i++) {
      html += "<input type='time' id='" + days[d] + String(i) + "' name='" + days[d] + String(i) + "'>";
    }
    html += "</td></tr>";
  }

  return html;
}

String schedule_processor(const String& var) {
  if (var == "SCHEDULE_ROWS") {
    String rows = makeScheduleRows();
    return String(rows);
  }
  return String(); // Return empty string for unknown placeholders
}

/*String motortime_processor(const String& var) {
  if (var == "MOTORTIME"){
    return String(motorTime);
  }
  return String();
}*/

bool checkInternet() {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  http.setConnectTimeout(3000);  // 3 sec timeout
  http.begin("http://clients3.google.com/generate_204");  // Lightweight, returns 204
  int code = http.GET();
  http.end();

  return (code == 204);
}

std::map<String, std::vector<String>> feedingSchedule;
Preferences prefs;

void setup() {
  Serial.begin(115200);
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("\nReset reason: %d → ", reason);
  switch (reason) {
    case ESP_RST_POWERON:      Serial.println("POWERON_RESET"); break;
    case ESP_RST_EXT:          Serial.println("EXT_RESET"); break;
    case ESP_RST_SW:           Serial.println("SW_RESET"); break;
    case ESP_RST_PANIC:        Serial.println("PANIC_RESET"); break;
    case ESP_RST_INT_WDT:      Serial.println("INT_WDT_RESET"); break;
    case ESP_RST_TASK_WDT:     Serial.println("TASK_WDT_RESET"); break;
    case ESP_RST_WDT:          Serial.println("OTHER_WDT_RESET"); break;
    case ESP_RST_DEEPSLEEP:    Serial.println("DEEPSLEEP_RESET"); break;
    case ESP_RST_BROWNOUT:     Serial.println("BROWNOUT_RESET"); break;
    case ESP_RST_SDIO:         Serial.println("SDIO_RESET"); break;
    case ESP_RST_USB:          Serial.println("USB_RESET"); break;
    default:                   Serial.println("UNKNOWN"); break;
  } 

  Wire.begin(SDA, SCL);
  delay(100);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  //display.setFont(&FreeSerif9pt7b);
  display.clearDisplay();

  pinMode(sleep_pin, OUTPUT);
  pinMode(en_pin, OUTPUT);
  pinMode(ph_pin, OUTPUT);
  pinMode(fault_pin, INPUT);
  pinMode(ipropi, INPUT);
  pinMode(PGOOD, INPUT);
  pinMode(PRST, INPUT);

  digitalWrite(sleep_pin, LOW);//turn sleep on
  //digitalWrite(en_pin, HIGH);//enable
  Serial.println("Pins setup complete!\nStarting WiFi...");

  //load saved data from flash memory
  loadSavedData();

  if (strlen(ssid) > 0){
    // Initialize Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    setupmillis = millis();
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (tog){printCentered("WiFi STA .");} else {printCentered("WiFi STA  ");}
      tog=!tog;
      if (millis() - setupmillis > 20000){//20 seconds not connecting
        prefs.begin("wifi_network", false);
        prefs.putString("ssid", "");
        prefs.putString("pass", "");
        prefs.end();

        Serial.println("Rebooting now due to failed connection...");
        printCentered("Failed connection!");
        delay(2000);
        ESP.restart();
      }
    }
    Serial.println("\nConnected to WiFi");
    Serial.println("IP Address:");
    Serial.println(WiFi.localIP());
    printCentered("WiFi Connected\n'catfeeder.local'");
    delay(500);
  
  // Set time zone and NTP server
  //configTzTime("EST5EDT,M3.2.0/2,M11.1.0/2", "pool.ntp.org", "time.nist.gov");
  configTime(-4 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for time sync");

  setupmillis = millis();
  time_t now = time(nullptr);

  while (now < 8 * 3600 * 2 && (millis() - setupmillis < 20000)) {
    if (tog){printCentered("WiFi Connected\n'catfeeder.local'\nTime sync .");} else {printCentered("WiFi Connected\n'catfeeder.local'\nTime sync  ");}
    tog=!tog;
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  if (now < 8 * 3600 * 2) {
    Serial.println("\n Time sync failed after 20 seconds.");
    printCentered("Time sync failed!");
    delay(2000);
  } else {
    Serial.println("\n Time synchronized.");
    printCentered("WiFi Connected\n'catfeeder.local'\nTime sync success!");
    struct tm* t = localtime(&now);
    char currentTime[6];
    sprintf(currentTime, "%02d:%02d", t->tm_hour, t->tm_min);
    Serial.println(currentTime);
  }

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      request->redirect("/index");
      notifyClients();
    } else {
      request->send_P(200, "text/html", login_html);
    }
  });

  // Route to Handle Login
  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("username", true) && request->hasParam("password", true)) {
      String username = request->getParam("username", true)->value();
      String password = request->getParam("password", true)->value();
      if (username == login_username && password == login_password) {
        AsyncWebServerResponse *response = request->beginResponse(301);
        response->addHeader("Set-Cookie", "ESPSESSIONID=1; HttpOnly");
        response->addHeader("Location", "/index");
        request->send(response);
      } else {
        request->send(401, "text/plain", "Unauthorized");
      }
    }
  });

  // Route for Control Page
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      request->send_P(200, "text/html", index_html);
      notifyClients();
    } else {
      request->redirect("/");
    }
  });

  //route for password change
  server.on("/change", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      request->send_P(200, "text/html", change_html);
    } else {
      request->redirect("/");
    }
  });

  server.on("/save-change", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("password", true) && request->hasParam("passwordd", true)) {
      String password = request->getParam("password", true)->value();
      String passwordd = request->getParam("passwordd", true)->value();

      if (password == passwordd) {
        // Save the new password logic here
        login_password = password.c_str();
        prefs.begin("auth", false);
        prefs.putString("password", login_password);
        prefs.end();
        request->send(200, "text/plain", "Password successfully changed.");
      } else {
        request->send(400, "text/plain", "Passwords do not match.");
      }
    } else {
      request->send(400, "text/plain", "Missing parameters.");
    }
  });

  //route for settings
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      request->send(200, "text/html", settings_html);
    } else {
      request->redirect("/");
    }
  });

  server.on("/save-settings", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
      request->send(401, "text/plain", "Unauthorized");
      return;
    }

    if (request->hasParam("motorTime", true)) {
      String motorTimeR = request->getParam("motorTime", true)->value();
      motorTime = motorTimeR.toInt();
      Serial.println("Motor Time Length updated to: " + String(motorTime));

      prefs.begin("settings", false); // namespace = "settings", readWrite = false
      prefs.putInt("motorTime", motorTime);
      prefs.end();

      request->send(200, "text/plain", "Settings saved successfully!");
      //request->redirect("/index");
    } else {
      request->send(400, "text/plain", "Motor Time Length not provided");
    }
  });

  server.on("/api/motortime", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!isAuthenticated(request)) {
      request->send(401, "application/json", "{\"error\":\"unauthorized\"}");
      return;
    }
    request->send(200, "application/json", "{\"motorTime\":" + String(motorTime) + "}");
  });

  /*server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!isAuthenticated(request)) {
      request->redirect("/");
      return;
    }
    String html = String(schedule_html);
    html.replace("%SCHEDULE_ROWS%", makeScheduleRows());
    request->send(200, "text/html", html);
  });*/
  server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!isAuthenticated(request)) {
      request->redirect("/");
      return;
    }
    request->send(200, "text/html", schedule_html, schedule_processor);
  });


  server.on("/save-schedule", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!isAuthenticated(request)) {
      request->send(401, "text/plain", "Unauthorized");
      return;
    }
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
      request->send(400, "text/plain", "Bad JSON");
      return;
    }

    feedingSchedule.clear();

    for (JsonPair kv : doc.as<JsonObject>()) {
      String day = kv.key().c_str();
      JsonArray times = kv.value().as<JsonArray>();
      for (JsonVariant v : times) {
        feedingSchedule[day].push_back(v.as<String>());
      }
    }
    // 🔒 Save entire schedule JSON to NVS (flash)
    prefs.begin("schedule", false);
    String jsonString;
    serializeJson(doc, jsonString);
    prefs.putString("feedmap", jsonString);
    prefs.end();

    request->send(200, "text/plain", "Schedule saved");
  });
  
  server.on("/api/schedule", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!isAuthenticated(request)) {
      request->send(401, "application/json", "{\"error\":\"unauthorized\"}");
      return;
    }

    StaticJsonDocument<2048> doc;

    for (auto const& kv : feedingSchedule) {
      JsonArray arr = doc.createNestedArray(kv.first);
      for (auto const& time : kv.second) {
        arr.add(time);
      }
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.on("/reset-schedule", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!isAuthenticated(request)) {
      request->send(401, "text/plain", "Unauthorized");
      return;
    }

    feedingSchedule.clear();  // Clear the stored schedule
    Serial.println("Schedule has been reset.");
    request->send(200, "text/plain", "Schedule reset");
  });

  server.on("/manage", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      request->send_P(200, "text/html", manage_html);
    } else {
      request->redirect("/");
    }
  });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      prefs.begin("settings", false); prefs.clear(); prefs.end();
      prefs.begin("schedule", false); prefs.clear(); prefs.end();
      prefs.begin("auth", false); prefs.clear(); prefs.end();
      request->send(200, "text/plain", "Memory erased!");
    } else {
      request->send(401);
    }
  });

  server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      request->send(200, "text/plain", "Restarting...");
      delay(100);  // Ensure response is sent
      ESP.restart();
    } else {
      request->send(401);
    }
  });

  // Route to serve the OTA update page
  server.on("/ota", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      request->send_P(200, "text/html", update_html);
    } else {
      request->send(401);
    }
  });

  // Handle Toggle for "Everyone"
  server.on("/automated", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      if (request->hasParam("state")) {
        automated = request->getParam("state")->value() == "1";
        Serial.print("Automated mode: ");
        Serial.println(automated ? "ON" : "OFF");
        prefs.begin("settings", false);
        prefs.putInt("automated", automated);
        prefs.end();
        notifyClients();
      }
      request->send(200, "text/plain", "OK");
    } else {
      request->send(401);
    }
  });

  // Handle Toggle for "Roommates"
  server.on("/cat_recognition", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      if (request->hasParam("state")) {
        cat_recognition = request->getParam("state")->value() == "1";
        Serial.print("Cat_Recognition mode: ");
        Serial.println(cat_recognition ? "ON" : "OFF");
        prefs.begin("settings", false);
        prefs.putInt("cat_recognition", cat_recognition);
        prefs.end();
        notifyClients();
      }
      request->send(200, "text/plain", "OK");
    } else {
      request->send(401);
    }
  });

  // Handle door unlock request
  server.on("/feed", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
      feed();
      request->send(200, "text/plain", "Feeding Initiated");
    } else {
      request->send(401);
    }
  });

  // Handle Firmware Update via OTA
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
      response->addHeader("Connection", "close");
      response->addHeader("Location", "/index");
      request->send(response);
      ESP.restart();
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (isAuthenticated(request)) {
      if (!index) {
        Serial.printf("Update Start: %s\n", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      }
      if (!Update.hasError()) {
        if (Update.write(data, len) != len) {
          Update.printError(Serial);
        }
      }
      if (final) {
        if (Update.end(true)) {
          Serial.printf("Update Success: %u bytes\n", index + len);
        } else {
          Update.printError(Serial);
        }
      } 
    }
    else {
      request->send(401);
    }
  });
  
  }
  else {
    if (!WiFi.softAP(ap_ssid, ap_password)) {
      Serial.println("AP failed to start");
      printCentered("AP failed!");
      delay(2000);
      ESP.restart();
    } 
    printCentered("AP started.");
    delay(200);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    dnsServer.start(DNS_PORT, "*", myIP);

    server.on("/configure", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", configForm);
    });

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
      if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
        String savessid = request->getParam("ssid", true)->value();
        String savepass = request->getParam("password", true)->value();

        if (savessid.length() > 0) {
          Serial.println("Received credentials:");
          Serial.println("SSID: " + savessid);
          Serial.println("PASS: " + savepass);

          prefs.begin("wifi_network", false);
          prefs.putString("ssid", savessid);
          prefs.putString("pass", savepass);
          prefs.end();

          request->send(200, "text/html", "<h2>Saved. Rebooting...</h2>");
          Serial.println("Rebooting now due to save handler...");
          delay(2000);
          ESP.restart();
        } else {
          request->send(400, "text/html", "<h2>SSID cannot be empty.</h2>");
          Serial.println("Empty SSID submitted — ignoring");
        }
      } else {
        request->send(400, "text/html", "<h2>Missing parameters.</h2>");
        Serial.println("Missing SSID or password fields");
      }
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
      request->redirect("/configure");
    });
  }
  // Start the server
  server.begin();
  delay(200);
  
  if (WiFi.getMode() == WIFI_AP) {
    esp_wifi_set_max_tx_power(40);  // 10 dBm (units are 0.25 dBm steps)
    Serial.println("WiFi TX power set to 10 dBm");
  }
  // Only run mDNS in STA mode
  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
    if (!MDNS.begin("catfeeder")) {
      Serial.println("Error setting up mDNS!");
    } else {
      Serial.println("mDNS responder started");
      MDNS.addService("http", "tcp", 80);
    }
  }
}

void loop() {
  if (WiFi.getMode() != WIFI_STA){
    printCentered("Waiting for cred");
    dnsServer.processNextRequest();
    return;
  }

  if (jammed){
    ESP.restart();
  }

  if (millis() - internetmillis > 10000){//check for internet every ten seconds
    if (!checkInternet()) {//if no internet, restart
      prefs.begin("wifi_network", false);
      prefs.putString("ssid", "");
      prefs.putString("pass", "");
      prefs.end();
      Serial.println("No internet, restarting and wiping SSID&PASS...");
      printCentered("Internet failed");
      delay(1000);
      ESP.restart();
    }
    internetmillis = millis();
  }

  ws.cleanupClients();
  delay(50);
  yield();
  if (startmillis != 0 && millis() - startmillis > motorTime*1000) {
    digitalWrite(sleep_pin, LOW);//turn sleep on
    digitalWrite(en_pin, LOW);//disable
    motorOn = false;
    Serial.println("Feeder motor off");
    notifyClients();
    printed=false;
    startmillis = 0;
  }

  if (automated){
    checkSchedule();
  }

  if (millis() - lastSend >= 100) {
    if (motorOn){
      if (!printed){printCentered("Feeding ");printed=true;}
      if (tog){display.drawBitmap(60, 24, motor1Char, 5, 8, WHITE);display.display();} else {display.drawBitmap(60, 24, motor1Char, 5, 8, WHITE);display.display();}
      tog=!tog;
      float adcVoltage = analogRead(ipropi) * (3.3 / 4095.0); // volts
      float motorCurrent = adcVoltage / (0.000455 * 2700.0);      // amps
      //motorCurrent = motorCurrent*1000; //ma
      smoothedCurrent_mA = smoothedCurrent_mA * (1.0 - smoothingFactor) + (motorCurrent * 1000.0) * smoothingFactor;

      if (smoothedCurrent_mA >= jamcurrent){
        digitalWrite(ph_pin, LOW);//direction backwards
        delay(1000);
        digitalWrite(ph_pin, HIGH);//direction backwards
        delay(500);
        jamFixAttempt++;
        if (jamFixAttempt >= jamFixAttemptLimit){
          jammed = true;
          return;
        }
      }

      notifyClients();  // only stream current if motor is on
      lastSend = millis();
    }
    else { printed=false;display.clearDisplay();jamFixAttempt=0;lastSend = millis();}
  }
  
  /*if (digitalRead(fault_pin) == LOW){
    Serial.println("FAULT");
  }*/
  
  /*if (millis() - lastms2 > 100){
    float adcVoltage = analogRead(ipropi) * (3.3 / 4095.0); // volts
    float motorCurrent = adcVoltage / (0.000455 * 2700.0);      // amps
    Serial.print("Current pin: ");
    Serial.println(motorCurrent*1000,2);
    lastms2 = millis();
  }*/
}

void printCentered(const String& fullText) {
  display.clearDisplay();
  display.setTextWrap(false); // Prevent automatic wrapping

  display.setTextSize(1); // REQUIRED: set font size (1–3 typically)
  display.setTextColor(SSD1306_WHITE);
  display.setFont();      // Use built-in font (optional, but good reset)

  // Step 1: Count lines
  int numLines = 1;
  for (unsigned int i = 0; i < fullText.length(); i++) {
    if (fullText[i] == '\n') numLines++;
  }

  // Step 2: Get height of one line
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds("A", 0, 0, &x1, &y1, &w, &h);
  int lineHeight = h + 2;
  int totalHeight = numLines * lineHeight;

  // Step 3: Y offset
  int yStart = (SCREEN_HEIGHT - totalHeight) / 2;

  // Step 4: Draw each line centered
  String currentLine = "";
  int lineNum = 0;

  for (unsigned int i = 0; i <= fullText.length(); i++) {
    char c = fullText[i];
    if (c == '\n' || i == fullText.length()) {
      display.getTextBounds(currentLine, 0, 0, &x1, &y1, &w, &h);
      int xStart = (SCREEN_WIDTH - w) / 2;

      display.setCursor(xStart, yStart + lineNum * lineHeight);
      display.print(currentLine);

      currentLine = "";
      lineNum++;
    } else {
      currentLine += c;
    }
  }

  display.display();
}



void checkSchedule() {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  char currentTime[6];
  sprintf(currentTime, "%02d:%02d", t->tm_hour, t->tm_min);

  String dayStr[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  auto& todayTimes = feedingSchedule[dayStr[t->tm_wday]];

  for (auto& feedTime : todayTimes) {
    if (millis() - scheduleMillis > 60000){//only one trigger per minute
      String fullMessage = "Next feed time is\n" + feedTime;
      printCentered(fullMessage.c_str());
      if (feedTime == String(currentTime)) {
        Serial.println("Scheduled Feed Triggered at " + String(currentTime));
        feed(); // existing feed() function
        scheduleMillis = millis();
        break;
      }
    }
  }
}

void feed() {
  digitalWrite(sleep_pin, HIGH);//turn sleep off
  digitalWrite(en_pin, HIGH);//enable
  digitalWrite(ph_pin, HIGH);//direction
  motorOn = true;
  startmillis = millis();
  Serial.println("Feeding initiated");
  notifyClients();
}

void loadSavedData(){
  prefs.begin("settings", true); // readOnly = true
  motorTime = prefs.getInt("motorTime", 5); // default to 5 seconds if not set
  automated = prefs.getInt("automated", 0);
  cat_recognition = prefs.getInt("cat_recognition", 0);
  prefs.end();

  prefs.begin("schedule", true);
  String saved = prefs.getString("feedmap", "");
  prefs.end();
  if (saved.length() > 0) {
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, saved);
    if (!error) {
      for (JsonPair kv : doc.as<JsonObject>()) {
        String day = kv.key().c_str();
        JsonArray times = kv.value().as<JsonArray>();
        for (JsonVariant v : times) {
          feedingSchedule[day].push_back(v.as<String>());
        }
      }
      Serial.println("Feeding schedule loaded from flash.");
    } else {
      Serial.println("Failed to parse saved schedule JSON.");
    }
  }

  prefs.begin("auth", true);
  String savedPassword = prefs.getString("password", "");
  prefs.end();
  if (savedPassword.length() > 0) {
    login_password = savedPassword.c_str();
    Serial.println("Login password restored from flash.");
  }
  
  prefs.begin("wifi_network", true);
  String ssid_str = prefs.getString("ssid", "");
  String pass_str = prefs.getString("pass", "");
  ssid = strdup(ssid_str.c_str());       // safely store heap copy
  password = strdup(pass_str.c_str());   // safely store heap copy
  Serial.print("Loaded SSID: "); Serial.print(ssid); Serial.print(" with password: "); Serial.println(password);
  prefs.end();
}
