#include "ATmega32_One_Wire.h"

#include <avr/interrupt.h> /* sei, cli */
#include "Bit_Field.h"
#include "Timing.h"


/*============================================================================*/
/* Macros */
/*============================================================================*/
/*
Me->Port_Address = &PORTx : Data Register
Me->Port_Address - 1 = &DDRx : Data Direction Register
Me->Port_Address - 2 = &PINx – Input Pins Address
*/
#define READ_PIN()             TEST_BIT( *(Me->Port_Address-2), Me->Bit_Index )
#define SET_PIN_MODE_INPUT()   CLEAR_BIT( *(Me->Port_Address-1), Me->Bit_Index )
#define SET_PIN_MODE_OUTPUT()  SET_BIT( *(Me->Port_Address-1), Me->Bit_Index )
#define SET_PIN_LOW()          CLEAR_BIT( *(Me->Port_Address), Me->Bit_Index )
#define SET_PIN_HIGH()         SET_BIT( *(Me->Port_Address), Me->Bit_Index )


/*============================================================================*/
/* Private methods declaration */
/*============================================================================*/
static bool Reset_Slaves( const ATmega32_One_Wire* Me );
static void Write_Bit(
    const ATmega32_One_Wire* Me,
    uint8_t bit_value );
static uint8_t Read_Bit( const ATmega32_One_Wire* Me );
static void Write_Byte( 
    const ATmega32_One_Wire* Me,
    uint8_t byte_value );
static uint8_t Read_Byte( const ATmega32_One_Wire* Me );
static void Write_Bytes( 
    const ATmega32_One_Wire* Me,
    const uint8_t* bytes_array,
    uint8_t nb_bytes );
static void Read_Bytes(
    const ATmega32_One_Wire* Me,
    uint8_t* bytes_array,
    uint8_t nb_bytes );
static void Select_Slave(
    const ATmega32_One_Wire* Me,
    const uint8_t address[8] );


/*============================================================================*/
/* Realized interfaces */
/*============================================================================*/
void ATmega32_OneWire__Communication__Send_Simple_Command(
    const ATmega32_One_Wire* Me,
    const T_One_Wire_Device_Address* slave_address,
    uint8_t command )
{
    Reset_Slaves( Me );
    Select_Slave( Me, *slave_address );
    Write_Byte( Me, command );
}
/*----------------------------------------------------------------------------*/
void ATmega32_OneWire__Communication__Send_Write_Command(
    const ATmega32_One_Wire* Me,
    const T_One_Wire_Device_Address* slave_address,
    uint8_t command,
    const uint8_t message[],
    uint8_t message_length )
{
    Reset_Slaves( Me );
    Select_Slave( Me, *slave_address );
    Write_Byte( Me, command );
    Write_Bytes( Me, message, message_length );
}
/*----------------------------------------------------------------------------*/
void ATmega32_OneWire__Communication__Send_Read_Command(
    const ATmega32_One_Wire* Me,
    const T_One_Wire_Device_Address* slave_address,
    uint8_t command,
    uint8_t message[],
    uint8_t message_length )
{
    Reset_Slaves( Me );
    Select_Slave( Me, *slave_address );
    Write_Byte( Me, command );
    Read_Bytes( Me, message, message_length );
}


