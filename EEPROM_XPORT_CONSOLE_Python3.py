#Terminal for sending EEPROM varible updates to Drone Aviation PCBs w/ microcontroller & Xport module
#Written by Jakob A. Howe 2/24/17 - Massive credit to Joel Dunham A.K.A. the Code Wizard

import serial # may decide to switch to Joels serial connection files in dacutils 1.0 conection
import sys
from dacutils import messagedefinition  #from dacutils import 
from dacutils import messages_V3
from dacutils import checksum
#import ctypes

# pyserial
from struct import *
comportnum = input('enter comport number: ')
comport = 'com'+comportnum
ser = serial.Serial(comport)
baudrate = input('enter baud rate: ')
ser.baudrate = baudrate

class EEPROM_UPDATES(object):
    def __init__(self, sync1, sync2, sync3, spare, messageID, messageSize, hcsum, csum,
                 action, value1, value2, value3, spare1, spare2, align, align2):
        self.sync1 = sync1
        self.sync2 = sync2
        self.sync3 = sync3
        self.spare = spare
        self.messageID = messageID
        self.messageSize = messageSize
        self.hcsum = hcsum
        self.csum = csum
        self.action = action
        self.value1 = value1
        self.value1 = value1
        self.value1 = value1
        self.spare1 = spare1
        self.spare2 = spare2
        self.align = align
        self.align2 = align2


def main():

    #EEPROM_UPDATES = 

    #for sending messages
    header = messagedefinition.HeaderDefinition(messages_V3.get_header_format(), checksum.compute_check_sum) #EEPROM_UPDATES.
    
    #message = EEPROM_UPDATES
    MESSAGE_ID_EEPROM_VARIBLES = 500
    messageID = MESSAGE_ID_EEPROM_VARIBLES #message.messageID = MESSAGE_ID_EEPROM_VARIBLES
    EEPROM_UPDATES.sync1 = 0xa3 #message.DATALINK_SYNC0 = 0xa3
    EEPROM_UPDATES.sync2 = 0xb2 #message.DATALINK_SYNC1 = 0xb2
    EEPROM_UPDATES.sync3 = 0xc1 #message.DATALINK_SYNC2 = 0xc1
    EEPROM_UPDATES.action = 0x1
    EEPROM_UPDATES.value1 = int(input("Please input value1, in hex w/o 0x prefix: "), 32) #add ),16) if not using 0x before hex number #was message.value1
    print (EEPROM_UPDATES.value1)
    EEPROM_UPDATES.value2 = int(input("Please input value2, in hex w/o 0x prefix: "), 32)           #was message.value2
    print (EEPROM_UPDATES.value2)
    EEPROM_UPDATES.value3 = int(input("Please input value3, in hex w/o 0x prefix: "), 32)   # was message.value3
    print (EEPROM_UPDATES.value3)

    EEPROM_UPDATES_message = messagedefinition.MessageDefinition("EEPROM_UPDATES", "EEPROM_UPDATES_MESSAGE", messageID,
                                                          messages_V3.get_EEPROM_UPDATES_format(), header, False )

    #checkSumEncode( message, sys.getsizeof(EEPROM_UPDATES) )
    encoded_message = EEPROM_UPDATES.encode()
    ser.write(encoded_message)
    
    s = 'message sent'
    print (s)

    #i = 20

  
    #while (i >= 12): # >= 12 # 12 is the number of bytes sub with expected number of bytes
    
    s = ser.read(12)
        #i = ser.in_waiting()
    #ser.flushInput()

    print (s) 


if __name__ == "__main__":
    main()
