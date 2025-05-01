#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "lib/ssd1306/ssd1306.h"
#include "lib/ssd1306/display.h"
#include "lib/led/led.h"
#include "lib/button/button.h"
#include "lib/ws2812b/ws2812b.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define MATRIX_LED_PIN 7

void gpio_irq_handler(uint gpio, uint32_t events);
void vBlinkLed1Task();
void vBlinkLed2Task();
void vDisplay3Task();
void vMatrixDisplayTask();

bool is_night_mode = false;

int main()
{
    stdio_init_all();

    init_btn(BUTTON_B_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    /* xTaskCreate(vBlinkLed1Task, "Blink Task Led1", configMINIMAL_STACK_SIZE,
         NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBlinkLed2Task, "Blink Task Led2", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplay3Task, "Cont Task Disp3", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL); */
    xTaskCreate(vMatrixDisplayTask, "Semáforo - Matriz de Led", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

void vBlinkLed1Task()
{
    init_led(GREEN_LED_PIN);

    while (true)
    {
        gpio_put(GREEN_LED_PIN, true);
        vTaskDelay(pdMS_TO_TICKS(250));
        gpio_put(GREEN_LED_PIN, false);
        vTaskDelay(pdMS_TO_TICKS(1223));
    }
}

void vBlinkLed2Task()
{
    init_led(BLUE_LED_PIN);

    while (true)
    {
        gpio_put(BLUE_LED_PIN, true);
        vTaskDelay(pdMS_TO_TICKS(250));
        gpio_put(BLUE_LED_PIN, false);
        vTaskDelay(pdMS_TO_TICKS(1223));
    }
}

void vDisplay3Task()
{
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    init_display(&ssd);                                          // Inicializa o display

    char str_y[5]; // Buffer para armazenar a string
    int contador = 0;
    bool cor = true;
    while (true)
    {
        sprintf(str_y, "%d", contador); // Converte em string
        contador++;                     // Incrementa o contador
        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);           // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6); // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);  // Desenha uma string
        ssd1306_draw_string(&ssd, "  FreeRTOS", 10, 28); // Desenha uma string
        ssd1306_draw_string(&ssd, "Contador  LEDs", 10, 41);    // Desenha uma string
        ssd1306_draw_string(&ssd, str_y, 40, 52);          // Desenha uma string
        ssd1306_send_data(&ssd);                           // Atualiza o display
        sleep_ms(735);
    }
}

void vMatrixDisplayTask(){
    ws2812b_init(MATRIX_LED_PIN); // Inicializa o driver de LED

    ws2812b_clear(); // Limpa a matriz de LED
    while (true) {
        if (is_night_mode) {
            ws2812b_clear(); // Limpa a matriz de LED
            ws2812b_set_led(12, 4, 8, 0); // Define o LED 12 como amarelo
            ws2812b_write();
            vTaskDelay(pdMS_TO_TICKS(250));

        } else {
            ws2812b_clear(); // Limpa a matriz de LED
            ws2812b_set_led(17, 0, 8, 0); // Define o LED 17 como verde
            ws2812b_write();
            vTaskDelay(pdMS_TO_TICKS(2000));

            ws2812b_clear(); // Limpa a matriz de LED
            ws2812b_set_led(12, 4, 8, 0); // Define o LED 12 como amarelo
            ws2812b_write();
            vTaskDelay(pdMS_TO_TICKS(2000));

            ws2812b_clear(); // Limpa a matriz de LED
            ws2812b_set_led(7, 8, 0, 0); // Define o LED 7 como vermelho
            ws2812b_write();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}

