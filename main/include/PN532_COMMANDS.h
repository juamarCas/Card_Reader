#ifndef PN532_COMMANDS_H_
#define PN532_COMMANDS_H_

/**
 * @author Juan D. Martín
 * @details Commands set for the PN532, this command set can be found in the PN532 user manual
 * @note Add more commands and options in the future if needed
*/

//message frame specifications
#define PREAMBLE          0x00U
#define POSTAMBLE         0x00U
#define START_CODE_1      0x00U
#define START_CODE_2      0xFFU
#define PN532_FROM_UC_TFI 0xD4U
#define PN532_TO_UC_TFI   0XD5U

//pn532 commands
#define PN532_GET_FIRMWARE_VERSION_COMMAND   0x02U
#define PN532_SAM_CONFIGURATION_COMMAND      0x14U
#define PN532_RF_CONFIGURATION_COMMAND       0x32U
#define PN532_IN_LIST_PASSIVE_TARGET_COMMAND 0x4AU

//pn532 rf configuration params
#define PN532_RF_CONF_CFGITEM_RF_FIELD    0x01U
#define PN532_RF_CONF_CFGITEM_MAX_RETRIES 0x05U
#define PN532_RF_CONF_RFCAOFF_RFON        0x01U
#define PN532_RF_CONF_RFCAOFF_RFOFF       0x00U
#define PN532_RF_CONF_INF_TRIES           0xFFU
#define PN532_RF_CONF_TWO_TRIES           0x02U

//mifare commands

#endif