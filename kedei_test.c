#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>

#define LCD_CS RPI_GPIO_P1_24 // GPIO 8
#define DATA_BIT 10           // GPIO 10 (MOSI)
#define CLOCK_BIT 11          // GPIO 11 (SCLK)

// Send 8-bit command/data through shift registers
void send_data_595(unsigned char data) {
    for (int i = 0; i < 8; i++) {
        bcm2835_gpio_write(CLOCK_BIT, LOW);
        if (data & 0x80)
            bcm2835_gpio_write(DATA_BIT, HIGH);
        else
            bcm2835_gpio_write(DATA_BIT, LOW);
        bcm2835_gpio_write(CLOCK_BIT, HIGH);
        data <<= 1;
    }
}

void write_command(unsigned char cmd) {
    bcm2835_gpio_write(LCD_CS, LOW);
    // 74HC595 control bit for Command goes here
    send_data_595(cmd); 
    bcm2835_gpio_write(LCD_CS, HIGH);
}

void write_data(unsigned char data) {
    bcm2835_gpio_write(LCD_CS, LOW);
    // 74HC595 control bit for Data goes here
    send_data_595(data);
    bcm2835_gpio_write(LCD_CS, HIGH);
}

int main() {
    if (!bcm2835_init()) return 1;
    
    bcm2835_gpio_fsel(LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(DATA_BIT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(CLOCK_BIT, BCM2835_GPIO_FSEL_OUTP);
    
    // Example: Soft Reset
    write_command(0x01); 
    bcm2835_delay(120);

    // Turn Display On
    write_command(0x29);

    // Simple fill screen (e.g., solid red, 5-6-5 format: 0xF800)
    write_command(0x2A); // Column addr
    write_data(0x00); write_data(0x00); write_data(0x01); write_data(0x3F);
    write_command(0x2B); // Page addr
    write_data(0x00); write_data(0x00); write_data(0x01); write_data(0xDF);
    
    write_command(0x2C); // Memory Write
    for(int i=0; i<480*320; i++) {
        write_data(0xF8); // Red High
        write_data(0x00); // Red Low
    }

    bcm2835_close();
    return 0;
}
