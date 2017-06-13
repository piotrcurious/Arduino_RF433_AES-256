// Note: Updated Manchester library from github fails when
// transmit data values above 767 (0x02FF), the receiver can't 
// get the package

#include "Manchester.h"
#include "src/AVR_Crypto_Lib_aes/aes.h"

#define TX_DEVICE_ID 0x1B
#define CMD 0xFF
#define P_RX 4
#define AES_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

const uint8_t key[AES_KEY_SIZE] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

uint8_t toDecodeData[AES_BLOCK_SIZE];
aes256_ctx_t key_context;

void setup()
{
    Serial.begin(9600);
    aes256_init(key, &key_context);
    man.setupReceive(P_RX, MAN_1200);
    man.beginReceiveArray(AES_BLOCK_SIZE, toDecodeData);
}

void loop()
{
    uint8_t id, cmd;
    
    if(man.receiveComplete())
    {
        aes256_dec(toDecodeData, &key_context);
        id = toDecodeData[0];
        cmd = toDecodeData[1];

        man.beginReceiveArray(AES_BLOCK_SIZE, toDecodeData);

        Serial.println("Package received");
        Serial.print("ID: ");
        Serial.print(id);
        Serial.print("     CMD: ");
        Serial.println(cmd);

        if(id == TX_DEVICE_ID)
        {
            if(cmd == CMD)
            {
                Serial.print("Expected command from [");
                Serial.print(TX_DEVICE_ID);
                Serial.println("] has been received");
            }
        }

        Serial.println(" ");
    }
}

/*
Serial.print("Package content (decodec) - ");
for(int i = 0; i < AES_BLOCK_SIZE; i++)
{
    Serial.print(i);
    Serial.print(": ");
    Serial.print(toDecodeData[i]);
    Serial.print(", ");
}            
Serial.println(" ");
*/
