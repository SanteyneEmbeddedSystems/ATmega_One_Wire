extern "C" {
    #include "My_One_Wire_IL.h"
}

/* Test is done with a DS18B20 device */

const T_One_Wire_Device_Address device_adress = { 0x28, 0xEE, 0xF4, 0x79, 0xA2, 0x0, 0x3, 0xFD };

void setup( void )
{
    Serial.begin(9600);
    while (!Serial) {
    }
    Serial.println("Serial communication ready");
}


void loop( void )
{
   uint8_t read_bytes[2] = {0};

    /* Send a command to mesure temperature */
    My_One_Wire_IL__One_Wire_Protocol.Send_Simple_Command( &device_adress, 0x44 );

    delay(500);

    /* Send a command to read the scratchpad */
    My_One_Wire_IL__One_Wire_Protocol.Send_Read_Command( &device_adress, 0xBE, read_bytes, 2);
    Serial.print(read_bytes[0]);
    Serial.print(" ");
    Serial.println(read_bytes[1]);
}