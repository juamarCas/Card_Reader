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
 * @return array of 2 position containing the version of the reader
*/
uint8_t * pn532_GetFirmwareVersionCommand(USART_DEVICE * uart_dev);

/**
 * @brief Send SAM configuration to start the system
 * @param uart_dev pointer to a struct that contains write command for serial
 * @return int, 1 if the operation was succesfull, 0 if there was a problem
*/
uint8_t pn532_SendSAMConfiguration(USART_DEVICE * uart_dev);

/**
 * @brief looks in the response of the NFC reader for the ACK
 * @param data, array of data containing the PN532 response
 * @return 1 if the ACK was successfull, 0 if there is no ACK
*/
static uint8_t pnf322_checkACK(uint8_t * data);

void pn532_SendCommand(USART_DEVICE * uart_dev);

#endif