#include <WiFi.h>
#include <FirebaseESP32.h>
#include <WebServer.h>

// WiFi credentials
#define WIFI_SSID "TECH_B"
#define WIFI_PASSWORD "void0708"

// Firebase credentials
#define API_KEY "AIzaSyCIZfK50sbPROKnBbXCs4QP3WWGnDlPMtU"
#define PROJECT_ID "sluglamp-f70d3"
#define DATABASE_URL "https://sluglamp-f70d3-default-rtdb.firebaseio.com/"


// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// WebServer
WebServer server(80);

// HTML UI
String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Slug Lamp Control</title>
  <style>
    body { font-family: Arial; text-align:center; background:#111; color:white; }
    .btn { padding:20px; font-size:24px; margin:20px; border-radius:12px; cursor:pointer; }
    .on { background:lime; color:black; }
    .off { background:red; color:white; }
    .status { font-size:18px; margin:20px; }
  </style>
</head>
<body>
  <h1>Slug Technological Lamp</h1>
  <div class="status" id="status">Status: Loading...</div>
  <button class="btn on" onclick="turnOn()">LED ON</button>
  <button class="btn off" onclick="turnOff()">LED OFF</button>
  
  <script>
    function turnOn() {
      fetch('/on').then(response => response.text()).then(data => {
        document.getElementById('status').innerHTML = 'Status: ON';
        console.log(data);
      });
    }
    
    function turnOff() {
      fetch('/off').then(response => response.text()).then(data => {
        document.getElementById('status').innerHTML = 'Status: OFF';
        console.log(data);
      });
    }
    
    // Update status every 2 seconds
    setInterval(() => {
      fetch('/status').then(response => response.text()).then(data => {
        document.getElementById('status').innerHTML = 'Status: ' + data;
      });
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

// Web server handlers
void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleOn() {
  Serial.println("handleOn() called - updating Firebase...");
  
  FirebaseJson json;
  json.set("status", true);
  json.set("timestamp", String(millis()));
  
  Serial.printf("Writing to Firebase path: /lamp/ledstrip\n");
  Serial.printf("JSON data: {\"status\": true, \"timestamp\": %s}\n", String(millis()).c_str());
  
  if (Firebase.setJSON(fbdo, "/lamp/ledstrip", json)) {
    Serial.println("LED turned ON via web - Firebase updated successfully!");
    server.send(200, "text/plain", "ON");
  } else {
    Serial.print("Failed to turn ON: ");
    Serial.println(fbdo.errorReason());
    Serial.printf("Firebase error code: %d\n", fbdo.errorCode()); and 
    server.send(500, "text/plain", "Error");
  }
}

void handleOff() {
  Serial.println("handleOff() called - updating Firebase...");
  
  FirebaseJson json;
  json.set("status", false);
  json.set("timestamp", String(millis()));
  
  Serial.printf("Writing to Firebase path: /lamp/ledstrip\n");
  Serial.printf("JSON data: {\"status\": false, \"timestamp\": %s}\n", String(millis()).c_str());
  
  if (Firebase.setJSON(fbdo, "/lamp/ledstrip", json)) {
    Serial.println("LED turned OFF via web - Firebase updated successfully!");
    server.send(200, "text/plain", "OFF");
  } else {
    Serial.print("Failed to turn OFF: ");
    Serial.println(fbdo.errorReason());
    Serial.printf("Firebase error code: %d\n", fbdo.errorCode());
    server.send(500, "text/plain", "Error");
  }
}

void handleStatus() {
  if (Firebase.getBool(fbdo, "/lamp/ledstrip/status")) {
    if (fbdo.dataType() == "boolean") {
      bool state = fbdo.boolData();
      server.send(200, "text/plain", state ? "ON" : "OFF");
    } else {
      server.send(200, "text/plain", "Unknown");
    }
  } else {
    server.send(200, "text/plain", "Error");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Connected!");

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Anonymous sign-in (no email/password needed for open DB rules)
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signUp OK");
  } else {
    Serial.printf("Firebase signUp failed, %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  // Test write to create initial data
  Serial.println("Testing Firebase write...");
  FirebaseJson json;
  json.set("status", false);
  json.set("timestamp", String(millis()));
  
  if (Firebase.setJSON(fbdo, "/lamp/ledstrip", json)) {
    Serial.println("Initial data written successfully!");
  } else {
    Serial.print("Failed to write initial data: ");
    Serial.println(fbdo.errorReason());
  }
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("Web server started!");
  Serial.print("Open your browser and go to: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Update status every 5 seconds (less frequent now)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    lastUpdate = millis();
    
    // Try to read the current state
    if (Firebase.getBool(fbdo, "/lamp/ledstrip/status")) {
      if (fbdo.dataType() == "boolean") {
        bool state = fbdo.boolData();
        Serial.printf("Ledstrip status: %s\n", state ? "ON" : "OFF");
      }
    } else {
      String error = fbdo.errorReason();
      Serial.println(error);
      
      // If path doesn't exist, create initial data
      if (error == "path not exist") {
        Serial.println("Creating initial data structure...");
        
        FirebaseJson json;
        json.set("status", false);
        json.set("timestamp", String(millis()));
        
        if (Firebase.setJSON(fbdo, "/lamp/ledstrip", json)) {
          Serial.println("Initial data created successfully!");
        } else {
          Serial.print("Failed to create data: ");
          Serial.println(fbdo.errorReason());
        }
      }
    }
  }
}
