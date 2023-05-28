

// Arduino code for RF433 communication using Manchester encoding, AES256 encryption and Hamming error correction

#include <Manchester.h> // https://github.com/mchr3k/arduino-libs-manchester

#include <AESLib.h> // https://github.com/DavyLandman/AESLib

#include <Hamming.h> // https://github.com/arduino12/Hamming

#define TX_PIN 12 // RF433 transmitter pin

#define RX_PIN 11 // RF433 receiver pin

#define BAUD_RATE 1200 // Manchester baud rate

#define KEY_LENGTH 32 // AES256 key length in bytes

#define DATA_LENGTH 16 // AES256 data length in bytes

#define HAMMING_DEPTH 2 // Hamming error correction depth

// AES256 key (32 bytes)

byte aes_key[KEY_LENGTH] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,

                            0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,

                            0x76, 0x2e, 0x71, 0x60, 0xf3, 0x86, 0x75, 0x9a,

                            0xfb, 0xe7, 0x9f, 0xa5, 0xb7, 0xa6, 0xda, 0x89};

// Data to be sent (16 bytes)

byte data[DATA_LENGTH] = {0x6b, 0xc1, 0xbe, 0xe2,

                          0x2e, 0x40, 0x9f, 0x96,

                          0xe9, 0x3d, 0x7e, 0x11,

                          0x73, 0x93, 0x17, 0x2a};

// Encrypted data buffer (16 bytes)

byte encrypted[DATA_LENGTH];

// Hamming encoded data buffer (32 bytes)

byte encoded[DATA_LENGTH * 2];

// Hamming decoded data buffer (16 bytes)

byte decoded[DATA_LENGTH];

void setup() {

  // Initialize serial communication

  Serial.begin(9600);

  // Initialize Manchester library

  man.setupTransmit(TX_PIN, BAUD_RATE);

  man.setupReceive(RX_PIN, BAUD_RATE);

  man.beginReceive();

  // Initialize AES256 library

  aesLib.gen_iv(aes_key); // Generate initialization vector from key

  aesLib.set_paddingmode((paddingMode)0); // Set padding mode to none

}

void loop() {

  // Check if data is available from receiver

  if (man.receiveComplete()) {

    // Get the received data length and buffer

    int len = man.getMessageLength();

    byte* msg = man.getMessage();

    // Print the received data in hex format

    Serial.print("Received: ");

    for (int i = 0; i < len; i++) {

      Serial.print(msg[i], HEX);

      Serial.print(" ");

    }

    Serial.println();

    // Decode the received data using Hamming

    Hamming.decode(msg, len, decoded, DATA_LENGTH, HAMMING_DEPTH);

    // Decrypt the decoded data using AES256

    aesLib.decrypt(decoded, encrypted, KEY_LENGTH, aes_key);

    // Print the decrypted data in hex format

    Serial.print("Decrypted: ");

    for (int i = 0; i < DATA_LENGTH; i++) {

      Serial.print(encrypted[i], HEX);

      Serial.print(" ");

    }

    Serial.println();

    // Restart receiving

    man.beginReceive();

  }

  // Encrypt the data using AES256

  aesLib.encrypt(data, encrypted, KEY_LENGTH, aes_key);

  // Encode the encrypted data using Hamming

  Hamming.encode(encrypted, DATA_LENGTH, encoded, DATA_LENGTH * 2, HAMMING_DEPTH);

  // Transmit the encoded data using Manchester

  man.transmitArray(DATA_LENGTH * 2, encoded);

  // Print the transmitted data in hex format

  Serial.print("Transmitted: ");

  for (int i = 0; i < DATA_LENGTH * 2; i++) {

    Serial.print(encoded[i], HEX);

    Serial.print(" ");

  }

  Serial.println();

  // Wait for a second

  delay(1000);

}

