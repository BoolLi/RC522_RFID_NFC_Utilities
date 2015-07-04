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

#define RST_PIN		9		// 
#define SS_PIN		10		//

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance

// Define keys
MFRC522::MIFARE_Key NFCDefaultKeyA;
MFRC522::MIFARE_Key NFCKeyA;
MFRC522::MIFARE_Key NFCKeyB;
MFRC522::MIFARE_Key MADKeyA;
MFRC522::MIFARE_Key MADKeyB;

/*
 * Prints a sector using key A
 * 
 * Arguments:
 *  sector: the sector number
 *  keyA: the authentication key
 */
void printSector(byte sector, MFRC522::MIFARE_Key *keyA)
{
    byte trailerBlock = sector * 4 + 3; // get the trailer block
    Serial.println(F("Authenticating using key A..."));
    byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, keyA, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Show the whole sector as it currently is
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), keyA, sector);
    Serial.println();
}

/*
 * Prints all the sectors
 */
void printAllSectors(MFRC522::MIFARE_Key *MADKeyA, MFRC522::MIFARE_Key *NFCKeyA) 
{
  byte sector = 0;
  printSector(sector, MADKeyA);

  for (sector = 1; sector <= 15; sector++) {
    printSector(sector, NFCKeyA);
  }
}

/*
 * Copies a byte array to a Mifare key
 * Arguments:
 *  array: the byte array
 *  key: the Miare key
 */
void copyBytesToKey(byte *array,  MFRC522::MIFARE_Key *key)
{
  for (int i = 0; i < 6; i++)
    key->keyByte[i] = array[i];
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

/**
 * Writes a block of data to the tag
 * 
 * Arguments:
 *  sector: the sector number
 *  blockAddr: the destination block address
 *  dataBlock: a pointer to a 16-byte block 
 *  keyB: a pointer to a keyB
 */
void writeBlock(byte sector, byte blockAddr, byte* dataBlock, MFRC522::MIFARE_Key *keyB)
{
    byte trailerBlock = sector * 4 + 3;
    Serial.println(F("Authenticating again using key B..."));
    byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, keyB, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }


    // Write first chunk of data to the block
    Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    dump_byte_array(dataBlock, 16); Serial.println();
    status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();
}

void writeDefaultMessage() {
    /* sector 1 block 4 */
  byte dataBlock14[] ={0x03, 0x11, 
                      0xD1, 0x01, 0x0D, 0x55, 
                      0x01, 0x61, 0x64, 0x61, 
                      0x66, 0x72, 0x75, 0x69, 0x74, 0x2E};       
  /* sector 1 block 5 */
  byte dataBlock15[] = { 0x63, 0x6F, 
                      0x6D, 0xFE, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00};     
  /* sector 1 block 6 */
  byte dataBlock16[] = {0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00};   

  /* Rest of the data blocks are null */
  byte dataBlockNull[] = {0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00};   

  writeBlock(1,4, dataBlock14, &NFCKeyB);
  writeBlock(1,5, dataBlock15, &NFCKeyB);
  writeBlock(1,6, dataBlock16, &NFCKeyB);

  for (int i = 2; i <= 15; i++) {
    writeBlock(i, 4 * i, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 1, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 2, dataBlockNull, &NFCKeyB);
  }
}

void writePlainText() {
    /* sector 1 block 4 */
  byte dataBlock14[] ={0x03, 0x17, 0xD1, 0x01,
                      0x13, 0x54, 0x02, 0x65, 
                      0x6E, 0x61, 0x62, 0x63, 
                      0x64, 0x65, 0x66, 0x67};       
  /* sector 1 block 5 */
  byte dataBlock15[] = {0x68, 0x69, 0x6A, 0x6B,
                      0x6C, 0x6D, 0x6E, 0x6F, 
                      0x70, 0xFE, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00};     
  /* sector 1 block 6 */
  byte dataBlock16[] = {0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00};   

  /* Rest of the data blocks are null */
  byte dataBlockNull[] = {0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00};   

  writeBlock(1,4, dataBlock14, &NFCKeyB);
  writeBlock(1,5, dataBlock15, &NFCKeyB);
  writeBlock(1,6, dataBlock16, &NFCKeyB);

  for (int i = 2; i <= 15; i++) {
    writeBlock(i, 4 * i, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 1, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 2, dataBlockNull, &NFCKeyB);
  }
}

void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522

  byte NFCKeyADefaultByteArray[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  copyBytesToKey(NFCKeyADefaultByteArray, &NFCDefaultKeyA);

  byte NFCKeyAByteArray[] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
  copyBytesToKey(NFCKeyAByteArray, &NFCKeyA);

  byte NFCKeyBByteArray[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  copyBytesToKey(NFCKeyBByteArray, &NFCKeyB);

  byte MADKeyAByteArray[] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
  copyBytesToKey(MADKeyAByteArray, &MADKeyA);

  byte MADKeyBByteArray[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  copyBytesToKey(MADKeyBByteArray, &MADKeyB);
 
	ShowReaderDetails();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, type, and data blocks..."));
}

void loop() {
	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  /* sector 0 block 3
  byte dataBlock[] = {0xA0, 0xA1, 0xA2, 0xA3, 
                      0xA4, 0xA5, 0x78, 0x77, 
                      0x88, 0xC1, 0xFF, 0xFF, 
                      0xFF, 0xFF, 0xFF, 0xFF};   */
                 
  printAllSectors(&MADKeyA, &NFCKeyA);
}

void ShowReaderDetails() {
	// Get the MFRC522 software version
	byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
	Serial.print(F("MFRC522 Software Version: 0x"));
	Serial.print(v, HEX);
	if (v == 0x91)
		Serial.print(F(" = v1.0"));
	else if (v == 0x92)
		Serial.print(F(" = v2.0"));
	else
		Serial.print(F(" (unknown)"));
	Serial.println("");
	// When 0x00 or 0xFF is returned, communication probably failed
	if ((v == 0x00) || (v == 0xFF)) {
		Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
	}
}




