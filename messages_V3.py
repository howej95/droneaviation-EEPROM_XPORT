from dacutils.messagedefinition import FieldDefinition

MESSAGE_ID_NAME = "messageID"
MESSAGE_IDS = {
    "status": 500,  # GPIU to GCS
}


def get_header_format():
    """
    Creates and returns a new header format - used for each message definition to maintain memory independence
     between the message headers
    """
    header_format = [
        # An ordered list of the fields in the header
        # The header precedes every subsequent message defined here
        FieldDefinition("sync1", "Sync byte 1", "uchar", 1, 0xa3),
        FieldDefinition("sync2", "Sync byte 2", "uchar", 1, 0xb2),
        FieldDefinition("sync3", "Sync byte 3", "uchar", 1, 0xc1),
        FieldDefinition("spare", "spare, occasionally used as a unique index for single sends", "uchar", 1, 0),
        FieldDefinition("messageID", "Message identifier", "int", 1, 0),
        FieldDefinition("messageSize", "Message size, including the header", "int", 1, 0),
        FieldDefinition("hcsum", "Header checksum", "uint", 1, 0),
        FieldDefinition("csum", "checksum", "uint", 1, 0)
    ]
    return header_format


def get_EEPROM_UPDATES_format():
    """
    Creates and returns a new status format - maintain memory independence of the format
    """

    status_format = [
        FieldDefinition("action", "desired action for peripheral PCB, write to EEPROM", "uchar", 1, 0),
        FieldDefinition("value1", "value for the 1st 4 bytes/ 1st & 2nd EEPROM variables", "uchar", 1, 0),
        FieldDefinition("value2", "value for the 2nd 4 bytes/ 3rd & 4th EEPROM variables", "uchar", 1, 0),
        FieldDefinition("value2", "value for the 3rd 4 bytes/ 5th & 6th EEPROM variables", "uchar", 1, 0),
        FieldDefinition("spare", "spare, occasionally used as a unique index for single sends", "uchar", 1, 0),
        FieldDefinition("spare", "spare, occasionally used as a unique index for single sends", "uchar", 1, 0),
        FieldDefinition("align", "byte alignment", "uchar", 1, 0),
        FieldDefinition("align2", "byte alignment", "uchar", 1, 0)
    ]
    return status_format
