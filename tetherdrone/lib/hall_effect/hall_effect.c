#include "hall_effect.h"
#include "pins.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_attr.h"
#include "stepper_motor.h" 

volatile int current_tether_length_mm = 0;
volatile int64_t last_pulse_time = 0;

// 60mm diameter * Pi = 188.5mm
const int SPOOL_CIRCUMFERENCE_MM = 188; 

// detect magnet
static void IRAM_ATTR hall_isr_handler(void* arg) {
    int64_t current_time = esp_timer_get_time(); // time
    
    //ignore nect bounce for the next 2 seconds
    if (current_time - last_pulse_time > 2000000) {
        
        //see which motor direction
        int8_t speed = stepper_motor_get_speed();
        
        if (speed < 0) { 
            // Unwinding (Giving slack)
            current_tether_length_mm += SPOOL_CIRCUMFERENCE_MM;
        } else if (speed > 0) { 
            // Winding (Pulling tight)
            current_tether_length_mm -= SPOOL_CIRCUMFERENCE_MM;
        }
        last_pulse_time = current_time;
    }
}

void hall_init(void) {
    gpio_reset_pin(HALL_SENSOR_PIN);
    gpio_set_direction(HALL_SENSOR_PIN, GPIO_MODE_INPUT);
    
    
    gpio_pullup_en(HALL_SENSOR_PIN);
    
    gpio_set_intr_type(HALL_SENSOR_PIN, GPIO_INTR_NEGEDGE); 
    
    gpio_install_isr_service(0);
    gpio_isr_handler_add(HALL_SENSOR_PIN, hall_isr_handler, NULL);
}

void hall_reset_length(void) {
    current_tether_length_mm = 0;
}

int hall_get_length_mm(void) {
    return current_tether_length_mm;
}