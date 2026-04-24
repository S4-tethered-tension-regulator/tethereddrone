#include "load_cell.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void load_cell_init(load_cell_t *dev, gpio_num_t dout, gpio_num_t pd_sck) {
    dev->dout = dout;
    dev->pd_sck = pd_sck;
    dev->offset = 0;
    dev->scale = 1.0f;

    gpio_reset_pin(dev->dout);
    gpio_set_direction(dev->dout, GPIO_MODE_INPUT);

    gpio_reset_pin(dev->pd_sck);
    gpio_set_direction(dev->pd_sck, GPIO_MODE_OUTPUT);
    gpio_set_level(dev->pd_sck, 0);
}

long load_cell_read(load_cell_t *dev) {
    while (gpio_get_level(dev->dout) == 1) {
        vTaskDelay(1);
    }

    long count = 0;
    for (int i = 0; i < 24; i++) {
        gpio_set_level(dev->pd_sck, 1);
        ets_delay_us(1);
        count = count << 1;
        gpio_set_level(dev->pd_sck, 0);
        ets_delay_us(1);
        if (gpio_get_level(dev->dout)) {
            count++;
        }
    }

    gpio_set_level(dev->pd_sck, 1);
    ets_delay_us(1);
    gpio_set_level(dev->pd_sck, 0);
    ets_delay_us(1);

    if (count & 0x800000) {
        count |= 0xFF000000;
    }
    return count;
}

long load_cell_read_average(load_cell_t *dev, int times) {
    long sum = 0;
    for (int i = 0; i < times; i++) {
        sum += load_cell_read(dev);
    }
    return sum / times;
}

void load_cell_tare(load_cell_t *dev, int times) {
    long sum = load_cell_read_average(dev, times);
    dev->offset = sum;
}

void load_cell_set_scale(load_cell_t *dev, float scale) {
    dev->scale = scale;
}

float load_cell_get_units(load_cell_t *dev, int times) {
    return ((float)(load_cell_read_average(dev, times) - dev->offset)) / dev->scale;
}