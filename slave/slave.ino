#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Redmi Note 13 Pro 5G";        // Replace with your WiFi SSID
const char* password = "hi1234500"; // Replace with your WiFi password

const int UDP_PORT = 4210;               // The port to communicate over UDP
const int SLAVE_ID = 1;                  // Unique ID for this slave ESP32

WiFiUDP udp;
IPAddress masterIP;

unsigned long lastHeartbeatResponse = 0;
const long heartbeatTimeout = 15000;     // Timeout period to consider connection lost

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendStatusUpdate() {
  if (masterIP) {
    String status = "Slave " + String(SLAVE_ID) + " OK";
    udp.beginPacket(masterIP, UDP_PORT);
    udp.print(status);
    udp.endPacket();
  }
}

void checkForMaster() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0; // Null-terminate the string
    }
    
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    Serial.printf("Contents: %s\n", incomingPacket);
    
    // If we receive a "PING" message from the master, update the masterIP and respond with a status update
    if (strcmp(incomingPacket, "PING") == 0) {
      masterIP = udp.remoteIP();
      lastHeartbeatResponse = millis(); // Update the last time a heartbeat was received
      sendStatusUpdate();               // Respond to the master with a status update
    }
  }
}

void checkConnection() {
  // If more than heartbeatTimeout milliseconds have passed since the last heartbeat was received, assume the connection is lost
  if (millis() - lastHeartbeatResponse > heartbeatTimeout) {
    Serial.println("Connection to master lost. Waiting for new heartbeat...");
    masterIP = IPAddress(0, 0, 0, 0); // Reset master IP to 0.0.0.0 to indicate no connection
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  udp.begin(UDP_PORT); // Start listening on the defined UDP port
}

void loop() {
  checkForMaster();  // Continuously check for incoming heartbeat messages from the master
  checkConnection(); // Monitor connection status and detect if the master becomes unreachable

  // Add your sensor reading and processing logic here
  // Example: if a sensor reading changes, you might want to send an update to the master
  // if (sensorDataChanged) {
  //   sendStatusUpdate();
  // }

  delay(1000); // Adjust the delay based on your needs, it controls how often the loop runs
}
