#include <ft900.h>



static inline int8_t pot_read( uint8_t address, uint8_t *data )
{
    int8_t retval = 0;
    retval = i2cm_read( address, 100, data, 1 );
    return retval;
}

void pots_class_task( void *pvParameters )
{
	uint8_t address = 0x50;
	uint32_t delay = 10;

	while( 1 )
	{
		static uint8_t val=0;
		pot_read( address, &val );
		tfp_printf( "raw=%3d\r", val );
		delayms( delay );
	}
}
