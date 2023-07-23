/**
* @author Juan D. Martín
* @details Abstract methods to send commands to the PN532 NFC reader
*/

#ifndef PN532_H
#define PN532_H

//use it for the "DetectCard" function parameter "max_cards"
#define PN532_ONE_CARD  0x01
#define PN532_TWO_CARDS 0x02

#include "usart_device.h"
#include "PN532_COMMANDS.h"

/**
*@brief wake up PN532, is the first thing you should do :)
*@param uart_dev to a struct that contains write/read command for serial
*/
void pn532_SendWakeUpCommand(USART_DEVICE * uart_dev);

/**
 * @brief Send commad to get the firmware version
 * @param uart_dev pointer to a struct that contains write/read command for serial
 * @return array of 2 position containing the version of the reader
*/
uint8_t * pn532_GetFirmwareVersionCommand(USART_DEVICE * uart_dev);

/**
 * @brief Send SAM configuration to start the system
 * @param uart_dev pointer to a struct that contains write/read command for serial
 * @return int, 1 if the operation was succesfull, 0 if there was a problem
*/
uint8_t pn532_SendSAMConfiguration(USART_DEVICE * uart_dev);

/**
 * @brief looks in the response of the NFC reader for the ACK
 * @param data, array of data containing the PN532 response
 * @return 1 if the ACK was successfull, 0 if there is no ACK
*/
static uint8_t pn532_checkACK(uint8_t * data);

/**
 * @brief Detect if any card is present in the reader
 * @param uart_dev pointer to a struct that contains write/read command for serial
 * @param max_cards max quantity of cars the reader will read, Min 1, Max 2
 * @return 1 if card detected, 0 if no, by now, this library only will return if 1 card is detected
*/
uint8_t pn532_DetectCard(USART_DEVICE * uart_dev, uint8_t max_cards);

/**
 * @brief send Hardcoded RF configuration
 * @note this should be workd differently
*/
uint8_t pn532_ConfigRF(USART_DEVICE * uart_dev);


static void pn532_SendCommand(USART_DEVICE * uart_dev);

#endif