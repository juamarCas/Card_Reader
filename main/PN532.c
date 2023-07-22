#include "PN532.h"

static uint8_t ACK_arr[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

void pn532_SendWakeUpCommand(USART_DEVICE * uart_dev){
    const uint8_t wakeup[5] = {0x55, 0x55, 0x00, 0x00, 0x00};
    uart_dev->Write(wakeup, 5);
}

//lcs -> the number of data bytes (inverse)
//cs  -> checkshum,   is calculated as the inverse sum of every data byte including TFI
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
    uint8_t checkACK = pnf322_checkACK(data);

    if(checkACK > 0){
        //these two bytes contains the two numbers of the version of the NFC
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
				0x14, 0x01, 0x00, 0x00, cs, POSTAMBLE
	};

    uint8_t data[100];
    uint32_t length = 0;
    uart_dev->Write(packet, 12);
    
    //wait for answer from NFC read
    length = uart_dev->Read(data);
   
    //check for ACK
    uint8_t checkACK = pnf322_checkACK(data);
    if(checkACK <= 0) return 0;  

    return 1;
}


static uint8_t pnf322_checkACK(uint8_t * data){
    //6 because the first 6 bytes are the ACK code
    for(int i = 0; i < 6; i++){
        if(data[i] != ACK_arr[i]) return 0;
    }

    return 1;
}