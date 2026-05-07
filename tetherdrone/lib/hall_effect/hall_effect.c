#include "hall_effect.h"
#include "esp_adc/adc_oneshot.h"

// Set your ADC channel here (ADC_CHANNEL_0 is usually GPIO 1 on ESP32-S3)
#define HALL_ADC_UNIT ADC_UNIT_1
#define HALL_ADC_CHAN ADC_CHANNEL_0

// Global handle hidden from main.c
static adc_oneshot_unit_handle_t adc_handle;

void hall_init(void) {
    // 1. Setup the ADC unit
    adc_oneshot_unit_init_cfg_t init_config = { 
        .unit_id = HALL_ADC_UNIT 
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    // 2. Setup the channel to read 0-3.3V (12dB attenuation)
    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc_handle, HALL_ADC_CHAN, &chan_config);
}

int hall_read(void) {
    int val = 0;
    // Read the raw analog value into 'val'
    adc_oneshot_read(adc_handle, HALL_ADC_CHAN, &val);
    return val;
}