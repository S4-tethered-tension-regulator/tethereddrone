#include "stepper_motor.h"
#include "pins.h"
#include "button.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

volatile int8_t current_speed = 0;

void stepper_task(void *pvParameter) {
    uint32_t step_counter = 0;
    
    while(1) {
        if(button_read_up() == 0 || button_read_down() == 0) {
            current_speed = 0;
        }

        if (current_speed != 0) {
            if (current_speed > 0) {
                gpio_set_level(MOTOR_DIR_PIN, 1);
            } else {
                gpio_set_level(MOTOR_DIR_PIN, 0);
            }
            esp_rom_delay_us(10);  // <-- give driver time to register direction change

            uint32_t delay_us = 500 + ((100 - abs(current_speed)) * 45);
            gpio_set_level(MOTOR_PUL_PIN, 1);
            esp_rom_delay_us(delay_us);
            gpio_set_level(MOTOR_PUL_PIN, 0);
            esp_rom_delay_us(delay_us);
            
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
    gpio_reset_pin(MOTOR_DIR_PIN);
    gpio_set_direction(MOTOR_DIR_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(MOTOR_PUL_PIN);
    gpio_set_direction(MOTOR_PUL_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(MOTOR_ENA_PIN);
    gpio_set_direction(MOTOR_ENA_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(MOTOR_ENA_PIN, 0);
    gpio_set_level(MOTOR_DIR_PIN, 1);
    xTaskCreate(stepper_task, "stepper_task", 2048, NULL, 5, NULL);
}

void stepper_motor_move(int8_t speed_percent) {
    if (speed_percent > 100) speed_percent = 100;
    current_speed = speed_percent;
}

void stepper_motor_stop(void) {
    current_speed = 0;
}

int8_t stepper_motor_get_speed(void) {
    return current_speed;
}