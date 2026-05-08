#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "load_cell.h"  
#include "stepper_motor.h" 
#include "pins.h"
#include "lcd.h"
#include "button.h"

long load_cell_max;
long load_cell_min;
long load_cell_our_zero; //this one tell us when I pick up the thing how much force I need to apply
int hall_effect_zero;
int tether_length_currently = 0; //in mm
int tether_length_max = 1500; //mm 
uint32_t last_lcd_update = 0;
const uint32_t LCD_UPDATE_INTERVAL = 250;

void app_main() {
    
    // Start, I want here that all the porgramms are init and the calibartion of the tether should be given.

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
    //hall_init();
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


    //Clear so I know that I finished all the inits
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

    //okay now it needs to calibrate. There are 2 things that need to be calibrated that is the load cell and the hall effekt
    // the load cell should go as high as possible and then go as low as possible 
    // the hall effekt is just for zeroing so when the load call is finished it should save the load cell value so its know that its home

    lcd_set_cursor(0,0);
    lcd_send_string("calibrating");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    //first we look at the load cell and I want to make it that it first finds its maximum.
    // this means the motor needs to be slowly taken until button 1 is pressed
    lcd_set_cursor(0,1);
    lcd_send_string("Start Max Tension");
    lcd_set_cursor(0,2);
    lcd_send_string("Starting Motor");
    while(button_read_up() == 1) //button not triggered
    {
       stepper_motor_move(-90);
       vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    if(button_read_up() == 0){
        load_cell_max = load_cell_read();
        stepper_motor_stop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    lcd_set_cursor(0,3);
    lcd_send_string("Max Tension Found");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    lcd_clear();
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    //now repreat the same again for the downside
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
        load_cell_min = load_cell_read();
        stepper_motor_stop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    lcd_set_cursor(0,3);
    lcd_send_string("Min Tension Found");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    
    lcd_clear();

    //okay now here I want to zero the hall_effekt sensor
    lcd_set_cursor(0,0);
    lcd_send_string("Calib Effekt");
    //hall_effect_zero = hall_read();
    lcd_set_cursor(0,0);
    lcd_send_string("Calib Hall  - Passed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    

    lcd_clear();


    //here I want to read of the values of min and max for testing so show the min and max value (the first 10 digits)
    // --- DISPLAY MIN/MAX VALUES FOR TESTING ---
    char display_buffer[20]; // Buffer to hold the formatted text

    // Display the Maximum Value
    lcd_set_cursor(0, 0);
    lcd_send_string("Max Tension:");
    lcd_set_cursor(0, 1);
    // Format the number and save it to display_buffer
    snprintf(display_buffer, sizeof(display_buffer), "%ld", load_cell_max);
    lcd_send_string(display_buffer);

    // Display the Minimum Value
    lcd_set_cursor(0, 2);
    lcd_send_string("Min Tension:");
    lcd_set_cursor(0, 3);
    // Format the number and save it to display_buffer
    snprintf(display_buffer, sizeof(display_buffer), "%ld", load_cell_min);
    lcd_send_string(display_buffer);

    
    //okay now I hav the 2 values and I need to make the tether centered
    load_cell_our_zero = load_cell_max / 2; // changes this here
    stepper_motor_move(-90);

    // Wait until the tension drops to our target zero
    while(load_cell_read() > load_cell_our_zero) {
        vTaskDelay(10 / portTICK_PERIOD_MS); 
    } 
    stepper_motor_stop();

    // Calculate the 7% threshold using integer math (no decimals!) - aka activation_wiggle_room
    long wiggle_amount = (load_cell_our_zero * 7) / 100;
    long upper_tension_threshold = load_cell_our_zero + wiggle_amount;
    long lower_tension_threshold = load_cell_our_zero - wiggle_amount;

    //Calibration Complete
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


   //now we can finnally start to fly now all is setup. My logic here is that I want percentagewise more or less tether
   // The hall effekt sensosr should start reading how much tether is taken/given 
   //I can have 3 states:
   // - forwars (if the value is over 7% this is triggered. The higher the tension the faster the motor should be rotating)
   // - still (if the value is in 7% of the median than it should be remain to be still)
   // - backwards (if the value is over 7% this is triggered. The higher the tension the faster the motor should be rotating)
   // - I also need to soften the blow so that the drone if it coming down it not janked out of the air -> okay new testing showed me 
   // that the motor moves very slow anyway so I think there is no reason to make this

  

    while(1)
    {
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (current_time - last_lcd_update >= LCD_UPDATE_INTERVAL) {
        last_lcd_update = current_time; 
      
        // display what the motor does atm
        lcd_set_cursor(0,0);
        int8_t current_speed = stepper_motor_get_speed(); // read speedy
        if (current_speed > 0) {
            lcd_send_string("Forwards ");
        } else if (current_speed < 0) {
            lcd_send_string("Backwards");
        } else {
            lcd_send_string("Idle     ");
        }
        
        //this displays the tether length
        lcd_set_cursor(0,1);
        lcd_send_string("Tether ");
        lcd_send_int(tether_length_currently);
        lcd_send_string("/");
        lcd_send_int(tether_length_max);

       }
        

        //now I want the motor to actually do something so here comes the logic of the motor 
        // Read the sensor exactly ONE time per loop
        long current_tension = load_cell_read(); 

        // Use the pre-calculated integer limits
        if(current_tension > upper_tension_threshold){ //if the tension is big you give tether
            stepper_motor_move(-90);
        } else if(current_tension < lower_tension_threshold){
            stepper_motor_move(90);
        } else{ //if the tension is close to our calibrated zero
            stepper_motor_stop();
        }
        
        // small delay so the programm is not overwehlmed
        vTaskDelay(10 / portTICK_PERIOD_MS);
   }
}   