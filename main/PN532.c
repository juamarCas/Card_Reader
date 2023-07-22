#include "PN532.h"

void pn532_SendWakeUpCommand(USART_DEVICE * uart_dev){
    const uint8_t wakeup[5] = {0x55, 0x55, 0x00, 0x00, 0x00};
    uart_dev->Write(wakeup, 5);
}

//lcs -> the number of data bytes (inverse)
//cs  -> checkshum,   is calculated as the inverse sum of every data byte including TFI
void pn532_GetFirmwareVersionCommand(USART_DEVICE * uart_dev){
    uint8_t lcs = ~0x02 + 1;
    uint8_t cs  = ~(PN532_FROM_UC_TFI + PN532_GET_FIRMWARE_VERSION_COMMAND ) + 1;

    uint8_t packet[9] = { 0x00, 0x00, 0xFF, 0x02, lcs, PN532_FROM_UC_TFI,
			PN532_GET_FIRMWARE_VERSION_COMMAND , cs, 0x00
	};

    uart_dev->Write(packet, 9);
}


uint8_t pn532_SendSAMConfiguration(USART_DEVICE * uart_dev){
    uint8_t lcs = ~(0x05) + 1;
    uint8_t cs = ~(PN532_FROM_UC_TFI + 0x14 + 0x01 + 0x00 + 0x00) + 1;

    uint8_t packet[12] = { 0x00, 0x00, 0xFF, 0x05, lcs, PN532_FROM_UC_TFI,
				0x14, 0x01, 0x00, 0x00, cs, 0x00
	};

    uint8_t data[20];
    uint32_t length = 0;
    uart_dev->Write(packet, 12);
    
    //wait for answer from NFC read
    length = uart_dev->Read(data);

    //check for ACK

    uart_dev->Write(data, length);

    return 0;
}