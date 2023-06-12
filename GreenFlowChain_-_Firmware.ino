#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <qrcode.h>

#include <Servo.h>

Servo myservo; 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define RST_PIN         D0           // Configurable, see typical pin layout above
#define SS_PIN          D8          // Configurable, see typical pin layout above

const int qrCodeVersion = 3;
const int pixelSize = 2;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void displayScrollingText() {


  int16_t scrollPos = SCREEN_WIDTH;
  while (scrollPos >= -SCREEN_WIDTH * 12) {
    display.clearDisplay();
    display.setTextSize(8);
    display.setTextColor(WHITE);
    display.setCursor(scrollPos, 0);
    display.println("GreenFlowChain");
    display.display();
    delay(5);
    scrollPos--;
  }
}
void showQRCode(String qrCodeString) {
  QRCode qrcode;

  uint8_t qrcodeBytes[qrcode_getBufferSize(qrCodeVersion)];
  qrcode_initText(&qrcode, qrcodeBytes, qrCodeVersion, ECC_LOW,
                  qrCodeString.c_str());

  display.clearDisplay();

  int startX = (SCREEN_WIDTH - (qrcode.size * pixelSize) - (pixelSize * 2))
               / 2;
  int startY = (SCREEN_HEIGHT - (qrcode.size * pixelSize) - (pixelSize * 2))
               / 2;

  int qrCodeSize = qrcode.size;

  display.fillRect(startX, startY, (qrCodeSize * pixelSize) + (pixelSize * 2),
                   (qrCodeSize * pixelSize) + (pixelSize * 2), WHITE);

  for (uint8_t y = 0; y < qrCodeSize; y++) {
    for (uint8_t x = 0; x < qrCodeSize; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(x * pixelSize + startX + pixelSize,
                         y * pixelSize + startY + pixelSize, pixelSize,
                         pixelSize, BLACK);
      }
    }
  }
  display.display();
}

const char* ssid = "gabraal";
const char* password = "nimishhh";

// Your Domain name with URL path or IP address with path
String serverName = "https://192.168.29.115:3001/checkProduct?productHash=";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
// unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

const int PRODUCT_HASH_LENGTH = 11;  // Define the desired length of the product hash

//*****************************************************************************************//
void setup() {
  myservo.attach(2);
  Serial.begin(115200); 
  SPI.begin();                                                  // Init SPI bus
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  mfrc522.PCD_Init();                                              // Init MFRC522 card
//  displayScrollingText();
}

//*****************************************************************************************//
void loop() {
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  // STORE PRODUCT HASH IN A STRING
  String productHash;
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      productHash += (char)buffer1[i];
    }
  }

  // CHECK IF PRODUCT HASH LENGTH REACHED DESIRED VALUE
  if (productHash.length() == PRODUCT_HASH_LENGTH) {
    String slicedHash = productHash.substring(1);  // Slice the productHash to exclude the first character
    processProductHash(slicedHash);  // Pass the product hash to the function for printing
  }

  delay(1000); // Change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

//*****************************************************************************************//

// Function to send an HTTP request
void sendHttpRequest(const String& url) {
  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(client, url.c_str());

    // If you need Node-RED/server authentication, insert user and password below
    // http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      if (httpResponseCode == 200) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Please Claim reward");
        display.println("in next 2 minutes");
        display.display();
        delay(5000);
        showQRCode("http://192.168.29.115:3000/" + payload);
        delay(20000);

        int pos;
            for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
                            // in steps of 1 degree
             myservo.write(pos);              // tell servo to go to position in variable 'pos'
              delay(15);                       // waits 15ms for the servo to reach the position
              }
          delay(3000);
          for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
             myservo.write(pos);              // tell servo to go to position in variable 'pos'
              delay(15);                       // waits 15ms for the servo to reach the position
             }
//        displayScrollingText();
          display.clearDisplay();
          display.display();
      }
    }
    else {
      
      Serial.println(httpResponseCode);
    }
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

// Function to print the product hash
void processProductHash(const String& pHash) {
  Serial.println(pHash);
  // Send an HTTP POST request depending on timerDelay
  if ((millis() - lastTime) > timerDelay) {
    String serverPath = serverName + pHash;
    sendHttpRequest(serverPath);
    lastTime = millis();
  }
}
