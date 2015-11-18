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

#define RST_PIN		9		// 
#define SS_PIN		10		//

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance
RC522_RFID_Utilities rc522Utilities;

void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
  rc522Utilities.setMFRC(&mfrc522);
	rc522Utilities.showReaderDetails();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, type, and data blocks..."));
}

void loop() {

  if (rc522Utilities.detectNewCard()) {

    /* To write a byte to a tag, use the function below.
       The first paramater is the byte you want to write to the tag. The example below uses 'c'. 
       Note that you can only write one byte (one character) using this function, so things like 'ab'
       won't work.

       The second parameter is the block address. Just use 5 since it is the first block of the
       second sector. 

       The third parameter is the keyB to use. By default it uses &rc522Utilities.NFCKeyB. 
    */
    rc522Utilities.writeByteToTag('c', 5, &rc522Utilities.NFCKeyB);

    /* To read a byte from the tag, use the function below.
     *  The first parameter is the block address. Since we just wrote a byte 'c' to block 5, we will be 
     *  reading from block 5 as well.
     *  
     *  The second parameter is the keyA that is used to authenticate the read operation. By default it 
     *  uses &rc522Utilities.NFCKeyA.
     *  
     *  The function returns a char. We can print that char using Serial.print(b).
     */
    char b = rc522Utilities.readByteFromTag(5, &rc522Utilities.NFCKeyA);
    Serial.print(b);
  }
}






