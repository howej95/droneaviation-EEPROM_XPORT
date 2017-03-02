#include <Arduino.h>
//#include <EEPROM.h>
//#include <string>
//#include <iostream>
//#include < avr / eeprom .h >//#include <Winsock2.h> // may need for memcpy

#define MESSAGE_ID_EEPROM_VARIBLES 500
#define DATALINK_SYNC0 0xa3
#define DATALINK_SYNC1 0xb2
#define DATALINK_SYNC2 0xc1

//typedef unsigned __int128 uint128_t;


// Variables
// A = byte 0 of EEPROM memory
// B = byte 2 of EEPROM memory
// C = byte 4 of EEPROM memory
// D = byte 6 of EEPROM memory
// E = byte 8 of EEPROM memory
// F = byte 10 of EEPROM memory

// G = byte 12 of EEPROM memory
// H = byte 14 of EEPROM memory
// I = byte 16 of EEPROM memory
// J = byte 18 of EEPROM memory
// K = byte 20 of EEPROM memory

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
	uint8_t sync1;	//1 byte
	uint8_t sync2;													//1 byte
	uint8_t sync3;													//1 byte
	uint8_t spare;													//1 byte
	int32_t messageID;												//4 bytes
	int32_t messageSize; // Includes header							//4 bytes
	uint32_t hcsum;  // Header checksum								//4 bytes
	uint32_t csum;  // Payload checksum								//4 bytes
	//--------19 bytes
	
	uint8_t action;		 // desired action, read, write, other.		//1 byte
	uint32_t value1;												//4 byte	//10 variables desired each 2 bytes long so when value 1, 2, & 3 are combined it will give the value and location of the EEPROM update
	uint32_t value2;       // New value of variable if writing		//4 byte	// order value3-value2-value1, value3's MSB is MSB of EEPROM
	uint32_t value3;												//4 byte
	uint8_t spare1;													//1 byte
	uint8_t spare2;													//1 byte
	uint16_t align;      // 4-byte alignment						//2 bytes
	uint32_t align2;     // 8-byte alignment						//4 bytes
} message; // Define the message to send							//--------21 bytes
//-----------------total = 40 bytes = 5 Deca-bytes = 320 bits 8 byte aligned

//char hldr[] = { sync1, sync2, sync3, spare, messageID, messageSize, hcsum, csum, action, spare, value, spare, spare, align, align2};

char testmsg[300];

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

// I Assume this will need to be modified in order to decode rather than encode.
/*
// Convenience function to finalize the header of a message before sending it
void checkSumDEncode( uint8_t *buf, int32_t byteCount ) {

struct messageHeader *h = (struct messageHeader *)buf;

h->sync1 = DATALINK_SYNC0;
h->sync2 = DATALINK_SYNC1;
h->sync3 = DATALINK_SYNC2;

h->messageSize = byteCount;

h->hcsum = checkSumCompute(  buf, sizeof( struct messageHeader ) - sizeof( int32_t )*2 );
h->csum  = checkSumCompute( &(buf[sizeof( struct messageHeader )]),
byteCount - sizeof( struct messageHeader ) );

}
*/
/**
* Inputs:
*          buf (uint8_t): the pointer to the incoming message buffer
*          byteCount (int32_t): is the number of bytes currently in the message buffer
* Outputs:
*          int32_t : the number of bytes processed in the message buffer
*/

int32_t ReadDemBytes( uint8_t *buf, int32_t byteCount ) {  //for storing serial data storing serial data in a struct
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
								// TODO: Handle the data that is not stored in message
							}
							break;
							default:
							// Bad message ID.  Log error?
							break;
						}
						} else{
						// Bad checksum.  Log error?
					}
					// Finished processing this message - index past it in the buffer
					bufferIndex += headerToRead.messageSize - 1;
					} else {
					// The full message has not been read yet.
					done = 1; // Break the while loop to read after more data is acquired
				}
				} else {
				// Header checksum is bad.  Log error?
				bufferIndex += sizeof( struct messageHeader ) - 1;  // Read past the message header, then start looking for sync bytes again
			}
			} else {
			// Sync bytes not found - look at next byte
			bufferIndex++;
		}
	}
}


unsigned long COMM_START = 0;      // The time that the last status message was sent
unsigned long COMM_CURRENT = 0;    // The current time of the system

void setup() {
	// put your setup code here, to run once:
	message.messageID = MESSAGE_ID_EEPROM_VARIBLES;
	BUFFERSIZE = 1024*100; // Define the appropriate max buffer size, may be an issue default buffer size for atmega328p is 64 bytes or 512 bits
	// set to 64 bytes
	Serial.begin(9600);
	Serial.println("Ready");

}

void loop() {
	
	// if message to update is received
	//if(){
	// this is where message is read
	//ReadDemBytes( (unsigned char*)&message, sizeof( struct EEPROM_UPDATES ) ); // calls read function to get message decode and compute checksum
	//}
	if(Serial.available() >= 10){
	Serial.println("about to Read Dem Bytes");
	ReadDemBytes( (unsigned char*)&message, sizeof( struct EEPROM_UPDATES ) ); // calls read function to get message decode and compute checksum
	
	uint8_t ACTN = message.action;
	Serial.println(ACTN);
	Serial.println("message read");
	//if(ACTN == 1){ // Write to EEPROM
	uint32_t MSB = message.value3;
	Serial.println(MSB);
	uint32_t MID = message.value2;
	Serial.println(MID);
	uint32_t LSB = message.value1;
	Serial.println(LSB);
		/*if (MSB = 0xffff){
			int a = 1;
			Serial.write(a);
		}
		if (MSB = 0xaaaa){
			int b = 1;
			Serial.write(b);
		}
		if (MSB = 0x1212){
			int c = 1;
			Serial.write(c);
		}
		*/
		uint8_t VALS[12]; // use if we need uint8_t numbers in an array instead of one uint128_t number
		
		//uint128_t VALS = ((MSB << 64) | (MID << 32)) | LSB;
		//uint8_t msb[4];
		//uint8_t mid[4];
		//uint8_t lsb[4];
		
		//memcpy(msb, &MSB, sizeof(MSB));
		//memcpy(mid, &MID, sizeof(MID));
		//memcpy(lsb, &LSB, sizeof(LSB));
		//Serial1.println(MSB);
		//Serial1.println(MID);
		//Serial1.println(LSB);
		memcpy(VALS, &MSB, sizeof(MSB));
		memcpy(VALS+sizeof(MSB),&MID,sizeof(MID));
		memcpy(VALS+sizeof(MSB)+sizeof(MID),&LSB,sizeof(LSB));
		
		//VALS =  // this is where value1,2,&3  are stored in array
		
		//eeprom_update_block( const void * VALS, void * 0, 96);  //eeprom_update_block( const void * __src, void * __dst, #bytes) //may be byte by byte so if one byte wears out the ones around it are unaffected
		for (int i=0; i<sizeof(VALS) ;++i){
			//printf("0x%02X\n",VALS[i]);
			Serial.print(VALS[i]);
			//Serial1.println(1);
			//printf(VALS[i]); //"%u",
			
		}
	//}
	}
	//else{			// other if code needs to execute only when not reading or writing place here
	
	//}
	delay(1000);
}
