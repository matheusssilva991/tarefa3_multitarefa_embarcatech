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
#include "lib/buzzer/buzzer.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define MATRIX_LED_PIN 7
#define TRAFFIC_LIGHT_DELAY_MS 2000

typedef struct traffic_light_config_t
{
    int rgb_led_state[3][3];     // Estado dos LEDs RGB (0: off, 1: on)
    int matrix_led_indexes[3];   // Índices dos LEDs na matriz
    int matrix_led_colors[3][3]; // Cores dos LEDs (R, G, B)
    bool is_night_mode;          // Modo noturno
    int buzzer_frequency[3];     // Frequência do buzzer
    int buzzer_active_time[3];   // Tempo do buzzer
    int buzzer_inactive_time[3]; // Tempo do buzzer inativo
} traffic_light_config_t;

void gpio_irq_handler(uint gpio, uint32_t events);
void vModeToggleTask();
void vLedMatrixTask();
void vRGBLedTask();
void vDisplayTask();
void vTrafficLightControlTask();
void vBuzzerTask();

/// Configuração do semáforo
volatile traffic_light_config_t tl_settings = {
    .rgb_led_state = {{0, 1, 0}, {1, 1, 0}, {1, 0, 0}},
    .matrix_led_indexes = {17, 12, 7},
    .matrix_led_colors = {{0, 8, 0}, {4, 8, 0}, {8, 0, 0}},
    .is_night_mode = false,
    .buzzer_frequency = {220, 1950, 450},     // Frequências do buzzer para cada estado
    .buzzer_active_time = {1000, 250, 500},    // Tempo do buzzer ativo para cada estado
    .buzzer_inactive_time = {1000, 250, 1500}, // Tempo do buzzer inativo para cada estado
};
volatile int light_state = 2; // Estado do semáforo (0: Verde, 1: Amarelo, 2: Vermelho)

