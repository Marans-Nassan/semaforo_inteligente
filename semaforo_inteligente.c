#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "ws2812.pio.h"
#include "lib/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"


#define matriz_led 7
#define green_led 11
#define red_led 13
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define buzzer 21
#define matriz_led_pins 25

ssd1306_t ssd;

typedef struct pixeis {
    uint8_t cor1, cor2, cor3;
}pixeis;

pixeis leds [matriz_led_pins];

PIO pio;
uint sm;

const volatile bool modo_noturno = false;

// Protótipos
void ledinit();
void matriz_init(uint pin); //inicialização da matriz de led.
void setled(const uint index, const uint8_t r, const uint8_t g, const uint8_t b); //configuração para permitir o uso da função display.
void matriz(uint8_t r, uint8_t g, uint8_t b);
void display(); //esta é a função responsável por permitir ditar qual led vai acender.
void i2cinit();
void oledinit();
void vLed_basico();
void vMatriz();
void vDisplay();
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
    xTaskCreate(vLed_basico, "Led básico", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vMatriz, "Matriz", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
 
}

void vLed_basico(){
    ledinit();
        while(true){
            gpio_put(green_led, 1);
            vTaskDelay(pdMS_TO_TICKS(5000));
            gpio_put(red_led, 1);
            vTaskDelay(pdMS_TO_TICKS(5000));
            gpio_put(green_led, 0);
            vTaskDelay(pdMS_TO_TICKS(5000));
            gpio_put(red_led, 0);
        }
}

void vMatriz(){
    matriz_init(matriz_led);
        while(true){
            matriz(0, 100, 0);
            vTaskDelay(pdMS_TO_TICKS(5000));
            matriz(1, 100 ,0);
            vTaskDelay(pdMS_TO_TICKS(5000));
            matriz(1, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(5000));
        }

}

void vDisplay(){
    oledinit();

        while(true){
            
        }
}

void ledinit(){ //inicialização dos leds básicos.
    uint8_t leds[2] = {11, 13};
    for(uint8_t i = 0 ; i < 2; i++){
        gpio_init(leds[i]);
        gpio_set_dir(leds[i], GPIO_OUT);
        gpio_put(leds[i], 0);
    }
}

void matriz_init(uint pin){

    uint offset = pio_add_program(pio0, &ws2812_program);
    pio = pio0;
    
    sm = pio_claim_unused_sm(pio, false);
        if(sm < 0){
            pio = pio1;
            sm = pio_claim_unused_sm(pio, true);
        }
    
    ws2812_program_init(pio, sm, offset, pin, 800000.f);
    }

void setled(const uint index, const uint8_t r, const uint8_t g, const uint8_t b){
    leds[index].cor1 = g;
    leds[index].cor2 = r;
    leds[index].cor3 = b;
}

void matriz(uint8_t r, uint8_t g, uint8_t b){
    const uint8_t digit_leds[] = {24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    size_t count = sizeof(digit_leds)/sizeof (digit_leds[0]);
        for (size_t i = 0; i < count; ++i) {
            setled(digit_leds[i], r, g, b); // RGB
        }
        display();    
}

void display(){
    for (uint i = 0; i < matriz_led_pins; ++i) {
        pio_sm_put_blocking(pio, sm, leds[i].cor1);
        pio_sm_put_blocking(pio, sm, leds[i].cor2);
        pio_sm_put_blocking(pio, sm, leds[i].cor3);
        }
    sleep_us(100); 
}

void i2cinit(){ //iniciando o i2c
    i2c_init(I2C_PORT, 400*1000);
        
        for(uint8_t i = 14 ; i < 16; i++){
            gpio_set_function(i, GPIO_FUNC_I2C);
            gpio_pull_up(i);
        }
}

void oledinit(){
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}