#include <Arduino.h>
#include <RC522_RFID_Utilities.h>

RC522_RFID_Utilities::RC522_RFID_Utilities()
  : mfrc522(NULL)
{
  byte NFCKeyADefaultByteArray[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  copyBytesToKey(NFCKeyADefaultByteArray, &NFCDefaultKeyA);
  copyBytesToKey(NFCKeyADefaultByteArray, &NFCDefaultKeyB);

  byte NFCKeyAByteArray[] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
  copyBytesToKey(NFCKeyAByteArray, &NFCKeyA);

  byte NFCKeyBByteArray[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  copyBytesToKey(NFCKeyBByteArray, &NFCKeyB);

  byte MADKeyAByteArray[] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
  copyBytesToKey(MADKeyAByteArray, &MADKeyA);

  byte MADKeyBByteArray[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  copyBytesToKey(MADKeyBByteArray, &MADKeyB);
}

RC522_RFID_Utilities::RC522_RFID_Utilities(MFRC522 *mfrc522)
  : mfrc522(mfrc522)
{
  byte NFCKeyADefaultByteArray[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  copyBytesToKey(NFCKeyADefaultByteArray, &NFCDefaultKeyA);
  copyBytesToKey(NFCKeyADefaultByteArray, &NFCDefaultKeyB);

  byte NFCKeyAByteArray[] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
  copyBytesToKey(NFCKeyAByteArray, &NFCKeyA);

  byte NFCKeyBByteArray[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  copyBytesToKey(NFCKeyBByteArray, &NFCKeyB);

  byte MADKeyAByteArray[] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
  copyBytesToKey(MADKeyAByteArray, &MADKeyA);

  byte MADKeyBByteArray[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  copyBytesToKey(MADKeyBByteArray, &MADKeyB);
}

void RC522_RFID_Utilities::setMFRC(MFRC522 *mfrc522)
{
  this->mfrc522 = mfrc522;
}

void RC522_RFID_Utilities::showReaderDetails()
{
  // Get the MFRC522 software version
  byte v = mfrc522->PCD_ReadRegister(mfrc522->VersionReg);
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

bool RC522_RFID_Utilities::detectNewCard()
{
    // Look for new cards
  if (!mfrc522->PICC_IsNewCardPresent()) {
    return false;
  }

  // Select one of the cards
  if (!mfrc522->PICC_ReadCardSerial()) {
    return false;
  }

  return true;
}

void RC522_RFID_Utilities::copyBytesToKey(byte *array,  MFRC522::MIFARE_Key *key)
{
  for (int i = 0; i < 6; i++)
    key->keyByte[i] = array[i];
}

void RC522_RFID_Utilities::printSector(byte sector, MFRC522::MIFARE_Key *keyA)
{
  byte trailerBlock = sector * 4 + 3; // get the trailer block
  Serial.println(F("Authenticating using key A..."));
  byte status = mfrc522->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, keyA, &(mfrc522->uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522->GetStatusCodeName(status));
    return;
  }

  // Show the whole sector as it currently is
  Serial.println(F("Current data in sector:"));
  mfrc522->PICC_DumpMifareClassicSectorToSerial(&(mfrc522->uid), keyA, sector);
  Serial.println();
}

void RC522_RFID_Utilities::printAllSectors(MFRC522::MIFARE_Key *MADKeyA, MFRC522::MIFARE_Key *NFCKeyA)
{
  byte sector = 0;
  printSector(sector, MADKeyA);

  for (sector = 1; sector <= 15; sector++) {
    printSector(sector, NFCKeyA);
  }
}

void RC522_RFID_Utilities::dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void RC522_RFID_Utilities::writeBlock(byte sector, byte blockAddr, byte* dataBlock, MFRC522::MIFARE_Key *keyB)
{
  byte trailerBlock = sector * 4 + 3;
  Serial.println(F("Authenticating using key B..."));
  byte status = mfrc522->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, keyB, &(mfrc522->uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522->GetStatusCodeName(status));
    return;
  }

  Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
  Serial.println(F(" ..."));
  dump_byte_array(dataBlock, 16); Serial.println();
  status = mfrc522->MIFARE_Write(blockAddr, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522->GetStatusCodeName(status));
  }
  Serial.println();
}


void RC522_RFID_Utilities::formatToNDEF(MFRC522::MIFARE_Key *OldMADKeyB, MFRC522::MIFARE_Key *OldNFCKeyB)
{
  // Block 1 and 2 of sector 0
  byte dataBlock01[] = {0x14, 0x01, 0x03, 0xE1,
                        0x03, 0xE1, 0x03, 0xE1,
                        0x03, 0xE1, 0x03, 0xE1,
                        0x03, 0xE1, 0x03, 0xE1
                       };
  byte dataBlock02[] = {0x03, 0xE1, 0x03, 0xE1,
                        0x03, 0xE1, 0x03, 0xE1,
                        0x03, 0xE1, 0x03, 0xE1,
                        0x03, 0xE1, 0x03, 0xE1
                       };
  // trailer block for sector 0. By default MAD key A should be [0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5],
  // and MAD key B should be [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]. Access bits are [0x78, 0x77, 0x88],
  // and the GPB is 0xC1.
  byte dataBlock03[] = {0xA0, 0xA1, 0xA2, 0xA3,
                        0xA4, 0xA5, 0x78, 0x77,
                        0x88, 0xC1, 0xFF, 0xFF,
                        0xFF, 0xFF, 0xFF, 0xFF
                       };
  /* sector 1 block 4 */
  byte dataBlock14[] = {0x03, 0x17, 0xD1, 0x01,
                        0x13, 0x54, 0x02, 0x65,
                        0x6E, 0x61, 0x62, 0x63,
                        0x64, 0x65, 0x66, 0x67
                       };
  /* sector 1 block 5 */
  byte dataBlock15[] = {0x68, 0x69, 0x6A, 0x6B,
                        0x6C, 0x6D, 0x6E, 0x6F,
                        0x70, 0xFE, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00
                       };
  /* sector 1 block 6 */
  byte dataBlockNull[] = {0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00
                       };
  // trailer block for rest of the NFC sectors. By default the NFC key A should be [0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7],
  // and the NFC key B should be [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]. Access bits are [0x7F, 0x07, 0x88], 
  // and the GPB is 0x40.
  byte nfcTrailerBlock[] = {0xD3, 0xF7, 0xD3, 0xF7,
                            0xD3, 0xF7, 0x7F, 0x07,
                            0x88, 0x40, 0xFF, 0xFF,
                            0xFF, 0xFF, 0xFF, 0xFF
                           };

  // write to sector 0 block 3 first using the default key B
  writeBlock(0, 3, dataBlock03, OldMADKeyB);
  // write to the rest of the blocks using the new MAD key B
  writeBlock(0, 1, dataBlock01, &MADKeyB);
  writeBlock(0, 2, dataBlock02, &MADKeyB);

  // format sector 1 to 15
  for (int i = 1; i <= 15; i++) {
    // format the trailer block first
    writeBlock(i, 4 * i + 3, nfcTrailerBlock, OldMADKeyB);
    // nullify the rest of the blocks
    writeBlock(i, 4 * i, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 1, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 2, dataBlockNull, &NFCKeyB);
  }
}




void RC522_RFID_Utilities::writeDefaultMessage() {
  /* sector 1 block 4 */
  byte dataBlock14[] = {0x03, 0x11,
                        0xD1, 0x01, 0x0D, 0x55,
                        0x01, 0x61, 0x64, 0x61,
                        0x66, 0x72, 0x75, 0x69, 0x74, 0x2E
                       };
  /* sector 1 block 5 */
  byte dataBlock15[] = { 0x63, 0x6F,
                         0x6D, 0xFE, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                       };
  /* sector 1 block 6 */
  byte dataBlock16[] = {0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00
                       };

  /* Rest of the data blocks are null */
  byte dataBlockNull[] = {0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00
                         };

  writeBlock(1, 4, dataBlock14, &NFCKeyB);
  writeBlock(1, 5, dataBlock15, &NFCKeyB);
  writeBlock(1, 6, dataBlock16, &NFCKeyB);

  for (int i = 2; i <= 15; i++) {
    writeBlock(i, 4 * i, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 1, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 2, dataBlockNull, &NFCKeyB);
  }
}

void RC522_RFID_Utilities::writePlainText() {
  /* sector 1 block 4 */
  byte dataBlock14[] = {0x03, 0x17, 0xD1, 0x01,
                        0x13, 0x54, 0x02, 0x65,
                        0x6E, 0x61, 0x62, 0x63,
                        0x64, 0x65, 0x66, 0x67
                       };
  /* sector 1 block 5 */
  byte dataBlock15[] = {0x68, 0x69, 0x6A, 0x6B,
                        0x6C, 0x6D, 0x6E, 0x6F,
                        0x70, 0xFE, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00
                       };
  /* sector 1 block 6 */
  byte dataBlock16[] = {0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00
                       };

  /* Rest of the data blocks are null */
  byte dataBlockNull[] = {0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00
                         };

  writeBlock(1, 4, dataBlock14, &NFCKeyB);
  writeBlock(1, 5, dataBlock15, &NFCKeyB);
  writeBlock(1, 6, dataBlock16, &NFCKeyB);

  for (int i = 2; i <= 15; i++) {
    writeBlock(i, 4 * i, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 1, dataBlockNull, &NFCKeyB);
    writeBlock(i, 4 * i + 2, dataBlockNull, &NFCKeyB);
  }
}