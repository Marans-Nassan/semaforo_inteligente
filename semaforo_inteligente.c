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

#define botao_a 5
#define matriz_led 7
#define green_led 11
#define red_led 13
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define buzzer_a 21
#define matriz_led_pins 25

ssd1306_t ssd;

typedef struct pixeis {
    uint8_t cor1, cor2, cor3;
}pixeis;

pixeis leds [matriz_led_pins];

PIO pio;
uint sm;

TaskHandle_t vLed_basico_handler = NULL;
TaskHandle_t vMatriz_handler = NULL;
TaskHandle_t vSom_handler = NULL;
TaskHandle_t vDisplay_handler = NULL;
TaskHandle_t vMatriz_noturno_handler = NULL;
TaskHandle_t vLed_basico_noturno_handler = NULL;
TaskHandle_t vSom_noturno_handler = NULL;
TaskHandle_t vDisplay_noturno_handler = NULL;



volatile bool modo_noturno = false;

// Protótipos
void ledinit();
void botinit();
void matriz_init(uint pin); //inicialização da matriz de led.
void setled(const uint index, const uint8_t r, const uint8_t g, const uint8_t b); //configuração para permitir o uso da função display.
void matriz(uint8_t r, uint8_t g, uint8_t b);
void display(); //esta é a função responsável por permitir ditar qual led vai acender.
void i2cinit();
void oledinit();
void oledisplay(uint8_t segundos);
void pwm_setup();
void pwm_level(uint32_t duty_cycle);
void vAlternar_modo();
void vModo();
void vLed_basico();
void vMatriz();
void vSom();
void vDisplay();
void vLed_basico_noturno();
void vMatriz_noturno();
void vSom_noturno();
void vDisplay_noturno();
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
    ledinit();
    matriz_init(matriz_led);
    pwm_setup();
    i2cinit();
    oledinit();
    xTaskCreate(vAlternar_modo, "Pressionamento do botão", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vModo, "Alternância de modo", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();

 
}

void vAlternar_modo(){
    botinit();
    static bool botao_a_estado = true;
    static uint64_t last_time = 0;

        while(true){
            uint64_t current_time = to_us_since_boot(get_absolute_time()) /1000000;
            bool botao_pressionado = gpio_get(botao_a);

            if(!botao_pressionado && !botao_a_estado && (current_time - last_time > 1)) {
                modo_noturno = !modo_noturno;
                xTaskCreate(vModo, "Alternância de modo", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
                last_time = current_time;
            }
        botao_a_estado = botao_pressionado;
        vTaskDelay(pdMS_TO_TICKS(10));
        }
}

void vModo(){
    vTaskDelay(pdMS_TO_TICKS(10));
    if(!modo_noturno){
        xTaskCreate(vLed_basico, "Led básico", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &vLed_basico_handler);
        xTaskCreate(vMatriz, "Matriz", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &vMatriz_handler);
        xTaskCreate(vSom, "Som da sinaleira", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &vSom_handler);
        xTaskCreate(vDisplay, "Contagem no display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &vDisplay_handler);
        vTaskDelete(vLed_basico_noturno_handler);
        vTaskDelete(vMatriz_noturno_handler);
        vTaskDelete(vSom_noturno_handler);
        vTaskDelete(vDisplay_noturno_handler);


    }
    else{
        xTaskCreate(vMatriz_noturno, "Matriz modo noturno", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &vMatriz_noturno_handler);
        xTaskCreate(vLed_basico_noturno, "Led modo noturno", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &vLed_basico_noturno_handler);
        xTaskCreate(vSom_noturno, "Som da sinaleira noturna", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &vSom_noturno_handler);
        xTaskCreate(vDisplay_noturno, "Mensagem noturna", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &vDisplay_noturno_handler);
        vTaskDelete(vLed_basico_handler);
        vTaskDelete(vMatriz_handler);
        vTaskDelete(vSom_handler);
        vTaskDelete(vDisplay_handler);

    }
    vTaskDelete(NULL);
}

void vLed_basico(){
    gpio_put(green_led, 0);
    gpio_put(red_led, 0);
        while(true){
            gpio_put(green_led, 1);
            vTaskDelay(pdMS_TO_TICKS(5000));
            gpio_put(red_led, 1);
            vTaskDelay(pdMS_TO_TICKS(3000));
            gpio_put(green_led, 0);
            vTaskDelay(pdMS_TO_TICKS(6000));
            gpio_put(red_led, 0);
        }
}

void vMatriz(){
        while(true){
            matriz(0, 100, 0);
            vTaskDelay(pdMS_TO_TICKS(5000));
            matriz(1, 100 ,0);
            vTaskDelay(pdMS_TO_TICKS(3000));
            matriz(1, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(6000));
        }

}

void vSom(){
    while(true){
        for(uint8_t i = 0; i < 50; i++ ){
            pwm_level(25);
            vTaskDelay(pdMS_TO_TICKS(50));
            pwm_level(0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        
        for(uint8_t i = 0; i < 15; i++ ){
            pwm_level(25);
            vTaskDelay(pdMS_TO_TICKS(100));
            pwm_level(0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        for(uint8_t i = 0; i < 12; i++ ){
            pwm_level(25);
            vTaskDelay(pdMS_TO_TICKS(250));
            pwm_level(0);
            vTaskDelay(pdMS_TO_TICKS(250));
        }
    }
}


void vDisplay(){
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    while(true){
        for(uint8_t i = 5 ; i >= 0; i--){
            oledisplay(i);
        }
        for(uint8_t i = 3 ; i >= 0; i--){
            oledisplay(i);
        }
        for(uint8_t i = 6 ; i >= 0; i--){
            oledisplay(i);
        }
    }
}

void vLed_basico_noturno(){
    gpio_put(green_led, 0);
    gpio_put(red_led, 0);
    while(true){
        gpio_put(green_led, 1);
        gpio_put(red_led, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_put(green_led, 0);
        gpio_put(red_led, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vMatriz_noturno(){
        while(true){
            matriz(1, 100, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
            matriz(0, 0 ,0);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

}

void vSom_noturno(){
    while(true){
        pwm_level(25);
        vTaskDelay(pdMS_TO_TICKS(150));
        pwm_level(0);
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void vDisplay_noturno(){
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
        while(true){
            ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, true, false);
            ssd1306_draw_string(&ssd, "MODO NOTURNO", 15, 25);
            ssd1306_send_data(&ssd);
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

void botinit(){
    gpio_init(botao_a);
    gpio_set_dir(botao_a, GPIO_IN);
    gpio_pull_up(botao_a);
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

void oledisplay(uint8_t segundos){
    char tempo[3];
    tempo[0] = '0' + (segundos / 10);
    tempo[1] = '0' + (segundos % 10);
    tempo[2] = '\0';
    for(uint8_t i = 0; i <= 2 ; i++){
        ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, true, false);
        ssd1306_draw_string(&ssd, tempo, 58, 25 );
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    
}

void pwm_setup(){
    uint8_t slice;
    gpio_set_function(buzzer_a, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(buzzer_a);
    pwm_set_clkdiv(slice, 64.0);
    pwm_set_wrap(slice, 3900);
    pwm_set_gpio_level(buzzer_a, 0);
    pwm_set_enabled(slice, true);
}

void pwm_level(uint32_t duty_cycle){
    pwm_set_gpio_level(buzzer_a, 3900 * duty_cycle / 100);
}