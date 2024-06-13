// Include necessary libraries
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <SoftwareSerial.h>  // Include the SoftwareSerial library

// Define the RFID sensor pins
#define SS_PIN 10
#define RST_PIN 9

// Define pins for devices
const int greenLedPin = 5;
const int redLedPin = 4;
const int buzzerPin = 7;
const int servoPin = 3;

// Define pins for Bluetooth communication
const int bluetoothRXPin = 0;  // HC-05 TX pin connected to Arduino RX pin
const int bluetoothTXPin = 1;  // HC-05 RX pin connected to Arduino TX pin

// Create instances
MFRC522 rfid(SS_PIN, RST_PIN);
Servo myServo;
SoftwareSerial bluetooth(bluetoothRXPin, bluetoothTXPin);  // RX, TX

// Define authorized UID and passcode
byte authorizedUID[4] = {0x13, 0xBB, 0xCE, 0x1C};
const String passcode = "strongpwd";

void setup() {
    // Initialize serial communications
    Serial.begin(9600);
    bluetooth.begin(9600);
    
    // Initialize SPI bus and RFID reader
    SPI.begin();
    rfid.PCD_Init();
    
    // Initialize devices and servo motor
    pinMode(greenLedPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    myServo.attach(servoPin);
    
    // Print initialization message
    Serial.println("Place an RFID card near the reader or enter passcode via Bluetooth...");
    bluetooth.println("Enter passcode:");
    digitalWrite(buzzerPin, HIGH);  // Initially, buzzer is off
}

void loop() {
    // Check for Bluetooth data
    if (bluetooth.available()) {
        handleBluetooth();
    }

    // Check for RFID card presence and read it
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        handleRFID();
    }
}

void handleBluetooth() {
    // Read the Bluetooth data
    String receivedData = bluetooth.readStringUntil('\n');
    receivedData.trim();  // Remove any leading/trailing whitespace
    
    // Debug statement: print the received data
    Serial.println("Received data from Bluetooth: " + receivedData);
    
    // Compare received passcode with the correct passcode
    if (receivedData == passcode) {
        // Grant access and print the message to the serial monitor
        grantAccess();
        bluetooth.println("Access granted");
    } else {
        // Deny access and print the message to the serial monitor
        denyAccess();
        bluetooth.println("Access denied");
    }
}


void handleRFID() {
    // Compare card UID with the authorized UID
    if (memcmp(rfid.uid.uidByte, authorizedUID, 4) == 0) {
        grantAccess();
        Serial.println("Access granted");
    } else {
        denyAccess();
        Serial.println("Access denied");
    }
    
    // Halt PICC and stop encryption
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

void grantAccess() {
    // Activate green LED and servo motor
    digitalWrite(greenLedPin, HIGH);
    myServo.write(90);  // Open door
    
    // Delay to keep the door open for a while
    delay(2000);
    
    // Return servo to the original position and turn off green LED
    myServo.write(0);
    digitalWrite(greenLedPin, LOW);
}

void denyAccess() {
    // Activate red LED
    digitalWrite(redLedPin, HIGH);
    digitalWrite(buzzerPin, LOW);
    
    // Delay to keep red LED and buzzer on
    delay(1000);
    
    // Turn off red LED and buzzer
    digitalWrite(redLedPin, LOW);
    digitalWrite(buzzerPin, HIGH);
}
