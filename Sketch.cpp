//software for reading and saving EEPROM varible updates for Drone Aviation PCBs w/ microcontroller & Xport module
//Written by Jakob A. Howe 3/6/17 - Massive credit to Joel Dunham A.K.A. the Code Wizard

#include <Arduino.h>
#include <avr/eeprom.h>

#define MESSAGE_ID_EEPROM_VARIBLES 1000 //500
#define DATALINK_SYNC0 0xa3
#define DATALINK_SYNC1 0xb2
#define DATALINK_SYNC2 0xc1

// EEPROM Variables
// A = byte 0 of EEPROM memory
// B = byte 2 of EEPROM memory
// C = byte 4 of EEPROM memory
// D = byte 6 of EEPROM memory
// E = byte 8 of EEPROM memory
// F = byte 10 of EEPROM memory

// Define the message header structure
struct __attribute__((__packed__)) messageHeader {
	uint8_t sync1;
	uint8_t sync2;
	uint8_t sync3;
	uint8_t spare;
	int32_t messageID;
	int32_t messageSize; // Includes header
	uint32_t hcsum;  // Header checksum
	uint32_t csum;  // Payload checksum
};

// Define the header that will be used for processing all incoming messages
struct messageHeader headerToRead;

// Define an appropriate max buffer size
uint32_t BUFFERSIZE; // See setup()

// Define the message received by peripheral controller
struct __attribute__((__packed__)) EEPROM_UPDATES {
	uint8_t sync1;								//1 byte
	uint8_t sync2;								//1 byte
	uint8_t sync3;								//1 byte
	uint8_t spare;								//1 byte
	int32_t messageID;							//4 bytes
	int32_t messageSize; // Includes header     //4 bytes
	uint32_t hcsum;  // Header checksum         //4 bytes
	uint32_t csum;  // Payload checksum         //4 bytes
												//--------20 bytes

	uint32_t value1;							//4 byte  
	uint32_t value2;							//4 byte  // order value3-value2-value1, value3's MSB is MSB of EEPROM values stored 
	uint32_t value3;							//4 byte
	uint8_t action;								//1 byte // desired action, read, write, other. currently 1=write to EEPROM   
	uint8_t spare1;								//1 byte
	uint16_t align;      // 4-byte alignment    //2 bytes
	uint32_t align2;     // 8-byte alignment    //4 bytes
} message; // Define the message to send        //--------20 bytes
//-----------------total = 40 bytes = 5 Deca-bytes = 320 bits 8 byte aligned


// Define the checksum computation algorithm for messages sent to the GCS
uint32_t checkSumCompute( uint8_t *buf, int32_t byteCount ) {

	uint32_t sum1 = 0xffffUL;
	uint32_t sum2 = 0xffffUL;
	uint32_t tlen = 0UL;
	uint32_t shortCount = byteCount / sizeof(int16_t);
	uint32_t oddLength  = byteCount % 2;

	/* this is Fletcher32 checksum modified to handle buffers with an odd number of bytes */

	while( shortCount ) {
		/* 360 is the largest number of sums that can be performed without overflow */
		tlen = shortCount > 360UL ? 360UL : shortCount;
		shortCount -= tlen;
		do {
			sum1 +=  (uint32_t)*buf++;
			sum1 += ((uint32_t)*buf++ << 8UL);
			sum2 += sum1;
		} while (--tlen);

		/* add last byte if there's an odd number of bytes (equivalent to appending a zero-byte) */
		if( (oddLength==1) && (shortCount<1) ) {
			sum1 += (uint32_t)*buf++;
			sum2 += sum1;
		}
		sum1 = (sum1 & 0xffffUL) + (sum1 >> 16UL);
		sum2 = (sum2 & 0xffffUL) + (sum2 >> 16UL);
	}

	/* Second reduction step to reduce sums to 16 bits */
	sum1 = (sum1 & 0xffffUL) + (sum1 >> 16UL);
	sum2 = (sum2 & 0xffffUL) + (sum2 >> 16UL);

	return( sum2 << 16UL | sum1 );
}

