//written by BingAI 

// Include the libraries for RF433 communication, AES256 encryption and Hamming error correction

#include <RH_ASK.h>
#include <AESLib.h>
#include <Hamming.h>

// Define the pins for RF433 transmitter and receiver
#define TX_PIN 12
#define RX_PIN 11

// Define the AES256 encryption key and initialization vector
#define AES_KEY "0123456789ABCDEF0123456789ABCDEF"
#define AES_IV "FEDCBA9876543210FEDCBA9876543210"

// Define the Hamming error correction depth (number of parity bits)
#define HAMMING_DEPTH 4

// Create an instance of RH_ASK for RF433 communication
RH_ASK rf433(2000, RX_PIN, TX_PIN);

// Create an instance of Hamming for error correction
Hamming hamming(HAMMING_DEPTH);

// Initialize the AES256 encryption library
aes256_context aes_ctx;
byte aes_key[32];
byte aes_iv[16];

// Convert the AES_KEY and AES_IV strings to byte arrays
void convertKeyAndIV() {
  for (int i = 0; i < 32; i++) {
    aes_key[i] = (byte)AES_KEY[i];
    aes_iv[i] = (byte)AES_IV[i];
  }
}

// Encrypt a message using AES256 and return the encrypted data and its length
void encryptMessage(char* message, byte* encrypted_data, int* encrypted_length) {
  // Pad the message with zeros to make it a multiple of 16 bytes
  int message_length = strlen(message);
  int padded_length = message_length + (16 - (message_length % 16));
  byte padded_message[padded_length];
  for (int i = 0; i < padded_length; i++) {
    if (i < message_length) {
      padded_message[i] = (byte)message[i];
    } else {
      padded_message[i] = 0;
    }
  }
  // Encrypt the padded message using AES256
  aes256_init(&aes_ctx, aes_key);
  aes256_cbc_encrypt_buffer(&aes_ctx, encrypted_data, padded_message, padded_length, aes_iv);
  aes256_done(&aes_ctx);
  // Set the encrypted length to the padded length
  *encrypted_length = padded_length;
}

// Decrypt a message using AES256 and return the decrypted data and its length
void decryptMessage(byte* encrypted_data, int encrypted_length, char* decrypted_data, int* decrypted_length) {
  // Decrypt the encrypted data using AES256
  aes256_init(&aes_ctx, aes_key);
  aes256_cbc_decrypt_buffer(&aes_ctx, decrypted_data, encrypted_data, encrypted_length, aes_iv);
  aes256_done(&aes_ctx);
  // Find the actual length of the decrypted message by removing the padding zeros
  int actual_length = encrypted_length;
  for (int i = encrypted_length - 1; i >= 0; i--) {
    if (decrypted_data[i] == 0) {
      actual_length--;
    } else {
      break;
    }
  }
  // Set the decrypted length to the actual length
  *decrypted_length = actual_length;
}

// Encode a message using Hamming and return the encoded data and its length
void encodeMessage(byte* message, int message_length, byte* encoded_data, int* encoded_length) {
  // Calculate the number of bytes needed to store the encoded message
  int bits_per_byte = 8 - HAMMING_DEPTH;
  int encoded_bytes = (message_length * 8 + bits_per_byte - 1) / bits_per_byte;
  // Encode each byte of the message using Hamming
  for (int i = 0; i < encoded_bytes; i++) {
    // Get the next bits_per_byte bits from the message
    byte data = 0;
    for (int j = 0; j < bits_per_byte; j++) {
      int bit_index = i * bits_per_byte + j;
      if (bit_index < message_length * 8) {
        // Get the bit from the message and shift it to the right position
        int byte_index = bit_index / 8;
        int bit_offset = bit_index % 8;
        byte bit = (message[byte_index] >> (7 - bit_offset)) & 1;
        data |= (bit << (bits_per_byte - 1 - j));
      }
    }
    // Encode the data using Hamming and store it in the encoded data
    byte encoded = hamming.encode(data);
    encoded_data[i] = encoded;
  }
  // Set the encoded length to the encoded bytes
  *encoded_length = encoded_bytes;
}

