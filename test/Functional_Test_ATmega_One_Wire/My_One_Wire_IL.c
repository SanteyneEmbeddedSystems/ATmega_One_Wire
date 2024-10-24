#include "My_One_Wire_IL.h"

const ATmega32_One_Wire My_One_Wire_IL =
{
    .Port_Address = &PORTD,
    .Bit_Index = 6
};

static void My_One_Wire_IL__Send_Simple_Command(
    const T_One_Wire_Device_Address* slave_address,
    uint8_t command )
{
    ATmega32_OneWire__Communication__Send_Simple_Command(
        &My_One_Wire_IL,
        slave_address,
        command );
}

static void My_One_Wire_IL__Send_Write_Command(
    const T_One_Wire_Device_Address* slave_address,
    uint8_t command,
    const uint8_t message[],
    uint8_t message_length )
{
   ATmega32_OneWire__Communication__Send_Write_Command(
        &My_One_Wire_IL,
        slave_address,
        command,
        message,
        message_length );
}

static void My_One_Wire_IL__Send_Read_Command(
    const T_One_Wire_Device_Address* slave_address,
    uint8_t command,
    uint8_t message[],
    uint8_t message_length )
{
    ATmega32_OneWire__Communication__Send_Read_Command(
        &My_One_Wire_IL,
        slave_address,
        command,
        message,
        message_length );
}
    
const One_Wire_Protocol My_One_Wire_IL__One_Wire_Protocol =
{
    .Send_Simple_Command = My_One_Wire_IL__Send_Simple_Command,
    .Send_Write_Command = My_One_Wire_IL__Send_Write_Command,
    .Send_Read_Command = My_One_Wire_IL__Send_Read_Command
};
