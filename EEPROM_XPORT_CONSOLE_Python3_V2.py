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
    
    eeprom_updates_message.mapped["size"].values = 48 # size of EEPROM_UPDATES structure

    #int(input("Please input value1, in hex w/o 0x prefix: "), 32))

    # Encode and send
    encoded_message = eeprom_updates_message.encode()
    ser.write(encoded_message)
    s = 'message sent'
    print (s)

    sleep(2)

    # Read and Decode

    #received = bytearray(ser.read(150)) # adjust the number of bytes read later
    # For fragmented messages
    messages1 = None

        # Run this thread forever
    LOOP_GUARD = True
    while LOOP_GUARD:
                # Get the next message from the connection - only one message
                #header = HeaderDefinition(gust_message.get_header_format(), compute_check_sum)

                # Get the next message from the connection - only one message
            received = bytearray(ser.read(300))

            if messages1 is None:
                    messages1 = received
            else:
                    # Append new bytes
                    messages1.extend(received)

            while (messages1 is not None) and (len(messages1) > 0):
                    sync_index = -1
                    found_message_start = False
                    while (sync_index < (len(messages1) - 3)) and (found_message_start is False):
                            # Start with index to enable clean exit without break inside if
                            sync_index += 1  # Check the next location
                            if (messages1[sync_index] == header.mapped["sync1"].values) and\
                                            (messages1[sync_index + 1] == header.mapped["sync2"].values) and\
                                            (messages1[sync_index + 2] == header.mapped["sync3"].values):
                                    found_message_start = True  # Found the start of the message

                    if found_message_start is True:
                            if (len(messages1) - sync_index) >= header.size():
                                    # Enough to decode the header

                                    # Decode the header and check checksum
                                    decoded, bytes_read = header.decode(messages1[sync_index:])

                                    # If we decoded it successfully, read and decode the rest of the message
                                    if decoded is not False:
                                            # Ensure enough is available to decode the full message
                                            if (len(messages1) - sync_index) >= header.mapped["messageSize"].values:
                                                    checksum_verified = False
                                                    if header.mapped[header.msg_id_field_name].values == messages_V4.MESSAGE_IDS["EEPROM_UPDATES"]:
                                                            # Create the message to which to unpack the data
                                                            #message = MessageDefinition("message0", "status message 0",
                                                            #defined earlier                            #0, gust_message.get_message0_format(), header, False)
                                                            message = eeprom_updates_message

                                                            # Decode the message
                                                            checksum_verified, bytes_read = message.decode(messages1[sync_index:], bytes_read)

                                                            if checksum_verified:
                                                                 s = 'Message Read'
                                                                 print (s)
                                                                 print(eeprom_updates_message.mapped["data"].values[0])
                                                                 print(eeprom_updates_message.mapped["data"].values[1])
                                                                 print(eeprom_updates_message.mapped["data"].values[2])
                                                                 print(eeprom_updates_message.mapped["data"].values[3])
                                                                 print(eeprom_updates_message.mapped["data"].values[4])
                                                                 s = 'responce'
                                                                 print (s)
                                                                 print(eeprom_updates_message.mapped["action"].values)
                                                                 s = 'ErrorCode'
                                                                 print (s)
                                                                 print(eeprom_updates_message.mapped["errorCode"].values)
                                                                 ser.close()
                                                                 LOOP_GUARD = False
                                                            else:
                                                                receive_debug="Received EEPROM_UPDATES responce which failed checksum verification"
                                                                print (receive_debug)
                                                        
                                                    if checksum_verified:
                                                            read_msg = messages1[:sync_index + bytes_read]
                                                            messages1 = messages1[bytes_read + sync_index:]
                                    else:
                                        # Bad header - throw it away
                                        receive_debug="Received header which failed checksum verification"
                                        print (receive_debug)
                                        messages1 = messages1[bytes_read:]
                            else:
                                # Message start found, but not whole message - throw the useless part away
                                messages1 = messages1[sync_index:]
                                if sync_index > 0:
                                    receive_debug="Threw away partial message before sync bytes found"
                                    print (receive_debug)
                                break  # Must retrieve more
                    else:
                        # Message start not found - throw it away
                        messages1 = None
                        if sync_index > 0:
                            receive_debug="Threw away entire message - no sync bytes found"
                            print (receive_debug)
                        # Will break out on next while check
    # no sleep is needed here because reading from the connection will
    # simply block until data is available

    s = 'HIT RESET!'
    print (s)


if __name__ == "__main__":
    main()
