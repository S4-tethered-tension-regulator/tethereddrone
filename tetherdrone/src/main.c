#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "load_cell.h"  
#include "stepper_motor.h" 
#include "pins.h"

void app_main() {
    printf("Initializing Load Cell...\n");

    load_cell_t loadcell;
    
    // Using the exact macros from your pins.h
    load_cell_init(&loadcell, LOAD_DT_PIN, LOAD_SCK_PIN);

    printf("Taring the scale. Please ensure it is empty.\n");
    load_cell_tare(&loadcell, 10);
    
    // You will need to calibrate this scale factor with a known weight
    load_cell_set_scale(&loadcell, 1.0f); 

    stepper_motor_init();

    printf("Setup complete. Starting read loop.\n");

    // Variables to track the time and the motor's current state
    TickType_t last_motor_switch = xTaskGetTickCount();
    bool motor_is_spinning = true;
    
    // Start the motor for the first cycle
    stepper_motor_move(100); 

    while(1) {
        // 1. Read the load cell continuously (e.g., sample it 1 time per loop)
        float weight = load_cell_get_units(&loadcell, 1);
        printf("Weight reading: %.2f\n", weight);

        // 2. Get the current OS tick time
        TickType_t current_time = xTaskGetTickCount();

        // 3. Handle the motor timing without blocking the loop
        if (motor_is_spinning) {
            // Has it been spinning for 5000ms?
            if ((current_time - last_motor_switch) >= pdMS_TO_TICKS(5000)) {
                stepper_motor_stop();
                motor_is_spinning = false;
                last_motor_switch = current_time; // Reset the timer
            }
                    // In main.c
            } else {
                // Has it been stopped for 10000ms? (Changed from 3000 for testing)
                if ((current_time - last_motor_switch) >= pdMS_TO_TICKS(10000)) {
                    stepper_motor_move(100);
                    motor_is_spinning = true;
                    last_motor_switch = current_time; 
                }
            }
        
        // Wait just 100 milliseconds before reading the load cell again.
        // This gives you 10 weight readings per second.
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}