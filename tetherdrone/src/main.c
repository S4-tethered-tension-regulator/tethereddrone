#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "load_cell.h"  
#include "stepper_motor.h" 
#include "pins.h"
#include "lcd.h"

void app_main() {
    // Elevate the main task priority to 6 (higher than the motor's 5).
    // This guarantees the main loop gets CPU time to update the screen.
    vTaskPrioritySet(NULL, 6);

    printf("Initializing Components...\n");

    // Init Load Cell
    load_cell_t loadcell;
    load_cell_init(&loadcell, LOAD_DT_PIN, LOAD_SCK_PIN);

    printf("Taring the scale. Please ensure it is empty.\n");
    load_cell_tare(&loadcell, 10);
    
    // You will need to calibrate this scale factor with a known weight
    load_cell_set_scale(&loadcell, 1.0f); 

    // Init Screen
    lcd_init();
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string("Starting Up...");

    // Init Motor
    stepper_motor_init();

    printf("Setup complete. Starting read loop.\n");

    TickType_t last_motor_switch = xTaskGetTickCount();
    TickType_t last_lcd_update = xTaskGetTickCount();
    bool motor_is_spinning = true;
    
    stepper_motor_move(100); 

    // Clear the "Starting Up..." text before entering the loop
    lcd_clear();

    while(1) {
        TickType_t current_time = xTaskGetTickCount();

        // 1. Screen and Load Cell Update Logic (Every 1000ms)
        if ((current_time - last_lcd_update) >= pdMS_TO_TICKS(1000)) {
            float weight = load_cell_get_units(&loadcell, 1);
            printf("Weight reading: %.2f\n", weight);

            // Print the title on the first row (Row 0)
            lcd_set_cursor(0, 0);
            lcd_send_string("Load Cell:");

            // Format the number with a space before 'g' and padding spaces after
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.0f g       ", weight);
            
            // Move cursor to the second row (Row 1) and print the number
            lcd_set_cursor(0, 1);
            lcd_send_string(buffer);

            last_lcd_update = current_time;
        }

        // 2. Motor State Logic
        if (motor_is_spinning) {
            // Has it been spinning for 5000ms?
            if ((current_time - last_motor_switch) >= pdMS_TO_TICKS(5000)) {
                stepper_motor_stop();
                motor_is_spinning = false;
                last_motor_switch = current_time;
            }
        } else {
            // Has it been stopped for 10000ms?
            if ((current_time - last_motor_switch) >= pdMS_TO_TICKS(10000)) {
                stepper_motor_move(100);
                motor_is_spinning = true;
                last_motor_switch = current_time; 
            }
        }
        
        // Safely yield the CPU back to the motor task for 10 milliseconds.
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}