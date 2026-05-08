#include "load_cell.h"
#include "pins.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void load_cell_init(void) {
    // Setup Data Pin as Input
    gpio_reset_pin(LOAD_DT_PIN);
    gpio_set_direction(LOAD_DT_PIN, GPIO_MODE_INPUT);

    // Setup Clock Pin as Output
    gpio_reset_pin(LOAD_SCK_PIN);
    gpio_set_direction(LOAD_SCK_PIN, GPIO_MODE_OUTPUT);
    
    // Set clock to 0 initially
    gpio_set_level(LOAD_SCK_PIN, 0);
}

long load_cell_read(void) {
    // Wait until the load cell is ready (Data pin goes LOW)
    while (gpio_get_level(LOAD_DT_PIN) == 1) {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    long count = 0;
    
    // Read 24 bits of data from the HX711
    for (int i = 0; i < 24; i++) {
        gpio_set_level(LOAD_SCK_PIN, 1);
        ets_delay_us(1);
        count = count << 1;
        gpio_set_level(LOAD_SCK_PIN, 0);
        ets_delay_us(1);
        
        if (gpio_get_level(LOAD_DT_PIN)) {
            count++;
        }
    }

    // Send the 25th pulse (Standard for 128 Gain)
    gpio_set_level(LOAD_SCK_PIN, 1);
    ets_delay_us(1);
    gpio_set_level(LOAD_SCK_PIN, 0);
    ets_delay_us(1);

    // Convert to a signed 32-bit integer for negative readings
    if (count & 0x800000) {
        count |= 0xFF000000;
    }
    
    return count;
}