# ESP8266 RFID Reader with Web Interface

This project uses an ESP8266 microcontroller to read RFID cards and display the card data via a web interface. The code initializes an MFRC522 RFID reader and a web server on the ESP8266, allowing users to read RFID cards, view the card data, and test keys through a web interface.

## Components Used

- ESP8266 (e.g., NodeMCU)
- MFRC522 RFID Reader
- RFID Cards/Tags
- Breadboard and jumper wires

## Libraries Required

- `ESP8266WiFi.h`
- `ESP8266WebServer.h`
- `SPI.h`
- `MFRC522.h`
- `ArduinoJson.h`

## Wiring

Connect the MFRC522 RFID reader to the ESP8266 as follows:

| MFRC522 Pin | ESP8266 Pin |
| ----------- | ----------- |
| RST         | D0          |
| SDA (SS)    | D2          |
| MOSI        | D7          |
| MISO        | D6          |
| SCK         | D5          |
| GND         | GND         |
| 3.3V        | 3.3V        |

## Configuration

Update the WiFi credentials in the code:

```cpp
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
Code Explanation
The code consists of the following main parts:

Setup Function:

Initializes the serial communication and the RFID reader.
Connects to the specified WiFi network.
Sets up the web server routes.
Loop Function:

Handles incoming client requests.
Periodically checks for new RFID cards and reads the card data.
Web Server Handlers:

handleRoot: Serves the main web page.
handleCardData: Serves the card data in JSON format.
handleCheckCard: Checks if a new card has been read.
handleSaveJson: Saves the card data in JSON format.
handleTestKey: Tests the key generated for the RFID card.
Helper Functions:

generateKey: Generates a random key for testing.
tryKey: Tries the generated key on the RFID card.
testKey: Tests the current key on the RFID card.
buildCardDataJson: Builds a JSON object with the card data.
Web Interface
The web interface allows users to:

View the card data.
Save the card data in JSON format.
Test the generated key.
The main web page periodically checks for new cards and updates the displayed card data.

Usage
Connect the ESP8266 and MFRC522 as described in the wiring section.
Upload the code to the ESP8266 using the Arduino IDE.
Open the serial monitor to view the initialization messages and the IP address assigned to the ESP8266.
Open a web browser and navigate to the IP address of the ESP8266.
Place an RFID card near the reader to view the card data on the web page.
Example Output
Here is an example of the card data displayed on the web page:

JSON
{
  "Card_UID": [123, 234, 56, 78],
  "Card_Type": "MIFARE 1KB",
  "sectors": [
    {
      "sector": 0,
      "block": 0,
      "data": [255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255]
    },
    ...
  ]
}
License
This project is licensed under the MIT License.

Author
Pantelis Kapoulas

Code
Feel free to modify the README file according to your specific needs and preferences.
