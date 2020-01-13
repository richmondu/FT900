#include <ft900.h>



static inline int8_t beep_note( uint8_t address, uint8_t duration, uint8_t note )
{
    int8_t retval = 0;
    retval = i2cm_write( address, duration, &note, 1 );
    return retval;
}

void beep_class_task( void *pvParameters )
{
    uint8_t address = 0x60;
    int8_t note = 55;
    uint32_t duration = 100;
    uint32_t delay = 100;

    while( 1 ) {
        beep_note( address, duration, note );
        delayms( delay );
    }
}
