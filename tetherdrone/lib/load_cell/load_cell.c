#include "load_cell.h"
#include "pins.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

long load_cell_offset = 0;

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
    // Wait until the data pin goes LOW (Ready)
    while (gpio_get_level(LOAD_DT_PIN) == 1) {
        vTaskDelay(1); // Give the watchdog a tiny break while waiting
    }

    long count = 0;
    
    // Read 24 bits directly
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

    // Send the 25th pulse for standard 128 Gain
    gpio_set_level(LOAD_SCK_PIN, 1);
    ets_delay_us(1);
    gpio_set_level(LOAD_SCK_PIN, 0);
    ets_delay_us(1);

    // Convert to a signed 32-bit integer
    if (count & 0x800000) {
        count |= 0xFF000000;
    }
    
    return count;
}

long load_cell_read_average(int times) {
    long sum = 0;
    for (int i = 0; i < times; i++) {
        sum += load_cell_read();
    }
    return sum / times;
}

void load_cell_tare(int times) {
    load_cell_offset = load_cell_read_average(times);
}