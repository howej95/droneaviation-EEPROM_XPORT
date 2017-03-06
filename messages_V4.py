from dacutils.messagedefinition import FieldDefinition

MESSAGE_IDS = {
    "EEPROM_UPDATES": 1000  # Set to correct value later
}


def get_eeprom_updates_format():
    """
    Creates and returns a new eeprom update format - maintain memory independence of the format
    """

    update_format = [
        FieldDefinition("data", "value for the 1st 4 bytes/ 1st & 2nd EEPROM variables", "uint", 5, [0,0,0,0,0]),
        FieldDefinition("action", "desired action (0 = read, 1 = write, 2 = read responce, 3 = write responce)", "uchar", 1, 0),
        FieldDefinition("errorCode", "0 = no error, 1 = out of range, 2 = invalid data, 3 = write not available", "uchar", 1, 0),
        FieldDefinition("offset", "starting EEPROM offset", "ushort", 1, 0),
        FieldDefinition("size", "EEPROM access size", "ushort", 1, 0),
        FieldDefinition("align", "byte alignment", "ushort", 1, 0)
    ]
    return update_format