// Convenience function to finalize the header of a message before sending it
void checkSumEncode( uint8_t *buf, int32_t byteCount ) {
	struct messageHeader *h = (struct messageHeader *)buf;

	h->sync1 = DATALINK_SYNC0;
	h->sync2 = DATALINK_SYNC1;
	h->sync3 = DATALINK_SYNC2;

	h->messageSize = byteCount;

	h->hcsum = checkSumCompute(  buf, sizeof( struct messageHeader ) - sizeof( int32_t )*2 );
	h->csum  = checkSumCompute( &(buf[sizeof( struct messageHeader )]),
		byteCount - sizeof( struct messageHeader ) );
}

/**
* Inputs:
*          buf (uint8_t): the pointer to the incoming message buffer
*          byteCount (int32_t): is the number of bytes currently in the message buffer
* Outputs:
*          int32_t : the number of bytes processed in the message buffer
*/

int32_t readBytes( uint8_t *buf, int32_t byteCount ) {  //for storing serial data in a struct
	int32_t bufferIndex = 0; // The current read index in the received buffer
	uint8_t done = 0;  // When the loop is finished
	uint8_t *bf; // The buffer indexed at bufferIndex

	// Step through each byte in the buffer until there is not enough to read a message header or the processing is complete
	while( ( bufferIndex <= byteCount - (int32_t)sizeof( struct messageHeader ) ) && !done ) {

		// Look for the sync bytes.  This signals the start of a message
		if( ( buf[bufferIndex]   == DATALINK_SYNC0 ) &&
			( buf[bufferIndex + 1] == DATALINK_SYNC1 ) &&
			( buf[bufferIndex + 2] == DATALINK_SYNC2 ) ) {

				// Assign the indexed buffer for easier processing
				bf = &(buf[bufferIndex]);

				// Copy into the reading header for easy access to all fields
				memcpy( &headerToRead, bf, sizeof( struct messageHeader ) );

				if( checkSumCompute( bf, sizeof( struct messageHeader ) - sizeof( int32_t )*2 ) == headerToRead.hcsum && // Verify the header checksum
					headerToRead.messageSize >= sizeof( struct messageHeader ) && // Verify that the message size (which includes the header) is at least large enough
					headerToRead.messageSize < BUFFERSIZE ) { // Verify that we aren't waiting for something insanely large

						if( headerToRead.messageSize + bufferIndex <= byteCount ) { // The entire message has been read

							// Verify the message checksum
							if( checkSumCompute( &bf[sizeof( struct messageHeader )], headerToRead.messageSize - sizeof( struct messageHeader ) ) == headerToRead.csum ) { // is datalinkCheckSumCompute a separate function than checkSomeCompute

								// Switch based on the message ID
								switch( headerToRead.messageID ) {
									// The message you defined
								case MESSAGE_ID_EEPROM_VARIBLES:
									// Final check - is the message size correct?  If not, then there's an encoding issue from the other side, or the message definitions on both sides don't match
									if( headerToRead.messageSize == sizeof( struct EEPROM_UPDATES ) ) {
										// Copy the message to the internal message structure
										memcpy( &message, bf, sizeof( struct EEPROM_UPDATES ) );
										
									}
									break;
								default:
									// Bad message ID.  Log error?
									Serial.println("Unknown message ID - throw message away");
									break;
								}
							} else{
								// Bad checksum.  Log error?
								Serial.println("Bad checksum - message thrown away");
							}
							// Finished processing this message - index past it in the buffer
							bufferIndex += headerToRead.messageSize - 1;
							
						} else {
							// The full message has not been read yet.
							done = 1; // Break the while loop to read after more data is acquired
						}
				} else {
					// Header checksum is bad.  Log error?
					Serial.println("Bad header checksum - header thrown away");
					bufferIndex += sizeof( struct messageHeader ) - 1;  // Read past the message header, then start looking for sync bytes again
				}
		} else {
			// Sync bytes not found - look at next byte
			bufferIndex++;
		}
	}
	// How much data to clear from the buffer
	return bufferIndex;
}

