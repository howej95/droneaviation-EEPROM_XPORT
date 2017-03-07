#Terminal for sending EEPROM varible updates to Drone Aviation PCBs w/ microcontroller & Xport module
#Written by Jakob A. Howe 2/24/17 - Massive credit to Joel Dunham A.K.A. the Code Wizard

import serial # may decide to switch to Joels serial connection files in dacutils 1.0 conection
import sys
from dacutils.messagedefinition import MessageDefinition, HeaderDefinition
from dacutils import messages
import messages_V4
from dacutils import checksum
from time import sleep
import struct
#from dacutils.connection import SerialConnector

from struct import *
comportnum = input('enter comport number: ')
comport = 'com'+comportnum
ser = serial.Serial(comport)
baudrate = input('enter baud rate: ')

ser.baudrate = baudrate 


def main():
    sleep(2)
    # Define the message to send
    header = HeaderDefinition(messages.get_header_format(), checksum.compute_check_sum)
    eeprom_updates_message = MessageDefinition("eeprom_updates", "Message to update the EEPROM",
                                               0, messages_V4.get_eeprom_updates_format(), header,
                                               False, messages_V4.MESSAGE_IDS["EEPROM_UPDATES"]) 

    # Set all values
    eeprom_updates_message.mapped["action"].values = 1
    eeprom_updates_message.mapped["data"].values[0] = 4294967295#= 0xffffffff, 0xffff = 65535, 0xff = 255
    print(eeprom_updates_message.mapped["data"].values[0])      #
    eeprom_updates_message.mapped["data"].values[1] = 2863311530#= 0xaaaaaaaa, 0xaaaa = 43690, 0xaa = 170
    print(eeprom_updates_message.mapped["data"].values[1])      #
    eeprom_updates_message.mapped["data"].values[2] = 3149642683#= 0xbbbbbbbb, 0xbbbb = 48059, 0xbb = 187
    print(eeprom_updates_message.mapped["data"].values[2])      #
    eeprom_updates_message.mapped["data"].values[3] = 3435973836#= 0xcccccccc, 0xcccc = 52428, 0xcc = 204
    print(eeprom_updates_message.mapped["data"].values[3])      #
    eeprom_updates_message.mapped["data"].values[4] = 3722304989#= 0xdddddddd, 0xdddd = 56797, 0xdd = 221
    print(eeprom_updates_message.mapped["data"].values[4])      #

    #int(input("Please input value1, in hex w/o 0x prefix: "), 32))

    # Encode and send
    encoded_message = eeprom_updates_message.encode()
    ser.write(encoded_message)
    s = 'message sent'
    print (s)

    #i = 20

  
    #while (i >= 12): # >= 12 # 12 is the number of bytes sub with expected number of bytes
    sleep(2)
    #r = ser.read(100)
   

    #print (r)

    ser.close()


if __name__ == "__main__":
    main()