int main()
{
    stdio_init_all();

    init_btn(BUTTON_B_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    init_buzzer(BUZZER_A_PIN, 4.0f); // Inicializa o PWM para o buzzer A
    init_buzzer(BUZZER_B_PIN, 4.0f); // Inicializa o PWM para o buzzer B

    xTaskCreate(vDisplayTask, "Display OLED", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vRGBLedTask, "Led RGB", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vLedMatrixTask, "Matriz de Led", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vModeToggleTask, "Mudar modo", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vTrafficLightControlTask, "Controle do Semáforo", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

void vModeToggleTask()
{
    init_btn(BUTTON_A_PIN);
    absolute_time_t last_press = 0;

    while (true)
    {
        absolute_time_t now = to_ms_since_boot(get_absolute_time());

        if (btn_is_pressed(BUTTON_A_PIN) && now - last_press > 250)
        {
            tl_settings.is_night_mode = !tl_settings.is_night_mode; // Alterna o modo
            last_press = now;
            stop_tone(BUZZER_A_PIN); // Para o tom do buzzer A
            stop_tone(BUZZER_B_PIN); // Para o tom do buzzer B

            if (tl_settings.is_night_mode)
            {
                light_state = 1; // Muda para o estado amarelo no modo noturno
            }

            printf("Modo noturno: %s\n", tl_settings.is_night_mode ? "Ativado" : "Desativado");
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vRGBLedTask()
{
    init_leds();

    while (true)
    {
        if (tl_settings.is_night_mode)
        {
            gpio_put(RED_LED_PIN, true);
            gpio_put(GREEN_LED_PIN, true);
            gpio_put(BLUE_LED_PIN, false);
        }
        else
        {
            gpio_put(RED_LED_PIN, tl_settings.rgb_led_state[light_state][0]);
            gpio_put(GREEN_LED_PIN, tl_settings.rgb_led_state[light_state][1]);
            gpio_put(BLUE_LED_PIN, tl_settings.rgb_led_state[light_state][2]);
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Atualiza frequentemente
    }
}

void vLedMatrixTask()
{
    ws2812b_init(MATRIX_LED_PIN);
    ws2812b_clear();

    while (true)
    {
        if (tl_settings.is_night_mode)
        {
            ws2812b_clear();
            ws2812b_set_led(12, 4, 8, 0); // Define o LED 12 como amarelo
            ws2812b_write();
        }
        else
        {
            ws2812b_clear();
            ws2812b_set_led(tl_settings.matrix_led_indexes[light_state],
                            tl_settings.matrix_led_colors[light_state][0],
                            tl_settings.matrix_led_colors[light_state][1],
                            tl_settings.matrix_led_colors[light_state][2]);
            ws2812b_write();
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Atualiza frequentemente
    }
}

void vDisplayTask()
{
    ssd1306_t ssd;      // Inicializa a estrutura do display
    init_display(&ssd); // Inicializa o display

    char mode_text[20]; // Buffer para armazenar o texto do modo
    bool cor = true;

    while (true)
    {

        if (tl_settings.is_night_mode)
            sprintf(mode_text, "Modo Noturno"); // Define o texto do modo
        else
            sprintf(mode_text, "Modo Normal"); // Define o texto do modo

        ssd1306_fill(&ssd, !cor);                     // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor); // Desenha um retângulo
        draw_centered_text(&ssd, mode_text, 8);       // Desenha o texto do modo
        ssd1306_line(&ssd, 3, 19, 127, 19, cor);      // Desenha uma linha

        if (light_state == 0)
        {
            draw_centered_text(&ssd, "Pode", 28);       // Desenha "Pode"
            draw_centered_text(&ssd, "Atravessar", 38); // Desenha "Atravessar"
        }
        else if (light_state == 1)
            draw_centered_text(&ssd, "Atencao!", 36); // Desenha "Atenção"
        else if (light_state == 2)
            draw_centered_text(&ssd, "Pare!", 36); // Desenha "Pare"

        ssd1306_send_data(&ssd);        // Atualiza o display
        vTaskDelay(pdMS_TO_TICKS(200)); // Atualiza frequentemente
    }
}

void vTrafficLightControlTask()
{
    while (true)
    {
        if (!tl_settings.is_night_mode)
        {
            // Atualiza o estado do semáforo
            light_state = (light_state + 1) % 3;               // Incrementa e reinicia para 0 após 2
            vTaskDelay(pdMS_TO_TICKS(TRAFFIC_LIGHT_DELAY_MS)); // Aguarda o tempo do estado atual
        }
        else
        {
            // No modo noturno, mantém o estado fixo
            vTaskDelay(pdMS_TO_TICKS(100)); // Aguarda um tempo menor no modo noturno
        }
    }
}

void vBuzzerTask()
{
    // Inicializa o buzzer
    init_buzzer(BUZZER_A_PIN, 4.0f); // Inicializa o PWM para o buzzer A
    init_buzzer(BUZZER_B_PIN, 4.0f); // Inicializa o PWM para o buzzer B

    while (true)
    {
        if (tl_settings.is_night_mode)
        {
            play_tone(BUZZER_A_PIN, 150);    // Toca um tom no buzzer A
            vTaskDelay(pdMS_TO_TICKS(2000)); // Aguarda dois segundos
            stop_tone(BUZZER_A_PIN);         // Para o tom
            vTaskDelay(pdMS_TO_TICKS(2000)); // Aguarda dois segundos
        }
        else
        {
            play_tone(BUZZER_B_PIN, tl_settings.buzzer_frequency[light_state]);       // Toca um tom no buzzer B
            vTaskDelay(pdMS_TO_TICKS(tl_settings.buzzer_active_time[light_state]));   // Aguarda o tempo ativo do buzzer
            stop_tone(BUZZER_B_PIN);                                                  // Para o tom
            vTaskDelay(pdMS_TO_TICKS(tl_settings.buzzer_inactive_time[light_state])); // Aguarda o tempo inativo do buzzer
        }
    }
}