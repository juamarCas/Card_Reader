/*
* Author: Juan D. Martín
* Description: Abstract methods to send commands to the PN532 NFC reader
*/

#ifndef PN532_H
#define PN532_H

#include "usart_device.h"
#include "PN532_COMMANDS.h"

void pn532_init(usart_device * uart_dev);

#endif