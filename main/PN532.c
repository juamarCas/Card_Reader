#include "PN532.h"

static const uint8_t ACK_arr[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

void pn532_SendWakeUpCommand(USART_DEVICE * uart_dev){
    const uint8_t wakeup[5] = {0x55, 0x55, 0x00, 0x00, 0x00};
    uart_dev->Write(wakeup, 5);
}

//lcs -> the number of data bytes (inverse) checksum
//cs  -> checkshum,   is calculated as the inverse sum of every data byte including TFI
//packets are build with the following structure: 
// {0x00, 0x00, 0xFF, DATA_LENGTH, DATA_LENGTH_CHECKSUM, TFI, [DATA ARRAY], DATA_CHECKSUM. 0x00}
//where DATA_LENGTH_CHECKSUM = ~(DATA_LENGTH) + 1
//the data length is the sum of the TFI + length(DATA ARRAY)
//DATA_CHECKSUM = ~(TFI + DATA[0] + DATA[1] + ... + DATA[n]) + 1
uint8_t * pn532_GetFirmwareVersionCommand(USART_DEVICE * uart_dev){
    uint8_t LENGTH = 0x02;
    uint8_t lcs = ~LENGTH + 1;
    uint8_t cs  = ~(PN532_FROM_UC_TFI + PN532_GET_FIRMWARE_VERSION_COMMAND ) + 1;

    uint8_t packet[9] = { 
                            PREAMBLE, START_CODE_1, START_CODE_2, LENGTH, lcs, PN532_FROM_UC_TFI,
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
    uint8_t LENGTH = 0x05;
    uint8_t lcs = ~(LENGTH) + 1;
    uint8_t cs = ~(PN532_FROM_UC_TFI + PN532_SAM_CONFIGURATION_COMMAND + 0x01 + 0x00 + 0x00) + 1;

    uint8_t packet[12] = { 
                            PREAMBLE, START_CODE_1, START_CODE_2, LENGTH, lcs, PN532_FROM_UC_TFI,
				            PN532_SAM_CONFIGURATION_COMMAND, 0x01, 0x00, 0x00, cs, POSTAMBLE
	                     };

    uint8_t data[100];
    uint32_t length = 0;

    if(length < 0){
        //reading error, restart the system
        return 0;
    }

    uart_dev->Write(packet, 12);
    
    //wait for answer from NFC read
    length = uart_dev->Read(data);
   
    //check for ACK
    uint8_t checkACK = pn532_checkACK(data);
    if(checkACK <= 0) return 0;  

    return 1;
}

//Hardcode for two retries :)
//NOTE: eh, this could cause a problem in the future, but let stay with it for now :)
uint8_t pn532_ConfigRF(USART_DEVICE * uart_dev, uint8_t retries){
    uint8_t MAX_RETRIES = retries;
    const uint8_t LENGTH = 0x06;
    uint8_t lcs = ~(LENGTH) + 1;
    uint8_t cs  = ~(PN532_FROM_UC_TFI + PN532_RF_CONFIGURATION_COMMAND + PN532_RF_CONF_CFGITEM_MAX_RETRIES
                    + 0xFF + 0xFF + MAX_RETRIES) + 1;

    uint8_t packet[13] = {  
                            PREAMBLE, START_CODE_1, START_CODE_2, LENGTH, lcs, PN532_FROM_UC_TFI, 
                            PN532_RF_CONFIGURATION_COMMAND, PN532_RF_CONF_CFGITEM_MAX_RETRIES, 
                            0xFF, 0xFF, MAX_RETRIES
                            ,cs, POSTAMBLE
                         };

    uart_dev->Write(packet, 13);

    uint8_t data[100];
    uint32_t length = 0;
    length = uart_dev->Read(data);
    if(length < 0){
        //reading error, restart the system
        return 0;
    }

    return 0;
}


uint8_t pn532_DetectCard(USART_DEVICE * uart_dev, uint8_t max_cards){
    const uint8_t LENGTH = 0x04;
    uint8_t lcs = ~(LENGTH) + 1;
    static uint8_t errArr[2] = {0x95, 0x95};
    uint8_t cs = ~(PN532_FROM_UC_TFI + PN532_IN_LIST_PASSIVE_TARGET_COMMAND + max_cards + 0x00) + 1;

    //this is hardcoded for 106 kbps type A (ISO/IEC14443 TypeA)
    /**
     * @todo add more parameters to the inListPassiveTarget command, 
     * for now it only takes iso 14443-A iso
    */
    const uint8_t type_a_iso_option = 0x00;
    uint8_t packet[11] = {
                           PREAMBLE, START_CODE_1, START_CODE_2, LENGTH, lcs, PN532_FROM_UC_TFI, 
                           PN532_IN_LIST_PASSIVE_TARGET_COMMAND, max_cards, type_a_iso_option, cs, POSTAMBLE
                         };

    uint8_t ack[20];
    uint32_t length = 0;
    uart_dev->Write(packet, 11);

    length = uart_dev->Read(ack);


    if(length < 0){
        //reading error, restart the system
        return 0;
    }

    //this command takes some time to make a response, so it first
    //receives ACK and then receives response
    if(length <= 6){

        if(pn532_checkACK(ack) <= 0){
            return 0;
        }

        static uint8_t card_info[100];
        uint32_t card_info_length = 0;
        

        card_info_length = uart_dev->Read(card_info);
        if(card_info_length < 0){
            //reading error, restart the system
            return 0;
        }

        return card_info;
    }
    return 0;
}

uint8_t * pn532_GetUID(USART_DEVICE * uart_dev){
    static uint8_t uid_7b[7] = {0}; 
    return uid_7b;
}

static uint8_t pn532_checkACK(uint8_t * data){
    //6 because the first 6 bytes are the ACK code
    for(int i = 0; i < 6; i++){
        if(data[i] != ACK_arr[i]) return 0;
    }

    return 1;
}
