#ifndef PN532_COMMANDS_H_
#define PN532_COMMANDS_H_

//message frame specifications
#define PREAMBLE          0x00U
#define POSTAMBLE         0x00U
#define START_CODE_1      0x00U
#define START_CODE_2      0xFFU
#define PN532_FROM_UC_TFI 0xD4U
#define PN532_TO_UC_TFI   0XD5U

//pn532 commands
#define PN532_GET_FIRMWARE_VERSION_COMMAND 0x02U
#define PN532_SAM_CONFIGURATION_COMMAND    0x14U

//mifare commands

#endif