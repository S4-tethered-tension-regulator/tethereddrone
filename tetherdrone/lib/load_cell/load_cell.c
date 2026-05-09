#include "load_cell.h"
#include "pins.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

long load_cell_offset = 0;

// Initialize the FreeRTOS spinlock to protect our timing
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void load_cell_init(void) {
    // Setup Data Pin as Input
    gpio_reset_pin(LOAD_DT_PIN);
    gpio_set_direction(LOAD_DT_PIN, GPIO_MODE_INPUT);

    // Setup Clock Pin as Output
    gpio_reset_pin(LOAD_SCK_PIN);
    gpio_set_direction(LOAD_SCK_PIN, GPIO_MODE_OUTPUT);
    
    // Send a reset pulse on startup to ensure a clean state
    gpio_set_level(LOAD_SCK_PIN, 1);
    ets_delay_us(100);
    gpio_set_level(LOAD_SCK_PIN, 0);
}

long load_cell_read(void) {
    // Grab the current hardware time
    TickType_t start_tick = xTaskGetTickCount();
    
    // Set a strict real-world timeout of 500 milliseconds
    TickType_t timeout_ticks = pdMS_TO_TICKS(500); 

    // Wait until the data pin goes LOW
    while (gpio_get_level(LOAD_DT_PIN) == 1) {
        
        // Check if 500ms has physically passed
        if ((xTaskGetTickCount() - start_tick) > timeout_ticks) {
            // The sensor crashed from motor noise. Force a hardware reset!
            gpio_set_level(LOAD_SCK_PIN, 1);
            ets_delay_us(100);
            gpio_set_level(LOAD_SCK_PIN, 0);
            return 0; // Return 0 instantly to keep the main loop moving
        }
        
        // Tiny yield to prevent watchdog crash
        vTaskDelay(1); 
    }

    long count = 0;
    
    // --- ENTER CRITICAL SECTION ---
    // The ESP32 is now forbidden from pausing our code. Perfect timing guaranteed.
    portENTER_CRITICAL(&mux);
    
    // Read 24 bits directly
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

    // Send the 25th pulse for standard 128 Gain
    gpio_set_level(LOAD_SCK_PIN, 1);
    ets_delay_us(2);
    gpio_set_level(LOAD_SCK_PIN, 0);
    ets_delay_us(2);

    // --- EXIT CRITICAL SECTION ---
    // The ESP32 is allowed to resume background tasks now.
    portEXIT_CRITICAL(&mux);

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