# Semáforo Inteligente

## Descrição

Projeto de semáforo inteligente usando Raspberry Pi Pico W (RP2040) e FreeRTOS.

Integra:

* Sinalização visual via LEDs (verde, vermelho) e matriz WS2812B.
* Alertas sonoros via buzzer (PWM).
* Display OLED SSD1306 para contagem e mensagens em tempo real.
* Controle de modos por botão.

## Componentes e Pinos

| Componente           | GPIO/Pino  | Função                                    |
| -------------------- | ---------- | ----------------------------------------- |
| LED Verde            | GPIO 11    | "Pode atravessar"                         |
| LED Vermelho         | GPIO 13    | "Pare"                                    |
| Matriz WS2812B       | GPIO 7     | Padrões de cor (verde, amarelo, vermelho) |
| Buzzer               | GPIO 21    | Alertas sonoros                           |
| Botão A              | GPIO 5     | Alterna Modo Normal ↔ Modo Noturno        |
| I²C SDA / SCL        | GPIO 14/15 | Comunicação com OLED SSD1306              |
| Display OLED SSD1306 | I2C        | Contagens e mensagens                     |

## Operação

### Modo Normal

Ciclo:

```
5 s Verde → 3 s Amarelo → 6 s Vermelho → repete
```

* **Verde**: 1 s de buzzer (pwm\_level=25), LED verde ON, matriz verde.
* **Amarelo**: bipes rápidos (50 ms ON / 50 ms OFF) por 3 s, matriz amarela.
* **Vermelho**: bipes intermitentes (500 ms ON / 1 500 ms OFF) até mudar, LED vermelho ON, matriz vermelha.

### Modo Noturno

* **Beep curto** de 300 ms a cada 2 s (intervalo entre inícios = 2 s).
* LEDs verde + vermelho piscam juntos 1 s ON / 1 s OFF.
* Matriz alterna entre ligada e apagada.
* Display mostra “MODO NOTURNO”.

## Instalação

1. **Hardware**

   * Raspberry Pi Pico W
   * LEDs (GPIO 11, 13), buzzer (GPIO 21), WS2812B (GPIO 7), SSD1306 (GPIO 14/15), botão A (GPIO 5)

2. **Dependências**

   * Pico SDK + CMake
   * `lib/ssd1306` (OLED)
   * `ws2812.pio` (PIO driver)
   * FreeRTOS (`FreeRTOSConfig.h`)

3. **Compilação**

   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

   Ou import utilizando a extensão do Raspberry Pico Pi do VSCode.

## Autor

Hugo Martins Santana (TIC370101267)