uint8_t buf[64];
int32_t bytesInBuff;
const int LED_PIN = 12;


void setup() {
	// put your setup code here, to run once:
	message.messageID = MESSAGE_ID_EEPROM_VARIBLES; //MESSAGE_ID_EEPROM_VARIBLES;
	BUFFERSIZE = 64; // Define the appropriate max buffer size
	bytesInBuff = 0;
	Serial.begin(9600);
	Serial.println("Ready");
	Serial.setTimeout(1000); // 1 second timeout
	pinMode(LED_PIN,OUTPUT);

}

void loop() {

	
	// Read bytes.  Either time out at 1 second, or read a full message in (of 40 bytes) or fills the buffer with data to process
	uint32_t bytesToRead = 40;
	if(Serial.available() + bytesInBuff >= BUFFERSIZE) {
		bytesToRead = BUFFERSIZE - bytesInBuff; // Read everything that can go into the hold buffer
	} else if(Serial.available() + bytesInBuff < 40) {
		bytesToRead = 40 - bytesInBuff;  // The message size is 40, so try to read in the entire message
	}
	// Read the serial line and return the number of bytes read
	bytesInBuff += Serial.readBytes(&buf[bytesInBuff], bytesToRead );
	// Returns the number of bytes processed
	int32_t bytesProcessed = readBytes( &buf[0], bytesInBuff ); // calls read function to get message decode and compute checksum
	if(bytesProcessed > 0) {
		// Clear read bytes in the buffer
		memmove( &buf, &buf[bytesProcessed], bytesInBuff - bytesProcessed );
		bytesInBuff -= bytesProcessed;
	}

	// Do whatever you want with the decoded message now
	int ACTN = message.action; //uint8_t
	if(ACTN == 1){
		Serial.println("Updating EEPROM");
		uint32_t MSB = message.value3;
		Serial.print("Value 3: ");
		Serial.println(MSB);
		uint32_t MID = message.value2;
		Serial.print("Value 2: ");
		Serial.println(MID);
		uint32_t LSB = message.value1;
		Serial.print("Value 1: ");
		Serial.println(LSB);
		delay(1000);
	
		uint8_t VALS[12];

		memcpy(VALS, &MSB, sizeof(MSB));						 
		memcpy(VALS+sizeof(MSB),&MID,sizeof(MID));
		memcpy(VALS+sizeof(MSB)+sizeof(MID),&LSB,sizeof(LSB));

		Serial.println("");
		Serial.println("what saves to EEPROM");
		eeprom_update_block( VALS, 0, 11);  //eeprom_update_block( const void * __src, void * __dst, #bytes) //string of bytes is u[pdated to 
		for (int i=0; i<sizeof(VALS) ;++i){
			Serial.print(VALS[i]);			
		}
		
		Serial.println("");
		Serial.println("Read from EEPROM");
		uint8_t c = 10;
		for(uint16_t p=0; p<sizeof(VALS) ;++p){
			uint16_t EEPROM_VALUE = eeprom_read_word( (uint16_t*)p ); //eeprom_read_word( const uint16_t *p); //shows how EEPROM variables are accesed 
			Serial.println(c,HEX);
			Serial.println( EEPROM_VALUE );
			p++;
			c++;
		}
		uint16_t A = eeprom_read_word( (uint16_t*)0 ); // Blinks LED for use example of updated EEPROM variables
		Serial.println("");
		Serial.print( "Watch LED Blink " );
		Serial.print( A );
		Serial.println( " times" );
		for(uint8_t d=0; d<A; d++){
			digitalWrite(LED_PIN, HIGH);   
			delay(1000);                  
			digitalWrite(LED_PIN, LOW);    
			delay(1000);                  
		}
		Serial.println("");
		Serial.println("HIT RESET!"); // to prevent code from looping and updating EEPROM more than one time
		delay(100000000);
	} 
}

