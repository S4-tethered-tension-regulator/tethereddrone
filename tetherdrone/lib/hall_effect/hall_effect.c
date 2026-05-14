#include "hall_effect.h"
#include "pins.h" // Make sure HALL_SENSOR_PIN is defined here!
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_attr.h"
#include "stepper_motor.h" 

volatile int current_tether_length_mm = 0;
volatile int64_t last_pulse_time = 0;

// 60mm diameter * Pi = 188.5mm. We use integer math for speed.
const int SPOOL_CIRCUMFERENCE_MM = 188; 

// This is the Hardware Interrupt. It runs the millisecond the magnet is detected!
static void IRAM_ATTR hall_isr_handler(void* arg) {
    int64_t current_time = esp_timer_get_time(); // Time in microseconds
    
    // DEBOUNCE: Ignore pulses within 2 seconds of the last one.
    if (current_time - last_pulse_time > 2000000) {
        
        // Ask the motor which way we are spinning to know if we are giving or taking slack.
        // Sign convention (matches main.c):
        //   negative speed = unwinding / giving slack (Forwards)
        //   positive speed = winding / pulling tight (Backwards)
        int8_t speed = stepper_motor_get_speed();
        
        if (speed < 0) { 
            // Unwinding (Giving slack)
            current_tether_length_mm += SPOOL_CIRCUMFERENCE_MM;
        } else if (speed > 0) { 
            // Winding (Pulling tight)
            current_tether_length_mm -= SPOOL_CIRCUMFERENCE_MM;
        }
        // Note: If speed == 0 and the spool spins (wind pulling it), we can't safely 
        // guess the direction with only 1 magnet, so we ignore it.
        
        last_pulse_time = current_time;
    }
}

void hall_init(void) {
    gpio_reset_pin(HALL_SENSOR_PIN);
    gpio_set_direction(HALL_SENSOR_PIN, GPIO_MODE_INPUT);
    
    // Enable internal pullup resistor (Standard for most Hall modules)
    gpio_pullup_en(HALL_SENSOR_PIN);
    
    // Trigger the interrupt on the falling edge (when the magnet gets close)
    gpio_set_intr_type(HALL_SENSOR_PIN, GPIO_INTR_NEGEDGE); 
    
    // Install the global Interrupt Service and attach our specific pin
    gpio_install_isr_service(0);
    gpio_isr_handler_add(HALL_SENSOR_PIN, hall_isr_handler, NULL);
}

void hall_reset_length(void) {
    current_tether_length_mm = 0;
}

int hall_get_length_mm(void) {
    return current_tether_length_mm;
}