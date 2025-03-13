#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SPIFFS.h"

#define mySerial Serial2
#define RXD2 16
#define TXD2 17

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id;


const char ssid[] = "GizanTech_network";
const char password[] = "#Analysis23";

// const char ssid[] = "Friday";
// const char password[] = "raad2003042";



// MySQL PHP Server URL
const String serverURL = "http://192.168.0.146/sensor/retrieve.php";

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(100);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  Serial.println("SPIFFS Initialized. Checking for unsynced data...");

  // Try connecting to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 10) { 
    delay(500);
    Serial.print(".");
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.println(WiFi.localIP());
    uploadStoredData();  // Sync unsynced data from SPIFFS to MySQL
  } else {
    Serial.println("\nWiFi NOT Connected. Data will be stored locally.");
  }

  String postData = "emp_name=Raiyan&emp_id=7&finger_id=7&in_time=2025-03-11T08:30:00&out_time=2025-03-11T17:30:00&duration=9";

  if (WiFi.status() == WL_CONNECTED) {
    sendAttendanceData(postData);
  } else {
    storeDataLocally(postData);
  }
}

// // Function to store data in SPIFFS when WiFi is unavailable

void storeDataLocally(String data) {
  Serial.println("Checking if data exists...");

  // Open file for reading
  File file = SPIFFS.open("/data_t.txt", FILE_READ);
  if (!file) {
    Serial.println("File not found, writing new data...");
  } else {
    String existingData = "";
    while (file.available()) {
      existingData += char(file.read());
    }
    file.close();

    // Check if the data already exists
    if (existingData.indexOf(data) != -1) {
      Serial.println("Data already exists, skipping write.");
      return;  // Don't write if it's already stored
    }
  }

  // Open file for appending
  file = SPIFFS.open("/data_t.txt", FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for writing!");
    return;
  }

  // Write data
  if (file.println(data)) {
    Serial.println("Data stored successfully!");
  } else {
    Serial.println("Failed to write data!");
  }

  file.close();
}


void readStoredData() {
  // Open file for reading
  File file = SPIFFS.open("/data_t.txt", FILE_READ);
  if (!file) {
    Serial.println("No data found");
    return;
  }

  // Read and print file contents
  Serial.println("Stored Data:");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.println(line);
  }
  file.close();
}




// Function to upload stored data from SPIFFS to MySQL
void uploadStoredData() {
  if (WiFi.status() == WL_CONNECTED) {
    File file = SPIFFS.open("/data_t.txt", FILE_READ);
    if (!file) {
      Serial.println("No stored data found.");
      return;
    }

    Serial.println("Uploading stored data...");
    while (file.available()) {
      String line = file.readStringUntil('\n');
      sendAttendanceData(line);  // Send each line to MySQL
    }
    file.close();

    // Clear the file after successful upload
    // SPIFFS.remove("/data_t.csv");
    Serial.println("All stored data uploaded and deleted from SPIFFS.");
  }
}

// Function to send data to MySQL via PHP
void sendAttendanceData(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(data);
    Serial.printf("HTTP Response Code: %d\n", httpResponseCode);

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Server Response: " + payload);
    } else {
      Serial.printf("HTTP POST failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected, storing data locally.");
    storeDataLocally(data);
  }
}

void loop() {
  String postData = "emp_name=Raiyan&emp_id=20&finger_id=20&in_time=2025-03-11T08:30:00&out_time=2025-03-11T17:30:00&duration=9";

  if (WiFi.status() == WL_CONNECTED) {
    sendAttendanceData(postData);
  } else {
    storeDataLocally(postData);
  }

  storeDataLocally(postData);
  delay(5000); // Simulating new data every 10 seconds
}