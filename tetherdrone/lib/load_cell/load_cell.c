#include "load_cell.h"
#include "pins.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// for the timing
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void load_cell_init(void) {
    gpio_reset_pin(LOAD_DT_PIN);
    gpio_set_direction(LOAD_DT_PIN, GPIO_MODE_INPUT);

    // clock pin is output
    gpio_reset_pin(LOAD_SCK_PIN);
    gpio_set_direction(LOAD_SCK_PIN, GPIO_MODE_OUTPUT);
    
    // reset
    gpio_set_level(LOAD_SCK_PIN, 1);
    ets_delay_us(100);
    gpio_set_level(LOAD_SCK_PIN, 0);
}

long load_cell_read(void) {
    // get the time
    TickType_t start_tick = xTaskGetTickCount();
    
    // wait max 500ms or it will get stuck
    TickType_t timeout_ticks = pdMS_TO_TICKS(500); 

    // wait until it drops to 0
    while (gpio_get_level(LOAD_DT_PIN) == 1) {
        
        // if it takes too long the sensor probably crashed from the motor noise
        if ((xTaskGetTickCount() - start_tick) > timeout_ticks) {
            // force a reset
            gpio_set_level(LOAD_SCK_PIN, 1);
            ets_delay_us(100);
            gpio_set_level(LOAD_SCK_PIN, 0);
            return 0; // return 0 so the main loop keeps going
        }
        
        // small delay to stop watchdog crashes
        vTaskDelay(1); 
    }

    long count = 0;
    
    // stop the esp from doing other stuff so timing is perfect
    portENTER_CRITICAL(&mux);
    
    // read the 24 bits
    for (int i = 0; i < 24; i++) {
        gpio_set_level(LOAD_SCK_PIN, 1);
        ets_delay_us(2); 
        count = count << 1;
        gpio_set_level(LOAD_SCK_PIN, 0);
        ets_delay_us(2); 
        
        if (gpio_get_level(LOAD_DT_PIN)) {
            count++;
        }
    }

    gpio_set_level(LOAD_SCK_PIN, 1);
    ets_delay_us(2);
    gpio_set_level(LOAD_SCK_PIN, 0);
    ets_delay_us(2);

    // let the esp do its normal stuff again
    portEXIT_CRITICAL(&mux);

    // make it a 32 bit int
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