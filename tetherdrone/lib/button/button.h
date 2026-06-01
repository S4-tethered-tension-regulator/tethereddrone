#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include "driver/gpio.h"

// Initializes all buttons defined in pins.h
void button_init(void);

// Returns true if the UP button is currently pressed
bool button_read_up(void);

// Returns true if the DOWN button is currently pressed
bool button_read_down(void);

#endif