#include "stepper_motor.h"
#include "pins.h" // This connects your pin definitions to the motor code
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

// Global variable to hold the requested speed (0 to 100)
volatile uint8_t current_speed = 0;

void stepper_task(void *pvParameter) {
    uint32_t step_counter = 0; // Add a counter
    
    while(1) {
        if (current_speed > 0) {
            uint32_t delay_us = 500 + ((100 - current_speed) * 45); 

            gpio_set_level(MOTOR_PUL_PIN, 1);
            esp_rom_delay_us(delay_us);
            
            gpio_set_level(MOTOR_PUL_PIN, 0);
            esp_rom_delay_us(delay_us);
            
            // YIELD FIX: Give the OS 1 tick of free time every 10 steps 
            // to reset the Watchdog timer and prevent crashes.
            step_counter++;
            if (step_counter >= 10) {
                vTaskDelay(pdMS_TO_TICKS(1)); 
                step_counter = 0;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void stepper_motor_init(void) {
    // Initialize Direction Pin
    gpio_reset_pin(MOTOR_DIR_PIN);
    gpio_set_direction(MOTOR_DIR_PIN, GPIO_MODE_OUTPUT);

    // Initialize Step/Pulse Pin
    gpio_reset_pin(MOTOR_PUL_PIN);
    gpio_set_direction(MOTOR_PUL_PIN, GPIO_MODE_OUTPUT);

    // Initialize Enable Pin
    gpio_reset_pin(MOTOR_ENA_PIN);
    gpio_set_direction(MOTOR_ENA_PIN, GPIO_MODE_OUTPUT);

    // Enable driver and set initial direction
    gpio_set_level(MOTOR_ENA_PIN, 0);
    gpio_set_level(MOTOR_DIR_PIN, 1); 

    // Launch the pulsing task in the background
    xTaskCreate(stepper_task, "stepper_task", 2048, NULL, 5, NULL);
}

void stepper_motor_move(uint8_t speed_percent) {
    // Cap the value at 100% just to be safe
    if (speed_percent > 100) {
        speed_percent = 100;
    }
    current_speed = speed_percent;
}

void stepper_motor_stop(void) {
    current_speed = 0;
}