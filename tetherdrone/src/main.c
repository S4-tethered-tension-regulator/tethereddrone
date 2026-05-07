#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "load_cell.h"  
#include "stepper_motor.h" 
#include "pins.h"
#include "lcd.h"
#include "button.h"

uint32_t load_cell_max;
uint32_t load_cell_min;

void app_main() {
 
    
    // Start, I want here that all the porgramms are init and the calibartion of the tether should be given.

    lcd_init();
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
    //load_cell_init();
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
    for(char i = 0; i < 2; i++){
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
    while(button_read_up() == 1)
    {
       stepper_motor_move(-20);
    }
    if(button_read_up() == 0){
        //here idk yet read the load cell and safe that value and set the value to load_cell_max
        stepper_motor_stop();
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
        stepper_motor_move(20);
    }
    if(button_read_down() == 0){
        //here idk yet read the load cell and safe that value and set the value to load_cell_min
        stepper_motor_stop();
    }
    lcd_set_cursor(0,3);
    lcd_send_string("Min Tension Found");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    
    lcd_clear();

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




   
    


    


    






}