/*============================================================================*/
/* Private methods definition */
/*============================================================================*/
static bool Reset_Slaves( const ATmega32_One_Wire* Me )
{
    /*
        Rest     Reset pulse       Wait     Slave presence     Rest
                 from master       slave        pulse
        ____                     _________                 _____________
            |                   |         |               |
            |                   |         |               |
            |___________________|         |_______________|

            <-------------------><-------><--------------->
                  > 480 µs        15-60 µs     60-240 µs
    */
    
    uint8_t retries = 0;
    bool slaves_are_present = false;

    /* Master releases bus */
    cli();
    SET_PIN_MODE_INPUT();
    sei();

    /* Wait for the bus is HIGH */
    do
    {
        retries++;
        Wait_Microsecond(2);
    } while ( !READ_PIN() && retries < 100 );

    if( retries < 100 )
    {
        /* Reset pulse from master (me) */
        cli();
        SET_PIN_LOW();
        SET_PIN_MODE_OUTPUT();
        sei();
        Wait_Microsecond(480);

        /* Wait for slaves */
        cli();
        SET_PIN_MODE_INPUT();
        sei();
        Wait_Microsecond(60);

        /* Presence pulse from slaves */
        slaves_are_present = !READ_PIN();
        Wait_Microsecond(240);
    }

    return slaves_are_present;
}
/*----------------------------------------------------------------------------*/
static void Write_Bit( 
    const ATmega32_One_Wire* Me,
    uint8_t bit_value )
{
    /*
        Master sets bus to 0 during 1 to 15 µs.
            ____          _________              ____                  ___
                |        |                           |                |
     "1" =      |        |               "0" =       |                |
                |________|                           |________________|
                <-------->
                 1-15 µs
                <---------------->                   <---------------->
                    60 µs max                             60 µs max 

        Slave reads the bus between 15 and 45 µs after falling edge.          
    */

    /* Set bus to 0 during 1 to 15 µs */
    cli();
    SET_PIN_LOW();
    SET_PIN_MODE_OUTPUT();
    Wait_Microsecond(10);

    /* Set bus to pin value */
    if(bit_value & 1)
    {
        SET_PIN_HIGH();
    }

    /* Let slave reads the bus */
    Wait_Microsecond(45);

    /* Restore bus to 1 */
    SET_PIN_HIGH();
    sei();
}
/*----------------------------------------------------------------------------*/
static uint8_t Read_Bit( const ATmega32_One_Wire* Me )
{
    /*
        Master sets bus to 0 during at least 1µs.

                        slave lets                          slave sets
                        bus                                 bus to 0
            ____       _____________             ____
                |     |           ^                  |
      "1" =     |     |           |        "0" =     |
                |_____|                              |_____________________
                <-----><--------->|                  <-----><--------->^
                1µs min 15µs max                    1µs min 15µs max   |
                                  |
                                 read 1                              read 0
    */
    
    cli();

    /* Master sets bus to 0 during at least 1 µs */
    SET_PIN_MODE_OUTPUT();
    SET_PIN_LOW();
    Wait_Microsecond(3);
    SET_PIN_MODE_INPUT();

    /* Slave lets the bus for "1" or sets it to 0 for "0" */
    Wait_Microsecond(10);

    /* Master reads the bus 15 µs after having setting it to 0 */
    uint8_t bit_value = READ_PIN();
    sei();

    Wait_Microsecond(47);
    return bit_value;    
}
/*----------------------------------------------------------------------------*/
static void Write_Byte(
    const ATmega32_One_Wire* Me,
    uint8_t byte_value )
{
    /* With 1-Wire, LSB is sent first. */
    for (uint8_t bitmask = 0x01; bitmask; bitmask <<= 1)
    {
        Write_Bit( Me, (bitmask & byte_value) ? 1 : 0 );
    }
}
/*----------------------------------------------------------------------------*/
static uint8_t Read_Byte( const ATmega32_One_Wire* Me )
{
    /* With 1-Wire, LSB is received first. */
    uint8_t byte_value = 0;
    for (uint8_t bitmask = 0x01; bitmask; bitmask <<= 1)
    {
        if ( Read_Bit( Me ) )
        {
            byte_value |= bitmask;
        }
    }
    return byte_value;    
}
/*----------------------------------------------------------------------------*/
static void Write_Bytes(
    const ATmega32_One_Wire* Me,
    const uint8_t* bytes_array,
    uint8_t nb_bytes )
{
    for ( uint8_t i = 0 ; i < nb_bytes ; i++ )
    {
        Write_Byte( Me, bytes_array[i] );
    }
}
/*----------------------------------------------------------------------------*/
static void Read_Bytes(
    const ATmega32_One_Wire* Me,
    uint8_t* bytes_array,
    uint8_t nb_bytes )
{
    for( uint8_t i = 0 ; i < nb_bytes ; i++ )
    {
        bytes_array[i] = Read_Byte( Me );
    }
}
/*----------------------------------------------------------------------------*/
static void Select_Slave(
    const ATmega32_One_Wire* Me,
    const uint8_t address[8] )
{
    /* Send a Match ROM command */
    Write_Byte( Me, 0x55 );

    /* Send the adress of the slave */
    Write_Bytes( Me, address, 8 );
}
