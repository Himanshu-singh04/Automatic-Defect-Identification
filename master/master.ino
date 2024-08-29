#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Redmi Note 13 Pro 5G";
const char* password = "hi1234500";

const int NUM_SLAVES = 5; // Adjust based on the number of slave ESP32s
const int UDP_PORT = 4210;

WiFiUDP udp;
IPAddress slaveIPs[NUM_SLAVES];

unsigned long lastHeartbeat = 0;
const long heartbeatInterval = 10000; // Send heartbeat every 10 seconds
bool slavesDiscovered = false;

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendHeartbeat() {
  if (slavesDiscovered) {
    for (int i = 0; i < NUM_SLAVES; i++) {
      if (slaveIPs[i]) {
        udp.beginPacket(slaveIPs[i], UDP_PORT);
        udp.write((uint8_t*)"PING", strlen("PING")); // Cast to uint8_t* and specify the length
        udp.endPacket();
      }
    }
  }
}

void checkForSlaves() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    Serial.printf("Contents: %s\n", incomingPacket);
    
    // Check if this is a new slave
    bool isNewSlave = true;
    for (int i = 0; i < NUM_SLAVES; i++) {
      if (slaveIPs[i] == udp.remoteIP()) {
        isNewSlave = false;
        break;
      }
    }
    
    // If it's a new slave, add it to the list
    if (isNewSlave) {
      for (int i = 0; i < NUM_SLAVES; i++) {
        if (!slaveIPs[i]) {
          slaveIPs[i] = udp.remoteIP();
          Serial.printf("New slave added: %s\n", udp.remoteIP().toString().c_str());
          slavesDiscovered = true; // Mark that at least one slave has been found
          break;
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  udp.begin(UDP_PORT);
}

void loop() {
  checkForSlaves(); // Continuously check for slaves

  unsigned long currentMillis = millis();
  if (slavesDiscovered && (currentMillis - lastHeartbeat >= heartbeatInterval)) {
    lastHeartbeat = currentMillis;
    sendHeartbeat(); // Only send heartbeat if slaves have been discovered
  }

  // Add any additional master logic here
}
