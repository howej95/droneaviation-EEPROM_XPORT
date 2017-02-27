#Terminal for sending EEPROM varible updates to Drone Aviation PCBs w/ microcontroller & Xport module
#Written by Jakob A. Howe 2/24/17 - Massive credit to Joel Dunham A.K.A. the Code Wizard

import serial # may decide to switch to Joels serial connection files in dacutils 1.0 conection
import sys
from dacutils.messagedefinition import MessageDefinition, HeaderDefinition
from dacutils import messages
import messages_V3
from dacutils import checksum

from struct import *
comportnum = input('enter comport number: ')
comport = 'com'+comportnum
ser = serial.Serial(comport)
baudrate = input('enter baud rate: ')
ser.baudrate = baudrate


def main():
    # Define the message to send
    header = HeaderDefinition(messages_V3.get_header_format(), checksum.compute_check_sum)
    eeprom_updates_message = MessageDefinition("eeprom_updates", "Message to update the EEPROM",
                                               0, messages_V3.get_eeprom_updates_format(), header,
                                               False, messages_V3.MESSAGE_IDS["EEPROM_UPDATES"])

    # Set all values
    eeprom_updates_message.mapped["action"].values = 0x1
    eeprom_updates_message.mapped["value1"].values =\
        int(input("Please input value1, in hex w/o 0x prefix: "), 32)
    print(eeprom_updates_message.mapped["value1"].values)
    eeprom_updates_message.mapped["value2"].values = int(input("Please input value2, in hex w/o 0x prefix: "), 32)
    print(eeprom_updates_message.mapped["value2"].values)
    eeprom_updates_message.mapped["value3"].values = int(input("Please input value3, in hex w/o 0x prefix: "), 32)
    print(eeprom_updates_message.mapped["value3"].values)

    # Encode and send
    encoded_message = eeprom_updates_message.encode()
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
