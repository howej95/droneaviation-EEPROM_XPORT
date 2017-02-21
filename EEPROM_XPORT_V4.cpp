#include <Arduino.h>
#include <EEPROM.h>
#define MESSAGE_ID_EEPROM_VARIBLES 500
#define DATALINK_SYNC0 0xa3
#define DATALINK_SYNC1 0xb2
#define DATALINK_SYNC2 0xc1

//#include <string>

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
	uint32_t value1:												//4 byte  // only want 10 variables 20 byte max used
	uint32_t value2;       // New value of variable if writing		//4 bytes	
	uint32_t value3;												//4 byte
	uint8_t spare;													//1 byte
	uint8_t spare;													//1 byte
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
void ReadDemBytes( uint8_t *buf, int32_t byteCount ) {  //for storing serial data storing serial data in a struct 
	struct messageHeader *h = (struct messageHeader *)buf;
	uint32_t mcsum = 0;
	h->hcsum = checkSumCompute(  buf, sizeof( struct messageHeader ) - sizeof( int32_t )*2 );
	Serial.readBytes(buf, sizeof( struct messageHeader ) - sizeof( int32_t )*2 );
	mcsum = checkSumCompute( buf, sizeof( struct messageHeader ) - sizeof( int32_t )*2 );
	if( hcsum == mcsum ){
		Serial.readBytes(buf, sizeof( byteCount ) );
	}
	buf = 0;
}

char c;
int action;
int variable;
int value;
int VARS;  //holds the value of the variable to read or write too
long VALS; //holds the value to write to the variable 
int ACTN;  // holds the value that represents the desired action 
unsigned long COMM_START = 0;      // The time that the last status message was sent
unsigned long COMM_CURRENT = 0;    // The current time of the system

void setup() {
	// put your setup code here, to run once:
	message.messageID = MESSAGE_ID_EEPROM_VARIBLES;
	Serial1.begin(9600);

}

void loop() {
	
	// if message to update is received 
	if(){
		// this is where message is read and check sum is computed 
		//Serial.read((unsigned char*)&message, sizeof( struct EEPROM_UPDATES ) );      // improper synyax serial dosnt have parameters need to read and store in buffer 1 byte a time 
		ReadDemBytes( (unsigned char*)&message, sizeof( struct EEPROM_UPDATES ) );
		//checkSumCompute( (unsigned char*)&message, sizeof( struct EEPROM_UPDATES ) );
		
		
	}
	
	
	
	
	ACTN = message.action;
	VARS = message.variable; 
	VALS = message.value;
	
	if(ACTN == 0){ //read from EEPROM
						// idk if this functionality is needed 
	}
	else if(ACTN == 1){ // Write to EEPROM 
		EEPROM.update(VARS, VALS);		// need to include something that reads VALS to see if its larger than 1 byte if so it needs to be split into multiple 
										// EEPROM addresses, max value of 255 (byte) 
										
		
	}
	else{			// other if code needs to execute only when not reading or writing place here 
		
	}
	
	checkSumEncode( (unsigned char*)&message, sizeof( struct monitoringboardstatus) );
}
