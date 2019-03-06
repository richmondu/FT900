#ifndef BUTTON_H
#define BUTTON_H


int button_setup(void (*button_isr)(void), int button_gpio);
int button_is_interrupted();
int button_is_pressed();
int button_is_released();
void button_release();

#endif // BUTTON_H
