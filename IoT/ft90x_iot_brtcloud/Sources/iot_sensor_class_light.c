#include <ft900.h>



/** Digi pot Read routine
 *  @param pnum The location to start reading
 *  @param data A pointer to the data to read into
 *  @param len The size of data to read */
static inline int8_t led_setcolor( uint8_t address, uint32_t color )
{
	//static uint8_t ping_pong = 0;
	uint8_t data[] = {color & 0xff,(color >> 8) & 0xff,(color >> 16) & 0xff};
    int8_t retval = 0;
    //uint8_t l_data = ((ping_pong != 0) + 3);
    retval = i2cm_write( address, 0, data, 3 );
   // ping_pong ^= 1;
    return retval;
}

/** Digi pot Read routine
 *  @param pnum The location to start reading
 *  @param data A pointer to the data to read into
 *  @param len The size of data to read */
static inline int8_t led_on( uint8_t address )
{
	//static uint8_t ping_pong = 0;
	uint8_t data[] = {0xFF,0xFF,0xFF};
    int8_t retval = 0;
    //uint8_t l_data = ((ping_pong != 0) + 3);
    retval = i2cm_write( address, 0, data, 3 );
   // ping_pong ^= 1;
    return retval;
}

/** Digi pot Read routine
 *  @param pnum The location to start reading
 *  @param data A pointer to the data to read into
 *  @param len The size of data to read */
static inline int8_t led_off( uint8_t address )
{
//	static uint8_t ping_pong = 0;
	uint8_t data[] = {00,00,00};
    int8_t retval = 0;
    //uint8_t l_data = ((ping_pong != 0) + 3);
    retval = i2cm_write( address, 0, data, 3 );
   // ping_pong ^= 1;
    return retval;
}

#if 0
static inline int8_t led_fade( uint8_t address, uint32_t color, uint8_t t )
{
	uint8_t cmd = 0x00;
	r = R(color);
	g = G(color);
	b = B(color);
	ftime = t;

	if(ftime==0)
		cmd =0;
	else
		cmd =1;

    int8_t retval = 0;
    //uint8_t l_data = ((ping_pong != 0) + 3);
    retval = i2cm_write(LED_ADDR, 0, &cmd, 1);
    retval = i2cm_write(LED_ADDR, 0, &r, 1);
    retval = i2cm_write(LED_ADDR, 0, &g, 1);
    retval = i2cm_write(LED_ADDR, 0, &b, 1);
    retval = i2cm_write(LED_ADDR, 0, &ftime, 1);
   // ping_pong ^= 1;
    return retval;
}
#endif

/** Digi pot Read routine
 *  @param pnum The location to start reading
 *  @param data A pointer to the data to read into
 *  @param len The size of data to read */
static inline int8_t led_fade( uint8_t address, uint32_t color, uint8_t t )
{
	//static uint8_t ping_pong = 0;
	uint8_t data[] = {color & 0xff,(color >> 8) & 0xff,(color >> 16) & 0xff, t};
    int8_t retval = 0;
    //uint8_t l_data = ((ping_pong != 0) + 3);
    retval = i2cm_write( address, 1, data, 4 );
   // ping_pong ^= 1;
    return retval;
}

void light_class_task( void *pvParameters )
{
	uint8_t address = 0x10;
	uint32_t delay = 10;

	while( 1 )
	{
#if 0
		static uint32_t val=0x000000ff;
		led_setcolor( address, val );
		//tfp_printf("raw=%d\r", val);
		delayms(1000);
		if(val < 0x00ff0000) {
			val <<= 8;
		}
		else {
			val = 0x000000ff;
		}
#endif

#if 0
		led_on( address );
		delayms(1000);
		led_off( address );
		delayms(1000);
#endif

#if 1
		static uint32_t val=0x000000ff;
		led_fade( address, val, 60 );
		delayms(2000);
		if(val < 0x00ff0000) {
			val <<= 8;
		}
		else {
			val = 0x000000ff;
		}
#endif
	}
}
