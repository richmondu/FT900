#include <ft900.h>



#define GPIO_LOW              0
#define GPIO_HIGH             1
#define INITIAL_STATUS        (GPIO_HIGH)

static char g_button = 0;
static char g_button_status = INITIAL_STATUS;



////////////////////////////////////////////////////////////////////////////////////////
// button_setup
////////////////////////////////////////////////////////////////////////////////////////
int button_setup(void (*button_isr)(void), int button_gpio)
{
    g_button = (char)button_gpio;

    /* Set up the pin */
    gpio_dir(g_button, pad_dir_input);
    gpio_pull(g_button, pad_pull_pullup);

    /* Attach an interrupt */
    if (button_isr) {
        interrupt_attach(interrupt_gpio, (uint8_t)interrupt_gpio, button_isr);
        gpio_interrupt_enable(g_button, gpio_int_edge_falling);
        interrupt_enable_globally();
    }

    if (gpio_read(g_button) != g_button_status) {
        return 0;
    }

    g_button_status = 1;
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////////
// button_is_interrupted
////////////////////////////////////////////////////////////////////////////////////////
int button_is_interrupted()
{
#if 0
    return 1;
#else
    return gpio_is_interrupted(g_button);
#endif
}


////////////////////////////////////////////////////////////////////////////////////////
// button_is_pressed
////////////////////////////////////////////////////////////////////////////////////////
int button_is_pressed()
{
    if (g_button_status == INITIAL_STATUS) {
        if (gpio_read(g_button) != g_button_status) {
            g_button_status = 1-g_button_status;
            return 1;
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// button_is_released
////////////////////////////////////////////////////////////////////////////////////////
int button_is_released()
{
    if (g_button_status != INITIAL_STATUS) {
        if (gpio_read(g_button) != g_button_status) {
            g_button_status = 1-g_button_status;
            return 1;
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// button_release
////////////////////////////////////////////////////////////////////////////////////////
void button_release()
{
    gpio_write(g_button, INITIAL_STATUS);
    g_button_status = (char)INITIAL_STATUS;
}

