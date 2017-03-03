from dacutils.messagedefinition import FieldDefinition

MESSAGE_IDS = {
    "EEPROM_UPDATES": 1000  # Set to correct value later
}


def get_eeprom_updates_format():
    """
    Creates and returns a new eeprom update format - maintain memory independence of the format
    """

    update_format = [
        FieldDefinition("action", "desired action for peripheral PCB, write to EEPROM", "uchar", 1, 0),
        FieldDefinition("value1", "value for the 1st 4 bytes/ 1st & 2nd EEPROM variables", "ulong", 1, 32),
        FieldDefinition("value2", "value for the 2nd 4 bytes/ 3rd & 4th EEPROM variables", "ulong", 1, 32),
        FieldDefinition("value3", "value for the 3rd 4 bytes/ 5th & 6th EEPROM variables", "ulong", 1, 32),
        FieldDefinition("spare1", "spare, occasionally used as a unique index for single sends", "uchar", 1, 0),
        FieldDefinition("align", "byte alignment", "uchar", 1, 0),
        FieldDefinition("align2", "byte alignment", "uchar", 1, 0)
    ]
    return update_format
