/**
* @author Juan D. Martín
* @details Abstract methods to send commands to the PN532 NFC reader
*/
#include "PN532.h"

static const uint8_t ACK_arr[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
 /**
 * @note by now i have cards with 4 bytes UID, this is hardcoded, change it in a future
 */
static uint8_t UID_arr[4] = {0,0,0,0};


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

    uint8_t data[50];
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

    return 1;
}


uint8_t pn532_DetectCard(USART_DEVICE * uart_dev, uint8_t max_cards){
    const uint8_t LENGTH = 0x04;
    uint8_t lcs = ~(LENGTH) + 1;
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
        //saves UID, the len of the UID is in the 12th position
        uint8_t UID_Len = card_info[12];
        for(int i = 0; i < UID_Len; i++){
            UID_arr[i] = card_info[13 + i];
        }
        
        if(card_info[7] >= 0x01U) return 1;
    }
    return 0;
}

uint8_t pn532_mifare_write_16(USART_DEVICE * usart_dev, uint8_t * data, uint8_t sector){
    return 1;
}

uint8_t pn532_mifare_authenticate_key_a(USART_DEVICE * uart_dev, uint8_t block, uint8_t * key_a, uint8_t * uid){
    return 1;

}

//this is up here in this file, but to remember
//lcs -> the number of data bytes (inverse) checksum
//cs  -> checkshum,   is calculated as the inverse sum of every data byte including TFI
//packets are build with the following structure: 
// {0x00, 0x00, 0xFF, DATA_LENGTH, DATA_LENGTH_CHECKSUM, TFI, [DATA ARRAY], DATA_CHECKSUM. 0x00}
//where len_cs = ~(DATA_LENGTH) + 1
//the data length is the sum of the TFI + length(DATA ARRAY)
//data_cs = ~(TFI + COMMAND +  DATA[0] + DATA[1] + ... + DATA[n]) + 1


static void pn532_SendCommand(USART_DEVICE * uart_dev, uint8_t command, uint8_t * data, uint8_t data_len){
    uint8_t packet[64] = {'\0'};
    //data_len + 2, that plus one is adding the TFI and command that count as data packet
    uint8_t total_data_len = data_len + 2;
    uint8_t len_cs   = ~(total_data_len) + 1;
    uint8_t data_sum = command + PN532_FROM_UC_TFI;
    uint8_t count = 0;

    //9 fixed data + user data
    uint8_t total_packet_len = 9 + data_len;
    packet[0] = PREAMBLE;
    packet[1] = START_CODE_1;
    packet[2] = START_CODE_2;
    packet[3] = total_data_len;
    packet[4] = len_cs;
    packet[5] = PN532_FROM_UC_TFI;
    packet[6] = command;
    for( uint8_t i = 0; i < data_len; i++){
        data_sum += data[i];
        packet[7 + i] = data[i];
    }

    uint8_t data_cs = ~(data_sum) + 1;
    packet[7 + data_len] = data_cs;
    packet[7 + data_len + 1] = POSTAMBLE;

    uart_dev->Write(packet, total_packet_len);
}

//remember, this only returns the UID of the last readed card, so call it only after read a card
uint8_t * pn532_GetUID(){
    return UID_arr;
}


static uint8_t pn532_checkACK(uint8_t * data){
    //6 because the first 6 bytes are the ACK code
    for(int i = 0; i < 6; i++){
        if(data[i] != ACK_arr[i]) return 0;
    }

    return 1;
}
