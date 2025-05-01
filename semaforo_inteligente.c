#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ws2812.pio.h"

#define matriz_led 7
#define blue_led 11
#define green_led 12
#define red_led 13
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define matriz_led_pins 25

PIO pio;
uint sm;

typedef struct pixeis {
    uint8_t R, G, B;
}pixeis;

pixeis leds [matriz_led_pins];

void ledinit();
void vSemaforo_normal();
//Facilitador de reset ---------------------------
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main(){
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
//Facilitador de reset -----------------------------    
    stdio_init_all();

    xTaskCreate(vSemaforo_normal, "Semáforo modo normal", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
 
}

void ledinit(){ //inicialização dos leds básicos.
    for(uint8_t i = 11 ; i < 14; i++){
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        gpio_put(i, 0);
    }
}

void vSemaforo_normal(){
    ledinit();
    while(true){
        gpio_put(blue_led, !gpio_get(blue_led));
        gpio_put(red_led, !gpio_get(red_led));
        vTaskDelay(pdMS_TO_TICKS(1000));
        }
}