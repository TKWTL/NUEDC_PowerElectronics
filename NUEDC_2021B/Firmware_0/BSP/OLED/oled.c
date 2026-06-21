/*
 * oled.c
 *
 *  Created on: 2025/02//14
 *      Author: TKWTL
 */

#include "oled.h"

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            break;
        case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
            osDelay(pdMS_TO_TICKS(arg_int));
            break;
        case U8X8_MSG_GPIO_CS:
            break;
        case U8X8_MSG_GPIO_DC:
            if(arg_int) LL_GPIO_SetOutputPin(D_C_GPIO_Port, D_C_Pin);
            else LL_GPIO_ResetOutputPin(D_C_GPIO_Port, D_C_Pin);
            break;
        case U8X8_MSG_GPIO_RESET: // delay arg_int * 100 nano seconds
            break;
        default:
            break;
    }
    return 1;
}

uint8_t u8x8_byte_4wire_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_BYTE_INIT:
        /* add your custom code to init i2c subsystem */
        break;
        
        case U8X8_MSG_BYTE_START_TRANSFER:
        break;
        
        case U8X8_MSG_BYTE_SEND:
            GSPI_Transmit(arg_ptr, arg_int);
        break;
            
        case U8X8_MSG_BYTE_END_TRANSFER:
        break;
        
        case U8X8_MSG_BYTE_SET_DC:
            if(arg_int) LL_GPIO_SetOutputPin(D_C_GPIO_Port, D_C_Pin);
            else LL_GPIO_ResetOutputPin(D_C_GPIO_Port, D_C_Pin);
        break;
        
        default:
            return 0;
    }
    return 1;
}
