#ifndef _USART_DEVICE_H
#define _USART_DEVICE_H
/**
* @author: Juan D. Martín
* @details: Abstract method definition for the user to implement their own USART write operation
*/

#include <stdint.h>

/*
 *This "devices" module are intended to be used to abstract different implementations
 *of the same function and make possible to use the same libraries of different sensors
 *or modules in different projects
 */

typedef struct usart_device{
	void (*Write)(uint8_t * data, uint32_t size);
	uint32_t (*Read)(uint8_t * data);
}USART_DEVICE;

#endif