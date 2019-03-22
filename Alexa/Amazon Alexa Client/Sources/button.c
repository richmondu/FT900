#include <ft900.h>



#define USE_GPIO              0



#if USE_GPIO
#define GPIO_LOW              0
#define GPIO_HIGH             1

static char g_button_init = GPIO_LOW;
static char g_button = 0;
static char g_button_status = 0;
#else // USE_GPIO
static char g_button_status = 0;
#define TAP_KEY               't'
#endif // USE_GPIO



////////////////////////////////////////////////////////////////////////////////////////
// button_setup
////////////////////////////////////////////////////////////////////////////////////////
int button_setup(void (*button_isr)(void), int button_gpio)
{
#if USE_GPIO
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

    g_button_init = gpio_read(g_button);
    g_button_status = g_button_init;
    tfp_printf("g_button_init=%d\r\n", g_button_init);
#else // USE_GPIO
    /* Attach the interrupt so it can be called... */
    interrupt_attach(interrupt_uart0, (uint8_t) interrupt_uart0, button_isr);
    /* Enable the UART to fire interrupts when receiving data... */
    uart_enable_interrupt(UART0, uart_interrupt_rx);
    /* Enable interrupts to be fired... */
    uart_enable_interrupts_globally(UART0);
    interrupt_enable_globally();
#endif // USE_GPIO

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
#if USE_GPIO
    return gpio_is_interrupted(g_button);
#else // USE_GPIO
    return uart_is_interrupted(UART0, uart_interrupt_rx);
#endif // USE_GPIO
#endif
}


////////////////////////////////////////////////////////////////////////////////////////
// button_is_pressed
////////////////////////////////////////////////////////////////////////////////////////
int button_is_pressed()
{
#if USE_GPIO
    if (g_button_status == g_button_init) {
        uint8_t c = gpio_read(g_button);
        if (c != g_button_status) {
            g_button_status = c;
            //tfp_printf("press=%d\r\n", c);
            return 1;
        }
    }
#else // USE_GPIO
    if (!g_button_status) {
        uint8_t c = {0};
        uart_read(UART0, &c);
        if (c == TAP_KEY) {
            g_button_status = 1;
            return 1;
        }
    }
#endif // USE_GPIO
    return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// button_is_released
////////////////////////////////////////////////////////////////////////////////////////
int button_is_released()
{
#if USE_GPIO
    if (g_button_status != g_button_init) {
        uint8_t c = gpio_read(g_button);
        if (c != g_button_status) {
            //tfp_printf("release=%d\r\n", c);
            g_button_status = c;
            return 1;
        }
    }
#else // USE_GPIO
    if (g_button_status) {
        uint8_t c = 0;
        uart_read(UART0, &c);
        if (c == TAP_KEY) {
            g_button_status = 0;
            return 1;
        }
    }
#endif // USE_GPIO
    return 0;
}


#if 0
////////////////////////////////////////////////////////////////////////////////////////
// button_release
////////////////////////////////////////////////////////////////////////////////////////
void button_release()
{
#if USE_GPIO
    gpio_write(g_button, g_button_init);
    g_button_status = g_button_init;
#endif // USE_GPIO
}
#endif
