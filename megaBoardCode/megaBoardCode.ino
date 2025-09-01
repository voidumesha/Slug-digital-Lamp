#define RELAY_PIN 7
#define LED_PIN 6

// State tracking variables
bool relayState = false;
bool ledState = false;

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("Mega Board Relay/LED Controller Ready!");
  Serial.println("Commands: ON, OFF, STATUS");
  Serial.println("=== Initial Status ===");
  Serial.println("Relay: OFF");
  Serial.println("LED: OFF");
  Serial.println("=====================");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();  // Convert to uppercase for case-insensitive commands

    if (cmd == "ON") {
      turnOnSequence();
    }
    else if (cmd == "OFF") {
      turnOffSequence();
    }
    else if (cmd == "STATUS") {
      showStatus();
    }
    else if (cmd == "HELP") {
      showHelp();
    }
    else {
      Serial.print("Unknown command: ");
      Serial.println(cmd);
      Serial.println("Type 'HELP' for available commands");
    }
  }
}

void turnOnSequence() {
  if (!relayState) {  // Only execute if relay is not already ON
    Serial.println("=== Starting ON Sequence ===");
    
    // Turn ON relay immediately
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    Serial.println("Relay turned ON (Fan started)");
    
    // Wait 4 seconds
    Serial.println("Waiting 4 seconds...");
    for (int i = 4; i > 0; i--) {
      Serial.print("LED will turn ON in ");
      Serial.print(i);
      Serial.println(" seconds");
      delay(1000);
    }
    
    // Turn ON LED after 4 seconds
    digitalWrite(LED_PIN, HIGH);
    ledState = true;
    Serial.println("LED turned ON (after 4 seconds)");
    Serial.println("=== ON Sequence Complete ===");
    
  } else {
    Serial.println("Relay is already ON!");
  }
}

void turnOffSequence() {
  if (relayState || ledState) {  // Only execute if something is ON
    Serial.println("=== Starting OFF Sequence ===");
    
    // Turn OFF both relay and LED immediately
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    relayState = false;
    ledState = false;
    
    Serial.println("Relay turned OFF (Fan stopped)");
    Serial.println("LED turned OFF");
    Serial.println("=== OFF Sequence Complete ===");
    
  } else {
    Serial.println("Relay and LED are already OFF!");
  }
}

void showStatus() {
  Serial.println("=== Current Status ===");
  Serial.print("Relay: ");
  Serial.println(relayState ? "ON" : "OFF");
  Serial.print("LED: ");
  Serial.println(ledState ? "ON" : "OFF");
  Serial.println("=====================");
}

void showHelp() {
  Serial.println("=== Available Commands ===");
  Serial.println("ON     - Turn ON relay (fan) immediately, LED after 4 seconds");
  Serial.println("OFF    - Turn OFF both relay and LED immediately");
  Serial.println("STATUS - Show current relay and LED states");
  Serial.println("HELP   - Show this help message");
  Serial.println("========================");
}
