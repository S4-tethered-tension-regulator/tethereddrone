#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "load_cell.h"  
#include "stepper_motor.h" 
#include "hall_effect.h" // <-- Added Hall Sensor Header
#include "pins.h"
#include "lcd.h"
#include "button.h"

long load_cell_max;
long load_cell_min;
long load_cell_our_zero; //this one tell us when I pick up the thing how much force I need to apply
int hall_effect_zero;
// tether_length_currently removed because hall_sensor.c now manages it
int tether_length_max = 1500; //mm 
uint32_t last_lcd_update = 0;
const uint32_t LCD_UPDATE_INTERVAL = 250;

volatile long current_tension_global = 0;

void load_cell_task(void *pvParameters) {
    while(1) {
        long temp_tension = load_cell_read();
        
        // Ignore 0 timeouts so math doesn't break
        if (temp_tension != 0) {
            current_tension_global = temp_tension;
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}

void app_main() {
    
    lcd_init();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_send_string("Init LCD    - Passed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    lcd_set_cursor(0,1);
    lcd_send_string("Init Motor");
    stepper_motor_init();
    lcd_set_cursor(0,1);
    lcd_send_string("Init Motor  - Passed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    lcd_set_cursor(0,2);
    lcd_send_string("Init Load");
    load_cell_init();
    lcd_set_cursor(0,2);
    lcd_send_string("Init Load   - Passed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    lcd_set_cursor(0,3);
    lcd_send_string("Init hall");
    hall_init(); // <-- Actually initialize the Hall sensor
    lcd_set_cursor(0,3);
    lcd_send_string("Init hall   - Passed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_send_string("Init button");
    button_init();
    lcd_set_cursor(0,0);
    lcd_send_string("Init button - Passed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    lcd_clear();
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    //Setup Complete
    for(char i = 0; i < 1; i++){
        lcd_set_cursor(0,0);
        lcd_send_string("===================");
        lcd_set_cursor(0,1);
        lcd_send_string("========Init========");
        lcd_set_cursor(0,2);
        lcd_send_string("========Done========");
        lcd_set_cursor(0,3);
        lcd_send_string("===================");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

       lcd_clear();
       vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    lcd_set_cursor(0,0);
    lcd_send_string("calibrating");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    // Max Tension Calibration
    lcd_set_cursor(0,1);
    lcd_send_string("Start Max Tension");
    lcd_set_cursor(0,2);
    lcd_send_string("Starting Motor");
    while(button_read_up() == 1) 
    {
       stepper_motor_move(-90);
       vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    if(button_read_up() == 0){
        stepper_motor_stop();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        load_cell_max = load_cell_read_average(10);
    }

    lcd_set_cursor(0,3);
    lcd_send_string("Max Tension Found");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    lcd_clear();
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Min Tension Calibration
    lcd_set_cursor(0,0);
    lcd_send_string("calibrating");
    lcd_set_cursor(0,1);
    lcd_send_string("Start Min Tension");
    lcd_set_cursor(0,2);
    lcd_send_string("Starting Motor");
    while(button_read_down() == 1)
    {
        stepper_motor_move(90);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    if(button_read_down() == 0){
        stepper_motor_stop();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        load_cell_min = load_cell_read_average(10);
    }
        
    lcd_set_cursor(0,3);
    lcd_send_string("Min Tension Found");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    lcd_clear();

    lcd_set_cursor(0,0);
    lcd_send_string("Calib Effekt");
    lcd_set_cursor(0,0);
    lcd_send_string("Calib Hall  - Passed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    lcd_clear();

    char display_buffer[32]; 

    lcd_set_cursor(0, 0);
    lcd_send_string("Max Tension:");
    lcd_set_cursor(0, 1);
    snprintf(display_buffer, sizeof(display_buffer), "%ld", load_cell_max);
    lcd_send_string(display_buffer);

    lcd_set_cursor(0, 2);
    lcd_send_string("Min Tension:");
    lcd_set_cursor(0, 3);
    snprintf(display_buffer, sizeof(display_buffer), "%ld", load_cell_min);
    lcd_send_string(display_buffer);

    // Give yourself 5 seconds to physically read the screen
    vTaskDelay(5000 / portTICK_PERIOD_MS); 

    // --- LIVE LCD DEBUG CENTERING CRAWL ---
    load_cell_our_zero = (load_cell_max + load_cell_min) / 2;
    
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_send_string("Centering Tether...");

    // Print the target immediately
    lcd_set_cursor(0, 1);
    snprintf(display_buffer, sizeof(display_buffer), "Target: %ld", load_cell_our_zero);
    lcd_send_string(display_buffer);

    int loop_counter = 0; 

    while(1) {
        loop_counter++; 

        // Step 1: Read the sensor
        long current_crawl_tension = load_cell_read();
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        // Step 2: Update LCD
        lcd_set_cursor(0, 2);
        snprintf(display_buffer, sizeof(display_buffer), "Cur: %ld     ", current_crawl_tension);
        lcd_send_string(display_buffer);

        lcd_set_cursor(0, 3);
        snprintf(display_buffer, sizeof(display_buffer), "Loop: %d      ", loop_counter);
        lcd_send_string(display_buffer);

        // Step 3: Check Target
        if (current_crawl_tension >= load_cell_our_zero && current_crawl_tension != 0) {
            stepper_motor_stop();
            break; 
        }

        // Step 4: Move Motor
        stepper_motor_move(-100); 
        vTaskDelay(100 / portTICK_PERIOD_MS); 
        
        // Step 5: Stop Motor
        stepper_motor_stop();
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
    // -------------------------------------

    // Zero the tether length perfectly now that the drone is physically centered
    hall_reset_length(); // <-- NEW

    // Centering is complete. Now start the background task for the flight loop.
    current_tension_global = load_cell_read();
    xTaskCreate(load_cell_task, "LoadCellTask", 2048, NULL, 5, NULL);

    long wiggle_amount = (load_cell_our_zero * 7) / 100;
    long upper_tension_threshold = load_cell_our_zero + wiggle_amount;
    long lower_tension_threshold = load_cell_our_zero - wiggle_amount;

    for(char i = 0; i < 2; i++){
        lcd_set_cursor(0,0);
        lcd_send_string("===================");
        lcd_set_cursor(0,1);
        lcd_send_string("====Calibration=====");
        lcd_set_cursor(0,2);
        lcd_send_string("========Done========");
        lcd_set_cursor(0,3);
        lcd_send_string("===================");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

       lcd_clear();
       vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while(1)
    {
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (current_time - last_lcd_update >= LCD_UPDATE_INTERVAL) {
            last_lcd_update = current_time; 
        
            lcd_set_cursor(0,0);
            int8_t current_speed = stepper_motor_get_speed(); 
            if (current_speed > 0) {
                lcd_send_string("Forwards ");
            } else if (current_speed < 0) {
                lcd_send_string("Backwards");
            } else {
                lcd_send_string("Idle     ");
            }
            
            lcd_set_cursor(0,1);
            lcd_send_string("Tether ");
            lcd_send_int(hall_get_length_mm()); // <-- Live sensor data
            lcd_send_string("/");
            lcd_send_int(tether_length_max);
            lcd_send_string("   "); // Pad with spaces to clear any leftover characters
        }
            
        long current_tension = current_tension_global; 

        if(current_tension > upper_tension_threshold){ 
            stepper_motor_move(90); 
        } else if(current_tension < lower_tension_threshold){
            stepper_motor_move(-90); 
        } else{ 
            stepper_motor_stop();
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
   }
}