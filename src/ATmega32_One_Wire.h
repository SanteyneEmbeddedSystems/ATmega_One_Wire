#ifndef ATMEGA_ONE_WIRE_INTERACTION_LAYER_H
#define ATMEGA_ONE_WIRE_INTERACTION_LAYER_H

#if !defined(__AVR__)
#error "This low level Component_Type is dedicated to be used with ATmega."
#endif


/*============================================================================*/
/* Inclusions */
/*============================================================================*/
/* Attributes */
#include <stdint.h>
#include <avr/io.h> /* for Port_Address */

/* Realized interfaces */
#include "One_Wire_Protocol.h"


/*============================================================================*/
/* Component_Type */
/*============================================================================*/
typedef struct {

    /* Configuration_Parameters */
    volatile uint8_t* Port_Address;
    const uint8_t Bit_Index;

} ATmega32_One_Wire;


/*============================================================================*/
/* Realized interfaces */
/*============================================================================*/
void ATmega32_OneWire__Communication__Send_Simple_Command(
    const ATmega32_One_Wire* Me,
    const T_One_Wire_Device_Address* slave_address,
    uint8_t command );
        
void ATmega32_OneWire__Communication__Send_Write_Command(
    const ATmega32_One_Wire* Me,
    const T_One_Wire_Device_Address* slave_address,
    uint8_t command,
    const uint8_t message[],
    uint8_t message_length );    

void ATmega32_OneWire__Communication__Send_Read_Command(
    const ATmega32_One_Wire* Me,
    const T_One_Wire_Device_Address* slave_address,
    uint8_t command,
    uint8_t message[],
    uint8_t message_length );


#endif
