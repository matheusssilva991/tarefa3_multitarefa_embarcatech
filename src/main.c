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
#define TRAFFIC_LIGHT_DELAY_MS 2000

void gpio_irq_handler(uint gpio, uint32_t events);
void vrgb_display_task();
void vmode_toggle_task();
void vDisplay3Task();
void vmatrix_display_task();

volatile bool is_night_mode = false;

int main()
{
    stdio_init_all();

    init_btn(BUTTON_B_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    xTaskCreate(vrgb_display_task, "Semáforo - Led RGB", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vmatrix_display_task, "Semáforo - Matriz de Led", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vmode_toggle_task, "Semáforo - Mudar modo", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY + 1, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

void vrgb_display_task()
{
    init_leds();

    while (true)
    {
        if (is_night_mode)
        {
            leds_off();
            gpio_put(GREEN_LED_PIN, true);
            gpio_put(RED_LED_PIN, true);
            // vTaskDelay(pdMS_TO_TICKS(TRAFFIC_LIGHT_DELAY_MS));
        }
        else
        {

            leds_off();
            gpio_put(GREEN_LED_PIN, true);
            for (int i = 0; i < TRAFFIC_LIGHT_DELAY_MS; i += 10) {
                if (is_night_mode) break;
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            leds_off();
            gpio_put(GREEN_LED_PIN, true);
            gpio_put(RED_LED_PIN, true);
            for (int i = 0; i < TRAFFIC_LIGHT_DELAY_MS; i += 10) {
                if (is_night_mode) break;
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            leds_off();
            gpio_put(RED_LED_PIN, true);
            for (int i = 0; i < TRAFFIC_LIGHT_DELAY_MS; i += 10) {
                if (is_night_mode) break;
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
    }
}

void vmode_toggle_task()
{
    init_btn(BUTTON_A_PIN);
    absolute_time_t last_press = 0;

    while (true)
    {
        absolute_time_t now = to_ms_since_boot(get_absolute_time());

        if (btn_is_pressed(BUTTON_A_PIN) && now - last_press > 250)
        {
            is_night_mode = !is_night_mode; // Alterna o modo
            last_press = now;

            printf("Modo noturno: %s\n", is_night_mode ? "Ativado" : "Desativado");
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vDisplay3Task()
{
    ssd1306_t ssd;      // Inicializa a estrutura do display
    init_display(&ssd); // Inicializa o display

    char str_y[5]; // Buffer para armazenar a string
    int contador = 0;
    bool cor = true;
    while (true)
    {
        sprintf(str_y, "%d", contador);                      // Converte em string
        contador++;                                          // Incrementa o contador
        ssd1306_fill(&ssd, !cor);                            // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);        // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);             // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);             // Desenha uma linha
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6);   // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);    // Desenha uma string
        ssd1306_draw_string(&ssd, "  FreeRTOS", 10, 28);     // Desenha uma string
        ssd1306_draw_string(&ssd, "Contador  LEDs", 10, 41); // Desenha uma string
        ssd1306_draw_string(&ssd, str_y, 40, 52);            // Desenha uma string
        ssd1306_send_data(&ssd);                             // Atualiza o display
        sleep_ms(735);
    }
}

void vmatrix_display_task()
{
    ws2812b_init(MATRIX_LED_PIN); // Inicializa o driver de LED

    ws2812b_clear(); // Limpa a matriz de LED
    while (true)
    {
        if (is_night_mode)
        {
            ws2812b_clear();              // Limpa a matriz de LED
            ws2812b_set_led(12, 4, 8, 0); // Define o LED 12 como amarelo
            ws2812b_write();
            vTaskDelay(pdMS_TO_TICKS(250));
        }
        else
        {
            ws2812b_clear();              // Limpa a matriz de LED
            ws2812b_set_led(17, 0, 8, 0); // Define o LED 17 como verde
            ws2812b_write();
            for (int i = 0; i < TRAFFIC_LIGHT_DELAY_MS; i += 10) {
                if (is_night_mode) break;
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            ws2812b_clear();              // Limpa a matriz de LED
            ws2812b_set_led(12, 4, 8, 0); // Define o LED 12 como amarelo
            ws2812b_write();
            for (int i = 0; i < TRAFFIC_LIGHT_DELAY_MS; i += 10) {
                if (is_night_mode) break;
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            ws2812b_clear();             // Limpa a matriz de LED
            ws2812b_set_led(7, 8, 0, 0); // Define o LED 7 como vermelho
            ws2812b_write();
            for (int i = 0; i < TRAFFIC_LIGHT_DELAY_MS; i += 10) {
                if (is_night_mode) break;
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
    }
}
