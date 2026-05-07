#pragma once

#include <stdint.h>
#include "driver/gpio.h"

typedef struct {
    gpio_num_t dout;
    gpio_num_t pd_sck;
    long offset;
    float scale;
} load_cell_t;

void load_cell_init(load_cell_t *dev, gpio_num_t dout, gpio_num_t pd_sck);
long load_cell_read(load_cell_t *dev);
long load_cell_read_average(load_cell_t *dev, int times);
void load_cell_tare(load_cell_t *dev, int times);
void load_cell_set_scale(load_cell_t *dev, float scale);
float load_cell_get_units(load_cell_t *dev, int times);