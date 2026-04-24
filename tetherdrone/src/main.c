#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "load_cell/load_cell.h"
#include "pins/pins.h"  // Changed to include the folder name

void app_main() {
    printf("Initializing Load Cell...\n");

    load_cell_t loadcell;
    
    // Replace LOADCELL_DOUT_PIN and LOADCELL_SCK_PIN with the actual macros from your pins.h
    load_cell_init(&loadcell, LOAD_DT_PIN, LOAD_SCK_PIN);

    printf("Taring the scale. Please ensure it is empty.\n");
    load_cell_tare(&loadcell, 10);
    
    // You will need to calibrate this scale factor with a known weight
    load_cell_set_scale(&loadcell, 1.0f); 

    printf("Setup complete. Starting read loop.\n");

    while(1) {
        float weight = load_cell_get_units(&loadcell, 3);
        printf("Weight reading: %.2f\n", weight);
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second between reads
    }
}