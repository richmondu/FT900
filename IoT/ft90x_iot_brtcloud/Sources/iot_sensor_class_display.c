#include <ft900.h>



static inline int8_t dig2_write_raw( uint8_t address, uint8_t digit1, uint8_t digit2 )
{
	int8_t retval = 0;
	uint8_t wdata[] = { digit1 };
	retval = i2cm_write( address, 0, wdata, 1 );
	return retval;
}

static inline int8_t dig2_write_hex( uint8_t address, uint8_t xdigit )
{
	int8_t retval = 0;
	uint8_t wdata[] = { xdigit };
	retval = i2cm_write( address, 1, wdata, 1 );
	return retval;
}

static inline int8_t dig2_write_dec( uint8_t address, uint8_t ddigit )
{
	int8_t retval = 0;
	uint8_t wdata[] = { ddigit };
	retval = i2cm_write( address, 2, wdata, 1 );
	return retval;
}

static inline int8_t dig2_brightness( uint8_t address, uint8_t bright )
{
	int8_t retval = 0;
	uint8_t wdata[] = { bright };
	retval = i2cm_write( address, 4, wdata, 1 );
	return retval;
}

void display_class_task( void *pvParameters )
{
	uint8_t address = 0x28;
	uint32_t delay = 10;

	while( 1 )
	{
#if 1
		static uint8_t digit=0;
		dig2_write_dec( address, digit );
		tfp_printf( "digit=%2d\r", digit );
		delayms( 1000 );
		digit++;
		if( digit==100 )
		{
			digit = 99;
		}
#endif
#if 0
		static uint8_t digit=0;
		//digit2_brightness(0xff);
		dig2_write_hex( address, digit );
		tfp_printf( "digit=%2X\r", digit );
		delayms( 1000 );
		digit++;
#endif
#if 0
		static uint8_t digit1=0,digit2=0;
		dig2_write_raw( address, digit1, digit2 );
		tfp_printf( "digit=%d%d\r", digit1, digit2 );
		delayms( 1000 );
		digit1++;
		digit2++;

		if( ( digit1==10 ) && ( digit2==10 ) )
		{
			digit1=0;
			digit2=0;
		}
#endif
	}
}
