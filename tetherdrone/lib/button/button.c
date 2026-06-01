#include "button.h"
#include "pins.h"

void button_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_UP) | (1ULL << BUTTON_DOWN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,      
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE        
    };
    gpio_config(&io_conf);
}

bool button_read_up(void) {
    return (gpio_get_level(BUTTON_UP) == 0);
}

bool button_read_down(void) {
    return (gpio_get_level(BUTTON_DOWN) == 0);
}