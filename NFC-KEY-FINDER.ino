#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

#define RST_PIN D0
#define SS_PIN D2

const char* ssid = "ASRock";
const char* password = "K@rhas1069";

ESP8266WebServer server(80);
MFRC522 rfid(SS_PIN, RST_PIN);

bool cardRead = false;
String cardDataJson = "";
String currentKey = "";
MFRC522::MIFARE_Key key;

unsigned long previousMillis = 0;
const long interval = 1000; // Reduced interval for checking new cards

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize SPI bus and MFRC522 module
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID reader initialized.");

  // Check the firmware version of the MFRC522
  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println("ERROR: MFRC522 not detected. Check wiring!");
    while (true);
  }
  Serial.print("MFRC522 Firmware Version: 0x");
  Serial.println(version, HEX);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/cardData", handleCardData);
  server.on("/checkCard", handleCheckCard);
  server.on("/saveJson", handleSaveJson);
  server.on("/testKey", handleTestKey);  // Route to test the key
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Check for new RFID card
    if (!cardRead) {
      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        Serial.println("Card detected. Reading card...");
        cardDataJson = buildCardDataJson();
        cardRead = true;
        Serial.println("Card data stored. Searching for key...");
        bool keyFound = false;
        while (!keyFound) {
          generateKey();
          Serial.println("Trying key: " + currentKey);
          if (tryKey(currentKey)) {
            Serial.println("Key found: " + currentKey);
            keyFound = true;
          }
        }
        rfid.PICC_HaltA(); // Halt the card
        rfid.PCD_StopCrypto1(); // Stop encryption on PCD
      }
    }
  }
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>NFC Reader</title>";
  html += "<style>body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f9; color: #333; }";
  html += "h1 { background-color: #007bff; color: white; margin: 0; padding: 20px; text-align: center; }";
  html += "nav { display: flex; justify-content: center; background-color: #007bff; }";
  html += "nav a { padding: 14px 20px; display: block; color: white; text-align: center; text-decoration: none; }";
  html += "nav a:hover { background-color: #0056b3; }";
  html += "p { text-align: center; font-size: 16px; margin: 20px; }";
  html += "table { width: 90%; margin: 20px auto; border-collapse: collapse; background-color: white; }";
  html += "th, td { padding: 10px; text-align: left; border: 1px solid #ddd; }";
  html += "th { background-color: #007bff; color: white; }";
  html += "tr:nth-child(even) { background-color: #f9f9f9; }";
  html += "tr:hover { background-color: #f1f1f1; }";
  html += "ul { list-style-type: none; padding: 0; margin: 0; }";
  html += "button { background-color: #007bff; color: white; border: none; padding: 10px 20px; text-align: center;";
  html += "text-decoration: none; display: inline-block; font-size: 16px; margin: 10px 2px; cursor: pointer; border-radius: 5px; }";
  html += "footer { text-align: center; padding: 10px; margin-top: 20px; font-size: 14px; color: #666; }</style>";
  html += "<script>function checkForNewCard() {";
  html += "fetch('/checkCard').then(response => response.json()).then(data => {";
  html += "if (data.newCard) { fetchCardData(); } }); }";
  html += "function fetchCardData() {";
  html += "fetch('/cardData').then(response => response.text()).then(data => {";
  html += "document.getElementById('cardData').innerHTML = data; }); }";
  html += "function saveJson() {";
  html += "fetch('/saveJson').then(response => response.json()).then(data => {";
  html += "if (data.success) { const blob = new Blob([JSON.stringify(data.json)], { type: 'application/json' });";
  html += "const url = URL.createObjectURL(blob); const a = document.createElement('a'); a.style.display = 'none';";
  html += "a.href = url; a.download = 'cardData.json'; document.body.appendChild(a); a.click(); window.URL.revokeObjectURL(url); }";
  html += "else { alert('Failed to save data.'); } }); }";
  html += "function testKey() {";
  html += "fetch('/testKey').then(response => response.json()).then(data => {";
  html += "if (data.success) { alert('Key is correct!'); }";
  html += "else { alert('Key is incorrect.'); } }); }";
  html += "setInterval(checkForNewCard, 2000);</script>";
  html += "</head><body><h1>NFC Card Data</h1>";
  html += "<nav><a href='/' onclick='fetchCardData()'>Home</a></nav>";
  html += "<div id='cardData'><p>No card data available yet. Please place card.</p></div>";
  html += "<button onclick='saveJson()'>Save to JSON</button>";
  html += "<button onclick='testKey()'>Test Key</button>";
  html += "<div id='generatedKey' style='text-align: center; margin-top: 20px;'></div>";
  html += "<footer>© 2025 NFC Reader. Powered by Pantelis Kapoulas.</footer>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleCardData() {
  String html = "<style>body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f9; color: #333; }";
  html += "p { text-align: center; font-size: 16px; margin: 20px; }";
  html += "table { width: 90%; margin: 20px auto; border-collapse: collapse; background-color: white; }";
  html += "th, td { padding: 10px; text-align: left; border: 1px solid #ddd; }";
  html += "th { background-color: #007bff; color: white; }";
  html += "tr:nth-child(even) { background-color: #f9f9f9; }";
  html += "tr:hover { background-color: #f1f1f1; }</style>";
  html += "<h2 style='text-align: center;'>Card Data:</h2>";
  html += "<pre>" + cardDataJson + "</pre>";
  server.send(200, "text/html", html);
}

