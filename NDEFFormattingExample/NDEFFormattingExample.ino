/*
 * This program is modeified from an example provided by the RC522 library:
 * https://github.com/miguelbalboa/rfid
 * 
 * Author: BoolLi
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>
#include <RC522_RFID_Utilities.h>

#define RST_PIN    9   // 
#define SS_PIN    10    //

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
RC522_RFID_Utilities rc522Utilities;

void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  rc522Utilities.setMFRC(&mfrc522);
  rc522Utilities.showReaderDetails(); // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, type, and data blocks..."));
}

void loop() {
  if (rc522Utilities.detectNewCard()) {
    rc522Utilities.formatToNDEF(&rc522Utilities.NFCDefaultKeyB, &rc522Utilities.NFCDefaultKeyB);

    //write some test messages to sector 1
    /* sector 1 block 4 
    byte dataBlock14[] = {0x03, 0x17, 0xD1, 0x01,
                          0x13, 0x54, 0x02, 0x65,
                          0x6E, 0x61, 0x62, 0x63,
                          0x64, 0x65, 0x66, 0x67
                         }; */
    /* sector 1 block 5 
    byte dataBlock15[] = {0x68, 0x69, 0x6A, 0x6B,
                          0x6C, 0x6D, 0x6E, 0x6F,
                          0x70, 0xFE, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00
                         }; */
    /* sector 1 block 6 
    byte dataBlock16[] = {0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00
                         }; */
    /*
    rc522Utilities.writeBlock(1, 4, dataBlock14, &rc522Utilities.NFCKeyB);
    rc522Utilities.writeBlock(1, 5, dataBlock15, &rc522Utilities.NFCKeyB);
    rc522Utilities.writeBlock(1, 6, dataBlock16, &rc522Utilities.NFCKeyB);
    */
    rc522Utilities.printAllSectors(&rc522Utilities.MADKeyA, &rc522Utilities.NFCKeyA);
  }
}

