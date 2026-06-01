#include "lcd.h"
#include "driver/i2c.h"
#include "pins.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"


#define LCD_ADDR 0x27 
#define I2C_PORT I2C_NUM_0

static void lcd_send_cmd(char cmd) {
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (cmd & 0xf0);
    data_l = ((cmd << 4) & 0xf0);
    data_t[0] = data_u | 0x0C; // en=1, rs=0
    data_t[1] = data_u | 0x08; // en=0, rs=0
    data_t[2] = data_l | 0x0C; // en=1, rs=0
    data_t[3] = data_l | 0x08; // en=0, rs=0
    i2c_master_write_to_device(I2C_PORT, LCD_ADDR, data_t, 4, 1000 / portTICK_PERIOD_MS);
}

static void lcd_send_data(char data) {
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (data & 0xf0);
    data_l = ((data << 4) & 0xf0);
    data_t[0] = data_u | 0x0D; // en=1, rs=1
    data_t[1] = data_u | 0x09; // en=0, rs=1
    data_t[2] = data_l | 0x0D; // en=1, rs=1
    data_t[3] = data_l | 0x09; // en=0, rs=1
    i2c_master_write_to_device(I2C_PORT, LCD_ADDR, data_t, 4, 1000 / portTICK_PERIOD_MS);
}

void lcd_clear(void) {
    lcd_send_cmd(0x01);
    esp_rom_delay_us(2000); // 2ms delay required for clear command
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54}; 
    if (row > 3) row = 3;
    lcd_send_cmd(0x80 | (col + row_offsets[row]));
}

void lcd_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = LCD_SDA_PIN,
        .scl_io_num = LCD_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);

    vTaskDelay(pdMS_TO_TICKS(50)); // Wait for LCD to fully power up

    lcd_send_cmd(0x30); esp_rom_delay_us(5000);
    lcd_send_cmd(0x30); esp_rom_delay_us(1000);
    lcd_send_cmd(0x30); esp_rom_delay_us(1000);
    lcd_send_cmd(0x20); esp_rom_delay_us(1000);

    lcd_send_cmd(0x28); esp_rom_delay_us(1000); 
    lcd_send_cmd(0x08); esp_rom_delay_us(1000);
    lcd_send_cmd(0x01); esp_rom_delay_us(2000);
    lcd_send_cmd(0x06); esp_rom_delay_us(1000);
    lcd_send_cmd(0x0C); esp_rom_delay_us(1000);
}

void lcd_send_string(const char *str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}

void lcd_send_int(int num) {
    char str[12]; 
    int i = 0;
    bool is_negative = false;

    if (num == 0) {
        lcd_send_string("0");
        return;
    }

    if (num < 0) {
        is_negative = true;
        num = -num; 
    }

    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }
    str[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
    lcd_send_string(str);
}