#include <ft900.h>



static inline int8_t temp_read( uint8_t address, uint8_t *data )
{
    int8_t retval = 0;
    retval = i2cm_read( address, 0, data, 2 );
    return retval;
}

void temp_class_task( void *pvParameters )
{
	uint8_t address = 0x90;
	uint32_t delay = 20;

	while( 1 )
	{
		static int8_t val[2]={0};
		int8_t tempval=0;

		if( !temp_read( address, val ) )
		{
			tempval = val[1];
			tempval = tempval << 8;
			tempval = tempval | val[0];
			tempval = 0x7FF & tempval;
			tfp_printf( "temp=%d\r", tempval );
		}
		delayms( delay );
	}
}
