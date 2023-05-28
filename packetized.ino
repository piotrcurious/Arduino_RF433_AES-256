// Include the necessary libraries
#include <Manchester.h>
#include <AESLib.h>
#include <Hamming.h>

// Define the RF433 pins
#define TX_PIN 12
#define RX_PIN 11

// Define the AES256 key and initialization vector
#define KEY "01234567890123456789012345678901"
#define IV "0123456789012345"

// Define the packet size and structure
#define PACKET_SIZE 16 // 4 bytes for ID + 12 bytes for data
struct Packet {
  uint32_t id; // 32-bit station ID
  byte data[12]; // Data payload
};

// Initialize the Manchester encoder/decoder
Manchester man;

// Initialize the AES256 encryptor/decryptor
AESLib aesLib;

// Initialize the Hamming encoder/decoder with adjustable depth
Hamming hamming(4); // Change this to adjust the depth

void setup() {
  // Set up serial communication for debugging
  Serial.begin(9600);

  // Set up the RF433 transmitter and receiver
  man.workAround1MhzTinyCore(); // Remove this line if not using ATtiny85/84
  man.setupTransmit(TX_PIN, MAN_1200);
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceive();
}

void loop() {
  // Check if there is a packet to receive
  if (man.receiveComplete()) {
    // Get the received bytes
    byte* bytes = man.getMessage();
    int len = man.getMessageLength();

    // Decrypt the bytes with AES256
    byte decrypted[len];
    aesLib.decrypt(bytes, decrypted, KEY, len, IV);

    // Decode the bytes with Hamming
    byte decoded[len / 2];
    hamming.decodeArray(decrypted, len, decoded);

    // Cast the bytes to a packet structure
    Packet* packet = (Packet*)decoded;

    // Print the packet contents
    Serial.print("Received packet from station ");
    Serial.println(packet->id, HEX);
    Serial.print("Data: ");
    for (int i = 0; i < 12; i++) {
      Serial.print(packet->data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Resume receiving
    man.beginReceive();
  }

  // Check if there is data to send
  if (Serial.available() > 0) {
    // Read the data from serial
    byte data[12];
    for (int i = 0; i < 12; i++) {
      data[i] = Serial.read();
    }

    // Create a packet with a random ID and the data
    Packet packet;
    packet.id = random(0xFFFFFFFF);
    memcpy(packet.data, data, 12);

    // Cast the packet to bytes
    byte* bytes = (byte*)&packet;

    // Encode the bytes with Hamming
    byte encoded[PACKET_SIZE];
    hamming.encodeArray(bytes, PACKET_SIZE / 2, encoded);

    // Encrypt the bytes with AES256
    byte encrypted[PACKET_SIZE];
    aesLib.encrypt(encoded, encrypted, KEY, PACKET_SIZE, IV);

    // Send the bytes with RF433
    man.transmitArray(PACKET_SIZE, encrypted);
    Serial.println("Sent packet");
  }
}
