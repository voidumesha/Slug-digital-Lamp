#include <WiFi.h>
#include <FirebaseESP32.h>

// WiFi credentials
#define WIFI_SSID "TECH_B"
#define WIFI_PASSWORD "void0708"

// Firebase credentials
#define API_KEY "AIzaSyCIZfK50sbPROKnBbXCs4QP3WWGnDlPMtU"
#define DATABASE_URL "https://sluglamp-f70d3-default-rtdb.firebaseio.com/"

// Relay and LED pins
#define RELAY_PIN 26
#define LED_PIN 27

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// State tracking variables
bool lastState = false;
bool relayState = false;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 Fan/LED Control...");

  // Setup pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Relay OFF initially
  digitalWrite(LED_PIN, LOW);    // LED OFF initially
  
  Serial.println("Pins configured - Relay and LED OFF");

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Connected!");

  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Serial.println("Configuring Firebase...");

  // Use anonymous authentication (no signup needed)
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Anonymous authentication successful!");
  } else {
    Serial.printf("Anonymous auth failed: %s\n", config.signer.signupError.message.c_str());
    Serial.println("Check your Firebase project settings for anonymous auth.");
    return;
  }

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  Serial.println("Firebase initialized successfully!");
  Serial.println("Ready to read from database and control relay/LED...");
  
  // Display initial status
  Serial.println("=== Initial Status ===");
  Serial.println("Relay: OFF");
  Serial.println("LED: OFF");
  Serial.println("=====================");
}

void loop() {
  // Read LED strip state from Firebase
  if (Firebase.getBool(fbdo, "/lamp/ledstrip/status")) {
    if (fbdo.dataType() == "boolean") {
      bool state = fbdo.boolData();
      
      // Only process if state changed
      if (state != lastState) {
        Serial.printf("Ledstrip state changed to: %s\n", state ? "ON" : "OFF");
        lastState = state;
        
        if (state) {
          // Turn ON relay immediately
          digitalWrite(RELAY_PIN, HIGH);
          relayState = true;
          Serial.println("Relay turned ON");
          
          // Schedule LED to turn ON after 4 seconds
          static unsigned long ledTimer = 0;
          ledTimer = millis() + 4000;
          
          // Wait for 4 seconds while keeping Firebase connection alive
          while (millis() < ledTimer) {
            delay(100);
          }
          
          // Turn ON LED after 4 seconds
          digitalWrite(LED_PIN, HIGH);
          ledState = true;
          Serial.println("LED turned ON (after 4 seconds)");
          
        } else {
          // Turn OFF both relay and LED immediately
          digitalWrite(RELAY_PIN, LOW);
          digitalWrite(LED_PIN, LOW);
          relayState = false;
          ledState = false;
          Serial.println("Relay and LED turned OFF");
        }
      }
      
    } else {
      Serial.printf("Unexpected data type: %s\n", fbdo.dataType().c_str());
    }
  } else {
    Serial.printf("Database read error: %s\n", fbdo.errorReason().c_str());
  }

  delay(1000);  // Check every second for faster response
}
