#include "PN532.h"

static uint8_t ACK_arr[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

void pn532_SendWakeUpCommand(USART_DEVICE * uart_dev){
    const uint8_t wakeup[5] = {0x55, 0x55, 0x00, 0x00, 0x00};
    uart_dev->Write(wakeup, 5);
}

//lcs -> the number of data bytes (inverse) checksum
//cs  -> checkshum,   is calculated as the inverse sum of every data byte including TFI
//packets are build with the following structure: 
// {0x00, 0x00, 0xFF, DATA_LENGTH, DATA_LENGTH_CHECKSUM, FRAMI_ID, [DATA ARRAY], DATA_CHECKSUM. 0x00}
//where DATA_LENGTH_CHECKSUM = ~(DATA_LENGTH) + 1
//DATA_CHECKSUM = ~(TFI + DATA[0] + DATA[1] + ... + DATA[n]) + 1
uint8_t * pn532_GetFirmwareVersionCommand(USART_DEVICE * uart_dev){
    uint8_t lcs = ~0x02 + 1;
    uint8_t cs  = ~(PN532_FROM_UC_TFI + PN532_GET_FIRMWARE_VERSION_COMMAND ) + 1;

    uint8_t packet[9] = { PREAMBLE, START_CODE_1, START_CODE_2, 0x02, lcs, PN532_FROM_UC_TFI,
			PN532_GET_FIRMWARE_VERSION_COMMAND , cs, POSTAMBLE
	};

    uint8_t data[100];
    static uint8_t res[2] = {0,0};
    uart_dev->Write(packet, 9);
    uart_dev->Read(data);
    //check for ACK
    uint8_t checkACK = pn532_checkACK(data);

    if(checkACK > 0){
        //these two bytes contains the two numbers of the version of the PN532
        res[0] = data[14];
        res[1] = data[15];
    }   
    return res;
}


//No timeout and no IRQ management for the SAM configuration
uint8_t pn532_SendSAMConfiguration(USART_DEVICE * uart_dev){
    uint8_t lcs = ~(0x05) + 1;
    uint8_t cs = ~(PN532_FROM_UC_TFI + 0x14 + 0x01 + 0x00 + 0x00) + 1;

    uint8_t packet[12] = { PREAMBLE, START_CODE_1, START_CODE_2, 0x05, lcs, PN532_FROM_UC_TFI,
				PN532_SAM_CONFIGURATION_COMMAND, 0x01, 0x00, 0x00, cs, POSTAMBLE
	};

    uint8_t data[100];
    uint32_t length = 0;
    uart_dev->Write(packet, 12);
    
    //wait for answer from NFC read
    length = uart_dev->Read(data);
   
    //check for ACK
    uint8_t checkACK = pn532_checkACK(data);
    if(checkACK <= 0) return 0;  

    return 1;
}

//Hardcode max retries rf configuration
uint8_t pn532_ConfigRF(USART_DEVICE * uart_dev){
    uint8_t lcs = ~(0x06) + 1;
    uint8_t cs  = ~(PN532_FROM_UC_TFI + PN532_RF_CONFIGURATION_COMMAND + PN532_RF_CONF_CFGITEM_MAX_RETRIES
                    + 0xFF + 0xFF + 0x02) + 1;

    uint8_t packet[13] = {  PREAMBLE, START_CODE_1, START_CODE_2, 0x06, lcs, PN532_FROM_UC_TFI, 
                            PN532_RF_CONFIGURATION_COMMAND, PN532_RF_CONF_CFGITEM_MAX_RETRIES, 
                            0xFF, 0xFF, 0x02, cs, POSTAMBLE
                        };

    uart_dev->Write(packet, 13);

    return 0;
}


uint8_t pn532_DetectCard(USART_DEVICE * uart_dev, uint8_t max_cards){
    uint8_t lcs = ~(0x04) + 1;
    uint8_t cs = ~(PN532_FROM_UC_TFI + PN532_IN_LIST_PASSIVE_TARGET_COMMAND + max_cards + 0x00) + 1;

    //this is hardcoded for 106 kbps type A (ISO/IEC14443 TypeA)
    uint8_t packet[11] = {PREAMBLE, START_CODE_1, START_CODE_2, 0x04, lcs, PN532_FROM_UC_TFI, 
                           PN532_IN_LIST_PASSIVE_TARGET_COMMAND, max_cards, 0x00, cs, POSTAMBLE
                         };
    uart_dev->Write(packet, 11);
    return 0;
}


static uint8_t pn532_checkACK(uint8_t * data){
    //6 because the first 6 bytes are the ACK code
    for(int i = 0; i < 6; i++){
        if(data[i] != ACK_arr[i]) return 0;
    }

    return 1;
}