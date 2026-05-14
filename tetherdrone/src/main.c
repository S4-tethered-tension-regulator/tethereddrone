#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "load_cell.h"  
#include "stepper_motor.h" 
#include "hall_effect.h"
#include "pins.h" //this is the libary where all the pins are setup (esy to change up later)
#include "lcd.h"
#include "button.h"

long load_cell_max; //this is the maximum the load cell can measure in our system
long load_cell_min; //this is the minimum the load cell can measure in our system
long load_cell_our_zero; //this one tell us when I pick up the thing how much force I need to apply
int hall_effect_zero; //
int tether_length_max = 1500; //mm (its the amount of tether we can give out) 
uint32_t last_lcd_update = 0; //
const uint32_t LCD_UPDATE_INTERVAL = 250;
volatile long current_tension_global = 0; //measrue the current tension sensor

void load_cell_task(void *pvParameters) {
    while(1) {
        long temp_tension = load_cell_read();
        
        // I had problems with calc /0 crashin the system so ths prevent it
        if (temp_tension != 0) {
            current_tension_global = temp_tension;
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}

void app_main() {

    // here I just init all the libaries I wanna use
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

    // okay now it needs to calibrate. there are 2 things that need to be calibrated: the load cell and the hall effect sensor.
    // the load cell should record the highest tension possible and then go as low as possible. Due that the "drone" starts always at the 
    // same position I can pull it as hard as I want to until the max is reached and the min is just giving 
    // the hall effect is just for zeroing so when the load cell is finished centering, it saves that value so it knows it is "home".
    
    lcd_set_cursor(0,0);
    lcd_send_string("calibrating");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    // Max Tension Calibration
    lcd_set_cursor(0,1);
    lcd_send_string("Start Max Tension");
    lcd_set_cursor(0,2);
    lcd_send_string("Starting Motor");
    while(button_read_up() == 1) //bassicly moves the motor backwards until the moveable pulley reached the max position and this 
    {                            // triggers the button, so then we know the max tension
       stepper_motor_move(100);
       vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    if(button_read_up() == 0){
        stepper_motor_stop(); //stop motor
        vTaskDelay(200 / portTICK_PERIOD_MS);
        load_cell_max = load_cell_read_average(10); //read the loadcell 10 times and finds the avarge value, saves it to load_cell_max
    }

    lcd_set_cursor(0,3);
    lcd_send_string("Max Tension Found");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    lcd_clear();
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Min Tension Calibration - its the same as max tension but instead it does the opposite
    lcd_set_cursor(0,0);
    lcd_send_string("calibrating");
    lcd_set_cursor(0,1);
    lcd_send_string("Start Min Tension");
    lcd_set_cursor(0,2);
    lcd_send_string("Starting Motor");
    while(button_read_down() == 1)
    {
        stepper_motor_move(-100);
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


    //this just displays min and max, due that the the min and max values are 24 bits its a bit big so this round it down so we
    // are able to see it on my 0-3 x 0-19 lcd screen
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

    //5 sec to read the value (bassicly just check if the value seem legit or not)
    vTaskDelay(5000 / portTICK_PERIOD_MS); 

    
    // now we want to center the entire thing so it has enough tension

    load_cell_our_zero = (load_cell_max + load_cell_min) / 2; //this find the middle between min and max
    
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_send_string("Centering Tether...");

    // Print the target immediately
    lcd_set_cursor(0, 1);
    snprintf(display_buffer, sizeof(display_buffer), "Target: %ld", load_cell_our_zero);
    lcd_send_string(display_buffer);

    int loop_counter = 0; 

    while(1) { //this loop increases in small steps the tension until the load_cell_our_zero is reached and then it has enough tension
        loop_counter++; 

        // read the sensor to see whhere we at
        long current_crawl_tension = load_cell_read();
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        lcd_set_cursor(0, 2);
        snprintf(display_buffer, sizeof(display_buffer), "Cur: %ld     ", current_crawl_tension);
        lcd_send_string(display_buffer);

        lcd_set_cursor(0, 3);
        snprintf(display_buffer, sizeof(display_buffer), "Loop: %d      ", loop_counter);
        lcd_send_string(display_buffer);

        // checks with target value if it fullfilled -> go  or if not -> repeat
        if (current_crawl_tension >= load_cell_our_zero && current_crawl_tension != 0) {
            stepper_motor_stop();
            break; 
        }

        //move motor
        stepper_motor_move(100); 
        vTaskDelay(100 / portTICK_PERIOD_MS); 
        
        //stops motor (I am scared that it overshoots to much if I leave it on the whole time)
        stepper_motor_stop();
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }

    lcd_set_cursor(0,0);
    lcd_send_string("Calib Effekt");
    hall_reset_length(); //zeros the hall effekt to 0 to start fresh
    lcd_set_cursor(0,0);
    lcd_send_string("Calib Hall  - Passed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    current_tension_global = load_cell_read();
    xTaskCreate(load_cell_task, "LoadCellTask", 2048, NULL, 5, NULL);

    // calculate the trigger amounts on the Simulation data
    long wiggle_amount = (load_cell_our_zero * 7) / 100; //it has a +-7% wiggle room where it in idle meaning it should just hover but through wind it might be that its goes a slight bit left and right. The 7% come from our simulation
    long upper_tension_threshold = load_cell_our_zero + wiggle_amount; //so this calc then when its higher than +7% (from load_cell_our_zero) to know when it should give tether
    long lower_tension_threshold = load_cell_our_zero - wiggle_amount; // so this calc then when its lower than -7% (from load_cell_our_zero) to know whneit should take tether

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

    // now we can finally start to fly since setup is all done. my logic here is to adjust the tether based on percentage.
    // the hall effect sensor will keep track of how much tether is given or taken.
    // we basically have 3 states:
    // - forwards: if the tension goes over the 7% threshold this triggers.
    // - still: if the value is within the 7% median sweet spot it just remains still.
    // - backwards: if the tension drops below the 7% threshold this triggers to reel it in.
    // originally I wanted to soften the movements so the drone doesn't get yanked out of the air if it comes down fast.
    // but new testing showed me the motor is pretty slow anyway so there is no reason to add that extra code.

    while(1)
    {
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (current_time - last_lcd_update >= LCD_UPDATE_INTERVAL) { // this loop just checks that it doesnt get updated to fast and the lcd start jittering, at the moment it gets updated 4 times per second
            last_lcd_update = current_time; 
        
            lcd_set_cursor(0,0);
            int8_t current_speed = stepper_motor_get_speed(); 
            if (current_speed < 0) {
                lcd_send_string("Forwards ");
            } else if (current_speed > 0) {
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
    // Drone is pulling too hard, need to give more tether (Forwards)
    if (hall_get_length_mm() >= tether_length_max) {
        // Tether is at max limit, stop the motor
        stepper_motor_stop();
    } else {
        // Limit not reached, safe to give more tether
        stepper_motor_move(-100); 
    }
} else if(current_tension < lower_tension_threshold){
    // Tether is loose, reel it in (Backwards)
    if (hall_get_length_mm() <= 0) {
        // Tether is fully reeled in, stop the motor
        stepper_motor_stop();
    } else {
        stepper_motor_move(100);
    }
} else { 
    // Tension is within the +-7% threshold (Idle)
    stepper_motor_stop();
}

vTaskDelay(10 / portTICK_PERIOD_MS);
}
}