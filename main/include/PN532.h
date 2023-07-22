/*
* Author: Juan D. Martín
* Description: Abstract methods to send commands to the PN532 NFC reader
*/

#ifndef PN532_H
#define PN532_H

#include "usart_device.h"
#include "PN532_COMMANDS.h"

/**
*@brief wake up PN532, is the first thing you should do :)
*@param uart_dev to a struct that contains write command for serial
*/
void pn532_SendWakeUpCommand(USART_DEVICE * uart_dev);

/**
 * @brief Send commad to get the firmware version
 * @param uart_dev pointer to a struct that contains write command for serial
*/
void pn532_GetFirmwareVersionCommand(USART_DEVICE * uart_dev);

/**
 * @brief Send SAM configuration to start the system
 * @param uart_dev pointer to a struct that contains write command for serial
 * @return int, 1 if the operation was succesfull, 0 if there was a problem
*/
uint8_t pn532_SendSAMConfiguration(USART_DEVICE * uart_dev);

void pn532_SendCommand(USART_DEVICE * uart_dev);

#endif