// Decode a message using Hamming and return the decoded data and its length
void decodeMessage(byte* encoded_data, int encoded_length, byte* decoded_data, int* decoded_length) {
  // Calculate the number of bits needed to store the decoded message
  int bits_per_byte = 8 - HAMMING_DEPTH;
  int decoded_bits = encoded_length * bits_per_byte;
  // Decode each byte of the encoded data using Hamming
  for (int i = 0; i < decoded_bits; i++) {
    // Get the next byte from the encoded data and decode it using Hamming
    int byte_index = i / bits_per_byte;
    int bit_offset = i % bits_per_byte;
    byte encoded = encoded_data[byte_index];
    byte decoded = hamming.decode(encoded);
    // Get the bit from the decoded data and shift it to the right position
    byte bit = (decoded >> (bits_per_byte - 1 - bit_offset)) & 1;
    // Store the bit in the decoded data
    int output_index = i / 8;
    int output_offset = i % 8;
    if (output_offset == 0) {
      // Clear the output byte before setting the bit
      decoded_data[output_index] = 0;
    }
    decoded_data[output_index] |= (bit << (7 - output_offset));
  }
  // Set the decoded length to the number of bytes needed to store the decoded bits
  *decoded_length = (decoded_bits + 7) / 8;
}

// Send a message using RF433 communication, Manchester encoding, AES256 encryption and Hamming error correction
void sendMessage(char* message) {
  // Encrypt the message using AES256
  byte encrypted_data[256];
  int encrypted_length;
  encryptMessage(message, encrypted_data, &encrypted_length);
  
  // Encode the message using Hamming
  byte encoded_data[256];
  int encoded_length;
  encodeMessage(encrypted_data, encrypted_length, encoded_data, &encoded_length);
  
  // Send the message using RF433 and Manchester encoding
  rf433.send(encoded_data, encoded_length);
  rf433.waitPacketSent();
}

// Receive a message using RF433 communication, Manchester encoding, AES256 encryption and Hamming error correction
bool receiveMessage(char* message) {
  // Receive the message using RF433 and Manchester encoding
  uint8_t encoded_data[256];
  uint8_t encoded_length = sizeof(encoded_data);
  if (rf433.recv(encoded_data, &encoded_length)) {
    // Decode the message using Hamming
    byte decrypted_data[256];
    int decrypted_length;
    decodeMessage(encoded_data, encoded_length, decrypted_data, &decrypted_length);
    
    // Decrypt the message using AES256
    char decrypted_message[256];
    int decrypted_message_length;
    decryptMessage(decrypted_data, decrypted_length, decrypted_message, &decrypted_message_length);
    
    // Copy the decrypted message to the output message
    strncpy(message, decrypted_message, decrypted_message_length);
    message[decrypted_message_length] = '\0';
    
    // Return true to indicate a successful reception
    return true;
  } else {
    // Return false to indicate a failed reception
    return false;
  }
}

void setup() {
  // Initialize the serial monitor for debugging
  Serial.begin(9600);
  
  // Initialize the RF433 communication
  if (!rf433.init()) {
    Serial.println("RF433 init failed");
  }
  
  // Convert the AES_KEY and AES_IV strings to byte arrays
  convertKeyAndIV();
}

void loop() {
  // Send a test message every 5 seconds
  static unsigned long last_send_time = 0;
  if (millis() - last_send_time > 5000) {
    char test_message[] = "Hello world!";
    sendMessage(test_message);
    Serial.print("Sent: ");
    Serial.println(test_message);
    last_send_time = millis();
  }
  
  // Receive any incoming messages and print them to the serial monitor
  char received_message[256];
  if (receiveMessage(received_message)) {
    Serial.print("Received: ");
    Serial.println(received_message);
  }
}