void handleCheckCard() {
  String json = (cardRead) ? "{\"newCard\":true}" : "{\"newCard\":false}";
  server.send(200, "application/json", json);
  if (cardRead) {
    cardRead = false; // Reset cardRead to allow reading of new card
  }
}

void handleSaveJson() {
  String json = (cardDataJson != "") ? "{\"success\":true, \"json\":" + cardDataJson + "}" : "{\"success\":false}";
  server.send(200, "application/json", json);
}

void handleTestKey() {
  bool success = testKey();
  String json = success ? "{\"success\":true}" : "{\"success\":false}";
  server.send(200, "application/json", json);
}

void generateKey() {
  // Example key generator logic
  // Here we set the key to a random value for simplicity
  currentKey = "";
  for (byte i = 0; i < 6; i++) {
    byte randomValue = random(0, 256);
    if (randomValue < 16) currentKey += "0";
    currentKey += String(randomValue, HEX);
    if (i < 5) currentKey += ":";
  }
}

bool tryKey(String keyString) {
  // Convert the keyString to MFRC522::MIFARE_Key format
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = strtoul(keyString.substring(i * 3, (i * 3) + 2).c_str(), NULL, 16);
  }

  for (byte sector = 0; sector < 16; sector++) {
    MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, sector * 4, &key, &(rfid.uid));
    if (status != MFRC522::STATUS_OK) {
      return false;
    }
  }

  return true;
}

bool testKey() {
  return tryKey(currentKey);
}

String buildCardDataJson() {
  StaticJsonDocument<2000> doc;  // Create a JSON document

  JsonArray uidArray = doc.createNestedArray("Card_UID");
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidArray.add(rfid.uid.uidByte[i]);
  }

  doc["Card_Type"] = rfid.PICC_GetTypeName(rfid.PICC_GetType(rfid.uid.sak));

  JsonArray sectors = doc.createNestedArray("sectors");
  for (byte sector = 0; sector < 16; sector++) {
    MFRC522::StatusCode status;
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF; // Default key
    }
    status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, sector * 4, &key, &(rfid.uid));
    if (status == MFRC522::STATUS_OK) {
      for (byte block = 0; block < 4; block++) {
        byte buffer[18];
        byte size = sizeof(buffer);
        status = rfid.MIFARE_Read(sector * 4 + block, buffer, &size);
        if (status == MFRC522::STATUS_OK) {
          JsonObject blockData = sectors.createNestedObject();
          blockData["sector"] = sector;
          blockData["block"] = block;
          JsonArray blockArray = blockData.createNestedArray("data");
          for (byte i = 0; i < 16; i++) {
            blockArray.add(buffer[i]);
          }
        }
      }
    }
  }

  String json;
  serializeJson(doc, json);  // Serialize JSON document to string
  return json;
}
